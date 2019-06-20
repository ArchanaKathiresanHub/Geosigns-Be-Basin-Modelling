//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTCTC_CRUSTALTHICKNESS_CALCULATOR_H
#define FASTCTC_CRUSTALTHICKNESS_CALCULATOR_H

// TableIO library
#include "ProjectFileHandler.h"

// CrustalThickness library
#include "InterfaceInput.h"
#include "InterfaceOutput.h"

// DataAccess library
#include "Interface.h"

// DataMining library
#include "DataMiningProjectHandle.h"

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
class CrustalThicknessCalculator : public DataAccess::Mining::ProjectHandle {

   typedef formattingexception::GeneralException CtcException;

public :
   CrustalThicknessCalculator( const database::ProjectFileHandlerPtr database,
                               const std::string & name,
                               const std::string & accessMode,
                               const DataAccess::Interface::ObjectFactory* objectFactory );
   ~CrustalThicknessCalculator() = default;

   /// @brief Return the reference to the project data
   static CrustalThicknessCalculator& getInstance ();

   /// @brief Open the project file
   /// @param inputFileName The file name of the project file such as project.project3d
   /// @ param factory The object factory
   static CrustalThicknessCalculator* createFrom( const string& inputFileName, DataAccess::Interface::ObjectFactory* factory );

   /// @brief Initialise the CTC instance, projecthandle, interface input and interface output
   void initialiseCTC();

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

   int    m_outputOptions;    ///< The output option is the combination the output options defined in the command line (xyz, sur, hdf)
   bool   m_debug;            ///< Run the CTC in debug mode
   bool   m_applySmoothing;   ///< Smooth the WLS map

   std::shared_ptr<InterfaceInput>  m_inputData;  ///< Interface for input data (user inputs and configuration file)
   InterfaceOutput  m_outputData;                 ///< Interface for output data (maps)

   /// @defgroup PreviousIterationData
   /// @{
   GridMap* m_previousTTS;                         ///< The Total Tectonic Subsidence of the previous iteration (i.e. we are at snapshot 10Ma, the previous iteration was at 15Ma)
   GridMap* m_previousRiftTTS;                     ///< The Total Tectonic Subsidence of the previous rift (i.e. we are at rift ID 2, the previous iteration was at rift ID 1)
   GridMap* m_previousContinentalCrustalThickness; ///< The continental crustal thickness of the previous iteration (i.e. we are at snapshot 10Ma, the previous iteration was at 15Ma)
   GridMap* m_previousOceanicCrustalThickness;     ///< The oceanic crustal thickness of the previous iteration (i.e. we are at snapshot 10Ma, the previous iteration was at 15Ma)
   /// @}

   /// @brief Set requested output properties from the Project file
   void setRequestedOutputProperties( InterfaceOutput & theOutput) const;

   /// @brief Smooth the TTS and PWD maps according to the user defined smoothing radius
   void smoothOutputs();

   /// @brief Parse additional informations from command lines:
   ///    -# nosmooth turns smoothing off
   ///    -# smooth <SmoothingRadius> turns smoothing on and set the smoothing radius
   void setAdditionalOptionsFromCommandLine();

   /// @brief Update geophysics ProjectHandle valid nodes (area of interest) using the CTC input maps
   /// @param[in] age The snapshot at which we define the area of interest
   void updateValidNodes( const double age );

   /// @brief Retrieve the grid map data
   void retrieveData();

   /// @brief Restore the grid map data
   void restoreData();
};

//------------------------------------------------------------//

inline CrustalThicknessCalculator& CrustalThicknessCalculator::getInstance () {
   return *m_crustalThicknessCalculator;
}
#endif
