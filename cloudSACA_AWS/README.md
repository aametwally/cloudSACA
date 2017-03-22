Author: Ahmed Abdelhadi
Special Thanks to Mohamed Kalioby for his support (The main developer of EHPC package)


A) To create a cluster:
1- On demand Cluster:
./cloudSACA_AWS.sh --create <options>
2- spot instance cluster 
./cloudSACA_AWS.sh --spot-request <options>


B) To login to the master node:
./cloudSACA_AWS.sh --login --kp=<path_to_keypair>

C) To terminate the cluster:
./cloudSACA_AWS.sh --terminate


Examples:

1- Creating on demand Cluster
./cloudSACA_EHPC.sh --create -r=EU1 --name=hadympi -ami=ami-fc4cad8b -pk=/home/hady/Desktop/AWS/pk-KLDQBZR7MLEDGVRSSETGH4KBXWTLGSGO.pem -cert=/home/hady/Desktop/AWS/cert-KLDQBZR7MLEDGVRSSETGH4KBXWTLGSGO.pem -kp=hadyuseast -sg=sg-dd19f0b2 -t=m1.medium -n=2


2- Creating spot request Cluster:
./cloudSACA_EHPC.sh --spot-request -r=EU1 --name=hadympi -ami=ami-fc4cad8b -pk=/home/hady/Desktop/AWS/pk-KLDQBZR7MLEDGVRSSETGH4KBXWTLGSGO.pem -cert=/home/hady/Desktop/AWS/cert-KLDQBZR7MLEDGVRSSETGH4KBXWTLGSGO.pem -kp=hadyuseast -sg=sg-dd19f0b2 -t=m1.medium -n=2 --price=0.2
