import array     # for array
import numpy  # for fromfunction
import math     # for sin

def dist(x,y):
   return ((numpy.sin((x-50)/15.0) + numpy.sin((y-50)/15.0))*100)

N=500
m = numpy.fromfunction(dist, (N,N), dtype=float)

n = numpy.ravel(m) # make 1-d

examplearray = array.array('f', n.astype(float)) 
#print examplearray

gxsm.createscanf (0,N, N, 1000, 1000, examplearray)
