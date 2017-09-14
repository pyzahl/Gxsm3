import os

inputDir = "."
outputDir = "."

files = [f for f in os.listdir(inputDir) if f.endswith(".nc")]
files.sort()

for f in files:
	fin = inputDir+"/"+f
	print ("Reading: ", fin)
	gxsm.load (0, fin)
	fout = outputDir+"/"+os.path.splitext(f)[0]+".top"
	print ("Exporting: ", fout, " ret=", gxsm.export (0, fout))