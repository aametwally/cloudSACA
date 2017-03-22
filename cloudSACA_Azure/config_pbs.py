import os
import sys
import sshazure

def install_pbs_master(hostname):
	script	= "cd ~/pbstorque;sudo ./torque-2.3.8/torque-package-clients-linux-x86_64.sh  --install; sudo ./torque-2.3.8/torque-package-devel-linux-x86_64.sh --install ;sudo ./torque-2.3.8/torque-package-doc-linux-x86_64.sh --install; sudo ./torque-2.3.8/torque-package-mom-linux-x86_64.sh --install; sudo ./torque-2.3.8/torque-package-server-linux-x86_64.sh --install"
	sshazure.run_over_ssh(hostname,script)
def install_pbs_slave(hostname):
	script	= "cd ~/pbstorque;sudo ./torque-2.3.8/torque-package-clients-linux-x86_64.sh  --install; sudo ./torque-2.3.8/torque-package-devel-linux-x86_64.sh --install ;sudo ./torque-2.3.8/torque-package-doc-linux-x86_64.sh --install; sudo ./torque-2.3.8/torque-package-mom-linux-x86_64.sh --install"
	sshazure.run_over_ssh(hostname,script)
