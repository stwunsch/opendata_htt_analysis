#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TLorentzVector.h"
#include "TStopwatch.h"

#include <string>
#include <vector>
#include <iostream>

const std::string samplesBasePath = "/ceph/wunsch/opendata_samples/";

const std::vector<std::string> sampleNames = {
    "GluGluToHToTauTau",
    "VBF_HToTauTau",
    "DYJetsToLL",
    "TTbar",
    "W1JetsToLNu",
    "W2JetsToLNu",
    "W3JetsToLNu",
    "Run2012B_SingleMu",
    "Run2012C_SingleMu",
};

//const float integratedLuminosity = 4.412 * 1000.0; // Run2012B only
//const float integratedLuminosity = 7.055 * 1000.0; // Run2012C only
const float integratedLuminosity = 11.467 * 1000.0; // Run2012B+C
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
    return df.Filter("HLT_IsoMu17_eta2p1_LooseIsoPFTau20", "Passes trigger")
             .Filter("nMuon > 0", "nMuon > 0")
             .Filter("nTau > 0", "nTau > 0")
             .Filter("Muon_pt[0] < 1000", "Sanity check Muon_pt")
             .Filter("Tau_pt[0] < 1000", "Sanity check Tau_pt");
}

template <typename T>
auto FindGoodMuons(T &df) {
    return df.Define("goodMuons", "Muon_tightId == true && abs(Muon_eta) < 2.1 && Muon_pt > 25");
}

template <typename T>
auto FindGoodTaus(T &df) {
    return df.Define("goodTaus",
            "Tau_charge != 0 && abs(Tau_eta) < 2.4 && Tau_pt > 25 &&\
             Tau_idDecayMode && Tau_idIsoTight && Tau_idAntiEleTight && Tau_idAntiMuTight");
}


template <typename T>
auto FilterGoodEvents(T &df) {
    return df.Filter("Sum(goodTaus) > 0", "Event has good taus")
             .Filter("Sum(goodMuons) > 0", "Event has good muons");
}

template <typename T>
float DeltaPhi(T v1, T v2, const T c = M_PI)
{
    auto r = std::fmod(v2 - v1, 2.0 * c);
    if (r < -c) {
        r += 2.0 * c;
    }
    else if (r > c) {
        r -= 2.0 * c;
    }
    return r;
}

