import os

inputDir = "."
outputDir = "."

files = [f for f in os.listdir(inputDir) if f.endswith(".nc")]
files.sort()

for f in files:
	fin = inputDir+"/"+f
	print ("Reading: ", fin)
	gxsm.load (fin,1)
	fout = outputDir+"/"+os.path.splitext(f)[0]+".top"
	print ("Exporting: ", fout, " ret=", gxsm.gnuexport (fout, 1))