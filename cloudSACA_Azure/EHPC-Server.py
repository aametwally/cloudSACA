#!/usr/bin/python
from Crypto.PublicKey import RSA
from Crypto import Random
import socket,threading,subprocess,os,base64,xml.dom.minidom
import config,PBS,Response,Request,ServerJob as Job  
JOBS={}
EOM="\n\n###"
def startServer():
    port=int(config.port)
    s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    s.bind(('',port))
    s.listen(5)
    print "Server started at " + str(port)
    return s
def importKey():
	file=open(config.publicKey,'r')
	st = "".join(file.readlines())
	key = RSA.importKey(st)
	return key
def validReq(req):
	key =importKey()
	decrypted=key.decrypt(base64.decodestring(req["sec"]))
	if (req["JobID"] + req["Owner"]==decrypted):
		return True
	else: return False
def HandleClient(clientsock):
	name = threading.currentThread().getName()
	print name, ' Started.............'
	global EOM
	chunks=[]
	while 1:
		buf = clientsock.recv(2048)
		chunks.append(str(buf))
		if (EOM in chunks[-1]):
			msg= "".join(chunks)[:-5]
			if (msg=="TEST: HELLO"):
				return
			req =Request.parseRequest(msg)
			if (not validReq(req)):
				Response.sendData(clientsock,"Invalid Request")
				print "invalid request"
				clientsock.close()
				return 
			if (req["requestType"]=="SUBMIT"):
				job=Request.parseJob(msg)
				global	JOBS
				if (req["Owner"]=="system" or req["Owner"]=="utils"):
                                        res= PBS.run(job["command"],req["JobID"])
                                        if req["Owner"]=="system":
                                                Response.sendData(clientsock,"Done")
                                        else:
                                                Response.sendData(clientsock,res)
				elif req["Owner"]=="ubuntu":
					res= PBS.run("su ubuntu -c '"+job["command"]+"'",req["JobID"])
					Response.sendData(clientsock,res)
				elif req["Owner"]=="ehpcuser":
                                        res= PBS.run("su ehpcuser -c '"+job["command"]+"'",req["JobID"])
                                        Response.sendData(clientsock,res)
				else:
					print "command:" + job["command"]
#					print "inputs:" + job["inputs"]
					c = PBS.AddFileWrapper("direct",job["command"],job["inputs"],job["outputs"])
					id= PBS.runAsPBSJob(req["Owner"],req["JobID"],c)
					Response.sendData(clientsock,'recieved:'+id)
				clientsock.close()
				
			elif (req["requestType"]=="STATUS"):
				status=Job.checkPBSstatus(req["JobID"])
				Response.sendData(clientsock,Job.perpareJobStatusMessage(req,status))
			elif (req["requestType"]=="FETCH"):
				OutputList = Request.getOutputList(msg)
				response = Response.generateResponse(msg,OutputList)
				Response.sendData(clientsock,response)
			elif (req["requestType"]=='UPLOAD'):
				UploadDict = Request.parseUpload(msg)
				for key, value in UploadFileData.iteritems():
					createFile = open(key, "wb")
					while True:
                        data = conn.recv(1024)
                        createFile.write(value)
					createFile.close()

			break



s=startServer()
i=0
while 1:
	clientsock,clientaddr=s.accept()
	i+=1
	print 'Got connection from ' , clientsock.getpeername()
	t=threading.Thread(target=HandleClient,args=[clientsock], name="Thread #" + str(i))
	t.start()

sys.exit(0)
