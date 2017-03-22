import sys, unittest, paramiko, os, time
from base64 import b64encode
from M2Crypto import RSA
from getpass import getpass
import logging
from azure.storage.blobservice import BlobService
from azure.storage.storageclient import _StorageClient
from azure.servicemanagement import LinuxConfigurationSet
from azure.servicemanagement import OSVirtualHardDisk
from azure import WindowsAzureConflictError
from azure import WindowsAzureError
from azure import *
from azure.servicemanagement import * 
from azure.servicemanagement.servicemanagementservice import ServiceManagementService
import base64
import urllib2

blob_service = BlobService(account_name='sjsssiohdsiu', account_key='tBlxaMOoA+bE6zMsT5i1epphb25V/sD62MpAO7UA0fRK0GbpYLiVwhpe+WIwCtg80XxC+1uaVDtOvopZRHQ3Nw==')
#myblob = open(r'vmdepoteastus.blob.core.windows.net/linux-community-store/community-23970-525c8c75-8901-4870-a937-7277414a6eaa-1.vhd', 'r').read()
#blob_service.put_blob('mycontainerazure','ahmedblob.vhd',myblob,x_ms_blob_type='BlockBlob')
#cert_data_path = "/media/ahmed/1578-4B0E/WindowsAzure/AzureCert/mycert.pfx"
#with open(cert_data_path, "rb") as bfile:
#    cert_data = base64.b64encode(bfile.read())
    
log = logging.getLogger()
log.setLevel(logging.DEBUG)
ch = logging.StreamHandler( sys.__stdout__ ) # Add this
ch.setLevel(logging.DEBUG)
# create formatter
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
# add formatter to ch
ch.setFormatter(formatter)
# add ch to logger
log.addHandler(ch)

subscription_id	 = '8be5609b-07c9-4114-8865-921ad82cb64a'
certificate_path = '/home/ahmed/Desktop/AzureCert/mycert.pem'
pkfile = "/home/ahmed/Desktop/AzureCert/mycert.cer"
thumbprint = "B633855BB1B273C48F3FCFB9F38D4E8F0CA7A3BB"
chunk_size = 4 * 1024 * 1024
pageSize	= 512



# Connect To Azure Platform:
#---------------------------
def CheckSubscription(sub_id,cert_path):
	log.info("Connect to the Azure platform and check that the location is valid")
	sms = ServiceManagementService(subscription_id, certificate_path)
	return sms
# Create Storage account:
#------------------------
def StorageCreate(sms,name,label,desc,location,affGrp):
	if name not  in [storageAcc.service_name for storageAcc in sms.list_storage_accounts()]:
		try:
			result = sms.create_storage_account(name, desc, label,affinity_group=affGrp)
			time.sleep(5)
			operation_result = sms.get_operation_status(result.request_id)
			log.info('Create Storage Account status: %s', operation_result.status)
			return name
		except WindowsAzureConflictError:
			log.info("Error: Storage Service name is already found")
			sys.exit(1)
	else:
		storageProp = sms.get_storage_account_properties(name)
		log.info("Storage Account %s is already exist", name)
		return name
# Create Account Service:
#------------------------
def newCloudService(sms,name,label,desc,loc,affGrp):
	if name not in [service.service_name for service in sms.list_hosted_services()]:
		try:
			result = sms.create_hosted_service(name, label, desc,affinity_group=affGrp)
			time.sleep(60)
			#while(result.request_id != None):
			#	break
			#operation_result = sms.get_operation_status(result.request_id)
			#log.info('Create Cloud Service status: %s', operation_result.status)
			serviceProb = sms.get_hosted_service_properties(name)
			log.info("Cloud Sevice URL: %s",serviceProb.url)
			return serviceProb.url
		except WindowsAzureConflictError:
			log.info("Error: Can not create Cloud Service")
			sys.exit(1)
	else:
		log.info("Cloud Service %s is already exist", name)
		serviceProb = sms.get_hosted_service_properties(name)
		return serviceProb.url
