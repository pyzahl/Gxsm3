import Image, ImageFile

class Gxsmnetcdf(ImageFile.ImageFile):

    format = "nc"
    format_description = "Gxsm netcdf"

    def _open(self):
	self.i=1
        self.mode = "L"
	self.fp.seek(0) 
 	t=self.fp.read(3)
 	if t != "CDF":
		raise SyntaxError, "not a CDF file"

	from Scientific.IO.NetCDF import NetCDFFile
	self.f= NetCDFFile(self.filename, 'r')
	self.size = self.f.dimensions['dimx'], self.f.dimensions['dimy']

    def load(self):
	#for some reason, load is executed twice ->dont need this
	self.i=self.i+1
	if self.i!=2:
		return

	import Numeric
	foo=self.f.variables['H']
	a=foo.getValue()

	b=a[0][0]

	d=Numeric.ravel(b)	# linearize
	d=d-min(d)		#  map to [0..256]
	d=d*256./max(d)	# we should catch div by 0

        self.im = Image.core.new(self.mode, self.size)
        self.im.putdata( d )

Image.register_open("nc", Gxsmnetcdf)
Image.register_extension("NC", ".nc")
