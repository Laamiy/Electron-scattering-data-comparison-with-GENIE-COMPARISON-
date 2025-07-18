#pragma  once
#include "single_pion.hpp"



// namespace Uitls 
// {
//   //_________________________________________________________________________________
// TGraphErrors*  Data(unsigned int iset)
// {
//   const double dE = 1.0E-3;
//   const double dtheta = 2.5E-2;

//   double E = gDataSets[iset]->E();
//   double theta = gDataSets[iset]->Theta();

//   int Z = gDataSets[iset]->TgtZ();
//   int A = gDataSets[iset]->TgtA();

//   const char *selection =
//       Form("E > %f && E < %f && theta > %f && theta < %f && Z == %d && A == %d",
//            E - dE, E + dE, theta - dtheta, theta + dtheta, Z, A);

//   gResDataTree->Draw("W2:xsec:xsec_err", selection, "goff");

//   int n = gResDataTree->GetSelectedRows();

//   //  LOG("gvld_eN_jlab", pNOTICE)
//   //  << "Found " << n << " data points in the xsec archive";

//   if (n == 0)
//     return 0; // return null graph

//   // Data returned by TTree::Draw() are not necessarily ordered in W
//   // Do the ordering here before building the graph
//   int *idx = new int[n];
//   double *xv = new double[n];
//   double *yv = new double[n];
//   double *dyv = new double[n];

//   TMath::Sort(n, gResDataTree->GetV1(), idx, false);

//   for (int i = 0; i < n; i++)
//   {
//     int ii = idx[i];
//     xv[i] = (gResDataTree->GetV1())[ii];
//     yv[i] = (gResDataTree->GetV2())[ii];
//     dyv[i] = (gResDataTree->GetV3())[ii];
//   }

//   TGraphErrors *gr = new TGraphErrors(n, xv, yv, 0, dyv);
//   genie::utils::style::Format(gr, 1, 1, 1, 1, 8, 0.4);

//   delete[] idx;
//   delete[] xv;
//   delete[] yv;
//   delete[] dyv;

//   return gr;
// }
// 	//_________________________________________________________________________________
// void Draw(unsigned int iset)
// {
//    const uint32_t kNCx  = 2 , kNCy = 2;
//   // get all measurements for the current channel from the NuValidator MySQL
//   // dbase
//   TGraphErrors *data = Data(iset);
//   if (!data)
//     return;

//   // get the corresponding GENIE model prediction
//   vector<vector<TGraph *>> model(2);
//   model[0] = Model(iset,0);

//   int plots_per_page = kNCx * kNCy;
//   int iplot = 1 + iset % plots_per_page;

//   if (iplot == 1)
//   {
//     gPS->NewPage();
//     gC->Clear();
//     gC->Divide(kNCx, kNCy);
//   }

//   gC->GetPad(iplot)->Range(0, 0, 100, 100);
//   gC->GetPad(iplot)->SetFillColor(0);
//   gC->GetPad(iplot)->SetBorderMode(0);
//   gC->GetPad(iplot)->cd();

//   TH1F *hframe = 0;
//   double xmin = 9999999999, scale_xmin = 0.50;
//   double xmax = -9999999999, scale_xmax = 1.05;
//   double ymin = 9999999999, scale_ymin = 0.40;
//   double ymax = -9999999999, scale_ymax = 1.30;
//   // get W2 and d2sigma/dEdOmega range in the the data
//   xmin = (data->GetX())[TMath::LocMin(data->GetN(), data->GetX())];
//   xmax = (data->GetX())[TMath::LocMax(data->GetN(), data->GetX())];
//   ymin = (data->GetY())[TMath::LocMin(data->GetN(), data->GetY())];
//   ymax = (data->GetY())[TMath::LocMax(data->GetN(), data->GetY())];
//   xmin = TMath::Max(xmin, 0.5); // some data go very low
//   // take also into account the d2sigma/dEdOmega range in the the models
//   // (for the W2 range of the dataset) just in case data and MC are not that
//   // similar...
//   for (unsigned int imodel = 0; imodel < model.size(); imodel++) 
//   {
//     for (unsigned int imode = 0; imode < model[imodel].size(); imode++)
//     {
//       TGraph *mm = model[imodel][imode];
//       if (mm)
//       {
//         for (int k = 0; k < mm->GetN(); k++)
//         {
//           double x = (mm->GetX())[k];
//           if (x < xmin || x > xmax)
//             continue;
//           ymin = TMath::Min(ymin, (mm->GetY())[k]);
//           ymax = TMath::Max(ymax, (mm->GetY())[k]);
//         } // k
//       } // mm
//     } // imode
//   } // imodel

