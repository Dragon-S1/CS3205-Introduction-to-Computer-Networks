from pythonping import ping
import geocoder

# get the list of servers from given file
file = open("../assets/ping-servers.txt", "r")
data = file.read()
ping_servers = data.split("\n")

# get own ip and location
src = geocoder.ip('me')
rtts = []

# ping and get rtt for each server 10 times
for ip in ping_servers:
	res = ping(ip, count=10, timeout=1)
	rtt = []
	for i in res:
		rtt.append(round(i.time_elapsed*1000, 2))
	rtts.append(rtt)

# OUTPUT
output = open("rtt.txt", "w")
output.write("<YOUR_LOC>\t\t\t\t\t\t<SERVER1>\t\t\t\t\t\t<SERVER_LOC>\t\t\t\t\t\t<RTT1, RTT2, ... RTT10>\n\n")

for i in range(len(ping_servers)):
	dest = geocoder.ip(ping_servers[i])
	output.write(f"{src.latlng}\t\t{ping_servers[i]}\t\t{dest.latlng}\t\t{rtts[i]}\n")