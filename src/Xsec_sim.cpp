#include "Xsec_sim.hpp"
#include "TError.h"

genie::XSecAlgorithmI* Utils::Init(const std::string& spp_model_name)
{
	//checking for the tune.
	if (!genie::RunOpt::Instance()->Tune())
	{
		pLOG("Xsec_sim",pERROR)<<"No TuneId in RunOption\n";
		return nullptr ;
	}
	genie::RunOpt::Instance()->BuildTune();
	// Set GENIE style
	genie::utils::style::SetDefaultStyle();
	// Creating a genie algorithm instance. 
	genie::AlgFactory *algf = genie::AlgFactory::Instance();
	uint32_t n_model = 0;

	if (spp_model_name == "")
	{
		pLOG("Xsec_sim",pERROR)<<"Invalid model name\n";
		return nullptr ;
	}
	const std::vector< std::string > spp_model = genie::utils::str::Split(spp_model_name, "/");
	assert(spp_model.size() == 2);
	const char* model_name = spp_model[0].c_str();
	const char* model_conf = spp_model[1].c_str();
	// Creating a SPP-MODEL :
	genie::XSecAlgorithmI* gRESXSecModel = dynamic_cast< genie::XSecAlgorithmI * >(algf->AdoptAlgorithm(model_name, model_conf));
	// Create plot canvas
	#if 0 
	TCanvas gC ("c", "", 20, 20, 500, 650);
	gC.SetBorderMode(0);
	gC.SetFillColor(0);
	gC.SetGridx();
	gC.SetGridy();
	// Get local time to tag outputs
	std::string lt_for_filename   = genie::utils::system::LocalTimeAsString("%02d.%02d.%02d_%02d.%02d.%02d");
	std::string lt_for_cover_page = genie::utils::system::LocalTimeAsString("%02d/%02d/%02d %02d:%02d:%02d");
	// Create output postscript file
	std::string filename = Form("genie-spp_data_comp-%s.pdf", lt_for_filename.c_str());
	TPostScript gPS(filename.c_str(), 111);
	// Add cover page
	gPS.NewPage();
	gC.Range(0, 0, 100, 100);
	TPavesText hdr(10, 40, 90, 70, 3, "tr");
	hdr.AddText(" ");
	hdr.AddText(" ");
	hdr.AddText("GENIE comparison with (e,e') data on 1H and 2D in the resonance region");
	hdr.AddText(" ");
	hdr.AddText(" ");
	hdr.AddText(" ");
	hdr.AddText(lt_for_cover_page.c_str());
	hdr.AddText(" ");
	hdr.Draw();
	gC.Update();
	#endif
	return gRESXSecModel; 

}