//   // LOG("gvld_eN_jlab", pNOTICE)
//   //     << "Plot range:"
//   //     << "W^{2} = [" << xmin << ", " << xmax << "] GeV^{2}, "
//   //     << "d^{2}#sigma / d#Omega dE = [" << ymin << ", " << ymax
//   //     << "]  nb/sr/GeV";

//   hframe = (TH1F *)gC->GetPad(iplot)->DrawFrame(
//       scale_xmin * xmin, scale_ymin * ymin, scale_xmax * xmax,
//       scale_ymax * ymax);
//   hframe->Draw();
//   hframe->GetXaxis()->SetTitle("W^{2} (GeV^{2})");
//   hframe->GetYaxis()->SetTitle("d^{2}#sigma / d#Omega dE (nb/sr/GeV)");

//   // draw data and GENIE models
//   data->Draw("P");
//   for (unsigned int imodel = 0; imodel < model.size(); imodel++) {
//     for (unsigned int imode = 0; imode < model[imodel].size(); imode++) {
//       TGraph *mm = model[imodel][imode];
//       if (!mm)
//         continue;
//       mm->Draw("L");
//     }
//   }

//   // add legend
//   double lymin = (gOptRESFitEnabled || gOptRESFitEnabled) ? 0.65 : 0.75;
//   TLegend *legend = new TLegend(0.20, lymin, 0.50, 0.85);
//   legend->SetLineStyle(0);
//   legend->SetFillStyle(0);
//   legend->SetTextSize(0.025);
//   legend->SetHeader("GENIE");
//   for (unsigned int imodel = 0; imodel < model.size(); imodel++)
//   {
//     for (unsigned int imode = 0; imode < model[imodel].size(); imode++)
//     {
//       TGraph *mm = model[imodel][imode];
//       if (!mm)
//         continue;
//       legend->AddEntry(mm, mm->GetTitle(), "L");
//     }
//   }
//   legend->Draw();

//   // scaling region
//   TBox *scaling_region = 0;
//   if (kDrawHatchcedScalingRegion)
//   {
//     double W2c = kWcut * kWcut;
//     if (W2c > scale_xmin * xmin && W2c < scale_xmax * xmax)
//     {
//       scaling_region = new TBox(W2c, scale_ymin * ymin, scale_xmax * xmax,
//                                 scale_ymax * ymax);
//       //       scaling_region->SetFillColor(kRed);
//       //       scaling_region->SetFillStyle(3905);
//       scaling_region->SetFillStyle(0);
//       scaling_region->SetLineColor(kRed);
//       scaling_region->SetLineStyle(kDashed);
//       scaling_region->Draw();
//     }
//   }

//   // some data show the elastic peak - mark the are to avoid confusion
//   if (xmin < 1)
//   {
//     double Wm2 = 1.21; // between the QE and Delta peaks
//     TBox *qe_peak =
//         new TBox(scale_xmin * xmin, scale_ymin * ymin, Wm2, scale_ymax * ymax);
//     qe_peak->SetFillColor(kBlue);
//     qe_peak->SetFillStyle(3005);
//     qe_peak->Draw();
//   }

//   // title
//   TLatex *title = new TLatex(
//       scale_xmin * xmin + 0.2 * (scale_xmax * xmax - scale_xmin * xmin),
//       1.01 * scale_ymax * ymax, gDataSets[iset]->LabelTeX().c_str());
//   title->SetTextSize(0.027);
//   title->Draw();

