#ifndef __SPP__
#define __SPP__
//Stl
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
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



// Single pion produciton : e + p -> e' + n + pi_+ 
// Utility class to hold info on plotted datasets
enum class target :  int
{
	PROTON   = 1000010010 ,
 	NEUTRON  = 1000000010
};

struct Kinematics 
{
	Kinematics() : q2(0.0f), w(0.0f) , theta_pi(0.0f) ,sigma(0.0f),unc(0.0f)
	{

	}
	Kinematics(double Q2 ,double W , double theta , double xsec, double Unc) :
	q2(Q2), w(W) , theta_pi(theta) ,sigma(xsec),unc(Unc)
	{

	} 
	public : 
		double  q2 , w, theta_pi , sigma ,unc ;
};// Kinematics

class Spp_dataset_description 
{
  public:
    Spp_dataset_description(int tgtpdg, const std::string& expt , double E , double w , double q2, double theta,double xsec,double Unc)
    : m_target_pdg(tgtpdg), m_experiment(expt), m_kine(q2,w,theta,xsec,Unc) 
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
  const char* spp_model_name ;
};
namespace Utils 
{
  Cmd_args GetCommandLineArgs(int argc, char **argv);  
}
#endif