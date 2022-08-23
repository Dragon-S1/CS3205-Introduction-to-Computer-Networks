import geocoder
from numpy import average
from haversine_distance import haversine
from pythonping import ping
import matplotlib.pyplot as plt
from scipy import constants
import math

# get the list of servers from given file
file = open("../assets/ping-servers.txt", "r")
data = file.read()
ping_servers = data.split("\n")

# get own ip and location
src = geocoder.ip('me')
rtt_avg = []
dists = []

# calculate distance and average rtt for each server
for ip in ping_servers:
	dest = geocoder.ip(ip)
	res = ping(ip, count=10, timeout = 1)
	dist = haversine(src.lng, src.lat, dest.lng, dest.lat)
	rtt = []
	for i in res:
		rtt.append(round(i.time_elapsed*1000, 2))
	rtt_avg.append(res.rtt_avg_ms)
	dists.append(dist)
	plt.scatter(res.rtt_avg_ms, dist)

# plot and save graph
plt.show()
plt.savefig('plot.png')

# get the round trip time for light
rtt_c = [round(d/constants.c*1000000, 2) for d in dists]

# print(dists)
# print(rtt_avg)
# print(rtt_c)

# get the factor by which light is faster
factor =[i / j for i, j in zip(rtt_avg, rtt_c)]
print(average(factor))