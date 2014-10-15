# tic - a simple twitch notification icon.
# See LICENSE file for copyright and license details.

include config.mk

SRC = tic.c io.c twitch.c
OBJ = ${SRC:.c=.o}

all: options tic

options:
	@echo tic build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

tic: ${OBJ}
	@echo CC -o $@ ${OBJ}
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f tic ${OBJ} tic-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p tic-${VERSION}
	@cp -R LICENSE Makefile config.mk config.def.h README \
		tic.1 ${SRC} tic-${VERSION}
	@tar -cf tic-${VERSION}.tar tic-${VERSION}
	@gzip tic-${VERSION}.tar
	@rm -rf tic-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f tic ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/tic
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < tic.1 > ${DESTDIR}${MANPREFIX}/man1/tic.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/tic.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/tic
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/tic.1

.PHONY: all options clean dist install uninstall
