#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TLorentzVector.h"
#include "TStopwatch.h"

#include <string>
#include <vector>
#include <iostream>

const std::string samplesBasePath = "/home/stefan/cms_opendata_samples/";

const std::vector<std::string> sampleNames = {
    "GluGluToHToTauTau",
    "VBF_HToTauTau",
    "DYJetsToLL",
    "TTbar",
    "W1JetsToLNu",
    "W2JetsToLNu",
    "W3JetsToLNu",
    "Run2012B_SingleMu",
    //"Run2012C_SingleMu",
};

const float integratedLuminosity = 4.5 * 1000.0;
std::map<std::string, float> eventWeights = {
    {"GluGluToHToTauTau", 19.6 / 476963.0 * integratedLuminosity},
    {"VBF_HToTauTau", 1.55 / 491653.0 * integratedLuminosity},
    {"DYJetsToLL", 3503.7 / 30458871.0 * integratedLuminosity},
    {"TTbar", 225.2 / 6423106.0 * integratedLuminosity},
    {"W1JetsToLNu", 6381.2 / 29784800.0 * integratedLuminosity},
    {"W2JetsToLNu", 2039.8 / 30693853.0 * integratedLuminosity},
    {"W3JetsToLNu", 612.5 / 15241144.0 * integratedLuminosity},
    {"Run2012B_SingleMu", 1.0},
    {"Run2012C_SingleMu", 1.0},
};

template <typename T>
auto MinimalSelection(T &df) {
    return df.Filter("HLT_IsoMu17_eta2p1_LooseIsoPFTau20_v2", "Passes trigger")
             .Filter("nMuon > 0", "nMuon > 0")
             .Filter("nTau > 0", "nTau > 0")
             .Filter("Muon_pt[0] < 1000", "Sanity check Muon_pt")
             .Filter("Tau_pt[0] < 1000", "Sanity check Tau_pt");
}

template <typename T>
auto FindGoodMuons(T &df) {
    return df.Define("goodMuons", "Muon_tightId == true && abs(Muon_eta) < 2.4");
}

template <typename T>
auto FindGoodTaus(T &df) {
    return df.Define("goodTaus", "Tau_charge != 0 && Tau_decayMode >= 0 && abs(Tau_eta) < 2.4");
}


template <typename T>
auto FilterGoodEvents(T &df) {
    return df.Filter("Sum(goodTaus) > 0", "Event has good taus")
             .Filter("Sum(goodMuons) > 0", "Event has good muons");
}

template <typename T>
auto FindMuonTauPair(T &df) {
    using namespace ROOT::VecOps;
    return df.Define("pairIdx",
                     [](RVec<int>& goodMuons,
                        RVec<float>& pt_1, RVec<float>& eta_1, RVec<float>& phi_1,
                        RVec<int>& goodTaus,
                        RVec<float>& eta_2, RVec<float>& phi_2,
                        RVec<float>& niso, RVec<float>& ciso)
                         {
                             // Get indices of all possible combinations
                             auto comb = Combinations(pt_1, eta_2);
                             const auto numComb = comb[0].size();

                             // Find valid pairs based on delta r
                             std::vector<int> validPair(numComb);
                             auto deltar = sqrt(
                                     pow(Take(eta_1, comb[0]) - Take(eta_2, comb[1]), 2) +
                                     pow(Take(phi_1, comb[0]) - Take(phi_2, comb[1]), 2));
                             for(size_t i = 0; i < numComb; i++) {
                                 if(goodTaus[i] == 1 && goodMuons[i] == 1 && deltar[i] > 0.5) {
                                     validPair[i] = 1;
                                 } else {
                                     validPair[i] = 0;
                                 }
                             }

                             // Find best muon based on pt
                             int idx_1 = -1;
                             float maxPt = -1;
                             for(size_t i = 0; i < numComb; i++) {
                                 if(validPair[i] == 0) continue;
                                 const auto tmp = comb[0][i];
                                 if(maxPt < pt_1[tmp]) {
                                     maxPt = pt_1[tmp];
                                     idx_1 = tmp;
                                 }
                             }

                             // Find best tau based on iso
                             int idx_2 = -1;
                             float minIso = 999;
                             const auto iso = niso + ciso;
                             for(size_t i = 0; i < numComb; i++) {
                                 if(validPair[i] == 0) continue;
                                 if(int(comb[0][i]) != idx_1) continue;
                                 const auto tmp = comb[1][i];
                                 if(minIso > iso[tmp]) {
                                     minIso = iso[tmp];
                                     idx_2 = tmp;
                                 }
                             }

                             return std::vector<int>({idx_1, idx_2});
                         },
                     {"goodMuons",
                      "Muon_pt", "Muon_eta", "Muon_phi",
                      "goodTaus",
                      "Tau_eta", "Tau_phi",
                      "Tau_chargedIso", "Tau_neutralIso"})
             .Define("idx_1", "pairIdx[0]")
             .Define("idx_2", "pairIdx[1]")
             .Filter("idx_1 != -1", "Valid muon in selected pair")
             .Filter("idx_2 != -1", "Valid tau in selected pair");
}

