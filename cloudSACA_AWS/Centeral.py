"""Setup and launch web service instances.
"""
import logging
import time
import datetime
from httplib import HTTP
from urlparse import urlparse

import boto
from boto.ec2.regioninfo import RegionInfo
from boto.exception import EC2ResponseError

log = logging.getLogger(__name__)

CM_POLICY =  """{
  "Statement": [
    {
      "Sid": "Stmt1319820532566",
      "Action": [
        "ec2:AttachVolume",
        "ec2:CreateSnapshot",
        "ec2:CreateTags",
        "ec2:CreateVolume",
        "ec2:DeleteSnapshot",
        "ec2:DeleteTags",
        "ec2:DeleteVolume",
        "ec2:DescribeAvailabilityZones",
        "ec2:DescribeInstanceAttribute",
        "ec2:DescribeInstances",
        "ec2:DescribeKeyPairs",
        "ec2:DescribePlacementGroups",
        "ec2:DescribeRegions",
        "ec2:DescribeSecurityGroups",
        "ec2:DescribeSnapshotAttribute",
        "ec2:DescribeSnapshots",
        "ec2:DescribeTags",
        "ec2:DescribeVolumes",
        "ec2:DetachVolume",
        "ec2:GetConsoleOutput",
        "ec2:Monitoring",
        "ec2:MonitorInstances",
        "ec2:RebootInstances",
        "ec2:RunInstances",
        "ec2:TerminateInstances"
      ],
      "Effect": "Allow",
      "Resource": "*"
    },
    {
      "Sid": "Stmt1319820637269",
      "Action": [
        "s3:CreateBucket",
        "s3:DeleteBucket",
        "s3:DeleteObject",
        "s3:GetBucketAcl",
        "s3:GetBucketPolicy",
        "s3:GetObject",
        "s3:GetObjectAcl",
        "s3:ListAllMyBuckets",
        "s3:ListBucket",
        "s3:ListBucketMultipartUploads",
        "s3:PutBucketAcl",
        "s3:PutBucketPolicy",
        "s3:PutObject",
        "s3:PutObjectAcl"
      ],
      "Effect": "Allow",
      "Resource": "arn:aws:s3:::*"
    }
  ]
}"""

# ## Cloud interaction methods
def connect_ec2(a_key, s_key, cloud):
    """ Create and return an EC2-compatible connection object.
    """
    #ci = _get_cloud_info(cloud)
    ci={}
    ci["region_name"] = 'us-east-1'
    ci["region_endpoint"]='ec2.amazonaws.com'
    ci["is_secure"]=True
    ci["ec2_port"]= None
    ci["ec2_conn_path"] = '/'
    ci["cloud_type"]="ec2"

    # AWS connection values
    # region_name = 'us-east-1'
    # region_endpoint = 'ec2.amazonaws.com'
    # is_secure = True
    # ec2_port = None
    # ec2_conn_path = '/'
    r = RegionInfo(name=ci['region_name'], endpoint=ci['region_endpoint'])
    ec2_conn = boto.connect_ec2(aws_access_key_id=a_key,
                          aws_secret_access_key=s_key,
                          # api_version is needed for availability zone support for EC2
                          api_version='2011-11-01' if ci['cloud_type'] == 'ec2' else None,
                          is_secure=ci['is_secure'],
                          region=r,
                          port=ci['ec2_port'],
                          path=ci['ec2_conn_path'])
    return ec2_conn

