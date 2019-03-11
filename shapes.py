import ROOT


ranges = {
        "pt_1": (30, 80),
        "pt_2": (30, 80),
        "eta_1": (-2.4, 2.4),
        "eta_2": (-2.4, 2.4),
        "phi_1": (-3.14, 3.14),
        "phi_2": (-3.14, 3.14),
        "met": (0, 80),
        }


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


def main():
    ROOT.ROOT.EnableImplicitMT()
    tfile = ROOT.TFile("shapes.root", "RECREATE")

    variables = ranges.keys()
    nbins = 30
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
        hists = {}
        for variable in variables:
            hists[variable] = bookHistogram(df1, variable, nbins, ranges[variable])

        df2 = applyBaseline(df, lumi_, samesign=True)
        hists_ss = {}
        for variable in variables:
            hists_ss[variable] = bookHistogram(df2, variable, nbins, ranges[variable])

        for variable in variables:
            writeHistogram(hists[variable], "{}_{}".format(label, variable))
        for variable in variables:
            writeHistogram(hists_ss[variable], "{}_{}_ss".format(label, variable))

    tfile.Close()


if __name__ == "__main__":
    main()
