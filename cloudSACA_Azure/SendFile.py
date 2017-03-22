#! /usr/bin/python
import socket,base64,time,sys,subprocess

class Client:
	gateway = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	host = ''
	port = 6000
	file = ''
	def __init__(self, host, port, file):
		self.port = port
		self.host = host
		self.file = file
		self.connect()

	def connect(self):
		self.gateway.connect((self.host, self.port))
		self.sendFileName()
		self.sendFile()

	def sendFileName(self):
		path,fileName = os.path.split(self.file)
		self.gateway.send(self.outputDir+","+fileName)

	def sendFile(self):
		readByte = open(self.file, "rb")
		data = readByte.read()
		readByte.close()

		self.gateway.send(data)
		self.gateway.close()

a = Client(sys.argv[1], 6000, sys.argv[2])
