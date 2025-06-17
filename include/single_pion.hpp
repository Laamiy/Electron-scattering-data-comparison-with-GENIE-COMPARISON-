#pragma once
//Stl
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <iostream>
#include <sstream>

// Root
#include <TBox.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPavesText.h>
#include <TPostScript.h>
#include <TSystem.h>
#include <TText.h>
#include <TVirtualFitter.h>
#include "TStorage.h"
#include <optional>
#include <vector>

// Genie

#include "Framework/Algorithm/AlgConfigPool.h"
#include "Framework/Algorithm/AlgFactory.h"
#include "Framework/Conventions/Constants.h"
#include "Framework/Conventions/Units.h"
#include "Framework/EventGen/XSecAlgorithmI.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/ParticleData/BaryonResUtils.h"
#include "Framework/ParticleData/BaryonResonance.h"
#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/Registry/Registry.h"
#include "Framework/Utils/CmdLnArgParser.h"
#include "Framework/Utils/KineUtils.h"
#include "Framework/Utils/RunOpt.h"
#include "Framework/Utils/StringUtils.h"
#include "Framework/Utils/Style.h"
#include "Framework/Utils/SystemUtils.h"
#include "Framework/Conventions/KineVar.h"
#include "Framework/Messenger/Messenger.h"

#define EXPECTED_SIZE 800



// Single pion produciton : e + p -> e' + n + pi_+ 
enum class target :  int
{
	PROTON   = 1000010010 ,
 	NEUTRON  = 1000000010
};

struct Kinematics 
{
	Kinematics() : q2(0.0f), w(0.0f) ,epsilon(0.0f), theta_pi(0.0f) ,sigma_0(0.0f),sigma_t(0.0f),sigma_l(0.0f)
	,unc_0(0.0f) ,unc_t(0.0f) ,unc_l(0.0f)
  {

	}
	Kinematics(double Q2 ,double W , double eps,double theta , double sigma,double unc, double sigmat,double sigmal) :
	q2(Q2), w(W) , epsilon(eps),theta_pi(theta) ,sigma_0(sigma),unc_0(unc),sigma_t(sigmat),sigma_l(sigmal)
	{

	} 
  friend std::ostream& operator<<(std::ostream& stream,const Kinematics& kine)
  {
      char buffer[392] ; 
      sprintf(buffer,"|| %14.7e GeV | %14.7e GeV | %14.7e n/a | %14.7e deg | %14.7e GeV | %14.7e n/a ||",
      kine.q2, kine.w ,kine.epsilon, kine.theta_pi ,kine.sigma_0, kine.unc_0 );
      stream << buffer <<'\n'; 
      return stream;
  }
	public : 
		double  q2 , w, epsilon ,theta_pi , sigma_0 ,sigma_t ,sigma_l,unc_0 ,unc_t ,unc_l;
};// Kinematics
  
  
// Utility class to hold info on plotted datasets
// Needs to be signleton -like .
class Spp_dataset_description 
{
  public:
    Spp_dataset_description(int tgtpdg, const std::string& expt , double E , double w , double q2 , 
                            double eps, double theta,double sigma_0,double unc,double sigma_t,double sigma_l)
    : m_target_pdg(tgtpdg), m_experiment(expt), m_kine(q2,w, eps,theta,sigma_0,unc,sigma_t,sigma_l) 
    {}

    Spp_dataset_description() = default ;
    
    virtual ~Spp_dataset_description(){};

    int Target_pdg(void) const
     { 
     	return m_target_pdg;
 	  }
    int Target_Z(void) const 
    {
     	return genie::pdg::IonPdgCodeToZ(m_target_pdg);
 	  }
    int Target_A(void) const 
    { 
		  return genie::pdg::IonPdgCodeToA(m_target_pdg);
    }
    std::string Expt(void) const 
    {
      return m_experiment;
    }
    double Theta(void) const 
    { 
    	return m_kine.theta_pi;
    }
    std::string Target_name(void) const
    {
  		// For now only nucleon  : P or N targets are valid
  		if (m_target_pdg      == (int)target::PROTON )
  			return "Proton"; // hydrogen
  		else if (m_target_pdg == (int)target::NEUTRON )
  			return "Neutron";
  		return "";
    } 
    std::string LabelTeX(void) const
    {
  	  std::ostringstream label;
      label << m_experiment << " (" << this->Target_name() << "), ";
      label << "W = " << m_kine.w << " GeV, ";
      label << "#theta = " << m_kine.theta_pi << "^{o}";
      return label.str();
    }

  private:
    int m_target_pdg;   //
    std::string m_experiment;  //
    Kinematics m_kine;

}; // class Spp_dataset_description

struct Cmd_args
{
  std::filesystem::path file_path  ; 
  std::string spp_model_name ;
};//Cmd_args

namespace Utils 
{
  namespace fs = std::filesystem ;
  Cmd_args GetCommandLineArgs(int argc, char **argv);  
  void  minus_uni_2_ascii(std::string& buffer,const std::string& uni_code ,const std::string& ascii_code);
  std::optional<std::vector<Kinematics>> Read_data_file(const fs::path& file_path);
  void Print_dataset(const std::vector<Kinematics>& data_kinematics,const char* model_name,const fs::path& file_path, bool save = false ) ;
}
