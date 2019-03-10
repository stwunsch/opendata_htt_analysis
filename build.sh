#!/bin/bash

FILE="skim.cxx"
OUT="skim.out"
shift
g++-7 -g -std=c++17 -O3 -Wall -Wextra -Wpedantic -o $OUT $FILE $(root-config --cflags --libs) "$@"
