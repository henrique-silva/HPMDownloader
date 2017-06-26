#!/bin/bash

counter_upgrade=0
counter_success=0
slot=2
addr=$(($slot*2))
addr=$((addr+112))

while :
do

	echo -e "0\n0x60\n0x1235\n1\n0\n1\n1\n192.168.178.160\nadmin\nadmin\n$slot\n" | bin/hpmdownloader.exe img/atmega128_pmTestPad.hex > /dev/null
	counter_upgrade=$((counter_upgrade+1))
	sleep 5

	test_pmTestPadFRU=$(/cygdrive/c/ipmitool/src/ipmitool.exe -I lan -H 192.168.178.160 -U admin -P admin -T 0x82 -b 7 -t $addr fru)

	if [[ $test_pmTestPadFRU == *"PM TestPad"* ]] 
	then
		counter_success=$((counter_success+1))	
		echo "Success : $counter_success / $counter_upgrade tests"
	fi


	echo -e "0\n0x60\n0x1235\n1\n0\n1\n1\n192.168.178.160\nadmin\nadmin\n$slot\n" | bin/hpmdownloader.exe img/atmega128_alb.hex > /dev/null
	counter_upgrade=$((counter_upgrade+1))
	sleep 5

	test_mtf7FRU=$(/cygdrive/c/ipmitool/src/ipmitool.exe -I lan -H 192.168.178.160 -U admin -P admin -T 0x82 -b 7 -t $addr fru)

	if [[ $test_mtf7FRU == *"AMC-Loadboard"* ]] 
	then
		counter_success=$((counter_success+1))	
		echo "Success : $counter_success / $counter_upgrade tests"
	fi

done
