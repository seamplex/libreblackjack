#!/bin/bash -e

srcs="../src/conf.cpp"
grep "///help+${1}+desc" ${srcs} | cut -d" " -f2- | sed 's/@$//' | sed 's_/\\/_//_' | pandoc -t plain
