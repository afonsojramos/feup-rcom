#!/bin/bash

echo -n "Restarting networking service... "
service networking restart
echo "done!"

ifconfig eth0 up 172.16.31.1/24
echo "IP Address set."

route add default gw 172.16.31.254
echo "Default gateway route set."

route add -net 172.16.30.0/24 gw 172.16.31.253
echo "Route to VLAN 30 set."

printf "search netlab.fe.up.pt\nnameserver 172.16.1.1\nnameserver 172.16.2.1\n" > /etc/resolv.conf
echo "DNS set."

echo "Performing ping test..."
ping -q -c 1 google.pt  > /dev/null


zero=0

if [ "$?" -ne "$zero" ]; then
	echo -n -e "\033[0;31mERROR\033[0m: Could NOT ping a foreign host. Please review! Error "
else
	echo -e "\033[0;32mSUCCESS\033[0m: Could ping a foreign host, by hostname. Network and DNS are OK."
fi
