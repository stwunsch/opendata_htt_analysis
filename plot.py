import ROOT
#ROOT.PyConfig.IgnoreCommandLineOptions = True
import argparse


labels = {
        "pt_1": "Muon p_{T} / GeV",
        "pt_2": "Tau p_{T} / GeV",
        "eta_1": "Muon #eta",
        "eta_2": "Tau #eta",
        "phi_1": "Muon #phi",
        "phi_2": "Tau #phi",
        "met": "Missing transverse energy / GeV",
        }

colors = {
        "ggH": ROOT.TColor.GetColor("#BF2229"),
        "qqH": ROOT.TColor.GetColor("#00A88F"),
        "TT": ROOT.TColor.GetColor(155, 152, 204),
        "W": ROOT.TColor.GetColor(222, 90, 106),
        "QCD":  ROOT.TColor.GetColor(250, 202, 255),
        "ZLL": ROOT.TColor.GetColor(248, 206, 104),
        }


def getHistogram(tfile, name, variable, tag=""):
    name = "{}_{}{}".format(name, variable, tag)
    h = tfile.Get(name)
    if not h:
        raise Exception("Failed to load histogram {}.".format(name))
    return h


def main(variable):
    tfile = ROOT.TFile("shapes.root", "READ")

    # Styles
    ROOT.gStyle.SetOptStat(0)

    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasColor(ROOT.kWhite)
    ROOT.gStyle.SetCanvasDefH(600)
    ROOT.gStyle.SetCanvasDefW(600)
    ROOT.gStyle.SetCanvasDefX(0)
    ROOT.gStyle.SetCanvasDefY(0)

    ROOT.gStyle.SetPadTopMargin(0.08)
    ROOT.gStyle.SetPadBottomMargin(0.13)
    ROOT.gStyle.SetPadLeftMargin(0.16)
    ROOT.gStyle.SetPadRightMargin(0.05)

    ROOT.gStyle.SetHistLineColor(1)
    ROOT.gStyle.SetHistLineStyle(0)
    ROOT.gStyle.SetHistLineWidth(1)
    ROOT.gStyle.SetEndErrorSize(2)
    ROOT.gStyle.SetMarkerStyle(20)

    ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetTitleFont(42)
    ROOT.gStyle.SetTitleColor(1)
    ROOT.gStyle.SetTitleTextColor(1)
    ROOT.gStyle.SetTitleFillColor(10)
    ROOT.gStyle.SetTitleFontSize(0.05)

    ROOT.gStyle.SetTitleColor(1, "XYZ")
    ROOT.gStyle.SetTitleFont(42, "XYZ")
    ROOT.gStyle.SetTitleSize(0.05, "XYZ")
    ROOT.gStyle.SetTitleXOffset(1.00)
    ROOT.gStyle.SetTitleYOffset(1.60)

    ROOT.gStyle.SetLabelColor(1, "XYZ")
    ROOT.gStyle.SetLabelFont(42, "XYZ")
    ROOT.gStyle.SetLabelOffset(0.007, "XYZ")
    ROOT.gStyle.SetLabelSize(0.04, "XYZ")

    ROOT.gStyle.SetAxisColor(1, "XYZ")
    ROOT.gStyle.SetStripDecimals(True)
    ROOT.gStyle.SetTickLength(0.03, "XYZ")
    ROOT.gStyle.SetNdivisions(510, "XYZ")
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)

    ROOT.gStyle.SetPaperSize(20., 20.)
    ROOT.gStyle.SetHatchesLineWidth(5)
    ROOT.gStyle.SetHatchesSpacing(0.05)

    # Load and prepare histograms
    ggH = getHistogram(tfile, "ggH", variable)
    ggH.SetLineColor(colors["ggH"])
    scaleSignal = 100.0
    ggH.Scale(scaleSignal)

    qqH = getHistogram(tfile, "qqH", variable)
    qqH.SetLineColor(colors["qqH"])
    qqH.Scale(scaleSignal)

    W = getHistogram(tfile, "W1J", variable)
    W2J = getHistogram(tfile, "W2J", variable)
    W3J = getHistogram(tfile, "W3J", variable)
    W.Add(W2J)
    W.Add(W3J)
    W.SetFillColor(colors["W"])

    TT = getHistogram(tfile, "TT", variable)
    TT.SetFillColor(colors["TT"])

    ZLL = getHistogram(tfile, "ZLL", variable)
    ZLL.SetFillColor(colors["ZLL"])

    data = getHistogram(tfile, "dataRunB", variable)
    data.SetMarkerStyle(20)
    data.SetLineColor(ROOT.kBlack)

    QCD = data.Clone()
    for name in ["W1J", "W2J", "W3J", "TT", "ZLL"]:
        QCD.Add(getHistogram(tfile, name, variable, "_ss"), -1.0)
    for i in range(1, QCD.GetNbinsX() + 1):
        if QCD.GetBinContent(i) < 0.0:
            QCD.SetBinContent(i, 0.0)
    QCD.SetFillColor(colors["QCD"])

    stack = ROOT.THStack("", "")
    stack.Add(W)
    stack.Add(TT)
    stack.Add(ZLL)
    stack.Add(QCD)

    for x in [ggH, qqH]:
        x.SetLineWidth(3)

    for x in [QCD, TT, ZLL, W]:
        x.SetLineWidth(0)


    # Draw histograms
    c = ROOT.TCanvas("", "", 600, 600)

    stack.Draw("hist")
    name = data.GetTitle()
    if name in labels:
        title = labels[name]
    else:
        title = name
    stack.GetXaxis().SetTitle(title)
    stack.GetYaxis().SetTitle("N_{Events}")
    stack.SetMaximum(max(stack.GetMaximum(), data.GetMaximum()) * 1.4)
    stack.SetMinimum(1.0)
    ROOT.TGaxis.SetExponentOffset(-0.08, 0.0, "Y")

    ggH.Draw("HIST SAME")
    qqH.Draw("HIST SAME")

    data.Draw("E1P SAME")
    #scaleData = sum([x.Integral() for x in [W, TT, ZLL, QCD]]) / data.Integral()
    scaleData = 17.5
    data.Scale(scaleData)

    # Add legend
    legend = ROOT.TLegend(0.4, 0.75, 0.90, 0.90)
    legend.SetNColumns(2)
    legend.AddEntry(W, "W", "f")
    legend.AddEntry(TT, "TT", "f")
    legend.AddEntry(ZLL, "ZLL", "f")
    legend.AddEntry(QCD, "QCD", "f")
    legend.AddEntry(ggH, "ggH (x{:.1f})".format(scaleSignal), "l")
    legend.AddEntry(qqH, "qqH (x{:.1f})".format(scaleSignal), "l")
    legend.AddEntry(data, "Data (x{:.1f})".format(scaleData), "lep")
    legend.SetBorderSize(0)
    legend.Draw()

    # Add title
    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.04)
    latex.SetTextFont(42)
    latex.DrawLatex(0.6, 0.935, "11.6 fb^{-1} (2012, 8 TeV)")
    latex.DrawLatex(0.16, 0.935, "#bf{CMS Open Data}")

    c.SaveAs("{}.pdf".format(variable))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot variable")
    parser.add_argument("-x", "--variable", default="", type=str, help="Variable")
    args = parser.parse_args()
    if args.variable in ["", "all"]:
        for variable in labels.keys():
            main(variable)
    else:
        main(args.variable)
