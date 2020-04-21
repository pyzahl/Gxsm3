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
        self.a2=0L
        self.b2=0L
        self.a3=0L
        self.b3=0L
        self.a4=0L
        self.b4=0L
        self.A0=0.
        self.A1=0.
        self.A2=0.
        self.A3=0.
        self.A4=0.
        self.P0=0.
        self.P1=0.
        self.P2=0.
        self.P3=0.
        self.P4=0.
        self.time=0.0
        self.dt=1.0/frq
        self.fref=f0
        self.tau_lp=int(Q22*self.dt/tau)
        self.iirf = iir_f
        self.ss = 0L
        self.cc = 0L
        self.n = int(10*navg*frq/f0)
        self.corrx=np.arange (self.n).astype(np.float)
        self.corry=np.arange (self.n).astype(np.float)
        self.corrdphi=np.arange (self.n).astype(np.float)
        self.corri=0
        self.sumcorrx=0.0
        self.sumcorry=0.0
        self.sumdphi=0.0
        self.corrlen=0
        self.phi=0.0
        self.NN=0
        
    def dds_adjust(self, f0):
        self.fref=f0
        
    def circ(self, i):
        if (i < 0):
            return i+self.n
        if (i >= self.n):
            return i-self.n
        return  i
        
    def run_step(self, signal):
        dphi=self.dt*math.pi*2.*self.fref
        self.phi += dphi
        #wt=self.time*math.pi*2.*self.fref
        #self.phasedetect_prod (int(Q22*signal), int(Q22*math.sin(self.phi)), int(Q22*math.cos (self.phi)))
        #self.phasedetect_lp (int(Q22*signal), int(Q22*math.sin (self.phi)), int(Q22*math.cos (self.phi)))
        #self.phasedetect (int(Q22*signal), int(Q22*math.sin (self.phi)), int(Q22*math.cos (self.phi)))
        self.phasedetect_dphi (int(Q22*signal), int(Q22*math.sin (self.phi)), int(Q22*math.cos (self.phi)), dphi)
        self.time = self.time+self.dt
        #self.p()
        return self.time, self.phi, dphi

    def filter (self, old, x):
        q = Q22 - self.tau_lp
        return int(old * q + x * self.tau_lp + 0x200000)>>22

    def filterf (self, old, x):
        q = Q22 - self.tau_lp
        return (old * q + x * self.tau_lp)/Q22
    
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
        
    def phasedetect_prod (self, signal, s, c):
        self.sumdphi=1.0
        x = (signal*s + 0x200000) >> 22
        y = (signal*c + 0x200000) >> 22
        self.a = x
        self.b = y
        
    def phasedetect_lp (self, signal, s, c):
        self.sumdphi=1.0
        x = (signal*s + 0x200000) >> 22
        y = (signal*c + 0x200000) >> 22
        self.a4 = self.filter(self.a4, x)
        self.b4 = self.filter(self.b4, y)
        self.a3 = self.filter(self.a3, self.a4)
        self.b3 = self.filter(self.b3, self.b4)
        self.a2 = self.filter(self.a2, self.a3)
        self.b2 = self.filter(self.b2, self.b3)
        self.a = self.filter(self.a, self.a2)
        self.b = self.filter(self.b, self.b2)

    #localparam integer PH_WIDTH_CUT       = 10;               // reduce max F by 1024
    #localparam integer PH_SHIFT           = 14;               // 12 ;   14 fits!
    #localparam integer LCK_DPH_WIDTH      = DPHASE_WIDTH - PH_WIDTH_CUT; // 125MHz / 256 max
    #localparam integer LCK_INT_PH_WIDTH   = DPHASE_WIDTH + 1; // 0 .. 2pi (plus need to fit 2pi+dPhi
    #localparam integer LCK_INT_WIDTH      = LMS_DATA_WIDTH + DPHASE_WIDTH - PH_SHIFT; // integral over sine should not exceed 1
    #localparam integer LCK_NORM           = DPHASE_WIDTH-PH_SHIFT-PH_WIDTH_CUT; // "2pi" is the norm, splitup  
       
    def phasedetect_dphi (self, signal, s, c, dphi):
        self.NN += 1
        x = (signal*s + 0x200000) >> 22
        y = (signal*c + 0x200000) >> 22

        if 0: # classic
            self.sumdphi  += dphi
            self.sumcorrx += x*dphi
            self.sumcorry += y*dphi

            k=0
            # adjust interval
            while (self.sumdphi > 2*math.pi+dphi/2):
                k += 1
                self.sumdphi  -= self.corrdphi[self.circ(self.corri-self.corrlen)];
                self.sumcorrx -= self.corrx   [self.circ(self.corri-self.corrlen)];
                self.sumcorry -= self.corry   [self.circ(self.corri-self.corrlen)];
                if k>1:
                    print ("AI", self.NN, k, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi, self.campl(), self.cphase() )
                    self.corrlen -= 1;
                    if k==0 and self.NN>100:
                        print ("AI", self.NN, k, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi, self.campl(), self.cphase() )
            self.corrlen += 1;
        else: # unrolled version for "one shot" FPGA impl
            print ("AI:", self.NN, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi, dphi, math.log(dphi/2/math.pi*(1<<44))/(math.log(2)))
            if (self.sumdphi + dphi - self.corrdphi[self.circ(self.corri-self.corrlen)] >= 2*math.pi + dphi/2):
                print ("AI-", self.NN, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi) #, self.campl(), self.cphase() )
                self.sumdphi  +=   dphi - self.corrdphi[self.circ(self.corri-self.corrlen)] - self.corrdphi[self.circ(self.corri-self.corrlen+1)]
                self.sumcorrx += x*dphi - self.corrx   [self.circ(self.corri-self.corrlen)] - self.corrx   [self.circ(self.corri-self.corrlen+1)]
                self.sumcorry += y*dphi - self.corry   [self.circ(self.corri-self.corrlen)] - self.corry   [self.circ(self.corri-self.corrlen+1)]
                self.corrlen  -= 1; # shorter
            elif (self.sumdphi + dphi >= 2*math.pi + dphi/2):
                print ("AI=", self.NN, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi, dphi, math.log(dphi/2/math.pi*(1<<44))/(math.log(2))) #, self.campl(), self.cphase() )
                self.sumdphi  +=   dphi - self.corrdphi[self.circ(self.corri-self.corrlen)]
                self.sumcorrx += x*dphi - self.corrx   [self.circ(self.corri-self.corrlen)]
                self.sumcorry += y*dphi - self.corry   [self.circ(self.corri-self.corrlen)]
                # self.corrlen unchanged
            else:
                print ("AI+", self.NN, self.corrlen, self.corri, self.circ(self.corri-self.corrlen),  self.sumdphi) #, self.campl(), self.cphase() )
                self.sumdphi  +=   dphi
                self.sumcorrx += x*dphi
                self.sumcorry += y*dphi
                self.corrlen  += 1; # longer
                
        self.corrdphi[self.corri] =   dphi;
        self.corrx   [self.corri] = x*dphi;
        self.corry   [self.corri] = y*dphi;
        self.corri   += 1

        if self.corri >= self.n:
            self.corri=0
        
        #self.a4 = self.filter(self.a4, self.sumcorrx)
        #self.b4 = self.filter(self.b4, self.sumcorry)
        #self.a3 = self.filter(self.a3, self.a4)
        #self.b3 = self.filter(self.b3, self.b4)
        #self.a2 = self.filter(self.a2, self.a3)
        #self.b2 = self.filter(self.b2, self.b3)
        #self.a = self.filter(self.a, self.a4)
        #self.b = self.filter(self.b, self.b4)
        #self.a = self.filter(self.a, self.sumcorrx)
        #self.b = self.filter(self.b, self.sumcorry)
        self.a = self.sumcorrx
        self.b = self.sumcorry
        
    def x (self):
        return self.a/Q22

    def y (self):
        return self.b/Q22

    def ampl_norm (self):
        return 2.0*math.sqrt (self.a*self.a + self.b*self.b)/Q22/self.n

    def ampl (self):
        return 2.0*math.sqrt (self.a*self.a + self.b*self.b)/Q22/(2*math.pi)

    def ampl_lp (self):
        self.A0 = self.filterf(self.A0, self.ampl())
        #self.A1 = self.filterf(self.A1, self.A0)
        self.A2 = self.filterf(self.A2, self.A0)
        self.A3 = self.filterf(self.A3, self.A2)
        self.A4 = self.filterf(self.A4, self.A3)
        return self.A4

    def phase_lp (self):
        self.P0 = self.filterf(self.P0, self.phase())
        #self.P1 = self.filterf(self.P1, self.P0)
        self.P2 = self.filterf(self.P2, self.P0)
        self.P3 = self.filterf(self.P3, self.P2)
        self.P4 = self.filterf(self.P4, self.P3)
        return self.P4

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)

    def campl_norm (self):
        return 2.0*math.sqrt (self.sumcorrx*self.sumcorrx + self.sumcorry*self.sumcorry)/Q22/self.n

    def campl (self):
        return 2.0*math.sqrt (self.sumcorrx*self.sumcorrx + self.sumcorry*self.sumcorry)/Q22/(2*math.pi)

    def cphase (self):
        return math.atan2 (self.sumcorrx-self.sumcorry, self.sumcorrx+self.sumcorry)

    def p(self):
        print(self.corri, self.corrlen, self.sumdphi)
    

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
        
    def dds_adjust(self, f0):
        self.fref=f0
        
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
        self.phi=0.0
        
    def dds_adjust(self, f0):
        self.fref=f0
        
    def run_step(self, signal):
        dphi=self.dt*math.pi*2.*self.fref
        self.phi += dphi
        wt=self.time*math.pi*2.*self.fref
        self.phasedetect (signal, math.sin (self.phi), math.cos (self.phi))
        self.time = self.time+self.dt
        return self.time, self.phi, dphi
        
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



