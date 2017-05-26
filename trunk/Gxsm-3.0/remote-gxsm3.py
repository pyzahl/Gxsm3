import array
#import Numeric
import math
import numpy as np
from numpy.random import shuffle
import os

# wait for VP finished and check/return Motor value/abort cond.
def wait_for_vp ():
        vpaction = 1
        M = 0.
        while vpaction > 0 and M > -2.:
	        gxsm.sleep (10) # sleep 10/10 sec
 		M=gxsm.get ("dsp-fbs-motor")
 	      	svec=gxsm.rtquery ("s")
		s = int(svec[0])
		vpaction = s&8
	        if os.path.exists("remote.py-stop"):
	        	M = -10.
			break
	print "FB: ", s&1, " Scan: ", s&(2+4), "  VP: ", s&8, "   Mov: ", s&16, " PLL: ", s&32, " **Motor=", M
	gxsm.sleep(20)
	return M

def wait_scan_pos():
        scaction = 1
        M = 0
        while scaction > 0 and M > -2.:
	        gxsm.sleep (10) # sleep 10/10 sec
 		M=gxsm.get ("dsp-fbs-motor")
 	      	svec=gxsm.rtquery ("s")
		s = int(svec[0])
		scaction = s&(2+4)
	        if os.path.exists("remote.py-stop"):
	        	M = -10.
			break
	print "FB: ", s&1, " Scan: ", s&(2+4), "  VP: ", s&8, "   Mov: ", s&16, " PLL: ", s&32, " **Motor=", M
	gxsm.sleep(20)
	return M

def wait_move_pos():
        scaction = 1
        M = 0
        while scaction > 0 and M > -2.:
	        gxsm.sleep (10) # sleep 10/10 sec
 		M=gxsm.get ("dsp-fbs-motor")
 	      	svec=gxsm.rtquery ("s")
		s = int(svec[0])
		scaction = s&16
	        if os.path.exists("remote.py-stop"):
	        	M = -10.
			break
	print "FB: ", s&1, " Scan: ", s&(2+4), "  VP: ", s&8, "   Mov: ", s&16, " PLL: ", s&32, " **Motor=", M
	gxsm.sleep(20)
	return M

# wait for scan finish or abort
def wait_for_scan ():
        scaction = 1
        M = 0.
        while scaction > 0 and M > -2.:
	        gxsm.sleep (10) # sleep 10/10 sec
 		M=gxsm.get ("dsp-fbs-motor")
 	      	svec=gxsm.rtquery ("s")
		s = int(svec[0])
		scaction = s&(2+4)	
	        if os.path.exists("remote.py-stop"):
	        	M = -10.
			breaksp
	print "FB: ", s&1, " Scan: ", s&(2+4), "  VP: ", s&8, "   Mov: ", s&16, " PLL: ", s&32, " **Motor=", M
	gxsm.sleep(20)
	return M


# store current CP, CI
bias=gxsm.get ("dsp-fbs-bias")
motor=gxsm.get ("dsp-fbs-motor")
current=gxsm.get ("dsp-fbs-mx0-current-set")
cp=gxsm.get ("dsp-fbs-cp")
ci=gxsm.get ("dsp-fbs-ci")
print "CP,CI=", cp, ci

start_x0=gxsm.get ("OffsetX")
start_y0=gxsm.get ("OffsetY")


# ref point (0,0)
def goto_refpoint():
	gxsm.set ("ScanX","0")
        gxsm.set ("ScanY","0")
#	time.sleep(1)
	gxsm.sleep(40)

def goto_point(x,y, s=40):
	xd=x+0.1
	gxsm.set ("ScanX","%f"%xd)
	gxsm.set ("ScanX","%f"%x)
        gxsm.set ("ScanY","%f"%y)
	gxsm.sleep(s)

def freeze_Z():
	gxsm.set ("dsp-fbs-cp","0")
        gxsm.set ("dsp-fbs-ci","0")


def const_height_slow_Z(cich=0.25):
	gxsm.set ("dsp-fbs-cp","0")
        gxsm.set ("dsp-fbs-ci","%f"%(cich))

def release_Z(s=40):
	gxsm.set ("dsp-fbs-cp","%f"%(cp))
        gxsm.set ("dsp-fbs-ci","%f"%(ci))
        gxsm.sleep (s)

def pause_tip_retract():
	M = -1;
	current=gxsm.get ("dsp-fbs-mx0-current-set")
	gxsm.set ("dsp-fbs-mx0-current-set","0")
        while M < -0.9:
	        gxsm.sleep (10) # sleep 10/10 sec
 		M=gxsm.get ("dsp-fbs-motor")
	gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current/2.))
	gxsm.sleep (600) # sleep 600/10 sec
	gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current))
	gxsm.sleep (100) # sleep 100/10 sec


