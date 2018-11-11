#!/bin/bash -xe

# the first one should be the root
ips=(10.83.142.133 10.181.123.150 10.234.145.144 10.197.251.208 10.45.32.61)

. ../sdk.env

# init namespaces
brctl addbr br-chall
brctl stp br-chall off
ip link set dev br-chall up
i=0
ip link add tap$i type veth peer name br-tap$i
brctl addif br-chall br-tap$i
ip link set dev br-tap$i up
ifconfig tap$i 10.0.0.1/8
i=1
for ip in ${ips[@]}; do
	ip netns add ns$i
	ip link add tap$i type veth peer name br-tap$i
	brctl addif br-chall br-tap$i
	ip link set dev br-tap$i up
	ip link set tap$i netns ns$i
	ip netns exec ns$i ifconfig tap$i $ip/8
	i=$((i+1))
done

# start servers
i=1
for ip in ${ips[@]}; do
	ip netns exec ns$i qemu-arm -L /usr/arm-linux-gnueabi ../highest-power &
	i=$((i+1))
done
sleep 2

# query flag
curl -v "http://${ips[0]}:8000/gather_everyone_to_reveal_our_secret"

# tear down
ip link set dev br-chall down
brctl delbr br-chall
i=0
ip link del br-tap$i
i=1
for ip in ${ips[@]}; do
	ip netns del ns$i
	ip link del br-tap$i
	i=$((i+1))
done
