import sys,os,simplejson,subprocess
mainPath=os.path.dirname(os.path.abspath( __file__ ))+'/'


def run(excuter,Wait=True):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	if Wait:
		p.wait()
		st=p.stderr.read()
		
		if (len(st)>0):
			return "ERR: " + st		
		return p.stdout.read()
	else:
		return p
		
def getAllZones(pkfile,certfile,region):
	
	cmd=mainPath+"/EC2 %s %s %s --zones"%(pkfile,certfile,region)
	#print cmd
	zones =run(cmd,True).split("\n")
	#print zones
	z = []
	for zone in zones:
		if zone!="":
			z.append(zone.split('\t')[1].strip().replace('"',''))
	return simplejson.dumps(z)

def avaVolumesZone(pkfile,certfile,region,zone):
	cmd=mainPath+"EC2 %s %s %s --ava-volumes-zone %s"%(pkfile,certfile,region,zone)
	#print cmd
	x=run(cmd,True ).strip()
	x=x.replace("+0000\n","+0000\t")
#	print x
#	return x
	lines=x.split('VOLUME')
	if len(lines)==1 and lines[0]=='':
		return simplejson.dumps(["No Volumes avaliable in " +  zone])
	#print lines
#	return lines
	d=[]
	for line in lines:
        	v={}
	        l=line.split()
		if len(l)==0:
			continue
     #   	print "l=" , l
	        v["id"]=l[0]
        	v["size"]=l[1]
	        if "snap-" in l[2]:
        	        v["snapshot-id"]=l[2]
                	v["zone"]=l[3]
	                v["status"]=l[4]
        	        v["created_on"]=l[5]
	        else:
        	        v["zone"]=l[2]
                	v["status"]=l[3]
	                v["created_on"]=l[4]
        	if "Name" in l:
                	v["name"]=l[l.index("Name")+1]
	        d.append(v)

	return simplejson.dumps(d)
def attachMount(instanceID, volID, device,mountPoint,format):
	#return volID
	x=run(mainPath+"/EC2 " + region  + " --attach %s %s %s" %(volID,instanceID,device),True)
#	return x
	command= mainPath+"/EC2 " + region + " --details " + instanceID
	t = run(command,True)
        k=t.split("\n")
	domain=""
#	return k
        for line in k:
                s=line.strip().split(": ")
               	if s[0]=="PublicDNS":
			 domain=s[1][:-2]
#	return format
	cmd=""
	if format=='N':
		cmd= "/root/HPCloud/CommandClient.py --run " + domain + " 5 'system' 'mkdir " + mountPoint + ";mount /dev/xvd"+ device[-1] + " " + mountPoint+"'"
	else:
		cmd= "/root/HPCloud/CommandClient.py --run " + domain + " 5 'system' 'mkfs /dev/xvd"  + device[-1] + ";mkdir " + mountPoint + ";mount /dev/xvd"+ device[-1] + " " + mountPoint+"'"
	#return cmd
        run(cmd,True)
	return "Done"	

def getVolInfo(volID):
	text=run(mainPath+"/EC2 "+ region + " --volume " + volID,True)
	v=text.split()
#	return v
	vol={}
	vol["id"]=v[1]
	vol["size"]=v[2]
	i=2
	if "snap-" in text:
		vol["snapshot"]=v[3]
		i=3
	else:
		vol["snapshot"]="N/A"
	vol["createdOn"]=v[i+3]
	vol["device"]=v[i+7]
	vol["attachedOn"]=v[i+9]
	return simplejson.dumps(vol)
def createVolume(pkfile,certfile,region, zone,size=0,snapshotID=None):
	t=""
	if snapshotID!=None:
		t=run(mainPath+"/EC2 %s %s %s --create-vol-from-snapshot %s %s" %(pkfile,certfile,region,snapshotID,zone),True)
	else:
		t=run(mainPath+"/EC2 %s %s %s --create-volume %s %s" %(pkfile,certfile,region,size,zone),True)
		print t
	return simplejson.dumps(t.split()[1])

def deleteVolume(pkfile,certfile,region,volID):
	t=run(mainPath+"/EC2 %s %s %s --delete-volume %s " %(pkfile,certfile,region,volID),True)
	return simplejson.dumps("DELETED " + t.strip())

def attachSnapShot(snapshotID,devicePath,mountPoint):
	f=open('/root/nodesList','r')
	nodes=f.readlines()
	for i  in range(0,len(nodes)):
		insID=nodes[i].strip()
		if ',X' in insID: continue
		zone=getNodeZone(insID)
		print "Node " + insID + " is at " + zone
		volume=run(mainPath+"/EC2 " + region + " --create-vol-from-snapshot " +  snapshotID + " " + zone,True).split()[1]
		print "New Vol: " + volume
		attachMount(insID,volume,devicePath,mountPoint,'N')
	return "Done"
