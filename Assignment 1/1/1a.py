import geocoder
from haversine_distance import haversine

# get the list of servers from given file
file = open("../assets/ping-servers.txt", "r")
data = file.read()
ping_servers = data.split("\n")

# get own ip and location
src = geocoder.ip('me')
dist = []

# get the distance of each server
for ip in ping_servers:
	dest = geocoder.ip(ip)
	dist.append(haversine(src.lng, src.lat, dest.lng, dest.lat))

# OUTPUT
output = open("distance.txt", "w")

for d in dist:
	output.write(f"{d}\n")