template <typename T>
auto FindMuonTauPair(T &df) {
    using namespace ROOT::VecOps;
    return df.Define("pairIdx",
                     [](RVec<int>& goodMuons, RVec<float>& pt_1, RVec<float>& eta_1, RVec<float>& phi_1,
                        RVec<int>& goodTaus, RVec<float>& iso_2, RVec<float>& eta_2, RVec<float>& phi_2)
                         {
                             // Get indices of all possible combinations
                             auto comb = Combinations(pt_1, eta_2);
                             const auto numComb = comb[0].size();

                             // Find valid pairs based on delta r
                             std::vector<int> validPair(numComb, 0);
                             for(size_t i = 0; i < numComb; i++) {
                                 const auto i1 = comb[0][i];
                                 const auto i2 = comb[1][i];
                                 if(goodMuons[i1] == 1 && goodTaus[i2] == 1) {
                                     const auto deltar = sqrt(
                                             pow(eta_1[i1] - eta_2[i2], 2) +
                                             pow(DeltaPhi(phi_1[i1], phi_2[i2]), 2));
                                     if (deltar > 0.5) {
                                         validPair[i] = 1;
                                     }
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
                             for(size_t i = 0; i < numComb; i++) {
                                 if(validPair[i] == 0) continue;
                                 if(int(comb[0][i]) != idx_1) continue;
                                 const auto tmp = comb[1][i];
                                 if(minIso > iso_2[tmp]) {
                                     minIso = iso_2[tmp];
                                     idx_2 = tmp;
                                 }
                             }

                             return std::vector<int>({idx_1, idx_2});
                         },
                     {"goodMuons", "Muon_pt", "Muon_eta", "Muon_phi",
                      "goodTaus", "Tau_relIso_all", "Tau_eta", "Tau_phi"})
             .Define("idx_1", "pairIdx[0]")
             .Define("idx_2", "pairIdx[1]")
             .Filter("idx_1 != -1", "Valid muon in selected pair")
             .Filter("idx_2 != -1", "Valid tau in selected pair");
}

template <typename T>
auto DeclareVariables(T &df) {
    auto add_p4 = [](float pt_1, float eta_1, float phi_1, float mass_1,
                           float pt_2, float eta_2, float phi_2, float mass_2)
    {
        TLorentzVector p1, p2;
        p1.SetPtEtaPhiM(pt_1, eta_1, phi_1, mass_1);
        p2.SetPtEtaPhiM(pt_2, eta_2, phi_2, mass_2);
        return p1 + p2;
    };

    using namespace ROOT::VecOps;
    auto get_first = [](RVec<float> &x, RVec<int>& g)
    {
        if (Sum(g) >= 1) return x[g][0];
        return -999.f;
    };
    auto get_second = [](RVec<float> &x, RVec<int>& g)
    {
        if (Sum(g) >= 2) return x[g][1];
        return -999.f;
    };
    auto compute_mjj = [](TLorentzVector& p4, RVec<int>& g)
    {
        if (Sum(g) >= 2) return float(p4.M());
        return -999.f;
    };
    auto compute_ptjj = [](TLorentzVector& p4, RVec<int>& g)
    {
        if (Sum(g) >= 2) return float(p4.Pt());
        return -999.f;
    };
    auto compute_jdeta = [](float x, float y, RVec<int>& g)
    {
        if (Sum(g) >= 2) return x - y;
        return -999.f;
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
             .Define("iso_2", "Tau_relIso_all[idx_2]")
             .Define("q_2", "Tau_charge[idx_2]")
             .Define("dm_2", "Tau_decayMode[idx_2]")
             .Define("met", "MET_pt")
             .Define("p4", add_p4,
                     {"pt_1", "eta_1", "phi_1", "m_1", "pt_2", "eta_2", "phi_2", "m_2"})
             .Define("m_vis", "float(p4.M())")
             .Define("pt_vis", "float(p4.Pt())")
             .Define("npv", "PV_npvs")
             .Define("goodJets", "Jet_puId == true && abs(Jet_eta) < 2.4 && Jet_pt > 20")
             .Define("njets", "Sum(goodJets)")
             .Define("jpt_1", get_first, {"Jet_pt", "goodJets"})
             .Define("jeta_1", get_first, {"Jet_eta", "goodJets"})
             .Define("jphi_1", get_first, {"Jet_phi", "goodJets"})
             .Define("jm_1", get_first, {"Jet_mass", "goodJets"})
             .Define("jbtag_1", get_first, {"Jet_btag", "goodJets"})
             .Define("jpt_2", get_second, {"Jet_pt", "goodJets"})
             .Define("jeta_2", get_second, {"Jet_eta", "goodJets"})
             .Define("jphi_2", get_second, {"Jet_phi", "goodJets"})
             .Define("jm_2", get_second, {"Jet_mass", "goodJets"})
             .Define("jbtag_2", get_second, {"Jet_btag", "goodJets"})
             .Define("jp4", add_p4,
                     {"jpt_1", "jeta_1", "jphi_1", "jm_1", "jpt_2", "jeta_2", "jphi_2", "jm_2"})
             .Define("mjj", compute_mjj, {"jp4", "goodJets"})
             .Define("ptjj", compute_ptjj, {"jp4", "goodJets"})
             .Define("jdeta", compute_jdeta, {"jeta_1", "jeta_2", "goodJets"});
}

template <typename T>
auto AddEventWeight(T &df, const std::string& sample) {
    const auto weight = eventWeights[sample];
    return df.Define("weight", [weight](){ return weight; });
}

const std::vector<std::string> finalVariables = {
    "njets", "npv",
    "pt_1", "eta_1", "phi_1", "m_1", "iso_1", "q_1",
    "pt_2", "eta_2", "phi_2", "m_2", "iso_2", "q_2", "dm_2",
    "jpt_1", "jeta_1", "jphi_1", "jm_1", "jbtag_1",
    "jpt_2", "jeta_2", "jphi_2", "jm_2", "jbtag_2",
    "met", "m_vis", "pt_vis", "mjj", "ptjj", "jdeta",
    "run", "weight"
};

int main() {
    ROOT::EnableImplicitMT(24);
    const auto poolSize = ROOT::GetImplicitMTPoolSize();
    std::cout << "Pool size: " << poolSize << std::endl;

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
