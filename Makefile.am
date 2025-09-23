# Copyright (C) 2020-2021, 2025 jeremy theler
# This file is part of libreblackjack.
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

# SUBDIRS = doc po
SUBDIRS = doc
AUTOMAKE_OPTIONS = subdir-objects
dist_doc_DATA = README doc/blackjack.texi

TESTS = tests/internal.sh \
        tests/stand.sh \
        tests/no-bust.sh \
        tests/mimic-the-dealer.sh \
        tests/variance.sh

EXTRA_DIST = ChangeLog  players tests utils

bin_PROGRAMS = blackjack
blackjack_INCLUDES = $(all_includes)
blackjack_LDADD = $(all_libraries) 

blackjack_SOURCES = \
 src/main.cpp \
 src/dealer.cpp \
 src/conf.cpp \
 src/report.cpp \
 src/version.cpp \
 src/blackjack.cpp \
 src/cards.cpp \
 src/players/stdinout.cpp \
 src/players/tty.cpp \
 src/players/basic.cpp \
 src/players/informed.cpp

noinst_HEADERS = \
 src/dealer.h \
 src/blackjack.h \
 src/conf.h \
 src/version-conf.h \
 src/version-vcs.h \
 src/players/stdinout.h \
 src/players/tty.h \
 src/players/basic.h \
 src/players/informed.h

.PHONY: dist-macos dist-macos-x86_64 dist-macos-arm64

MACOS_RELEASE_DIR = $(top_builddir)/binary-macos
MACOS_TARBALL_X86_64 = $(PACKAGE)-$(VERSION)-macos-x86_64.tar.gz
MACOS_TARBALL_ARM64  = $(PACKAGE)-$(VERSION)-macos-arm64.tar.gz

# Default target builds both
dist-macos: dist-macos-x86_64 dist-macos-arm64

dist-macos-x86_64: all
	@echo "==> Building x86_64 macos"
	rm -rf $(MACOS_RELEASE_DIR)-x86_64
	$(MAKE) distclean || true
	./configure CXXFLAGS="-O3 -arch x86_64" LDFLAGS="-arch x86_64"
	$(MAKE)
	$(MAKE) install DESTDIR=$(MACOS_RELEASE_DIR)-x86_64
	cd $(MACOS_RELEASE_DIR)-x86_64 && \
	    tar czf $(top_builddir)/$(MACOS_TARBALL_X86_64) usr/local
	@echo "==> Created $(MACOS_TARBALL_X86_64)"

dist-macos-arm64: all
	@echo "==> Building arm64 macos"
	rm -rf $(MACOS_RELEASE_DIR)-arm64
	$(MAKE) distclean || true
	./configure CXXFLAGS="-O3 -arch arm64" LDFLAGS="-arch arm64"
	$(MAKE)
	$(MAKE) install DESTDIR=$(MACOS_RELEASE_DIR)-arm64
	cd $(MACOS_RELEASE_DIR)-arm64 && \
	    tar czf $(top_builddir)/$(MACOS_TARBALL_ARM64) usr/local
	@echo "==> Created $(MACOS_TARBALL_ARM64)"

