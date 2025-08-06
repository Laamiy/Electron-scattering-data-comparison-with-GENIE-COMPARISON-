#pragma once 
#include "single_pion.hpp"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Ntuple/NtpMCEventRecord.h"
#include "Framework/ParticleData/PDGCodes.h"

#include "Framework/Conventions/Controls.h"
#include "Framework/Conventions/Units.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Numerical/Spline.h"
#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/Utils/XSecSplineList.h"
#include "Framework/Utils/XmlParserUtils.h"
#include "RtypesCore.h"
#include "TString.h"
#include "THnSparse.h"
#include "single_pion.hpp"

// Particles in the record
 struct Event_charac
 {
 	THnSparseD* sparse_hist{} , *h_gamma{} ; 
 	Long64_t n_entries ;
 };// Event_charac
struct xsec_n_w
{
	std::vector<CrossSectionBin> xsec ; 
	std::vector<double> sigma_w; 
	std::vector<double> w ;
}; // xsec_n_w
namespace Utils 
{
	namespace fs = std::filesystem ;
	Event_charac spp_read_events(const fs::path &filename) ;
	std::optional<xsec_n_w> xsec_from_spline(const fs::path & event_file_path,const fs::path &root_file_path,double energy_GeV ) ; 

};// Utils
