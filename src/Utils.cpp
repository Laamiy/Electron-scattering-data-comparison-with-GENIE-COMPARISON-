#include "Framework/Messenger/Messenger.h"
#include "single_pion.hpp"
#include <sys/types.h>
#include <vector>

Cmd_args Utils::GetCommandLineArgs(int argc, char **argv)
  {
    std::string def_spp_model = "genie::DCCSPPPXSec/NoPauliBlock"; 
    std::filesystem::path input_data_path  =""; 
    pLOG("Utils", pINFO)<<" Parsing command line arguments";
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
    return {input_data_path, std::move(def_spp_model) } ; 
  }  
  
void Utils:: minus_uni_2_ascii(std::string& buffer,const std::string& uni_code ,const std::string& ascii_code)
{
  // This replaces utf-8 '-' into ascii '-' : 
    size_t pos = 0;
  while ((pos = buffer.find(uni_code, pos)) != std::string::npos)
  {
      buffer.replace(pos, uni_code.length(), ascii_code);
      pos += ascii_code.length();  // continue after the replacement
  }
};

std::optional<std::vector<Kinematics> > Utils :: Read_data_file(const fs::path& file_path)
{
  std::ifstream reader ;
  reader.open(file_path);
  
  if(!reader)
  {
    pLOG("Utils", pERROR)<<" Couldn't open file :" << file_path.stem().c_str();
    return {};
  } 
  double q2{}   , W{}    , epsilon{} , theta_pi{};
  double sig0{} , sig1{} , sig2{}    , unc0{} , unc1{} , unc2{} ;

  std::string pm0 , pm1 , pm2;
  // 0.6 , 1.39 , 0.541 , 52.5 , 4.322 , ± , 0.44 , −0.049 , ± , 0.51 , −2.415 , ± , 0.34
  std::vector<Kinematics> data_kinematics{};
  data_kinematics.reserve(EXPECTED_SIZE);
  
  std::string current_line ;
  std::getline(reader , current_line); 
  std::stringstream stream ; 
  
  const std::string uni_code = u8"−"; // UTF-8 encoding of U+2212
  const std::string ascii_code = "-"; 
  
  while(std::getline(reader,current_line))
  {
    Utils::minus_uni_2_ascii(current_line,uni_code ,ascii_code);
    stream.str(current_line);          
    stream >> q2 >>    W >> epsilon >> theta_pi >> sig0 >> pm0 >> unc0 >> sig1 >>
             pm1 >> unc1 >> sig2    >> pm2      >> unc2;
    data_kinematics.emplace_back(q2 , W , epsilon , theta_pi , sig0, unc0 , sig1, sig2);
    stream.clear();
  }
  return  data_kinematics ;
}

void Utils::Print_dataset(const std::vector<Kinematics>& data_kinematics,const char* model_name ,const fs::path& file_path, bool save )
{
  pLOG("Utils", pINFO)<<" Dataset : ";
  std::vector<double> xsec , w ,unc ;
  for (auto& kine : data_kinematics)
  {
    std::cout << kine ;
    xsec.push_back(kine.sigma_0);
    w.push_back(kine.w);
    unc.push_back(kine.unc_0);
  }

  pLOG("Utils", pINFO)<<" Dataset size : "    << data_kinematics.size();
  pLOG("Utils", pINFO)<<" Model name   : "    << model_name     ;
  pLOG("Utils", pINFO)<<" Egiyan data file path : "<< file_path ;

  if (save)
  {
   TGraphErrors* graph = new TGraphErrors
   (
      xsec.size(),
      w.data(),        // x-axis: W
      xsec.data(),     // y-axis: σ₀
      nullptr,         // x-errors (none)
      unc.data()       // y-errors (uncertainty on σ₀)
    );

    graph->SetTitle("Data with Uncertainties;Index;Measurement");
    graph->SetMarkerStyle(21);
    graph->SetMarkerColor(kRed + 1);
    graph->SetLineColor(kRed + 1);

    TCanvas* canvas = new TCanvas("c", "Plot", 800, 600);
    graph->Draw("AP");
    canvas->Update();
    canvas->SaveAs("my_plot.pdf");  
  }

}
