#include "SPP_event.hpp"
#include "Framework/GHEP/GHepParticle.h"
#include "THnSparse.h"
#include "TMath.h"
#include "single_pion.hpp"
#include <ios>
#include <unistd.h>

Event_charac Utils::spp_read_events(const fs::path &filename) 
{
  TFile *event_file = TFile::Open(filename.c_str());
  if (!event_file || event_file->IsZombie()) 
  {
    pLOG("Utils::spp_read_events", pERROR)
        << "Cannot open event_file: " << filename << '\n';
    return {};
  }

  genie::NtpMCEventRecord *nt_event = nullptr;
  TTree *event_tree = (TTree *)event_file->Get("gtree");
  if (!event_tree) 
  {
    pLOG("Utils::spp_read_events", pERROR) << "Could not find 'gtree' in file";
    event_file->Close();
    return {};
  }
  event_tree->SetBranchAddress("gmcrec", &nt_event);
// Egiyan-like binned hist : 
  const int ndim = 4;
  int nbins[ndim] = {25, 4, 12, 12};
  double xmin[ndim] = {1.10, 0.25, 0.0, 0.0};
  double xmax[ndim] = {1.60, 0.65, 180.0, 360.0};
  THnSparseD* h_egiyan =  new THnSparseD
  (
      "h_egiyan",
      "ep→e′π⁺n cross section;W (GeV);Q² (GeV²);theta* (deg);phi* (deg)",
      ndim,
      nbins,
      xmin,
      xmax
  );
    THnSparseD* h_gamma =  new THnSparseD
  (
      "h_gamma",
      "Gamma(GeV)",
      ndim,
      nbins,
      xmin,
      xmax
  );
  Long64_t n_entries = event_tree->GetEntries();

  for (Long64_t i = 0; i < n_entries; ++i) 
  {
    event_tree->GetEntry(i);
    if (!nt_event) 
    {
      pLOG("Utils::spp_read_events", pERROR) << "Null event at entry " << i;
      continue;
    }

    genie::EventRecord *ev_rec = nt_event->event;
    if (!ev_rec) 
    {
      pLOG("Utils::spp_read_events", pERROR)
          << "Null EventRecord inside NtpMCEventRecord";
      continue;
    }

    genie::Interaction *interact = ev_rec->Summary();
    if (!interact->ProcInfo().IsEM()) 
    {
      pLOG("Utils::spp_read_events", pERROR) << "Got a non-EM event";
      continue;
    }

    // Hadronic categorization
    std::vector<genie::GHepParticle *> pip, pi0, pim, pr, ne, oth, leptons;
    TLorentzVector tot_had, PLepton_Lorentz;
    double TotPLonTrack = 0;

    TObjArrayIter p_iter(ev_rec);
    genie::GHepParticle *current_particle = nullptr;

    while ((current_particle = (genie::GHepParticle *)p_iter.Next()))
     {
      if (genie::pdg::IsLepton(current_particle->Pdg()))  
      {
        leptons.push_back(current_particle);
        // continue;  // Need to not skip lepton and keep their kinematics.
      }
      if (current_particle->Status() != genie::kIStStableFinalState)
        continue;

      if (current_particle->Charge() != 0)
        TotPLonTrack += current_particle->Pz();

      // if (genie::pdg::IsLepton(current_particle->Pdg()))
      //     leptons.push_back(current_particle);

      tot_had += *(current_particle->GetP4());

      switch (current_particle->Pdg()) 
      {
        case genie::kPdgPiP:
          pip.push_back(current_particle);
          break;
        case genie::kPdgPi0:
          pi0.push_back(current_particle);
          break;
        case genie::kPdgPiM:
          pim.push_back(current_particle);
          break;
        case genie::kPdgProton:
          pr.push_back(current_particle);
          break;
        case genie::kPdgNeutron:
          ne.push_back(current_particle);
          break;
        default:
          oth.push_back(current_particle);
          break;
      }
    }

    // The next step is now to select the events with exactly  1e  + 1 n + 1pi_+
    // in the FS.
    if (pip.size() == 1 && ne.size() == 1 && pi0.empty() && pim.empty() &&
        pr.empty() && oth.size() == 1 && oth[0]->Pdg() == genie::kPdgElectron)
    {
      //----------------------< Getting the kinematic vars from the events>------------------//
      TLorentzVector k_in, k_out;
      bool found_init = false, found_final = false;

      for (auto *lep : leptons)
      {
        if (!lep)
          continue;
        if (lep->Status() == genie::kIStInitialState)
        {
          k_in = *(lep->GetP4());
          found_init = true;
        }

        if (lep->Status() == genie::kIStStableFinalState)
        {
          k_out = *(lep->GetP4());
          found_final = true;
        }
      }

      if (!found_init || !found_final) 
      {
        pLOG("Utils::spp_read_events", pERROR)
            << "Missing incoming or outgoing lepton. Skipping event.";
        continue;
      }

      double p_E = 0.938272;
      TLorentzVector q = k_in - k_out;
      double Q2 = -1.0 * q.M2();                           // GeV^2
      double nu = q.E();                                   // energy transfer
      double W2 = (q + TLorentzVector(0, 0, 0, p_E)).M2(); // (q + p_target)^2
      double W = std::sqrt(W2);

      //-------------------------------< Boost direction>-------------------------------//
      TLorentzVector p_target(0, 0, 0, p_E);
      TLorentzVector p_cm = q + p_target;
      TVector3 beta_cm = p_cm.BoostVector(); // Boost direction

      //-----------------------------------< Final state n and pi+>------------------------//
      genie::GHepParticle *pion = pip[0];   // only 1 guaranteed
      genie::GHepParticle *neutron = ne[0]; // only 1 guaranteed

      TLorentzVector p4_pion = *(pion->GetP4());
      TLorentzVector p4_neut = *(neutron->GetP4());
      // Boosting to CM :
      p4_pion.Boost(-beta_cm);
      p4_neut.Boost(-beta_cm);
      pLOG("Utils::spp_read_events", pNOTICE)
          << "Event # " << i << ", Electron PDG: " << ev_rec->Probe()->Pdg();

      pLOG("Utils::Is_proc_spp", pNOTICE) << " Bp4_pion_m2 : " << p4_pion.M2()
                                          << " Bp4_neut_m2 : " << p4_neut.M2();
      // Define coordinate system for CM angles
      TVector3 z_axis = q.Vect().Unit(); // virtual photon direction
      TVector3 y_axis = z_axis.Cross(k_in.Vect()).Unit(); // normal to scattering plane
      TVector3 x_axis = y_axis.Cross(z_axis).Unit(); // completes right-handed system

      TVector3 pion_cm_dir = p4_pion.Vect().Unit(); // direction of boosted pion

      // Compute angles
      double cos_theta_star = pion_cm_dir.Dot(z_axis);
      double theta_star = std::acos(cos_theta_star); // radians
      double phi_star   = std::atan2(pion_cm_dir.Dot(y_axis), pion_cm_dir.Dot(x_axis));
      if (phi_star < 0)
      {
        phi_star += 2 * M_PI; // force into [0, 2pi]
      }
      double theta_deg    = theta_star * 180.0 / M_PI;
      double phi_deg      = phi_star * 180.0 / M_PI;
      double fill_vals[4] = {W, Q2, theta_deg, phi_deg};

  // Theta e : 
      double cos_theta_e = k_in.Vect().Unit().Dot(k_out.Vect().Unit());
      double theta_e     = std::acos(cos_theta_e);
      // Virtual photon polarization : 
      double epsilon     = (1 + 2*( 1 + std::pow(nu,2)*Q2 *std::tan(theta_e/2) )) ;
      epsilon            = 1/epsilon; 
      // Virtual flux : 
      constexpr float alpha = (float)1/137 ; 
      double gamma =  alpha/(2.0 * std::pow(M_PI,2) * Q2 );
      gamma        *= (std::pow(W,2) - std::pow(p_E,2))/(2*p_E* std::pow(k_in.E(),2));
      gamma        *= (k_out.E())/(1 - epsilon);

      pLOG("Utils::Is_proc_spp", pNOTICE)
          << "W = " << W << ", Q2 = " << Q2
          << ", theta* = " << theta_deg
          << ", phi* = " << phi_deg;

      h_egiyan->Fill(fill_vals);
      h_gamma->Fill(fill_vals, gamma); // Sum of gamma weights


      // sigma(W, Q^2, theta*, phi*) = (N_bin / N_total_events) × (sigma_total / bin_width)
      for (size_t j = 0; j < ev_rec->GetEntries(); ++j)
      {
        genie::GHepParticle *p = ev_rec->Particle(j);
        pLOG("Utils::Is_proc_spp", pNOTICE)
            << "Particle PDG: " << p->Pdg() << ", Status: " << p->Status()
            << ", 4-momentum: (" << p->Px() << ", " << p->Py() << ", "
            << p->Pz() << ", " << p->E() << ")";
      }
      // if (!oth.empty())
      // {
      //     pLOG("Utils::Is_proc_spp",pERROR) << "Got other particles in the
      //     event_record"; continue;
      // }

      pLOG("Utils::Is_proc_spp", pNOTICE)
          << "Event has a hadronic system with " << pr.size() << " protons, "
          << ne.size() << " neutrons, " << pip.size() << " pi+'s, "
          << pi0.size() << " pi0's, " << pim.size() << " pi-'s and "
          << oth.size() << " other particles";
    }
    nt_event->Clear(); // Clear current event before next loop
  }

  event_file->Close();
  return { h_egiyan,h_gamma,n_entries } ; 
}

