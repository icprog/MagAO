# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

ARBITRATORS = wfsarbitrator adsecarbitrator aoarbitrator
.PHONY: $(ARBITRATORS) framework

all: $(ARBITRATORS)

$(ARBITRATORS): framework

framework $(ARBITRATORS):
	$(Q)$(MAKE) $(MAKEOPTS) -C $@ all

install: all
	$(Q)$(MAKE) $(MAKEOPTS) -C wfsarbitrator install
	$(Q)$(MAKE) $(MAKEOPTS) -C adsecarbitrator install
	$(Q)$(MAKE) $(MAKEOPTS) -C aoarbitrator install

clean:
	$(Q)$(MAKE) $(MAKEOPTS) -C framework clean
	$(Q)$(MAKE) $(MAKEOPTS) -C wfsarbitrator clean
	$(Q)$(MAKE) $(MAKEOPTS) -C adsecarbitrator clean
	$(Q)$(MAKE) $(MAKEOPTS) -C aoarbitrator clean
