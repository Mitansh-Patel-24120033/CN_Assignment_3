from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller, OVSSwitch
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel, info
import time

def buildTOPO():
	net=Mininet(switch=OVSSwitch, link=TCLink)
	c0=net.addController('c0')
	# Add switches
	s1 = net.addSwitch('s1')
	s2 = net.addSwitch('s2')
	s3 = net.addSwitch('s3')
	s4 = net.addSwitch('s4')

	# Add hosts
	h1 = net.addHost('h1', ip='10.1.1.2/24')
	h2 = net.addHost('h2', ip='10.1.1.3/24')
	h3 = net.addHost('h3', ip='172.16.10.4/24')
	h4 = net.addHost('h4', ip='172.16.10.5/24')
	h5 = net.addHost('h5', ip='172.16.10.6/24')
	h6 = net.addHost('h6', ip='172.16.10.7/24')
	h7 = net.addHost('h7', ip='172.16.10.8/24')
	h8 = net.addHost('h8', ip='172.16.10.9/24')
	
	# NAT host
	h9 = net.addHost('h9', ip='172.16.10.10/24')

	# Connect switches to switches
	net.addLink(s1, s2, delay='7ms')
	net.addLink(s2, s3, delay='7ms')
	net.addLink(s3, s4, delay='7ms')
	net.addLink(s4, s1, delay='7ms')
	net.addLink(s1, s3, delay='7ms')

        
	# Connect hosts to switches
	net.addLink(h3, s2, delay='5ms')
	net.addLink(h4, s2, delay='5ms')
	net.addLink(h5, s3, delay='5ms')
	net.addLink(h6, s3, delay='5ms')
	net.addLink(h7, s4, delay='5ms')
	net.addLink(h8, s4, delay='5ms')
	
	# NAT links
	net.addLink(s1, h9, delay='5ms')
	net.addLink(h1, h9, delay='5ms')
	net.addLink(h2, h9, delay='5ms')
        
	net.start()
	for i in range(3, 9):
		host = net.get('h%d' % i)
		host.cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
		info(f'*** Added route to {host.name}\n')
	h9 = net.get('h9')
	h1 = net.get('h1')
	h2 = net.get('h2')

	# Create bridge for private network
	h9.cmd('brctl addbr br-private')
	h9.cmd('brctl addif br-private h9-eth1')
	h9.cmd('brctl addif br-private h9-eth2')
	h9.cmd('ifconfig br-private 10.1.1.1/24 up')

	# Enable IP forwarding
	h9.cmd('sysctl -w net.ipv4.ip_forward=1')

	# Configure NAT rules
	h9.cmd('iptables -t nat -A POSTROUTING -o h9-eth0 -j MASQUERADE')
	h9.cmd('iptables -A FORWARD -i br-private -o h9-eth0 -j ACCEPT')
	h9.cmd('iptables -A FORWARD -i h9-eth0 -o br-private -m state --state RELATED,ESTABLISHED -j ACCEPT')

	# Enable proxy ARP
	h9.cmd('sysctl -w net.ipv4.conf.br-private.proxy_arp=1')
	h9.cmd('sysctl -w net.ipv4.conf.all.proxy_arp=1')

	# Configure default routes for private hosts
	h1.cmd('ip route add default via 10.1.1.1')
	h2.cmd('ip route add default via 10.1.1.1')
	h9.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p icmp --icmp-type echo-request -j DNAT --to-destination 10.1.1.2')
	h9.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p icmp --icmp-type echo-request -j DNAT --to-destination 10.1.1.3')
	
	# Enabling STP
	for switch in net.switches:
		info('*** Enabling STP on switch ' + switch.name + '\n')
		switch.cmd('ovs-vsctl set bridge ' + switch.name + ' stp_enable=true')
		
	#Working with CLI
	CLI(net)
	# Server: <server_host> iperf3 -s -p 1111 &
	# Client: <client_host> iperf3 -c <server_host> -p 1111 -t 120
	net.stop()

# Run the topology
if __name__ == '__main__':
	setLogLevel('info')
	buildTOPO()
	

