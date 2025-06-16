#include "Framework/Messenger/Messenger.h"
#include "single_pion.hpp"
#include <iostream>

using namespace genie::constants;

int main(int argc , char* argv[])
{
	//
	namespace fs = std::filesystem;
	Cmd_args args = Utils::GetCommandLineArgs(argc, argv);
	auto [file_path , model_name] = args ;
	if(!fs::exists(file_path))
	{
		std::cerr << "[ERROR] : Couldn't load the data file\n";
		return -1 ;
	}
	std::cout  <<"[INFO] : Model name : " << model_name <<'\n';
	std::cout << "[INFO] : Egiyan data file path : "<<file_path<<"\n";
	std::cin.get();
	return 0 ; 
}	