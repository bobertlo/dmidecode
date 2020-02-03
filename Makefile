#
#   DMI Decode
#   BIOS Decode
#   VPD Decode
#
#   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
#   Copyright (C) 2002-2015 Jean Delvare <jdelvare@suse.de>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

CC      = gcc
CFLAGS  = -W -Wall -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual \
          -Wcast-align -Wwrite-strings -Wmissing-prototypes -Winline -Wundef

# Let lseek and mmap support 64-bit wide offsets
CFLAGS += -D_FILE_OFFSET_BITS=64

#CFLAGS += -DBIGENDIAN
#CFLAGS += -DALIGNMENT_WORKAROUND

# When debugging, disable -O2 and enable -g.
CFLAGS += -O2
#CFLAGS += -g

# Pass linker flags here
LDFLAGS =

DESTDIR =
prefix  = /usr/local
sbindir = $(prefix)/sbin
mandir  = $(prefix)/share/man
man8dir = $(mandir)/man8
docdir  = $(prefix)/share/doc/dmidecode

INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644
INSTALL_DIR     := $(INSTALL) -m 755 -d
INSTALL_PROGRAM := $(INSTALL) -m 755
RM              := rm -f

PROGRAMS := dmidecode biosdecode ownership vpddecode

all : $(PROGRAMS)

#
# Programs
#

dmidecode : dmidecode.o dmiopt.o dmioem.o util.o mem_fsx.o getopt.o getopt1.o inet_ntop.o
	$(CC) $(LDFLAGS) dmidecode.o dmiopt.o dmioem.o util.o mem_fsx.o getopt.o getopt1.o inet_ntop.o -o $@

biosdecode : biosdecode.o util.o mem_fsx.o getopt.o getopt1.o
	$(CC) $(LDFLAGS) biosdecode.o util.o mem_fsx.o getopt.o getopt1.o -o $@

ownership : ownership.o util.o mem_fsx.o getopt.o getopt1.o
	$(CC) $(LDFLAGS) ownership.o util.o mem_fsx.o getopt.o getopt1.o -o $@

vpddecode : vpddecode.o vpdopt.o util.o mem_fsx.o getopt.o getopt1.o
	$(CC) $(LDFLAGS) vpddecode.o vpdopt.o util.o mem_fsx.o getopt.o getopt1.o -o $@

#
# Objects
#

dmidecode.o : dmidecode.c version.h types.h util.h config.h dmidecode.h \
	      dmiopt.h dmioem.h
	$(CC) $(CFLAGS) -c $< -o $@

dmiopt.o : dmiopt.c config.h types.h util.h dmidecode.h dmiopt.h
	$(CC) $(CFLAGS) -c $< -o $@

dmioem.o : dmioem.c types.h dmidecode.h dmioem.h
	$(CC) $(CFLAGS) -c $< -o $@

biosdecode.o : biosdecode.c version.h types.h util.h config.h 
	$(CC) $(CFLAGS) -c $< -o $@

ownership.o : ownership.c version.h types.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

vpddecode.o : vpddecode.c version.h types.h util.h config.h vpdopt.h
	$(CC) $(CFLAGS) -c $< -o $@

vpdopt.o : vpdopt.c config.h util.h vpdopt.h
	$(CC) $(CFLAGS) -c $< -o $@

util.o : util.c types.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

mem_fsx.o : mem_fsx.c
	$(CC) $(CFLAGS) -c $< -o $@

getopt.o : getopt.c getopt.h gettext.h
	$(CC) $(CFLAGS) -c $< -o $@

getopt1.o : getopt1.c getopt.h
	$(CC) $(CFLAGS) -c $< -o $@

inet_ntop.o: inet_ntop.c
	$(CC) $(CFLAGS) -c $< -o $@

#
# Commands
#

strip : $(PROGRAMS)
	strip $(PROGRAMS)

install : install-bin install-man install-doc

uninstall : uninstall-bin uninstall-man uninstall-doc

install-bin : $(PROGRAMS)
	$(INSTALL_DIR) $(DESTDIR)$(sbindir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_PROGRAM) $$program $(DESTDIR)$(sbindir) ; done

uninstall-bin :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(sbindir)/$$program ; done

install-man :
	$(INSTALL_DIR) $(DESTDIR)$(man8dir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_DATA) man/$$program.8 $(DESTDIR)$(man8dir) ; done

uninstall-man :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(man8dir)/$$program.8 ; done

install-doc :
	$(INSTALL_DIR) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) README $(DESTDIR)$(docdir)
	$(INSTALL_DATA) NEWS $(DESTDIR)$(docdir)
	$(INSTALL_DATA) AUTHORS $(DESTDIR)$(docdir)

uninstall-doc :
	$(RM) -r $(DESTDIR)$(docdir)

clean :
	$(RM) *.o $(PROGRAMS) core
