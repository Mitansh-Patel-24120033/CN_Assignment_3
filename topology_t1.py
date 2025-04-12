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
	#sh ovs-vsctl set Bridge <switch_name> stp_enable=true			For Enabling STP
	# Add switches
	s1 = net.addSwitch('s1')
	s2 = net.addSwitch('s2')
	s3 = net.addSwitch('s3')
	s4 = net.addSwitch('s4')

	# Add hosts
	h1 = net.addHost('h1', ip='10.0.0.2/24')
	h2 = net.addHost('h2', ip='10.0.0.3/24')
	h3 = net.addHost('h3', ip='10.0.0.4/24')
	h4 = net.addHost('h4', ip='10.0.0.5/24')
	h5 = net.addHost('h5', ip='10.0.0.6/24')
	h6 = net.addHost('h6', ip='10.0.0.7/24')
	h7 = net.addHost('h7', ip='10.0.0.8/24')
	h8 = net.addHost('h8', ip='10.0.0.9/24')

	# Connect switches to switches
	net.addLink(s1, s2, delay='7ms')
	net.addLink(s2, s3, delay='7ms')
	net.addLink(s3, s4, delay='7ms')
	net.addLink(s4, s1, delay='7ms')
	net.addLink(s1, s3, delay='7ms')

        
	# Connect hosts to switches
	net.addLink(h1, s1, delay='5ms')
	net.addLink(h2, s1, delay='5ms')
	net.addLink(h3, s2, delay='5ms')
	net.addLink(h4, s2, delay='5ms')
	net.addLink(h5, s3, delay='5ms')
	net.addLink(h6, s3, delay='5ms')
	net.addLink(h7, s4, delay='5ms')
	net.addLink(h8, s4, delay='5ms')

        
	net.start()
	#Working with CLI
	CLI(net)
	net.stop()

# Run the topology
if __name__ == '__main__':
	setLogLevel('info')
	buildTOPO()
	

