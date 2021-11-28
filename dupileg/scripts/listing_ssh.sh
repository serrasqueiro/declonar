#!/bin/bash
#	(c)2021 Henrique Moreira


usage ()
{
 echo "$0 [COMMAND]

No options.

If command is entered, the following are available:
	connect HOST
"
 exit 0
}


main_script()
{
 dump_ssh_client
 return $?
}


dump_ssh_client()
{
 # taken from:
 #	https://unix.stackexchange.com/questions/223276/how-do-i-list-available-host-key-algorithms-for-an-ssh-client
 local RES=2
 local F
 for F in $(ssh -Q help); do 
	printf "=== $F ===\n"
	ssh -Q $F
	RES=$?
	echo ""
 done
 return $RES
}


connect_host()
{
 # ssh -o "HostKeyAlgorithms ssh-rsa" user@hostname
 local CMD
 local U=$USER
 local H=$1

 [ "$H" = "" ] && echo "Please enter host!!!

" && usage
 [ "$U" = "" ] && usage
 # This one might yield...
 #ssh -o "HostKeyAlgorithms ssh-rsa" ${U}@${H}
 #
 #	Unable to negotiate with 127.0.0.1 port 22: no matching host key type found. Their offer: rsa-sha2-512,rsa-sha2-256,ecdsa-sha2-nistp256,ssh-ed25519

 # Run using 'Host key ED25519' :
 ssh -o "HostKeyAlgorithms ssh-ed25519" ${U}@${H}
 return 0
}


# Main script

case $1 in
	"")
		main_script; exit $?
	;;
	connect)
		shift
		connect_host $*; exit $?
	;;
	*)
		usage
	;;
esac

# Exit status
exit $RES
