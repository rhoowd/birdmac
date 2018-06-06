CONTIKI = ../

TARGET = z1

APPS = serial-shell

CONTIKIDIRS += $(CONTIKI)/lib
PROJECT_SOURCEFILES += birdtrace.c

%.shell-upload: %.ce
	(echo; sleep 4; echo "~K"; sleep 4; \
         echo "dec64 | write $*.ce | null"; sleep 4; \
	 ../../tools/base64-encode < $<; sleep 4; \
         echo ""; echo "~K"; echo "read $*.ce | size"; sleep 4) | make login

include $(CONTIKI)/Makefile.include
