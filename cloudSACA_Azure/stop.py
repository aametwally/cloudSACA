#! /usr/bin/env python
import subprocess,sys
def run(excuter):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	return p.stdout.read()
print "Stopping EHPC Server"
p=run("ps aufx | grep EHPC-Server")
p=p.split("\n")
for line in p:
	if line=="": break
	parts=line.split()
#	print parts
	if "EHPC/EHPC-Server" in parts[-1]:
		run("kill -9 " + parts[1])
		print "Server Stopped Successfully"
		sys.exit(0)
print "Can't find the Server Process. Are you sure that the server is running?"	