def run_ref_image (u,c=0.):
	bias=gxsm.get ("dsp-fbs-bias")
	current=gxsm.get ("dsp-fbs-mx0-current-set")
	print "Ref Image, Bias=", u
	gxsm.set ("dsp-fbs-bias","%f"%(u))
	if c > 0.:
		gxsm.set ("dsp-fbs-mx0-current-set","%f"%(c))
	gxsm.sleep (10)
	gxsm.startscan ()
	gxsm.waitscan ()

	if wait_for_scan () < -1:
		gxsm.set ("dsp-fbs-bias","%f"%(bias))
		gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current))
		return 1
		
	gxsm.set ("dsp-fbs-bias","%f"%(bias))
	gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current))
	return 0

def run_set (voltages, x=0):
	if x:
		return 1
	for u in voltages:
		print "Bias=", u
		freeze_Z()
		gxsm.set ("dsp-fbs-bias","%f"%(u))
		gxsm.sleep (10)
		release_Z()
		gxsm.startscan ()
		gxsm.waitscan ()

		if wait_for_scan () < -1:
			return 1
			break
	return 0

def run_vp_test_print (coords, num):
	for i in range(0, num):
	        sx=coords[i][0]
	        sy=coords[i][1]
# force offset and set scan coords
	        print "ScanXY: ", sx, sy
	        gxsm.set ("ScanX","%f"%(sx))
	        gxsm.set ("ScanY","%f"%(sy))
	        gxsm.sleep (10)


def run_vp (coords, num, ref_bias=1.8, ref_current=0.02, ff=0, run_ref=0, ref_bias_list=[0.1]):
	x0=gxsm.get ("OffsetX")
	y0=gxsm.get ("OffsetY")
	i_ac_amp=gxsm.get ("dsp-LCK-AC-Bias-Amp")
	for i in range(0, num):
		if run_ref > 0 and i % run_ref == 0:
			if ff:
			        release_Z ()
			gxsm.set ("dsp-LCK-AC-Bias-Amp","0")
			for bias in ref_bias_list:
				run_ref_image (bias)
			gxsm.set ("dsp-LCK-AC-Bias-Amp","%f"%i_ac_bias_amp)    
			gxsm.set ("dsp-fbs-bias","%f"%(ref_bias))
			gxsm.set ("dsp-fbs-mx0-current-set","%f"%(ref_current))
        	if ff:
	        	goto_refpoint ()
		        freeze_Z ()             

	        sx=coords[i][0]
	        sy=coords[i][1]
# force offset and set scan coords
	        gxsm.set ("OffsetX","%f"%(start_x0))
	        gxsm.set ("OffsetY","%f"%(start_y0))
	        gxsm.sleep (20)
	        print "ScanXY: ", sx, sy
	        gxsm.set ("ScanX","%f"%(sx))
	        gxsm.set ("ScanY","%f"%(sy))
	        gxsm.sleep (20)
		gxsm.set ("dsp-fbs-bias","%f"%(ref_bias))
		gxsm.set ("dsp-fbs-mx0-current-set","%f"%(ref_current))
 	        print "Zzzz"
	        gxsm.sleep (20)
	        print "VP Execute #", i, " of ", len2, " (", (100.*float(i)/len2), "%)"
	        gxsm.action ("dsp-fbs-VP_IV_EXECUTE")
#	        gxsm.action ("dsp-fbs-VP_LM_EXECUTE")
# wait until VP action has finished
	        M = wait_for_vp ()
	        if M < -3:
	                terminate = 1
	                break

		if ff:
		        release_Z ()    

		if M < -0.9:
			pause_tip_retract ()

def run_vp_simple (coords, num, ref_bias=1.8, ref_current=0.02,run_ref=0):
	gxsm.set ("dsp-fbs-bias","%f"%(ref_bias))
	gxsm.set ("dsp-LCK-AC-Bias-Amp","0.03")
	gxsm.set ("dsp-fbs-mx0-current-set","%f"%(ref_current))
	i_ac_bias_amp=gxsm.get ("dsp-LCK-AC-Bias-Amp")
	for i in range(0, num):
		if run_ref > 0 and i % run_ref == 0:
			gxsm.set ("dsp-LCK-AC-Bias-Amp","0")
			run_ref_image (0.2, 0.01)
			gxsm.set ("dsp-LCK-AC-Bias-Amp","%f"%i_ac_bias_amp)    
			gxsm.set ("dsp-fbs-bias","%f"%(ref_bias))
			gxsm.set ("dsp-fbs-mx0-current-set","%f"%(ref_current))
	        sx=coords[i][0]
	        sy=coords[i][1]
