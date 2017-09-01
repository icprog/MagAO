# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

LIBS = aoArb adSecArb wfsArb
.PHONY: $(LIBS) base

all: $(LIBS)

$(LIBS): base


clean: clean_basearb clean_aoarblib clean_adsecarblib clean_wfsarblib
	rm -f AdSecArbLib.a

# modules make #

base $(LIBS):
	$(Q)$(MAKE) $(MAKEOPTS) -C $@ all

# modules clean #
	
clean_basearb:
	$(Q)$(MAKE) $(MAKEOPTS) -C $(ADOPT_LIB)/arblib/base clean 

clean_aoarblib:
	$(Q)$(MAKE) $(MAKEOPTS) -C $(ADOPT_LIB)/arblib/aoArb clean 
	
clean_adsecarblib:
	$(Q)$(MAKE) $(MAKEOPTS) -C $(ADOPT_LIB)/arblib/adSecArb clean 
	
clean_wfsarblib:
	$(Q)$(MAKE) $(MAKEOPTS) -C $(ADOPT_LIB)/arblib/wfsArb clean 
	
# tests #

arbtests:
	cd tests; $(MAKE) all; cd ..;
	
clean_arbtests:
	cd tests; $(MAKE) clean; cd ..;
	
