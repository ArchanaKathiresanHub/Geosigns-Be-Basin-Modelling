//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// to do: 
// 1) add erosion
// 2) add missing twt maps 

#ifndef FASTDEPTHCALIBRATION_H
#define FASTDEPTHCALIBRATION_H

// STL
#include <string>
#include <vector>
#include <map>

// cmbAPI and utilities
#include "cmbAPI.h"
#include "FilePath.h"
#include "FolderPath.h"
#include "ErrorHandler.h"

typedef formattingexception::GeneralException T2Zexception;

class FastDepthCalibration : public ErrorHandler
{
public:

   /// @brief Constructor 
   /// @param projectFileName the name of the project file
   /// @param referenceSurface the number of the the reference surface (where the depth are assumed to be reliable, typically the sea floor)
   /// @param endSurface the final surface to calibrate
   /// @param argc the number of command line parameters
   /// @param argv the values of the command line parameters
   /// @param rank the current rank
   FastDepthCalibration( char * projectFileName, int referenceSurface, int endSurface, int argc, char** argv, int rank );
   
   /// @brief Destructor
   ~FastDepthCalibration();

   /// @brief Get the twt maps and the reference depth maps. Calculate the initial isoPacks or erosion maps
   void calculateInitialMaps();
      
   /// @brief The calibration loop
   void calibrateDepths();

   /// @brief Write the final project and calibrated input files into the s_finalResultsFolder
   void writeFinalProject();

private:

   /// Disable the default constructor
   FastDepthCalibration();

   /// @brief Run fastcauldron and truncate the depth to "snap" the the next depth to the provided twt
   /// @param layerName the layer which bottom depth nedds to be calibrated
   /// @param refDepths the depth of the top surface
   /// @param tarTwt the target twt to match
   /// @param newDepths the calibrated depth
   void runFastCauldron( const std::string         & layerName,
                         const std::vector<double> & refDepths,
                         const std::vector<double> & tarTwt,
                         std::vector<double>       & newDepths );

   /// @brief Clean specific project tables before FastDepthCalibration
   void prepareProject( );

   /// @brief Modify the StratIoTbl and GridMapIoTbl tables for the next calibration loop 
   void modifyTables( const mbapi::StratigraphyManager::SurfaceID nextSurface );

   /// @brief Fill in the vector v with the values of the GridMap grid at one specific depth level k
   /// @param grid the GridMap
   /// @param v the vector to fill
   /// @param k the depth level
   /// @param convFact a user defined multiplayer (e.g. 1 for depths, 0.001 for twt)
   /// @return ErrorHandler::NoError on success, error code otherwise
   void fillArray( const DataAccess::Interface::GridMap * grid, 
                   std::vector<double> & v, 
                   int k, 
                   const double convFact );

   /// @brief Create a new folder casePath for the caseProject with casePathResults (the input maps BEFORE the calibration)
   /// @param casePath the ibs::FolderPath for the new case
   /// @param caseProject the ibs::FilePath for the new case filename
   /// @param casePathResults the ibs::FilePath destination of the input maps
   /// @return ErrorHandler::NoError on success, error code otherwise
   void createCase( ibs::FolderPath & casePath,
                    ibs::FilePath   & caseProject,
                    ibs::FilePath   & casePathResults );

   // Here we could handle the case of missing TWT Maps before starting the calibration loop. It should simply fill the m_twtMaps data member 
   // void calculateMissingTWTSurfaces( );

   // Here we could handle erosion before starting the calibration loop. Another m_erosionMaps data member should be created and filled up. 
   // This additional member can be used to generate the final depth maps in writeFinalProject() function 
   // void calculateErosionSurfaces( );
   
   // Data structures that belongs to the class instance
   std::string                                                          m_projectFileName;
   const mbapi::StratigraphyManager::SurfaceID                          m_referenceSurface;
   const mbapi::StratigraphyManager::SurfaceID                          m_endSurface;
   std::unique_ptr<mbapi::Model>                                        m_mdl;
   std::map<mbapi::StratigraphyManager::SurfaceID, std::vector<double>> m_isoPacks;
   std::map<mbapi::StratigraphyManager::SurfaceID, std::string>         m_twtMaps;

   // Folder paths
   ibs::FolderPath                                                      m_fullMasterPath;
   ibs::FolderPath                                                      m_finalResultsPath;

   // File paths
   ibs::FilePath                                                        m_masterResults;
   ibs::FilePath                                                        m_finalResults;

   // The command line parameters to pass to fastcauldron
   int                                                                  m_argc;
   char**                                                               m_argv;

   // The current rank
   int                                                                  m_rank;

   // The final depth of the end surface (needed to append the isopacks)
   std::vector<double>                                                  m_depthsEndSurface; 

   // Data structures for the calibrated maps
   std::map<mbapi::StratigraphyManager::SurfaceID, std::string>         m_correctedMapsNames;
   std::map<mbapi::StratigraphyManager::SurfaceID, int >                m_correctedMapsSequenceNbr;
   std::map<mbapi::StratigraphyManager::SurfaceID, int >                m_correctedMapsIDs;

};

#endif // FASTDEPTHCALIBRATION_H
