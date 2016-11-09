#
# Searches for the necessary header files in your system
# not 100% safe, also finds commented headers. but gives a hint.
#
# Perhaps this can be refined to propose installation packages for RH, Suse
# and Debian
for i in `find ".." -name *.C`; 
do 
  grep "include" $i >> allincludes1.txt ;
done

grep "#" allincludes1.txt | grep "<" | cut -f2 -d"<" |\
 cut -f1 -d">" | sort | uniq > allincludes2.txt

for i in `cat allincludes2.txt`
do
  x=`locate $i 2> /dev/null`
  if [ -z "$x" ]; then
    echo 
    echo $i was not found
    wget -q -O dummy.html "http://packages.debian.org/cgi-bin/search_contents.pl?word=$i&searchmode=searchfiles&case=insensitive&version=testing&arch=i386"
    echo "In Debian/Testing for this file you need:"
    unhtml dummy.html | sed -n '/FILE/,/The used/p' | grep -v "The used"
  fi
done
rm allincludes[12].txt dummy.html
echo
echo "NOTES:"
echo "dfftw is not needed, if fftw.h is there"
echo "Python is only needed in one version (2.1|2.2|2.3)"
echo "gtkdatabox is old and no longer needed"
echo "altivec is hardware specific"

## These packages contain the libs necessary to run gxsm2 on debian/testing
## 12/2004, get them with grep "###"
### fftw2
### freeglut3
### libart-2.0-2 
### libatk1.0-0
### libaudiofile0
### libbonobo2-0
### libbonoboui2-0
### libc6   
### libesd0
### libexpat1
### libfontconfig1
### libfreetype6
### libgcc1
### libgconf2-4
### libgcrypt11
### libglib2.0-0
### libgnome-keyring0
### libgnome2-0
### libgnomecanvas2-0
### libgnomeui-0
### libgnomevfs2-0
### libgnutls11
### libgpg-error0
### libgtk2.0-0
### libgtkglext1
### libice6
### libjpeg62
### liborbit2
### libpango1.0-0
### libpopt0
### libsm6
### libstdc++5
### libtasn1-2
### libx11-6
### libxcursor1
### libxext6
### libxft2
### libxi6
### libxml2
### libxmu6
### libxrandr2
### libxrender1
### libxt6
### netcdfg3
### python2.3
### xlibmesa-gl
### xlibmesa-glu
