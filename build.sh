#!/bin/bash

g++ -g -std=c++11 -O3 -Wall -Wextra -Wpedantic -o skim skim.cxx $(root-config --cflags --libs)
