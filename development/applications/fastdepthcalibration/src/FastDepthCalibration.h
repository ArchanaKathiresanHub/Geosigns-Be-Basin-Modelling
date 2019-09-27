//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FastDepthCalibration class handles the main depth calibration algorithms and
// continas the input and calibration data, as well as the manager objects.

// to do (bring it on the prototype status):
// 1) add erosion
// 2) add missing twt maps -> Done! (testing necessary)
// 3) add super fast depth calibration with subsampling! 
#pragma once

#include "CaseStorageManager.h"
#include "FDCFastCauldronManager.h"
#include "FDCLithoProperties.h"
#include "FDCMapFieldProperties.h"
#include "FDCProjectManager.h"
#include "FDCVectorFieldProperties.h"
#include "FinalCaseManager.h"

// cmbAPI and utilities
#include "cmbAPI.h"
#include "ErrorHandler.h"

// STL
#include <map>
#include <string>
#include <vector>

namespace fastDepthCalibration
{

class FastDepthCalibration : public ErrorHandler
{
public:

   /// @brief Constructor
   /// @param projectFileName the name of the project file
   /// @param referenceSurface the number of the the reference surface (where the depth are assumed to be reliable, typically the sea floor)
   /// @param endSurface the final surface to calibrate
   /// @param noCalculatedTWToutput flag to not export the calculated TWT maps in the final result
   /// @param preserveErosionFlag flag to preserve erosion
   /// @param argc the number of command line parameters
   /// @param argv the values of the command line parameters
   /// @param rank the current rank
   explicit FastDepthCalibration(char * projectFileName, int referenceSurface, int endSurface,
                                 bool noCalculatedTWToutput, bool preserveErosionFlag, bool noExtrapolationFlag,
                                 int argc, char** argv, int rank);

   /// @brief Destructor
   ~FastDepthCalibration();

   /// @brief run the fast depth calibration
   void run();

private:
   /// Disable the default constructor
   FastDepthCalibration();

   void reloadModel();

   void checkReferenceAndEndSurfaceBoundsAreValid() const;

   /// @brief The calibration loop
   void calibrateDepths();

   void setDepthAndTwtMapsForNextSurfaceInTables(std::string & mapName, const bool generated_Twt, const std::vector<double> & increasedDepths);

   void runFastCauldronAndCalculateNewDpeths();

   void writeNewDepthAndCorrectedMapstoCaseFileInMasterDirectory(const string & mapName);

   /// @brief Get new depths after twtGridMap and depthGridMap were obtained from fastcauldron runs
   /// @param twtGridMap
   /// @param depthGridMap the depth grid map
   void calculateNewDepths(const Interface::GridMap * twtGridMap,
                           const Interface::GridMap * depthGridMap);

   /// @brief Fill in the vector v with the values of the GridMap grid at one specific depth level k
   /// @param grid the GridMap
   /// @param v the vector to fill
   /// @param k the depth level
   /// @param convFact a user defined multiplayer (e.g. 1 for depths, 0.001 for twt)
   void fillArray( const DataAccess::Interface::GridMap * grid,
                   std::vector<double> & v,
                   int k,
                   const double convFact );

   // Here we could handle erosion before starting the calibration loop. Another m_erosionMaps data member should be created and filled up.
   std::vector<double> calculateErosion(const mbapi::StratigraphyManager::SurfaceID surfID) const;

   void prepareModelToRunAutomaticDepthCalibration();
   bool checkForHiatus(const mbapi::StratigraphyManager::SurfaceID currentSurface);
   bool setTwtMapsIfMissingInBottomSurface();

   const mbapi::StratigraphyManager::SurfaceID m_referenceSurface;
   const mbapi::StratigraphyManager::SurfaceID m_endSurface;
   const bool                                  m_noCalculatedTWToutput;
   const bool                                  m_preserveErosionFlag;
   const bool                                  m_noExtrapolationFlag;
   int                                         m_argc;
   char**                                      m_argv;
   int                                         m_rank;
   std::vector<double>                         m_depthsEndSurface;

   FDCProjectManager             m_fdcProjectManager;
   std::shared_ptr<mbapi::Model> m_mdl;
   FDCMapFieldProperties         m_fdcMapFieldProperties;
   CaseStorageManager            m_caseStorageManager;

   FDCVectorFieldProperties m_fdcVectorFieldProperties;
   FDCLithoProperties       m_fdcLithoProperties;
};

} // namespace fastDepthCalibration
