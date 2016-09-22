//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTCAULDRON__FASTCAULDRON_STARTUP__H
#define FASTCAULDRON__FASTCAULDRON_STARTUP__H

#include "propinterface.h"
#include "FastcauldronFactory.h"

#include <string>

#ifdef FLEXLM
#undef FLEXLM
#endif

#ifdef DISABLE_FLEXLM
#undef FLEXLM
#else
#define FLEXLM 1
#endif

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

/// @brief Contains the sequence of operations that are required to initialise, run and finilize fastcauldron.
class FastcauldronStartup
{
public :

   FastcauldronStartup( int argc, char** argv, bool checkLicense = true, bool saveResults = true );
   ~FastcauldronStartup();
   void run();
   bool getPrepareStatus()  { return m_prepareOk; };
   bool getStartUpStatus( ) { return m_startUpOk; };
   bool getRunStatus()      { return m_runOk; };

   // here we delete m_cauldron, m_factory. We do not want the destructor to do this job, because can be after PetscFinalize.
   void finalize();         

private:

   // Disable default constructor (arguments must be provided to instantiate FastcauldronStartup) 
   FastcauldronStartup();

   /// @brief Setup FlexLM license if FlexLM library is available
   /// @return true on success false on error
   bool prepare();

   /// @brief Create fastcauldron simulator, create the lithologies, initialise layer thickness history
   /// @param argc number of command line parameters 
   /// @param argv command line parameters values
   /// @param saveAsInputGrid 
   /// @param createResultsFile
   /// @return true on success false on error
   bool startup( int argc, char** argv, const bool saveAsInputGrid = false, const bool createResultsFile = true );

   /// @brief determines if salt modelling can be used
   /// @return true if salt modelling can be used, false otherwise
   bool determineSaltModellingCapability( );

   /// @brief Determines the current rank
   /// @return the current rank
   int ourRank( );

   /// How many instances of FastcauldronStartup are present.
   static unsigned int  s_instances;

   AppCtx * m_cauldron;              // propinterface
   FastcauldronFactory* m_factory;   // fastcauldronFactory
   std::string m_errorMessage;       
   bool m_solverHasConverged;       
   bool m_errorInDarcy;
   bool m_geometryHasConverged;
   bool m_checkLicense;
   bool m_prepareOk;                // flag to indicate if the license was checked out correctly
   bool m_startUpOk;                // flag to indicate if startup step (creating factories and other instances) was correct
   bool m_runOk;                    // flag to indicate if the run was successful
   bool m_canRunSaltModelling;      // flag to indicate if we can run salt modelling
   bool m_saveResults;              // flag to indicate if we can save the results

#ifdef FLEXLM
   char m_feauture[EPTFLEXLM_MAX_FEATURE_LEN];
#else
   char m_feauture[256];            // flexlm feature
#endif                     
};

#endif // FASTCAULDRON__FASTCAULDRON_STARTUP__H