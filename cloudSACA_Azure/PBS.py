#!/usr/bin/env python
import socket,threading,subprocess,os,base64,xml.dom.minidom
def AddFileWrapper(protocol,executer,inputList,outList):
	indir=""
	outDir=""
	st1=""
	st2=""
	print "inList: ",inputList
	print "outList: ",outList
	if inputList!=[]:
		indir="mkdir -p"
		st1='/root/EHPC/inputGrabber.py ' + protocol + ' input "' 
		for inp in inputList:
			if (inp != inputList[-1]):
				st1 += inp + ','
				indir+=" " +  '/'.join(inp.split('/')[:-1])	
			else:
				st1 +=inp +'"\n'
				indir+= " " +'/'.join(inp.split('/')[:-1])+"\n"
	#st += st1
	#st += executer+'\n'
	if outList!=[]:
		outDir= "mkdir -p"
		st2 = '/root/EHPC/inputGrabber.py ' + protocol + ' output "' 
		for out in outList:
			if (out != outList[-1]):
				st2+= out +','
				outDir += " " + '/'.join(out.split('/')[:-1])
			else:
				st2+= out +'"\n'
				outDir+=" " + '/'.join(out.split('/')[:-1])+"\n"
#	st+=st2
	return indir+outDir+st1+executer+'\n'+st2
	
def runAsPBSJob(owner,id,executer):
	file=open('/tmp/'+owner+'.'+id,'w')
	file.write("#! /bin/bash\nexport PBS_O_HOME=/tmp\nexport PBS_O_WORKDIR=/tmp\numask 022\n")
	file.write(executer)
	file.flush()
	file.close()
	order ="su ubuntu -c 'qsub -N " + owner+'.'+id + " /tmp/"+owner+'.'+id+"'"
	print "order: " + order
	pbsID =run(order).strip()
	print "PBSID: "+pbsID
	return pbsID

def run(excuter,id=None):
	try:
		p = subprocess.check_output(executer, shell=True)
		return p
	except subprocess.CalledProcessError as e:
		return e
        """PIPE=subprocess.PIPE
        p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	(stdout,stderr)=   p.communicate()
        st=stderr
	if (id!=None):
		 f = open("/tmp/"+id+".err",'w')
                 f.write(st)
                 f.flush()
                 f.close()
   		 f = open("/tmp/"+id+".out",'w')
                 f.write(stdout)
                 f.flush()
                 f.close()
	
	if len(stderr)>0:
		return  stderr
	return stdout
	"""
