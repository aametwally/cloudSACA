#!/bin/bash


MasterNode=`cat MasterNodeURL`
command=`echo $@`
OPTIND=1
keypair=""


if [ $1 == "--create" ] || [ $1 == "--spot-request" ]; then
#./cloudSACA_EHPC.sh --create <options>
#./cloudSACA_EHPC.sh --spot-request <options>

	echo "You have called --create or --spot-request!"
	./EHPC-Client $command > CreateLog
	if [ $1 == "--spot-request" ]; then
	grep URL CreateLog| cut -c 8- | sed '$s/.$//' > MasterNodeURL
	else
	grep URL CreateLog| cut -c 6- | sed '$s/ //' > MasterNodeURL
	fi

else 
#./cloudSACA_EHPC.sh --login --kp=<path_to_keypair>
	if [ $1 == "--login" ]; then
		echo "You have called --login"
		for i in "$@"
		do
		case $i in
			--kp=*)
			keypair=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
			;;
		esac
		done

		ssh -i $keypair ubuntu@$MasterNode
	else 
	#./cloudSACA_EHPC.sh --terminate
		if [ $1 == "--terminate" ]; then
			echo "You have called  --terminate"
			./EHPC-Client --terminate -d=$MasterNode
		else
			echo "Enter correct choice"
		fi
	fi
fi

