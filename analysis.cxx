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
        auto df3 = Skim::FindMuonTauPair(df2);
        auto df4 = Skim::DeclareVariables(df3);

        auto dfFinal = df4;
        auto report = dfFinal.Report();
        dfFinal.Snapshot("ntuple", sample + "Skim.root", Skim::finalVariables);
        time.Stop();

        report->Print();
        time.Print();
    }
}
