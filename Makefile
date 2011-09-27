#
# Makefile for xPLLib
#

#
#CCOPTS = -g -DLINUX -pedantic -I./xPLLib -I./libroxml
CCOPTS = -g -DLINUX -I./xPLLib -I./libroxml
LIBS = -g -lpthread  


# *******************************************************
# ******* No more customizations from here down *********
# *******************************************************

LDOPTS	= -O
CC	=	gcc $(CCOPTS)
LD	= 	gcc $(LDOPTS)

#CMD_LIST = xPL_Hub xPL_Logger xPL_Clock xPL_ConfigClock xPLSend
CMD_LIST = xPLHal4L

SUBDIR= xPLLib libroxml

.c.o:	
	$(CC) -c $<

.o:

	$(LD) -o $@ $< ./libroxml.a ./xPL.a $(LIBS) 

.c:	
	$(CC) -c $<
	$(LD) -o $@ $< ./libroxml.a ./xPL.a $(LIBS)


all:	${CMD_LIST}
	for i in $(SUBDIRS); do (cd $$i; $(MAKE) all); done


clean:
	for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done
#	rm -f *.o *.a core ${CMD_LIST}
	rm -f *.o core ${CMD_LIST}

xPL_Threads: xPL_Threads.c
	$(CC) -c xPL_Threads.c -g
	$(LD) -g -o xPL_Threads xPL_Threads.o ./libroxml.a ./xPLLib.a -lpthread $(LIBS)

xPLHal4L: xPLHal4L.c XHCP_server.c XHCP_server.h
	$(CC) -c xPLHal4L.c -g
	$(CC) -c XHCP_server.c -g
	$(CC) -c xPLHal_scheduler.c -g
	$(LD) -g -o xPLHal4L xPLHal_scheduler.o xPLHal4L.o XHCP_server.o ./libroxml.a ./xPLLib.a -lpthread -luuid $(LIBS)

rebuild: clean all





