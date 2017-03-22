#! /usr/bin/env python
import subprocess
def run(excuter):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	if len(st)>0:
		f = open("/tmp/ToolServiceT.err",'w')
		f.write(st)
		f.flush()
		f.close()
		print "Childerr:", st
		
	else:
		print p.stdout.read()

run ("/root/EHPC/torque/torque-package-server-linux-x86_64.sh --install")
run("/root/EHPC/torque/torque-package-clients-linux-x86_64.sh --install")
run("/root/EHPC/torque/torque-package-devel-linux-x86_64.sh --install")
run("/root/EHPC/torque/torque-package-doc-linux-x86_64.sh --install")
run("/root/EHPC/torque/torque-package-mom-linux-x86_64.sh --install")