fsample=125e6
fref=30e3
frefA=fref
xx=1
xx2=2.0
xx3=2.4
ftest=fref*xx
ftest2=fref*xx2
ftest3=fref*xx3
tau0=0.055 #0.15 # 0.15
taulck=0.01
taulck=0.2
a1=0.2
a2=0.0
a3=0.0
na=0.0
tscale=10
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

phi_signal=0.

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
    [tmpt, phi, dphi] = pac.run_step(0)
    sdphi=0.0
    sdamp=0.0
    dphi_err=0.0
    fint = fref;
    for i in range (0,N):
        #if i == 500:
        #    pac.dt = pac.dt*1.1
        #    lck.dt = lck.dt*1.1
        if i>300 and i < 310:
            sdamp=sdamp+0.1/10.

        if i>400 and i < 415:
            sdphi=sdphi-math.pi/90.

        if i>500 and i < 550:
            dphi += 0.5*2*math.pi/100

        if i == 550:
            dphi += 12*2*math.pi/100

        if i == 600:
            dphi -= 12*2*math.pi/100

            
        if i>700 and i < 788:
            dphi += 0.5*2*math.pi/100
            
        if i>900 and i < 995:
            dphi -= 0.5*2*math.pi/100
            
        phi_signal += dphi
        wt2=i*pac.dt*math.pi*2.*ftest2
        wt3=i*pac.dt*math.pi*2.*ftest3
        s[i]=(a1+sdamp)*math.sin(phi_signal+sdphi) + a2*math.sin(wt2) + a3*math.sin(wt3) + na*randint(-1000, 1000)*1e-3 #+0.2*math.sin(wt/200)
        sdc[i] = s[i]
        if i>1:
            sdc[i]=(1.-iir)*sdc[i-1]+iir*sdc[i]
        m[i] = s[i] #- sdc[i]
        t[i]=i
        [tmpt, phi, dphi] = pac.run_step(m[i])
        ts[i] =1e3*tmpt*tscale
        a[i]=pac.ampl()
        p[i]=pac.phase()
        lmse[i]=pac.lerror()
        lmsp[i]=pac.lpredict()
        lck.run_step(m[i])*tscale
        la[i]=lck.ampl()
        lp[i]=lck.phase()
        x[i]=lck.x()
        y[i]=lck.y()

        ## PLL
        if (i>500):
            dphi_err = p[i] - 0.56
            fint += dphi_err*(-280);
            fref = fint + dphi_err*(-550);
            lck.dds_adjust(fref)
            pac.dds_adjust(fref)
        

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
plt.title ('PAConvergence LMS vs. LockIn Simulation (124 MHz sampling) tau: %g'%tau+'us Lck: %g'%taulck+'us f=%g'%(frefA*1e-6)+'MHz')
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
