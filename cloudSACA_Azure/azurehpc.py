import sys, unittest, paramiko, os, time
from base64 import b64encode
from M2Crypto import RSA
from getpass import getpass
import logging
import datetime as mytime
from azure import *
from azure import WindowsAzureConflictError
from azure import WindowsAzureError
from azure.storage.blobservice import BlobService
from azure.storage.storageclient import _StorageClient
from azure.servicemanagement import LinuxConfigurationSet
from azure.servicemanagement import OSVirtualHardDisk
import re
from azure.servicemanagement import * 
from azure.servicemanagement.servicemanagementservice import ServiceManagementService
import base64
import urllib2
import subprocess
import socket
import random
import urllib2
import send
"""
	This module provides a python library for Windows Azure and creating the following tasks
	
	0- Creating cluster on Azure .................. Ok
	1- Start machines.............................. OK
	2- attache and detache disks. 
	3- access the IPs of this machines............. OK
	4- terminate machines. ........................ Ok
	5- Passwordless authentication access.......... Ok
	6- parallelize the instance creation ...........Ok
	7- delete machines .............................Ok
	9- listing available nodes in cluster ..........Ok
	10-Restart Machines ............................OK

"""
class ehpcazure:
	#Initialization:
	#---------------
	def __init__(self):
		
		self.CHUNK_SIZE = 4 * 1024 * 1024
		self.PAGE_SIZE	= 512
	
		# Configuring Logging:
		#---------------------
		mainPath 	= os.path.dirname(os.path.abspath(__file__))
		self.logger = logging.getLogger()
		self.logger.setLevel(logging.DEBUG)
		cha = logging.StreamHandler( sys.__stdout__ )
		Logs_file_name = mytime.datetime.now()
		Logs_file_name = mainPath+"/logs/"+str(Logs_file_name).replace(" ",'-').replace(":","-")+".logs"
		loggertoFile = logging.FileHandler(Logs_file_name)
		loggertoFile.setLevel(logging.DEBUG)	
		cha.setLevel(logging.DEBUG)
		formatter = logging.Formatter('%(levelname)s -- %(message)s')
		cha.setFormatter(formatter)
		loggertoFile.setFormatter(formatter)
		self.logger.addHandler(cha)
		self.logger.addHandler(loggertoFile)
		self.chunk_size = 4 * 1024 * 1024
		self.pageSize	= 512

	# Connect To Azure Platform:
	#---------------------------
	def CheckSubscription(self, sub_id,cert_path):
		try:
			sms = ServiceManagementService(sub_id, cert_path)
			self.logger.info("Connect to the Azure platform and check that the location is valid")
			return sms
		except WindowsAzureError:
			self.logger.info("Error: you subscription is not valid")
			sys.exit(0)

	# Create Storage account:
	#------------------------
	def StorageCreate(self, sms,name,label,desc,location,affGrp):
		if name not  in [storageAcc.service_name for storageAcc in sms.list_storage_accounts()]:
			try:
				result = sms.create_storage_account(name, desc, label,affinity_group=affGrp)
				time.sleep(5)
				operation_result = sms.get_operation_status(result.request_id)
				self.logger.info('Create Storage Account status: %s', operation_result.status)
				return name
			except WindowsAzureConflictError:
				self.logger.info("Error: Storage Service name is already found")
				sys.exit(1)
		else:
			storageProp = sms.get_storage_account_properties(name)
			self.logger.info("Storage Account %s is already exist", name)
			return name

	# Create Account Service:
	#------------------------
	def newCloudService(self, sms,name,label,desc,loc,affGrp):
		if name not in [service.service_name for service in sms.list_hosted_services()]:
			try:
				self.logger.info("Creating Hosted Service: "+name)
				result = sms.create_hosted_service(name, label, desc,affinity_group=affGrp)
				time.sleep(60)
				serviceProb = sms.get_hosted_service_properties(name)
				self.logger.info("Cloud Sevice URL: %s",serviceProb.url)
				return serviceProb.url
			except WindowsAzureConflictError:
				self.logger.info("Error: Can not create Cloud Service")
				sys.exit(1)
		else:
			self.logger.info("Cloud Service %s is already exist", name)
			serviceProb = sms.get_hosted_service_properties(name)
			return serviceProb.url

	# Create Affinity Group:
	#-----------------------
	def createAffinityGrp(self, sms,name,label,desc,location):
		if name not in [affGrp.name for affGrp in sms.list_affinity_groups()]:
			self.logger.info("Creating Affinity Group %s", name)
			result 	= sms.create_affinity_group(name, label, location, desc)
			return name
		else:
			self.logger.info("Affininty Group %s is already exist", name)
			return name

	# Create Virtual Machine:
	#------------------------
	def createInstances(self, sms,name, region, imageID, instanceSize,pkfile,count, thumbprint,cert_data,num_instances,certPasswd,storagename,master_size):
		hostname		= name
		mainPath		= os.path.dirname(os.path.abspath(__file__))
		username		= "ehpcuser"
		password		= None 
		affGrp			= self.createAffinityGrp(sms,storagename,storagename,storagename,region)
		storage_name	= self.StorageCreate(sms,storagename,storagename,storagename,region,affGrp)
		account_key		= self.getStorageAccountKey(sms,storage_name)
		permission		= None
		container_name	= self.get_vm_name()
		account_name	= storage_name
		container_name		= self.create_containers(sms,storage_name,account_key,container_name.lower(),None)
		time.sleep(5)
		#print "Container Name:"+disks
		medialink		= "http://"+storage_name+".blob.core.windows.net/"+container_name+"/"
		# --------------------------------------------------------
		blobService = BlobService(account_name=storage_name, account_key=account_key)
		blobName	= container_name+"blob.vhd"
		try: 
			image = sms.get_os_image(imageID)
		except: 
			if(self.copy_image_vmDepot(blobService,container_name,blobName,imageID,sms)):
				print "INFO -- The Disk Blob has been copied"
			media_link 	= "http://"+storage_name+".blob.core.windows.net/"+container_name+"/"+blobName 
			if(self.make_os_image(sms,media_link,imageID)):
				print "INFO -- The image '"+imageID+"' is ready now!!"
			
			else:
				print "Error: Can not complete creating The image"
				exit(0)
		
		#-----------------------------------------------------------
		medialink		= "http://"+storage_name+".blob.core.windows.net/"+container_name+"/"
		media_link		= ""
		# Configuring EndPoint "Firwall Configuration":
		endpoint_config = ConfigurationSet()
		endpoint_config.configuration_set_type = 'NetworkConfiguration'
		endpoint1 = ConfigurationSetInputEndpoint(name='XML', protocol='tcp', port='5000', local_port='5000', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
		endpoint_config.input_endpoints.input_endpoints.append(endpoint1)
		self.logger.info("Configuring EndPoints 'Firwall Configuration' SHH, PBS Torque and OpenMPI ports")
		# Linux VM Configuration:
		linux_config = LinuxConfigurationSet(hostname, username, password , True)
		publickey = PublicKey(thumbprint, pkfile)
		linux_config.ssh.public_keys.public_keys.append(publickey)
		self.logger.info("Linux VM Configuration")
		# Configuring Image ID:
		#----------------------
		os_hd 	   = OSVirtualHardDisk(imageID, media_link)
		self.logger.info("Configuring The Virtual Hard Disk using Image ID: %s", imageID)
		
		# Start Deployment of VM on Azure:
		self.logger.info("Start Deployment of Elastic hpc on Azure")
		
		# Configuring Certificates:
		cert_format = 'pfx'
		cert_password = certPasswd
		VMname = hostname
		vmname = hostname
		instances = []
		
		try:
			for num in range(count):
				name 		= vmname+str(num)
				vname		= vmname
				Service_name= vname
				Service_url 	= self.newCloudService(sms,Service_name,Service_name,Service_name,region,affGrp)
				endpoint3 = ConfigurationSetInputEndpoint(name='SSH'+str(num), protocol='tcp', port='220'+str(num), local_port='22', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
				endpoint_config.input_endpoints.input_endpoints.append(endpoint3)
				#endpoint4 = ConfigurationSetInputEndpoint(name="FTP", protocol='tcp', port='21', local_port='21', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
				#endpoint_config.input_endpoints.input_endpoints.append(endpoint4)
				#endpoint5 = ConfigurationSetInputEndpoint(name="FTP1", protocol='tcp', port='20', local_port='20', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
				#endpoint_config.input_endpoints.input_endpoints.append(endpoint5)
				#endpoint6 = ConfigurationSetInputEndpoint(name="FTPudp", protocol='udp', port='21', local_port='21', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
				#endpoint_config.input_endpoints.input_endpoints.append(endpoint6)
				#for i in range(6):
				#	endpointpasv = ConfigurationSetInputEndpoint(name="FTPpasv"+str(i), protocol='tcp', port='4000'+str(i), local_port='4000'+str(i), load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
				#	endpoint_config.input_endpoints.input_endpoints.append(endpointpasv)
				pbs_endpoints = self.get_pbs_endpoints(0)
				for endpoint in pbs_endpoints:
					endpoint_config.input_endpoints.input_endpoints.append(endpoint)
				media_link	= medialink+name[:-1]+".vhd"
				self.logger.info("Configuring Media Link %s",media_link)
				# Configuring Image ID:
				#----------------------
				os_hd 	   = OSVirtualHardDisk(imageID, media_link)
				self.logger.info("Configuring The Virtual Hard Disk using Image ID: %s", imageID)
				self.logger.info("Deploying Node number: %d",num)
				result_cert = sms.add_service_certificate(service_name=Service_name,
								data=cert_data,
								certificate_format=cert_format,
								password=cert_password)
				self.logger.info("Start Deploying VM with Name: "+vname)
				try:
					self.logger.info(vars(result_cert))
				except:
					self.logger.info(result_cert)
				time.sleep(5)
				result = sms.create_virtual_machine_deployment(service_name=Service_name,
					deployment_name=vname,
					deployment_slot='production',
					label=Service_name,
					role_name=vname,
					system_config=linux_config,
					os_virtual_hard_disk=os_hd,
					network_config=endpoint_config,
					role_size=master_size)
					#role_size="Large")
				self.logger.info("Start Deployment")
				self.wait_for_async(sms, result.request_id)
				self.wait_for_deployment_status(sms,Service_name, vname, 'Running')
				self.wait_for_role_instance_status(sms,Service_name, vname, vname, 'ReadyRole')
				instances.append(Service_name+".cloudapp.net")
				instances.append(container_name)
		except WindowsAzureConflictError:
			self.logger.info("Error: Can not Create VM")
			sys.exit(1)
		return instances
	# Get Status of cluster nodes:
	#-----------------------------
	def getRolesStatus(self,num,sms,service_name,deployment,vname):
		try:
			for i in range(int(num)):
				self.logger.info("Role name: "+vname[:-1]+str(i))
				self.wait_for_deployment_status(sms,service_name, deployment, 'Running')
				self.wait_for_role_instance_status(sms,service_name, deployment, vname[:-1]+str(i), 'ReadyRole')
			self.logger.info("Cluster is ready")
		except:
			self.logger.info("Error: Role Status")
			sys.exit(1)
	# Get status of a single node:
	#-----------------------------
	def getRoleStatus(self,sms,service_name,deployment,vname):
		try:
			self.wait_for_deployment_status(sms,service_name,deployment,'Running')
			self.wait_for_role_instance_status(sms,service_name,deployment,vname,'ReadyRole')
			self.logger.info("Machine is ready")
		except:
			self.logger.info("Error: Can not start role "+ vname)
			sys.exit(1)
	# wait for asynchronous:
	#-----------------------
	def wait_for_async(self, sms, request_id):
		count = 0
		toolbar_width = 40
		i = 0
		sys.stdout.write("INFO -- Submitting Request In Progress : [%s]" % (" " * toolbar_width))
		sys.stdout.flush()
		sys.stdout.write("\b" * (toolbar_width+1))
		result = sms.get_operation_status(request_id)
		i = 1
		"""while True:
			print result.status
			time.sleep(4)
			print str(i)+"--------"
			i = i +1
			result = sms.get_operation_status(request_id)
			if result.status != "Failed":
				break
		"""
		while result.status == 'InProgress':
			count = count + 1
			i +=1
			sys.stdout.write("-")
			sys.stdout.flush()
			
			if count > 120:
				self.logger.info('Timed out waiting for async operation to complete.')
				sys.exit(0)
				time.sleep(4)
			result = sms.get_operation_status(request_id)
			if result.status == "Failed":
				print "\nFailed Request\n"
				break
		for i in xrange(toolbar_width-i):
				time.sleep(0.1) # do real work here
				# update the bar
				sys.stdout.write("-")
				sys.stdout.flush()
		sys.stdout.write("\n")
		self.logger.info('Wait for async process')
		

	# wait for deployment to be running:
	#-----------------------------------
	def wait_for_deployment_status(self, sms, service_name, deployment_name,status):
		count = 0
		props = sms.get_deployment_by_name(service_name, deployment_name)
		while props.status != status:
			print props.status
			count = count + 1
			if count > 120:
				self.logger.info('Timed out waiting for deployment status.')
				sys.exit(0)
			time.sleep(6)
			props = sms.get_deployment_by_name(service_name, deployment_name)

	# check role instance status:
	#---------------------------- 
	def wait_for_role_instance_status(self, sms, service_name, deployment_name, role_instance_name, status):
		count = 0
		toolbar_width = 40
		i = 0
		sys.stdout.write("INFO -- Deploying In Progress : [%s]" % (" " * toolbar_width))
		sys.stdout.flush()
		sys.stdout.write("\b" * (toolbar_width+1))
		props = sms.get_deployment_by_name(service_name, deployment_name)
		while self.get_role_instance_status(props, role_instance_name) != status:
			sys.stdout.write("-")
			sys.stdout.flush()
			i +=1
			count = count + 1
			if count > 120:
				self.logger.info('Timed out waiting for role instance status.')
				sys.exit(0)
			time.sleep(6)
			props = sms.get_deployment_by_name(service_name, deployment_name)
		if (props == False):
			self.logger.info("Deployment is not completed!!")
			sys.exit(0)
		else:
			for i in xrange(toolbar_width-i):
					time.sleep(0.1) # do real work here
					# update the bar
					sys.stdout.write("-")
					sys.stdout.flush()
			sys.stdout.write("\n")
			self.logger.info("Role is ready")

	# Get role instance status:
	#--------------------------
	def get_role_instance_status(self,deployment, role_instance_name):
		for role_instance in deployment.role_instance_list:
			if role_instance.instance_name == role_instance_name:
				return role_instance.instance_status
		return None
	
	# Get disks list:
	#---------------------
	def list_os_disks(self,sms,service_name, deployment, role_name):
		base_role 	= role_name[:-1]
		disk_names	= []
		i = 0
		while(1):
			role_name = base_role+str(i)
			if(self.check_role_exists(sms,service_name,deployment,role_name)!= False):
				i +=1
				role_props = sms.get_role(service_name,deployment,role_name)
				disk_name = role_props.os_virtual_hard_disk.disk_name
				disk_names.append(disk_name)
				role_name = base_role
			else:
				break
				
		return disk_names
	# Delete Instances:
	#------------------
	def deleteInstances(self, sms, service_name, deployment,role_name):
			#try:
			disk_names =self.list_os_disks(sms,service_name, deployment, role_name)
			roleName = role_name[:-1]
			i = 0 
			while(1):
				roleName = roleName+str(i)
				if(self.check_role_exists(sms,service_name,deployment,roleName)!= False):
					result 	  = sms.delete_deployment(service_name, deployment)
					self.wait_for_async(sms, result.request_id)
					result	  =	sms.delete_hosted_service(service_name)
				else:
					break
				roleName = role_name[:-1]
				i +=1
			self.logger.info("Wait to detach the hard disk")
			time.sleep(200)
			for disk_name in disk_names:
				self.logger.info("Deleting OS Disk: "+disk_name)
				sms.delete_disk(disk_name)
			#except:
			#self.logger.info("Error: Azure can not delete VM")
			#sys.exit(1)

	# Terminating Instance:
	#----------------------
	def terminateInstance(self,sms,vname):
		try:
			sms.shutdown_role(vname, vname, vname)
			time.sleep(10)
			self.logger.info("The instance has been terminated "+vname)
		except:
			self.logger.info("The instance is already down")

	# Detach and Delete Disks:
	#-------------------------
	def deleteDisks(self,sms,vname):
		disks = sms.list_disks()
		time.sleep(120)
		disk_name = None
		for diskName in disks:
			if(diskName.name.startswith(vname)):
				disk_name = diskName.name
				break
		try:
			self.logger.info("Deleting Disk: "+diskName.name)
			del_request = sms.delete_disk(disk_name)
			log.info(del_request)
			return disk_name
		except WindowsAzureError:
			self.logger.info("Error: This Disk may be currently in use by machine: "+ vname)
			self.logger.info("Trying to Delete the Deployment: "+ vname)
			time.sleep(20)
			deleteDisks(sms,vname)


	# Creat a new blob:
	#------------------
	def create_blob(self,blob_service, container_name, blob_name,storage_name):
		self.blob_service.create_container('disks', None, None, False)
		self.blob_service.put_blob("disks", blob_name+".vhd", '', 'BlockBlob')
		media_link = "http://"+storage_name+".blob.core.windows.net/disks/"+blob_name+".vhd"
		self.logger.info("Media Link: "+media_link)
		return media_link

	# List Nodes:
	#------------
	def list_my_nodes(self,sms,service_name,deployment_name):
		try:
			name = self.sms.get_deployment_by_name(service_name, deployment_name)
			return name.status
		except:
			return "Machine may be not found or down."

	# Start machine:
	#---------------
	def start_instance(self,sms,service_name,deployment_name,role_name):
		try:
			result = sms.start_role(service_name, deployment_name, role_name)
			self.wait_for_async(sms, result.request_id)
			self.logger.info("starting instance: "+role_name)
		except:
			self.logger.info("The instance may be not found, "+role_name)
	
	# Restart Machine:
	#-----------------
	def restart_instance(self, sms,vname):
		try:
			#self.sms.restart_role(vname,vname,vname)
			command = "azure vm restart "+vname
			output 	= self.run_cmd(command)
			print output
			self.logger.info("Restarting Instance: "+vname)
		except:
			self.logger.info("The instance may be not found"+vname)
	
	# Shutdown machine:
	#------------------
	def shutdown_instance(self,sms,cloudservice,vname):
		result = sms.delete_deployment(cloudservice,vname)
		wait_for_async(sms,result.request)
		
		#try:
			#command = "azure vm shutdown "+vname
			#output 	= self.run_cmd(command)
			#print output
			#self.logger.info("Shutdown instance: "+vname)
		#except:
		#	self.logger.info("The instance may be not found")
			
	# Generate a VM Name:
	#--------------------
	def get_vm_name(self):
		vmname = socket.gethostname()[:10]+str(int(random.random()*1000))
		return vmname
	
	# Get the Public IP of node:
	#---------------------------
	def get_myIP(self,hostname):
		IP		= ""
		command = "telnet "+hostname+" 80 > echo "
		print command
		mystr = self.run_cmd(command)
		ipfile=open('echo','r')
		for sttr in ipfile.readlines():
			IP = re.findall(r'([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})', sttr )
			print sttr
			print IP
		command = "rm echo"
		self.run_cmd(command)
		IP = ''.join(IP)
		print IP
		return {hostname:IP}
	
	# Run Commandline:
	#-----------------
	def run_cmd(self,command):
		PIPE = subprocess.PIPE
		p = subprocess.Popen(command,stdout=PIPE,stderr=PIPE,shell=True)
		p.wait()
		st=p.stderr.read()
		if (len(st)>0):
			return "ERR: " + st		
		return p.stdout.read()
	# Creating PBS EndPoints:
	# -------------------------
	def get_pbs_endpoints(self,num):
		pbs_endpoints = []
		port    = 15000
		port_out = 150+int(num)
		for i in range(2):
			tcp_end_point = ConfigurationSetInputEndpoint(name='tcppbs'+str(port_out), protocol='tcp', port=str(port_out), local_port= str(port), load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
			#udp_end_point = ConfigurationSetInputEndpoint(name='udppbs'+str(port_out), protocol='udp', port=str(port_out), local_port= str(port), load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
			port += 1
			port_out +=1
			pbs_endpoints.append(tcp_end_point)
			#pbs_endpoints.append(udp_end_point)
		return pbs_endpoints

	"""
	def get_pbs_endpoints(self):
		pbs_endpoints = []
		port 	= 15000
		for i in range(20):
			tcp_end_point = ConfigurationSetInputEndpoint(name='tcppbs'+str(port), protocol='tcp', port=str(port), local_port= str(port), load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
			udp_end_point = ConfigurationSetInputEndpoint(name='udppbs'+str(port), protocol='udp', port=str(port), local_port= str(port), load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
			port += 1
			pbs_endpoints.append(tcp_end_point)
			pbs_endpoints.append(udp_end_point)
		return pbs_endpoints
	"""
	# Get Virtual Machine Status
	#----------------------------
	def getVmStatus(self, vmname):
		status = ""
		command = "azure vm list"
		output	= self.run_cmd(command).split("\n")
		if "ERR:" in output:
			self.logger.info("ERROR: We try again ..... !!")
			output	= self.run_cmd(command).split("\n")
		i = 0 
		for line in output:
			words = line.split()
			for i in range(len(words)):
				if words[i] == vmname:
					status = words[i+1]
					break
		if status == "ReadyRole":
			status = "Running"
		elif status == "RoleStateUnknown":
			status = "Down"
		else:
			status = "Not Found"
		return status
	
	# list all VMs on azure:
	#-----------------------
	def listMachines(self):
		command = "azure vm list"
		output 	= self.run_cmd(command)
		return output
		
	# Save Nodes in cluster info file:
	#---------------------------------- 
	def saveNodes(self,nodes,master_node):
		mainPath = os.path.dirname(os.path.abspath(__file__))
		file=open(mainPath+'/clusters_info/'+master_node+'.txt','wa')
		for node in nodes:
			file.write(node+'\n')
		file.flush()
		file.close()
		
	# Delete Storage account:
	#------------------------
	def deleteStorageAccount(self,sms,name):
		if name in [storageAcc.service_name for storageAcc in sms.list_storage_accounts()]:
			try:
				result = sms.delete_storage_account(name)
				time.sleep(10)
				self.logger.info('Create Storage Account status: Deleted')
				return name
			except WindowsAzureConflictError:
				self.logger.info("Error: Storage Service name %s is not exist", name)
				sys.exit(1)
		else:
			self.logger.info("Storage Account %s is not exist", name)
			return name
		
	# list Storage account:
	#----------------------
	def list_storage_account(self):
		command = "azure storage account list "
		output = self.run_cmd(command)
		return output
	
	# get storage account key:
	#-------------------------
	def getStorageAccountKey(self,sms,strAccountName):
		try:
			storageKey = sms.get_storage_account_keys(strAccountName)
			return storageKey.storage_service_keys.primary
		except WindowsAzureConflictError:
			print "Error: Account may be not found."
			sys.exit(1)
	
	# create storage container:
	#--------------------------
	def create_containers(self,sms,storage_account,storage_key,container_name,permission):
		try:
			self.logger.info("Storage account: "+storage_account)
			self.logger.info("Container Name: "+container_name)
			self.logger.info("permission: "+str(permission))
			blob_service = BlobService(account_name=storage_account, account_key=storage_key)
			self.logger.info("creating Blob Service connection")
			blob_service.create_container(container_name, None, permission, False)
			self.logger.info("creating container: %s",container_name)
			return container_name
		except WindowsAzureConflictError:
			self.logger.info("Error: can not create storage container with name %s ", container_name)
			sys.exit(1)
			
	# Delete storage container:
	#--------------------------
	def delete_containers(self,container_name,storage_account,storage_key):
		try:
			self.logger.info("Container Name: "+container_name)
			self.logger.info("Storage account: "+storage_account)
			self.logger.info("Storage key: "+storage_key)
			blob_service = BlobService(account_name=storage_account, account_key=storage_key)
			self.logger.info("creating Blob Service connection")
			result = blob_service.delete_container(container_name,False)
			return result
		except WindowsAzureConflictError:
			self.logger.info("Error: Can not delete storage container: "+container_name)
			sys.exit(1)
	
	# List storage containers:
	#-------------------------
	def list_storage_container(self,storage_account,storage_key):
		try:
			container_names = []
			self.logger.info("Listing storage containers")
			blob_service = BlobService(account_name=storage_account, account_key=storage_key)
			result = blob_service.list_containers(None,None,10,None)
			for i in range(len(result.containers)):
				container_names.append(result.containers[i].name)
			return container_names
		except WindowsAzureConflictError:
			self.logger.info("Error: Can not delete storage container: "+container_name)
			sys.exit(1)
			
	# Upload Block Blob on Windows Azure:
	#------------------------------------
	def upload_block_blob(self, container_name, blob_name, file_path,storage_account, storage_key):
		result = self.list_storage_container(storage_account, storage_key)
		found = False
		blob_service	=  BlobService(account_name=storage_account, account_key=storage_key)
		for name in result:
			if name ==container_name:
				found = True
		if found:
			self.logger.info("container is already exist")
		else:
			blob_service.create_container(container_name, None, None, False)
		blob_service.put_blob(container_name, blob_name, '', 'BlockBlob')
		data_sent=0
		sent = 0
		block_ids = []
		index = 0
		with open(file_path, 'rb') as f:
			while True:
				data = f.read(self.chunk_size)
				if data:
					length = len(data)
					block_id = base64.b64encode(str(index))
					blob_service.put_block(container_name, blob_name, data, block_id)
					block_ids.append(block_id)
					index += 1
					data_sent += self.chunk_size
					sent = data_sent/(1024*1024)
					sys.stdout.write("\rUploaded data = %d MB"%sent)
					sys.stdout.flush()           
				else:
					print "\n"
					break
		blob_service.put_block_list(container_name, blob_name, block_ids)
	
	# Attach Data Disk to Virtual Machine:
	#-------------------------------------
	def attachDiskToInstance(self,diskSize,vmname,disklabel,storage_account,storage_key,container_name,blob_name):
		# Create Page Blob:
		blob_service	=  BlobService(account_name=storage_account, account_key=storage_key)
		#blob_service.put_blob(container_name, blob_name, b'', 'PageBlob', x_ms_blob_content_length='1073741824')
		#url		= "http://"+storage_account+".blob.core.windows.net/"+container_name+"/"+blob_name
		#medialink		= "http://"+storage_account+".blob.core.windows.net/"+container_name+"/nfsnew.vhd"
		url = "http://elastichpc.blob.core.windows.net/ahmed-sate931/ahmed.vhd"
		lun = 5
		label = 'diskdata' + str(lun)
		diskname = label
		sms = self.CheckSubscription('8be5609b-07c9-4114-8865-921ad82cb64a','/media/ahmed/92b488cc-077b-480a-8a6c-62e6fd95339b/elastichpc/ehpc_azure/keys/mycert.pem')
		os = "Linux"
		result = sms.add_disk(False, label, url, 'disk1ahmed', 'Linux')
		self.wait_for_async(sms,result.request_id)
		result = sms.list_disks()
		for i in range(len(result.__dict__['disks'])):
			print "\n("+str(i)+")\n"
			print result.__dict__['disks'][i].__dict__['name']
			print result.__dict__['disks'][i].__dict__['media_link']
			print result.__dict__['disks'][i].__dict__['os']
			print "------------------------------------------"
		#result = sms.add_data_disk(vmname, vmname, vmname, lun, "ReadWrite", None, label,None, "1", url)
		#self.wait_for_async(sms,result.request_id)
		#print url
		
	# Check Status if the role exists or not:
	#----------------------------------------
	def check_role_exists(self,sms, service_name, deployment_name,role_name):
		try:
			props = sms.get_role(service_name, deployment_name, role_name)
			return props is not None
		except:
			return False
			
	# Copy Image from Public VMDepot to Local Azure Container:
	#---------------------------------------------------------
	def copy_image_vmDepot(self,blobService,containerName, blobName,os_image_name,sms):
		#publicImage = "http://vmdepotwestus.blob.core.windows.net/linux-community-store/community-23970-fa0808af-dddf-4f5c-8e87-01c303e640b9-1.vhd"
		publicImage = "http://elastichpc.blob.core.windows.net/ahmed-sate434/image6-os-2014-05-20.vhd"
		try:
			output = blobService.copy_blob(containerName,blobName,publicImage)
			if (self.wait_async_copy(blobService,containerName,blobName)):
				return True
			return False
		except WindowsAzureConflictError:
			print "INFO -- There is a currently a pending copy operation"
			sys.exit(0)

	def wait_async_copy(self,bs,container_name, blob_name):
		count = i = 0
		toolbar_width = 50
		print "INFO -- Start Copying Public Image to your local storage"
		sys.stdout.write("INFO -- Submitting Request In Progress : [%s]" % (" " * toolbar_width))
		sys.stdout.flush()
		sys.stdout.write("\b" * (toolbar_width+1))
		props = bs.get_blob_properties(container_name, blob_name)
		while props['x-ms-copy-status'] != 'success':
			sys.stdout.write("-")
			sys.stdout.flush()
			count = count + 1
			i +=1
			if count > 50:
				print "Error: Time out "
				return False
			time.sleep(60)
			props = bs.get_blob_properties(container_name, blob_name)
		for i in xrange(toolbar_width-i):
				time.sleep(0.1) # do real work here
				# update the bar
				sys.stdout.write("-")
				sys.stdout.flush()
		sys.stdout.write("\n")
		return True


	# Make Image on your local storage:
	#----------------------------------
	def make_os_image(self,sms,LINUX_OS_VHD_URL,os_image_name):
		try:
			result = sms.add_os_image(os_image_name+'label', LINUX_OS_VHD_URL, os_image_name, 'Linux')	
			if (self.wait_async_image(sms,result.request_id)):
				return True
			return False
		except WindowsAzureConflictError:
			print "INFO -- Error: Can not create image"
			sys.exit(0)

	def wait_async_image(self,sms,request_id):
		count = 0
		toolbar_width = 40
		i = 0
		sys.stdout.write("INFO -- Submitting Request In Progress : [%s]" % (" " * toolbar_width))
		sys.stdout.flush()
		sys.stdout.write("\b" * (toolbar_width+1))
		result = sms.get_operation_status(request_id)
		while result.status == 'InProgress':
			count = count + 1
			i +=1
			sys.stdout.write("-")
			sys.stdout.flush()
			if count > 120:
				print 'INFO -- Timed out waiting for async operation to complete.'
				return False
			time.sleep(4)
			result = sms.get_operation_status(request_id)
		for i in xrange(toolbar_width-i):
				time.sleep(0.1) # do real work here
				# update the bar
				sys.stdout.write("-")
				sys.stdout.flush()
		sys.stdout.write("\n")
		return True
		
	# Run Command lines on Cluster nodes:
	def run(self,master_node,command,url=None,downloadFile=None, uploadFile=None):
		if url:
			if self.linkIsExist(url):
				self.logger.info("Link is exist: "+url)
				send.UploadFiles(domain,port,id,owner,url,"/home/ehpcuser/ehpcazure/Config/")
		if uploadFile:
			print uploadFile
		if downloadFile:
			print downloadFile
			
	# Check if the link is exist:
	#------------------------------		
	def linkIsExist(self, url):
		try:
			f = urllib2.urlopen(urllib2.Request(url))
			return True
		except:
			return False
		 
	# Download Block Blobs:
	#----------------------	
	def download_block_blob(self,account,key,container,blobname,path):
		if not os.path.exists(path):
			os.makedirs(path)
		blob_service	=  BlobService(account_name=account, account_key=key)
		blob 			=  blob_service.get_blob(container, blobname)
		try:
			with open(path+"/"+blobname, 'w') as f:
				f.write(blob)
			return True
		except:
			return False 
