#!/usr/bin/python
import socket,base64,time,sys,subprocess
from Crypto.PublicKey import RSA
import ClientJob as Job,Connect
import os
from Crypto import Random
forceFiles=False
EOM="\n\n###"
port=5000
domain=""

def submitJob(domain,port,id,owner,command,inputs=None,outputs=None,retry=True):
	#print "sending new job to..",domain
	"""global forceFiles
	if not forceFiles:
		if inputs!=None:
			for file in inputs:
				if not file in command:
					print "Input file '" + file + "' doesn't exists in the command, please revise your command, files list or use --force-files"
					exit(-10)
		if outputs !=None:
			for file in outputs:
				if not file in command:
					print "Output file '" + file + "' doesn't exists in the command, please revise your command, files list or use --force-files"
					exit(-10)
	"""
	msg=Job.createJobMessage(id,owner,command.strip(),inputs,outputs,retry)
	#raise Exception(msg)
	
	Connect.domain	= domain
	Connect.port	= port
	#print Connect.domain
	#print msg
	res = Connect.Send(msg,retry) 
	return res

def checkStatus(domain,port,id,owner,waitTillCompleted=True,retry=True):
	#print "retry in CheckStatus:",retry
			
	Connect.domain= domain
	Connect.port  = port
	status = Job.checkJobStatus(Connect,id,owner,waitTillCompleted,retry)
	return status

def FetchOutput(domain,port,id,owner,outputs=None,retry=True):
	#print "in FetchOutput"
	#print "retry in fetchOutput:",retry
			
	if outputs!=None:
		status=checkStatus(domain,port,id,owner,False,retry)
	#	print status
		if status=='C' or status=='E':
				#print "Preparing to fetch files in 15 seconds"
				#import time
				#time.sleep(15)
				
				outlist=FetchFiles(domain,port,outputs,retry=retry)
				print len(outlist) , "file(s) returned"
				if len(outlist)==len(outputs):
					return outlist
				else:
					return "Error Occured while fetching output, please check job logs on the server"
		else:
			print "Job doesn't finish yet to get outFiles"
	else:
		return None

def run(excuter):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	
	if (len(st)>0):
		return "ERR: " + st		
	return p.stdout.read()

def sendNewJob(domain, port,id,owner,command,inputs=None,outputs=None,retry=True):
	res=submitJob(domain, port,id,owner,command,inputs,outputs,retry)
	print res
	id=""
	if ('recieved' in res):
		id=res.split(":")[1]
	else:
		return res
	#status= Job.checkJobStatus(Connect,id,owner,True,retry)
	#if status=='C' or status=='E':
	#	print "Job Finished, collecting output in 15 seconds"
	#	import time
	#	time.sleep(15)
	#	if outputs!=None:
	#		outlist=FetchOutput(domain,id,owner,outputs,retry)
	#		if outlist!=None:
			#	print outlist
	#			return outlist
	#else:
	#	return res
sshKeyPath=""
def UploadFiles(domain,port,id,owner,files,outputDir):
	Connect.domain = domain
	print Connect.domain
	Connect.port   = port
	print Connect.port
	retry = True
	for file in files:
		print file
		uploadData 	= Job.UploadFileData(id,owner,file,outputDir)
		res 	 	= Connect.Send(uploadData,retry)
	return res
		
def FetchFiles(domain,port,files,mode='auto',destination="",retry=True):
	#print domain,files,mode,destination,sshKeyPath
	Connect.domain=domain
	Connect.port  =port
	if (files !=None):
		#print "in files"
		if mode=="auto":
			if sshKeyPath!="":
				mode="scp"
				
			else:
				mode="xml"
				
			print 'Decided to use ' + mode + " to transfer files"
		if mode=='xml':
			msg=Job.FetchOutput(id,owner,files)
	#		print msg
			#print "retry in fetchFiles:",retry
			res = Connect.Send(msg,retry)
			outlist=Job.parseResponse(res)
	#		print outlist
			return outlist
		elif mode=='scp':
			print "in scp"	
			print files
			for file in files:
				dest=""
				if destination=="":
					dest=os.path.dirname(os.path.abspath(file))
				else:
					dest=destination
				if not os.path.exists(dest):
					os.makedirs(dest)
				cmd='scp -r -i %s ubuntu@%s:%s %s'%(sshKeyPath,domain,file,dest)
				#print 'cmd:' ,cmd	
				#exit()
				scp(cmd)
			return files
	else:
		return "No Files"
#domain 	= "portalvhds316-0.cloudapp.net"
#id 		= "1"
#owner 	= "ubuntu"
#command = "ls -la"
#command = "python /home/ahmed/Desktop/ehpcazure/ehpc-client.py --create --number=1 --size=Small --region=WUS --MasterNode=ubuntu"
#inputs 	= None
#outputs = None 
#result 	= sendNewJob(domain,id,owner,command,inputs,outputs)
#print result

#domain 	= "ahmed-sate125-0.cloudapp.net"
#id 		= "1"
#owner 	= "ubuntu"
#command = "cd /home/ehpcuser/ehpcazure/logs; mydir=$(ls -Art |tail -n 1 >&1);more $mydir"
#inputs 	= None
#outputs = None 
#result 	= sendNewJob(domain,id,owner,command,inputs,outputs)
#print result
