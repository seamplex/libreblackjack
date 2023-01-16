# Copyright (C) 2020-2021 jeremy theler
# This file is part of libreblackjack.
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

# SUBDIRS = doc po
SUBDIRS = doc
AUTOMAKE_OPTIONS = subdir-objects
dist_doc_DATA = README doc/blackjack.texi

TESTS = check-internal.sh check-stand.sh
EXTRA_DIST = ChangeLog  players $(TESTS)

bin_PROGRAMS = blackjack
blackjack_INCLUDES = $(all_includes)
blackjack_LDADD = $(all_libraries) 

blackjack_SOURCES = \
 src/main.cpp \
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
 src/base.h \
 src/blackjack.h \
 src/conf.h \
 src/version-conf.h \
 src/version-vcs.h \
 src/players/stdinout.h \
 src/players/tty.h \
 src/players/basic.h \
 src/players/informed.h
 