//   gC->GetPad(iplot)->Update();
//   gC->Update();
//   }

//   //____________________________________________
//   //
//   vector<TGraph *> Model(unsigned int iset, unsigned int imodel) 
//   {
//   // Get GENIE predictions for the `iset' dataset

//   vector<TGraph *> model;
//   //
//   // LOG("gvld_eN_jlab", pNOTICE)
//   // << "Getting GENIE prediction (model ID = "
//   //  << imodel << ", data set ID = " << iset << ")";

//   bool calc_res = (gRESXSecModel != 0);
//   bool calc_dis = (gDISXSecModel != 0);
//   bool calc_tot = calc_res && calc_dis;

//   double M = kNucleonMass;
//   double M2 = M * M;

//   double E = gDataSets[iset]->E();
//   double theta = gDataSets[iset]->Theta();
//   double costh = TMath::Cos(2 * kPi * theta / 360.);

//   //  LOG("gvld_eN_jlab", pINFO)
//   //  << " E = " << E
//   //  << ", theta = " << theta << " (cos(theta) = " << costh << ")";

//   int Z = gDataSets[iset]->TgtZ();
//   int A = gDataSets[iset]->TgtA();
//   int N = A - Z;
//   bool tgt_has_p = (Z > 0);
//   bool tgt_has_n = (N > 0);
//   double number_p = (double)Z;
//   double number_n = (double)N;

//   const int n = 500;

//   double W2_array[n];
//   double d2sigRES_dEpdOmega_array[n];
//   double d2sigDIS_dEpdOmega_array[n];
//   double d2sigTOT_dEpdOmega_array[n];

//   double Epmin = 0.01;
//   double Epmax = E;
//   double dEp = (Epmax - Epmin) / (n - 1);

//   for (int i = 0; i < n; i++) 
//   {
//     double Ep = Epmin + i * dEp;
//     double Q2 = 2 * E * Ep * (1 - costh);
//     double W2 = M2 + 2 * M * (E - Ep) - Q2;

//     if (W2 <= 0) {
//       //    LOG("gvld_eN_jlab", pDEBUG)
//       //   << "Ep = " << Ep << ", Q2 = " << Q2 << ", W2 = " << W2
//       //  << "... Skipping point";
//       W2_array[i] = 0.;
//       continue;
//     }
//     W2_array[i] = W2;

//     double W = TMath::Sqrt(TMath::Max(0., W2));

//     d2sigRES_dEpdOmega_array[i] = 0;
//     d2sigDIS_dEpdOmega_array[i] = 0;
//     d2sigTOT_dEpdOmega_array[i] = 0;

//     // Will calculate  d^2 sigma / dW dQ^2 and then convert to d^2sigma / dE'
//     // dOmega
//     //     double jacobian = (E*Ep)*(M+2*E*(1-costh))/(kPi*W);
//     double jacobian = (E * Ep * M) / (kPi * W);

//     //
//     // Calculate resonance cross-section
//     //

//     if (calc_res) 
//     {

//       double d2sigRES_dWdQ2 = 0.;
//       double d2sigRES_dWdQ2_p = 0.;
//       double d2sigRES_dWdQ2_n = 0.;

//       //
//       //??  e+p -> e+Resonance
//       //!!  COULD POTENTIALLY CHANGE MAKE CHANGES HERE TO MAKE IT WORK : 
//       if (tgt_has_p)
//       {
//         Interaction *ep_res = Interaction::SPP(1000010010, kPdgProton, kPdgElectron, E);//RESEM(1000010010, kPdgProton, kPdgElectron, E);
//         ep_res->KinePtr()->SetW(W);
//         ep_res->KinePtr()->SetQ2(Q2);
//         // ep_res->set_spp_int_flags(0,1); 
//         // loop over resonances
//         for (int ires = 0; ires < kNRes; ires++)
//         {
//           ep_res->ExclTagPtr()->SetResonance(kResId[ires]);
//          double xsec = gRESXSecModel->XSec(ep_res, kPSWQ2fE)/ units::nb; 
//           xsec /=10; // ! L : This makes the best fit so far .
//           xsec = TMath::Max(0., xsec);
//           d2sigRES_dWdQ2_p += xsec;
//              // Was commented before : --------------------------------
//               LOG("gvld_eN_jlab", pINFO)<< "d2xsec_dWdQ2(ep; " << utils::res::AsString(kResId[ires])
//             << "; E = " << E << " GeV, W = " << W << " GeV, Q2 = " << Q2 <<" GeV^2"
//             << "; Ep = " << Ep << " GeV, theta = " << theta << " deg) = "
//             << xsec << " nbarn/GeV^3";
//             // Was commented before : ------------------------------ 
//         } // res
//         LOG("gvld_eN_jlab", pINFO)<< "Current interaction : " << *ep_res  ; 
//         delete ep_res;
//       } // has_p

