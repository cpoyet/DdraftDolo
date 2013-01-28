#
# Makefile for xPLLib
#

#
#CCOPTS = -g -DLINUX -pedantic -I./xPLLib -I./libroxml
CCOPTS = -g -DLINUX -I./xPLLib -I./libroxml
LIBS = -g -luuid 


# *******************************************************
# ******* No more customizations from here down *********
# *******************************************************

#LDOPTS	= -O
LDOPTS	= 
CC	=	gcc $(CCOPTS)
LD	= 	gcc $(LDOPTS)

#CMD_LIST = xPL_Hub xPL_Logger xPL_Clock xPL_ConfigClock xPLSend
CMD_LIST = xPLHal4L

SUBDIR= xPLLib libroxml


%.o: %.c %.h
	$(CC) -c $<

all:	${CMD_LIST}
	for i in $(SUBDIRS); do (cd $$i; $(MAKE) all); done


clean:
	for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done
#	rm -f *.o *.a core ${CMD_LIST}
	rm -f *.o core ${CMD_LIST}

xPLHal4L: xPLHal_scheduler.o xPLHal4L.o XHCP_server_ipv6.o xPLHal_rules.o xPLHal_common.o
	$(LD) -g -o xPLHal4L xPLHal_scheduler.o xPLHal4L.o XHCP_server_ipv6.o xPLHal_rules.o xPLHal_common.o ./libroxml.a ./xPLLib.a $(LIBS)

rebuild: clean all
