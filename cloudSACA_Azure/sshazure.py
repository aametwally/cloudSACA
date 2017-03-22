import os
import sys
import subprocess
"""
Run Command lines in the master nodes and other nodes.

"""
def run_over_ssh(HostName,script):
	cmd_string  = 'bash sshShell.sh '+HostName+" "+script
	try:
		p = subprocess.check_output(cmd_string, shell=True)
		return p
	except subprocess.CalledProcessError as e:
		return e

def scp(cmd_string):
	try:
		p = subprocess.check_output(cmd_string,shell=True)
		print p
	except subprocess.CalledProcessError as e:
		print e
#myout =run_over_ssh('ahmed-Sate516-0.cloudapp.net','"cd ElasticHPCAzure;python azure-client.py --create --number=4 --size=Small --region=WUS"')
#print myout
#inet addr:10.76.98.89  Bcast:10.76.99.255  Mask:255.255.254.0
#ifconfig eth0 | grep "inet addr:"
#scp("scp -i keys/mycert.pem ehpcuser@ahmed-Sate765-0.cloudapp.net:/home/ehpcuser/ElasticHPCAzure/nodes.txt clusters/ahmed-Sate346-0.txt")
