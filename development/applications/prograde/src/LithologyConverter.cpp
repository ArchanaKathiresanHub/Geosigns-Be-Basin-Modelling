//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "LithologyConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "StratigraphyManager.h"
using namespace mbapi;
#include<map>
#include <algorithm> 
#include <sstream> 

std::map<std::string, std::string> Prograde::LithologyConverter::lithologyNameMaps = createMapForLithoNames();

std::map<std::string, std::string> Prograde::LithologyConverter::createMapForLithoNames()
{
	std::map<std::string, std::string> m;
	m.insert(std::pair<std::string, std::string>("Diorite/Granodiorite", "Diorite/Granodiorite (SI)"));
	m.insert(std::pair<std::string, std::string>("Gabbro/Dry basalt", "Dry Gabbro/Basalt (SI)"));
	m.insert(std::pair<std::string, std::string>("Gabbro/Wet basalt", "Granite/Rhyolite (SI)"));
	m.insert(std::pair<std::string, std::string>("Granite/Rhyolite", "Wet Gabbro/Basalt (SI)"));
	m.insert(std::pair<std::string, std::string>("HEAT Chalk", "Chalk, white"));
	m.insert(std::pair<std::string, std::string>("Std. Chalk", "Chalk, white"));
	m.insert(std::pair<std::string, std::string>("HEAT Dolostone", "Grainstone, dolomitic, typical"));
	m.insert(std::pair<std::string, std::string>("HEAT Limestone", "Grainstone, calcitic, typical"));
	m.insert(std::pair<std::string, std::string>("HEAT Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("Std. Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("SM. Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.40%Clay", "Mudstone, 40% clay"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.50%Clay", "Mudstone, 50% clay"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.60%Clay", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("Std. Shale", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("HEAT Shale", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("Standard Ice", "Ice"));
	m.insert(std::pair<std::string, std::string>("Std. Anhydrite", "Anhydrite"));
	m.insert(std::pair<std::string, std::string>("Std. Basalt", "Basalt, typical"));
	m.insert(std::pair<std::string, std::string>("Std. Coal", "Coal"));
	m.insert(std::pair<std::string, std::string>("Std. Marl", "Marl"));
	m.insert(std::pair<std::string, std::string>("Std. Salt", "Halite"));
	m.insert(std::pair<std::string, std::string>("Std. Siltstone", "Siltstone, 20% clay"));
	m.insert(std::pair<std::string, std::string>("Std.Dolo.Grainstone", "Grainstone, dolomitic, typical"));
	m.insert(std::pair<std::string, std::string>("Std.Dolo.Mudstone", "Lime-Mudstone, dolomitic"));
	m.insert(std::pair<std::string, std::string>("Std.Grainstone", "Grainstone, calcitic, typical"));
	m.insert(std::pair<std::string, std::string>("Std.Lime Mudstone", "Lime-Mudstone"));
	m.insert(std::pair<std::string, std::string>("Sylvite", "Potash-Sylvite"));
	m.insert(std::pair<std::string, std::string>("Astheno. Mantle", "Mantle"));
	m.insert(std::pair<std::string, std::string>("Litho. Mantle", "Mantle"));

	return m;
}
std::map<std::string, std::string> Prograde::LithologyConverter::mappingOfLithologyNameBasedOndescription = createMapForUnparentedLithoNames();

std::map<std::string, std::string> Prograde::LithologyConverter::createMapForUnparentedLithoNames()
{
	std::map<std::string, std::string> map;
	////////////////////////////////////////////////////////////////////////////////////////// Mapping for scenarios in AMS database //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Map created for Exponential porosity model based unparented legacy lithologies => std::pair::(<Decription>, <legacy parent litholgy name>)
	map.insert(std::pair<std::string, std::string>("Al Bashair Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Allow minor porosity and permeability near surface.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Chalk as used by IBS-HEAT", "HEAT Chalk"));
	map.insert(std::pair<std::string, std::string>("Cold Shale_1.0", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("DeepCompactedSand", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("DeepSh_LP20%", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("DeepShaleLowPerm", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("hot_shale to introduce a great amount of heat from the surface into the model by introducitn a hot Radiogenic heat genration", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Klaus_Sand", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Klaus_Sand2", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Lower Depositional Perm (value of std shale not std siltstone).\\n\\nLower thermal conductivity (using geometric not arithmetic mean).\\n\\nModified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Lower thermal conductivity (using geometric not arithmetic mean).\\n\\nModified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("lowporositywadsand", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("LSB_Vlieland", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mod. Basalt", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("NewAnhydrite", "Std. Anhydrite"));
	map.insert(std::pair<std::string, std::string>("Nile Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Qatar Anhydrite", "Std. Anhydrite"));
	map.insert(std::pair<std::string, std::string>("Qatar Dolo Lime", "Std.Dolo.Mudstone"));
	map.insert(std::pair<std::string, std::string>("Qatar Dolomite", "Std.Dolo.Mudstone"));
	map.insert(std::pair<std::string, std::string>("Qatar Limestone", "HEAT Limestone"));
	map.insert(std::pair<std::string, std::string>("Qatar Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale as used by IBS-HEAT", "HEAT Shale"));
	map.insert(std::pair<std::string, std::string>("silica ooze", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 40% Clay", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 50% Clay", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Standard Basalt", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Standard Calcareous Mudstone (Limestone)", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Standard Chalk", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Standard Coal", "Std. Coal"));
	map.insert(std::pair<std::string, std::string>("Standard Domlomitic Grainstone (Dolostone)", "Std.Dolo.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Standard Grainstone (Limestone)", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Standard Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("Standard Salt", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Shale\\nFor tests constructed by Philipp in WSB", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Siltstone", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("WAD_chalk", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("WAD_sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("WAD_shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); ", "Std. Basalt"));//////
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSeisVel=6500 instead of 5750", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nHighest thermal conductivity", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nLowest thermal conductivity", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%);\\nAssign density 2830 for consistency with CTC basalt (combines true basalt and mantle depletion uplift effects).", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(80%);\\n\\nEst 12%clay equivalent\\nEst 6.46 comp coeff\\nEst Surf Phi 0.656\\nExcel estimates\\nVP solid 5670\\nRho solid 2652\\nThC Geom Mean 1.72", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(80%);\\n\\nUse for Orange SPOB12-09 test\\nEdited k = weighted harm mean\\ncomp coeff from spreadsheet", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(40%); Project Lithology Std. Siltstone(60%); ", "Std. Siltstone"));//////
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 15%clay equivalent\\nEst 4.2325 comp coeff\\nEst Surf Phi 0.535\\nRaise to 80% due to Wyllie prob\\nExcel estimates\\nVP solid 5575\\nRho solid 2652\\nThC Geom Mean 3.02", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 15%clay equivalent\\nEst 4.2325 comp coeff\\nEst Surf Phi 0.535\\nRaise to 85% due to Wyllie prob\\nExcel estimates\\nVP solid 5575\\nRho solid 2652\\nThC Geom Mean 3.02", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 30%clay equivalent\\nEst 5.245 comp coeff\\nEst Surf Phi 0.59\\nChanged to 0.65% due to Wyllie problem.\\nExcel estimates\\nVP solid 5150\\nRho solid 2654\\nThC Geom Mean 2.34", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(25%); Project Lithology Std. Sandstone(75%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xls", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Modified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx", "Std. Shale"));

	//Part2 mapping....lithologies are based on new BPA2 lithotypes
	map.insert(std::pair<std::string, std::string>("Based on New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\nAllow minor porosity and permeability near surface.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Low porosity like new BPA2 basalt\\n\\nMix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nHighest thermal conductivity", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("See \\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\nFor use in upper part of oceanic crust extrusive, rubble sediment layers.", "Basalt, oceanic crust"));
	map.insert(std::pair<std::string, std::string>("See \\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\nUse in most cases for rift filling basalt.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("See \\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\nUse in SDRs if known very high density in logs (eg Lopra, Faroes).", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("See \\n\\nBPA2_Standard_lithologies_for_MvdM_OM_PB_prograde_LK01102019v2.xlsx", "Basalt, SDR extrusive flows"));

	//Map created for Soil Mecahnics porosity model based unparented legacy lithologies =>
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Sandstone", "SM. Sandstone"));
	map.insert(std::pair<std::string, std::string>("SM Mudstone 20% Clay", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("SGP_Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("SM Mudstone 80% Clay", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 60% Clay", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("SM Siltstone", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Standard Coal", "Std. Coal"));
	map.insert(std::pair<std::string, std::string>("StaSm Grainstone", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Limestone as used by IBS-HEAT", "HEAT Limestone"));
	map.insert(std::pair<std::string, std::string>("New Salt SM GVB", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("StanSM Dolo Grainsto", "Std.Dolo.Grainstone"));

	////////////////////////////////////////////////////////////////////////////////////////// Mapping for scenarios in HOU database //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Map created for Exponential porosity model based unparented legacy lithologies => std::pair::(<Decription>, <legacy parent lithology name>)
	///mapped to BPA-legacy lithology names =>
	map.insert(std::pair<std::string, std::string>("See BPA2_Standard_lithologies_for_MvdM_OM_PB_prograde_LK28052019.xlsx", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("See \\n\\n\\n\\nBPA2_Standard_lithologies_for_MvdM_OM_PB_prograde_LK01102019v2.xlsx", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("See \\n\\n\\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\n\\n\\nUse in most cases for rift filling basalt.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(50%); Project Lithology Std. Siltstone(50%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(47%); Project Lithology Std. Siltstone(53%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(35%); Project Lithology Std. Siltstone(65%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(78%); Project Lithology Std. Siltstone(22%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(80%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(20%); Project Lithology Std. Siltstone(80%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Less Low TC Shale(90%); Project Lithology Std. Sandstone(10%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Anhydrite(50%); Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(30%); ", "Std. Anhydrite"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(30%); Project Lithology Std.Lime Mudstone(70%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(25%); Project Lithology Std. Shale(75%); \\n\\nExcel settings 2650/0.6/5.5/4750/0\\nHighly non-unique. May not fit deeper section well if carbonates more abundant. Based on crude T, Z pairs from Redden, Teasdale. No well logs available? Suitable for hydrostatic runs only.", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Mod 2 Std Candidate Extrusive Basalt SDR Hybrid LK21032017(40%); Project Lithology New Std Candidate Shale Hybrid LK26052016(60%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(1%); Project Lithology Std. Sandstone(99%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(99%); Project Lithology Std. Siltstone(1%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology ELEPA-5-SPDC-TIGHT-SHALE3(50%)", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 12 Sandstone, typical, BPA2(80%); Project Lithology 15 Siltstone, BPA2(15%); Project Lithology 16 Mudstone, 30% clay, BPA2(5%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(2%); Project Lithology Std. Shale(98%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(5%); Project Lithology Std. Shale(95%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("My Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("high thermal conductive Sandstone to reflect cool basins with high sedimentation rates", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("My Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Continental_Sst", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Dolo.Mudstone(50%); Project Lithology Std. Chalk(25%); Project Lithology Std. Sandstone(25%); ", "Std.Dolo.Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(34%); Project Lithology Std. Sandstone(33%); Project Lithology Std. Shale(33%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 12_Sandstone_typical_BPA2_LK03072019(20%); Project Lithology 13 Grainstone Calcitic Well Cemented LK29052017 Slower4500(60%); Project Lithology 23_Mudstone_65CF_BPA2_LK03072019(20%); ", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Grainstone", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Lime Mudstone", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Lime Silt", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("Shale-Mudstone", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Standard Siltstone", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(70%); Project Lithology Std. Siltstone(10%); Project Lithology Std. Shale(20%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Delta-Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 15 Siltstone, BPA2(5%); Project Lithology 12 Sandstone, typical, BPA2(10%); Project Lithology 30%Clay_LowThermConduc(85%); ", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(75%); Project Lithology Std. Sandstone(25%);\\n\\nThis is equivalent to c. 50% clay if shale is taken as 65% clay. True clay may be less, reflected in custom coefficients.", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Hard_OP_Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Mild_OP_Shale(90%); Project Lithology ZARAMA-DEEP-1X-SHALE-OVP(10%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Paralic Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology My Shale1(10%); Project Lithology Eja-3-Shale(90%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("provides good fit with rock property database", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mild_OP_Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology ELEPA-5-SPDC-TIGHT-SHALE3(90%); Project Lithology ELEPA-5-SPDC-TIGHT-Shale2(10%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>(">60% Clay, developed from predicted shale prosities calculated from GR at Etan-002X, developed by J.Frielingsdorf\\nhigher thermal conductivity", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>(">60% Clay, developed from predicted shale prosities calculated from GR at Etan-002X, developed by J.Frielingsdorf\\n\\nhigher thermal conductivity\\n\\nChanged Cappilary pressure C1 0.3  C2 0.62\\n\\nchanged Density and Seis.Velocity", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("70_SHL__30_SS", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Trial to desribe Sst endmember in French Guiana.\\nAjustment of Density and seismic velocity to fit log data", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Permeabilities adjsuted to Poro/Perm relationship from Juan Urregro\\nTrial to desribe Sst endmember in French Guiana.\\nAjustment of Density and seismic velocity to fit log data", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("trial to describe shales in French Guiana.\\nprovides good fit with rock property database", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("changes to the capillary entry pressure parameter and lower surface porosity\\ntrial to describe shales in French Guiana.\\nprovides good fit with rock property database", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Dolo.Mudstone(50%); Project Lithology Std.Lime Mudstone(50%); ", "Std.Dolo.Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(50%); Project Lithology Std.Dolo.Mudstone(50%); ", "Std.Dolo.Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(80%); Project Lithology Std.Dolo.Grainstone(20%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("60_SHL__30_LMST", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Calcareous Mudstone (Limestone)", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Modified Mudstone", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("Modified SS", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Modified Lime Mdst", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(60%); Project Lithology Std. Sandstone(40%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Silty Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Silty Shlae", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("GYshale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(5%); Project Lithology Std. Shale(95%); \\n\\ndeveloped by Herwig Ganz", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 60% Clay", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Limestone as used by IBS-HEAT", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model\\n\\nNon-zero porosity, high CC to align with UMCL crust lithology\\n\\nCrank up Cond to increase thermal subsidence", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Calcilutite(68%); Project Lithology Calcarenite(28%); Project Lithology Std. Shale(4%); ", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Standard Crust", "Crust"));
	map.insert(std::pair<std::string, std::string>("Labrador sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Low radioactive Sh.", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(99%); Project Lithology Std. Shale(1%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Dolostone as used by IBS-HEAT", "Std.Dolo.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(70%); Project Lithology Std. Siltstone(20%); Project Lithology Std. Sandstone(10%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(30%); Project Lithology Std. Shale(70%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(50%); Project Lithology Std. Shale(30%); Project Lithology Std. Sandstone(20%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(70%); Project Lithology Std.Lime Mudstone(30%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(30%); Project Lithology Std. Shale(70%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(20%); Project Lithology Std. Shale(80%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); ", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Malampaya Conglom", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Malampaya Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Marly Limestone", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mez Lst", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mez sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mesozoic Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(40%); Project Lithology Std. Shale(60%); \\n\\n\\n\\ndeveloped by Herwig Ganz", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 40% Clay", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Chalk with SM Perm LK26012015(50%); Sandstone with SM Perm LK27112014(10%); Mod Perm Shale v4 LK10122014(40%); \\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Permeability Model to that of Chalk, Shale and Sand mix (separate calculation, geometric means), slightly more permeable than 40% Clay Mudstone.\\n\\nAlso lower Therm Conduct seems realistic and necessary. Changed 2.43 into self-calculated 1.9", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Chalk with SM Perm LK26012015(50%); Sandstone with SM Perm LK27112014(10%); Mod Perm Shale v4 LK10122014(40%); \\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Permeability Model to that of 50%Clay SM.Mudst (similar to Chalk).", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Chalk with SM Perm LK26012015(50%); Sandstone with SM Perm LK27112014(10%); Mod Perm Shale v4 LK10122014(40%); ", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Chalk with SM Perm LK26012015(50%); Sandstone with SM Perm LK27112014(10%); Mod Perm Shale v4 LK10122014(40%); \\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Permeability Model to that of Chalk, Shale and Sand mix (separate calculation, geometric means), slightly more permeable than 40% Clay Mudstone.\\nThis version with high Comp Coeff (7.27 instead of 6) for FC runs.\\n\\nAlso lower Therm Conduct seems realistic and necessary. Changed 2.43 into self-calculated 1.9", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Sandstone with SM Perm for Mixing EH08042015(66%) & Mod Perm Shale v4 LK10122014(34%);\\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Perm Model to with weighted arithmetic mean of used mixed lithologies.", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Sandstone with SM Perm for Mixing EH08042015(66%) & Mod Perm Shale v4 LK10122014(34%);\\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Permeability Model to that of Sand/Shale mix (separate calculation, geometric means), slightly more permeable than 40% Clay Mudstone.\\n\\nAlso lower Therm Conduct seems realistic and necessary. Changed 2.788 into self-calculated 1.9", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Sandstone with SM Perm for Mixing EH08042015(66%); Mod Perm Shale v4 LK10122014(34%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Sandstone with SM Perm for Mixing EH08042015(66%) & Mod Perm Shale v4 LK10122014(34%);\\nFurther adjusted Surface Porosity, Velocity, Density and Comp Coeff to make compaction trend compatible with well data.\\nAdjusted Multipoint Permeability Model to that of Sand/Shale mix (separate calculation, geometric means), slightly more permeable than 40% Clay Mudstone.\\nThis version with high Comp Coeff (7.27 instead of 6) for FC runs.\\n\\nAlso lower Therm Conduct seems realistic and necessary. Changed 2.788 into self-calculated 1.9", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Siltstone with SM Perm EH08042015(34%); Project Lithology Sandstone with SM Perm LK27112014(33%); Project Lithology Mod Perm Shale v4 LK10122014(33%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Lime80-Shale20", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Chukchi sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Beaufort Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Chukchi Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("My Shale1", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("My Shale2", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Chukchi SR Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale80-Lime20", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Silt60-Sand40", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Chukchi Siltstone", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mysand", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Myshale", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(90%); Project Lithology Std. Siltstone(10%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(60%); Project Lithology ND_Shale_Otuka(40%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(80%); Project Lithology Std. Siltstone(20%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("New Salt GVB", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("New_Salt", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("Sew_Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Modified Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("properties derived from adjsutment with rock property database.\\n\\nContinental Sst. is charcterised by high thermal conductivity", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("high thermal conductivity to account for shallow cool thermal gradient", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(95%); Project Lithology Std. Shale(5%); \\n\\nExtrem High thermal conductivity to mimic low shallow thermal gradients\\n\\n\\n\\nJ.Frielingsdorf", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("properties derived from adjsutment with rock property database.\\n\\n", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("derived by achieving fit to rock property database", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(40%); Project Lithology Std. Shale(60%); \\n\\n\\n\\nJurgen Frielingsdorf", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("derived by achievinf fit to rock property database", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(40%); Project Lithology Std. Shale(60%); \\n\\nJurgen Frielingsdorf", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("best fit to the Rock property databse.\\n\\nRigid Sandstone ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("rigid sand based on Rockproperty database adjustment", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Ajustment of Density and seismic velocity to fit log data", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("properties derived from adjsutment with rock property ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("JRF April2012", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("JRF, Benin Formation, with high thermal conductivity and no radiogenic heat production\\n\\nfit best Density logs, and Niger Delta Sand compaction trend, velocity has been reduces to fit Sonic velocity", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("to fit data", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Hard_OP_Shale(90%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Nile Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology NMR_LimeMudstone(50%); Project Lithology NMR_Shale(50%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model\\nincreased velocity", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Shale - with SM 30%clay permeability model", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model\\nVelocity exponent reduced from 0.45 to  0.25", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model\\n\\nVelocity exponent reduced from 0.45 to  0.25", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model\\nAssign min porosity 4%", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone - with SM permeability model\\n\\nAssign min porosity 4%", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Anhydrite(90%); Project Lithology Std. Shale(10%); ", "Std. Anhydrite"));
	map.insert(std::pair<std::string, std::string>("trial to describe shales Offshore Colombia\\nmultipoint poro perm relation\\nPoro/Perm relation is uncalibrated here\\nOutboard environment shale rich with Mud Volcanos", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("lowered anisotropy to 1\\napplied more vertical leak\\ntrial to describe shales Offshore Colombia\\nmultipoint poro perm relation\\nPoro/Perm relation is uncalibrated here\\nOutboard environment shale rich with Mud Volcanos - pro perm describes unclean shale\\nCap entry pressure reduced", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("medium leak (capillary entry pressure change)\\nanisotropy to 2\\napplied more vertical leak\\ntrial to describe shales Offshore Colombia\\nmultipoint poro perm relation\\nPoro/Perm relation is uncalibrated here\\nOutboard environment shale rich with Mud Volcanos - pro perm describes unclean shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("lowered anisotropy to 1\\napplied more vertical leak\\ntrial to describe shales Offshore Colombia\\nmultipoint poro perm relation\\nPoro/Perm relation is uncalibrated here\\nOutboard environment shale rich with Mud Volcanos - pro perm describes unclean shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Paralic Sand", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Paralic Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Pf", "Standard Ice"));
	map.insert(std::pair<std::string, std::string>("Permafrost", "Standard Ice"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Reference Coal(10%); Project Lithology Std. Sandstone(50%); Project Lithology Std. Shale(40%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(65%); Project Lithology Std. Shale(5%); Project Lithology Std. Siltstone(30%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(80%); Project Lithology Std. Siltstone(20%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(10%); Project Lithology Std. Siltstone(25%); Project Lithology Std. Sandstone(65%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(55%); Project Lithology Std. Siltstone(30%); Project Lithology Std. Shale(15%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Reference Coal(20%); Project Lithology Std. Sandstone(40%); Project Lithology Std. Siltstone(40%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("SabahShale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Salt_heavy", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("Sandstone Niger", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Sandstone_NEP", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("SB DW Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\n\\nCopy SM sand perm model", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Average thermal cond k.\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Increased density based on gravity fit.\\nAverage thermal cond k.\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\n\\nSeisVel=6500 instead of 5750\\nHighest realistic thermal conductivity of 3.5 (instead of average 2.5)", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\n\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5)", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Lower velocity like 08_Basalt_typical_BPA2_LK17012020\\n\\nSlightly adjusted to new hybrid properties (density, heat prod, porosity).\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5).\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Slightly adjusted to new hybrid properties (density, heat prod, porosity).\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5).\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Slightly adjusted to new hybrid properties (density, heat prod, porosity).\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5).\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\n", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Higher perm just to run faster.\\n\\nSlightly adjusted to new hybrid properties (density, heat prod, porosity).\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5).\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Zero por (to ignore low perm) just to run faster.\\n\\nSlightly adjusted to new hybrid properties (density, heat prod, porosity).\\nSeisVel=6500 instead of 5750\\nLowest realistic thermal conductivity of 1.5 (instead of average 2.5).\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model\\n\\nNon-zero porosity, high CC to align with UMCL crust lithology", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%);\\nAssign density 2830 for consistency with CTC basalt (combines true basalt and mantle depletion uplift effects).", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Grainstone(30%); Project Lithology Std. Shale(10%); Project Lithology Gippsland Basin Calcillutite(60%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("trial to describe shales in Sergipe Alagoas Basin (JRF2015), elevated permeability", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(65%); Project Lithology Std. Sandstone(35%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(85%); Project Lithology Std. Sandstone(15%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(95%); Project Lithology Std. Sandstone(5%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale_a", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale_b", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale_c", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale_NEP", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Shale created based on an ealier model input\\n\\nthe STP thermal conductivity has been lowered from\\n\\n1.4 => 1.1 m", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Gr Luconia", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(10%); Project Lithology Std. Shale(90%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Chalk", "Std. Chalk"));
	map.insert(std::pair<std::string, std::string>("Standard Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("Standard Grainstone (Limestone)", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(60%); Project Lithology Std. Siltstone(40%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology STD-Bonny-SHale(90%); Project Lithology Std. Siltstone(10%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Charles", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Shahejie_Shale(65%); Project Lithology Delta_SS_For_BBB(35%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Shahejie_Shale(56%); Project Lithology 12 Sandstone, typical, BPA2(44%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("IGNS Coal", "Std. Coal"));
	map.insert(std::pair<std::string, std::string>("IGNS Limestone", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("IGNS Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("IGNS Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("IGNS Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("IGNS Siltstone", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("IGNS Coal Tar_Regl", "Std. Coal"));
	map.insert(std::pair<std::string, std::string>("IGNS Sst Tar_Regl", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("IGNS Silts Tar_regl", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 12_Sandstone_typical_BPA2_LK03072019_OPT(10%); Project Lithology 15_Siltstone_BPA2_03072019(10%); Project Lithology 23_Mudstone_65CF_BPA2_LK03072019(80%); ", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Top mobile shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Ulakwo-Shale-STD(20%); Project Lithology Ulakwo-STD(80%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(10%); Project Lithology Std.Grainstone(90%); ", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(70%); Project Lithology Std.Dolo.Mudstone(30%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology ULAKWO-SST-STD(90%); Project Lithology Std. Sandstone(10%); ", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(75%); Project Lithology Std. Sandstone(25%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(75%); Project Lithology Std.Lime Mudstone(25%); ", "Std. Shale"));
	//Part2 mapping....lithologies are based on new BPA2 lithotypes
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Hugo Std Sand(50%); Project Lithology hugo Std Shale(50%); ", "Sandstone, lithic"));
	map.insert(std::pair<std::string, std::string>("See \\n\\n\\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\n\\n\\nFor use in upper part of oceanic crust extrusive, rubble sediment layers.", "Basalt, oceanic crust"));
	map.insert(std::pair<std::string, std::string>("See \\n\\n\\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\n\\n\\nUse in SDRs if known very high density in logs (eg Lopra, Faroes).", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 29 Shale, organic-rich, BPA2(40%); Project Lithology 15 Siltstone, BPA2(10%); Project Lithology 25 Mudstone, 70% clay, BPA2(50%); ", "Shale, organic rich"));
	map.insert(std::pair<std::string, std::string>("Based on New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\n\\nAllow minor porosity and permeability near surface.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Based on New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\nAllow minor porosity and permeability near surface.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Higher-end k\\n\\nBased on New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\nAllow minor porosity and permeability near surface.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Lower-end k\\n\\nBased on New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\nAllow minor porosity and permeability near surface.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(40%); Project Lithology Std. Siltstone(40%); ", "Mudstone, 30% clay"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(40%); Project Lithology Std. Shale(40%); Project Lithology Std. Marl(20%); ", "Mudstone, 30% clay"));
	map.insert(std::pair<std::string, std::string>("Allow minor porosity and permeability near surface.\\n\\nEx New Std Candidate Extrusive Basalt SDR Hybrid LK16032017\\n\\nLower density.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Average density at 7 km c. 2850, similar to gross typical oceanic crust, tending to c. 2900+-15 at >> 10 km. Virtually no porosity left below 3 km, or if there is thick overburden, so will introduce density deficit at seaward end of line. Test with UP density of 2950.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Try to adjust both density and compaction to reproduce velocity in EW refraction points.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Comparison of delLP and basalt thickness suggest more or less 200 kgm-3 apparent drop required (matches lower density mantle expected in 70-100 km thick melt extraction zone).", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Slightly denser than v3. 3180 peak density, to mimic possible metamorphic effects beneath Permo-Triassic overburden and or intermixing of serp and basalt component in slow spreading type crust in phase 1.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Compacts to c. 2835 at depth.\\n\\nMax out compensation due to basalt thickness depletion effect on mantle = 2 kgm-3 per % melt fraction.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Compacts to c. 2835 at depth.\\nMax out compensation due to basalt thickness depletion effect on mantle = 2 kgm-3 per % melt fraction.", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("As SDR Basal +Volcanoclastics LK17062015\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Mimic Hoggard 2017 then drop for SDR component in NW", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Reduce conductivity slightly following density reduction", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Mimic Hoggard 2017", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std.Dolo.Grainstone(50%); ", "Mudstone, 45% clay, high-velocity-density"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology 30%Clay_LowThermConduc(80%); Project Lithology 12 Sandstone, typical, BPA2(20%); ", "Mudstone, 30% clay"));
	map.insert(std::pair<std::string, std::string>("IGNS Andesite", "Basalt, oceanic crust"));
	map.insert(std::pair<std::string, std::string>("As SDR Basal +Volcanoclastics LK17062015\\nAdjusted velocity again\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model", "Basalt, SDR extrusive flows"));
	map.insert(std::pair<std::string, std::string>("As SDR Basal +Volcanoclastics LK17062015 but with higher k\\nAdjusted velocity again\\n\\nMix Lithology created from: Project Lithology Std. Basalt (90%); Project Lithology Std. Sandstone (10%); Created for SDR Wedge Orange Basin.\\nCopy SM sand perm model", "Basalt, SDR extrusive flows"));

	//Map created for Soil Mechanics porosity model based unparented legacy lithologies 
	///mapped to BPA-legacy lithology names =>
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(30%); Project Lithology Std.Lime Mudstone(70%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("New Salt SM GVB", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(75%); Project Lithology Std.Lime Mudstone(25%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(40%); Project Lithology Std.Lime Mudstone(60%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("Cal_sand_base", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("SM80_AJ", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Klaus_Carb", "Std.Dolo.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Chukchi_sand_base", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 60% Clay", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("New Salt SM GVB\\n90% Bill salt (5 W/mK) + 10% shale (1.8 W/mK) = 4.5 (W/mK) following power average", "Std. Salt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(99%); Project Lithology Std. Siltstone(1%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(1%); Project Lithology HEAT Shale(99%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(70%); Project Lithology Std. Siltstone(30%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(30%); Project Lithology Std. Siltstone(70%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 40% Clay", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("20 pc of salt / 80pc of Klaus SH 50", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 60% Clay\\nBased on Klasu_HP_shale cold", "SM.Mudst.60%Clay"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Mudstone 50% Clay", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Std. Grainstone", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Klaus_Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("Soil Mechanics Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Soil Mecha40% Clay", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("Standard Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Klaus_SS_nopress", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mechanics Sandstone", "Std. Sandstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Dolo.Mudstone(20%); Project Lithology Std.Grainstone(30%); Project Lithology Std.Lime Mudstone(50%); ", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("20 pc of salt / 80pc of Klaus SH 50", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Standard Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Pebble_HP_shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Soil Mecha40% Clay mixed with Sand", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("QClevis_UWX_ori", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Standard Grainstone (Limestone)", "Std.Grainstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(50%); Project Lithology Std.Lime Mudstone(30%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std.Lime Mudstone(30%); Project Lithology Std. Shale(70%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Uncompacted shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Special Shale", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Shale_A4_2", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("SM Lime Mudstone", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("SM Marl", "Std. Marl"));
	map.insert(std::pair<std::string, std::string>("SM seal", "SM.Mudst.50%Clay"));
	map.insert(std::pair<std::string, std::string>("Low radioactive Sh.", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("SM.Lime Mudstone", "Std.Lime Mudstone"));
	map.insert(std::pair<std::string, std::string>("SM.Mdst40_heavier", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("Standard Basalt", "Std. Basalt"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(1%); Project Lithology Std. Sandstone(99%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Siltstone(99%); Project Lithology Std. Sandstone(1%); ", "Std. Siltstone"));
	map.insert(std::pair<std::string, std::string>("Soil 50_SS__50_CLAY", "SM.Mudst.40%Clay"));
	map.insert(std::pair<std::string, std::string>("IGNS Shale Tar_Regl", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std.Lime Mudstone(30%); Project Lithology Std. Siltstone(20%); ", "Std. Shale"));
	map.insert(std::pair<std::string, std::string>("Assuming 30% Notional porosity for capturing the THCB", "SM.Mudst.50%Clay"));

	///mapped to BPA2 lithology names=>
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Siltstone(50%); ", "Mudstone, 30% clay"));
	map.insert(std::pair<std::string, std::string>("Mix Lithology created from: Project Lithology Std. Sandstone(50%); Project Lithology Std. Shale(50%); ", "Mudstone, 30% clay"));

	return map;
}

std::string Prograde::LithologyConverter::upgradeLithologyName(const std::string & legacyLithotypeName)
{
	//std::string newLithoTypeNameAssigned;
	auto lithology = lithologyNameMaps.find(legacyLithotypeName);

	if (lithology != lithologyNameMaps.end())
	{
		return lithology->second;
	}

	return legacyLithotypeName;
}
std::string Prograde::LithologyConverter::upgradeLithologyDescription(std::string & legacyDescription, const int & lithologyFlag, const std::string & legacyParentLithology)
{
	std::string updatedDescription;
	updatedDescription = legacyDescription;
	if (lithologyFlag == 1)
	{
		updatedDescription = legacyDescription + "( Based on BPA " + legacyParentLithology + ")";
	}
	if (updatedDescription.size()>1000)
	{
		if (lithologyFlag == 0)
		{
			updatedDescription.resize(1000);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Description: Length of legacy description exceeds the allowed length of BPA2. Resetting the length to 1000 characters.";
		}
		else
		{
			int SizeOfAppendedString = static_cast<int>(legacyParentLithology.size() + 16);
			int allowedLegacyDescriptionSize = 1000 - SizeOfAppendedString;
			legacyDescription.resize(allowedLegacyDescriptionSize);
			updatedDescription = legacyDescription + "( Based on BPA " + legacyParentLithology + ")";

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Description: Length of legacy description exceeds the allowed length of BPA2. Cutting the length of the original description and appended 'Based on legacy BPA <legacyParentLithoName' to it.";
		}
	}
	else
	{
		if (lithologyFlag == 0)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Description: No upgrade is done for this standard lithology which will get updated as per the description of the BPA2 mapped lithology while importing into BPA2-Basin.";
		else
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Description: Updated the description for this userDefined lithology by appending 'Based on legacy BPA <legacyParentLithoName>'";

	}
	return updatedDescription;
}

void Prograde::LithologyConverter::upgradeLithologyAuditInfo(std::string & DefinitionDate, std::string & LastChangedBy, std::string & LastChangedDate, const int & lithologyFlag)
{
	std::string OriginalDefinitionDate = DefinitionDate;
	std::string OriginalLastChangedBy = LastChangedBy;
	std::string OriginalLastChangedDate = LastChangedDate;

	if (lithologyFlag == 0)
	{
		DefinitionDate = "November 21 2019 12:00";
		LastChangedBy = "Lorcan Kenan";
		LastChangedDate = "November 21 2019 12:00";

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Audit Info: Updating DefinitionDate from '" << OriginalDefinitionDate << "' to '" << DefinitionDate << "', LastChangedBy from '" << OriginalLastChangedBy << "' to '" << LastChangedBy << "' and LastChangedDate from '" << OriginalLastChangedDate << "' to '" << LastChangedDate << "'.";
	}
	else
	{
		LastChangedBy = "Lorcan Kenan";
		LastChangedDate = "November 21 2019 12:00";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Audit Info: Not changing the DefinitionDate but updating LastChangedBy from '" << OriginalLastChangedBy << "' to '" << LastChangedBy << "' and LastChangedDate from '" << OriginalLastChangedDate << "' to '" << LastChangedDate << "'.";
	}
}

std::string Prograde::LithologyConverter::findParentLithology(std::string legacydefinedBy)
{
	std::string ParentLithoName;

	//The legacy format of DefinedBy field is: "BPA REF INFO|<StadandLitho1>|<StadandLitho2>"...and the actual parent lithology name is <StadandLitho1>
	size_t pos_FirstPipe = legacydefinedBy.find("|");
	size_t pos_SecondPipe = legacydefinedBy.find_last_of("|");
	ParentLithoName = legacydefinedBy.substr((pos_FirstPipe + 1), (pos_SecondPipe - 1) - pos_FirstPipe);

	return ParentLithoName;
}

std::string Prograde::LithologyConverter::findMissingParentLithology(const std::string lithologyName, std::string lithologyDescription)
{
	std::string ParentLithoName{};

	//Remove the "spaces" from the original lithology description and then compare find the corresponding descriptions available in the mappingOfLithologyNameBasedOndescription
	lithologyDescription.erase(remove(lithologyDescription.begin(), lithologyDescription.end(), ' '), lithologyDescription.end());

	for (auto it = mappingOfLithologyNameBasedOndescription.begin(); it != mappingOfLithologyNameBasedOndescription.end(); ++it)
	{
		std::string description;
		description = it->first;
		description.erase(remove(description.begin(), description.end(), ' '), description.end());

		if (description == lithologyDescription)
		{
			ParentLithoName = it->second;
			if (ParentLithoName == "Std. Basalt" and (lithologyName == "SDR_basalt+volclastic_LK04022014 EH27102015 (high k)" or lithologyName == "SDR_basalt+volclastic_LK04022014 EH26102015 (low k)"))
				//This check is done because the same description is mapped to "Std. Basalt" and "Basalt, SDR extrusive flows". The later is basically derived from the lithotype name. 
				ParentLithoName = "Basalt, SDR extrusive flows";
			break;
		}
	}

	return ParentLithoName;
}

bool Prograde::LithologyConverter::isDefinedBeforeThanCutOffDate(const std::string dateOfCreation)
{
	std::string creationMonth, creationDate, creationYear, creationDateAndYear;

	//in BPA1, the date format is : month_date_year_time (underscore are spaces only) i.e., "June 01 2020 09:49"
	size_t startPos = dateOfCreation.find_first_not_of(" ");;
	size_t pos_FirstSpace = dateOfCreation.find_first_of(" ");
	size_t pos_lastSpace = dateOfCreation.find_last_of(" ");

	creationMonth = dateOfCreation.substr(startPos, pos_FirstSpace);
	creationDateAndYear = dateOfCreation.substr((pos_FirstSpace + 1), ((pos_lastSpace - 1) - pos_FirstSpace));

	startPos = creationDateAndYear.find_first_not_of(" ");
	pos_FirstSpace = creationDateAndYear.find_first_of(" ");
	creationDate = creationDateAndYear.substr(startPos, pos_FirstSpace);
	creationYear = creationDateAndYear.substr(pos_FirstSpace + 1);

	std::vector<std::pair<std::string, std::string>> monthMap;
	monthMap.push_back(std::make_pair("January", "1"));
	monthMap.push_back(std::make_pair("February", "2"));
	monthMap.push_back(std::make_pair("March", "3"));
	monthMap.push_back(std::make_pair("April", "4"));
	monthMap.push_back(std::make_pair("May", "5"));
	monthMap.push_back(std::make_pair("June", "6"));
	monthMap.push_back(std::make_pair("July", "7"));
	monthMap.push_back(std::make_pair("August", "8"));
	monthMap.push_back(std::make_pair("September", "9"));
	monthMap.push_back(std::make_pair("October", "10"));
	monthMap.push_back(std::make_pair("November", "11"));
	monthMap.push_back(std::make_pair("December", "12"));

	// Convert name of the month to number format
	for (int i = 0; i < monthMap.size(); i++)
	{
		if (creationMonth == monthMap[i].first)
		{
			creationMonth = monthMap[i].second;
			break;
		}
	}

	std::stringstream y(creationYear), m(creationMonth), d(creationDate);
	int year = 0, month = 0, date = 0;
	y >> year;
	m >> month;
	d >> date;

	if (year<2020)
	{
		return true;
	}
	else
	{
		if (month <= 4)
			return true;
	}

	return false;
}


void Prograde::LithologyConverter::upgradePermModelForUsrDefLitho(const std::string & permModelName, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts)
{
	if ((permModelName == "None") || (permModelName == "Impermeable"))
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6;
		mpPerm[1] = -6;
	}
	else if (permModelName == "Sands")
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = 0.2;
		mpPerm[1] = 4.9;
	}
	else if (permModelName == "Shales")
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6.4;
		mpPerm[1] = -2.16;
	}
}

ErrorHandler::ReturnCode Prograde::LithologyConverter::PreprocessLithofaciesInputOfStratIoTbl(std::vector<std::string>& lithologyNamesSingleLayer, std::vector<double>& lithoPercntSingleLayer, std::vector<std::string>& lithoPercntGridSingleLayer)
{
	if (lithologyNamesSingleLayer.size() != 3 or lithoPercntSingleLayer.size() != 3 or lithoPercntGridSingleLayer.size() != 2 or
		(lithologyNamesSingleLayer[0] == "" and lithologyNamesSingleLayer[1] == "" and lithologyNamesSingleLayer[2] == ""))
	{//Added just as a precautionary measure to filter it out erronious scenarios like this from importing in to BPA2-Basin
		return ErrorHandler::ReturnCode::ValidationError;
	}
	else
	{
		for (size_t temp = 0; temp < lithologyNamesSingleLayer.size(); temp++)
		{
			std::string legacyLithoTypeName = lithologyNamesSingleLayer[temp];
			double legacyLithoPerctScalar = lithoPercntSingleLayer[temp];

			//This loop is provided to preprocess the inputs as per the need of Import validation...If the inputs are not as per the BPA2 import validation code structure then modify the inputs
			//	-> Lithology can't be imported if the lithology name is not available but lithology percents are available.
			//		-> If this is the case, then reset the lithology percentages to NO-DATA-VALUE
			//	-> If lithology is available but lithology percentages have both scalar and map specified then 
			//		-> Keep the map info and reset the scalar value to NO-DATA-VALUE
			if (legacyLithoTypeName.compare("") == 0 && legacyLithoPerctScalar != DataAccess::Interface::DefaultUndefinedScalarValue)
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Lithology percent is found whereas the Lithotype" << (temp + 1) << " is not defined. " <<
					"Resetting Percent" << (temp + 1) << " from " << legacyLithoPerctScalar << " to " << DataAccess::Interface::DefaultUndefinedScalarValue;

				lithoPercntSingleLayer[temp] = DataAccess::Interface::DefaultUndefinedScalarValue;
				if (temp < 2)
				{
					std::string legacyLithoPerctMap = lithoPercntGridSingleLayer[temp];
					if (legacyLithoPerctMap != DataAccess::Interface::NullString)
					{
						lithoPercntGridSingleLayer[temp] = DataAccess::Interface::NullString;
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Resetting lithology PercentGrid" << (temp + 1) << " from " << legacyLithoPerctMap << " to " << DataAccess::Interface::NullString;
					}
				}

				///Check if lithology==NULL, percent=-9999 but PercentGrid != NULL...Is this a possible use case????
			}
			else
			{
				std::string bpa2LithoName = upgradeLithologyName(legacyLithoTypeName);
				lithologyNamesSingleLayer[temp] = bpa2LithoName;
				if (bpa2LithoName != legacyLithoTypeName)
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Lithotype" << (temp + 1) << " name is updated from '" << legacyLithoTypeName << "' to '" << bpa2LithoName << "'";
				if (temp < 2 && lithoPercntSingleLayer[temp] != DataAccess::Interface::DefaultUndefinedScalarValue && lithoPercntGridSingleLayer[temp] != "")
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Lithotype" << (temp + 1) << " has both scalar and maps. Percent" << (temp + 1) << " value is updated from " << lithoPercntSingleLayer[temp] << " to " << DataAccess::Interface::DefaultUndefinedScalarValue;
					lithoPercntSingleLayer[temp] = DataAccess::Interface::DefaultUndefinedScalarValue;
				}
			}
		}
	}

	return ErrorHandler::ReturnCode::NoError;
}

void Prograde::LithologyConverter::upgradePermModelForSysDefLitho(const std::string & bpa2LithoName, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts)
{
	numPts = 2;
	mpPor.resize(numPts);
	mpPerm.resize(numPts);
	if ((bpa2LithoName.compare("Mantle") == 0) || (bpa2LithoName.compare("Crust") == 0) ||
		(bpa2LithoName.compare("Diorite/Granodiorite (SI)") == 0) || (bpa2LithoName.compare("Dry Gabbro/Basalt (SI)") == 0) ||
		(bpa2LithoName.compare("Wet Gabbro/Basalt (SI)") == 0) || (bpa2LithoName.compare("Granite/Rhyolite (SI)") == 0) ||
		(bpa2LithoName.compare("Basalt, typical") == 0))
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -4;
		mpPerm[1] = -4;
	}
	else if (bpa2LithoName.compare("Chalk, white") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -5.45;
		mpPerm[1] = 1.8;
	}
	else if (bpa2LithoName.compare("Grainstone, dolomitic, typical") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = 1.6;
		mpPerm[1] = 6.4;
	}
	else if (bpa2LithoName.compare("Grainstone, calcitic, typical") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = 1.6;
		mpPerm[1] = 6.4;
	}
	else if (bpa2LithoName.compare("Sandstone, typical") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = 0.2;
		mpPerm[1] = 4.9;
	}
	else if (bpa2LithoName.compare("Mudstone, 60% clay") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6.4;
		mpPerm[1] = -2.16;
	}
	else if (bpa2LithoName.compare("Mudstone, 50% clay") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6.19;
		mpPerm[1] = -1.28;
	}
	else if (bpa2LithoName.compare("Mudstone, 40% clay") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -5.98;
		mpPerm[1] = -0.39;
	}
	else if ((bpa2LithoName.compare("Ice") == 0) || (bpa2LithoName.compare("Anhydrite") == 0) ||
		(bpa2LithoName.compare("Coal") == 0) || (bpa2LithoName.compare("Halite") == 0) ||
		(bpa2LithoName.compare("Potash-Sylvite") == 0))
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6;
		mpPerm[1] = -6;
	}
	else if (bpa2LithoName.compare("Marl") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -5.98;
		mpPerm[1] = -0.39;
	}
	else if (bpa2LithoName.compare("Siltstone, 20% clay") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -5.55;
		mpPerm[1] = 1.37;
	}
	else if (bpa2LithoName.compare("Lime-Mudstone, dolomitic") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6.19;
		mpPerm[1] = -1.28;
	}
	else if (bpa2LithoName.compare("Lime-Mudstone") == 0)
	{
		mpPor[0] = 5;
		mpPor[1] = 60;
		mpPerm[0] = -6.19;
		mpPerm[1] = -1.28;
	}
	else
	{
		//
	}
}

void Prograde::LithologyConverter::computeSingleExpModelParameters(const std::string legacyParentLithoName, const int lithologyFlag, mbapi::LithologyManager::PorosityModel &porModel, std::vector<double> & originalPorModelParam, std::vector<double> & newPorModelParam)
{
	std::string baseLithologyType = legacyParentLithoName;

	double newSurfacePorosity;
	double newMinMechanicalPorosity;
	double newExponentialCompCoeff;

	if (lithologyFlag == 1 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics)
	{
		if ((baseLithologyType.compare("HEAT Sandstone") == 0) || (baseLithologyType.compare("SM. Sandstone") == 0) || (baseLithologyType.compare("Std. Sandstone") == 0))
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 45.0;
			newExponentialCompCoeff = 3.25;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Sandstone,typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Siltstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 5.5;
			newMinMechanicalPorosity = 2.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Siltstone, clean'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Shale") == 0 || baseLithologyType.compare("HEAT Shale") == 0 ||
			baseLithologyType.compare("SM.Mudst.40%Clay") == 0 || baseLithologyType.compare("SM.Mudst.50%Clay") == 0 ||
			baseLithologyType.compare("SM.Mudst.60%Clay") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with model parameters calculated from the imperical formula. ";

			newSurfacePorosity = (0.0173*originalPorModelParam[0] * originalPorModelParam[0]) - (0.6878*originalPorModelParam[0]) + 27.73;
			//Newly calculated SurfacePorosity values must be in the range [0.1,100]. Resetting it's value if it is outside this range
			if (newSurfacePorosity < 0.1) {
				newSurfacePorosity = 0.1;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated SurfacePorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
			}
			if (newSurfacePorosity > 100.0) {
				newSurfacePorosity = 100.0;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated SurfacePorosity is greater than the maximum allowed value of 100. Resetting its value to 100.";
			}

			newExponentialCompCoeff = (-1.508*originalPorModelParam[1] * originalPorModelParam[1]) + (2.526*originalPorModelParam[1]) + 5.05;
			//Newly calculated compaction coefficient values must be in the range [0,25]. Resetting it's value if it is outside this range
			if (newExponentialCompCoeff < 0.0) {
				newExponentialCompCoeff = 0.0;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated CompactionCoeff. is less than the minimum allowed value of 0. Resetting its value to 0.";
			}
			if (newExponentialCompCoeff > 25.0) {
				newExponentialCompCoeff = 25.0;//Checking for the allowed range specified for BPA2
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated CompactionCoeff. is greater than the maximum allowed value of 25. Resetting its value to 25.";
			}

			newMinMechanicalPorosity = (0.00123*originalPorModelParam[0] * originalPorModelParam[0]) - (0.04913*originalPorModelParam[0]) + 4.34;
			//Newly calculated Min. Porosity values must be in the range [0.1,SurfacePorosity]. Resetting it's value if it is outside this range
			if (newMinMechanicalPorosity < 0.1) {
				newMinMechanicalPorosity = 0.1;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated MinPorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
			}
			if (newMinMechanicalPorosity > newSurfacePorosity) {
				newMinMechanicalPorosity = newSurfacePorosity;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Newly calculated MinPorosity is greater than the value of SurfacePorosity. Resetting its value to SurfacePorosity.";
			}

			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Dolo.Grainstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Grainstone, dolomitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Grainstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Grainstone, calcitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Lime Mudstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 6.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Lime-Mudstone'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if ((baseLithologyType.compare("Std.Dolo.Mudstone")) == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 6.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Lime-Mudstone, dolomitic'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Marl") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 50.0;
			newExponentialCompCoeff = 5.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Marl'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("HEAT Limestone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Graistone, calcitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Salt") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 0.1;
			newExponentialCompCoeff = 0.0;
			newMinMechanicalPorosity = 0.1;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Halite'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Coal") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 75.0;
			newExponentialCompCoeff = 15;
			newMinMechanicalPorosity = 1.1;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Warning> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Coal'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else {
			newPorModelParam = originalPorModelParam;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is not upgraded to 'Exponential' as no mapping is available for this parent lithology. ";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Error> Migration from BPA to BPA2 Basin Aborted...";
			exit(42);
		}
	}
	else if (lithologyFlag == 0 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics) {
		newPorModelParam = originalPorModelParam;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Deprecated 'SoilMechanics' model is detected for this standard lithotype. No upgradation of the porosity model is done via PROGRADE which will be updated from the BPA2 reference catalog while importing the scenario in BPA2. ";
	}
	else if (lithologyFlag == 0 && porModel == mbapi::LithologyManager::PorosityModel::PorExponential) {
		newPorModelParam = originalPorModelParam;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Latest porosity model is detected for this standard lithotype. No upgradation of the porosity model parameters are done via PROGRADE which will be updated from the BPA2 reference catalog while importing the scenario in BPA2. ";
	}
	else //For all legacy user defined defined lithologies with Expoenential porosity model. 
		 //No upgradation of porosity model is done, but checking for the out-of-range values only and handled them 
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Latest porosity model is detected for this lithotype. No upgradation of the porosity model is needed. ";

		//SurfacePorosity values must be in the range [0.1,100]. Resetting it's value if it is outside this range
		if (originalPorModelParam[0] < 0.1) {
			originalPorModelParam[0] = 0.1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy SurfacePorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
		}
		if (originalPorModelParam[0] > 100.0) {
			originalPorModelParam[0] = 100.0;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy SurfacePorosity is greater than the maximum allowed value of 100. Resetting its value to 100.";
		}

		//Compaction coefficient values must be in the range [0,25]. Resetting it's value if it is outside this range
		if (originalPorModelParam[1] < 0.0) {
			originalPorModelParam[1] = 0.0;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy CompactionCoeff. is less than the minimum allowed value of 0. Resetting its value to 0.";
		}
		if (originalPorModelParam[1] > 25.0) {
			originalPorModelParam[1] = 25.0;//Checking for the allowed range specified for BPA2
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy CompactionCoeff. is greater than the maximum allowed value of 25. Resetting its value to 25.";
		}
		//Newly calculated Min. Porosity values must be in the range [0.1,SurfacePorosity]. Resetting it's value if it is outside this range
		if (originalPorModelParam[2] < 0.1) {
			originalPorModelParam[2] = 0.1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy MinPorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
		}
		if (originalPorModelParam[2] > originalPorModelParam[0]) {
			originalPorModelParam[2] = originalPorModelParam[0];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Legacy MinPorosity is greater than the value of SurfacePorosity. Resetting its value to SurfacePorosity.";
		}
		newPorModelParam = originalPorModelParam;
	}

	//return newPorModelParam;

}

void Prograde::LithologyConverter::upgradeLitPropDensity(double & value)
{
	double lwrLimit = 500;
	double uprLimit = 10000;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Density value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Density value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropHeatProduction(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 25;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Heat Production value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Heat Production value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropThrConductivity(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 10;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal Conductivity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal Conductivity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropThrCondAnistropy(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 10;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal Conductivity Anistropy value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal Conductivity Anistropy value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropPermAnistropy(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 100;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Permeability Anistropy value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Permeability Anistropy value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropSeisVelocity(double & value)
{
	double lwrLimit = 1000;
	double uprLimit = 9000;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropSeisVeloExponent(double & value)
{
	double lwrLimit = -1;
	double uprLimit = 1;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity Exponent value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity Exponent value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropEntryPresCoeff1(double & value)
{
	double lwrLimit = 0.1;
	double uprLimit = 2;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 1 value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 1 value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropEntryPresCoeff2(double & value)
{
	double lwrLimit = -1.5;
	double uprLimit = 1.5;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 2 value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 2 value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropHydFracturing(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 100;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Hydraulic Fracturing value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Hydraulic Fracturing value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropRefSoldViscosity(double & value)
{
	double lwrLimit = 0;
	double uprLimit = 1e18;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original ReferenceSolidViscosity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original ReferenceSolidViscosity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}

}
void Prograde::LithologyConverter::upgradeLitPropIntrTemperature(double & value)
{
	double lwrLimit = 600;
	double uprLimit = 1650;
	if (value == -9999)
		return;
	if (value < lwrLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Intrusion Temperature value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
		value = lwrLimit;
	}
	else if (value > uprLimit)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Intrusion Temperature value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
		value = uprLimit;
	}
}
