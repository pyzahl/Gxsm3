#!/usr/bin/env python

import numpy as np
import math
import matplotlib.pyplot as plt

Q22 = (1<<22)-1

class lms():
    def __init__(self, f0=1000., frq=125e6, tau=2e-6):
        self.a=0L
        self.b=0L
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.tau_pac=int(Q22*self.dt/tau)

    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (int(signal), int(Q22*math.sin (wt)), int(Q22*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time
        
# s,c signal 28bit in S5Q22
    def phasedetect (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        ci1t=s  # DelayForRef (delayRef, s, &InputRef1[0]);
        cr1t=c  # DelayForRef (delayRef, c, &InputRef2[0]);

        #// Compute the prediction

        temll = ci1t * self.a  #// Q22 * Q22 : Q44
        temll = temll + cr1t * self.b + 0x200000  #// Q22 * Q22 : Q44
        predict = temll >> 22 #// Q22

        #// Compute d_mu_e  

        errordet = signal-predict #// Q22 - Q22 : Q22
        temll = errordet * self.tau_pac + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22  #// Q22
        d_mu_e = temll;

        #// Compute LMS

        temll = cr1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22 #// Q22
        self.b = self.b+temll

        temll = ci1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22  #// Q22
        self.a = self.a+temll


    def ampl (self):
        return math.sqrt (self.a*self.a + self.b*self.b)/Q22

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)

fref=1000.0
xx=3
ftest=fref*xx
tau=1000
pac = lms(fref, 125e6, tau*1e-6)

#print pac.ampl(), pac.phase()

N=500000
t=np.arange (N).astype(np.float)
ts=np.arange (N).astype(np.float)
s=np.arange (N).astype(np.float)
a=np.arange (N).astype(np.float)
p=np.arange (N).astype(np.float)

#print t

for i in range (0,N):
    wt=i*pac.dt*math.pi*2.*ftest
    s[i]=0.1*math.sin(wt)
    t[i]=i
    ts[i]=1e3*pac.run_step(s[i]*Q22)
    a[i]=pac.ampl()
    p[i]=pac.phase()
    
    #print t[i], a[i], p[i]

plt.xlabel('time in ms')
plt.plot (ts, s, label="signal")
plt.plot (ts, a, label="ampl")
plt.plot (ts, p, label="phase")
plt.title ('PAC LMS Simulation tau: %g'%tau+'us f=%g'%fref+'Hz fs=x%g'%xx)
plt.grid (True)
plt.legend()
plt.show ()

plt.xlabel('time in samples')
plt.plot (t, s, label="signal")
plt.plot (t, a, label="ampl")
plt.plot (t, p, label="phase")
plt.title ('PAC LMS Simulation')
plt.grid (True)
plt.legend()
plt.show ()
