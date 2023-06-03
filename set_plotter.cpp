#include <iostream>
#include <map>

#include "TFile.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLegend.h"

static std::string plot_dir = "/home/matthew/gad_work/gad_utils/set_plots/";

int main(int argc, char* argv[]){
  try{

    if (argc != 4){
      throw std::runtime_error("wrong args: usage is ./set_plotter <abs> <combchi2> <abschi2>");
    }

    std::map<TMultiGraph*, std::string> mvec = {};
    
    TFile* abs = TFile::Open(argv[1], "OPEN");
    TMultiGraph* m_abs = static_cast<TMultiGraph*>(abs->Get("Graph"));
    mvec[m_abs] = plot_dir + "/absscaling";
    abs->Close();
  

    TFile* comb_chi2 = TFile::Open(argv[2], "OPEN");
    TMultiGraph* m_combchi2 = static_cast<TMultiGraph*>(comb_chi2->Get("Graph"));
    mvec[m_combchi2] = plot_dir + "/chicomb";
    comb_chi2->Close();

    TFile* abs_chi2 = TFile::Open(argv[3], "OPEN");
    TMultiGraph* m_abschi2 = static_cast<TMultiGraph*>(abs_chi2->Get("Graph"));
    mvec[m_abschi2] = plot_dir + "/chiabs";
    abs_chi2->Close();

    for (const auto& [g, fname] : mvec){
	
      TCanvas c1 = TCanvas("c1", "c1", 2500, 2000);
      c1.cd();
      c1.SetGrid();
      g->Draw("apl");
      for (int i = 0; i < g->GetListOfGraphs()->GetSize(); ++i){ 
	TGraph* f = static_cast<TGraph*>(g->GetListOfGraphs()->At(i));
	f->SetMarkerStyle(9);
	f->SetMarkerSize(10);
	f->SetMarkerColor(static_cast<Color_t>(i+1));
	f->SetLineColor(static_cast<Color_t>(i+1));
	f->SetLineWidth(3);
      }
      TLegend* l1 = c1.BuildLegend(0.15, 0.7, 0.3, 0.85);
      l1->SetLineWidth(0);
      l1->SetFillColor(kGray);
      c1.Modified();
      c1.SaveAs((fname+".png").c_str());
    }
	
    
  }
  catch (const std::exception& e){
    std::cout << e.what() << std::endl;
    return 1;
  }
				 
  return 0;
}
