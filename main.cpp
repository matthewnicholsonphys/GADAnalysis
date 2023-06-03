#include "gad_utils.h"
#include <memory>
#include <fstream>

#include "TTree.h"
#include "TGraph.h"
#include "TFile.h"

[[maybe_unused]] static int numb_of_pure_files = 406;
[[maybe_unused]] static constexpr int numb_of_gd_files  = 54; //120;

//static std::string pure_dataset_str = "/home/matthew/gad_work/gad_utils/pure_data1/00182_24MarPureStability_";
//static std::string gd_dataset_str = "/home/matthew/gad_work/gd_data1/00150_18Mar2023Calibration_";
//static std::string gd_dataset_str = "/home/matthew/gad_work/LastChanceCalFull/00184_17Apr_EGADS_Calibration_";
static std::string calib_str = "/home/matthew/gad_work/LastChanceCal/run_00184_17_April_2023_calibfile_";
static std::string dataset_str = "/home/matthew/gad_work/gad_data_april2023/00185_25_Apr_2023_EGADS_Run_";

static std::map<std::string, int> led_names_w_dark_offset = {{"275_A", 10}, {"275_B", 9}};
//                                                             {"R_G_B", 11}, {"White_385", 12}};

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

std::vector<double> PopulateConc(const std::string& fname){
  std::vector<double> result;
  std::fstream fs(fname);
  if (!fs){
    throw std::invalid_argument("PopulateConc: bad conc.txt file!!!");
  }
  double conc = 0;
  while(fs >> conc){result.push_back(conc);}
  if (result.size() != numb_of_gd_files){
    throw std::runtime_error("PopulateConc: wrong number of concentrations!!!");
  }
  return result;
}

