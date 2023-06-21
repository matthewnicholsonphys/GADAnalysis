#include "gad_utils.h"

#include <string>
#include <vector>

#include "TGraph.h"
#include "TMultiGraph.h"

int main(){

  static const std::string dataset_str = "/home/matthew/gad_work/EmptyTubeStability/EmptyTube/04/OldModuleEmptyTubeStability_";
  static const int number_of_files = 2648;

  struct LEDInfo {
    std::string name = "";
    std::string calib_fname = "";
    std::string calib_curve_name = "";
    int dark_offset = 0;
    std::vector<double> initial_combined_fit_param_values;
    std::vector<std::pair<double, double>> fit_param_ranges;
  };
  
  static std::vector<LEDInfo> led_infos = {
    {"275_A", "/home/matthew/gad_work/gad_utils/cal_a.root", "led_a_calcurve", 10,
     {12.266, -0.0957, 0.9, 0.726, 0, -0.0531, 0.164},
     {{0.1, 20}, {-5, 5}, {0.5, 1.1}, {0.5,1}, {-0.3, 0.3}, {-0.3,0.3}, {-100, 1000}}},

    {"275_B", "/home/matthew/gad_work/gad_utils/cal_b.root", "led_b_calcurve", 9,
     {1, 0, 0, 0.3, 0, 0, 0},
     {{0.1, 20}, {-5, 5}, {0.9, 1.1}, {0,1}, {-0.3, 0.3}, {-0.3,0.3}, {-100, 1000}}}
  };
  
  for (const auto& led : led_infos){
    TMultiGraph mg = TMultiGraph("Graph", "Graph");
    TGraph peak;
    peak.SetTitle("Graph");
    peak.SetName("Graph");
    for (int file_idx = 0; file_idx < number_of_files; ++file_idx){

      std::string file_idx_str = std::to_string(file_idx);
      const std::string file_number_str = file_idx_str.insert(0, 5 - file_idx_str.size(), '0');

      TGraph* dark_sub = new TGraph(ZeroNegative(GetDarkSubtractFromFile(dataset_str+file_number_str+".root", led.name, led.dark_offset)));
      peak.AddPoint(file_idx, dark_sub->Eval(270));
      mg.Add(dark_sub);
    }
    peak.SaveAs(("emptytube_peak_"+led.name+".root").c_str());
    mg.SaveAs(("emptytube_"+led.name+".root").c_str());
  }
  
  return 0;
}
