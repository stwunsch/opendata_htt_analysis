#!/bin/bash

FILE="skim.cxx"
OUT="skim.out"
shift
g++ -g -std=c++11 -O3 -Wall -Wextra -Wpedantic -o $OUT $FILE $(root-config --cflags --libs) "$@"