int main(){
  try {

    [[maybe_unused]] static std::vector<double> concentrations =
      PopulateConc("/home/matthew/gad_work/LastChanceCal/conc.txt");

    // TMultiGraph peak_diff("Graph", "CombinedFit Peak Difference;Measurement Number, aka Time [~15mins]; Peak Difference");
    // TMultiGraph peak_ratio("Graph", "CombinedFit Peak Ratio;Measurement Number, aka Time [~15mins]; Peak Ratio");
    TMultiGraph abs_scaling("Graph", "CombinedFit:ABSFIT Scaling Param ;Measurement Number, aka Time [~15mins]; abs scaling factor");
    TMultiGraph chi_comb("Graph", "CombinedFit #chi^{2};Measurement Number, aka Time [~15mins]; #chi^{2}");
    TMultiGraph chi_abs("Graph", "AbsFit #chi^{2};Measurement Number, aka Time [~15mins]; #chi^{2}");

    TMultiGraph abs_scaling_conc("Graph",
				 "CombinedFit:ABSFIT Scaling Param ;Gd concentration; abs scaling factor");
    TMultiGraph chi_comb_conc("Graph", "CombinedFit #chi^{2};Gd concentration; #chi^{2}");
    TMultiGraph chi_abs_conc("Graph", "AbsFit #chi^{2};Gd concentration; #chi^{2}");
    
    //for (const auto& [led_name, led_info.dark_offset] : led_names_w_led_info.dark_offset){
    for (const auto& led_info : led_infos){
    
      TMultiGraph residuals("Graph", (led_info.name+";wavelength [nm]; % residual").c_str());
      TMultiGraph abs_residuals("Graph", (led_info.name+";wavelength [nm]; % residual").c_str());
      
      TGraph* comb_chi2_over_time = new TGraph(); // TMultiGraph handles deletion.
      comb_chi2_over_time->SetTitle((led_info.name+";measurement number [~15mins];#chi^2").c_str());

      TGraph* abs_chi2_over_time = new TGraph(); // TMultiGraph handles deletion.
      abs_chi2_over_time->SetTitle((led_info.name+";measurement number [~15mins];#chi^2").c_str());
      
      TGraph* abs_scaling_over_time = new TGraph(); // TMultiGraph handles deletion.
      abs_scaling_over_time->SetTitle((led_info.name+";measurement number [~15mins];peak ratio").c_str());

      TGraph* comb_chi2_over_conc = new TGraph(); // TMultiGraph handles deletion.
      comb_chi2_over_conc->SetTitle((led_info.name+";Gd concentration;#chi^2").c_str());

      TGraph* abs_chi2_over_conc = new TGraph(); // TMultiGraph handles deletion.
      abs_chi2_over_conc->SetTitle((led_info.name+";Gd concentration;#chi^2").c_str());
      
      TGraph* abs_scaling_over_conc = new TGraph(); // TMultiGraph handles deletion.
      abs_scaling_over_conc->SetTitle((led_info.name+";Gd concentration;peak ratio").c_str());
      
      // TGraph* peak_diff_over_time = new TGraph(); // TMultiGraph handles deletion.
      // peak_diff_over_time->SetTitle((led_info.name+";measurement number [~15mins];peak diff").c_str());

      // TGraph* peak_ratio_over_time = new TGraph(); // TMultiGraph handles deletion.
      // peak_ratio_over_time->SetTitle((led_info.name+";measurement number [~15mins];peak ratio").c_str());

      TGraph* np_abs_scaling_over_time = new TGraph(); // TMultiGraph handles deletion.
      np_abs_scaling_over_time->SetTitle((led_info.name+"-NewPure;measurement number [~15mins];peak ratio").c_str());

      TGraph* np_comb_chi2_over_time = new TGraph(); // TMultiGraph handles deletion.
      np_comb_chi2_over_time->SetTitle((led_info.name+"-NewPure;measurement number [~15mins];#chi^2").c_str());

      TGraph* np_abs_chi2_over_time = new TGraph(); // TMultiGraph handles deletion.
      np_abs_chi2_over_time->SetTitle((led_info.name+"-NewPure;measurement number [~15mins];#chi^2").c_str());
      
      TGraph first_pure_ds = GetDarkSubtractFromFile(calib_str + "0.root", led_info.name, led_info.dark_offset);
      first_pure_ds.SaveAs("firstpure.root");
      TGraph high_conc_ds = GetDarkSubtractFromFile(calib_str + "50.root", led_info.name, led_info.dark_offset);
      high_conc_ds.SaveAs("highconc.root");
      TGraph high_conc_ds_rr = RemoveRegion(high_conc_ds, abs_region_low, abs_region_high);
      high_conc_ds_rr.SaveAs("highconcrr.root");
      std::unique_ptr<SimplePureFunc> simple_pure_func = std::make_unique<SimplePureFunc>(first_pure_ds);
      FunctionalFit simple_fit = FunctionalFit(simple_pure_func.get(), "simple");
      simple_fit.SetExampleGraph(first_pure_ds);
      simple_fit.PerformFitOnData(high_conc_ds_rr);
      simple_fit.GetGraph().SaveAs("firstfit.root");

      TGraph high_conc_ds_for_abs_fit = GetDarkSubtractFromFile(calib_str+"50.root", led_info.name, led_info.dark_offset);
      const TGraph gd_abs_try = CalculateGdAbs(simple_fit.GetGraph(), high_conc_ds_for_abs_fit);    

      const TGraph spec_absorbance = PWDifference(simple_fit.GetGraph(), high_conc_ds);
      
      spec_absorbance.SaveAs("spec_absorbance.root");
      //std::unique_ptr<CombinedGdPureFunc> combined_func = std::make_unique<CombinedGdPureFunc>(first_pure_ds, spec_absorbance);
      std::unique_ptr<CombinedGdPureFunc_DATA> combined_func = std::make_unique<CombinedGdPureFunc_DATA>(first_pure_ds,
													  gd_abs_try);
      FunctionalFit combined_fit = FunctionalFit(combined_func.get(), "combined");
      combined_fit.SetExampleGraph(first_pure_ds);

      if (led_info.name == "275_A"){
        combined_fit.SetFitParameters(led_info.initial_combined_fit_param_values);
        combined_fit.SetFitParameterRanges(led_info.fit_param_ranges);
      }
      
      // std::unique_ptr<CombinedGdPureFunc> new_combined_func = std::make_unique<CombinedGdPureFunc>(first_pure_ds, spec_absorbance);
      // FunctionalFit new_combined_fit = FunctionalFit(new_combined_func.get(), "newcombined");
      // new_combined_fit.SetExampleGraph(first_pure_ds);

      const TGraph orig_ratio_absorbance = PWRatio(simple_fit.GetGraph(), high_conc_ds);
      orig_ratio_absorbance.SaveAs("ratio_absorbance.root");
      std::unique_ptr<AbsFunc> abs_func = std::make_unique<AbsFunc>(orig_ratio_absorbance);
      FunctionalFit abs_fit = FunctionalFit(abs_func.get(), "abs");
      abs_fit.SetExampleGraph(first_pure_ds);

      [[maybe_unused]] double previous_scaling_result = 0;
      [[maybe_unused]] double previous_comb_chi2 = 0;
      [[maybe_unused]] TGraph previous_ratabs = TGraph(number_of_points);
      [[maybe_unused]] TGraph previous_purefit = TGraph(number_of_points);

      [[maybe_unused]] const double scaling_trigger_threshold = 0.03;
      [[maybe_unused]] const double comb_chi2_threshold = 20000;

      /* -----------CALIB DATA ANALYSIS -------------- */
      
      for (int file_idx = 1; file_idx < numb_of_gd_files; ++file_idx){
	
	std::string file_index_str = std::to_string(file_idx);
	const std::string file_number_str = file_index_str;
	//const std::string file_number_str = file_index_str.insert(0, 5 - file_index_str.size(), '0');
	TGraph meas_ds = GetDarkSubtractFromFile(calib_str + file_number_str + ".root", led_info.name, led_info.dark_offset);   
	combined_fit.PerformFitOnData(meas_ds);
            
	TGraph residual_temp = PWPercentageDiff(combined_fit.GetGraph(), meas_ds);
	TGraph* residual = new TGraph(TrimGraph(residual_temp));
	residual->SetTitle((led_info.name+";wavelength [nm];residual").c_str());
	residuals.Add(residual);

	const TGraph rat_abs_untrimmed = PWRatio(combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}), meas_ds);
	const TGraph ratio_absorbance = TrimGraph(rat_abs_untrimmed, abs_region_low, abs_region_high);
	// peak_diff_over_time->AddPoint(file_idx, GetPeakDiff(ratio_absorbance));
	// peak_ratio_over_time->AddPoint(file_idx, GetPeakRatio(ratio_absorbance));
	
	abs_fit.PerformFitOnData(ratio_absorbance);
	abs_scaling_over_time->AddPoint(file_idx, abs_fit.GetParameterValue(abs_func->ABS_SCALING));

	TGraph abs_residual_temp = PWPercentageDiff(TrimGraph(abs_fit.GetGraph(), abs_region_low, abs_region_high), ratio_absorbance);
	TGraph* abs_residual = new TGraph(TrimGraph(abs_residual_temp));
	abs_residuals.Add(abs_residual);

	comb_chi2_over_time->AddPoint(file_idx, combined_fit.GetChiSquared());
	abs_chi2_over_time->AddPoint(file_idx, abs_fit.GetChiSquared());

	abs_scaling_over_conc->AddPoint(concentrations.at(file_idx), abs_fit.GetParameterValue(abs_func->ABS_SCALING));
	comb_chi2_over_conc->AddPoint(concentrations.at(file_idx), combined_fit.GetChiSquared());
	abs_chi2_over_conc->AddPoint(concentrations.at(file_idx), abs_fit.GetChiSquared());

	meas_ds.SaveAs(("./lookin_plots/meas_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}).SaveAs(("./lookin_plots/cf_pure_comp_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	combined_fit.GetGraph().SaveAs(("./lookin_plots/combfit_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	residual->SaveAs(("./lookin_plots/res_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());

	ratio_absorbance.SaveAs(("./lookin_plots/abs_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	TrimGraph(abs_fit.GetGraph(),
		  abs_region_low,
		  abs_region_high).SaveAs(("./lookin_plots/absfit_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());
	abs_residual->SaveAs(("./lookin_plots/absres_"+std::to_string(file_idx)+"_"+led_info.name+".root").c_str());

	// ------------------------ NewPure --------------------------//	
	
	// if (file_idx > 23 /*&& file_idx < 80*/) {
      
	//   if ((abs(abs_fit.GetParameterValue(abs_func->ABS_SCALING) - previous_scaling_result) > scaling_trigger_threshold) && 
	//       (abs(combined_fit.GetChiSquared() - previous_comb_chi2) > comb_chi2_threshold) &&
	//       (previous_scaling_result != 0) &&
	//       (previous_comb_chi2 != 0)){
	    
	//     std::cout << "THRESHOLD TRIGGER AT FILE_IDX: " << file_idx << "\n";

	//     const TGraph new_pure_ds = PWMultiply(previous_ratabs, meas_ds);
	//     [[maybe_unused]] const TGraph new_spec_absorbance = PWDifference(previous_purefit, meas_ds);	    
	//     new_combined_func = std::make_unique<CombinedGdPureFunc>(new_pure_ds, spec_absorbance);
	//     new_combined_fit = FunctionalFit(new_combined_func.get(), "newcombined");
	//     new_combined_fit.SetExampleGraph(new_pure_ds);
	    
	//   }
	  
	//   previous_scaling_result = abs_fit.GetParameterValue(abs_func->ABS_SCALING);
	//   previous_comb_chi2 = combined_fit.GetChiSquared();
	//   previous_ratabs = rat_abs_untrimmed;
	//   previous_purefit = combined_fit.GetGraphExcluding({combined_func->ABS_SCALING});
	  
	// }

	// new_combined_fit.PerformFitOnData(meas_ds);
	// const TGraph new_rat_abs_untrimmed = PWRatio(new_combined_fit.GetGraphExcluding({combined_func->ABS_SCALING}), meas_ds);
	// const TGraph new_ratio_absorbance = TrimGraph(new_rat_abs_untrimmed, abs_region_low, abs_region_high);
	// abs_fit.PerformFitOnData(new_ratio_absorbance);
	// np_abs_scaling_over_time->AddPoint(file_idx, abs_fit.GetParameterValue(abs_func->ABS_SCALING));

	// np_comb_chi2_over_time->AddPoint(file_idx, new_combined_fit.GetChiSquared());
	// np_abs_chi2_over_time->AddPoint(file_idx, abs_fit.GetChiSquared());
	
      }

      // peak_diff.Add(peak_diff_over_time);
      // peak_ratio.Add(peak_ratio_over_time);

      abs_scaling.Add(abs_scaling_over_time);
      chi_comb.Add(comb_chi2_over_time);
      chi_abs.Add(abs_chi2_over_time);

      abs_scaling_conc.Add(abs_scaling_over_conc);
      chi_comb_conc.Add(comb_chi2_over_conc);
      chi_abs_conc.Add(abs_chi2_over_conc);

      // abs_scaling.Add(np_abs_scaling_over_time);
      // chi_comb.Add(np_comb_chi2_over_time);
      // chi_abs.Add(np_abs_chi2_over_time);
      residuals.SaveAs(("res"+led_info.name+".root").c_str());
      abs_residuals.SaveAs(("absres"+led_info.name+".root").c_str());
      
      //pure_residuals.SaveAs(("pureres"+led_info.name+".root").c_str());
      
    }
    abs_scaling.SaveAs("absscalingpd.root");
    chi_abs.SaveAs("chiabs.root");
    chi_comb.SaveAs("chicomb.root");

    abs_scaling_conc.SaveAs("absscalingpd_conc.root");
    chi_abs_conc.SaveAs("chiabs_conc.root");
    chi_comb_conc.SaveAs("chicomb_conc.root");

    // peak_diff.SaveAs("peakdiff.root");
    // peak_ratio.SaveAs("peakratio.root");
  }
  catch (const std::exception& e){
    std::cout << "\n" << e.what() << "\n";
    return 1;
  }
  return 0;
}


