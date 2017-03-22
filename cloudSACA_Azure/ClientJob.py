import socket,base64,time,sys,subprocess
from Crypto.PublicKey import RSA
import os
from Crypto import Random
def importKey():
	mainPath=os.path.dirname(os.path.abspath( __file__ ))
	file=open(mainPath + "/key",'r')
	st = "".join(file.readlines())
	key = RSA.importKey(st)
	#print "KEY Opened" , key
	return key


#Provide id, owner and command as string 
# inputsFiles as List of file path
def createJobMessage(id,owner,command,inputsFiles,outputFiles,retry):
	sec=base64.encodestring(importKey().encrypt(id+owner,"")[0])
	f='<job id="' + id + '" owner="' + owner + '" type="SUBMIT" sec="'+sec+'">\n'
	previousPath=""
	
	if (inputsFiles!=None):
		for file in inputsFiles:
			newName=os.path.abspath(os.path.expanduser(file))
			command=command.replace(file+" " ,newName+ " ")
			file=newName
			if file =='': continue
			parts= file.split("/")
			path="/".join(parts[:-1])
			if not os.path.exists(file):
				print "Error: " + file +" doesn't exist"
				exit(-11)
			out=open(file+'.64','w')
			base64.encode(open(file,'r'),out)
			out.flush()	
			out.close()
			out=open(file+'.64','r')
			encoded=out.read()	
			out.close()
			if (path != previousPath):
				if (previousPath != ""):	
					f+="</folder>\n"
				previousPath=path
				f += '\t<folder path="' + path  + '">\n'
			f += '\t\t<input filename="'+parts[-1]+'">'+encoded +'</input>\n'
		if(len(inputsFiles)!=0):		
			f+='</folder>\n'
			
	if (outputFiles!=None):	
		previousPath=""
		for file in outputFiles:
			parts= file.split("/")
			path="/".join(parts[:-1])
			if (path != previousPath):
				if (previousPath !=""):	f+="</folder>"
				previousPath=path
				f += '<folder path="' + path  + '">\n'
				f += '\t<output filename="'+parts[-1]+'"/>'
		if(len(outputFiles)!=0):		
			f+='</folder>\n'
	f += '\t<command>'+base64.encodestring(command)+'</command>\n'
	f+= '</job>'
	
	return f

def FindFiles(id,folder,owner):
	sec=base64.encodestring(importKey().encrypt(id+owner,"")[0])
	f='<job id="' + id + '" owner="' + owner + '" type="FIND" sec="'+ sec+'"></job>'
	return f
	
def parseFileList(msg):
	import xml.dom.minidom, os
	outputFilesList=[]
	doc = xml.dom.minidom.parseString(msg)
	folders=doc.getElementsByTagName("folder")
	res=[]
	for folder in folders:
			path=folder.getAttribute("path")
			Files = folder.getElementsByTagName("file")
			if (Files!=None):
					for file in Files:
							filename= file.getAttribute('filename')
							res.append(path+'/' +filename)
	return res

	
	

def FetchOutput(id,owner,outputFiles):
	sec=base64.encodestring(importKey().encrypt(id+owner,"")[0])
	f='<job id="' + id + '" owner="' + owner + '" type="FETCH" sec="' +sec+'">\n'
	previousPath=''
	
	for file in outputFiles:
		parts= file.split("/")
		#print parts
		path="/".join(parts[:-1])
		#print path
		if (path != previousPath):
			if (previousPath !=""):	f+="</folder>"
			previousPath=path
			f += '<folder path="' + path  + '">\n'
		f += '\t<output filename="'+parts[-1]+'"/>'
	if(len(outputFiles)!=0):		
		f+='</folder>\n'
	f+= '</job>'

	#print f
	return f
	
def UploadFileData(id,owner,filePath,outputdir):
	sec=base64.encodestring(importKey().encrypt(id+owner,"")[0])
	st =  '<job id="'+id+'" owner="'+owner+'" type="UPLOAD" sec="' + sec +'">\n'
	path,fileName = os.path.split(filePath)
	readByte = open(filePath, "rb")
	data = readByte.read()
	readByte.close()
	# send the file name to server:
	st += '\t<uploadfilename>'+base64.encodestring(outputdir+"/"+fileName)+'</uploadfilename>\n' 
	st += '\t<uploaddata>'+base64.encodestring(data)+'</uploaddata>\n'
	st += '</job>'
	return st
	
def checkJobStatus(Connect,id,owner,waitTillCompleted=True,retry=True):
	sec=base64.encodestring(importKey().encrypt(id+owner,"")[0])
	st =  '<job id="'+id+'" owner="'+owner+'" type="STATUS" sec="' + sec +'"/>'
	while 1:
		res = Connect.Send(st,retry)
		#print res
		status=res[res.find('status="')+8:][0]
		if waitTillCompleted:
			print 'status of ' + id  + ' = ' + status
		else:
			return status
		if (status!='R' and status!='Q'): break
		#print "checking agin in 15 seconds"
		time.sleep(15)
		
	return status
def parseResponse(message,outDir=""):
		
        import xml.dom.minidom
        import os
        outputFilesList=[]
        doc = xml.dom.minidom.parseString(message)
        folders=doc.getElementsByTagName("folder")
        for folder in folders:
			path=folder.getAttribute("path")
			Temp=folder.getAttribute("Temp")
			if (Temp=="True" and outDir!=""):
				path=outDir
			#print 'In folder:' , path
			if (not os.path.exists(path)): os.makedirs(path)
			outputs = folder.getElementsByTagName("output")
			if (outputs!=None):
					for output in outputs:
							filename= output.getAttribute('filename')
							text=""
							content=""
							if output.firstChild!=None:
								text=output.firstChild.nodeValue
								content=base64.decodestring(text)
			 
							file= open(path+"/"+filename,'w')
							file.write(content)
							file.flush()
							file.close()
							outputFilesList.append(str(path+'/' +filename))
								

        #print "output files: " , str(outputFilesList)
        return outputFilesList

def run(excuter):
	
	PIPE=subprocess.PIPE
	p=subprocess.Popen(excuter,stdout=PIPE,stderr=PIPE,shell=True)
	p.wait()
	st=p.stderr.read()
	return p.stdout.read()

#command='clustalw2 -profile -profile1 /home/user/clustalw/profile1 -profile2 /home/user/clustalw/profile2 -out /home/user/clustalw/output'
#command = command.strip()
#owner = "ahmed"
#id = "1"
#inputFiles=['/home/ahmed/.ssh/id_rsa','/home/ahmed/.ssh/id_rsa.pub'] 
#outFiles=['/home/ahmed/Desktop/ahmed.output']
#createJobMessage(id,owner,command,inputFiles,outFiles,1)
