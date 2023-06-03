#include <iostream>

#include "TFile.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLegend.h"

int main(int argc, char* argv[]){
  if (argc != 11){
    std::cout << "INCORRECT NUMBER OF ARGUMENTS GIVEN: " << argc << " - CORRECT FORMAT IS:\n\n";
    std::cout << "./lookin  <outputfile> <meas_number> <LED> <absratio> <absfit> <absres>  <puremeasfit> <measfit> <measdata> <combres>  \n";
    // abs_23_275_A.root absfit_23_275_A.root absres_23_275_A.root cf_pure_comp_23_275_A.root combfit_23_275_A.root meas_23_275_A.root res_23_275_A.root
    return 1;
  }
  [[maybe_unused]] const std::string outfname = std::string(argv[1]);
  //  std::cout << "outfname: " << outfname << "\n";
  [[maybe_unused]] const int measurement_number = atoi(argv[2]);
  //std::cout << "meas number: " << measurement_number << "\n";
  [[maybe_unused]] const std::string led_name = std::string(argv[3]);
  // std::cout << "led: " << led_name << "\n";

  try {
    TFile* abs_f_ptr = TFile::Open(argv[4], "OPEN");
    //std::cout<< "abs:ratio :: " << argv[4] << std::endl;
    TGraph* abs_ptr = new TGraph(); abs_ptr = nullptr;
    abs_ptr = static_cast<TGraph*>(abs_f_ptr->Get("Graph"));
    if (abs_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[4]) + "!!!\n");
    }
    abs_ptr->SetTitle("abs");
    abs_f_ptr->Close();

    TFile* absfit_f_ptr = TFile::Open(argv[5], "OPEN");
    //std::cout<< "abs:fit :: " << argv[5] << std::endl;
    TGraph* absfit_ptr = new TGraph(); absfit_ptr = nullptr;
    absfit_ptr = static_cast<TGraph*>(absfit_f_ptr->Get("Graph"));
    if (absfit_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[5]) + "!!!\n");
    }
    absfit_ptr->SetTitle("fit");
    absfit_f_ptr->Close();
    
    TFile* absres_f_ptr = TFile::Open(argv[6], "OPEN");
    //std::cout<< "abs:residual :: " << argv[6] << std::endl;
    TGraph* absres_ptr = static_cast<TGraph*>(absres_f_ptr->Get("Graph"));
    if (absres_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[6]) + "!!!\n");
    }
    absres_ptr->SetTitle("Absorbance Fit Residual; wavelength [nm]; frac. residual");
    absres_f_ptr->Close();
    
    
    TFile* purefit_f_ptr = TFile::Open(argv[7], "OPEN");
    //std::cout<< "purecomp :: " << argv[7] << std::endl;
    TGraph* pure_ptr = new TGraph(); pure_ptr = nullptr;
    pure_ptr = static_cast<TGraph*>(purefit_f_ptr->Get("Graph"));
    if (pure_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[7]) + "!!!\n");
    }
    pure_ptr->SetTitle("fit w/o Gd"); 
    purefit_f_ptr->Close();
    
    TFile* fit_f_ptr = TFile::Open(argv[8], "OPEN");
    TGraph* fit_ptr = new TGraph(); fit_ptr = nullptr;
    fit_ptr = static_cast<TGraph*>(fit_f_ptr->Get("Graph"));
    if (fit_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[8]) + "!!!\n");
    }
    fit_ptr->SetTitle("fit");
    fit_f_ptr->Close();

    TFile* meas_f_ptr = TFile::Open(argv[9], "OPEN");
    //std::cout<< "meas:data :: " << argv[9] << std::endl;
    TGraph* meas_ptr = new TGraph(); meas_ptr = nullptr;
    meas_ptr = static_cast<TGraph*>(meas_f_ptr->Get("Graph"));
    if (meas_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[9]) + "!!!\n");
    }
    meas_ptr->SetTitle("data");
    meas_f_ptr->Close();

    TFile* combres_f_ptr = TFile::Open(argv[10], "OPEN");
    //std::cout<< "measfit :: " << argv[4] << std::endl;
    TGraph* combres_ptr = static_cast<TGraph*>(combres_f_ptr->Get("Graph"));
    if (combres_ptr == nullptr){
      throw std::invalid_argument("COULD NOT OPEN" + std::string(argv[10]) + "!!!\n");
    }
    combres_ptr->SetTitle("Combined Fit Residual; wavelength [nm]; frac. residual");
    combres_f_ptr->Close();

    TCanvas c1 = TCanvas("c1", ("LED: "+led_name+" i: "+std::to_string(measurement_number)).c_str(), 2500, 2000);
    c1.Divide(2,2);

    c1.cd(1);
    TMultiGraph comb = TMultiGraph("comb", ("Measurement "+std::to_string(measurement_number)+"  w/ Combined Fit;wavelength [nm]; intensity").c_str());

    const auto lw = 3;
    
    meas_ptr->SetLineWidth(lw);
    comb.Add(meas_ptr);
    fit_ptr->SetLineWidth(lw);
    fit_ptr->SetLineColor(kRed);
    comb.Add(fit_ptr);
    pure_ptr->SetLineWidth(lw);
    pure_ptr->SetLineStyle(7);
    comb.Add(pure_ptr);
    comb.GetXaxis()->SetRangeUser(250, 300);
    comb.GetYaxis()->SetRangeUser(0, 70000);
    
    comb.Draw("apl");
    TLegend* l1 = c1.GetPad(1)->BuildLegend(0.60, 0.60, 0.88, 0.88);
    l1->SetFillColor(kGray);
    l1->SetLineWidth(0);

    
    c1.cd(2);
    c1.GetPad(2)->SetGrid();
    combres_ptr->GetXaxis()->SetRangeUser(250, 300);
    combres_ptr->GetYaxis()->SetRangeUser(-0.05, 1.5);
    combres_ptr->SetLineWidth(lw);
    combres_ptr->Draw();
    
    
    c1.cd(3);
    TMultiGraph abs = TMultiGraph("abs", "Absorbance;wavelength [nm];");
    abs_ptr->SetLineWidth(lw);
    abs.Add(abs_ptr);
    absfit_ptr->SetLineWidth(lw);
    absfit_ptr->SetLineColor(kRed);
    abs.Add(absfit_ptr);
    abs.GetXaxis()->SetRangeUser(250, 300);
    abs.GetYaxis()->SetRangeUser(0, 7);
    abs.Draw("apl");
    TLegend* l2 = c1.GetPad(3)->BuildLegend(0.6, 0.6, 0.88, 0.88);
    l2->SetLineWidth(0);
    l2->SetFillColor(kGray);

    c1.cd(4);
    c1.GetPad(4)->SetGrid();
    absres_ptr->GetXaxis()->SetRangeUser(250, 300);
    absres_ptr->GetYaxis()->SetRangeUser(-0.15, 0.15);
    absres_ptr->SetLineWidth(lw);
    absres_ptr->Draw();
    
    c1.SaveAs(outfname.c_str());
    
    //std::cout<< "made " << outfname << " plot!!\n";
    
  }
  catch( std::exception& e){
    std::cout << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