# force offset and set scan coords
	        print "ScanXY: ", sx, sy
	        gxsm.set ("ScanY","%f"%(sy))
		wait_scan_pos()
	        gxsm.set ("ScanX","%f"%(sx))
		wait_scan_pos()
	        print "VP Execute #", i, " of ", len2, " (", (100.*float(i)/len2), "%)"
	        gxsm.action ("DSP_VP_IV_EXECUTE")
# wait until VP action has finished
	        M = wait_for_vp ()
	        if M < -3:
	                terminate = 1
	                break
		if M < -0.9:
			pause_tip_retract ()
	gxsm.set ("dsp-LCK-AC-Bias-Amp","0.0")

def run_survey (coords, num, bias=0.1, current=0.01):
	for i in range(0, num):
	        ox=coords[i][0]
	        oy=coords[i][1]
	        print "OffsetXY: ", ox, oy
	        gxsm.set ("OffsetY","%f"%(oy))
		wait_move_pos()
	        gxsm.set ("OffsetX","%f"%(ox))
		wait_move_pos()
		run_ref_image (bias, current)


def run_lm (coords, num, ff, ref_bias=2.35, ref_current=0.045):
##def run_vp (coords, num, ff, ref_bias=2.35, ref_current=0.045):
#       x0=gxsm.get ("OffsetX")
#	y0=gxsm.get ("OffsetY")
	for i in range(0, num):
#		if i % 100 == 0:
#			if ff:
#			        release_Z ()
#			gxsm.set ("dsp-LCK-AC-Bias-Amp","0")    
###			run_ref_image (0.25)
#			run_ref_image (0.5)
#			run_ref_image (2.35)
#			run_ref_image (2.60)
#			run_ref_image (-1.0)

#		gxsm.set ("dsp-LCK-AC-Bias-Amp","0.015")    
###		gxsm.set ("dsp-fbs-bias","%f"%(ref_bias))
###		gxsm.set ("dsp-fbs-mx0-current-set","%f"%(ref_current))
#        	if ff:
#	        	goto_refpoint ()
#		        freeze_Z ()             

	        sx=coords[i][0]
	        sy=coords[i][1]
# force offset and set scan coords
#	        gxsm.set ("OffsetX","%f"%(start_x0))
#	        gxsm.set ("OffsetY","%f"%(start_y0))
#	        gxsm.sleep (20)
	        print "ScanXY: ", sx, sy
#	        gxsm.set ("ScanX","%f"%(sx))
#	        gxsm.set ("ScanY","%f"%(sy))
	        gxsm.set ("ScanX","%f"%(sx))
	        gxsm.set ("ScanY","%f"%(sy))
	        print "Zzzz"
	        gxsm.sleep (40)
	        print "VP Execute #", i, " of ", len2, " (", (100.*float(i)/len2), "%)"
#	        gxsm.action ("dsp-fbs-VP_IV_EXECUTE")
	        gxsm.action ("dsp-fbs-VP_LM_EXECUTE")
# wait until VP action has finished
	        M = wait_for_vp ()
	        if M < -3:
	                terminate = 1
	                break

#		if ff:
#		        release_Z ()    

#		if M < -0.9:
#			pause_tip_retract ()


def chdfrun (x,y):
	release_Z (10)
    	goto_point (0,0, 50)
    	freeze_Z ()
    	gxsm.set ("SPMC_SLS_Xs", "%d"%x)
    	gxsm.set ("SPMC_SLS_Ys", "%d"%y)
    	gxsm.startscan ()
    	gxsm.waitscan ()
    
def chdcircle (m=10,n=400):
	xn=m
	yn=m
	gxsm.set ("SPMC_SLS_Xn", "%d"%xn)
	gxsm.set ("SPMC_SLS_Yn", "%d"%(yn+2))
	rv=[[1,0],[0,-1],[0,-1],[-1,0],[-1,0],[0,1],[0,1],[1,0]]

    	chdfrun (n/2,n/2-yn)
    	for r in range (0,m):
        	x = n/2
        	y = n/2 + yn*r
        	for i in range (0,m-2):
            		for j in range (0,r+1):
                		chdfrun (x,y)
                		x = x + xn*rv[i][0]
                		y = y + yn*rv[i][1]


#cp=0
#ci=5
#chdcircle (10, 400)

#cp=10
#ci=10
#release_Z (10)

#gxsm.save ()

