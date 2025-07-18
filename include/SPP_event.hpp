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
 	THnSparseD* sparse_hist{} ; 
 	Long64_t n_entries ;
 };// Event_charac

namespace Utils 
{
	namespace fs = std::filesystem ;
	Event_charac spp_read_events(const fs::path &filename) ;
	std::optional<std::vector<CrossSectionBin>>  xsec_from_spline(const fs::path & event_file_path,const fs::path &root_file_path,double energy_GeV ); 
};// Utils


// 	// Data members
// 	struct spp_prediction
// 	{
// 		bool                fIsConfigured;            //
// 		genie::Registry     fMetadata;                // Data about the data
// 		string              fName;                    // Prediction name
// 		string              fDataReleaseName;         // Name of MB data release GENIE is compared against
// 		const genie::cmp::GExDataI *    fDataSet;     // will probably not be used : use another way : // Corresponding dataset  
// 		string              fMCProName;               // MC production name
// 		TFile *             fXSecFile;                // GENIE cross-section spline file based on MC production name  
// 		int                 fEpdg;                    // electron-probe PDG code
// 		int                 fHitNucPdg;               // hit nucleon PDG (either free or bound)
// 		int                 fTgtPdg;                  // target PDG (nucleus code -if any- otherwise this defaults to the hit nucleon code)
// 		bool                fWithNuclearCorrection;   // effect of nucleus (if present) corrected in data
// 		bool                fIsCC;                    //
// 		bool                fIsNC;                    //
// 		genie::PDGCodeList *       fExclHadrSt;       // 
// 		double              fEmin;                    //
// 		double              fEmax;                    //
// 		bool                fScaleWithE;              //
// 		bool                fInLogX;                  //
// 		bool                fInLogY;                  //
// 		int                 fNPredBins;               // Prediction is discretised because histograms are used (see header); Specify the number of bins
// 		TH1D *              fNevX;                    //
// 		TH1D *              fNevCC;                   //

// 		double *            fWMin ;
// 		double *            fWMax ;
// 		double *            fPLeptonMin ;
// 		double *            fPLeptonMax ;
// 		double *            fTotPLonTrackMin ;
// 		double *            fTotPLonTrackMax ;

// 		TGraphAsymmErrors * fPrediction;              //
// 	};// spp_prediciton

// }; // Utils	
//    // 
