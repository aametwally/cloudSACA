#!/usr/bin/python
import sys
import os
import subprocess
import azurehpc

# run command line on terminal:
#------------------------------
def runCommand(command):
	PIPE = subprocess.PIPE
	p = subprocess.Popen(command,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	if (len(st)>0):
		return "ERR: " + st		
	return p.stdout.read()

# install Azure commandline interface:
#-------------------------------------
def installAzurecli():
	command	= "cd "+mainPath+"/packages/ ;tar xzvf node-latest.tar.gz && cd node-v*; ./configure ; make"
	log.info("configure nodejs package ......... OK")
	output 	= runCommand(command)
	command = "sudo make install"
	log.info("install nodejs package ........... OK")
	output 	= runCommand(command)
	command = "sudo rm node-v0.10.24/"
	log.info("cleaning ........ OK")
	output 	= runCommand(command)
	log.info("Azure command line Configuration and installation .......... OK")
	command = "mkdir azure ; mv windowsazure-cli.0.7.3.tar.gz azure/ ; cd azure; tar xvzf windowsazure-cli.0.7.3.tar.gz; ./configure; sudo make install"
	runCommand(command)
	
	
# install Prerequisites packages:
#--------------------------------
def installPrerequisites():
	command = "sudo apt-get update; sudo apt-get install g++ gpp kcc libssl-dev libxml2-dev libtool openssh-server python-m2crypto"
	output = runCommand(command).split("\n")
	for line in output:
		log.info(line)
	
if __name__ = "__main__":
	azureclient		= azurehpc.ehpcazure()
	log = azureclient.logger
	mainPath		=	os.path.dirname(os.path.abspath(__file__))
	installPrerequisites()
	installAzurecli()
