#include <numeric>
#include <TSpectrum.h>
#include <TFile.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <cmath>
#include <vector>
#include <tuple>
#include <deque>

#include "TH1F.h"
#include "TTree.h"
#include "TTimeStamp.h"
#include "TGraph.h"

int BinsNumber = 200;

using namespace std;

void execute (string filename, string treename, string outfilename) {

    // --- Prepare output file and tree ---
    TFile *file_out = TFile::Open(outfilename.c_str(), "RECREATE");
    TTree *pair_tree = new TTree("PromptDelayedPairs", "Prompt-Delayed Event Pairs");

    int nMuonsTotal = 0;

    // Variables for prompt and delayed events
    Float_t prompt_JRecoX, prompt_JRecoY, prompt_JRecoZ;
    Float_t prompt_deltaT_muon, prompt_RecoEnergy, prompt_NPE;
    Int_t   prompt_fSec, prompt_fNanoSec;

    Float_t delayed_JRecoX, delayed_JRecoY, delayed_JRecoZ;
    Float_t delayed_deltaT_muon, delayed_RecoEnergy, delayed_NPE;
    Int_t   delayed_fSec, delayed_fNanoSec;
    Int_t   prompt_Nhit, delayed_Nhit;

    Float_t pair_dt_us;

    pair_tree->Branch("prompt_JRecoX", &prompt_JRecoX, "prompt_JRecoX/F");
    pair_tree->Branch("prompt_JRecoY", &prompt_JRecoY, "prompt_JRecoY/F");
    pair_tree->Branch("prompt_JRecoZ", &prompt_JRecoZ, "prompt_JRecoZ/F");
    pair_tree->Branch("prompt_deltaT_muon", &prompt_deltaT_muon, "prompt_deltaT_muon/F");
    pair_tree->Branch("prompt_RecoEnergy", &prompt_RecoEnergy, "prompt_RecoEnergy/F");
    pair_tree->Branch("prompt_NPE", &prompt_NPE, "prompt_NPE/F");
    pair_tree->Branch("prompt_fSec", &prompt_fSec, "prompt_fSec/I");
    pair_tree->Branch("prompt_fNanoSec", &prompt_fNanoSec, "prompt_fNanoSec/I");
    pair_tree->Branch("prompt_Nhit", &prompt_Nhit, "prompt_Nhit/I");

    pair_tree->Branch("delayed_JRecoX", &delayed_JRecoX, "delayed_JRecoX/F");
    pair_tree->Branch("delayed_JRecoY", &delayed_JRecoY, "delayed_JRecoY/F");
    pair_tree->Branch("delayed_JRecoZ", &delayed_JRecoZ, "delayed_JRecoZ/F");
    pair_tree->Branch("delayed_deltaT_muon", &delayed_deltaT_muon, "delayed_deltaT_muon/F");
    pair_tree->Branch("delayed_RecoEnergy", &delayed_RecoEnergy, "delayed_RecoEnergy/F");
    pair_tree->Branch("delayed_NPE", &delayed_NPE, "delayed_NPE/F");
    pair_tree->Branch("delayed_fSec", &delayed_fSec, "delayed_fSec/I");
    pair_tree->Branch("delayed_fNanoSec", &delayed_fNanoSec, "delayed_fNanoSec/I");
    pair_tree->Branch("delayed_Nhit", &delayed_Nhit, "delayed_Nhit/I");

    pair_tree->Branch("pair_dt_us", &pair_dt_us, "pair_dt_us/F");

	TFile *fin = new TFile (filename.c_str());
    TTree* intree = (TTree*)fin->Get(treename.c_str());

	std::vector<int>* PMTID = new std::vector<int>;
	std::vector<float>* Charge = new std::vector<float>;
	std::vector<float>* Time = new std::vector<float>;

    int fSec, fNanoSec, muonTag, Nhit;
	float npe, deltaT_muon, RecoEnergy;
	float JRecoX, JRecoY, JRecoZ;

	intree -> SetBranchAddress("fSec",&fSec);
	intree -> SetBranchAddress("fNanoSec",&fNanoSec);
	intree -> SetBranchAddress("PMTID",&PMTID);
    intree -> SetBranchAddress("Charge",&Charge);
    intree -> SetBranchAddress("Time",&Time);
    intree -> SetBranchAddress("npe",&npe);
    intree -> SetBranchAddress("Nhit",&Nhit);
    intree -> SetBranchAddress("deltaT_muon",&deltaT_muon);
	intree -> SetBranchAddress("JRecoX",&JRecoX);
	intree -> SetBranchAddress("JRecoY",&JRecoY);
	intree -> SetBranchAddress("JRecoZ",&JRecoZ);
	intree -> SetBranchAddress("RecoEnergy_MP",&RecoEnergy);
    intree -> SetBranchAddress("muonTag",&muonTag);


    int TotalEvents = intree -> GetEntries();
    struct EventInfo {
        float JRecoX, JRecoY, JRecoZ;
        float deltaT_muon, RecoEnergy, NPE;
        int fSec, fNanoSec;
    };

   std::deque<EventInfo> window;

    for (int i = 0; i < TotalEvents; i++) {
        intree->GetEntry(i);

        if (muonTag == 1) nMuonsTotal++;

        if (deltaT_muon < 0.002 || muonTag != 0) continue;

        EventInfo evt{JRecoX, JRecoY, JRecoZ,
                    deltaT_muon, RecoEnergy,
                    npe, fSec, fNanoSec};

        long long t_evt = static_cast<long long>(evt.fSec) * 1000000000LL + evt.fNanoSec;

        // Remove old events (more than 5 µs before current)
        while (!window.empty()) {
            long long t_old = static_cast<long long>(window.front().fSec) * 1000000000LL + window.front().fNanoSec;
            if ((t_evt - t_old) / 1000.0 > 5.0) {
                window.pop_front();
            } else {
                break;
            }
        }

        // Compare with remaining candidates in window
        for (auto &cand : window) {
            long long t_cand = static_cast<long long>(cand.fSec) * 1000000000LL + cand.fNanoSec;
            float dt_us = (t_evt - t_cand) / 1000.0;
            if (dt_us > 0 && dt_us < 5.0) {
                // Fill with cand = prompt, evt = delayed
                prompt_JRecoX = cand.JRecoX;
                prompt_JRecoY = cand.JRecoY;
                prompt_JRecoZ = cand.JRecoZ;
                prompt_deltaT_muon = cand.deltaT_muon;
                prompt_RecoEnergy = cand.RecoEnergy;
                prompt_NPE = cand.NPE;
                prompt_fSec = cand.fSec;
                prompt_fNanoSec = cand.fNanoSec;

                delayed_JRecoX = evt.JRecoX;
                delayed_JRecoY = evt.JRecoY;
                delayed_JRecoZ = evt.JRecoZ;
                delayed_deltaT_muon = evt.deltaT_muon;
                delayed_RecoEnergy = evt.RecoEnergy;
                delayed_NPE = evt.NPE;
                delayed_fSec = evt.fSec;
                delayed_fNanoSec = evt.fNanoSec;

                pair_dt_us = dt_us;
                pair_tree->Fill();
            }
        }

        // Add current event to window
        window.push_back(evt);
    }

    fin->Close();
    file_out->cd();
    pair_tree->Write();

    // --- Read runLength from TH1D 'lt' in input file ---
    TFile* fin_lt = new TFile(filename.c_str(), "READ");
    TH1D* ltHist = nullptr;
    fin_lt->GetObject("lt", ltHist);
    float runLength = 0;
    if (ltHist) {
        runLength = ltHist->GetSum(); // total sum of bin contents (livetime in seconds)
    } else {
        std::cerr << "WARNING: Histogram 'lt' not found in input file. Setting runLength = 0." << std::endl;
    }
    fin_lt->Close();

    // --- Write summary tree ---
    TTree* summaryTree = new TTree("summary", "Run summary");
    summaryTree->Branch("nMuonsTotal", &nMuonsTotal, "nMuonsTotal/I");
    summaryTree->Branch("runLength", &runLength, "runLength/F");
    summaryTree->Fill();
    summaryTree->Write();

    file_out -> Close();

}


int main(int argc, char** argv) {

    string macro = argv[0];

    if(argc!=4) {
            cout << "\n     USAGE:  "<< macro << " Input_Rootfile  In_Tree_Name Out_File_Name \n" << endl;
            return 1;
    }

    string filename = argv[1];
	string intreename = argv[2];
	string outfilename = argv[3];

	execute (filename, intreename, outfilename);

    return 0;
}
