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
    "nMuon", "nTau",
    "pt_1", "eta_1", "phi_1", "m_1",
    "pt_2", "eta_2", "phi_2", "m_2",
    "met",
};

template <typename T>
auto MinimalSelection(T &df) {
    return df.Filter("nMuon > 0", "nMuon > 0")
             .Filter("nTau > 0", "nTau > 0")
             .Filter("Muon_pt[0] < 1000", "Sanity check Muon_pt")
             .Filter("Tau_pt[0] < 1000", "Sanity check Tau_pt");
}

template <typename T>
auto FindMuonTauPair(T &df) {
    return df.Define("idx_1", "0")
             .Define("idx_2", "0");
}

template <typename T>
auto DeclareVariables(T &df) {
    return df.Define("pt_1", "Muon_pt[idx_1]")
             .Define("eta_1", "Muon_eta[idx_1]")
             .Define("phi_1", "Muon_phi[idx_1]")
             .Define("m_1", "Muon_mass[idx_1]")
             .Define("pt_2", "Tau_pt[idx_2]")
             .Define("eta_2", "Tau_eta[idx_2]")
             .Define("phi_2", "Tau_phi[idx_2]")
             .Define("m_2", "Tau_mass[idx_2]")
             .Define("met", "MET_pt");
}

}
