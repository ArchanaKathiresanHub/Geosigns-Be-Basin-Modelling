//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FASTCTC_CRUSTALTHICKNESS_CALCULATOR_H
#define _FASTCTC_CRUSTALTHICKNESS_CALCULATOR_H

// CrustalThickness library
#include "InterfaceInput.h"
#include "InterfaceOutput.h"
#include "DensityCalculator.h"

// DataAccess library
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

// GeoPhysics library
#include "GeoPhysicsProjectHandle.h"
#include "Local2DArray.h"

// utilitites
#include "FormattingException.h"

using namespace DataAccess;
using namespace std;

/// @brief Displays the time and the message on the console.
/// @details This functions uses MPI communication via the PetscPrintf command
/// @param timeToDisplay The time to display in seconds
/// @param msgToDisplay The message to display
void displayTime( const double timeToDisplay, const char * msgToDisplay );

/// @class CrustalThicknessCalculator The main class used to runn the CTC (CrustalThicknessCalculator)
class CrustalThicknessCalculator : public GeoPhysics::ProjectHandle {

   typedef formattingexception::GeneralException CtcException;

public :
   // Constructor / Destructor
   CrustalThicknessCalculator( database::Database * database, const std::string & name, const std::string & accessMode, ObjectFactory* objectFactory );

   ~CrustalThicknessCalculator();

   /// @brief Return the reference to the project data
   static CrustalThicknessCalculator& getInstance ();

   /// @brief Open the project file
   /// @param inputFileName The file name of the project file such as project.project3d
   /// @ param factory The object factory
   static CrustalThicknessCalculator* CreateFrom( const string& inputFileName, ObjectFactory* factory );

   /// @brief Finish any activity and deallocate the singleton object
   /// @param saveResults Specify if the results must be saved in HDF file or not
   static void finalise ( const bool saveResults );

   /// @brief Parse fasctc command line
   /// @details Three command line parameters are available:
   ///    -# -xyz saves the outputs as xyz files
   ///    -# -sur saves the outputs as surface files
   ///    -# -hdf saves the outputs as hdf files
   ///    -# -debug runs the CTC in debug mode (includes debug outputs)
   ///    -# -save <FileName> saves the CTC input file in FileName
   bool parseCommandLine();

   /// @brief Run the CTC
   void run();

   /// @brief Delete property values associated with crustal thickness calculator
   void deleteCTCPropertyValues();

   /// @brief Merge output files if nessecary

   /// @details Must be called at end of calculation.
   bool mergeOutputFiles();
private :

   static CrustalThicknessCalculator* m_crustalThicknessCalculator; ///< The CrustalThicknessCalculator instance

   static string m_projectFileName;   ///< The input project file name, and output project file name by default
   static string m_outputFileName;    ///< The output project file name, specified via command line under -save

   LinearFunction m_LF;                     ///< The linear function object used by the CTC
   DensityCalculator m_DensityCalculator;   ///< The density calculator used by the CTC

   int    m_outputOptions;    ///< The output option is the combination the output options defined in the command line (xyz, sur, hdf)
   bool   m_debug;            ///< Run the CTC in debug mode
   bool   m_applySmoothing;   ///< Smooth the WLS map
   int    m_smoothRadius;     ///< The smoothing radius defined in the project file under HaflFilterWidth

   /// @brief Set requested output properties from the Project file
   void setRequestedOutputProperties( InterfaceOutput & theOutput);

   /// @brief Smooth the map according to the smoothing radius
   /// @param aMap The map to smooth
   bool movingAverageSmoothing( GridMap * aMap );

   /// @brief Parse additional informations from command lines:
   ///    -# nosmooth turns smoothing off
   ///    -# smooth <SmoothingRadius> turns smoothing on and set the smoothing radius
   void setAdditionalOptionsFromCommandLine();

   /// @brief Update geophysics ProjectHandle valid nodes using the CTC input maps
   void updateValidNodes( const InterfaceInput* theInterfaceData );
   /// @brief Computes the present day water loaded subsidence (WLS) map
   /// @return The WLS map
   GridMap * calculatePresentDayWLS( InterfaceInput* theInterfaceData );

};

//------------------------------------------------------------//

inline CrustalThicknessCalculator& CrustalThicknessCalculator::getInstance () {
   return *m_crustalThicknessCalculator;
}
#endif
