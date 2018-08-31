#!/usr/bin/env python

import numpy as np
import math
import matplotlib.pyplot as plt
from random import randint

Q22 = (1<<22)-1

class lms():
    def __init__(self, f0=1000., frq=125e6, tau=2e-6, iir_f=0.):
        self.a=0L
        self.b=0L
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.tau_pac=int(Q22*self.dt/tau)
        self.iirf = iir_f
        self.ss = 0L
        self.cc = 0L
        
    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (int(signal), int(Q22*math.sin (wt)), int(Q22*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time
        
# s,c signal 28bit in S5Q22
    def phasedetect (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        self.ss=int ((1.-self.iirf)*self.ss+self.iirf*s)
        self.cc=int ((1.-self.iirf)*self.cc+self.iirf*c)

        ci1t=s-self.ss  # DelayForRef (delayRef, s, &InputRef1[0]);
        cr1t=c-self.cc  # DelayForRef (delayRef, c, &InputRef2[0]);

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

fref=2e6
xx=1
ftest=fref*xx
tau=0.15
pac = lms(fref, 125e6, tau*1e-6)

iir    = 5e-4;

N=1000
t=np.arange (N).astype(np.float)
ts=np.arange (N).astype(np.float)
s=np.arange (N).astype(np.float)
m=np.arange (N).astype(np.float)
sdc=np.arange (N).astype(np.float)
a=np.arange (N).astype(np.float)
p=np.arange (N).astype(np.float)

for i in range (0,N):
    wt=i*pac.dt*math.pi*2.*ftest
    wt2=i*pac.dt*math.pi*2.*ftest2
    wt3=i*pac.dt*math.pi*2.*ftest3
    s[i]=0.2*math.sin(wt)+0.5*math.sin(wt2)+0.3*math.sin(wt3)+na*randint(-1000, 1000)*1e-3+0.2*math.sin(wt/200)
    sdc[i] = s[i]
    if i>1:
        sdc[i]=(1.-iir)*sdc[i-1]+iir*sdc[i]
    m[i] = s[i] - sdc[i]
    t[i]=i
    ts[i]=1e3*pac.run_step(m[i]*Q22)*tscale
    a[i]=pac.ampl()
    p[i]=pac.phase()

plt.xlabel('time in ms')
plt.plot (ts, s, label="signal")
plt.plot (ts, sdc, label="signal dc")
plt.plot (ts, m, label="signal-dc")
plt.plot (ts, a, label="ampl")
plt.plot (ts, p, label="phase")
plt.title ('PAC LMS Simulation tau: %g'%tau+'us f=%g'%fref+'Hz fs=x%g'%xx+'+x%g'%xx2+'+x%g'%xx3)
plt.grid (True)
plt.legend()
plt.show ()

#plt.xlabel('time in samples')
#plt.plot (t, s, label="signal")
#plt.plot (t, a, label="ampl")
#plt.plot (t, p, label="phase")
#plt.title ('PAC LMS Simulation')
#plt.grid (True)
#plt.legend()
#plt.show ()
