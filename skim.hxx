#pragma once

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include <string>
#include <vector>

namespace Skim {
    const std::string sampleBasePath = "/home/stefan/cms_opendata_samples/";

    const std::vector<std::string> sampleNames = {
        "DYJetsToLL",
    };

    const std::vector<std::string> finalVariables = {
        "nMuon",
        "nTau",
    };

    template <typename T>
    auto MinimalSelection(T& df) {
        auto df2 = df.Filter("nMuon > 0", "nMuon > 0");
        auto df3 = df2.Filter("nTau > 0", "nTau > 0");
        return df3;
    }

    template <typename T>
    auto BuildMuonTauPair(T& df) {
        return df;
    }
}