spnx = 2500.
stepx = 1000.

spny = 2500.
stepy = 1000.

sxlist = np.arange (-spnx, spnx+stepx, stepx)
sylist = np.arange (-spny, spny+stepy, stepy)

position = [[0.,0.]]
positions = 1
len2 = np.size(sxlist) * np.size(sylist) * positions
xy = np.arange(2.0*len2).reshape(len2,2)


i=0
for r in position:
	for y in sylist:
#		run_ref_image (0.7)
		for x in sxlist:
        	        xy[i][0] = x + r[0]
        	        xy[i][1] = y + r[1]
        	        i=i+1           
#print len2
#print xy
#np.random.shuffle(xy)
#print xy



###def run_vp (coords, num, ref_bias=2.35, ref_current=0.045, ff=0, run_ref=0, ref_bias_list=[0.1])
#run_vp_simple (xy, len2, 1.8, 0.02, 10)

#run_survey (xy, len2, 0.1, 0.01)

print "STARTING NOW..."

#terminate = run_set ([0.03, 0.05, 0.1, 0.2, 0.5, 1.0, 1.5, 2.0])

for z in np.arange(-18.1, -20.0, -0.1):
	print z
	gxsm.set ("dsp-adv-dsp-zpos-ref","%f"%z )
	gxsm.startscan ()
	gxsm.waitscan ()


z=-16
gxsm.set ("dsp-adv-dsp-zpos-ref","%f"%z )


#freeze_Z()
#gxsm.startscan ()
#gxsm.waitscan ()
#release_Z()

#gxsm.set ("dsp-fbs-mx0-current-set","0.01")
#gxsm.set ("dsp-adv-scan-speed-scan","50.0")
#gxsm.set ("dsp-fbs-scan-fast-return","1")
#gxsm.startscan ()


# ACHTUNG: Clear/Expand Bias Warning Range - Warning Popup will block/terminate script execution

#gxsm.set ("dsp-fbs-mx0-current-set","0.005")
#terminate = run_set (np.arange(0.5, 3.3, 0.25 ))
#terminate = run_set (np.arange(-0.5, -3.3, -0.25 ))
#gxsm.set ("dsp-fbs-mx0-current-set","0.01")
#terminate = run_set ([0.1, 0.05, 0.5, -0.1, -0.5, -1.0, -2.0, 0.1, 1.0, 2.0, 2.5, -2.5, 0.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.02")
#terminate = run_set ([0.1, 0.05, 0.5, -0.1, -0.5, -1.0, -2.0, 0.1, 1.0, 2.0, 2.5, -2.5, 0.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.025")
#terminate = run_set ([0.1, 0.05, 0.5, -0.1, -0.5, -1.0, -2.0, 0.1, 1.0, 2.0, 2.5, -2.5, 0.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.01")

#gxsm.set ("dsp-fbs-mx0-current-set","0.03")
#gxsm.set ("dsp-adv-scan-fast-return","6")
#gxsm.set ("dsp-fbs-scan-speed-scan","7")
#gxsm.sleep (40)
#freeze_Z()
#gxsm.startscan ()
#gxsm.waitscan ()
#release_Z()
#gxsm.set ("dsp-fbs-mx0-current-set","0.005")
#gxsm.set ("dsp-adv-scan-fast-return","1")
#gxsm.set ("dsp-fbs-scan-speed-scan","50")
##
#terminate = run_set ([0.05, 0.1, 0.5, 1.6, -0.1, -0.5, -1.6])
#terminate = run_set ([0.05, 0.1, 0.5, 1.6, -0.1, -0.5, -1.6])

#terminate = run_set ([0.02, 0.1, 0.5, 1.5, -0.1, -0.5, -1.0])


# random or line by line grid
#np.random.shuffle(xy)

#gxsm.set ("dsp-LCK-AC-Bias-Amp","0.025")

#run_vp_test_print (xy, len2)
#run_vp (xy, len2, 2.35, 0.05)

#run_lm (xy, len2, 0, 0.1, 0.01)


#	run_ref_image (0.1)
#	run_ref_image (0.7)
#####	run_vp (xy, len2, 0, 0.01, 0.08)
#	gxsm.set ("dsp-fbs-motor","%d"%(k))


#gxsm.set ("OffsetX","0")
#gxsm.set ("OffsetY","0")
#terminate = run_set ([-3, -2, -1, -0.5, -0.1, 0.1, 0.5, 1., 2., 3.])

#gxsm.set ("dsp-fbs-bias","0.1")
#gxsm.set ("dsp-fbs-mx0-current-set","0.02")
#gxsm.set ("dsp-fbs-ScanSpd","10")
#freeze_Z ()

