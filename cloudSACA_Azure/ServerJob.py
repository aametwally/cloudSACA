import socket,threading,subprocess,os,base64,xml.dom.minidom
import PBS
def perpareJobStatusMessage(req,status):
        st = """<job id="%s" owner="%s" status="%s"/>""" % (req["JobID"],req["Owner"], status)
#        print st
        return st


def getJobStatus(req):
	global JOBS
	print 'CurrentJobs:',JOBS
	st = """<job id="%s" owner="%s" status="%s"/>""" % (req["JobID"],req["Owner"], JOBS[req["JobID"]][1])
	print st
	return st

def checkPBSstatus(id):
	list= PBS.run("qstat -f "+ id).split("\n")
	for item in list:
		if "job_state" in item:
			return item.split('=')[-1].strip()
