import math
import numpy as np
from random import random
import matplotlib.pyplot as plt

KI = [1,4]
KM = [1,1.5]
KN = [0.5,1]
KF = [0.1,0.3]
PS = [0.99, 0.9999]
dT = 10

RWS = 1024 	#KB
MSS = 1 	#KB

for Ki in KI:
	for Km in KM:
		for Kn in KN:
			for Kf in KF:
				for Ps in PS:
					T = dT * np.round((1.00-Ps)**-1)
					threshold = RWS/2
					slow_start = True
					segments = 1
					CW = Ki*MSS
					cw = []
					cw.append(CW)
					while segments < T:
						N = math.ceil(CW/MSS)
						for i in range(N):
							if segments >= T:
								break
							P = random()
							if P > Ps:
								threshold = CW/2
								CW = max(1, Kf*CW)
								slow_start = True
								segments += 1
								cw.append(CW)
								break
							if slow_start:
								CW = min(CW + Km*MSS, RWS)
								if CW > threshold:
									slow_start = False
							else:
								CW = min(CW + Kn*MSS*MSS/CW, RWS)
							segments += 1
							cw.append(CW)
					plt.xlabel('Number of updates')
					plt.ylabel('CW (KB)')
					plt.title(f'Ki = {Ki}, Km = {Km}, Kn = {Kn}, Kf = {Kf}, Ps = {Ps}, T = {int(T)}')
					plt.plot(np.array(range(1,segments+1)), cw)
					plt.savefig(f'../plots/{Ki}-Ki_{Km}-Km_{Kn}-Kn_{Kf}-Kf_{Ps}-Ps_{int(T)}-T.jpg')
					plt.clf()