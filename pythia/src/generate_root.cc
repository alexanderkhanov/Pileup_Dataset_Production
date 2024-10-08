#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TString.h"

#include "Pythia8/Pythia.h"

#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

#include <TRandom.h>

// Program Parameters
int nevents = 1000;     // Number of HS Events to Generate
int mu = 60;            // Average number of PU processes overlayed on HS event
double pTmin_jet = 25;  // Min pT used for clustering jets using anti-kt

// Prototype functions
void find_ip(double pT, double eta, double phi, double xProd, double yProd, double zProd, double& d0, double& z0);
int trace_origin(const Pythia8::Event& event, int ix, int& bcflag);

// Main pythia loop
int main()
{
    // Initialiaze output ROOT file
    TFile *output = new TFile("../output/dataset.root", "recreate");
    
    // Define local vars to be linked to TTree branches
    int id, status, ID, label;
    double pT, eta, phi, m, q, xProd, yProd, zProd, tProd, xDec, yDec, zDec, tDec;

    // Define tree with jets clustered using fast jet
    TTree *FastJet = new TTree("fastjet", "fastjet");
    std::vector<float> jet_pt, jet_eta, jet_phi, jet_m;
    FastJet->Branch("jet_pt", &jet_pt);
    FastJet->Branch("jet_eta", &jet_eta);
    FastJet->Branch("jet_phi", &jet_phi);
    FastJet->Branch("jet_m", &jet_m);

    std::vector<float> trk_pT, trk_eta, trk_phi;
    std::vector<float> trk_q, trk_d0, trk_z0;
    std::vector<int> trk_pid, trk_label, trk_origin, trk_bcflag;
    FastJet->Branch("trk_pT", &trk_pT);
    FastJet->Branch("trk_eta", &trk_eta);
    FastJet->Branch("trk_phi", &trk_phi);
    FastJet->Branch("trk_q", &trk_q);
    FastJet->Branch("trk_d0", &trk_d0);
    FastJet->Branch("trk_z0", &trk_z0);
    FastJet->Branch("trk_pid", &trk_pid);
    FastJet->Branch("trk_label", &trk_label);
    FastJet->Branch("trk_origin", &trk_origin);
    FastJet->Branch("trk_bcflag", &trk_bcflag);

    std::vector<int> jet_ntracks;
    std::vector<int> jet_track_index;
    FastJet->Branch("jet_ntracks", &jet_ntracks);
    FastJet->Branch("jet_track_index", &jet_track_index);

    // Configure HS Process
    Pythia8::Pythia pythia;
    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 14.e3");
    pythia.readString("Beams:allowVertexSpread = on");
    pythia.readString("Beams:sigmaVertexX = 0.3");
    pythia.readString("Beams:sigmaVertexY = 0.3");
    pythia.readString("Beams:sigmaVertexZ = 50.");

    // the process: ttbar
    //pythia.readString("Top:gg2ttbar = on");
    //pythia.readString("Top:qqbar2ttbar = on");
    // the process: Z'->tt
    pythia.readString("NewGaugeBoson:ffbar2gmZZprime = on");
    pythia.readString("32:onMode = off");
    pythia.readString("32:onIfAny = 6");
    pythia.readString("32:m0 = 1000.");

    // ttbar->l+jets
    //pythia.readString("24:onMode = off");
    //pythia.readString("24:onPosIfAny = 11 13");
    //pythia.readString("24:onNegIfAny = 1 2 3 4 5");
    // ttbar->allhad
    pythia.readString("24:onMode = off");
    pythia.readString("24:onIfAny = 1 2 3 4 5");
    pythia.init();

    // Configure PU Process
    Pythia8::Pythia pythiaPU;
    pythiaPU.readString("Beams:idA = 2212");
    pythiaPU.readString("Beams:idB = 2212");
    pythiaPU.readString("Beams:eCM = 14.e3");
    pythiaPU.readString("Beams:allowVertexSpread = on");
    pythiaPU.readString("Beams:sigmaVertexX = 0.3");
    pythiaPU.readString("Beams:sigmaVertexY = 0.3");
    pythiaPU.readString("Beams:sigmaVertexZ = 50.");
    pythiaPU.readString("SoftQCD:all = on");
    if (mu > 0) pythiaPU.init();

    // Configure HS Process
    //Pythia8::Pythia pythia;
    //pythia.readFile("ttbar.cmnd");
    //pythia.init();

    // Configure PU Process
    //Pythia8::Pythia pythiaPU;
    //pythiaPU.readFile("pileup.cmnd");
    //if (mu > 0) pythiaPU.init();

    // Configure antikt_algorithm
    std::map<TString, fastjet::JetDefinition> jetDefs;
    jetDefs["Anti-#it{k_{t}} jets, #it{R} = 0.4"] = fastjet::JetDefinition(fastjet::antikt_algorithm, 0.4, fastjet::E_scheme, fastjet::Best);

    // Start main event loop
    auto &event = pythia.event;
    for(int i=0;i<nevents;i++){

        if(!pythia.next()) continue;

        ID = 0;
        std::vector<float> event_trk_pT;
        std::vector<float> event_trk_eta;
        std::vector<float> event_trk_phi;
        std::vector<float> event_trk_q;
        std::vector<float> event_trk_d0;
        std::vector<float> event_trk_z0;
        std::vector<int> event_trk_pid;
        std::vector<int> event_trk_label;

        int entries = pythia.event.size();
        std::vector<Pythia8::Particle> ptcls_hs, ptcls_pu;
        std::vector<fastjet::PseudoJet> stbl_ptcls;

        // Add in hard scatter particles!
        for(int j=0;j<event.size();j++){
            auto &p = event[j];
            id = p.id();
            status = p.status();
            
            pT = p.pT();
            eta = p.eta();
            phi = p.phi();
            m = p.m();
            q = p.charge();
            xProd = p.xProd();
            yProd = p.yProd();
            zProd = p.zProd();
            tProd = p.tProd();
            xDec = p.xDec();
            yDec = p.yDec();
            zDec = p.zDec();
            tDec = p.tDec();

            label = -1; // HS Process

            double d0,z0; find_ip(pT,eta,phi,xProd,yProd,zProd,d0,z0);

            ID++;
            event_trk_pT.push_back(pT);
            event_trk_eta.push_back(eta);
            event_trk_phi.push_back(phi);
            event_trk_q.push_back(q);
            event_trk_d0.push_back(d0);
            event_trk_z0.push_back(z0);
            event_trk_pid.push_back(id);
            event_trk_label.push_back(label);

            if (not p.isFinal()) continue;
            fastjet::PseudoJet fj(p.px(), p.py(), p.pz(), p.e());
            fj.set_user_index(ID);
            stbl_ptcls.push_back(fj);
            ptcls_hs.push_back(p);
        }

        // Add in pileup particles!
        int n_inel = gRandom->Poisson(mu);
        printf("Overlaying particles from %i pileup interactions!\n", n_inel);
        for (int i_pu= 0; i_pu<n_inel; ++i_pu) {
            if (!pythiaPU.next()) continue;
              for (int j = 0; j < pythiaPU.event.size(); ++j) {
                auto &p = pythiaPU.event[j];
                id = p.id();
                status = p.status();

                pT = p.pT();
                eta = p.eta();
                phi = p.phi();
                m = p.m();
                q = p.charge();
                xProd = p.xProd();
                yProd = p.yProd();
                zProd = p.zProd();
                tProd = p.tProd();
                xDec = p.xDec();
                yDec = p.yDec();
                zDec = p.zDec();
                tDec = p.tDec();

                label = i_pu; // PU Process

                double d0,z0; find_ip(pT,eta,phi,xProd,yProd,zProd,d0,z0);

                ID++;
                event_trk_pT.push_back(pT);
                event_trk_eta.push_back(eta);
                event_trk_phi.push_back(phi);
                event_trk_q.push_back(q);
                event_trk_d0.push_back(d0);
                event_trk_z0.push_back(z0);
                event_trk_pid.push_back(id);
                event_trk_label.push_back(label);

                if (not p.isFinal()) continue;
                fastjet::PseudoJet fj(p.px(), p.py(), p.pz(), p.e());
                fj.set_user_index(ID);
                stbl_ptcls.push_back(fj);
                ptcls_pu.push_back(p);
            }
        }

        // prepare for filling
        jet_pt.clear();
        jet_eta.clear();
        jet_phi.clear();
        jet_m.clear();
    
        trk_pT.clear();
        trk_eta.clear();
        trk_phi.clear();
        trk_q.clear();
        trk_d0.clear();
        trk_z0.clear();
        trk_pid.clear();
        trk_label.clear();
    
        jet_ntracks.clear();
        jet_track_index.clear();
        int track_index = 0;

        // Cluster stable particles using anti-kt
        for (auto jetDef:jetDefs) {
          fastjet::ClusterSequence clustSeq(stbl_ptcls, jetDef.second);
          auto jets = fastjet::sorted_by_pt( clustSeq.inclusive_jets(pTmin_jet) );
          // For each jet:
          for (auto jet:jets) {
            jet_pt.push_back(jet.pt());
            jet_eta.push_back(jet.eta());
            jet_phi.push_back(jet.phi());
            jet_m.push_back(jet.m());

            // For each particle:
            jet_track_index.push_back(track_index);
            int ntracks = 0;
              for (auto trk:jet.constituents()) {
                int ix = trk.user_index()-1;
                trk_pT.push_back(event_trk_pT[ix]);
                trk_eta.push_back(event_trk_eta[ix]);
                trk_phi.push_back(event_trk_phi[ix]);
                trk_q.push_back(event_trk_q[ix]);
                trk_d0.push_back(event_trk_d0[ix]);
                trk_z0.push_back(event_trk_z0[ix]);
                trk_pid.push_back(event_trk_pid[ix]);
                trk_label.push_back(event_trk_label[ix]);
                int bcflag = 0;
                int origin = event_trk_label[ix]<0 ? trace_origin(event,ix,bcflag):-999
;
                trk_origin.push_back(origin);
                trk_bcflag.push_back(bcflag);
                ++ntracks;
              }
            jet_ntracks.push_back(ntracks);
            track_index += ntracks;
          }
        }
    FastJet->Fill();
    }

    output->Write();
    output->Close();

    return 0;
}

