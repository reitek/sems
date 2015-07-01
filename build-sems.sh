#!/bin/sh

make  EXTRA_CXXFLAGS="-O3 -g -pipe -m32 -march=prescott"  exclude_modules="speex" all
