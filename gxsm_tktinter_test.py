import os
from tkinter import *
from tkinter import ttk


def execute_conversion(*args):
    try:
	files = [f for f in os.listdir(inputdir.get ()) if f.endswith(".nc")]
	files.sort()
        for f in files:
		fin = inputdir.get ()+"/"+f
		gxsm.load (0, fin)
		gxsm.sleep(0.1)
		fout = outputdir.get ()+"/"+os.path.splitext(f)[0]+".top"
		currentfile.set (fin + " -> " + fout + " staus=" + str(gxsm.export (0, fout)))
		gxsm.sleep(0.1)
    except ValueError:
        pass
        
    
root = Tk()
root.title("Gxsm PyTk GUI")

mainframe = ttk.Frame(root, padding="3 3 12 12")
mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
mainframe.columnconfigure(0, weight=1)
mainframe.rowconfigure(0, weight=1)

inputdir = StringVar()
inputdir.set (".")
outputdir = StringVar()
outputdir.set (".")
currentfile = StringVar()

inputd_entry = ttk.Entry(mainframe, width=7, textvariable=inputdir)
inputd_entry.grid(column=2, row=1, sticky=(W, E))
outputd_entry = ttk.Entry(mainframe, width=7, textvariable=outputdir)
outputd_entry.grid(column=2, row=2, sticky=(W, E))

ttk.Label(mainframe, textvariable=currentfile).grid(column=2, row=3, sticky=(W, E))
ttk.Button(mainframe, text="Convert Files", command=execute_conversion).grid(column=3, row=3, sticky=W)

ttk.Label(mainframe, text="Input Folder").grid(column=3, row=1, sticky=W)
ttk.Label(mainframe, text="Output Folder").grid(column=3, row=2, sticky=W)
ttk.Label(mainframe, text="Processing").grid(column=1, row=3, sticky=E)

for child in mainframe.winfo_children(): child.grid_configure(padx=5, pady=5)

inputd_entry.focus()
root.bind('<Return>', execute_conversion)

root.mainloop()
