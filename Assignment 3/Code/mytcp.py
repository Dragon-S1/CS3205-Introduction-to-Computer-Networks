import argparse
import math
import numpy as np
from random import random
from numpy import double

parser = argparse.ArgumentParser()

parser.add_argument("-i", metavar='<double>', help='values correspond to Ki', type=double, default=1)
parser.add_argument("-m", metavar='<double>', help='values correspond to Km', type=double, default=1)
parser.add_argument("-n", metavar='<double>', help='values correspond to Kn', type=double, default=1)
parser.add_argument("-f", metavar='<double>', help='values correspond to Kf', type=double, required=True)
parser.add_argument("-s", metavar='<double>', help='values correspond to Ps', type=double, required=True)
parser.add_argument("-T", metavar='<int>', help='total number of segments to be sent before the emulation stops', type=int, required=True)
parser.add_argument("-o", metavar='<string>', help='output filename', type=str, required=True)

args = parser.parse_args()

Ki = args.i		# 1 ≤ Ki ≤ 4, denotes the initial congestion window (CW)
Km = args.m		# 0.5 ≤ Km ≤ 2, denotes the multiplier of the Congestion Window, during the exponential growth phase.
Kn = args.n		# 0.5 ≤ Kn ≤ 2, denotes the multiplier of the Congestion Window, during the linear growth phase.
Kf = args.f		# 0.1 ≤ Kf ≤ 0.5, denotes the multiplier when a timeout occurs
Ps = args.s		# 0 < Ps < 1, denotes the probability of receiving the ACK packet for a given segment before its timeout occurs
T = args.T		# total number of segments to be sent before the emulation stops.
filename = args.o

RWS = 1024 	#KB
MSS = 1 	#KB
threshold = RWS/2
CW = Ki*MSS

slow_start = True

segments = 1

output = open(filename,'w')

while segments < T:
	N = math.ceil(CW/MSS)
	for i in range(N):
		output.write(f'{CW}\n')
		if segments >= T:
			break
		P = random()
		if P > Ps:
			threshold = CW/2
			CW = max(1, Kf*CW)
			if CW > threshold:
				slow_start = False
			else:
				slow_start = True
			segments += 1
			break
		if slow_start:
			CW = min(CW + Km*MSS, RWS)
			if CW > threshold:
				slow_start = False
		else:
			CW = min(CW + Kn*MSS*MSS/CW, RWS)
		segments += 1

output.close()