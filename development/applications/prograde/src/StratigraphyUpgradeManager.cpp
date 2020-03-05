//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StratigraphyUpgradeManager.h"
#include "StratigraphyModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "StratigraphyManager.h"

#include <algorithm>

using namespace mbapi;

//------------------------------------------------------------//

Prograde::StratigraphyUpgradeManager::StratigraphyUpgradeManager(Model& model) :
	IUpgradeManager("stratigraphy upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::StratigraphyUpgradeManager::upgrade() {
	Prograde::StratigraphyModelConverter modelConverter;
	
	std::string name;
	std::string updated_name;

	//updating the LayerName, SurfaceName and Fluidtype in StratIoTbl
	database::Table * stratIo_table = m_ph->getTable("StratIoTbl");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName, LayerName and Fluidtype in StratIoTbl :";

	for (size_t id = 0; id < stratIo_table->size(); ++id) {
		database::Record * rec = stratIo_table->getRecord(static_cast<int>(id));

		name = rec->getValue<std::string>("SurfaceName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
		rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));

		name = rec->getValue<std::string>("LayerName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
		rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		
		name = rec->getValue<std::string>("Fluidtype");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Fluidtype detected :" << name;
		if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water")) { //detecting user-defined fluid
			rec->setValue<std::string>("Fluidtype", modelConverter.upgradeName(name));
		}
		else {
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "System defined fluid detected. No change required";
		}
	}

	//updating the LayerName, SurfaceName and Fluidtype in the respective tables

	database::Table * ctcIo_table = m_ph->getTable("CTCIoTbl");
	if (ctcIo_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in CTCIoTbl :";
		for (size_t id = 0; id < ctcIo_table->size(); ++id) {
			database::Record * rec = ctcIo_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	
	database::Table * palinspasticio_table = m_ph->getTable("PalinspasticIoTbl");
	if (palinspasticio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in PalinspasticIoTbl :";
		for (size_t id = 0; id < palinspasticio_table->size(); ++id) {
			database::Record * rec = palinspasticio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	/*
	//////////////////////////////////////////////////NOT SURE///////////////////////////////////
	database::Table * faultcutio_table = m_ph->getTable("FaultcutIoTbl");
	if (faultcutio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in FaultcutIoTbl :";
		for (size_t id = 0; id < faultcutio_table->size(); ++id) {
			database::Record * rec = faultcutio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	database::Table * twowaytimeio_table = m_ph->getTable("TwoWayTimeIoTbl");
	if (twowaytimeio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in TwoWayTimeIoTbl :";
		for (size_t id = 0; id < twowaytimeio_table->size(); ++id) {
			database::Record * rec = twowaytimeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	
	database::Table * touchstonemapio_table = m_ph->getTable("TouchstoneMapIoTbl");
	if (touchstonemapio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in TouchstoneMapIoTbl :";
		for (size_t id = 0; id < touchstonemapio_table->size(); ++id) {
			database::Record * rec = touchstonemapio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}

	database::Table * moblaythickio_table = m_ph->getTable("MobLayThicknIoTbl");
	if (moblaythickio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in MobLayThicknIoTbl :";
		for (size_t id = 0; id < moblaythickio_table->size(); ++id) {
			database::Record * rec = moblaythickio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	
	database::Table * allochthonlithointerpio_table = m_ph->getTable("AllochthonLithoInterpIoTbl");
	if (allochthonlithointerpio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoInterpIoTbl :";
		for (size_t id = 0; id < allochthonlithointerpio_table->size(); ++id) {
			database::Record * rec = allochthonlithointerpio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}

	database::Table * allochthonlithodistribio_table = m_ph->getTable("AllochthonLithoDistribIoTbl");
	if (allochthonlithodistribio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoDistribIoTbl :";
		for (size_t id = 0; id < allochthonlithodistribio_table->size(); ++id) {
			database::Record * rec = allochthonlithodistribio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}

	database::Table * allochthonlithoio_table = m_ph->getTable("AllochthonLithoIoTbl");
	if (allochthonlithoio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoIoTbl :";
		for (size_t id = 0; id < allochthonlithoio_table->size(); ++id) {
			database::Record * rec = allochthonlithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}

	database::Table * sourcerocklithoio_table = m_ph->getTable("SourceRockLithoIoTbl");
	if (sourcerocklithoio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in SourceRockLithoIoTbl :";
		for (size_t id = 0; id < sourcerocklithoio_table->size(); ++id) {
			database::Record * rec = sourcerocklithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}

	database::Table * fluidtypeio_table = m_ph->getTable("FluidtypeIoTbl");
	if (fluidtypeio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the Fluidtype in FluidtypeIoTbl :";
		for (size_t id = 0; id < fluidtypeio_table->size(); ++id) {
			database::Record * rec = fluidtypeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("Fluidtype");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Fluidtype detected :" << name;
			if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water")) { //detecting user-defined fluid
				rec->setValue<std::string>("Fluidtype", modelConverter.upgradeName(name));
			}
			else {
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "System defined fluid detected. No change required";
			}
		}
	}

	//Updating the StratioTbl for DepoAge,
	database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");


	unsigned num = 0;
	for (size_t id = 0; id < stratIo_Table->size(); ++id) {
		database::Record * rec = stratIo_Table->getRecord(static_cast<int>(id));
		name = rec->getValue<std::string>("SurfaceName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the Surface " << name << " in StratIoTbl for Depth & Thickness, MixModel, ChemicalCompaction and DepoAge :";
		//////////////////////////////////////////////////////////////////////////////////////////////
		double depth = rec->getValue<double>("Depth");
		double thickness = rec->getValue<double>("Thickness");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Depth value for the surface " << name << " is : " << depth;
		rec->setValue<double>("Depth", modelConverter.upgradeDepthThickness(depth));
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Thickness value for the surface " << name << " is : " << thickness;
		rec->setValue<double>("Thickness", modelConverter.upgradeDepthThickness(thickness));























		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		double age = rec->getValue<double>("DepoAge");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deposition age for the surface "<<name<<" is : "<<age;

		if (age > 999) {
			rec->setValue<double>("DepoAge",999);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deposition age exceeds the limit (0-999). So, the value is changed to 999";
			num++;
		}
	}












	//failing the scenario if more than one surfaces exceed the age limit (0-900)
	if (num > 1) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "More than one surfaces are crossing the age limits; Scenario Rejected";
		//exit(200);
	}







}






















/*
Prograde::BrineModelConverter modelConverter;
auto fluids = m_model.fluidManager().getFluidsID();
std::vector<std::string> FluidTypesUsed = StratIoTblReferredFluids();
std::vector<std::string> baseFluidForUserDefinedBrine;//To store the base fluid type for a userDefined brine
//std::string baseFluidForUserDefinedBrine;//To store the base fluid type for a userDefined brine
for (auto flId : fluids) {

std::string fluidName;

m_model.fluidManager().getFluidName(flId, fluidName);

LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "FluidType encountered in the FluidTypeIoTbl is: " << fluidName;

int fluidUserDefined;                      //Fluid user defined flag
m_model.fluidManager().getUserDefined(flId, fluidUserDefined);
m_model.fluidManager().setUserDefined(flId, modelConverter.upgradeUserDefined(fluidName, fluidUserDefined));
std::string fluidDescription;              //Fluid description
m_model.fluidManager().getDescription(flId, fluidDescription);
m_model.fluidManager().setDescription(flId, modelConverter.upgradeDescription(fluidName, fluidDescription));
std::string heatCapType;              //Fluid HeatCaptype
m_model.fluidManager().getHeatCapType(flId, heatCapType);
m_model.fluidManager().setHeatCapType(flId, modelConverter.upgradeHeatCapType(fluidName, fluidDescription, heatCapType));
std::string ThermCondType;              //Fluid HeatCaptype
m_model.fluidManager().getThermCondType(flId, ThermCondType);
m_model.fluidManager().setThermCondType(flId, modelConverter.upgradeThermCondType(fluidName, fluidDescription, ThermCondType));
std::string OriginalDefinedBy;
m_model.fluidManager().getDefinedBy(flId, OriginalDefinedBy);
m_model.fluidManager().setDefinedBy(flId, modelConverter.upgradeDefinedBy(fluidName, OriginalDefinedBy));//Updating the definedBy field for each fluidName

FluidManager::FluidDensityModel densModel; // Fluid density calculation model
double refDens;
m_model.fluidManager().densityModel(flId, densModel, refDens);
refDens = -9999;
m_model.fluidManager().setDensityModel(flId, modelConverter.upgradeDensityModel(densModel, fluidName), refDens);

FluidManager::CalculationModel seisVelModel; // Seismic velocity calculation model
double refSeisVel;
m_model.fluidManager().seismicVelocityModel(flId, seisVelModel, refSeisVel);
refSeisVel = -9999;
m_model.fluidManager().setSeismicVelocityModel(flId, modelConverter.upgradeSeismicVelocityModel(seisVelModel, fluidName), refSeisVel);

m_model.fluidManager().getUserDefined(flId, fluidUserDefined);
if (fluidUserDefined == 1)
{
for (size_t size = 0; size < (FluidTypesUsed.size() - 1); size++)
{
if (FluidTypesUsed[size] == fluidName)
{

m_model.fluidManager().getHeatCapType(flId, heatCapType);
baseFluidForUserDefinedBrine.push_back(heatCapType);
}
}

}
}
size_t countOfNotMatchingflId = 0;
bool isMatch = false;
int originalRowPosition = -1;
int nextFluidTypeIOTbl_Index = -1;

for (auto flId : fluids) {

std::string fluidName;
originalRowPosition++; // Original location of row entry to be checked
nextFluidTypeIOTbl_Index++;

m_model.fluidManager().getFluidName(nextFluidTypeIOTbl_Index, fluidName);
size_t size = FluidTypesUsed.size() - 1;//size of StratIoTbl neglecting the basement entry
size_t size_baseFluidForUserDefinedBrine = baseFluidForUserDefinedBrine.size();

for (size_t iteratorFluidTypeUsed = 0; iteratorFluidTypeUsed < size; iteratorFluidTypeUsed++)
{
if (size_baseFluidForUserDefinedBrine > 0)
{
for (size_t iteratorBaseFluidForUserDefinedBrine = 0; iteratorBaseFluidForUserDefinedBrine < size_baseFluidForUserDefinedBrine; iteratorBaseFluidForUserDefinedBrine++)
{
std::string xyz = FluidTypesUsed[iteratorFluidTypeUsed];
if (FluidTypesUsed[iteratorFluidTypeUsed] == fluidName || fluidName == baseFluidForUserDefinedBrine[iteratorBaseFluidForUserDefinedBrine])
{
isMatch = true;
break;
}
}
}
else if (size_baseFluidForUserDefinedBrine == 0)
{
if (FluidTypesUsed[iteratorFluidTypeUsed] == fluidName)
{
isMatch = true;
break;
}
}
}
if (isMatch)
{
// Don't remove from table
isMatch = false;
}
else
{
countOfNotMatchingflId++;
// remove record from table if not match
m_model.removeRecordFromTable("FluidtypeIoTbl", (originalRowPosition + 1) - countOfNotMatchingflId);
LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fluidName << " is not referred in StratIoTbl for the scenario";
LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deleting the row from FluidTypeIoTbl for " << fluidName;
nextFluidTypeIOTbl_Index--;
}
}
this->ResetFltThCondIoTbl();
this->ResetFltHeatCapIoTbl();
*/

/*
std::vector<std::string> Prograde::BrineUpgradeManager::StratIoTblReferredFluids()
{
auto layerId = m_model.fluidManager().getLayerID();
std::string FluidName;
std::vector<std::string> fluids;//vector to store the fluid types specified in the StratIoTbl corresponding to each layer
for (auto tsId : layerId)
{
m_model.fluidManager().getBrineType(tsId, FluidName);
fluids.push_back(FluidName);
}
//Removing the fluid types from the vector fluids which are repeated
auto end = fluids.end();
for (auto i = fluids.begin(); i != end; i++)
{
end = std::remove(i + 1, end, *i);
}
fluids.erase(end, fluids.end());

return fluids;
}

std::vector<std::string> Prograde::BrineUpgradeManager::ThermCondtypeReferred()
{
auto fluidsId = m_model.fluidManager().getFluidsID();
std::string ThCondType;
std::vector<std::string> thCondType;//vector to store thermConductivity types specified for each id of FluidtypeIoTbl
for (auto tsId : fluidsId)
{
m_model.fluidManager().getThermCondType(tsId, ThCondType);
thCondType.push_back(ThCondType);
}
//Removing the thermConductivity types from the vector thCondType which are repeated
auto end = thCondType.end();
for (auto i = thCondType.begin(); i != end; i++)
{
end = std::remove(i + 1, end, *i);
}
thCondType.erase(end, thCondType.end());

return thCondType;
}

void Prograde::BrineUpgradeManager::ResetFltThCondIoTbl()
{
auto ThCondId = m_model.fluidManager().getFluidThCondID();
double TemperatureValue, PressVal, ThCondVal;
std::vector<double> TempIndexValuesAll, PressureValAll, ThCondValAll;
for (auto flId : ThCondId) {
m_model.fluidManager().getThermalCondTblTempIndex(flId, TemperatureValue);
TempIndexValuesAll.push_back(TemperatureValue);
m_model.fluidManager().getThermalCondTblPressure(flId, PressVal);
PressureValAll.push_back(PressVal);
m_model.fluidManager().getThermalCond(flId, ThCondVal);
ThCondValAll.push_back(ThCondVal);
}
m_model.clearTable("FltThCondIoTbl");
std::vector<std::string> TherCondTypeUsed = ThermCondtypeReferred();

for (size_t ThCondType = 0; ThCondType < TherCondTypeUsed.size(); ThCondType++)
{
for (auto flId : ThCondId)
{
m_model.addRowToTable("FltThCondIoTbl");
}
}

auto ThCondIdUpdated = m_model.fluidManager().getFluidThCondID();
size_t thCondIdCount = 0;

for (size_t ThCondType = 0; ThCondType < TherCondTypeUsed.size(); ThCondType++)
{
int count = -1;
for (size_t id = 0; id < ThCondId.size(); id++)
{
count++;

m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "Fluidtype", TherCondTypeUsed[ThCondType]);
m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "TempIndex", TempIndexValuesAll[count]);
m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "Pressure", PressureValAll[count]);
m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "ThCond", ThCondValAll[count]);

thCondIdCount++;
}
LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << " FltThCondIoTbl has been updated to include the property values for " << TherCondTypeUsed[ThCondType];
}

}

void Prograde::BrineUpgradeManager::ResetFltHeatCapIoTbl()
{
auto HeatCapId = m_model.fluidManager().getFluidHeatCapID();
double HCTemperatureVal, HCPressVal, HeatCapVal;
std::vector<double> HCTempIndexValuesAll, HCPressureValAll, HeatCapValAll;
for (auto flId : HeatCapId) {
m_model.fluidManager().getHeatCapTblTempIndex(flId, HCTemperatureVal);
HCTempIndexValuesAll.push_back(HCTemperatureVal);
m_model.fluidManager().getHeatCapTblPressure(flId, HCPressVal);
HCPressureValAll.push_back(HCPressVal);
m_model.fluidManager().getHeatCap(flId, HeatCapVal);
HeatCapValAll.push_back(HeatCapVal);

}
m_model.clearTable("FltHeatCapIoTbl");
std::vector<std::string> HeatCapTypeUsed = ThermCondtypeReferred();//HeatCapType and ThCondType values are same

for (size_t ThCondType = 0; ThCondType < HeatCapTypeUsed.size(); ThCondType++)
{
for (auto flId : HeatCapId)
{
m_model.addRowToTable("FltHeatCapIoTbl");
}
}
size_t heatCapIdCount = 0;

for (size_t HeatCapType = 0; HeatCapType < HeatCapTypeUsed.size(); HeatCapType++)
{
int rowCount = -1;
for (size_t id = 0; id < HeatCapId.size(); id++)
{
rowCount++;
m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "Fluidtype", HeatCapTypeUsed[HeatCapType]);
m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "TempIndex", HCTempIndexValuesAll[rowCount]);
m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "Pressure", HCPressureValAll[rowCount]);
m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "HeatCapacity", HeatCapValAll[rowCount]);

heatCapIdCount++;
}
LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << " FltHeatCapIoTbl has been updated to include the property values for " << HeatCapTypeUsed[HeatCapType];
}

}
*/
