#include "Framework/Messenger/Messenger.h"
#include "SPP_event.hpp"
#include "single_pion.hpp"
#include <filesystem>
#include <sys/types.h>
#include <vector>
#include "TMultiGraph.h"
#include <numeric>

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

// void Utils::Print_dataset(const std::vector<Kinematics> &data_kinematics,
//                           const char *model_name, const fs::path &file_path,
//                           bool save) 
// {
//   pLOG("Utils", pINFO) << " Dataset : ";
//   std::vector<double> xsec, w, unc;
//   for (auto &kine : data_kinematics) 
//   {
//     std::cout << kine;
//     xsec.push_back(kine.sigma_0);
//     w.push_back(kine.w);
//     unc.push_back(kine.unc_0);
//   }

//   pLOG("Utils", pINFO) << " Dataset size : " << data_kinematics.size();
//   pLOG("Utils", pINFO) << " Model name   : " << model_name;
//   pLOG("Utils", pINFO) << " Egiyan data file path : " << file_path;
//   if (save) 
//   {
//     TGraphErrors *graph =
//         new TGraphErrors(xsec.size(),
//                          w.data(),    // x-axis: W
//                          xsec.data(), // y-axis: σ₀
//                          nullptr,     // x-errors (none)
//                          unc.data()   // y-errors (uncertainty on σ₀)
//         );

//     graph->SetTitle("Data with Uncertainties;Index;Measurement");
//     graph->SetMarkerStyle(21);
//     graph->SetMarkerColor(kRed + 1);
//     graph->SetLineColor(kRed + 1); // set the Dataset line color ;

//     TCanvas *canvas = new TCanvas("c", "Plot", 800, 600);
//     graph->Draw("AP");
//     canvas->Update();
//     canvas->SaveAs("my_plot.pdf");
//   }

// } // Print_dataset;
void Utils::Write_xsec(std::vector<CrossSectionBin>&  Xsec_bin_vec , const std::string& out_file_name)
{
  TFile *outfile = TFile::Open(out_file_name.c_str(), "RECREATE");
  TTree *tree = new TTree("xsec", "GENIE model differential cross sections");

  CrossSectionBin bin;
  tree->Branch("W", &bin.W);
  tree->Branch("Q2", &bin.Q2);
  tree->Branch("theta", &bin.theta_deg);
  tree->Branch("phi", &bin.phi_deg);
  tree->Branch("dsigma", &bin.d_sigma_ub_per_sr);
  tree->Branch("dsigma_stat_unc", &bin.d_sigma_stat_unc_ub_per_sr);


  for (const auto& b : Xsec_bin_vec) 
  {
      bin = b;
      tree->Fill();
  }

  tree->Write();
  outfile->Close();

  }
void Utils::Plot_comparison(const fs::path& Egiyan_like_binned_MC, const std::vector<Kinematics>& Egiyan_data ,
                                      std::vector<double>& sigma_w , const std::vector<double>& w_center)
{
    if (!fs::exists(Egiyan_like_binned_MC)) 
    {
        pLOG("Utils::Plot_comparison", pERROR) << "No such file or directory: " << Egiyan_like_binned_MC;
        return;
    }

    std::unique_ptr<TFile> Eg_file(TFile::Open(Egiyan_like_binned_MC.c_str(), "READ"));
    if (!Eg_file || Eg_file->IsZombie()) {
        pLOG("Utils::Plot_comparison", pERROR) << "Failed to open ROOT file: " << Egiyan_like_binned_MC;
        return;
    }

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
    tree->SetBranchAddress("dsigma", &bin.d_sigma_ub_per_sr);
    tree->SetBranchAddress("dsigma_stat_unc", &bin.d_sigma_stat_unc_ub_per_sr);

    std::map<Q2WThetaBin, g_map_val_byq2w> genie_map_full;

    Long64_t nentries = tree->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) 
    {
        tree->GetEntry(i);
        int q2_bin = static_cast<int>(std::round(bin.Q2 * 1000));
        int w_bin  = static_cast<int>(std::round(bin.W * 100));
        int theta_bin = static_cast<int>(std::round(bin.theta_deg));

        Q2WThetaBin key{q2_bin, w_bin, theta_bin};

        double xsec_ub = bin.d_sigma_ub_per_sr ;//* 1e-7;
        double unc_ub = bin.d_sigma_stat_unc_ub_per_sr ;//* 1e-7;

        auto& entry = genie_map_full[key];
        entry.Theta = bin.theta_deg;
        entry.Xsec += xsec_ub;
        entry.Unc = std::sqrt(entry.Unc * entry.Unc + unc_ub * unc_ub);
    }
    // -- Normalize GENIE using total integrated σ in Q² ≈ 0.3 region --