std::optional<std::vector<CrossSectionBin>> Utils::xsec_from_spline(const fs::path & event_file_path,const fs::path &root_file_path,double energy_GeV ) 
{
  TFile *xsec_file = TFile::Open(root_file_path.c_str());
  if (!xsec_file || xsec_file->IsZombie())
  {
    pLOG("xsec_from_spline", pERROR) << "Could not open xsec file\n";
    return {};
  }

  // Read the TDirectory name in spline file :
  
  TDirectory *xsec_dir = (TDirectory *)xsec_file->Get("e-_H1");
  if (!xsec_dir)
  {
    pLOG("xsec_from_spline", pERROR) << "No directory named e-_H1 in file\n";
    return {};
  }

  TGraph *xsec_graph = (TGraph *)xsec_dir->Get("tot_em");
  if (!xsec_graph)
  {
    pLOG("xsec_from_spline", pERROR) << "No TGraph named tot_em in e-_H1\n";
    return {};
  }

  // Interpolate at a given energy : in this case around 1.5 GeV
  
  double sigma_tot = xsec_graph->Eval(energy_GeV);
  pLOG("Utils::xsec_from_spine", pNOTICE)<< "sig(E=" << energy_GeV << " GeV) = " << sigma_tot << " cm²";
  sigma_tot = sigma_tot * 1e-38;// convert to cm²
  //Egyain-like binning : of Q2  , W , Theta and Phi :
  double Q2_edges[5] = {0.25, 0.35, 0.45, 0.55, 0.65};
  double W_edges[26];
  double theta_edges_deg[13];
  double phi_edges_deg[13];

  for (int i = 0; i <= 25; ++i) 
  {
      W_edges[i] = 1.10 + 0.02 * i;
  }
  for (int i = 0; i <= 12; ++i)
  {
      theta_edges_deg[i] = 15.0 * i;
      phi_edges_deg[i] = 30.0 * i;
  }

  const int ndim    = 4;
  int nbins[ndim]   = {25, 4, 12, 12};
  double xmin[ndim] = {1.10, 0.25, 0.0, 0.0};
  double xmax[ndim] = {1.60, 0.65, 180.0, 360.0};

  auto [ h_egiyan ,h_gamma, N_total ] = spp_read_events(event_file_path);

  h_egiyan->SetBinEdges(0, W_edges);
  h_egiyan->SetBinEdges(1, Q2_edges);
  h_egiyan->SetBinEdges(2, theta_edges_deg);
  h_egiyan->SetBinEdges(3, phi_edges_deg);
  double delta_W  = 0.02;      // GeV
  double delta_Q2 = 0.10;     // GeV²
  // double sigma_tot = 1e-30;   // total cross section in cm^2 from spline
  std::vector<double>  sigma_W(25,0.0f) ; 
  double sigma_sum = 0.0f ;
  std::vector<CrossSectionBin> xsec_bins;
  std::cout << "sigma_tot = " << sigma_tot << std::endl;

  for (int iW = 0; iW < 25; ++iW) 
  {
      sigma_sum = 0.0f; // reset sigma_sum    
      for (int iQ2 = 0; iQ2 < 4; ++iQ2) 
      {
          for (int i_theta = 0; i_theta < 12; ++i_theta) 
          {
              for (int i_phi = 0; i_phi < 12; ++i_phi) 
              {
                  int bins[4]  = { iW , iQ2 , i_theta , i_phi };
                  double N_bin = h_egiyan->GetBinContent(bins);
                  // Solid angle in steradians
                  double theta_min_deg = theta_edges_deg[i_theta];
                  double theta_max_deg = theta_edges_deg[i_theta+1];
                  double phi_min_deg   = phi_edges_deg[i_phi];
                  double phi_max_deg   = phi_edges_deg[i_phi+1];

                  double theta_min_rad = theta_min_deg * TMath::DegToRad();
                  double theta_max_rad = theta_max_deg * TMath::DegToRad();
                  double phi_min_rad   = phi_min_deg * TMath::DegToRad();
                  double phi_max_rad   = phi_max_deg * TMath::DegToRad();

                  double dOmega = (phi_max_rad - phi_min_rad)* (cos(theta_min_rad) - cos(theta_max_rad));
                  double bin_volume = delta_W * delta_Q2 * dOmega;

                  if (N_bin > 0) 
                  {
                      double gamma_sum = h_gamma->GetBinContent(bins);
                      if (gamma_sum > 0 && bin_volume > 0) 
                      {
                        double d_sigma = (N_bin / (gamma_sum * bin_volume)) * sigma_tot;
                        double d_sigma_stat_unc = (std::sqrt(N_bin) / (gamma_sum * bin_volume)); //* 1e33;
                        double d_sigma_ub = d_sigma * 1e30;  // cm² → μb
                        double d_sigma_stat_unc_ub = d_sigma_stat_unc * 1e30;

                        sigma_sum += d_sigma * delta_Q2 * dOmega;

                        CrossSectionBin bin = {
                        0.5 * (W_edges[iW] + W_edges[iW+1]),
                        0.5 * (Q2_edges[iQ2] + Q2_edges[iQ2+1]),
                        0.5 * (theta_min_deg + theta_max_deg),
                        0.5 * (phi_min_deg + phi_max_deg),
                        d_sigma_ub,//d_sigma,
                        d_sigma_stat_unc_ub//d_sigma_stat_unc
                        };
                        xsec_bins.push_back(bin);
                       std::cout << "N_bin=" << N_bin 
                        << ", gamma_sum=" << gamma_sum 
                        << ", bin_volume=" << bin_volume 
                        << ", sigma_tot=" << sigma_tot 
                        << ", d_sigma=" << d_sigma 
                        << '\n';


                      }

                      // sigma_sum += d_sigma * delta_Q2 * dOmega;
                      // double d_sigma_stat_unc = (std::sqrt(N_bin) / N_total) * (sigma_tot / bin_volume);
                      // CrossSectionBin bin = 
                      //       {
                      //         0.5 * (W_edges[iW] + W_edges[iW+1]),
                      //         0.5 * (Q2_edges[iQ2] + Q2_edges[iQ2+1]),
                      //         0.5 * (theta_min_deg + theta_max_deg),
                      //         0.5 * (phi_min_deg + phi_max_deg),
                      //         d_sigma,
                      //         d_sigma_stat_unc
                      //       };
                      // xsec_bins.push_back(bin);
                }
        }
      }
    }
      sigma_W[iW] = sigma_sum;
  }

  std::vector<double> W_centers;
  for (int iW = 0; iW < sigma_W.size()/* 25 */; ++iW) 
  {
    W_centers.push_back( 0.5 * ( W_edges[iW] + W_edges[iW+1] ) );
  }
  // for (const auto& bin : xsec_bins) 
  // {
  //   std::cout << Form("W=%.3f Q²=%.3f θ*=%.1f° φ*=%.1f°  σ=%.3e cm²/sr\n",
  //                   bin.W, bin.Q2, bin.theta_deg, bin.phi_deg, bin.d_sigma_cm2_per_sr);
  // }
  return {xsec_bins};
}