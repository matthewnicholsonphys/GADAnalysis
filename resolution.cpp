#include <iostream>

#include "TFile.h"
#include "TMultiGraph.h"
#include "TGraph.h"
#include "TF1.h"

int main(int argc, char* argv[]){
  if (argc != 2){
    std::cout << "usage: ./resolution abscaling.root\n";
    return 1;
  }

  TFile* file = TFile::Open(argv[1], "OPEN");
  TMultiGraph* m_abs = static_cast<TMultiGraph*>(file->Get("Graph"));

  TGraph* led_a = static_cast<TGraph*>(m_abs->GetListOfGraphs()->At(0));
  //  led_a->SetName("Graph");
  TGraph* led_b = static_cast<TGraph*>(m_abs->GetListOfGraphs()->At(1));
  //led_b->SetName("Graph");
  
  TF1 cal_a = TF1("led_a_calcurve", "pol6", 0, 0.22);
  led_a->Fit(&cal_a, "R");
  TF1 cal_b = TF1("led_b_calcurve", "pol6", 0, 0.22);
  led_b->Fit(&cal_b, "R");

  std::vector<double> Xv;
  std::vector<double> Yv;

  cal_a.SaveAs("cal_a.root");
  cal_b.SaveAs("cal_b.root");
  
  for (double y = 0; y < 1; y += 0.01){
    double A = cal_a.GetX(y), B = cal_b.GetX(y);
    Xv.push_back(0.5*(A+B));
    Yv.push_back(50*(A-B)/(A+B));
  }

  //led_a->SaveAs("polfit_led_A.root");
  //led_b->SaveAs("polfit_led_B.root");
  
  TGraph g = TGraph(static_cast<int>(Xv.size()), Xv.data(), Yv.data());
  g.SaveAs("res.root");
  
  return 0;
}
