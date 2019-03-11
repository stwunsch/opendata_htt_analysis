#!/usr/bin/env python

import ROOT
#ROOT.PyConfig.IgnoreCommandLineOptions = True
import argparse


def createDataFrame(name):
    return ROOT.ROOT.RDataFrame("Events", name + "Skim.root")


def applyBaseline(df, lumi, samesign=False):
    return df.Filter("q_1*q_2>0" if samesign else "q_1*q_2<0")\
             .Define("plotting_weight", "weight * {}".format(float(lumi)))


def bookHistogram(df, variable, nbins, range_):
    return df.Histo1D(ROOT.ROOT.RDF.TH1DModel(variable, variable, nbins, range_[0], range_[1]),\
                      variable, "plotting_weight")


def writeHistogram(h, name):
    h.SetName(name)
    h.Write()


def parseArguments():
    parser = argparse.ArgumentParser(description="Produce histograms for plotting")
    parser.add_argument("variable", type=str, help="Variable")
    parser.add_argument("nBins", type=int, help="Number of bins in histogram")
    parser.add_argument("rangeMin", type=float, help="Minimum of range in histogram")
    parser.add_argument("rangeMax", type=float, help="Maximum of range in histogram")
    return parser.parse_args()


def main(args):
    ROOT.ROOT.EnableImplicitMT()
    tfile = ROOT.TFile("shapes.root", "RECREATE")

    variable = args.variable
    nbins = args.nBins
    range_ = (args.rangeMin, args.rangeMax)
    lumi = 11580.0

    for name, label, lumi_ in [
            ("GluGluToHToTauTau", "ggH", lumi),
            ("VBF_HToTauTau", "qqH", lumi),
            ("W1JetsToLNu", "W1J", lumi),
            ("W2JetsToLNu", "W2J", lumi),
            ("W3JetsToLNu", "W3J", lumi),
            ("TTbar", "TT", lumi),
            ("DYJetsToLL", "ZLL", lumi),
            ("Run2012B_SingleMu", "dataRunB", 1.0),
        ]:
        df = createDataFrame(name)

        df1 = applyBaseline(df, lumi_)
        h1 = bookHistogram(df1, variable, nbins, range_)

        df2 = applyBaseline(df, lumi_, samesign=True)
        h2 = bookHistogram(df2, variable, nbins, range_)

        writeHistogram(h1, label)
        writeHistogram(h2, label + "_ss")

    tfile.Close()


if __name__ == "__main__":
    args = parseArguments()
    main(args)
