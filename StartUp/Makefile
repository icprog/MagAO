all:

build:

clean:


install:
	install -d $(ADOPT_ROOT)/bin
	install init_wfs.py $(ADOPT_ROOT)/bin
	install ao_start.py $(ADOPT_ROOT)/bin
	install ao_stop.py $(ADOPT_ROOT)/bin
	install ao_check.py $(ADOPT_ROOT)/bin
	install ao_restart.sh $(ADOPT_ROOT)/bin
	install ao_restart_script.sh $(ADOPT_ROOT)/bin
	install msg.py $(ADOPT_ROOT)/bin
	ln -sf $(ADOPT_ROOT)/bin/ao_start.py $(ADOPT_ROOT)/bin/adsc_start
	ln -sf $(ADOPT_ROOT)/bin/ao_stop.py $(ADOPT_ROOT)/bin/adsc_stop
	ln -sf $(ADOPT_ROOT)/bin/ao_check.py $(ADOPT_ROOT)/bin/adsc_check
	ln -sf $(ADOPT_ROOT)/bin/ao_start.py $(ADOPT_ROOT)/bin/w_start
	ln -sf $(ADOPT_ROOT)/bin/ao_stop.py $(ADOPT_ROOT)/bin/w_stop
	ln -sf $(ADOPT_ROOT)/bin/ao_check.py $(ADOPT_ROOT)/bin/w_check

tags:
	ctags -t *.[ch]
