//
// Copyright (C) 2018-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BasicCrustThinningUpgradeManager.h"
#include "MapsManagerImpl.h"
//std
#include <vector>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"
//Prograde
#include "BasicCrustThinningModelConverter.h"

//cmbAPI
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "CrustFormation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "RunParameters.h"
#include "ContinentalCrustHistoryGenerator.h"
using namespace mbapi;
// The new interpolated maps are required to be in a new Layer of HDF file
constexpr char InterpMapName[] = "interpolatedMap_";

//------------------------------------------------------------//

Prograde::BasicCrustThinningUpgradeManager::BasicCrustThinningUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("basic crustal thinning model upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::BasicCrustThinningUpgradeManager::upgrade() {
   Prograde::BasicCrustThinningModelConverter modelConverter;
  
   BottomBoundaryManager::BottomBoundaryModel botBoundModel;

   m_model.bottomBoundaryManager().getBottomBoundaryModel(botBoundModel);

   if(botBoundModel== mbapi::BottomBoundaryManager::BottomBoundaryModel::BasicCrustThinning)
   {
       //upgrading the bottom boundary model
       m_model.bottomBoundaryManager().setBottomBoundaryModel(modelConverter.upgradeBotBoundModel(botBoundModel));
       (LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating  BasementIoTbl");
      //upgrading the crust property model to standard conductivity crust
      BottomBoundaryManager::CrustPropertyModel crstPropModel;
      m_model.bottomBoundaryManager().getCrustPropertyModel(crstPropModel);
      m_model.bottomBoundaryManager().setCrustPropertyModel(modelConverter.upgradeCrustPropModel(crstPropModel));

      //upgrading the mantle property model to high conductivity mantle model
      BottomBoundaryManager::MantlePropertyModel mntlPropModel;
      m_model.bottomBoundaryManager().getMantlePropertyModel(mntlPropModel);
      m_model.bottomBoundaryManager().setMantlePropertyModel(modelConverter.upgradeMantlePropModel(mntlPropModel));
      mbapi::MapsManager& mapsMgrLocal = m_model.mapsManager();

      //setting initial lithospheric mantle thickness in BasementIoTbl, to default value, 115000
      double InitLithMnThickness = 115000.0;
      m_model.bottomBoundaryManager().setInitialLithoMantleThicknessValue(InitLithMnThickness);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Initial lithospheric mantle thickness value is set to the default value of 115000";
      
      //setting initial LithoMantleThickness,InitialLthMntThickns & FixedCrustThickness in BasementIoTbl, to default value 0.0
      double legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "LithoMantleThickness");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
          << "setting LithoMantleThickness from, " << legayVal << ", to 0.0";
      auto err = m_model.setTableValue("BasementIoTbl", 0, "LithoMantleThickness", 0.0);

      legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "InitialLthMntThickns");
      err = m_model.setTableValue("BasementIoTbl", 0, "InitialLthMntThickns", 0.0);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
          << "setting InitialLthMntThickns from, " << legayVal << ", to 0.0";

      legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "FixedCrustThickness");
      err = m_model.setTableValue("BasementIoTbl", 0, "FixedCrustThickness", 0.0);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
          << "setting FixedCrustThickness from, " << legayVal << ", to 0.0";
      if (ErrorHandler::NoError != err)
          throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();

      // checking CrustHeatPDecayConst in BasementIoTbl for its default value of BPA2=10,000
      double CrustHeatPDecayConst = m_model.tableValueAsDouble("BasementIoTbl", 0, "CrustHeatPDecayConst");
      if (!NumericFunctions::isEqual(CrustHeatPDecayConst, 10000., 1e-4))
          (LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: CrustHeatPDecayConst-" << CrustHeatPDecayConst << " is not equal 10000");

      // checking TopAsthenoTemp in BasementIoTbl for its default value of BPA2=1333 oC
      auto TopAsthenoTemp = m_model.tableValueAsDouble("BasementIoTbl", 0, "TopAsthenoTemp");
      if (!NumericFunctions::isEqual(TopAsthenoTemp, 1333.0, 1e-4))
          (LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: TopAsthenoTemp-" << TopAsthenoTemp << " is not 1333 degrees centegrade ");
      /*Copy the value as is if the value lies within[0, 1000].Else reset its value to the nearest limiting value.If map is defined
          then copy the map name as is but put a check if the map is having out - of - range value or not.If the map
          contains out - of - range values then update the prograde log file.*/
          
          /* check for the acceptable limits of[0, 1000] for TopCrustHeatProd values*/
      auto TopCrustHeatProd = m_model.tableValueAsDouble("BasementIoTbl", 0, "TopCrustHeatProd");
      if (!NumericFunctions::inRange(TopCrustHeatProd, 0.0, 1000.0)) {
          LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TopCrustHeatProd-" << TopCrustHeatProd << " out of range [0, 1000], resetting to nearest limit! ";
          TopCrustHeatProd = NumericFunctions::clipValueToRange(TopCrustHeatProd, 0.0, 1000.0);
          m_model.setTableValue("BasementIoTbl", 0, "TopCrustHeatProd", TopCrustHeatProd);
      }

      auto TopCrustHeatProdGrid = m_model.tableValueAsString("BasementIoTbl", 0, "TopCrustHeatProdGrid");

      
      if (TopCrustHeatProdGrid.compare(DataAccess::Interface::NullString)) {
          auto mi = mapsMgrLocal.findID(TopCrustHeatProdGrid);
          double oldMin;
          double oldMax;
          if (ErrorHandler::NoError != mapsMgrLocal.mapValuesRange(mi, oldMin, oldMax))
              throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();
          if (!(NumericFunctions::inRange(oldMin, 0.0, 1000.0) && NumericFunctions::inRange(oldMax, 0.0, 1000.0)))
              LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << TopCrustHeatProdGrid + " map OUT of Range [0, 1000] ";
          
         
          LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMap found in TopCrustHeatProdGrid, " <<
              "resetting the 'TopCrustHeatProd': " <<TopCrustHeatProd<<" value to -9999";
          m_model.setTableValue("BasementIoTbl", 0, "TopCrustHeatProd", DataAccess::Interface::DefaultUndefinedScalarValue);
      }


      cleanContCrustIoTbl();

      auto BasinAge = m_ph->getCrustFormation()->getTopSurface()->getSnapshot()->getTime();
      BasinAge = BasinAge > 999.0 ? 999.0 : BasinAge;
      /*
        \\ For OceaCrustalThicknessIoTbl
            1.  Must have to be specified for all the major system defined snapsot ages.
                This is needed because fastcauldron simulator does not interpolate OceaCrustal maps for major system defined snapshots
                from the given inputs, unlike with Cont. CrustalThickness maps
            2.  The Thickness is always 0.
        \\
      */
      auto SnapshotList = m_ph->getSnapshots();
      std::vector<double> agesForOceaTbl(0);
      for (const auto snapshot : *SnapshotList)
      {
          auto SnpAge = snapshot->getTime();
          if ((SnpAge < BasinAge || abs(SnpAge - BasinAge) < 1e-6) && (snapshot->getKind() == "System Generated"))
          {
              agesForOceaTbl.push_back(SnpAge);
          }
      }
      delete SnapshotList;
      
      double age=0.0;
      bool doInterpolate = false;
      double thickness{};
      std::string thicknessGrid{}, TableName{};
      std::pair<double, std::string> lastAgeMapNamePair;
      std::pair<double, double> lastAgeThicknessPair;
      lastAgeMapNamePair =  std::make_pair(0,"");
      lastAgeThicknessPair = std::make_pair(0.0, 0.0);
      size_t index = 0;
      auto timesteps = m_model.bottomBoundaryManager().getTimeStepsID();

      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating Cont. CrustalThickness";

      for (auto tsId : timesteps) 
      {
          m_model.addRowToTable("ContCrustalThicknessIoTbl");
          // copying Age,thickness and thickness maps corresponding to a particular time step ID from CrustIoTbl to ContCrustalThicknessIoTbl
          m_model.bottomBoundaryManager().getAge(tsId, age); 
          // Checking if an age from ContCrustalThicknessIoTbl is missing from OceaCrustalThicknessIoTbl's list
          auto aCTage = std::find(agesForOceaTbl.begin(), agesForOceaTbl.end(), age);
          // if missing put in this age
          if (aCTage == std::end(agesForOceaTbl) && (age < BasinAge || abs(age - BasinAge) < 1e-6))
              agesForOceaTbl.push_back(age);
          // ================== Scalar ===============//
          m_model.bottomBoundaryManager().getThickness(tsId, thickness);
          if (thickness != DataAccess::Interface::DefaultUndefinedScalarValue) {
              /* check for the acceptable limits of[0, 6300000] for crustal thickness scalar */
              if (!NumericFunctions::inRange(thickness, 0.0, 6300000.0)) {
                  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << thickness << " OUT of Range [0, 6300000] ";
                  thickness = NumericFunctions::clipValueToRange(thickness, 0.0, 6300000.0);
              }
          }
          // ================= Grid ==================//
          m_model.bottomBoundaryManager().getCrustThicknessGrid(tsId, thicknessGrid);
          // if gripMap
          if (thicknessGrid.compare(DataAccess::Interface::NullString)) 
          {
              auto mi = mapsMgrLocal.findID(thicknessGrid);
              /* check for the acceptable limits of[0, 6300000] for crustal thickness grid*/
              double oldMin;
              double oldMax;
              if (ErrorHandler::NoError != mapsMgrLocal.mapValuesRange(mi, oldMin, oldMax))
                  throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();
              if (! (NumericFunctions::inRange(oldMin, 0.0, 6300000.0)
                        && NumericFunctions::inRange(oldMax, 0.0, 6300000.0)
                    )
                  )
                  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << thicknessGrid + " map OUT of Range [0, 6300000] ";
              // if a map is provided set Scalar to undefined value
              double ageToshow = age;
              if (!(age < BasinAge)) {
                  ageToshow = BasinAge;
                  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "For age: " << ageToshow << " Ma, setting thickness from " << thickness << " to -9999 since a thickness map is defined";
              }
              thickness = DataAccess::Interface::DefaultUndefinedScalarValue;
          }
          
          // Now, that we have both thickness & thicknessGrid... check if interpolation is required
          /* Thumb rule is that cont. crustal thickness must be defined at basement age
           if age > Basin age set cont. crust age to basin age and Interpolate thickness/thicknessGrid*/
          ((age < BasinAge) || (abs(age - BasinAge) < 1e-6)) ? false :(doInterpolate = true);        
          ++index;
          if (doInterpolate) 
          {
              double coeff = 0.0;
              // Scalar-Scalar interpolation
              /* Interpolate the Scalar Value from given data at this age to that at the Basin age*/
              if (thickness != DataAccess::Interface::DefaultUndefinedScalarValue 
                && lastAgeThicknessPair.second != DataAccess::Interface::DefaultUndefinedScalarValue // else its a map interpolation 
                ) 
              {
                  double thePoints[6] = { age,thickness,lastAgeMapNamePair.first,lastAgeThicknessPair.second,BasinAge, 0.0 };
                  // coeff = (BasinAge - lastAgeThicknessPair.first) / (age - lastAgeThicknessPair.first);
                  // thickness = lastAgeThicknessPair.second + (thickness - lastAgeThicknessPair.second) * coeff;
                  if (InterpolateIntermidiateValues(0, 0, 0, thePoints) != ErrorHandler::NoError)
                      throw ErrorHandler::Exception(ErrorHandler::OutOfRangeValue) << "thicknessGrid interpolation failed for ContCrustalThicknessIoTbl ";
                  thickness = thePoints[5];
                  // thicknessGrid = DataAccess::Interface::NullString; is this required?
              }
              else 
              {
                  // current age gridMap enrty is present
                  /* Interpolate the map from given data at this age to that at the Basin age*/
                  long N{}, M{};    
                  m_model.hiresGridArealSize(N, M);   
                  auto s = N * M; std::vector<double> vdata(s);
                  size_t mapId{}, mapIdmin{}, mapIdToSave{};  const auto refferedTable = "ContCrustalThicknessIoTbl"; 
                  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
                  if (!thicknessGrid.compare(DataAccess::Interface::NullString)) 
                  {
                      // scalar-map interpolation
                      // if this map is NULL then the scalar value MUST exist
                      std::fill(vdata.begin(), vdata.end(), thickness);
                      // lastAgeMapNamePair.second can not be null => both is null and its a Scalar-Scalar interpolation, that's handled
                      // So, create the thicknessGrid with scalar thickness like so... 
                      mapId = mapsMgrLocal.generateMap(refferedTable, InterpMapName + to_string(BasinAge), vdata, mapsSequenceNbr, "", false);
                      mapIdmin = mapsMgrLocal.findID(lastAgeMapNamePair.second);
                      mapIdToSave = mapId;
                  }
                  else if (!lastAgeMapNamePair.second.compare(DataAccess::Interface::NullString)) 
                  {
                      // last age gridMap enrty is present
                      // map-scalar interpolation
                      // if the last map is NULL then the scalar value MUST exist
                      std::fill(vdata.begin(), vdata.end(), lastAgeThicknessPair.second);
                      // present thicknessGrid can not be null => both is null and its a Scalar-Scalar interpolation, that's handled
                      // So, create the thicknessGrid with last age scalar thickness like so... 
                      mapIdmin = mapsMgrLocal.generateMap(refferedTable, InterpMapName + to_string(BasinAge), vdata, mapsSequenceNbr, "", false);
                      mapId = mapsMgrLocal.findID(thicknessGrid);
                      mapIdToSave = mapIdmin;
                  }
                  else 
                  {
                      // map-map interpolation
                      mapId = mapsMgrLocal.findID(thicknessGrid);
                      mapIdmin = mapsMgrLocal.findID(lastAgeMapNamePair.second);
                      mapIdToSave = mapsMgrLocal.generateMap(refferedTable, InterpMapName + to_string(BasinAge), vdata, mapsSequenceNbr, "Inputs.HDF", false);
                  }
                  // if valid maps with the IDs are created or found
                  if (!IsValueUndefined(mapId) && !IsValueUndefined(mapIdmin)) 
                  {
                      thicknessGrid = InterpMapName + to_string(BasinAge);
                      double thePoints[6] = { age,0.0,lastAgeMapNamePair.first,0.0,BasinAge,0.0 };
                      
                      if (InterpolateIntermidiateValues(mapIdToSave, mapIdmin, mapId,thePoints, &mapsMgrLocal)!= ErrorHandler::NoError)
                          throw ErrorHandler::Exception(ErrorHandler::OutOfRangeValue) << "thicknessGrid interpolation failed for ContCrustalThicknessIoTbl ";
                      // Save new map to file
                      if (ErrorHandler::NoError != mapsMgrLocal.saveMapToHDF(mapIdToSave, "Inputs.HDF"))
                          throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();
                  }
                  else
                      throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "thicknessGrid map not valid in ContCrustalThicknessIoTbl ";
              }
              // set the age to Basin age and the intepolated values/maps at basin age
              m_model.bottomBoundaryManager().setContCrustAge(tsId, BasinAge);
              m_model.bottomBoundaryManager().setContCrustThickness(tsId, thickness);
              m_model.bottomBoundaryManager().setContCrustThicknessGrid(tsId, thicknessGrid); 
              if (thickness != DataAccess::Interface::DefaultUndefinedScalarValue )
              {
                  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
                      << "For age: " << BasinAge << " Ma, the Continental crust thickness was interpolated to " << thickness;
              }
              else {
                  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
                      << "For age: " << BasinAge << " Ma, the Continental crust thickness was interpolated to Grip map " << thicknessGrid;
              }
              // Consider no more ages in CrustIoTbl cause they are all > Basin Age hence on
              break;
          }
          lastAgeMapNamePair.first = age; lastAgeMapNamePair.second = thicknessGrid;
          lastAgeThicknessPair.first = age; lastAgeThicknessPair.second = thickness;
          m_model.bottomBoundaryManager().setContCrustAge(tsId, age);
          m_model.bottomBoundaryManager().setContCrustThickness(tsId, thickness);
          m_model.bottomBoundaryManager().setContCrustThicknessGrid(tsId, thicknessGrid);
      }

      // If the last age in CrustIoTbl is less that basin age add a line with basin age 
      // & no intepolation needed just put the value/map of this age to basin age
      if (age < BasinAge) {
          m_model.addRowToTable("ContCrustalThicknessIoTbl");  
          m_model.bottomBoundaryManager().setContCrustAge(index, BasinAge); 
          m_model.bottomBoundaryManager().getThickness(index, thickness);
          m_model.bottomBoundaryManager().setContCrustThickness(index, thickness);
          m_model.bottomBoundaryManager().getCrustThicknessGrid(index, thicknessGrid);
          m_model.bottomBoundaryManager().setContCrustThicknessGrid(index, thicknessGrid);
          LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
              << "For " << age << " Ma, which is less that basin age "<< BasinAge << " Ma, setting the same , Map/Value of" 
              << " continetal crust thickness, for Basin age";
      }
      /* Updating OceaCrustalThicknessIoTblwith all the major, system defined snapshot ages*/
      index = 0;
      std::sort(agesForOceaTbl.begin(), agesForOceaTbl.end());

      (LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating  Ocea. CrustalThickness");
      (LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Setting entries to zero for all ages");
      for (const auto OceaAge : agesForOceaTbl) {
          m_model.addRowToTable("OceaCrustalThicknessIoTbl");
          //setting thickness of OceaCrustalThicknessIoTbl to 0.0 for scalers
          m_model.bottomBoundaryManager().setOceaCrustAge(index, OceaAge);
          m_model.bottomBoundaryManager().setOceaCrustThickness(index, 0.0);
          // "" for grids for all these time step ID is set automatically at initialization of the Record in addRowToTable
          ++index;
      }
      
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CrustIoTbl is copied/interpolated to the ContCrustalThicknessIoTbl";
      cleanCrustIoTbl();
      cleanBasaltThicknessIoTbl();
      cleanMntlHeatFlowIoTbl();
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "CrustIoTbl is cleaned";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "BasaltThicknessIoTbl is cleaned";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "MntlHeatFlowIoTbl is cleaned";

      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating GridMapIoTbl";
      timesteps = m_model.bottomBoundaryManager().getGridMapTimeStepsID();
      int delCount = 0;
      // cleaning ReferredBy field from GridMapIoTbl for all deleted entries in BasementIoTbl
      for (auto tsId : timesteps)
      {
         std::string TableName;
         m_model.bottomBoundaryManager().getReferredBy(delCount, TableName);
         auto newName = modelConverter.upgradeGridMapTable(TableName);
         if(newName.compare(""))
            m_model.bottomBoundaryManager().setReferredBy(delCount, newName);
         else {
             m_model.removeRecordFromTable("GridMapIoTbl", delCount);
             LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMapIoTbl is cleaned of "<< TableName<< " gridmaps!";
             continue;
         }
         delCount++;
      }
   }
   else {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deprecated basic crust thinning model is not found";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed";
   }

   
}

//------------------------------------------------------------//

void Prograde::BasicCrustThinningUpgradeManager::cleanCrustIoTbl() const {
   m_model.clearTable("CrustIoTbl");
}
//------------------------------------------------------------//
void Prograde::BasicCrustThinningUpgradeManager::cleanContCrustIoTbl() const {
   m_model.clearTable("ContCrustalThicknessIoTbl");
}

void Prograde::BasicCrustThinningUpgradeManager::cleanBasaltThicknessIoTbl() const
{
    m_model.clearTable("BasaltThicknessIoTbl");
}

//   \
//    *(age1,val1 or currentMap) 
//     \
//      *(Given_age,val/Map=?)
//       \
//        *(age2,val2 or lastMap)
//          \                   0     1     2     3          4    5
//          agePoints[6] = {age1, val1, age2, val2, Given_age,  val}
ErrorHandler::ReturnCode Prograde::BasicCrustThinningUpgradeManager::InterpolateIntermidiateValues(size_t mapIdToSave, size_t mapIdmin, size_t mapId,
    double* ageValuePoints, mbapi::MapsManager* mngr)
{
    double coeff = (ageValuePoints[4] - ageValuePoints[2]) / (ageValuePoints[0] - ageValuePoints[2]);
    if (!mngr) {
        
        ageValuePoints[5] = ageValuePoints[3] + (ageValuePoints[1] - ageValuePoints[3]) * coeff;
        return(ErrorHandler::NoError);
    }
    else
    {
        return (mngr->interpolateMap(mapIdToSave, mapIdmin, mapId, coeff));
    }
}

void Prograde::BasicCrustThinningUpgradeManager::cleanMntlHeatFlowIoTbl() const
{
    m_model.clearTable("MntlHeatFlowIoTbl");
}
