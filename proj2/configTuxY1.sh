#!/bin/bash

echo -n "Restarting networking service... "
service networking restart
echo "done!"

ifconfig eth0 up 172.16.30.1/24
echo "IP Address set."

route add default gw 172.16.30.254
echo "Default gateway route set."

route add -net 172.16.31.0/24 gw 172.16.30.254
echo "Route to VLAN 31 set."

echo "172.16.1.1" > /etc/resolv.conf
echo "DNS set."

ping -q -c5 google.com > /dev/null
 
if [ $? -eq 0 ]
then
	echo -e "\033[0;32mSUCCESS\033[0m: Could ping a foreign host, by hostname. Network and DNS are OK."
else
  echo -e "\033[0;31mERROR\033[0m: Could NOT ping a foreign host. Please review!" 
fi
