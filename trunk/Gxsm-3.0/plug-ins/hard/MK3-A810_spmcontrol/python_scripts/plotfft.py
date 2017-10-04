#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt

x=np.load ("mk3_recorder_xdata.npy")
y=np.load ("mk3_recorder_ydata.npy")
t=np.arange (1.*x.size)
t=t/150.

#fftx = abs (np.fft.rfft(x))
#ffty = abs (np.fft.rfft(y))
#freqs = np.fft.rfftfreq (x.size, t[1]-t[0])



#plt.semilogy (freqs, fftx, freqs, ffty)
plt.plot (t, y)
plt.title ('FFdT')
plt.grid (True)
plt.show ()
