#!/usr/bin/env python
import collections
import numpy as np
import math
import matplotlib.pyplot as plt
from random import randint

Q22 = (1<<22)-1

class lockin():
    def __init__(self, f0=1000., frq=125e6, tau=2e-6, iir_f=0., navg=1):
        self.a=0L
        self.b=0L
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.tau_lp=int(Q22*self.dt/tau)
        self.iirf = iir_f
        self.ss = 0L
        self.cc = 0L
        self.n = navg*frq/f0
        self.corrx=np.arange (self.n).astype(np.float)
        self.corry=np.arange (self.n).astype(np.float)
        self.corri=0
        self.sumcorrx=0.0
        self.sumcorry=0.0
        
    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (int(Q22*signal), int(Q22*math.sin (wt)), int(Q22*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time

    def filter (self, old, x):
        q = Q22 - self.tau_lp
        return int(old * q + x * self.tau_lp + 0x200000)>>22
    
# s,c signal 28bit in S5Q22
    def phasedetect (self, signal, s, c):
        x = (signal*s + 0x200000) >> 22
        y = (signal*c + 0x200000) >> 22
        self.sumcorrx = self.sumcorrx + x - self.corrx[self.corri]
        self.sumcorry = self.sumcorry + y - self.corry[self.corri]
        self.corrx[self.corri] = x
        self.corry[self.corri] = y
        self.corri = self.corri+1
        if self.corri >= self.n:
            self.corri=0
        
        #self.a = self.filter(self.a, x)
        #self.b = self.filter(self.b, y)
        self.a = self.filter(self.a, self.sumcorrx)
        self.b = self.filter(self.b, self.sumcorry)
        
    def x (self):
        return self.a/Q22

    def y (self):
        return self.b/Q22

    def ampl (self):
        return 2.0*math.sqrt (self.a*self.a + self.b*self.b)/Q22/self.n

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)

    def campl (self):
        return 2.0*math.sqrt (self.sumcorrx*self.sumcorrx + self.sumcorry*self.sumcorry)/Q22/self.n

    def cphase (self):
        return math.atan2 (self.sumcorrx-self.sumcorry, self.sumcorrx+self.sumcorry)



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
        self.errordet=0.0
        self.predict=0.0
        self.delayc = collections.deque(maxlen=5)
        self.delays = collections.deque(maxlen=5)
        
    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (int(Q22*signal), int(Q22*math.sin (wt)), int(Q22*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time
    
# s,c signal 28bit in S5Q22
    def phasedetect (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        #self.ss=int ((1.-self.iirf)*self.ss+self.iirf*s)
        #self.cc=int ((1.-self.iirf)*self.cc+self.iirf*c)

        self.delayc.append(c)
        self.delays.append(s)

        ci1t = s #self.delays[0] #-self.ss  # DelayForRef (delayRef, s, &InputRef1[0]);
        cr1t = c #self.delayc[0] #-self.cc  # DelayForRef (delayRef, c, &InputRef2[0]);

        #// Compute the prediction

        temll = ci1t * self.a  #// Q22 * Q22 : Q44
        temll = temll + cr1t * self.b + 0x200000  #// Q22 * Q22 : Q44
        self.predict = temll >> 22 #// Q22

        #// Compute d_mu_e  

        self.errordet = signal- self.predict #// Q22 - Q22 : Q22
        temll =  self.errordet * self.tau_pac + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22  #// Q22
        d_mu_e = temll;

        #// Compute LMS

        temll = cr1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22 #// Q22
        self.b = self.b+temll

        temll = ci1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        temll = temll >> 22  #// Q22
        self.a = self.a+temll

    def lerror (self):
        return float(self.errordet)/Q22

    def lpredict (self):
        return float(self.predict)/Q22

    def ampl (self):
        return math.sqrt (self.a*self.a + self.b*self.b)/Q22

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)

class lms_f():
    def __init__(self, f0=1000., frq=125e6, tau=2e-6, iir_f=0.):
        self.a=0.0
        self.b=0.0
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.mu=self.dt/tau
        self.iirf = iir_f
        self.errordet=0.0
        self.predict=0.0
        
    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (signal, math.sin (wt), math.cos (wt))
        self.time = self.time+self.dt
        return self.time
        
    def phasedetect (self, signal, s, c):
        # Compute the prediction
        self.predict = s * self.a  +  c * self.b

        # Compute d_mu_e  

        self.errordet = signal- self.predict
        d_mu_e = self.errordet * self.mu

        # Compute LMS

        self.b = self.b + c * d_mu_e
        self.a = self.a + s * d_mu_e

    def lerror (self):
        return self.errordet

    def lpredict (self):
        return self.predict

    def ampl (self):
        return math.sqrt (self.a*self.a + self.b*self.b)

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)



fsample=124e6
fref=2e6
xx=1
xx2=2.0
xx3=2.4
ftest=fref*xx
ftest2=fref*xx2
ftest3=fref*xx3
tau0=0.055 #0.15 # 0.15
taulck=0.01
a1=0.2
a2=0.0
a3=0.0
na=0.0
tscale=1
iir    = 5e-4;

N=1000*tscale
M=1
tau=tau0

t=np.arange (N).astype(np.float)
ts=np.arange (N).astype(np.float)
s=np.arange (N).astype(np.float)
m=np.arange (N).astype(np.float)
sdc=np.arange (N).astype(np.float)
a=np.arange (N).astype(np.float)
p=np.arange (N).astype(np.float)
la=np.arange (N).astype(np.float)
lp=np.arange (N).astype(np.float)
x=np.arange (N).astype(np.float)
y=np.arange (N).astype(np.float)
lmse=np.arange (N).astype(np.float)
lmsp=np.arange (N).astype(np.float)


ar=np.arange (M).astype(np.float)
lar=np.arange (M).astype(np.float)
f=np.arange (M).astype(np.float)

fsweep=False
tausweep=True

for k in range (0,M):
    ftest = fref

    if fsweep:
        pac = lms(fref, fsample, tau*1e-6)
        lck = lockin(fref, fsample, tau*1e-6)
        ff = float(k)/M
        ff = pow(10., ff*2.5-2)
        ftest = ff*fref
        f[k]=ff

    if tausweep:
        tau = tau0 + k*0.002
        
    if k == M-1:
        ftest=fref
        tau = tau0

    pac = lms_f(fref,  fsample, tau*1e-6)
    lck = lockin(fref,  fsample, taulck*1e-6)
    sdphi=0.0
    sdamp=0.0
    for i in range (0,N):
        #if i == 500:
        #    pac.dt = pac.dt*1.1
        #    lck.dt = lck.dt*1.1
        if i>300 and i < 310:
            sdamp=sdamp+0.1/10.

        if i>400 and i < 410:
            sdphi=sdphi+math.pi/90.

        wt=i*pac.dt*math.pi*2.*ftest
        wt2=i*pac.dt*math.pi*2.*ftest2
        wt3=i*pac.dt*math.pi*2.*ftest3
        s[i]=(a1+sdamp)*math.sin(wt+sdphi) + a2*math.sin(wt2) + a3*math.sin(wt3) + na*randint(-1000, 1000)*1e-3 #+0.2*math.sin(wt/200)
        sdc[i] = s[i]
        if i>1:
            sdc[i]=(1.-iir)*sdc[i-1]+iir*sdc[i]
        m[i] = s[i] #- sdc[i]
        t[i]=i
        ts[i]=1e3*pac.run_step(m[i])*tscale
        a[i]=pac.ampl()
        p[i]=pac.phase()
        lmse[i]=pac.lerror()
        lmsp[i]=pac.lpredict()
        lck.run_step(m[i])*tscale
        la[i]=lck.ampl()
        lp[i]=lck.phase()
        x[i]=lck.x()
        y[i]=lck.y()

    ar[k] = a[N-1]
    lar[k] = la[N-1]
    if tausweep and k != M-1:
        plt.plot (ts, a, label="LMS-ampl tau=%g"%tau)
        plt.plot (ts, p, label="LMS-phase tau=%g"%tau)
 
a[0]=0
p[0]=0
la[0]=0
lp[0]=0
a[1]=0
p[1]=0
la[1]=0
lp[1]=0
a[2]=0
p[2]=0
la[2]=0
lp[2]=0

plt.xlabel('time in ms')
plt.plot (ts, s, label="signal")
#plt.plot (ts, sdc, label="signal dc")
#plt.plot (ts, m, label="signal-dc")
plt.plot (ts, lmsp, label="LMS-predict")
#plt.plot (ts, lmse, label="LMS-error")
plt.plot (ts, a, label="LMS-ampl")
plt.plot (ts, p, label="LMS-phase")
plt.plot (ts, la, label="LCK-ampl")
plt.plot (ts, lp, label="LCK-phase")
#plt.plot (ts, x, label="LCK-X")
#plt.plot (ts, y, label="LCK-Y")
#plt.title ('PAConvergence LMS vs. LockIn Simulation (124 MHz sampling) tau: %g'%tau+'us f=%g'%(fref*1e-6)+'MHz fs=x%g'%xx+'+x%g'%xx2+'+x%g'%xx3)
plt.title ('PAConvergence LMS vs. LockIn Simulation (124 MHz sampling) tau: %g'%tau+'us f=%g'%(fref*1e-6)+'MHz')
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

if fsweep:
    plt.xlabel('Freq in Hz x F-ref')
    plt.plot (f, ar, label="LMS")
    plt.plot (f, lar, label="LockIn")
    #plt.plot (t, p, label="phase")
    #plt.title ('PAC LMS Simulation')
    plt.grid (True)
    plt.legend()
    plt.show ()
