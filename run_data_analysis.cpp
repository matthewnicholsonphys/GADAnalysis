#include "gad_utils.h"

#include "TApplication.h"
#include "TSystem.h"
#include <thread>
#include <chrono>
#include <fstream>

[[maybe_unused]] static int numb_of_data_files = 1780;
[[maybe_unused]] static int numb_of_calibfiles = 54;
static std::string calib_str = "/home/matthew/gad_work/LastChanceCal/run_00184_17_April_2023_calibfile_";
static std::string dataset_str = "/home/matthew/gad_work/gad_data_april2023/00185_25_Apr_2023_EGADS_Run_";
static std::string pure_dataset_str = "/home/matthew/gad_work/pure_data1/00182_24MarPureStability_";

std::vector<double> PopulateConc(const std::string& fname){
  std::vector<double> result;
  std::fstream fs(fname);
  if (!fs){
    throw std::invalid_argument("PopulateConc: bad conc.txt file!!!");
  }
  double conc = 0;
  while(fs >> conc){result.push_back(conc);}
  if (static_cast<int>(result.size()) != numb_of_calibfiles){
    throw std::runtime_error("PopulateConc: wrong number of concentrations!!!");
  }
  return result;
}

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

int main(){
  try {

    [[maybe_unused]] static const std::vector<double> concentrations =
      PopulateConc("/home/matthew/gad_work/LastChanceCal/conc.txt");
    
    for (const auto& led_info : led_infos){

      // PURE COMPARE
      
      [[maybe_unused]] const TGraph calib_pure = Normalise(GetDarkSubtractFromFile(calib_str+"0.root",
										   led_info.name,
										   led_info.dark_offset));
      [[maybe_unused]] const TGraph run_pure = Normalise(GetDarkSubtractFromFile(pure_dataset_str+"00000.root",
										 led_info.name,
										 led_info.dark_offset));

      calib_pure.SaveAs(("norm_pure_1_"+led_info.name+".root").c_str());
      run_pure.SaveAs(("norm_pure_2_"+led_info.name+".root").c_str());

      // END OF PURE COMPARE
      
      TGraph* conc_pred_against_conc_true = new TGraph(); // TMultiGraph will handle deletion.
      conc_pred_against_conc_true->SetTitle((led_info.name+";true concentration; conc % prediction").c_str());
      conc_pred_against_conc_true->SetName("Graph");

      TGraph* metric_pred_against_conc_true = new TGraph(); // TMultiGraph will handle deletion.
      metric_pred_against_conc_true->SetTitle((led_info.name+";true concentration; conc % prediction").c_str());
      metric_pred_against_conc_true->SetName("Graph");
      
      TGraph* conc_pred_over_time = new TGraph(); // TMultiGraph will handle deletion.
      conc_pred_over_time->SetTitle((led_info.name+";fix this; conc % prediction").c_str());
      conc_pred_over_time->SetName("Graph");

      TGraph* metric_pred_over_time = new TGraph(); // TMultiGraph will handle deletion.
      metric_pred_over_time->SetTitle((led_info.name+";fix this; metric % prediction").c_str());
      metric_pred_over_time->SetName("Graph");

      TGraph* chi2_over_time = new TGraph(); // TMultiGraph will handle deletion.
      chi2_over_time->SetTitle((led_info.name+";fix this; chi2").c_str());
      chi2_over_time->SetName("Graph");

      TGraph first_pure_ds = GetDarkSubtractFromFile(calib_str+"0.root", led_info.name, led_info.dark_offset);
      first_pure_ds.SaveAs(("first_pure"+led_info.name+".root").c_str());
      TGraph high_conc_ds = GetDarkSubtractFromFile(calib_str+"50.root", led_info.name, led_info.dark_offset);
      TGraph high_conc_ds_for_abs_fit = GetDarkSubtractFromFile(calib_str+"50.root", led_info.name, led_info.dark_offset);
      TGraph high_conc_ds_rr = RemoveRegion(high_conc_ds, abs_region_low, abs_region_high);
      high_conc_ds.SaveAs(("highconc"+led_info.name+".root").c_str());

      std::unique_ptr<SimplePureFunc> simple_pure_func = std::make_unique<SimplePureFunc>(first_pure_ds);
      FunctionalFit simple_fit = FunctionalFit(simple_pure_func.get(), "simple");
      simple_fit.SetExampleGraph(first_pure_ds);
      simple_fit.PerformFitOnData(high_conc_ds_rr);
      simple_fit.GetGraph().SaveAs(("firstfit"+led_info.name+".root").c_str());
      high_conc_ds_for_abs_fit.SaveAs(("highconcfors"+led_info.name+".root").c_str());
      
      const TGraph orig_ratio_absorbance = PWRatio(simple_fit.GetGraph(), high_conc_ds_for_abs_fit);
      const TGraph gd_abs_try = CalculateGdAbs(simple_fit.GetGraph(), high_conc_ds_for_abs_fit);
      gd_abs_try.SaveAs(("gd_abs_try"+led_info.name+".root").c_str());
      const TGraph rat_abs_trimmed = TrimGraph(orig_ratio_absorbance); 
      orig_ratio_absorbance.SaveAs(("ratioabs"+led_info.name+".root").c_str());
      std::unique_ptr<AbsFunc> abs_func = std::make_unique<AbsFunc>(orig_ratio_absorbance);
      FunctionalFit abs_fit = FunctionalFit(abs_func.get(), "abs");
      abs_fit.SetExampleGraph(first_pure_ds);

      const TGraph spec_absorbance = PWDifference(simple_fit.GetGraph(), high_conc_ds);
      spec_absorbance.SaveAs(("specabs"+led_info.name+".root").c_str());
      //      std::unique_ptr<CombinedGdPureFunc> combined_func = std::make_unique<CombinedGdPureFunc>(first_pure_ds, spec_absorbance);
      //      std::unique_ptr<CombinedGdPureFunc_DATA> combined_func = std::make_unique<CombinedGdPureFunc_DATA>(first_pure_ds, orig_ratio_absorbance);
      std::unique_ptr<CombinedGdPureFunc_DATA> combined_func = std::make_unique<CombinedGdPureFunc_DATA>(first_pure_ds,
													 gd_abs_try);
      FunctionalFit combined_fit = FunctionalFit(combined_func.get(), "combined");
      
      combined_fit.SetExampleGraph(first_pure_ds);
      
      // if (led_info.name == "275_A"){
      // 	combined_fit.SetFitParameters(led_info.initial_combined_fit_param_values);
      // 	combined_fit.SetFitParameterRanges(led_info.fit_param_ranges);
      // }

      TFile* calib_curve_file = TFile::Open((led_info.calib_fname).c_str(), "OPEN");
      if (!calib_curve_file){
	throw std::invalid_argument("main(): CALIB CURVE FILE DOES NOT EXIST!!!\n");
      }
      TF1* calib_curve = static_cast<TF1*>(calib_curve_file->Get((led_info.calib_curve_name).c_str()));
      if (calib_curve == nullptr){
	throw std::runtime_error("main(): FAILED TO RETRIEVE CALIB CURVE!!!\n");
      }
      
      for (int file_idx = 0; file_idx < numb_of_data_files; ++file_idx){
      //for (int file_idx = 1510; file_idx < 1520; ++file_idx){
      //for (int file_idx = 1; file_idx < numb_of_calibfiles -1; ++file_idx){

	std::string file_idx_str = std::to_string(file_idx);
	const std::string file_number_str = file_idx_str.insert(0, 5 - file_idx_str.size(), '0');

	TGraph meas_ds = GetDarkSubtractFromFile(dataset_str+file_number_str+".root", led_info.name, led_info.dark_offset);
	//TGraph meas_ds = GetDarkSubtractFromFile(calib_str+file_idx_str+".root", led_info.name, led_info.dark_offset);
	bool interactive = false;//static_cast<bool>(file_idx == 13 && led_info.name == "275_B");
	combined_fit.PerformFitOnData(meas_ds, interactive);

	//	const TGraph ratio_abs_untrimmed = PWRatio(combined_fit.GetGraphExcluding({combined_func->ABS_SCALING, combined_func->ABS_FIRST_ORDER_CORRECTION}), meas_ds);
	const TGraph ratio_abs_untrimmed = PWRatio(combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}), meas_ds);
	const TGraph ratio_absorbance = TrimGraph(ratio_abs_untrimmed, abs_region_low, abs_region_high);

	abs_fit.PerformFitOnData(ratio_absorbance);
	[[maybe_unused]] const double metric = abs_fit.GetParameterValue(abs_func->ABS_SCALING);
	[[maybe_unused]] const double conc_prediction = calib_curve->GetX(metric);

	if (file_idx == 20){
       	  combined_fit.GetGraph().SaveAs(("afit"+led_info.name+".root").c_str());
	  //PWMultiply(combined_fit.GetGraph(), orig_ratio_absorbance).SaveAs(("bfit"+led_info.name+".root").c_str());
	  combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}).SaveAs(("bfit"+led_info.name+".root").c_str());
	  PWDifference(combined_fit.GetGraph(), meas_ds).SaveAs(("residual_"+led_info.name+".root").c_str());
	  meas_ds.SaveAs(("cfit"+led_info.name+".root").c_str());
	  abs_fit.GetGraph().SaveAs((led_info.name+"absfit.root").c_str());
	  ratio_absorbance.SaveAs((led_info.name+"absdata.root").c_str());
	  
	}

	// conc_pred_against_conc_true->AddPoint(concentrations.at(file_idx), conc_prediction);
	// metric_pred_against_conc_true->AddPoint(concentrations.at(file_idx), metric);
	
	conc_pred_over_time->AddPoint(file_idx, conc_prediction);
	metric_pred_over_time->AddPoint(file_idx, metric);
	chi2_over_time->AddPoint(file_idx, combined_fit.GetChiSquared());

	/* LOOKIN PLOTS */
	
	TGraph residual = PWPercentageDiff(combined_fit.GetGraph(), meas_ds);
	TGraph abs_residual = PWPercentageDiff(TrimGraph(abs_fit.GetGraph(), abs_region_low, abs_region_high), ratio_absorbance);
	
	meas_ds.SaveAs(("./lookin_plots/meas_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
        combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}).SaveAs(("./lookin_plots/cf_pure_comp_"+std::to_string(file_idx)+"_"\
									     +led_info.name+".root").c_str());
        combined_fit.GetGraph().SaveAs(("./lookin_plots/combfit_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
        residual.SaveAs(("./lookin_plots/res_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());

        ratio_absorbance.SaveAs(("./lookin_plots/abs_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
        TrimGraph(abs_fit.GetGraph(),
                  abs_region_low,
                  abs_region_high).SaveAs(("./lookin_plots/absfit_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
        abs_residual.SaveAs(("./lookin_plots/absres_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	
	/* END OF LOOKIN PLOTS */
	
      }
      conc_pred_against_conc_true->SaveAs((led_info.name+"_pred_true.root").c_str());
      delete conc_pred_against_conc_true; conc_pred_against_conc_true = nullptr;
      metric_pred_against_conc_true->SaveAs((led_info.name+"metric_pred_true.root").c_str());
      delete metric_pred_against_conc_true; metric_pred_against_conc_true = nullptr;
      conc_pred_over_time->SaveAs((led_info.name+"_data.root").c_str());
      delete conc_pred_over_time; conc_pred_over_time = nullptr;
      metric_pred_over_time->SaveAs((led_info.name+"_metric.root").c_str());
      delete metric_pred_over_time; metric_pred_over_time = nullptr;
      chi2_over_time->SaveAs((led_info.name+"_chi2.root").c_str());
      delete chi2_over_time; chi2_over_time = nullptr;

    }
  }
  catch (const  std::exception& e){
    std::cout << e.what() << "\n";
    return 1;
  }
  return 0;
}