#terminate = run_set ([0.0])

#gxsm.set ("dsp-fbs-bias","0.5")
#gxsm.set ("dsp-fbs-mx0-current-set","0.01")
#gxsm.set ("dsp-fbs-ScanSpd","200")

#release_Z ()

#terminate = run_set ([0.5, 1.0, 2.0, -0.5, -1.0, 0.5])

#gxsm.set ("dsp-fbs-mx0-current-set","0.0")


#terminate = run_set ([2.0, 3.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.02")
#gxsm.set ("dsp-fbs-bias","0.1")
#terminate = run_set ([0.1, 0.4])
#gxsm.set ("dsp-fbs-bias","2.0")
#gxsm.set ("dsp-fbs-mx0-current-set","0.3")
#terminate = run_set ([2.0, 3.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.02")
#gxsm.set ("dsp-fbs-bias","0.1")
#terminate = run_set ([0.1, 0.4])
#gxsm.set ("dsp-fbs-bias","2.0")
#gxsm.set ("dsp-fbs-mx0-current-set","0.3")
#terminate = run_set ([2.0, 3.1])
#gxsm.set ("dsp-fbs-mx0-current-set","0.02")
#gxsm.set ("dsp-fbs-bias","0.1")




#gxsm.set ("dsp-fbs-bias","0.2")
#gxsm.set ("dsp-fbs-mx0-current-set","0.08")


#terminate = run_set ([0.2])

#gxsm.set ("dsp-fbs-bias","%f"%(bias))
#gxsm.set ("dsp-fbs-motor","1")
#gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current))
#cp=25
#ci=25
#gxsm.set ("dsp-fbs-cp","%f"%(cp))
#gxsm.set ("dsp-fbs-ci","%f"%(ci))
#gxsm.set ("dsp-fbs-ScanSpd","180")

#terminate = run_set ([0.05])

#run_vp (xy, len2, 0, 0.05, 0.08)


# random or line by line grid
#np.random.shuffle(xy)

# test
#M = wait_for_vp ()
#if M < -0.9:
#	pause_tip_retract ()

#run_ref_image (2.8)
#run_ref_image (3.4)
#run_ref_image (2.4)
#run_ref_image (1.8)
###run_vp (coords, num, ff, ref_bias=2.35, ref_current=0.045):

#run_vp (xy, len2, 0, 0.010, 0.100)


#gxsm.set ("dsp-fbs-mx0-current-set","0.07")
#gxsm.set ("dsp-fbs-bias","%f"%(u))
#gxsm.set ("OffsetX","%f"%(x0))
#gxsm.set ("OffsetY","%f"%(y0))
#gxsm.sleep (50)
#gxsm.startscan ()
#gxsm.waitscan ()

#terminate = run_set ([-2.0, -2.2, -2.4, -2.8, -3.4])
#terminate = run_set ([1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.8, 3.4])

###############
# Const H program:

#gxsm.set ("RangeX","60")
#gxsm.set ("PointsX","400")
#gxsm.set ("dsp-fbs-ScanSpd","488")

#terminate = run_set ([2.0])

#const_height_slow_Z(0.25)

#terminate = run_set ([1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.6, 2.8, 3.4])

#freeze_Z()
#gxsm.set ("dsp-fbs-bias","-2.0")
#gxsm.sleep (50)
#const_height_slow_Z(0.25)

#terminate = run_set ([ -2.0, -2.2, -2.4, -2.8, -3.0, -3.4])

#release_Z()
#terminate = run_set ([2.0, 2.1, 2.4])
##############

#terminate = run_set (np.arange (2.0, 0.2, -0.5))

#gxsm.set ("dsp-fbs-FastReturn","1")
#gxsm.set ("dsp-fbs-ScanSpd","30")
#terminate = run_set ([0.5,1.2,2.2])

#freeze_Z()
#gxsm.set ("dsp-fbs-bias","2.4")
#release_Z()
#terminate = run_set ([2.0,2.05,2.1,2.15,2.2,2.3,2.4,2.5,2.6,2.7,2.8])


###########################
#release_Z ()	

#gxsm.set ("dsp-fbs-bias","%f"%(bias))
#gxsm.set ("dsp-fbs-motor","1")
#gxsm.set ("dsp-fbs-mx0-current-set","%f"%(current))
#gxsm.set ("dsp-fbs-cp","%f"%(cp))
#gxsm.set ("dsp-fbs-ci","%f"%(ci))

#auto retract
#gxsm.set ("dsp-fbs-mx0-current-set","0.0")
     
