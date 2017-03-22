#! /usr/bin/env python
import subprocess

def run(excuter,wait=True):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	if (wait):
		p.wait()
		st=p.stderr.read()
		if len(st)>0:
			f = open("/tmp/ToolServiceC.err",'a')
			f.write(st)
			f.flush()
			f.close()
			return "Error: "+ st
		else:
			return p.stdout.read()
	else:
		return p
def installPBSMainServer(name,MasterPrivateIP,mainMachinePBSCores):
	
	run("/root/EHPC/installTorque.py")
	run('hostname ' +name)
	run ("echo '127.0.0.1 localhost \n' > /etc/hosts")
	run ("echo '" + MasterPrivateIP   + " "  +  name + "\n' >> /etc/hosts")
	run ('echo ' + name + ' > /var/spool/torque/server_name')
	run ('echo ' + name + ' np=' + mainMachinePBSCores + ' >> /var/spool/torque/server_priv/nodes')
	run ('echo $pbsserver ' + name + ' > /var/spool/torque/mom_priv/config')  
	run ('pbs_server -t create')
	run ("qmgr -c 'set server operators = root@" +name +"'") 
	run ("qmgr -c 'set server operators += ubuntu@" + name + "'")
	run ("qmgr -c 'create queue batch'")
	run("qmgr -c 'set queue batch queue_type = Execution'")
	run("qmgr -c 'set queue batch started = True'")
	run("qmgr -c 'set queue batch enabled = True'")
	run("qmgr -c 'set server default_queue = batch'")
	run("qmgr -c 'set server resources_default.nodes = 1'")
	run("qmgr -c 'set server scheduling = True'")
	run("qmgr -c 'set server keep_completed = 86400'")
	run ("qmgr -c 'set server submit_hosts = " + name + "'")

installPBSMainServer("HPC-cloud-0","127.0.0.1","2")