//       //
//       //??  e+n -> e+Resonance
//       //
//       if (tgt_has_n)
//       {
//         Interaction *en_res = Interaction::RESEM(1000000010, kPdgNeutron, kPdgElectron, E);
//         en_res->KinePtr()->SetW(W);
//         en_res->KinePtr()->SetQ2(Q2);
//         // loop over resonances
//         for (int ires = 0; ires < kNRes; ires++)
//         {
//           en_res->ExclTagPtr()->SetResonance(kResId[ires]);
//           double xsec = gRESXSecModel->XSec(en_res, kPSWQ2fE) / units::nb;
//           xsec = TMath::Max(0., xsec);
//           d2sigRES_dWdQ2_n += xsec;

//           //        LOG("gvld_eN_jlab", pINFO)
//           //  << "d2xsec_dWdQ2(en; " << utils::res::AsString(kResId[ires])
//           //  << "; E = " << E << " GeV, W = " << W << " GeV, Q2 = " << Q2 <<
//           //" GeV^2"
//           //  << "; Ep = " << Ep << " GeV, theta = " << theta << " deg) = "
//           //  << xsec << " nbarn/GeV^3";

//         } // res
//         delete en_res;
//       } // has_n

//       // sum over resonances and averaged over p,n
//       d2sigRES_dWdQ2 = (number_p * d2sigRES_dWdQ2_p + number_n * d2sigRES_dWdQ2_n);

//       // convert to d^2sigma / dE' dOmega and store
//       double d2sigRES_dEpdOmega = jacobian * d2sigRES_dWdQ2;
//       if (TMath::IsNaN(d2sigRES_dEpdOmega))
//       {
//         LOG("gvld_eN_jlab", pWARN) << "Got a NaN!";
//         d2sigRES_dEpdOmega = 0;
//       }
//       d2sigRES_dEpdOmega_array[i] = TMath::Max(0., d2sigRES_dEpdOmega);

//     } // calc_res

//     //
//     // Calculate NRB and DIS cross-section
//     //

//     if (calc_dis)
//     {
//       double d2sigDIS_dWdQ2 = 0.;
//       double d2sigDIS_dWdQ2_p = 0.;
//       double d2sigDIS_dWdQ2_n = 0.;

//       double x = 0;
//       double y = 0;
//       utils::kinematics::WQ2toXY(E, M, W, Q2, x, y);

//       if (tgt_has_p)
//       {
//         // Note: Not setting quark ID
//         // If the quark ID is set, the code returns (eg for neutrino CC) the
//         // vq->lq' cross-section. But if a quark ID is not specified then the
//         // code loops over all relevant valence and sea quark species and
//         // returns (eg for neutrino CC) the vN->lX cross-section.
//         Interaction *ep_dis =
//             Interaction::DISEM(1000010010, kPdgProton, kPdgElectron, E);
//         ep_dis->KinePtr()->SetW(W);
//         ep_dis->KinePtr()->SetQ2(Q2);
//         ep_dis->KinePtr()->Setx(x);
//         ep_dis->KinePtr()->Sety(y);

//         d2sigDIS_dWdQ2_p = gDISXSecModel->XSec(ep_dis, kPSWQ2fE) / units::nb;
//         d2sigDIS_dWdQ2_p = TMath::Max(0., d2sigDIS_dWdQ2_p);

