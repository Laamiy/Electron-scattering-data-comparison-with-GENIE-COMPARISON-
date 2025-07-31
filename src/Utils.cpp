#include "Framework/Messenger/Messenger.h"
#include "SPP_event.hpp"
#include "single_pion.hpp"
#include <filesystem>
#include <sys/types.h>
#include <vector>
#include "TMultiGraph.h"

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
    if (!fs::exists(Egiyan_like_binned_MC)) 
    {
        pLOG("Utils::Plot_comparison", pERROR) << "No such file or directory: " << Egiyan_like_binned_MC;
        return;
    }

    std::unique_ptr<TFile> Eg_file(TFile::Open(Egiyan_like_binned_MC.c_str(), "READ"));
    TTree* tree = (TTree*)Eg_file->Get("xsec");
    if (!tree) 
    {
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

    // Map key = (Q2, W), value = (sum_xsec, sum_unc2, W)
    std::map<std::pair<double, double>, std::tuple<double, double, double>> genie_map;

    Long64_t nentries = tree->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) 
    {
        tree->GetEntry(i);

        // Round Q2 to match Egiyan binning precision
        double q2_r = std::round(bin.Q2 * 1000.0) / 1000.0;

        // Use W directly (do not round)
        double W_val = bin.W;

        // Key is (Q2, W)
        auto key = std::make_pair(q2_r, W_val);

        // Convert GENIE cross section from cm^2 to μb (check this factor!)
        double genie_xsec_ub = bin.d_sigma_cm2_per_sr * 1e30;  // <-- verify this factor is correct for your units
        double genie_unc_ub = bin.d_sigma_stat_unc_cm2_per_sr * 1e30;

        auto& entry = genie_map[key];
        std::get<0>(entry) += genie_xsec_ub;               // sum cross sections
        std::get<1>(entry) += std::pow(genie_unc_ub, 2.0); // sum uncertainties squared
        std::get<2>(entry) = W_val;                         // store W (redundant)
    }

    // Prepare Egiyan data grouped by Q2 for plotting
    std::map<double, std::vector<std::tuple<double, double, double>>> q2_w_xsec_egiyan_map;
    for (const auto& eg : Egiyan_data) 
    {
        double q2_r = std::round(eg.q2 * 1000.0) / 1000.0;
        q2_w_xsec_egiyan_map[q2_r].emplace_back(eg.w, eg.sigma_0, eg.unc_0);
    }

    // Group GENIE points by Q2 for plotting
    std::map<double, std::vector<std::tuple<double, double, double>>> q2_w_xsec_map;
    for (const auto& [key, tup] : genie_map) 
    {
        auto [Q2, W] = key;
        double xsec_sum = std::get<0>(tup);
        double unc2_sum = std::get<1>(tup);
        double unc = std::sqrt(unc2_sum);

        // Filter invalid values just in case
        if (!std::isfinite(Q2) || !std::isfinite(W) || !std::isfinite(xsec_sum) || !std::isfinite(unc))
        {
            std::cerr << "Invalid data point Q2=" << Q2 << ", W=" << W << ", xsec=" << xsec_sum << ", unc=" << unc << std::endl;
            continue;
        }

        q2_w_xsec_map[Q2].emplace_back(W, xsec_sum, unc);
    }

    // Debug print of GENIE data points
    std::cout << "GENIE data points (Q2, W, xsec, unc):\n";
    for (const auto& [Q2, vec] : q2_w_xsec_map) 
    {
        for (const auto& [W, xsec, unc] : vec) 
        {
            std::cout << "Q2=" << Q2 << ", W=" << W << ", xsec=" << xsec << ", unc=" << unc << std::endl;
        }
    }

    // Canvas for combined plot
    TCanvas* c = new TCanvas("c", "GENIE vs Egiyan Comparison", 1000, 700);
    c->SetGrid();

    TMultiGraph* mg = new TMultiGraph();
    std::vector<int> colors = {kRed + 1, kBlue + 1, kGreen + 2, kMagenta + 1, kOrange + 7, kCyan + 2};
    int color_index = 0;

    // Plot GENIE (as lines) grouped by Q2
    for (const auto& [Q2, vec] : q2_w_xsec_map) 
    {
        std::vector<double> w_vals, xsec_vals, unc_vals;
        for (const auto& [W, xsec, unc] : vec) 
        {
            w_vals.push_back(W);
            xsec_vals.push_back(xsec);
            unc_vals.push_back(unc);
        }

        // Plot GENIE as lines (no error bars)
        TGraph* g_genie = new TGraph(w_vals.size(), w_vals.data(), xsec_vals.data());
        g_genie->SetLineColor(colors[color_index % colors.size()]);
        g_genie->SetLineWidth(2);
        g_genie->SetTitle(Form("GENIE Q^{2} = %.3f", Q2));
        mg->Add(g_genie, "L");

        ++color_index;
    }

    // Prepare Egiyan data arrays (all Q2 combined) for overlay plot as points with error bars
    std::vector<double> w_data_all, xsec_data_all, unc_data_all;
    for (const auto& [Q2, vec] : q2_w_xsec_egiyan_map) 
    {
        for (const auto& [W, xsec, unc] : vec) 
        {
            w_data_all.push_back(W);
            xsec_data_all.push_back(xsec);
            unc_data_all.push_back(unc);
        }
    }

    if (!w_data_all.empty()) 
    {
        TGraphErrors* g_data = new TGraphErrors(w_data_all.size(), w_data_all.data(), xsec_data_all.data(), nullptr, unc_data_all.data());
        g_data->SetMarkerStyle(24); // open circle
        g_data->SetMarkerColor(kBlack);
        g_data->SetLineColor(kBlack);
        g_data->SetTitle("Egiyan et al. data");
        mg->Add(g_data, "P");
    }

    mg->SetTitle("GENIE Cross Sections vs W;W [GeV];d^{2}#sigma / dE'd#Omega [μb/sr]");
    mg->Draw("A");

    c->BuildLegend();
    c->Print("Plot_combined.pdf");

    // Group Egiyan and GENIE points by (Q², W)
