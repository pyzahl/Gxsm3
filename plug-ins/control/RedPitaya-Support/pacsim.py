#!/usr/bin/env python

import numpy as np
import math
import matplotlib.pyplot as plt
from random import randint

Q22 = (1<<22)-1
Q24 = (1<<24)-1

Q22H = (1<<21)
Q24H = (1<<23)

NQLMS = 22
NQSC  = 24

QLMS = Q22
QSC  = Q24

LMSQH = Q22H
SCQH = Q24H


class lms():
    def __init__(self, f0=30000., frq=125e6, tau=2e-6, iir_f=0.):
        self.pipe_len=5
        self.a=0L
        self.b=0L
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.tau_pac=int(QLMS*self.dt/tau)
        self.iirf = iir_f
        self.ss = 0L
        self.cc = 0L

        self.s0 = 0L
        self.s1 = 0L
        self.s2 = 0L
        self.s3 = 0L
        self.c0 = 0L
        self.c1 = 0L
        self.c2 = 0L
        self.c3 = 0L
        self.c4 = 0L
        self.s1 = 0L

        self.sa_1 = 0L
        self.cb_1 = 0L

        self.predict_err_2 = 0L
        
        self.d_mu_e_3 = 0L

        self.Cd_mu_e_4 = 0L
        self.Sd_mu_e_4 = 0L
        
        self.signal_1 = 0L

    def phase_delay(self):
        return self.pipe_len, self.dt * self.pipe_len * math.pi*2.*self.fref
        
    def run_step(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (int(signal), int(QSC*math.sin (wt)), int(QSC*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time
        
    def run_step_pipe(self, signal):
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect_pipe (int(signal), int(QSC*math.sin (wt)), int(QSC*math.cos (wt)))
        self.time = self.time+self.dt
        return self.time
        
# s,c signal in QSC
    def phasedetect (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        self.ss=int ((1.-self.iirf)*self.ss+self.iirf*s)
        self.cc=int ((1.-self.iirf)*self.cc+self.iirf*c)

        S=s-self.ss  # DelayForRef (delayRef, s, &InputRef1[0]);
        C=c-self.cc  # DelayForRef (delayRef, c, &InputRef2[0]);

        #// Compute the prediction error
        sa = S * self.a
        cb = C * self.b
        predict_err = signal - (( sa + cb + SCQH ) >> NQSC)

        #// Compute d_mu_e  
        d_mu_e = ( predict_err * self.tau_pac + LMSQH ) >> NQLMS

        #// Compute LMS
        self.b = self.b + (( C * d_mu_e + SCQH ) >> NQSC)
        self.a = self.a + (( S * d_mu_e + SCQH ) >> NQSC)

        return predict_err
        
    def phasedetect_pipe (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        self.ss=int ((1.-self.iirf)*self.ss+self.iirf*s)
        self.cc=int ((1.-self.iirf)*self.cc+self.iirf*c)

        # simulate piplined algorithm
 
        ### 5
        #// Compute LMS
        self.b = self.b + ((self.Cd_mu_e_4 + SCQH ) >> NQSC)
        self.a = self.a + ((self.Sd_mu_e_4 + SCQH ) >> NQSC)


        ### 4
        self.Cd_mu_e_4 = self.c3 * (self.d_mu_e_3 >> NQLMS)
        self.Sd_mu_e_4 = self.s3 * (self.d_mu_e_3 >> NQLMS)
        
        ### 3
        #// Compute d_mu_e  
        self.d_mu_e_3 = self.predict_err_2 * self.tau_pac #+ LMSQH

        ### 2
        self.predict_err_2 = self.signal_1 - (( self.sa_1 + self.cb_1 + SCQH ) >> NQSC)

        ### 1
        #// Compute the prediction error
        self.sa_1 = self.s0 * self.a
        self.cb_1 = self.c0 * self.b


        #// simulate pipline
        self.c3 = self.c2;
        self.c2 = self.c1;
        self.c1 = self.c0;
        self.c0 = c-self.cc  # DelayForRef (delayRef, c, &InputRef2[0]);

        self.s3 = self.s2;
        self.s2 = self.s1;
        self.s1 = self.s0;
        self.s0 = s-self.ss  # DelayForRef (delayRef, s, &InputRef1[0]);

        self.signal_1 = signal
        
        return self.predict_err_2

    def ampl (self):
        return math.sqrt (self.a*self.a + self.b*self.b)/QLMS

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)

fref=330e3
tscale=10
xx=1
xx2=1.8
xx3=2.4
ftest=fref*xx
ftest2=fref*xx2
ftest3=fref*xx3
tau=2.5  #0.15
na=0.01
pac = lms(fref, 125e6, tau*1e-6)

dc_error=10*1e-3

iir    = 5e-3;

N=1000*tscale
t=np.zeros (N).astype(np.float)
ts=np.zeros (N).astype(np.float)
s=np.zeros (N).astype(np.float)
m=np.zeros (N).astype(np.float)
sdc=np.zeros (N).astype(np.float)
a=np.zeros (N).astype(np.float)
p=np.zeros (N).astype(np.float)
s=np.zeros (N).astype(np.float)

S=0.0
for i in range (0,N):
    dphi=pac.dt*math.pi*2.*ftest
    wt=i*dphi
    wt2=i*pac.dt*math.pi*2.*ftest2
    wt3=i*pac.dt*math.pi*2.*ftest3
    s[i]=0.2*math.sin(wt) #+0.5*math.sin(wt2) #+0.3*math.sin(wt3)+na*randint(-1000, 1000)*1e-3 #+0.2*math.sin(wt/200)
    sdc[i] = s[i]
    if i>1:
        sdc[i]=(1.-iir)*sdc[i-1]+iir*sdc[i]
    m[i] = s[i] + dc_error #- sdc[i]

    S = S + dphi*m[i]
    s[i]=S
    
    t[i]=i
    ts[i]=1e3*pac.run_step(m[i]*QLMS)*tscale
    a[i]=pac.ampl()
    p[i]=pac.phase()

plt.xlabel('time in ms')
#plt.plot (ts, s, label="signal")
#plt.plot (ts, sdc, label="signal dc")
plt.plot (ts, m, label="signal-dc")
plt.plot (ts, s, label="S")
plt.plot (ts, a, label="ampl")
plt.plot (ts, p, label="phase")



pac = lms(fref, 125e6, tau*1e-6)

iir    = 5e-3;

t=np.zeros (N).astype(np.float)
ts=np.zeros (N).astype(np.float)
s=np.zeros (N).astype(np.float)
m=np.zeros (N).astype(np.float)
sdc=np.zeros (N).astype(np.float)
a=np.zeros (N).astype(np.float)
p=np.zeros (N).astype(np.float)

for i in range (0,N):
    wt=i*pac.dt*math.pi*2.*ftest
    wt2=i*pac.dt*math.pi*2.*ftest2
    wt3=i*pac.dt*math.pi*2.*ftest3
    s[i]=0.2*math.sin(wt) #+0.5*math.sin(wt2) #+0.3*math.sin(wt3)+na*randint(-1000, 1000)*1e-3 #+0.2*math.sin(wt/200)
    sdc[i] = s[i]
    if i>1:
        sdc[i]=(1.-iir)*sdc[i-1]+iir*sdc[i]
    m[i] = s[i] + dc_error # - sdc[i]
    t[i]=i
    ts[i]=1e3*pac.run_step_pipe(m[i]*QLMS)*tscale
    a[i]=pac.ampl()
    p[i]=pac.phase()

plt.xlabel('time in ms')
plt.plot (ts, a, label="ampl-pipe")
plt.plot (ts, p, label="phase-pipe")

print(pac.phase_delay())

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
