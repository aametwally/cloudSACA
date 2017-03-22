import os
import sys
import subprocess
import azurehpc
from azurehpc import ehpcazure
import paramiko
from posixpath import basename, dirname

class KeyManagement:
	"""
	This class generate the keypairs and create the certification
	to communicate with windows azure cluster.
	"""
	def __init__(self):
		
		azureclient		= azurehpc.ehpcazure()
		self.log		= azureclient.logger
		self.KEYS_PATH	= os.path.expanduser(os.path.join("~", ".ssh",".azure","myPrivate.key"))
		self.azure_dir	= os.path.expanduser(os.path.join("~", ".ssh",".azure"))
	
	# Check and generate azure keys directory:
	#-----------------------------------------
	def check_azure_dir(self):
		if(os.path.exists(self.azure_dir)):
			return True
		else:
			try:
				cmd = "mkdir "+self.azure_dir
				p = subprocess.check_output(cmd, shell=True)
				return True
			except:
				return False
	
	# Generate Private Key:
	#----------------------
	def gen_keys(self): 
		try:
			if(self.check_azure_dir()):
				self.log.info("generating private key............ OK")
				cmd		= "ssh-keygen -b 4096 -t rsa -P '' -f "+self.KEYS_PATH
				proc	= subprocess.check_output(cmd, shell=True)
				key_path= dirname(self.KEYS_PATH)
				self.log.info("key generated successfully in that path:"+key_path)
				return self.KEYS_PATH
			else:
				self.log.info("Can not generate keys")
				exit(1)
		except:
			self.log.info("Can not generate keys")
	
	# Get Certification from Private key:
	#------------------------------------
	def get_cert(self, pkey_path):
		cert_path = dirname(pkey_path)+"/myCert.pem"
		cmd ='(echo; echo; echo;echo; echo; echo;)|openssl req -x509 -nodes -days 365 -new -key '+pkey_path+' -out '+cert_path
		self.log.info(cmd)
		cmd = cmd.split()
		cmd = tuple(cmd)
		#try:
		p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		#p = subprocess.check_output(cmd, shell=True)
		#p.communicate(input='')[0]
		#p.wait()
		self.log.info("Generating Certification myCert.pem")
		return cert_path
		#except:
			#self.log.info("Can not generate the certification")

key  = KeyManagement()
path = key.gen_keys()
key.get_cert(path)
