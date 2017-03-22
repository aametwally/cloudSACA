README:
-------

Pre-Installed packages:
------------------------

	$ sudo apt-get update ; sudo apt-get install python-dev g++ gpp kcc libssl-dev libxml2-dev libtool openssh-server python-m2crypto python-paramiko make cmake htop rcconf dialog

Configure elastic hpc Azure client:
-----------------------------------

	1- Create the certificates that will make you login to cluster nodes: 
		
		- Creating two certificates one of them is for server (.cer) and another is for client (.pem) using openssl:
		
				$ openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
				$ openssl x509 -inform pem -in mycert.pem -outform der -out mycert.cer
		
		- Create the .pfx file using the following command:
		
				$ openssl pkcs12 -export -out mycert.pfx -in mycert.pem -name "My Certificate"
		
		- The password of pfx certificate is needed in the azure.conf file.
		
		- Copy theses three files ".cer, .pem and .pfx" to $ROOT_EHPC/keys/

	2- Get the Subscription ID of your account and Thumbprint or "Fingerprint" from Windows Azure portal  "Settings -> management certification".

	3- Configure the $ROOT_EHPC/Config/azure.conf as following:

				subscription_id = '<YOUR SUBSCRIPTION ID>'
				certificate_path= '<.PEM CERTICATE>'
				pkfile 			= "<.CER CERTIFICATE>"
				thumbprint 		= "<THUMPRINT>"
				cert_data_path 	= "<.PFX CERTIFICATE>"
				storage_account = "<ACCOUNT STORAGE NAME>"
				imageID			= "vmdepot-33074-1-1"
				certPasswd		= "<.PFX CERTITFICATE PASSWORD>"
				storage_key		= "<LEAVE IT EMPTY>"

	4- To start a cluster:
		$ export ROOT_EHPC=<THE PATH OF CLIENT DIRECTORY>
		$ cd $ROOT_EHPC
		$ python ehpc-client.py --create --number=2 --size=Small --region=WUS
	
	5- For help:
		$ python ehpc-client.py -h
