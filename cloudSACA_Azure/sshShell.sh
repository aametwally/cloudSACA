#!/bin/bash
USERNAME=ehpcuser
HOSTS="$1"
SCRIPT="$2"
#ifconfig eth0 | grep 'inet addr:'
#ahmed-Sate516-0.cloudapp.net
key=keys/mycert.pem
#for HOSTNAME in ${HOSTS} ; do
ssh -i ${key} -l ${USERNAME} ${HOSTS}  "${SCRIPT}"
#done
