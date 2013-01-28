
CC=gcc
CFLAGS= -g -I/usr/local/include -DIGNORE_EMPTY_TEXT_NODES
LDFLAGS= -L/usr/local/lib

RM=/bin/rm
MAKE=/usr/bin/make
MAKEDEPEND=/usr/openwin/bin/makedepend
AR=/usr/bin/ar

SRC= roxml-internal.c roxml-parse-engine.c roxml.c
ARCH= ../libroxml.a

#OBJ=$(subst .c,.o,$(SRC))
OBJ=$(SRC:%.c=%.o)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: 
	@$(MAKE) $(ARCH)

$(ARCH): $(OBJ)
	@echo Création de la l\'archive $(ARCH)
	@$(RM) -f $@
	$(AR) rc $@  $(OBJ)


clean:
	@$(RM) -f $(OBJ) $(ARCH) core *~

