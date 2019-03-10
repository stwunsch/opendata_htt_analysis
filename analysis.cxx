#include "skim.hxx"

#include "ROOT/RDataFrame.hxx"
#include "TStopwatch.h"

#include <iostream>

int main(void) {
    ROOT::EnableImplicitMT();

    for (const auto &sample : Skim::sampleNames) {
        std::cout << "Process sample " << sample << ":" << std::endl;
        TStopwatch time;
        time.Start();

        ROOT::RDataFrame df("Events", Skim::samplesBasePath + sample + ".root");
        auto df2 = Skim::MinimalSelection(df);
        auto df3 = Skim::FindGoodMuons(df2);
        auto df4 = Skim::FindGoodTaus(df3);
        auto df5 = Skim::FilterGoodEvents(df4);
        auto df6 = Skim::FindMuonTauPair(df5);
        auto df7 = Skim::DeclareVariables(df6);

        auto dfFinal = df7;
        auto report = dfFinal.Report();
        dfFinal.Snapshot("Events", sample + "Skim.root", Skim::finalVariables);
        time.Stop();

        report->Print();
        time.Print();
    }
}
