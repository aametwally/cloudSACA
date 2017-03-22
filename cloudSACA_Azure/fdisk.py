import shlex, subprocess
command= 'echo -e "n\np\n1\n\n\na\n1\np\nq\n" | sudo fdisk /dev/sdb1'
#args = shlex.split(command_line)


def run(command):
	PIPE = subprocess.PIPE
	p = subprocess.Popen(command,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	if (len(st)>0):
		return "ERR: " + st		
	return p.stdout.read()

print run(command)
