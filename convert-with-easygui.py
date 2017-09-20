import os
import easygui
inputDir="."
outputDir="."

inputDir=easygui.diropenbox("Select Source Folder", inputDir)
outputDir=easygui.diropenbox("Select Destination Folder", outputDir)

try:
	#[inputDir, outputDir] = easygui.multenterbox("Setup","GXSM NC file converter", ["Input Folder", "Output Folder"])
	try:
		files = [f for f in os.listdir(inputDir) if f.endswith(".nc")]
		files.sort()

		for f in files:
			fin = inputDir+"/"+f
			print ("Reading: ", fin)
			gxsm.load (0, fin)
			fout = outputDir+"/"+os.path.splitext(f)[0]+".top"
			print ("Exporting: ", fout, " ret=", gxsm.export (0, fout))
	except (OSError, RuntimeError, TypeError, NameError):
		print ("folder or files not found.")
		pass
except (RuntimeError, TypeError, NameError):
	pass
	print ("Please enter input / output folders.")