def create_iam_user(a_key, s_key, group_name='BioCloudCentral', user_name='cloudman'):
    """ Create IAM connection, swetup IAM group, add a user to it, and create a
        set of new credentials for that user.
        
        NOTE: This approach of generating IAM access keys each time a user starts
        a cluster implies that this overall process can only be used to start a
        cluster for the very first time a cluster is being created (this is because
        a cluster is identified by a user account access key and cluster name
        while this method creates new keys each time it's invoked). In order to
        start the same cluster at a later date, user data generated by this process
        should be used and provided in the AWS console.
    """
    access_key = secret_key = None
    try:
        iam_conn = boto.connect_iam(a_key, s_key)
        # Create an IAM group that will house users, first checking if such group exists
        grps = iam_conn.get_all_groups()
        iam_grp = None
        for grp in grps.groups:
            if grp.group_name == group_name:
                iam_grp = grp
                log.debug("Found IAM group %s" % grp.group_name)
                break
        if iam_grp is None:
            log.debug("Creating IAM group %s" % group_name)
            iam_grp = iam_conn.create_group(group_name)
        # Create JSON policy and associate it with the group
        cm_policy = CM_POLICY
        cm_policy_name = '%sPolicy' % group_name
        log.debug("Adding/updating IAM group %s policy: %s" % (group_name, cm_policy_name))
        iam_conn.put_group_policy(group_name, cm_policy_name, cm_policy)
        # If not existent, create user
        usrs = iam_conn.get_all_users()
        usr = None
        for user in usrs.users:
            if user.user_name == user_name:
                usr = user
                log.debug("Found IAM user %s" % user.user_name)
                break
        if usr is None:
            log.debug("Creating IAM user %s" % user_name)
            usr = iam_conn.create_user(user_name)
        # Add user to the group (this does no harm if user is already member of the group)
        iam_conn.add_user_to_group(group_name, user_name)
        # AWS allows only 2 sets of access keys to be created for any IAM user
        # so check if a new key for the given user can be created.
        existing_a_keys = iam_conn.get_all_access_keys(user_name)
        if len(existing_a_keys.access_key_metadata) >= 2:
            log.error("Wanted to create a new pair of IAM access keys but user %s already has 2 keys, which is the maximum number allowed by AWS. Not creating new IAM keys." % user_name)
            return access_key, secret_key
        log.debug("Creating new access keys for IAM user '%s'" % user_name)
        r = iam_conn.create_access_key(user_name)
        access_key = r.access_key_id
        secret_key = r.secret_access_key
        # wait for key to become active
        time.sleep(5)
    except Exception, e:
        log.error("Trouble dealing with IAM: %s" % e)
    return access_key, secret_key

def create_cm_security_group(ec2_conn, cloud, sg_name='CloudMan'):
    """ Create a security group with all authorizations required to run CloudMan.
        If the group already exists, check its rules and add the missing ones.
        Return the name of the created security group.
    """
    cmsg = None
    # Check if this security group already exists
    sgs = ec2_conn.get_all_security_groups()
    for sg in sgs:
        if sg.name == sg_name:
            cmsg = sg
            log.debug("Security group '%s' already exists; will add authorizations next." % sg_name)
            break
    # If it does not exist, create security group
    if cmsg is None:
        log.debug("Creating Security Group %s" % sg_name)
        cmsg = ec2_conn.create_security_group(sg_name, 'A security group for CloudMan')
    # Add appropriate authorization rules
    # If these rules already exist, nothing will be changed in the SG
    ports = (('80', '80'), # Web UI
             ('20', '21'), # FTP
             ('22', '22'), # ssh
             ('30000', '30100'), # FTP transfer
             ('42284', '42284')) # CloudMan UI
    for port in ports:
        try:
            if not rule_exists(cmsg.rules, from_port=port[0], to_port=port[1]):
                cmsg.authorize(ip_protocol='tcp', from_port=port[0], to_port=port[1], cidr_ip='0.0.0.0/0')
            else:
                log.debug("Rule (%s:%s) already exists in the SG" % (port[0], port[1]))
        except EC2ResponseError, e:
            log.error("A problem with security group authorizations: %s" % e)
    # Add rule that allows communication between instances in the same SG
    g_rule_exists = False # Flag to indicate if group rule already exists
    ci = _get_cloud_info(cloud)
    cloud_type = ci['cloud_type']
    cidr_range = ci.get('cidr_range', '')
    # AWS allows grants to be named, thus allowing communication within a group.
    # Other cloud middlewares do now support that functionality so resort to CIDR grant.
    if cloud_type == 'ec2':
        for rule in cmsg.rules:
            for grant in rule.grants:
                if grant.name == cmsg.name:
                    g_rule_exists = True
                    log.debug("Group rule already exists in the SG")
            if g_rule_exists:
                break
    else:
        for rule in cmsg.rules:
            for grant in rule.grants:
                if grant.cidr_ip == cidr_range:
                    g_rule_exists = True
                    log.debug("Group rule already exists in the SG")
                if g_rule_exists:
                    break
    if g_rule_exists is False: 
        try:
            if cloud_type == 'ec2':
                cmsg.authorize(src_group=cmsg)
            else:
                cmsg.authorize(ip_protocol='tcp', from_port=1, to_port=65535, cidr_ip=cidr_range)
        except EC2ResponseError, e:
            log.error("A problem w/ security group authorization: %s" % e)
    log.info("Done configuring '%s' security group" % cmsg.name)
    return cmsg.name

