#include "skim.hxx"

#include "ROOT/RDataFrame.hxx"

#include <iostream>

int main(void) {
    for (const auto& sample : Skim::sampleNames) {
        std::cout << "Process sample " << sample << ":" << std::endl;
        ROOT::RDataFrame df("Events", Skim::sampleBasePath + sample + ".root");
        auto df2 = Skim::MinimalSelection(df);
        auto df3 = Skim::BuildMuonTauPair(df2);

        auto dfFinal = df3;
        auto report = dfFinal.Report();
        dfFinal.Snapshot("Events", sample + "Skim.root", Skim::finalVariables);
        report->Print();
    }
}