//         delete ep_dis;
//       }
//       if (tgt_has_n)
//       {
//         Interaction *en_dis =
//             Interaction::DISEM(1000000010, kPdgNeutron, kPdgElectron, E);
//         en_dis->KinePtr()->SetW(W);
//         en_dis->KinePtr()->SetQ2(Q2);
//         en_dis->KinePtr()->Setx(x);
//         en_dis->KinePtr()->Sety(y);

//         d2sigDIS_dWdQ2_n = gDISXSecModel->XSec(en_dis, kPSWQ2fE) / units::nb;
//         d2sigDIS_dWdQ2_n = TMath::Max(0., d2sigDIS_dWdQ2_n);

//         delete en_dis;
//       }

//       d2sigDIS_dWdQ2 =
//           (number_p * d2sigDIS_dWdQ2_p + number_n * d2sigDIS_dWdQ2_n);

//       // convert to d^2sigma / dE' dOmega and store
//       double d2sigDIS_dEpdOmega = jacobian * d2sigDIS_dWdQ2;
//       if (TMath::IsNaN(d2sigDIS_dEpdOmega))
//       {
//         /// LOG("gvld_eN_jlab", pWARN) << "Got a NaN!";
//         d2sigDIS_dEpdOmega = 0;
//       }

//       d2sigDIS_dEpdOmega_array[i] = TMath::Max(0., d2sigDIS_dEpdOmega);

//     } // calc_dis

//     //
//     // Calculate total cross-section
//     //
//     if (calc_tot)
//     {
//       d2sigTOT_dEpdOmega_array[i] =
//           d2sigRES_dEpdOmega_array[i] + d2sigDIS_dEpdOmega_array[i];
//     }

//   } // i

//   // Create graphs & store them in array
//   if (calc_tot)
//   {
//     TGraph *gr = new TGraph(n, W2_array, d2sigTOT_dEpdOmega_array);
//     const char *title = 0;
//     int lstyle = kSolid;
//     if (gOptRESFitEnabled || gOptDISFitEnabled)
//     {
//       if (imodel == 0) {
//         title = "Total (Best-fit)";
//       } else if (imodel == 1) {
//         title = "Total (Nominal)";
//         lstyle = kDashed;
//       }
//     } else {
//       title = "Total";
//     }
//     gr->SetLineColor(kBlack);
//     gr->SetLineStyle(lstyle);
//     gr->SetTitle(title);
//     model.push_back(gr);
//   }
//   if (calc_res) 
//   {
//     TGraph *gr = new TGraph(n, W2_array, d2sigRES_dEpdOmega_array);
//     const char *title = 0;
//     int lstyle = kSolid;
//     if (gOptRESFitEnabled || gOptDISFitEnabled)
//     {
//         if (imodel == 0)
//         {
//           title = "Resonance (Best-fit)";
//         } else if (imodel == 1) {
//           title = "Resonance (Nominal)";
//           lstyle = kDashed;
//         }
//     }
//     else 
//     {
//       title = "Resonance";
//     }
//     gr->SetLineColor(kRed);
//     gr->SetLineStyle(lstyle);
//     gr->SetTitle(title);
//     model.push_back(gr);
//   }
//   if (calc_dis)
//   {
//     TGraph *gr = new TGraph(n, W2_array, d2sigDIS_dEpdOmega_array);
//     const char *title = 0;
//     int lstyle = kSolid;
//     if (gOptRESFitEnabled || gOptDISFitEnabled)
//     {
//       if (imodel == 0)
//       {
//         title = "DIS (Best-fit)";
//       }
//       else if (imodel == 1)
//       {
//         title = "DIS (Nominal)";
//         lstyle = kDashed;
//       }
//     }
//     else
//     {
//       title = "DIS";
//     }
//     gr->SetLineColor(kBlue);
//     gr->SetLineStyle(lstyle);
//     gr->SetTitle(title);
//     model.push_back(gr);
//   }

//   return model;
// }
//  }// void Draw
