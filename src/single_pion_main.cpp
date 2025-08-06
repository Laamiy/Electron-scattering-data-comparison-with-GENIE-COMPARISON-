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
	// Utils::Print_dataset(data_kinematics, model_name.c_str(), file_path);
	

	auto dsigma_var = Utils::xsec_from_spline(event_file_path,xsec_file_path,1.52f);
	if(!dsigma_var.has_value())
	{
		pLOG("single_pion_main",pERROR) << "Got an empty vector : ";
		return -1 ; 
	}
	auto [ xsec_bins ,sigma_w , w_center] = dsigma_var.value();
	const fs::path Egyian_bin_like_mc = "../res/Egyan-like_bins_with_genie_model.root";

	Utils::Write_xsec(xsec_bins, Egyian_bin_like_mc.c_str());
	Utils::Plot_comparison( Egyian_bin_like_mc , data_kinematics , sigma_w , w_center );
	
	return 0 ; 
}	// main


