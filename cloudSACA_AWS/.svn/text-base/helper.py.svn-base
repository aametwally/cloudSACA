#! /usr/bin/env python	
import boto,sys,os
from boto.s3.connection import S3Connection
from boto.s3.connection import Location

############################################################################################################
## This file has some functions that may be useful to the user. This file is not used by the library.     ##
##  By: Mohamed M. El-Kalioby																			  ##
##  Last Update: Dec 21, 2011                                                                             ##
############################################################################################################
if sys.argv[1]=="--s3":
	AccessKey=sys.argv[2]
	SecretKey=sys.argv[3]
	conn = S3Connection(AccessKey,SecretKey)
	buckets = str(conn.get_all_buckets())
	print buckets