std::map<std::pair<double, double>, std::vector<std::tuple<double, double, double>>> egiyan_by_q2w;
std::map<std::pair<double, double>, std::vector<std::tuple<double, double, double>>> genie_by_q2w;

for (const auto& eg : Egiyan_data) {
    double q2_r = std::round(eg.q2 * 1000.0) / 1000.0;
    double w_r  = std::round(eg.w  * 100.0)  / 100.0;
    egiyan_by_q2w[{q2_r, w_r}].emplace_back(eg.theta_pi, eg.sigma_0, eg.unc_0);
}

for (const auto& [key, tup] : genie_map) {
    double q2 = key.first;
    double theta = key.second;
    double w = std::get<2>(tup); // Stored W in map
    double xsec = std::get<0>(tup);
    double unc  = std::sqrt(std::get<1>(tup));

    genie_by_q2w[{q2, w}].emplace_back(theta, xsec, unc);
}

// Create a canvas for each (Q², W) bin
int plot_index = 0;
for (const auto& [q2w, data_points] : egiyan_by_q2w) {
    auto [q2, w] = q2w;
    plot_index++;
    std::string cname = Form("c_theta_phi_q2_%.3f_w_%.3f", q2, w);
    TCanvas* c = new TCanvas(cname.c_str(), cname.c_str(), 900, 700);
    c->SetGrid();

    std::vector<double> theta_data, sigma_data, unc_data;
    for (const auto& [theta, sigma, unc] : data_points) {
        theta_data.push_back(theta);
        sigma_data.push_back(sigma);
        unc_data.push_back(unc);
    }

    TGraphErrors* g_egiyan = new TGraphErrors(theta_data.size(), theta_data.data(), sigma_data.data(), nullptr, unc_data.data());
    g_egiyan->SetMarkerStyle(20);
    g_egiyan->SetMarkerColor(kBlack);
    g_egiyan->SetLineColor(kBlack);
    g_egiyan->SetTitle(Form("Egiyan: Q^{2} = %.3f GeV^{2}, W = %.2f GeV", q2, w));
    g_egiyan->GetXaxis()->SetTitle("#theta^{*} [deg]");
    g_egiyan->GetYaxis()->SetTitle("#sigma_{T} + #varepsilon #sigma_{L} [#mu b/sr]");
    g_egiyan->Draw("AP");

    // If GENIE prediction exists for this (Q², W), draw it as a line
    // auto it = genie_by_q2w.find(q2w);
    // if (it != genie_by_q2w.end()) {
    //     std::vector<double> theta_mc, sigma_mc;
    //     for (const auto& [theta, sigma, unc] : it->second) {
    //         theta_mc.push_back(theta);
    //         sigma_mc.push_back(sigma);
    //     }

    //     TGraph* g_genie = new TGraph(theta_mc.size(), theta_mc.data(), sigma_mc.data());
    //     g_genie->SetLineColor(kBlue);
    //     g_genie->SetLineWidth(2);
    //     g_genie->Draw("L SAME");

    //     TLegend* legend = new TLegend(0.15, 0.75, 0.5, 0.88);
    //     legend->AddEntry(g_egiyan, "Egiyan et al.", "lep");
    //     legend->AddEntry(g_genie, "GENIE DCC", "l");
    //     legend->Draw();
    // }
    auto it = genie_by_q2w.find(q2w);
    if (it != genie_by_q2w.end())
    {
        std::vector<double> theta_mc, sigma_mc;
        for (const auto& [theta, sigma, unc] : it->second)
        {
            theta_mc.push_back(theta);
            sigma_mc.push_back(sigma);
        }

    if (!theta_mc.empty())
    {
        TGraph* g_genie = new TGraph(theta_mc.size(), theta_mc.data(), sigma_mc.data());
        g_genie->SetLineColor(kRed);
        g_genie->SetLineWidth(3);
        g_genie->Draw("PL SAME");

        TLegend* legend = new TLegend(0.15, 0.75, 0.5, 0.88);
        legend->AddEntry(g_egiyan, "Egiyan et al.", "lep");
        legend->AddEntry(g_genie, "GENIE DCC", "l");
        legend->Draw();
    }
}


    c->SaveAs(Form("Comparison_theta_q2_%.3f_w_%.2f.pdf", q2, w));
}

