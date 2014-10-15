# surf version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
#TICPREFIX = ${PREFIX}/share/tig/
TICPREFIX = ./gui/

CURLINC = `curl-config --cflags`
CURLLIB = `curl-config --libs`

BASEDIRINC = `pkg-config --cflags libxdg-basedir`
BALEDIRLIB = `pkg-config --libs libxdg-basedir`

GTKINC = `pkg-config --cflags gtk+-3.0`
GTKLIB = `pkg-config --libs gtk+-3.0`


# includes and libs
INCS = -I. -I/usr/include ${CURLINC} ${BASEDIRINC} ${GTKINC}
LIBS = -L/usr/lib -lc ${CURLLIB} ${BALEDIRLIB} ${GTKLIB} -lgthread-2.0 -ljson-c

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -DTICPREFIX=\"${TICPREFIX}\"
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -g ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
