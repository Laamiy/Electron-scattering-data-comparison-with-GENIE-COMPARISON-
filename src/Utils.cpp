#include "Framework/Messenger/Messenger.h"
#include "SPP_event.hpp"
#include "single_pion.hpp"
#include <filesystem>
#include <sys/types.h>
#include <vector>

Cmd_args Utils::GetCommandLineArgs(int argc, char **argv)
{
  namespace fs = std::filesystem ; 

  std::string def_spp_model = "genie::DCCSPPPXSec/NoPauliBlock";
  fs::path input_data_path = "", event_file_path = "";
  fs::path xsec_file_path = "";
  
  pLOG("Utils", pINFO) << " Parsing command line arguments";
  genie::RunOpt::Instance()->ReadFromCommandLine(argc, argv);
  genie::CmdLnArgParser parser(argc, argv);

  // Get GENIE model names to be used :
  if (parser.OptionExists("resonance-xsec-model")) 
  {
    def_spp_model = parser.Arg("resonance-xsec-model");
  }
  // Get Egiyan like dataset path  :
  if (parser.OptionExists("input_file")) 
  {
    input_data_path = parser.Arg("input_file");
  }
  if (parser.OptionExists("event_file")) 
  {
    event_file_path = parser.Arg("event_file");
  }
  if (parser.OptionExists("xsec_file")) 
  {
    xsec_file_path = parser.Arg("xsec_file");
  }

  return {input_data_path, event_file_path, xsec_file_path, std::move(def_spp_model)};
}

void Utils::minus_uni_2_ascii(std::string &buffer, const std::string &uni_code,
                              const std::string &ascii_code) 
{
  // This replaces utf-8 '-' into ascii '-' :
  size_t pos = 0;
  while ((pos = buffer.find(uni_code, pos)) != std::string::npos)
 {
    buffer.replace(pos, uni_code.length(), ascii_code);
    pos += ascii_code.length(); // continue after the replacement
  }
};

std::optional< std::vector<Kinematics> > Utils ::Read_data_file(const fs::path &file_path) 
{
  std::ifstream reader;
  reader.open(file_path);

  if (!reader) 
  {
    pLOG("Utils", pERROR) << " Couldn't open file :"
                          << file_path.stem().c_str();
    return {};
  }
  double q2{}, W{}, epsilon{}, theta_pi{};
  double sig0{}, sig1{}, sig2{}, unc0{}, unc1{}, unc2{};

  std::string pm0, pm1, pm2;
  // 0.6 , 1.39 , 0.541 , 52.5 , 4.322 , ± , 0.44 , −0.049 , ± , 0.51 , −2.415 ,
  // ± , 0.34
  std::vector<Kinematics> data_kinematics{};
  data_kinematics.reserve(EXPECTED_SIZE);

  std::string current_line;
  std::getline(reader, current_line);
  std::stringstream stream;

  const std::string uni_code = u8"−"; // UTF-8 encoding of U+2212
  const std::string ascii_code = "-";

  while (std::getline(reader, current_line))
 {
    Utils::minus_uni_2_ascii(current_line, uni_code, ascii_code);
    stream.str(current_line);
    stream >> q2 >> W >> epsilon >> theta_pi >> sig0 >> pm0 >> unc0 >> sig1 >>
        pm1 >> unc1 >> sig2 >> pm2 >> unc2;
    data_kinematics.emplace_back(q2, W, epsilon, theta_pi, sig0, unc0, sig1,
                                 sig2);
    stream.clear();
  }
  return data_kinematics;
}

void Utils::Print_dataset(const std::vector<Kinematics> &data_kinematics,
                          const char *model_name, const fs::path &file_path,
                          bool save) 
{
  pLOG("Utils", pINFO) << " Dataset : ";
  std::vector<double> xsec, w, unc;
  for (auto &kine : data_kinematics) 
  {
    std::cout << kine;
    xsec.push_back(kine.sigma_0);
    w.push_back(kine.w);
    unc.push_back(kine.unc_0);
  }

  pLOG("Utils", pINFO) << " Dataset size : " << data_kinematics.size();
  pLOG("Utils", pINFO) << " Model name   : " << model_name;
  pLOG("Utils", pINFO) << " Egiyan data file path : " << file_path;
  if (save) 
  {
    TGraphErrors *graph =
        new TGraphErrors(xsec.size(),
                         w.data(),    // x-axis: W
                         xsec.data(), // y-axis: σ₀
                         nullptr,     // x-errors (none)
                         unc.data()   // y-errors (uncertainty on σ₀)
        );

    graph->SetTitle("Data with Uncertainties;Index;Measurement");
    graph->SetMarkerStyle(21);
    graph->SetMarkerColor(kRed + 1);
    graph->SetLineColor(kRed + 1); // set the Dataset line color ;

    TCanvas *canvas = new TCanvas("c", "Plot", 800, 600);
    graph->Draw("AP");
    canvas->Update();
    canvas->SaveAs("my_plot.pdf");
  }

} // Print_dataset;
void Utils::Write_xsec(std::vector<CrossSectionBin>&  Xsec_bin_vec , const std::string& out_file_name)
{
  TFile *outfile = TFile::Open(out_file_name.c_str(), "RECREATE");
  TTree *tree = new TTree("xsec", "GENIE model differential cross sections");

  CrossSectionBin bin;
  tree->Branch("W", &bin.W);
  tree->Branch("Q2", &bin.Q2);
  tree->Branch("theta", &bin.theta_deg);
  tree->Branch("phi", &bin.phi_deg);
  tree->Branch("dsigma", &bin.d_sigma_cm2_per_sr);
  tree->Branch("dsigma_stat_unc", &bin.d_sigma_stat_unc_cm2_per_sr);


  for (const auto& b : Xsec_bin_vec) 
  {
      bin = b;
      tree->Fill();
  }

  tree->Write();
  outfile->Close();

  }