def rule_exists(rules, from_port, to_port, ip_protocol='tcp', cidr_ip='0.0.0.0/0'):
    """ A convenience method to check if an authorization rule in a security
        group exists.
    """
    for rule in rules:
        if rule.ip_protocol == ip_protocol and rule.from_port == from_port and \
           rule.to_port == to_port and cidr_ip in [ip.cidr_ip for ip in rule.grants]:
            return True
    return False

def create_key_pair(ec2_conn, key_name='cloudman_key_pair'):
    """ Create a key pair with the provided name.
        Return the name of the key or None if there was an error creating the key. 
    """
    kp = None
    # Check if a key pair under the given name already exists. If it does not,
    # create it, else return.
    kps = ec2_conn.get_all_key_pairs()
    for akp in kps:
        if akp.name == key_name:
            log.debug("Key pair '%s' already exists; not creating it again." % key_name)
            return akp.name, None
        # else:
        # This cannot be enabled if we're wanting to ensure user's can access
        # the instance via a keypair - need to ensure they can download the
        # private key
        #     log.debug("Using first existing keypair: '%s'" % akp.name)
        #     return akp.name, None
    try:
        kp = ec2_conn.create_key_pair(key_name)
    except EC2ResponseError, e:
        log.error("Problem creating key pair '%s': %s" % (key_name, e))
        return None, None
    log.info("Created key pair '%s'" % kp.name)
    return kp.name, kp.material

def run_instance(ec2_conn, user_provided_data, image_id,instance_type,
                 kernel_id=None, ramdisk_id=None, key_name='cloudman_key_pair',
                 security_groups=['CloudMan'], placement=''):
    """ Start an instance. If instance start was OK, return the ResultSet object
        else return None.
    """
  
    rs = None
   
    if placement == '':
        placement = _find_placements(ec2_conn, instance_type)[0]
    # Compose user data
    ud='\n'.join(user_provided_data.split('\\n'))
    if type(user_provided_data)!=type("a"):
		ud = _compose_user_data(user_provided_data)  
    msg = None
    try:
        rs = ec2_conn.run_instances(image_id=image_id,
                                    instance_type=instance_type,
                                    key_name=key_name,
                                    security_groups=security_groups,
                                    user_data=ud,
                                    kernel_id=kernel_id,
                                    ramdisk_id=ramdisk_id,
                                    placement=placement)
    except EC2ResponseError, e:
        log.error("Problem starting an instance: %s" % e)
        msg = str(e)
    if rs:
        try:
            log.info("Started an instance with ID %s" % rs.instances[0].id)
        except Exception, e:
            msg = str(e)
            log.error("Problem with the started instance object: %s" % e)
    else:
        log.warning("Problem starting an instance?")
    return {"rs": rs, "error": msg}