template <typename T>
auto DeclareVariables(T &df) {
    auto compute_mass = [](float pt_1, float eta_1, float phi_1, float mass_1,
                           float pt_2, float eta_2, float phi_2, float mass_2)
    {
        TLorentzVector p1, p2;
        p1.SetPtEtaPhiM(pt_1, eta_1, phi_1, mass_1);
        p2.SetPtEtaPhiM(pt_2, eta_2, phi_2, mass_2);
        return (p1 + p2).M();
    };
    return df.Define("pt_1", "Muon_pt[idx_1]")
             .Define("eta_1", "Muon_eta[idx_1]")
             .Define("phi_1", "Muon_phi[idx_1]")
             .Define("m_1", "Muon_mass[idx_1]")
             .Define("iso_1", "Muon_pfRelIso03_all[idx_1]")
             .Define("q_1", "Muon_charge[idx_1]")
             .Define("pt_2", "Tau_pt[idx_2]")
             .Define("eta_2", "Tau_eta[idx_2]")
             .Define("phi_2", "Tau_phi[idx_2]")
             .Define("m_2", "Tau_mass[idx_2]")
             .Define("iso_2", "(Tau_chargedIso[idx_2] + Tau_neutralIso[idx_2]) / Tau_pt[idx_2]")
             .Define("q_2", "Tau_charge[idx_2]")
             .Define("dm_2", "Tau_decayMode[idx_2]")
             .Define("met", "MET_pt")
             .Define("m_vis", compute_mass, {"pt_1", "eta_1", "phi_1", "m_1", "pt_2", "eta_2", "phi_2", "m_2"});
}

template <typename T>
auto AddEventWeight(T &df, const std::string& sample) {
    const auto weight = eventWeights[sample];
    return df.Define("weight", [weight](){ return weight; });
}

const std::vector<std::string> finalVariables = {
    "nMuon", "nTau",
    "pt_1", "eta_1", "phi_1", "m_1", "iso_1", "q_1",
    "pt_2", "eta_2", "phi_2", "m_2", "iso_2", "q_2", "dm_2",
    "met", "m_vis",
    "weight"
};

int main() {
    for (const auto &sample : sampleNames) {
        std::cout << ">>> Process sample " << sample << ":" << std::endl;
        TStopwatch time;
        time.Start();

        ROOT::RDataFrame df("Events", samplesBasePath + sample + ".root");
        std::cout << "Number of events: " << *df.Count() << std::endl;

        auto df2 = MinimalSelection(df);
        auto df3 = FindGoodMuons(df2);
        auto df4 = FindGoodTaus(df3);
        auto df5 = FilterGoodEvents(df4);
        auto df6 = FindMuonTauPair(df5);
        auto df7 = DeclareVariables(df6);
        auto df8 = AddEventWeight(df7, sample);

        auto dfFinal = df8;
        auto report = dfFinal.Report();
        dfFinal.Snapshot("Events", sample + "Skim.root", finalVariables);
        time.Stop();

        report->Print();
        time.Print();
    }
}
