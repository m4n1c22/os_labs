#!\bin\sh

mode=$1
rate=$2

if [ "$mode" = "c" ]
then 
	echo consumer mode with rate $rate
	count=0
	inst=$3
	if [ "$inst" = "" ]
	then
		echo command usage is pr_cs.sh \<c\> \<rate\> \<inst\>
	else
		while [ "$count" -lt 100 ]
		do		
			consume_item=$(cat /dev/deeds_fifo)
			echo consumer instance $inst : $consume_item
			sleep_rate=`expr 1 / $rate`
			sleep $sleep_rate
			count=`expr $count + 1`
		done
	fi
else
	echo producer mode with rate $rate
	count=0
	msg=$3
	inst=$4
	if [ "$msg" = "" ]
	then
		echo command usage is pr_cs.sh \<p\> \<rate\> \<msg\> \<inst\>
	elif [ "$inst" = "" ]
	then	
		echo command usage is pr_cs.sh \<p\> \<rate\> \<msg\> \<inst\>
	else
		while [ "$count" -lt 100 ]
		do
			sudo echo $msg>/dev/deeds_fifo
			echo producer instance $inst : written data.
			sleep_rate=`expr 1 / $rate`
			sleep $sleep_rate
			count=`expr $count + 1`
		done		
	fi

fi
