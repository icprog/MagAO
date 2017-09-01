# Note: most of the packages must be installed with their standard installation  procedure.
#       All installation commands are maneged with "sudo" because thei require root
#       privileges.
#
#       The packages cfitsio and parapin do not require installation, just "make".
#

IDL_LIBRARY_PATH = /usr/local/itt/idl/lib

all: cfitsio parapin-1.5.1-beta1  qwt-5.1.2

astro: astro.tgz
	tar xvfz $<

coyote: coyote.tgz
	tar xvfz $<

cfitsio: cfitsio.tgz
	tar xvfz $<
	cd cfitsio; echo "Running configure..."; ./configure --quiet; make

fsmc-1.1.1: fsmc-1.1.1.tgz
	tar xvfz $<
	cd fsmc-1.1.1; qmake; make

fsme-1.0.4: fsme-1.0.4m.tgz
	tar xvfz $<
	cd fsme-1.0.4; qmake; make

mpfit: mpfit.tgz
	tar xvfz $<
oaa_lib: oaa_lib-0.tgz
	tar xvfz $<
parapin-1.5.1-beta1: parapin-1.5.1-beta1.tgz
	tar xvfz $<
	cd parapin-1.5.1-beta1; make

python-idl: python-idl.tgz
	tar xvfz $<
	cd python-idl; python setup.py build; 

qwt-5.1.2: qwt-5.1.2.tgz
	tar xvfz $<
	find qwt-5.1.2 -name Makefile -exec rm -f {} \;
	cd qwt-5.1.2; qmake; make

TeXtoIDL: TeXtoIDL.tgz
	tar xvfz $<

install: fsmc-1.1.1 fsme-1.0.4 mpfit oaa_lib python-idl qwt-5.1.2 install-astro install-coyote TeXtoIDL
	cd fsmc-1.1.1; sudo make install
	cd fsme-1.0.4; sudo make install
	cd mpfit; sudo make install
	cd oaa_lib; sudo make install
	cd python-idl; sudo python setup.py install
	cd qwt-5.1.2; sudo make install
	cd TeXtoIDL; sudo make install

install-astro: astro
	install -d $(IDL_LIBRARY_PATH)/astrolib; cp -a astro/pro/* $(IDL_LIBRARY_PATH)/astrolib

install-coyote: coyote
	install -d $(IDL_LIBRARY_PATH)/coyote; cp -a coyote/*.pro $(IDL_LIBRARY_PATH)/coyote

clean:
	rm -rf cfitsio
	rm -rf parapin-1.5.1-beta1
	rm -rf astro
	rm -rf coyote
	rm -rf mpfit
	rm -rf TeXtoIDL
	rm -rf python-idl
	rm -rf qwt-5.1.2
	rm -rf oaa_lib
	rm -rf fsme-1.0.4
	rm -rf fsmc-1.1.1