void Utils::Plot_comparison(const fs::path& Egiyan_like_binned_MC, const std::vector<Kinematics>& Egiyan_data)
{
    if (!fs::exists(Egiyan_like_binned_MC)) {
        pLOG("Utils::Plot_comparison", pERROR) << "No such file or directory: " << Egiyan_like_binned_MC;
        return;
    }

    std::unique_ptr<TFile> Eg_file(TFile::Open(Egiyan_like_binned_MC.c_str(), "READ"));
    TTree* tree = (TTree*)Eg_file->Get("xsec");
    if (!tree) {
        pLOG("Utils::Plot_comparison", pERROR) << "TTree 'xsec' not found in file";
        return;
    }

    CrossSectionBin bin;
    tree->SetBranchAddress("W", &bin.W);
    tree->SetBranchAddress("Q2", &bin.Q2);
    tree->SetBranchAddress("theta", &bin.theta_deg);
    tree->SetBranchAddress("phi", &bin.phi_deg);
    tree->SetBranchAddress("dsigma", &bin.d_sigma_cm2_per_sr);
    tree->SetBranchAddress("dsigma_stat_unc", &bin.d_sigma_stat_unc_cm2_per_sr);

    std::vector<double> w_genie, xsec_genie, unc_genie;
    std::vector<double> w_data, xsec_data, unc_data;
/*
: q2(0.0f), w(0.0f), epsilon(0.0f), theta_pi(0.0f), sigma_0(0.0f),
        sigma_t(0.0f), sigma_l(0.0f), unc_0(0.0f), unc_t(0.0f), unc_l(0.0f)
*/
Long64_t nentries = tree->GetEntries();
for (Long64_t i = 0; i < nentries; ++i) {
    tree->GetEntry(i);

    for (const auto& eg : Egiyan_data) {
        if (std::abs(bin.Q2 - eg.q2) < 1e-3 &&
            std::abs(bin.theta_deg - eg.theta_pi) < 1e-2)
        {
            // Convert GENIE cm² to µb
            double genie_xsec_ub = bin.d_sigma_cm2_per_sr * 1e30;
            double genie_unc_ub  = bin.d_sigma_stat_unc_cm2_per_sr * 1e30;

            w_data.push_back(eg.w);
            xsec_data.push_back(eg.sigma_0);
            unc_data.push_back(eg.unc_0);

            w_genie.push_back(bin.W);
            xsec_genie.push_back(genie_xsec_ub);
            unc_genie.push_back(genie_unc_ub);

            break;
        }
    }
}

    // Draw with TGraphErrors
    TCanvas* c = new TCanvas("c", "GENIE vs Egiyan Comparison", 900, 600);
    c->SetGrid();
//------------------------------------------------------
    // 1. Zip the vectors
    std::vector<std::tuple<double, double, double>> genie_points;
    for (size_t i = 0; i < w_genie.size(); ++i) {
        genie_points.emplace_back(w_genie[i], xsec_genie[i], unc_genie[i]);
    }

    // 2. Sort by W
    std::sort(genie_points.begin(), genie_points.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    // 3. Unzip into sorted vectors
    w_genie.clear();
    xsec_genie.clear();
    unc_genie.clear();
    for (const auto& tup : genie_points) {
        w_genie.push_back(std::get<0>(tup));
        xsec_genie.push_back(std::get<1>(tup));
        unc_genie.push_back(std::get<2>(tup));
    }
//------------------------------------------------------
    TGraphErrors* g_genie = new TGraphErrors(w_genie.size(), w_genie.data(), xsec_genie.data(), nullptr, unc_genie.data());
    g_genie->SetTitle("GENIE DCC vs Egiyan Data;W (GeV);d#sigma/d#Omega^{*} (#mub/sr)");
    g_genie->SetMarkerStyle(21);
    g_genie->SetMarkerColor(kBlue);
    g_genie->SetLineColor(kBlue);
    g_genie->SetName("GENIE");
    g_genie->SetLineWidth(2);
    g_genie->SetMarkerSize(1.2);
    g_genie->SetMarkerStyle(21); // Keep circle marker


    TGraphErrors* g_egiyan = new TGraphErrors(w_data.size(), w_data.data(), xsec_data.data(), nullptr, unc_data.data());
    g_egiyan->SetMarkerStyle(20);
    g_egiyan->SetMarkerColor(kRed);
    g_egiyan->SetLineColor(kRed);
    g_egiyan->SetName("Egiyan");

    g_genie->Draw("APL");
    // g_egiyan->Draw("P SAME");

    TLegend* legend = new TLegend(0.15, 0.75, 0.4, 0.88);
    legend->AddEntry(g_genie, "GENIE DCC", "lep");
    legend->AddEntry(g_egiyan, "Egiyan et al. (2006)", "lep");
    legend->Draw();

    c->SaveAs("comparison_plot.pdf");
}