void find_ip(double pT, double eta, double phi, double xProd, double yProd, double zProd, double& d0, double& z0)
{
  // calculate IP

  double r = sqrt(pow(xProd,2) + pow(yProd,2));
  double dphi = phi - atan2(yProd,xProd);
  if (dphi>M_PI) dphi -= 2*M_PI;
  else if (dphi<=-M_PI) dphi += 2*M_PI;
  d0 = r*sin(dphi);
  z0 = zProd - r*sinh(eta);

  // smear according to resolution
  // tentative resolution parameterization: ATL-COM-PHYS-2021-377, Figs.12-13

  double sigma_d0 = sqrt(pow(80/pT,2) + pow(4,2))*1e-3; // um->mm
  double sigma_z0 = sqrt(pow(80/pT,2) + pow(10,2))*1e-3; // um->mm

  static TRandom3 rnd;
  d0 += rnd.Gaus(0,sigma_d0);
  z0 += rnd.Gaus(0,sigma_z0);
}

int trace_origin(const Pythia8::Event& event, int ix, int& bcflag) {
  // see if found W or top
  int id = event[ix].id();
  int ida = abs(id);
  if (ida==24 || ida==6) return id;
  // check b/c origin
  if (bcflag<5 && (ida/100==5 || ida/1000==5)) bcflag = 5;
  else if (bcflag<4 && (ida/100==4 || ida/1000==4)) bcflag = 4;
  // keep digging
  int mother1 = event[ix].mother1();
  int mother2 = event[ix].mother2();
  if (mother1==0) return 0;
  if (mother2==0 || mother2==mother1 || mother2<mother1) return trace_origin(event, mother1, bcflag);
  for (int j = mother1; j<=mother2; ++j) {
    // only trace quarks
    int ida = abs(event[j].id());
    if (ida>=1 && ida<=5) {
      int id = trace_origin(event, j, bcflag);
      if (abs(id)==24 || abs(id)==6) return id;
    }
  }
  // nothing good
  return 0;
}
