#include "config.hxx"
#include "skim.hxx"

#include "ROOT/RDataFrame.hxx"

#include <iostream>

int main(void) {
    for (const auto& sample : Config::sampleNames) {
        ROOT::RDataFrame df("Events", Config::sampleBasePath + sample + ".root");
        auto df2 = Skim::MinimalSelection(df);
        auto df3 = Skim::BuildMuonTauPair(df2);
    }
}
