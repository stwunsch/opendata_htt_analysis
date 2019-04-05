import ROOT


default_nbins = 30
ranges = {
        "pt_1": (default_nbins, 25, 70),
        "pt_2": (default_nbins, 25, 70),
        "eta_1": (default_nbins, -2.1, 2.1),
        "eta_2": (default_nbins, -2.4, 2.4),
        "phi_1": (default_nbins, -3.14, 3.14),
        "phi_2": (default_nbins, -3.14, 3.14),
        "iso_1": (default_nbins, 0, 0.10),
        "iso_2": (default_nbins, 0, 0.10),
        "q_1": (2, -2, 2),
        "q_2": (2, -2, 2),
        "met": (default_nbins, 0, 60),
        "m_1": (default_nbins, 0, 0.2),
        "m_2": (default_nbins, 0, 2),
        "dm_2": (11, 0, 11),
        "m_vis": (default_nbins, 50, 130),
        "pt_vis": (default_nbins, 0, 60),
        "jpt_1": (default_nbins, 20, 70),
        "jpt_2": (default_nbins, 20, 70),
        "jeta_1": (default_nbins, -2.4, 2.4),
        "jeta_2": (default_nbins, -2.4, 2.4),
        "jphi_1": (default_nbins, -3.14, 3.14),
        "jphi_2": (default_nbins, -3.14, 3.14),
        "jm_1": (default_nbins, 0, 20),
        "jm_2": (default_nbins, 0, 20),
        "jbtag_1": (default_nbins, 0, 1.0),
        "jbtag_2": (default_nbins, 0, 1.0),
        "npv": (25, 5, 30),
        "njets": (5, 0, 5),
        "mjj": (default_nbins, 0, 300),
        "ptjj": (default_nbins, 0, 100),
        "jdeta": (default_nbins, -4.8, 4.8),
        }


def createDataFrame(name):
    return ROOT.ROOT.RDataFrame("Events", name + "Skim.root")


def bookHistogram(df, variable, range_):
    return df.Histo1D(ROOT.ROOT.RDF.TH1DModel(variable, variable, range_[0], range_[1], range_[2]),\
                      variable, "weight")


def writeHistogram(h, name):
    h.SetName(name)
    h.Write()


def main():
    ROOT.ROOT.EnableImplicitMT(24)
    poolSize = ROOT.ROOT.GetImplicitMTPoolSize()
    print("Pool size: {}".format(poolSize))

    tfile = ROOT.TFile("shapes.root", "RECREATE")
    variables = ranges.keys()

    for name, label in [
            ("GluGluToHToTauTau", "ggH"),
            ("VBF_HToTauTau", "qqH"),
            ("W1JetsToLNu", "W1J"),
            ("W2JetsToLNu", "W2J"),
            ("W3JetsToLNu", "W3J"),
            ("TTbar", "TT"),
            ("DYJetsToLL", "ZLL"),
            ("Run2012B_SingleMu", "dataRunB"),
            ("Run2012C_SingleMu", "dataRunC"),
        ]:
        df = createDataFrame(name)

        df1 = df.Filter("q_1*q_2<0")
        hists = {}
        for variable in variables:
            hists[variable] = bookHistogram(df1, variable, ranges[variable])

        df2 = df.Filter("q_1*q_2>0")
        hists_ss = {}
        for variable in variables:
            hists_ss[variable] = bookHistogram(df2, variable, ranges[variable])

        for variable in variables:
            writeHistogram(hists[variable], "{}_{}".format(label, variable))
        for variable in variables:
            writeHistogram(hists_ss[variable], "{}_{}_ss".format(label, variable))

    tfile.Close()


if __name__ == "__main__":
    main()
