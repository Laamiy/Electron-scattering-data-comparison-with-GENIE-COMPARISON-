
#include "Xsec_sim.hpp"
#include "single_pion.hpp"

// The logs should use LOG from log4cpp but trouble with a core genie class for now .
int main(int argc , char* argv[])
{
	using namespace genie::constants;
	namespace fs =  std::filesystem;
	// using some genie features to easily parse the command line args :
	auto [file_path , model_name] = Utils::GetCommandLineArgs(argc, argv);
	if(!fs::exists(file_path))
	{
		pLOG("single_pion_main",pERROR) << "Couldn't load the data file , no such file or directory : " << file_path.c_str();
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
	genie::XSecAlgorithmI* model = Utils::Init(model_name) ;
	if(!model)
	{
		pLOG("single_pion_main",pERROR) << "Couldn't create model : "<< model;
		return -1 ;
	}
	return 0 ; 
}	// main