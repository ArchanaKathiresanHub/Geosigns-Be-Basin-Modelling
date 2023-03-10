//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file CauldronApp.h
/// @brief This file keeps API declaration for dealing with Cauldron app executables

#ifndef CASA_API_CAULDRON_APP_H
#define CASA_API_CAULDRON_APP_H

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaSerializer.h"
#include "CasaDeserializer.h"

// FileSystem
#include "FilePath.h"
#include "FolderPath.h"

// STL
#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace casa
{
   /// @brief define application sequence number in applications pipeline
   typedef enum
   {
      PTSolver          = 0, ///< fastcauldron option -temperature/-overpressure/-itcoupled
      Genex             = 2, ///< fastgenex4/5/6
      HiResDecompaction = 3, ///< fastcauldron option -hrdecompaction [-coupled]
      Migration         = 4, ///< fastmig
      Postprocessing    = 6  ///< tracktraps/track1d and any other
   } AppPipelineLevel;


   /// @brief Class for creating command line for cauldron application
   class CauldronApp : public CasaSerializable
   {
   public:
      /// @brief Shell which will be used for script generation
      typedef enum
      {
         bash = 0,
         cmd
      } ShellType;

      /// @brief Constructor
      /// @param appName name of an application like fastcauldron/fastgenex6/...
      /// @param isParallel is this application will be run as a parallel app (requires mpirun in command line)
      CauldronApp(const std::string & appName, bool isParallel = true );

      /// @brief Destructor
      virtual ~CauldronApp() {}

      /// @brief Get position of this application in cauldron applications pipeline
      /// @return position of this applicaiton in apps pipepline
      AppPipelineLevel appSolverDependencyLevel() const { return m_appDepLevel; }

      /// @brief  In case of general app defines script body.
      /// @param  cmdLine  full script body
      void setScriptBody( const std::string & cmdLine ) { m_scriptBody = cmdLine; }

      /// @brief Set which version of the Cauldron app will be used. This string is used as part of path to an application
      /// @param ver cauldron version like "v2020.01nightly"
      void setCauldronVersion( const std::string & ver ) {if ( !ver.empty() ) m_version = ver;}

      /// @brief Generates script file which contains environment set up and application run for given input/output project file
      /// @param inProjectFile input project file name
      /// @param outProjectFile output project file name
      /// @param scenarioID uniquie scenario ID to allow set reference between GUI and engine
      /// @return generated script as a string
      virtual std::string generateScript( const std::string & inProjectFile, const std::string & outProjectFile, const std::string & scenarioID );

      /// @brief Generate script which will create links for files with simulation results
      /// @param fromProj source project for results files
      /// @param toProj destination project for results files
      /// @param scenarioID uniquie scenario ID to allow set reference between GUI and engine
      /// @return generated script as a string
      virtual std::string generateCopyResultsScript( const std::string & fromProj, const std::string & toProj, const std::string & scenarioID );

      /// @brief Set the number of MPI processes which will be used for the application, if it is a parallel application
      /// @param numOfCPUs number of cpus for parallel simulation
      void setCPUs( int numOfCPUs ) { m_cpus = numOfCPUs; }

      /// @brief Get cpus number for this application
      /// @return cpus number
      int cpus() { return m_cpus; }

      /// @brief Set the run time limitiation in minutes for the application, if application is running longer it will be killed
      /// @param runTimeLim the maximal number of minutes the application is allowed to run
      void setRunTimeLimit( size_t runTimeLim ) { m_runTimeLim = runTimeLim; }

      /// @brief Get run time limits for this application [Minutes], a job scheduler is using this number
      /// @return minutes number for the run time limitation
      size_t runTimeLimit() { return m_runTimeLim; }

      /// @brief Add application command line option
      /// @param opt new option to be added
      void addOption( const std::string & opt );

      /// @brief  Return scirpt suffix which depends on the chosen shell - for example .sh for bash and .bat for cmd
      /// @return commands script suffix
      const char * scriptSuffix() const;

      // Serialization / Deserialization
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      /// Version 1. Added dependecy level of the application in cauldron applications pipeline
      virtual unsigned int version() const { return 1; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "CauldronApp"; }

      /// @brief Create a new CauldronApp instance and deserialize it from the given stream
      /// @param inStream input stream
      /// @param objName name of the application
      /// @return new observable instance on success, or throw and exception in case of any error
      CauldronApp( CasaDeserializer & inStream, const char * objName );
      /// @}

   private:

      std::map< std::string, std::string >   m_env;               ///< keeps environment variables in map: [influential] -> value
      std::string                            m_appName;           ///< name of application like fastcauldron/genex6/...
      std::string                            m_scriptBody;        ///< in case of general app keeps script body
      bool                                   m_parallel;          ///< is this application parallel?
      int                                    m_cpus;              ///< how many cpus should use application
      size_t                                 m_runTimeLim;        ///< hard limit for applictaion run time

      ShellType                              m_sh;                ///< type of shell (setting environment variables differ in different shell scripts

      std::string                            m_version;           ///< Cauldron version which will be used
      std::string                            m_rootPath;          ///< path prefix to applications root like /apps/sssdev/ibs
      std::string                            m_mpirunCmd;         ///< mpirun command with parameters

      std::string                            m_inputOpt;          ///< command line option for app to load input project file
      std::string                            m_outputOpt;         ///< command line option for app to save output project file

      std::vector< std::string >             m_optionsList;       ///< List of options for the application
      bool                                   m_clearSnapshots;    ///< Add or not to script file cleaning results call

      AppPipelineLevel                       m_appDepLevel;       ///< Define where this application is located in apps pipeline

      static const char *                    s_resFilesList[][6]; ///< List of HDF files names with simulation results

      /// @brief get environment influential
      const char * env( const char * varName );

      /// @brief Print list of environment influential to the output stream
      /// @param oss output stream
      void dumpEnv( std::ostream & oss );

      /// @brief Add environment influential to the list. If such influential exists in environment,\n
      ///        picks up it value from environment and does not use given influential value
      /// @param varName name of environment influential
      /// @param varValue the value of the environment influential if it does not exist in environment
      /// @return true if given varValue was used, false otherwise
      bool pushDefaultEnv( const std::string & varName, const std::string & varValue );

      CauldronApp( const CauldronApp & anotherApp );
      CauldronApp & operator = ( const CauldronApp & anotherApp );
   };
}

#endif // CASA_API_CAULDRON_APP_H