def _compose_user_data(user_provided_data):
    form_data = {}
    # Do not include the following fields in the user data but do include
    # any 'advanced startup fields' that might be added in the future
    excluded_fields = ['sg_name', 'image_id', 'instance_id', 'kp_name', 'cloud', 'cloud_type',
        'public_dns', 'cidr_range', 'kp_material', 'placement']
    for key, value in user_provided_data.iteritems():
        if key not in excluded_fields:
            form_data[key] = value
    # If the following user data keys are empty, do not include them in the request user data
    udkeys = ['post_start_script_url', 'worker_post_start_script_url', 'bucket_default', 'share_string']
    for udkey in udkeys:
        if udkey in form_data and form_data[udkey] == '':
            del form_data[udkey]
    # Check if bucket_default is defined for the given cloud and include
    # it if it was not provided by the user in the instance request form
    # but do so only if it's blank - blank conflicts with CloudMan's ec2autorun.py
    #if 'bucket_default' not in form_data and user_provided_data['cloud'].bucket_default != '':
     #   form_data['bucket_default'] = user_provided_data['cloud'].bucket_default
    # Convert form_data into the YAML format
    ud = "\n".join(['%s: %s' % (key, value) for key, value in form_data.iteritems()])
    # Also include connection info about the selected cloud
    ci = _get_cloud_info("ec2", as_str=True)
    return ud + "\n" + ci

def _get_cloud_info(cloud, as_str=False):
	ci={}
	ci["region_name"] = 'us-east-1'
	ci["region_endpoint"]='ec2.amazonaws.com'
	ci["is_secure"]=True
	ci["ec2_port"]= None
	ci["ec2_conn_path"] = '/'
	ci["cloud_type"]="ec2"
	if as_str:
		ci = "\n".join(['%s: %s' % (key, value) for key, value in ci.iteritems()])
	return ci

def _find_placements(ec2_conn, instance_type=None):
    """Find region zones that supports our requested instance type.

    If instance_type is None, finds all zones that are currently available.

    We need to check spot prices in the potential availability zones
    for support before deciding on a region:

    http://blog.piefox.com/2011/07/ec2-availability-zones-and-instance.html
    """
    zones = []
    yesterday = datetime.datetime.now() - datetime.timedelta(1)
    back_compatible_zone = "us-east-1b"
    for zone in ec2_conn.get_all_zones():
        if zone.state in ["available"]:
            if (instance_type is None or
                len(ec2_conn.get_spot_price_history(instance_type=instance_type,
                                                    end_time=yesterday.isoformat(),
                                                    availability_zone=zone.name)) > 0):
                zones.append(zone.name)
    zones.sort()
    if back_compatible_zone in zones:
        zones = [back_compatible_zone] + [z for z in zones if z != back_compatible_zone] 
    if len(zones) == 0:
        log.error("Did not find availabilty zone in {0} for {1}".format(base, instance_type))
        zones.append(back_compatible_zone)
    return zones

def instance_state(ec2_conn, instance_id):
    rs = None
    state = {'instance_state': "",
             'public_dns': "",
             'placement': ""}
    try:
        rs = ec2_conn.get_all_instances([instance_id])
        if rs is not None:
            inst_state = rs[0].instances[0].update()
            public_dns = rs[0].instances[0].ip_address #public_dns_name
            if inst_state == 'running':
                cm_url = "http://{dns}/cloud".format(dns=public_dns)
                if _checkURL(cm_url) is True:
                    state['public_dns'] = public_dns
                    state['instance_state'] = inst_state
                    state['placement'] = rs[0].instances[0].placement
                else:
                    # Wait until the URL is accessible to display the IP
                    state['instance_state'] = 'booting'
            else:
                state['instance_state']= inst_state
    except Exception, e:
        log.error("Problem updating instance '%s' state: %s" % (instance_id, e))
    return state

def _checkURL(url):
    """ Check if a url is alive (returns code 200(OK) or 401 (unauthorized))
    """
    try:
        p = urlparse(url)
        h = HTTP(p[1])
        h.putrequest('HEAD', p[2])
        h.endheaders()
        r = h.getreply()
        if r[0] == 200 or r[0] == 401: # CloudMan UI is pwd protected so include 401
            return True
    except Exception:
        # No response or no good response
        pass
    return False
    