# Create Affinity Group:
#-----------------------
def createAffinityGrp(sms,name,label,desc,location):
	if name not in [affGrp.name for affGrp in sms.list_affinity_groups()]:
		log.info("Creating Affinity Group %s", name)
		result = sms.create_affinity_group(name, label, location, desc)
		return name
	else:
		log.info("Affininty Group %s is already exist", name)
		return name
# Create Instances:
#------------------
def createInstances(sms,name, region, imageID, instanceSize,pkfile,count):
	hostname		= name
	username		= "ehcpuser"
	password		= None 
	affGrp			= affinity_group = createAffinityGrp(sms,name,name,name,region)
	storage_name	= StorageCreate(sms,name,name,name,region,affGrp)
	media_link		= "http://"+storage_name+".blob.core.windows.net/disks/"+name+".vhd"
	log.info("Configuring Media Link %s",media_link)
	Service_url 	= newCloudService(sms,name,name,name,region,affGrp)
	Service_name	= name
	
	# Configuring EndPoint "Firwall Configuration":
	endpoint_config = ConfigurationSet()
	endpoint_config.configuration_set_type = 'NetworkConfiguration'
	endpoint1 = ConfigurationSetInputEndpoint(name='HTTP', protocol='tcp', port='80', local_port='80', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
	endpoint2 = ConfigurationSetInputEndpoint(name='SSH', protocol='tcp', port='22', local_port='22', load_balanced_endpoint_set_name=None, enable_direct_server_return=False)
	endpoint_config.input_endpoints.input_endpoints.append(endpoint1)
	endpoint_config.input_endpoints.input_endpoints.append(endpoint2)
	log.info("Configuring EndPoints 'Firwall Configuration' port 22 and 80")
	
	# Linux VM Configuration:
	linux_config = LinuxConfigurationSet(hostname, username, password , True)
	publickey = PublicKey(thumbprint, pkfile)
	linux_config.ssh.public_keys.public_keys.append(publickey)
	log.info("Linux VM Configuration")
	
	# Configuring Image ID:
	#----------------------
	os_hd 	   = OSVirtualHardDisk(imageID, media_link)
	log.info("Configuring The Virtual Hard Disk using Image ID: %s", imageID)
	
	# Start Deployment of VM on Azure:
	log.info("Start Deployment of Elastic hpc on Azure")
	
	# Configuring Certificates:
	cert_format = 'pfx'
	cert_password = 'eng_1989'
	
	try:
		for num in range(count):
			log.info("Deploying Node number: %d",num)
			name = name+"-"+str(num)
			result_cert = sms.add_service_certificate(service_name=Service_name,
                            data=cert_data,
                            certificate_format=cert_format,
                            password=cert_password)
			log.info("Start Deploying VM with Name: "+name)
			try:
				log.info(vars(result_cert))
			except:
				log.info(result_cert)
			result = sms.create_virtual_machine_deployment(service_name=Service_name,
				deployment_name=name,
				deployment_slot='production',
				label=name,
				role_name=name,
				system_config=linux_config,
				os_virtual_hard_disk=os_hd,
				network_config=endpoint_config,
				role_size=instanceSize)
			operation_result = sms.get_operation_status(result.request_id)
			log.info("Start Deployment .............")
			for i in range(100):
				time.sleep(2)
				sys.stdout.write("Deployment Status Progress \r%d .%%" %i) 
				sys.stdout.flush()
			print "\n"
			log.info("ssh -i "+certificate_path+" "+username+"@"+Service_name+".cloudapp.net")
	except WindowsAzureConflictError:
		log.info("Error: Can not Create VM")
		sys.exit(1)


# Upload large blob:
#-------------------
"""
def upload(blob_service, container_name, blob_name, file_path):

	blob_service.create_container(container_name, None, None, False)
	blob_service.put_blob(container_name, blob_name, '', 'BlockBlob')
	file_name = file_path.split('/')[-1]
	block_ids = []
	index = 0
	#url = urllib2.urlopen(file_path)
	data_sent=0
	sent = 0
	with open(file_path, 'rb') as f:
		while True:
			data = f.read(chunk_size)
			if index < 999:
				if data:
					length = len(data)
					block_id = base64.b64encode(str(index))
					blob_service.put_block(container_name, blob_name, data, block_id)
					block_ids.append(block_id)
					index += 1 
					data_sent += chunk_size
					sent = data_sent/(1024*1024)
					print "Uploaded data = "+str(sent)+"MB"
				else:
					break
			else:
				break
	index = 1000
	with open(file_path, 'rb') as f:
		while True:
			data = f.read(chunk_size)
			if data:
				length = len(data)
				block_id = base64.b64encode(str(index))
				blob_service.put_block(container_name, blob_name, data, block_id)
				block_ids.append(block_id)
				index += 1 
				data_sent += chunk_size
				sent = data_sent/(1024*1024)
				print "Uploaded data = "+str(sent)+"MB"
			else:
				break
	blob_service.put_block_list(container_name, blob_name, block_ids)
"""
def upload_block_blob(blob_service, container_name, blob_name, file_path):
    blob_service.create_container(container_name, None, None, False)
    blob_service.put_blob(container_name, blob_name, '', 'BlockBlob')
    data_sent=0
    sent = 0
    block_ids = []
    index = 0
    with open(file_path, 'rb') as f:
        while True:
            data = f.read(chunk_size)
            if data:
                length = len(data)
                block_id = base64.b64encode(str(index))
                blob_service.put_blob(container_name, blob_name, data, block_id)
                block_ids.append(block_id)
                index += 1
                data_sent += chunk_size
                sent = data_sent/(1024*1024)
                sys.stdout.write("\rUploaded data = %d MB"%sent)
                sys.stdout.flush()           
            else:
                break

    blob_service.put_block_list(container_name, blob_name, block_ids)

# upload page blob :
# ------------------
def upload_page_blob(blob_service, container_name, blob_name, file_path):
    blob_service.create_container(container_name)
    blob_service.put_blob(container_name, blob_name,'', "PageBlob")
    data_sent=0
    sent = 0
    index = 0
    with open(file_path, 'rb') as f:
        while True:
            data = f.read(pageSize)
            if data:
                length 		= len(data)
                x_range  	= 'bytes={}-{}'.format(index, index + pageSize - 1)
                blob_service.put_page(container_name, blob_name, data, x_ms_range = x_range, x_ms_page_write = 'clear')
                data_sent += pageSize
                sent = data_sent/(1024*1024)
                sys.stdout.write("\rUploaded data = %d MB"%sent)
                sys.stdout.flush()           
            else:
                break

    blob_service.put_block_list(container_name, blob_name, block_ids)

# Download large blob:
#---------------------

def download(blob_service, container_name, blob_name, file_path):
    props = blob_service.get_blob_properties(container_name, blob_name)
    blob_size = int(props['content-length'])

    index = 0
    with urllib2.urlopen(file_path, 'wb') as f:
        while index < blob_size:
            chunk_range = 'bytes={}-{}'.format(index, index + chunk_size - 1)
            data = blob_service.get_blob(container_name, blob_name, x_ms_range=chunk_range)
            length = len(data)
            index += length
            if length > 0:
                f.write(data)
                if length < chunk_size:
                    break
            else:
                break

container_name 	= "ahmedpblob"
blob_name		= "myfile"
file_path  = "/home/ahmed/Desktop/getIp.py"
upload_page_blob(blob_service,container_name,blob_name,file_path)
print "\n Data Uploaded Successfully !!!"
#sms = CheckSubscription(subscription_id,certificate_path)
#sms.copy_blob('mycontainerazure','ahmedblob.vhd','http://vmdepoteastus.blob.core.windows.net/linux-community-store/community-23970-525c8c75-8901-4870-a937-7277414a6eaa-1.vhd')
#createInstances(sms,"elasticahmed20","West US","b39f27a8b8c64d52b05eac6a62ebad85__Ubuntu-12_04_2-LTS-amd64-server-20130325-en-us-30GB","Small",pkfile,1)
