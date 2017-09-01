# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: 
	$(Q)$(MAKE) $(MAKEOPTS) -C Housekeeper all
	$(Q)$(MAKE) $(MAKEOPTS) -C AdSecMir all
	$(Q)$(MAKE) $(MAKEOPTS) -C FastAdSecMir all

install: all
	$(Q)$(MAKE) $(MAKEOPTS) -C Housekeeper install
	$(Q)$(MAKE) $(MAKEOPTS) -C AdSecMir install
	$(Q)$(MAKE) $(MAKEOPTS) -C FastAdSecMir install

clean:
	$(Q)$(MAKE) $(MAKEOPTS) -C Housekeeper clean
	$(Q)$(MAKE) $(MAKEOPTS) -C AdSecMir clean
	$(Q)$(MAKE) $(MAKEOPTS) -C FastAdSecMir clean
