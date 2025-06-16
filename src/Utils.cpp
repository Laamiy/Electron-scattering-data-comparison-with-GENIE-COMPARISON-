#include "single_pion.hpp"

Cmd_args Utils::GetCommandLineArgs(int argc, char **argv)
  {
    std::string def_spp_model = "genie::DCCSPPPXSec/NoPauliBlock"; 
    std::filesystem::path input_data_path  =""; 
    std::cout <<"[INFO] Parsing command line arguments\n";
    genie::RunOpt::Instance()->ReadFromCommandLine(argc, argv);
    genie::CmdLnArgParser parser(argc, argv);

    // Get GENIE model names to be used :
    if (parser.OptionExists("resonance-xsec-model")) {
      def_spp_model = parser.Arg("resonance-xsec-model");
    }
    // Get Egiyan like dataset path  :
    if (parser.OptionExists("input_file"))
    {
      input_data_path = parser.Arg("input_file");
    }
    return {input_data_path, std::move(def_spp_model).c_str() } ; 
  }  