#if 0 
    // Canvas for separated subplots per Q2 bin
    TCanvas* c1 = new TCanvas("c1", "Separated GENIE and Egiyan by Q2", 1200, 800);
    c1->SetGrid();
    int nPadsX = 2;
    int nPadsY = (int)q2_w_xsec_map.size() / nPadsX + 1;
    c1->Divide(nPadsX, nPadsY);

    int pad_idx = 1;
    for (const auto& [Q2, genie_vec] : q2_w_xsec_map) 
    {
        c1->cd(pad_idx);

        // GENIE as line graph (no error bars)
        std::vector<double> w_genie, xsec_genie;
        for (const auto& [W, xsec, unc] : genie_vec) 
        {
            w_genie.push_back(W);
            xsec_genie.push_back(xsec);
        }
        TGraph* g_genie = new TGraph(w_genie.size(), w_genie.data(), xsec_genie.data());
        g_genie->SetLineColor(kBlue);
        g_genie->SetLineWidth(2);
        g_genie->SetTitle(Form("GENIE Q^{2} = %.3f", Q2));
        g_genie->GetXaxis()->SetTitle("W [GeV]");
        g_genie->GetYaxis()->SetTitle("Cross Section (μb/sr)");
        g_genie->Draw("AL");

        // Egiyan data with error bars
        auto eg_it = q2_w_xsec_egiyan_map.find(Q2);
        if (eg_it != q2_w_xsec_egiyan_map.end()) 
        {
            const auto& eg_vec = eg_it->second;
            std::vector<double> w_eg, xsec_eg, unc_eg;
            for (const auto& [W, xsec, unc] : eg_vec) 
            {
                w_eg.push_back(W);
                xsec_eg.push_back(xsec);
                unc_eg.push_back(unc);
            }
            TGraphErrors* g_egiyan = new TGraphErrors(w_eg.size(), w_eg.data(), xsec_eg.data(), nullptr, unc_eg.data());
            g_egiyan->SetMarkerStyle(20);
            g_egiyan->SetMarkerColor(kRed);
            g_egiyan->SetLineColor(kRed);
            g_egiyan->Draw("P SAME");
        }

        pad_idx++;
    }

    c1->BuildLegend();
    c1->Print("Plot_separated.pdf");
    //------------------------------------------------------- NOT NEEDED _____________
    // === Canvas for Egiyan-only plot ===
TCanvas* c_data = new TCanvas("c_data", "Egiyan Data Only", 800, 600);
c_data->SetGrid();

TH1D* h_dummy = new TH1D("h_dummy", "", 100, 0, 180); // Adjust theta range as needed
h_dummy->SetMinimum(0);
h_dummy->SetMaximum(10); // Adjust based on your data
h_dummy->GetXaxis()->SetTitle("#theta [deg]");
h_dummy->GetYaxis()->SetTitle("#sigma_{T} + #epsilon#sigma_{L} [#mu b/sr]");
h_dummy->Draw("AXIS");

for (const auto& eg : Egiyan_data)
{
    double x = eg.theta_pi;      // degrees
    double y = eg.sigma_0;    // μb/sr
    double err = eg.unc_0;

    TGraphErrors* g = new TGraphErrors(1);
    g->SetPoint(0, x, y);
    g->SetPointError(0, 0, err);
    g->SetMarkerStyle(20);
    g->SetMarkerSize(1.2);
    g->SetMarkerColor(kBlack);
    g->SetLineColor(kBlack);
    g->Draw("P SAME");
}

c_data->Print("Plot_Egiyan_only.pdf");

    //-------------------------------------------------------
#endif
}

