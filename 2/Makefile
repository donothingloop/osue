# Makefile for
# Project: encr
# Author: Christian Fiedler <e1363562@student.tuwien.ac.at>

PROJ=encr
PROJFILES=*.c *.h Makefile Doxyfile
PROJARCHIVE="$(PROJ).tgz"
CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -c -g -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L 
LFLAGS=-lcrypt
OBJS=encr.o childlist.o
DOXYFILE=./Doxyfile
DOXY_TRASH=html latex doxygen_sqlite3.db

# all = release target
all: release

release: CFLAGS+= -O3
release: $(PROJ) 

# debug -> debug mode 
debug: CFLAGS+= -DENDEBUG -g -O0
debug: clean $(PROJ) 
 
$(PROJ): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(PROJ) 

%.o: %.c
	$(CC) $(CFLAGS) $^

clean:
	rm -f *.o $(PROJ) $(PROJARCHIVE)
	rm -Rf $(DOXY_TRASH)

docs:
	doxygen $(DOXYFILE)

export:
	tar cfz $(PROJARCHIVE) $(PROJFILES)

.PHONY: all docs release debug clean

