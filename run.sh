#!/bin/bash

# Clean-up workspace
rm -f skim.out *.root *.png *.pdf

# Build and run skimming
time bash build.sh
time ./skim.out

# Produce histograms for plotting
time python histograms.py

# Make plots
time python plot.py
