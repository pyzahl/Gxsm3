import random
w=range(0,500)
for i in range (0,100):
	(x,y)=random.sample(w,2)
	gxsm.add_marker_object (0,"#%d"%i, int(random.sample(range(0,6),1)[0]), x,y )
for i in range(0,30):
	print (gxsm.get_object(0,i))
print(random.sample(range(0,1000),2))