//-------------------------------------------------------------------------------------------
        double genie_integral = 0.0;
        for (const auto& [key, val] : genie_map_full)
        {
            if (std::abs(key.q2_bin - 300) > 25) continue;
            genie_integral += val.Xsec;
        }

        double egiyan_integral = 0.0;
        for (const auto& eg : Egiyan_data)
        {
            if (std::abs(eg.q2 - 0.3) > 0.025) continue;
            egiyan_integral += eg.sigma_0;
        }

        double norm_factor = (genie_integral > 0) ? (egiyan_integral / genie_integral) : 1.0;

        // Apply the same scale to all GENIE cross sections and uncertainties:
        for (auto& [key, val] : genie_map_full)
        {
            val.Xsec *= norm_factor;
            val.Unc  *= norm_factor;
        }

        // Apply same factor to total σ(W) from GENIE (for dσ/dW overlay)
        for (auto& sig : sigma_w)
        {
            sig *= norm_factor;
        }
//-------------------------------------------------------------------------------------------
    std::map<Q2WBin, std::vector<g_map_val_byq2w>> egiyan_by_q2w;
    for (const auto& eg : Egiyan_data)
    {
        int q2_bin = static_cast<int>(std::round(eg.q2 * 1000));
        int w_bin = static_cast<int>(std::round(eg.w * 100));
        Q2WBin key{q2_bin, w_bin};
        egiyan_by_q2w[key].emplace_back(eg.theta_pi, eg.sigma_0, eg.unc_0);
    }

    std::map<int, std::vector<std::pair<double, double>>> genie_by_theta;
    std::map<int, std::vector<std::pair<double, double>>> egiyan_by_theta;

    for (const auto& [key, val] : genie_map_full)
    {
        if (std::abs(key.q2_bin - 300) > 25) continue;
        genie_by_theta[key.theta_bin].emplace_back(key.w_bin / 100.0, val.Xsec);
    }

    for (const auto& [key, points] : egiyan_by_q2w) 
    {
        if (std::abs(key.q2_bin - 300) > 25) continue;
        for (const auto& pt : points)
        {
            int theta_bin = static_cast<int>(std::round(pt.Theta));
            egiyan_by_theta[theta_bin].emplace_back(key.w_bin / 100.0, pt.Xsec);
        }
    }

    TCanvas* c_multi = new TCanvas("c_multi", "σ₀ vs W at fixed θ*", 1200, 800);
    c_multi->Divide(4, 3);

    int pad_index = 1;
    int plot_count = 0;

    for (const auto& [theta_bin, genie_points] : genie_by_theta)
     {
        if (genie_points.size() < 3 || egiyan_by_theta[theta_bin].empty()) continue;

        std::vector<std::pair<double, double>> genie_sorted = genie_points;
        std::sort(genie_sorted.begin(), genie_sorted.end());

        std::vector<double> w_genie, sigma_genie;
        for (auto& p : genie_sorted) 
        {
            w_genie.push_back(p.first);
            sigma_genie.push_back(p.second);
        }

        std::vector<std::pair<double, double>> egiyan_sorted = egiyan_by_theta[theta_bin];
        std::sort(egiyan_sorted.begin(), egiyan_sorted.end());

        std::vector<double> w_eg, sigma_eg;
        for (auto& p : egiyan_sorted) 
        {
            w_eg.push_back(p.first);
            sigma_eg.push_back(p.second);
        }

        c_multi->cd(pad_index);
        gPad->SetGrid();

        TGraph* g_genie = new TGraph(w_genie.size(), w_genie.data(), sigma_genie.data());
        g_genie->SetLineColor(kBlue + 1);
        g_genie->SetLineWidth(2);
        g_genie->SetTitle(Form("#theta^{*} = %d^{o}", theta_bin));
        g_genie->GetXaxis()->SetLimits(1.1, 1.6);
        g_genie->SetMinimum(0);
        g_genie->SetMaximum(25);
        g_genie->Draw("AL");

        TGraph* g_eg = new TGraph(w_eg.size(), w_eg.data(), sigma_eg.data());
        g_eg->SetMarkerStyle(20);
        g_eg->SetMarkerColor(kBlack);
        g_eg->SetLineColor(kBlack);
        g_eg->Draw("P SAME");

        pad_index++;
        plot_count++;

        if (plot_count % 12 == 0) {
            c_multi->Print("Comparison_sigma_vs_W_by_theta_fixed_q2_grid.pdf");
            c_multi->Clear();
            c_multi->Divide(4, 3);
            pad_index = 1;
        }
    }

    if (plot_count % 12 != 0) {
        c_multi->Print("Comparison_sigma_vs_W_by_theta_fixed_q2_grid.pdf");
    }

    delete c_multi;

    TCanvas* c_theta = new TCanvas("c_theta", "Theta Comparison", 900, 700);
    c_theta->Print("Comparison_theta_all.pdf[", "pdf");

    const double x_margin_deg = 10.0;
    const double y_margin_frac = 0.2;

    for (const auto& [q2w_key, egiyan_points] : egiyan_by_q2w) 
    {
        double q2 = q2w_key.q2_bin / 1000.0;
        double w = q2w_key.w_bin / 100.0;
        if (egiyan_points.empty())
         continue;

        c_theta->Clear();
        c_theta->SetGrid();

        std::vector<double> theta_eg, sigma_eg, unc_eg;
        for (const auto& pt : egiyan_points) 
        {
            theta_eg.push_back(pt.Theta);
            sigma_eg.push_back(pt.Xsec);
            unc_eg.push_back(pt.Unc);
        }

        std::vector<std::pair<double, double>> genie_points_theta;
        std::vector<double> unc_genie;
        for (const auto& [key_full, val] : genie_map_full)
        {
            if (key_full.q2_bin == q2w_key.q2_bin && key_full.w_bin == q2w_key.w_bin) 
            {
                genie_points_theta.emplace_back(val.Theta, val.Xsec);
                unc_genie.push_back(val.Unc);  
            }
        }


        if (genie_points_theta.empty()) 
            continue;

        std::sort(genie_points_theta.begin(), genie_points_theta.end());

        double theta_min = *std::min_element(theta_eg.begin(), theta_eg.end());
        double theta_max = *std::max_element(theta_eg.begin(), theta_eg.end());
        double genie_theta_min = genie_points_theta.front().first;
        double genie_theta_max = genie_points_theta.back().first;
        if (genie_theta_min < theta_min) theta_min = genie_theta_min;
        if (genie_theta_max > theta_max) theta_max = genie_theta_max;

        double y_min = *std::min_element(sigma_eg.begin(), sigma_eg.end());
        double y_max = *std::max_element(sigma_eg.begin(), sigma_eg.end());
        for (const auto& [theta, val] : genie_points_theta) 
        {
            if (val < y_min) y_min = val;
            if (val > y_max) y_max = val;
        }

        double x_min = theta_min - x_margin_deg;
        double x_max = theta_max + x_margin_deg;
        double y_range = y_max - y_min;
        if (y_range < 1e-12) y_range = 1.0;
        y_min -= y_margin_frac * y_range;
        y_max += y_margin_frac * y_range;

        TGraphErrors* g_egiyan = new TGraphErrors(theta_eg.size(), theta_eg.data(), sigma_eg.data(), nullptr, unc_eg.data());
        g_egiyan->SetMarkerStyle(20);
        g_egiyan->SetMarkerColor(kBlack);
        g_egiyan->SetLineColor(kBlack);
        g_egiyan->SetTitle(Form("Egiyan: Q^{2} = %.3f GeV^{2}, W = %.2f GeV", q2, w));
        g_egiyan->GetXaxis()->SetLimits(x_min, x_max);
        g_egiyan->GetYaxis()->SetRangeUser(y_min, y_max);
        g_egiyan->GetXaxis()->SetTitle("#theta^{*} [deg]");
        g_egiyan->GetYaxis()->SetTitle("#sigma_{T} + #varepsilon #sigma_{L} [#mu b/sr]");
        g_egiyan->Draw("AP");

        std::vector<double> theta_genie, sigma_genie;
        for (const auto& p : genie_points_theta) 
        {
            theta_genie.push_back(p.first);
            sigma_genie.push_back(p.second);
        }
        TGraphErrors* g_genie = new TGraphErrors(theta_genie.size(), theta_genie.data(), sigma_genie.data(), nullptr, unc_genie.data());
        g_genie->SetLineColor(kRed);
        g_genie->SetFillColorAlpha(kRed, 0.25);
        g_genie->SetLineWidth(2);
        g_genie->Draw("3 SAME");  // shaded band
        g_genie->Draw("LX SAME"); // line on top


        TLegend* legend = new TLegend(0.15, 0.75, 0.5, 0.88);
        legend->AddEntry(g_egiyan, "Egiyan et al.", "lep");
        legend->AddEntry(g_genie, "GENIE DCC", "lp");
        legend->Draw();

        gPad->Modified();
        gPad->Update();

        c_theta->Print("Comparison_theta_all.pdf", "pdf");
    }

    c_theta->Print("Comparison_theta_all.pdf]", "pdf");
    delete c_theta;
    //-----------------------------------------d2σ/dΩ vs W (GENIE vs Egiyan)----------------------------------//
