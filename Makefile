# You'll need a Bourne Shell, bash or ksh should work as well
SHELL = /bin/sh

CONFIG_TRASH=	\
	config_test.log \
	config_test.o \
	config_test.c \
	config_test.h \
	config_test \

# Leave the above line empty

all:	bitter	

clean:
	rm -f -- $(CONFIG_TRASH) && \
	cd src && test -f Makefile && make clean && \
	cd lib && test -f Makefile && make clean

clobber: distclean

distclean: clean
	rm -f -- config.h \
		src/config.h src/Makefile \
		src/lib/config.h src/lib/Makefile

bitter: config.h
	cd src && $(MAKE) $@

config.h: config.conf config.sh Makefile
	$(SHELL) config.sh

depend: Makefile config.sh
	rm -f src/Makefile.dep src/lib/Makefile.dep && \
	$(SHELL) config.sh && \
	rm -f -- config.h

checks: bitter checks.sh
	$(SHELL) checks.sh

install: bitter 
	cd src && $(MAKE) $@
