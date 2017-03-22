import os
import sys
import subprocess
import send
#ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa

# Generate new RSA key:
def generateRSAkey(domain,port,id,owner):
	print "INFO -- Domain: "+ domain
	command = "ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa"
	print "INFO -- Command: "+command
	output  = send.sendNewJob(domain,port,id,owner, 'ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa',None,None)
	command = "echo `cat ~/.ssh/id_rsa.pub`"
	print "INFO -- Command: "+command
	valueRSApublickey = send.sendNewJob(domain,port,id,owner, command,None,None).split("\n")[0]
	print valueRSApublickey
	return valueRSApublickey

# Add Keys to .ssh/authorized keys:
def authorized_keys(domain,port,id,owner,authorized):
	print 'INFO -- Domain: '+domain
	command = 'chmod 666 ~/.ssh/authorized_keys;echo "'+authorized+'" >> ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys'
	print "INFO -- Command: "+command
	#raise Exception("stop here")
	send.sendNewJob(domain,port,id,owner, command,None,None)
	
# Get Host's ip
def getHostIp(hostname,domain,port,id,owner):
	print "INFO -- Domain: "+domain
	print "INFO -- Port: "+str(port)
	mainpath = "/home/ehpcuser/ehpcazure"
	node_ip         = send.sendNewJob(domain,port,id,owner,"python "+mainpath+"/get_node_ip.py",None,None).split("\n")[0]
	print "INFO -- Node IP: "+node_ip
	return node_ip

# Add Hosts:
def addHosts(hosts,domain,port,id,owner):
	command		='sudo sh -c "echo '+hosts+' >> /etc/hosts"'
	print "INFO -- Command: "+command 
	configureKnownHosts = 'alias ssh="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l ehpcuser"'
	send.sendNewJob(domain,port,id,owner, command,None,None)
#mount NFS
def mountnfs(host,domain,port,id,owner):
	command = 'sudo apt-get install nfs-kernel-server; sudo sh -c "mount '+domain+':/dev/sdb1 /mount/nfs"'
	print "INFO -- Command: "+command
	send.sendNewJob(domain,port,id,owner, command,None,None)
	
def knownHosts(domain, port,id,owner):
	KnownHosts 	= '"ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l ehpcuser"'
	#sshHosts	= '"alias ssh='+KnownHosts+'"'
	#command	= "sudo sh -c 'echo "+sshHosts+">>/home/ehpcuser/.profile'"
	#send.sendNewJob(domain,port,id,owner, command,None,None)
	#print command
	command	= "sh -c 'alias ssh="+KnownHosts+"'"
	send.sendNewJob(domain,port,id,owner, command,None,None)
	print command
# Configure SSH
def setupKeylessSSH(nodes):
	i = 0
	authorized = []
	port 	= "5000"
	num  	= 1
	domain 	= nodes[0]+".cloudapp.net" 
	# Step 1: Generate RSA keys
	for node in nodes:
		id = str(i)
		print "INFO -- Domain:"+domain
		print "INFO -- Port number:"+port
		publicKey = generateRSAkey(domain,int(port),id,"ehpcuser") 
		authorized.append(publicKey)
		port = port[:-1]+str(num)
		num +=1
		i +=1
	# Step 2: Add keys to .ss/authorized_keys:
	port 	= "5000"
	num  	= 1
	domain 	= nodes[0]+".cloudapp.net" 
	for node in nodes:
		for author in authorized:
			authorized_keys(domain,int(port),"1","ehpcuser",author)
		port = port[:-1]+str(num)
		num +=1
	# Step 3: configure /etc/hosts
	port 	= "5000"
	num  	= 1
	domain 	= nodes[0]+".cloudapp.net"
	hosts 	= []
	hosts.append("127.0.0.1 localhost")
	for node in nodes:
		ip = getHostIp(node,domain,int(port),str(num),"ehpcuser")
		hosts.append(ip + " "+node)
		port = port[:-1]+str(num)
		num +=1
	# Configure NFS:
	port 	= "5000"
	id 		= "5000"
	command = 'sudo apt-get install nfs-kernel-server; sudo sh -c "chmod 666 /etc/exports"; sudo sh -c " echo "/mount/nfs' 
	nfsHosts = hosts[2:]
	for host in nfsHosts:
		command = command + host+"(ro,sync,no_root_squash)"
	command = command + ' "> /etc/exports ";sudo sh -c "chmod 644 /etc/exports"'
	print command
	send.sendNewJob(domain,int(port),id,owner, command,None,None)
	command = 'sudo sh -c "mount /dev/sdb1 /mount/nfs"; sudo /etc/init.d/nfs-kernel-server start'
	print command
	send.sendNewJob(domain,port,id,owner, command,None,None)
	# Mount NFS:
	port 	= "5001"
	for host in nfsHosts:
		mountnfs(host,domain,int(port),str(num),'ehpcuser')
		port = str(int(port)+1)
	# Step 4: add hosts to /etc/hosts
	num  	= 1
	port 	= "5000"
	for node in nodes:
		for host in hosts:
			addHosts(host,domain,int(port),str(num),"ehpcuser")
		port = port[:-1]+str(num)
		num +=1
	# Step 5: disable known host confirmation
	num  	= 1
	port 	= "5000"
	for node in nodes:
		knownHosts(domain,int(port),str(num),"ehpcuser")
		port = port[:-1]+str(num)
		num +=1
	print "INFO -- Nodes have been configured"
# Main : 
#-------
master_node = sys.argv[1]+".cloudapp.net"
nodes = []
main_path = os.path.dirname(os.path.abspath(__file__))
file2 = open(main_path+"/clusters_info/"+master_node+".txt",'r')
for node in file2.readlines():
	nodes.append(node.split("\n")[0])

setupKeylessSSH(nodes)