// -- Get all Q2 bins from Egiyan data --
    std::set<int> q2_bins_available;
    for (const auto& eg : Egiyan_data)
    {
        int q2_bin = static_cast<int>(std::round(eg.q2 * 1000));
        q2_bins_available.insert(q2_bin);
    }

// --- Multi-page PDF: d2sigma/dOmega vs W for each Q2 ---
TCanvas* c_w = new TCanvas("c_w", "d^{2}#sigma/d#Omega vs W", 800, 600);
bool first_page = true;

for (int q2_bin : q2_bins_available)
{
    double q2_val = q2_bin / 1000.0;

    // --- Regenerate Egiyan sigma(W) sum over Theta* for this Q2 ---
    std::map<int, double> egiyan_sigmaW_sum;
    for (const auto& eg : Egiyan_data)
    {
        if (std::abs(eg.q2 * 1000 - q2_bin) > 25) 
            continue;
        int w_bin = static_cast<int>(std::round(eg.w * 100));
        egiyan_sigmaW_sum[w_bin] += eg.sigma_0;
    }

    std::vector<double> eg_w, eg_sigma;
    for (const auto& [w_bin, sigma_sum] : egiyan_sigmaW_sum)
    {
        eg_w.push_back(w_bin / 100.0);
        eg_sigma.push_back(sigma_sum); // Already μb/sr
    }

    if (eg_sigma.empty()) continue; // No Egiyan data -> skip

    // --- Compute normalization for this Q² ---
    double egiyan_integral = std::accumulate(eg_sigma.begin(), eg_sigma.end(), 0.0);
    double genie_integral = std::accumulate(sigma_w.begin(), sigma_w.end(), 0.0); // Global GENIE

    double norm_factor = (genie_integral > 0) ? (egiyan_integral / genie_integral) : 1.0;

    // --- Apply normalization to a local copy of GENIE σ(W) ---
    std::vector<double> sigma_w_norm(sigma_w.size());
    for (size_t i = 0; i < sigma_w.size(); ++i)
    {
        sigma_w_norm[i] = sigma_w[i] * norm_factor;
    }

    // --- Draw the plot ---
    c_w->Clear();
    c_w->SetGrid();

    TGraph* g_genie_w = new TGraph(w_center.size(), w_center.data(), sigma_w_norm.data());
    g_genie_w->SetLineColor(kRed + 1);
    g_genie_w->SetLineWidth(2);
    g_genie_w->SetTitle(Form("GENIE vs Egiyan: d^{2}#sigma/d#Omega at Q^{2} #approx %.2f GeV^{2}", q2_val));
    g_genie_w->GetXaxis()->SetTitle("W [GeV]");
    g_genie_w->GetYaxis()->SetTitle("d^{2}#sigma/d#Omega [#mu b/sr]");
    g_genie_w->GetXaxis()->SetLimits(1.1, 1.6);
    g_genie_w->SetMinimum(0);
    g_genie_w->Draw("AL");

    TGraph* g_egiyan_w = new TGraph(eg_w.size(), eg_w.data(), eg_sigma.data());
    g_egiyan_w->SetMarkerStyle(20);
    g_egiyan_w->SetMarkerColor(kBlack);
    g_egiyan_w->Draw("P SAME");

    TLegend* leg = new TLegend(0.6, 0.75, 0.88, 0.88);
    leg->AddEntry(g_genie_w, "GENIE DCC", "l");
    leg->AddEntry(g_egiyan_w, Form("Egiyan et al. (Q^{2} #approx %.2f)", q2_val), "p");
    leg->Draw();

    if (first_page)
    {
        c_w->Print("Comparison_d2sigma_vs_W_by_q2.pdf("); 
        first_page = false;
    }
    else
    {
        c_w->Print("Comparison_d2sigma_vs_W_by_q2.pdf");
    }

    delete g_genie_w;
    delete g_egiyan_w;
    delete leg;
}

c_w->Print("Comparison_d2sigma_vs_W_by_q2.pdf)"); 
delete c_w;


}
