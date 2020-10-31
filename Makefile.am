# Copyright (C) 2020 jeremy theler
# This file is part of libreblackjack.
# This file is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

# SUBDIRS = doc po
AUTOMAKE_OPTIONS = subdir-objects
dist_doc_DATA = README doc/blackjack.texi

# TESTS = check-internal.sh check-stand.sh
# EXTRA_DIST = config.rpath m4/ChangeLog  blackjack.conf players $(TESTS)

ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = blackjack
blackjack_INCLUDES = $(all_includes)
blackjack_LDADD = $(all_libraries) 

blackjack_SOURCES = \
 src/main.cpp \
 src/blackjack.cpp \
 src/stdinout.cpp \
 src/tty.cpp
