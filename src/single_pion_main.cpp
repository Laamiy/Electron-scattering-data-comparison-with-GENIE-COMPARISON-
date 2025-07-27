#include "Xsec_sim.hpp"
#include "SPP_event.hpp"
#include "single_pion.hpp"

int main(int argc , char* argv[])
{
	using namespace genie::constants;
	namespace fs =  std::filesystem;
	// using some genie features to easily parse the command line args :
	auto [file_path ,event_file_path,xsec_file_path, model_name] = Utils::GetCommandLineArgs(argc, argv);

	if(!fs::exists(file_path))
	{
		pLOG("single_pion_main",pERROR) << "Couldn't load the data file , no such file or directory : " << file_path.c_str();
		return -1 ;
	}
	if(!fs::exists(event_file_path))
	{
		pLOG("single_pion_main",pERROR) << "Couldn't load the ghep event file , no such file or directory : " << event_file_path.c_str();
		return -1 ;
	}
	// Reading the file from the path : file_path :
	auto data_kinematics_opt = Utils::Read_data_file(file_path);
	if(!data_kinematics_opt.has_value())
	{
		pLOG("single_pion_main",pERROR) << "Couldn't read the data file at : " << file_path.c_str() ;
		return  -1;
	}
	// storing the returned std::vector <Kinematics> from there:
	auto& data_kinematics  = data_kinematics_opt.value();
    // Should implement a better way to print the dataset: 
	Utils::Print_dataset(data_kinematics, model_name.c_str(), file_path);
	

	auto dsigma_var = Utils::xsec_from_spline(event_file_path,xsec_file_path,1.5f);
	if(!dsigma_var.has_value())
	{
		pLOG("single_pion_main",pERROR) << "Got an empty vector : ";
		return -1 ; 
	}
	std::vector<CrossSectionBin> xsec_bins = dsigma_var.value();
	Utils::Write_xsec(xsec_bins, "../res/Egyan-like_bins_with_genie_model.root");
	return 0 ; 
}	// main





// /* First way to do comparison  : *///------------------------------------------
	// genie::XSecAlgorithmI* model = Utils::Init(model_name) ;
	// /*Second way to do comparison : *///------------------------------------------
	// genie::cmp::GCPlex * plex = genie::cmp::GCPlex::Instance();
	// plex->Configure(argc,argv);
		
	// if(!model)
	// {
	// 	pLOG("single_pion_main",pERROR) << "Couldn't create model : "<< model_name;
	// 	return -1 ;
	// }