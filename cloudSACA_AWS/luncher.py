#! /usr/bin/python

import Centeral,time

def startEC2_with_userdata(a_key,s_key,amiID,userData,key_name,security_group,instance_type):
	ec2_conn=Centeral.connect_ec2(a_key,s_key,'ec2')
	r= Centeral.run_instance(ec2_conn, userData, amiID,instance_type,kernel_id=None, ramdisk_id=None, key_name="MyUSEastKey", security_groups=[security_group], placement='')
	ReservationID=r["rs"]
	return  ReservationID
def getInstances_in_reservation(a_key,s_key,ReservationID):
	ec2_conn=Centeral.connect_ec2(a_key,s_key,'ec2')
	found=False
	while not found:
		time.sleep(15)
		reservations=ec2_conn.get_all_instances()
		for res in reservations:
			if str(res)==str(ReservationID):
				ins=res.instances
				for i in ins:
					id=str(i).split(":")[1]
					return id
		print "one more wait"
			
