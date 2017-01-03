import sys
import datetime

# Py3 only
#def print(*objects, sep=' ', end='\n', file=sys.stdout, flush=True):
#    __builtins__.print(*objects, sep=sep, end=end, file=file, flush=flush)
    
#Since no default file / script was found, here are some
# things you can do with the python interface
# - see the manual for more information
# Execute to try these
print('{:%Y-%m-%d %H:%M:%S} : Script start'.format(datetime.datetime.now()))
c = gxsm.get("script-control")
print ("Control value: "), c
if c < 2:
    print ("control value below limit - aborting!")
    print ("you can set the value in the window above")
    # you could set Script_Control like any other variable - see below
else:
    gxsm.set ("RangeX","400")
    gxsm.set ("RangeY","400")
    gxsm.set ("PointsX","400")
    gxsm.set ("PointsY","400")
    gxsm.set ("dsp-fbs-bias","0.1")
    gxsm.set ("OffsetX", "0")
    gxsm.set ("OffsetY", "0")
#    gxsm.action ("DSP_CMD_AUTOAPP")
    gxsm.startscan ()
    c = gxsm.get("script-control")
    print ("Control value: "), c
    sys.stdout.flush()
    while c > 0:
        x=gxsm.rtquery("s")
	vp=int(x[0])&8
        c = gxsm.get("script-control")
        while c > 0 and vp > 0:
            c = gxsm.get("script-control")
            x=gxsm.rtquery("s")
    	    vp=int(x[0])&8
            sys.stdout.flush()
            print ("{:%Y-%m-%d %H:%M:%S} : waiting...".format(datetime.datetime.now()))
    	    gxsm.sleep(10)
	print('{:%Y-%m-%d %H:%M:%S} : Script DSP_VP_EXECUTE'.format(datetime.datetime.now()))
        print ("start action: DSP_VP_EXECUTE")
        sys.stdout.flush()
        gxsm.set ("ScanX", str(5*((c%20)-10)))
        gxsm.set ("ScanY", str(5*((int((c%200)/20))-10)))
        gxsm.sleep(200)
        gxsm.set ("dsp-IV-Points00",str(500+10*c))
	if c % 2:
	        gxsm.action ("DSP_VP_IV_EXECUTE")
	else:
	        gxsm.action ("DSP_VP_LM_EXECUTE")
	cc = gxsm.get("script-control")
        c=c-1
	if cc > 0:
        	gxsm.set("script-control", str(c))
        else:
        	c=cc	
#    gxsm.startscan ()
gxsm.set("script-control", "2")
print('{:%Y-%m-%d %H:%M:%S} : Script completed'.format(datetime.datetime.now()))
