#!\bin\sh

mode=$1
rate=$2
limit=100
if [ "$mode" = "c" ]
then 
	echo consumer mode with rate $rate
	count=0
	inst=$3
	if [ "$inst" = "" ]
	then
		echo command usage is pr_cs.sh \<c\> \<rate\> \<inst\>
	else
		while [ "$count" -lt "$limit" ]
		do		
			consume_item=$(cat /dev/deeds_fifo)
			echo consumer instance $inst : $consume_item
			sleep_rate=`expr 1 / $rate`
			sleep $sleep_rate
			count=`expr $count + 1`
		done
	fi
elif [ "$mode" = "p" ] 
then
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
		while [ "$count" -lt "$limit" ]
		do
			sudo echo $msg>/dev/deeds_fifo
			echo producer instance $inst : written data.
			sleep_rate=`expr 1 / $rate`
			sleep $sleep_rate
			count=`expr $count + 1`
		done		
	fi
elif [ "$mode" = "stats" ]
then
	a=0
	while [ "$a" -lt "$limit" ]
	do
		stats=$(cat /proc/deeds_fifo_stats)
		echo Stats: '\n' $stats
		a=`expr $a + 1`
		sleep $rate
	done
else
	echo command usage is pr_cs.sh \<p\|c\|stats\> \<rate\> \<msg\> \<inst\>
fi

