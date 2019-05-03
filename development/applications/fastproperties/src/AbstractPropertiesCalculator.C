//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sys/types.h>
#include <assert.h>

#include "AbstractPropertiesCalculator.h"

#include "h5_parallel_file_types.h"
#include "GeoPhysicsFormation.h"

static bool snapshotSorter (const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2);
static bool snapshotIsEqual (const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2);

static const char * basementProperties [] = {"BulkDensity", "Diffusivity",
                                             "HeatFlow", "Reflectivity",
                                             "SonicSlowness", "ThCond", "Velocity", "Depth",
                                             "Temperature", "LithoStaticPressure",
                                             "Thickness", "Lithology", "ALC",
                                             "DepthHighRes", "ThicknessHighRes"};

//------------------------------------------------------------//

AbstractPropertiesCalculator::AbstractPropertiesCalculator(int aRank) : m_basementPropertiesList (basementProperties, basementProperties + 15)
{

   m_rank = aRank;

   m_debug            = false;
   m_basement         = true;
   m_all2Dproperties  = false;
   m_all3Dproperties  = false;
   m_listProperties   = false;
   m_listSnapshots    = false;
   m_listStratigraphy = false;
   m_primaryPod       = false;
   m_extract2D        = false;
   m_no3Dproperties   = false;

   m_snapshotsType = MAJOR;

   m_projectFileName = "";
   m_simulationMode  = "";
   m_activityName    = "";
   m_decompactionMode = false;

   m_projectHandle   = 0;
   m_propertyManager = 0;
   m_activeGrid      = 0;
}

//------------------------------------------------------------//

AbstractPropertiesCalculator::~AbstractPropertiesCalculator() {

   if (m_propertyManager != 0) delete m_propertyManager;
   m_propertyManager = 0;
}

//------------------------------------------------------------//

GeoPhysics::ProjectHandle* AbstractPropertiesCalculator::getProjectHandle() const {

   return m_projectHandle;
}
//------------------------------------------------------------//

DerivedPropertyManager * AbstractPropertiesCalculator::getPropertyManager() const {

   return m_propertyManager;
}
//------------------------------------------------------------//

bool  AbstractPropertiesCalculator::finalise (bool isComplete) {

   PetscLogDouble Start_Time;
   PetscTime(&Start_Time);

   // set false as there is no need to save any properties - all are saved
   m_projectHandle->finishActivity (false);

   bool status = true;

   if (isComplete)
   {
      if (! copyFiles ())
      {
         PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: Unable to copy output files\n");

         status = false;
      }
   }

   if (isComplete and status and m_rank == 0)
   {
      displayProgress("Project file", Start_Time, "Start saving ");

      m_projectHandle->setSimulationDetails ("fastproperties", m_simulationMode, m_commandLine);

      char outputFileName[ PETSC_MAX_PATH_LEN ];
      outputFileName[0] = '\0';

      PetscBool isDefined = PETSC_FALSE;
      PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &isDefined);
      if (isDefined)
      {
         m_projectHandle->saveToFile(outputFileName);
      }
      else
      {
         m_projectHandle->saveToFile(m_projectFileName);
      }
      displayProgress("", Start_Time, "Saving is finished for ProjectFile ");
   }

   delete m_propertyManager;
   m_propertyManager = 0;

   return status;
}

//------------------------------------------------------------//

bool AbstractPropertiesCalculator::CreateFrom (DataAccess::Interface::ObjectFactory* factory) {

   if (m_projectHandle == 0)
   {
      m_projectHandle = dynamic_cast<GeoPhysics::ProjectHandle*>(OpenCauldronProject(m_projectFileName, "r", factory));

      if (m_projectHandle != 0)
      {
         if (getProperiesActivity())
         {
            m_propertyManager = new DerivedPropertyManager (m_projectHandle, true, m_debug);
         }
         else
         {
            m_propertyManager = new DerivedPropertyManager (m_projectHandle, false, m_debug);
         }
      }
   }
   if (m_projectHandle == 0 ||  m_propertyManager == 0)
   {
      return false;
   }

   if (m_primaryPod)
   {
      H5_Parallel_PropertyList::setOneFilePerProcessOption(false);
   }

   return true;
}
//------------------------------------------------------------//

bool AbstractPropertiesCalculator::startActivity() {

   if (!setFastcauldronActivityName())
   {
      return false;
   }

   const Interface::Grid * grid = m_projectHandle->getLowResolutionOutputGrid();


   const bool vizFormatResults = getProperiesActivity();

   bool started = m_projectHandle->startActivity (m_activityName, grid, false, not vizFormatResults, (m_activityName != "Fastproperties") or vizFormatResults);

   // If this table is not present the assume that the last
   // fastcauldron mode was not pressure mode.
   // This table may not be present because we are running c2e on an old
   // project, before this table was added.
   bool coupledCalculation = false;

   coupledCalculation = m_simulationMode == "Overpressure" or
      m_simulationMode == "LooselyCoupledTemperature" or
      m_simulationMode == "CoupledHighResDecompaction" or
      m_simulationMode == "CoupledPressureAndTemperature" or
      m_simulationMode == "CoupledDarcy";

   coupledCalculation = coupledCalculation and m_projectHandle->getModellingMode() == Interface::MODE3D;

   if (started)
   {
      m_projectHandle->initialise (coupledCalculation);
   }

   if (started)
   {
      started = m_projectHandle->setFormationLithologies (true, true);
   }

   if (started)
   {
      started = m_projectHandle->initialiseLayerThicknessHistory (coupledCalculation);
      if (started)
      {
         m_projectHandle->applyFctCorrections();
      }
   }

   return started;
}

//------------------------------------------------------------//
bool AbstractPropertiesCalculator::showLists() {

   return m_listProperties || m_listSnapshots || m_listStratigraphy;

}

//------------------------------------------------------------//
void AbstractPropertiesCalculator::resetProjectActivityGrid(const DataAccess::Interface::Property * property) {

   // Reset activity grid to 0 to allow high resolution properties to be loaded in high resolution and not converted to low resolution
   if (property != 0)
   {
      if (property->getName().find("HighRes") != string::npos or property->getType() == RESERVOIRPROPERTY or property->getName() == "FlowDirectionIJK")
      {
         m_activeGrid = m_projectHandle->getActivityOutputGrid();
         m_projectHandle->setActivityOutputGrid(0);
      }
   }
   else
   {
      if (m_activeGrid != 0) {
         m_projectHandle->setActivityOutputGrid(m_activeGrid);
         m_activeGrid = 0;
      }
   }
}

//------------------------------------------------------------//
bool AbstractPropertiesCalculator::allowBasementOutput (const string & propertyName3D) const {

   string propertyName = propertyName3D;

   if (propertyName.find("HeatFlow") != string::npos)
   {
      propertyName = "HeatFlow";
   }
   else if (propertyName.find("FluidVelocity") != string::npos)
   {
      propertyName = "FluidVelocity";
   }
   else if (propertyName.find("ALC") != string::npos)
   {
      propertyName = "ALC";
   }
    else
   {
      size_t len = 4;
      size_t pos = propertyName.find("Vec2");

      if (pos != string::npos) {
         // Replace Vec2 with Vec.
         propertyName.replace(pos, len, "Vec");
      }
   }
   return isBasementProperty(propertyName);
}

//------------------------------------------------------------//

bool AbstractPropertiesCalculator::acquireSnapshots(SnapshotList & snapshots)
{
   if (m_ages.size() == 0)
   {
      SnapshotList * allSnapshots = m_projectHandle->getSnapshots(m_snapshotsType);
      snapshots = *allSnapshots;
      delete allSnapshots;
      return true;
   }
   else
   {
      unsigned int index;
      double firstAge = -1;
      double secondAge = -1;
      for (index = 0; index < m_ages.size(); ++index)
      {
         if (m_ages[ index ] >= 0)
         {
            if (firstAge < 0)
               firstAge = m_ages[ index ];
            else
               secondAge = m_ages[ index ];
         }
         else
         {
            if (secondAge < 0)
            {
               if (firstAge >= 0)
               {
                  const Snapshot * snapshot = m_projectHandle->findSnapshot(firstAge, m_snapshotsType);
                  if (snapshot) snapshots.push_back(snapshot);
                  if (m_debug && m_rank == 0 && snapshot) LogHandler(LogHandler::INFO_SEVERITY) << "adding single snapshot " << snapshot->getTime();
               }
            }
            else
            {
               if (firstAge >= 0)
               {
                  if (firstAge > secondAge)
                  {
                     std::swap(firstAge, secondAge);
                  }

                  SnapshotList * allSnapshots = m_projectHandle->getSnapshots(m_snapshotsType);
                  SnapshotList::iterator snapshotIter;
                  for (snapshotIter = allSnapshots->begin(); snapshotIter != allSnapshots->end(); ++snapshotIter)
                  {
                     const Snapshot * snapshot = *snapshotIter;
                     if (snapshot->getTime() >= firstAge && snapshot->getTime() <= secondAge)
                     {
                        if (snapshot) snapshots.push_back(snapshot);
                        if (m_debug && snapshot && m_rank == 0) LogHandler(LogHandler::INFO_SEVERITY) << "adding range snapshot " << snapshot->getTime();
                     }
                  }
                  delete allSnapshots;
               }
            }
            firstAge = secondAge = -1;
         }
      }
   }
   sort(snapshots.begin(), snapshots.end(), snapshotSorter);

   if (m_debug && m_rank == 0)
   {
      LogHandler(LogHandler::INFO_SEVERITY) << "Snapshots ordered";
      SnapshotList::iterator snapshotIter;
      for (snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter)
      {
         LogHandler(LogHandler::INFO_SEVERITY) << (*snapshotIter)->getTime();
      }
   }

   SnapshotList::iterator firstObsolete = unique(snapshots.begin(), snapshots.end(), snapshotIsEqual);
   snapshots.erase(firstObsolete, snapshots.end());

   if (m_debug && m_rank == 0)
   {
      LogHandler(LogHandler::INFO_SEVERITY) << "Snapshots uniquefied";
      SnapshotList::iterator snapshotIter;
      for (snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter)
      {
         LogHandler(LogHandler::INFO_SEVERITY) << (*snapshotIter)->getTime();
      }
   }

   return true;
}

//------------------------------------------------------------//
void AbstractPropertiesCalculator::acquireProperties(Interface::PropertyList & properties) {

   if (m_propertyNames.size() != 0)
   {
      // remove duplicated names
      std::sort(m_propertyNames.begin(), m_propertyNames.end());
      m_propertyNames.erase(std::unique(m_propertyNames.begin(), m_propertyNames.end(), DerivedProperties::isEqualPropertyName), m_propertyNames.end());

      DerivedProperties::acquireProperties(m_projectHandle, * m_propertyManager, properties, m_propertyNames);
   }
}
//------------------------------------------------------------//
void AbstractPropertiesCalculator::acquireFormationsSurfaces(FormationSurfaceVector& formationSurfaceItems) {

   bool load3d = false;
   if (m_formationNames.size() != 0 or m_all3Dproperties)
   {
      DerivedProperties::acquireFormations(m_projectHandle, formationSurfaceItems, m_formationNames, m_basement);
      load3d = true;
   }
   if (m_all2Dproperties)
   {
      if (not load3d)
      {
         DerivedProperties::acquireFormations(m_projectHandle, formationSurfaceItems, m_formationNames, m_basement);
      }
      DerivedProperties::acquireFormationSurfaces(m_projectHandle, formationSurfaceItems, m_formationNames, true, m_basement);
      DerivedProperties::acquireFormationSurfaces(m_projectHandle, formationSurfaceItems, m_formationNames, false, m_basement);

      if (not m_all3Dproperties)
      {
         m_no3Dproperties = true;
      }
   }

   // if the property is selected but formationSurface list is empty add all formations
   if (formationSurfaceItems.empty() and m_propertyNames.size() != 0)
   {
      DerivedProperties::acquireFormations(m_projectHandle, formationSurfaceItems, m_formationNames, m_basement);
      DerivedProperties::acquireFormationSurfaces(m_projectHandle, formationSurfaceItems, m_formationNames, true, m_basement);
      DerivedProperties::acquireFormationSurfaces(m_projectHandle, formationSurfaceItems, m_formationNames, false, m_basement);
   }

}
//------------------------------------------------------------//
void AbstractPropertiesCalculator::acquireAll2Dproperties() {

   if (m_all2Dproperties or (not m_formationNames.empty() and m_propertyNames.empty ()))
   {
      Interface::PropertyList * allProperties = m_projectHandle->getProperties(true);

      LogHandler(LogHandler::DEBUG_SEVERITY) << "Acquiring computable 2D properties";
      for (size_t i = 0; i < allProperties->size (); ++i)
      {
         const Interface::Property* property = (*allProperties)[ i ];

         if (property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
             (m_propertyManager->formationMapPropertyIsComputable (property) or m_propertyManager->reservoirPropertyIsComputable (property)))
         {
            m_propertyNames.push_back(property->getName());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "   #" << property->getName() << " (2D formation)";
         }
         if ((property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY) and
             m_propertyManager->surfacePropertyIsComputable (property))
         {
            m_propertyNames.push_back(property->getName());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "   #" << property->getName() << " (2D surface)";
         }
         if (property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              m_propertyManager->formationSurfacePropertyIsComputable (property))
         {
            m_propertyNames.push_back(property->getName());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "   #" << property->getName() << " (2D formation-surface)";
         }
      }

      delete allProperties;
   }
}
//------------------------------------------------------------//

void AbstractPropertiesCalculator::acquireAll3Dproperties() {

   if (m_all3Dproperties or (not m_formationNames.empty() and m_propertyNames.empty ()))
   {

      Interface::PropertyList * allProperties = m_projectHandle->getProperties(true);
      LogHandler(LogHandler::DEBUG_SEVERITY) << "Acquiring computable 3D property ";
      for (size_t i = 0; i < allProperties->size (); ++i)
      {
         const Interface::Property* property = (*allProperties)[ i ];

         if ((property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY) and
             m_propertyManager->formationPropertyIsComputable (property))
         {
            m_propertyNames.push_back(property->getName());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "   #" << property->getName();
         }

      }
      delete allProperties;
   }
}

//------------------------------------------------------------//
void AbstractPropertiesCalculator::printListSnapshots ()  {

   if (m_listSnapshots && m_rank == 0)
   {

      cout << endl;
      SnapshotList * mySnapshots = m_projectHandle->getSnapshots(m_snapshotsType);
      SnapshotList::iterator snapshotIter;

      cout.precision (8);
      cout << "Available snapshots are: ";

      for (snapshotIter = mySnapshots->begin(); snapshotIter != mySnapshots->end(); ++snapshotIter)
      {

         if (snapshotIter != mySnapshots->begin()) cout << ",";
         int oldPrecision = static_cast<int>(cout.precision());
         cout << setprecision(9);
         cout << (*snapshotIter)->getTime();
         cout <<setprecision(oldPrecision);
      }
      cout << endl;

      delete mySnapshots;
   }
}
//------------------------------------------------------------//
void AbstractPropertiesCalculator::printListStratigraphy () {

  if (m_listStratigraphy && m_rank == 0)  {
      cout << endl;
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot(0);

      Interface::FormationList * myFormations = m_projectHandle->getFormations(zeroSnapshot, true);
      Interface::FormationList::iterator formationIter;
      cout << "Stratigraphy: ";
      cout << endl;
      cout << endl;
      bool firstFormation = true;
      bool arrivedAtBasement = false;
      for (formationIter = myFormations->begin(); formationIter != myFormations->end(); ++formationIter)
      {
         const Interface::Formation * formation = *formationIter;
         if (formation->kind() == BASEMENT_FORMATION && arrivedAtBasement == false)
         {
            arrivedAtBasement = true;
            cout << "Next formations and surfaces belong to the basement and only produce output when used with '-basement'" << endl;
         }
         if (firstFormation)
         {
            cout << "\t" << formation->getTopSurface()->getName() << " (" << formation->getTopSurface()->getSnapshot()->getTime() << " Ma)" << endl;
            firstFormation = false;
         }

         cout << "\t\t" << formation->getName() << endl;
         const Interface::Surface * bottomSurface = formation->getBottomSurface();
         if (bottomSurface)
         {
            cout << "\t" << bottomSurface->getName();
            if (bottomSurface->getSnapshot())
            {
               cout << " (" << bottomSurface->getSnapshot()->getTime() << " Ma)";
            }
            cout << endl;
         }
      }
      cout << endl;

      delete myFormations;
   }
}

//------------------------------------------------------------//

void AbstractPropertiesCalculator::printOutputableProperties () {

   if (m_listProperties)
   {
      Interface::PropertyList * allProperties = m_projectHandle->getProperties (true);

      PetscPrintf(PETSC_COMM_WORLD, "Available 3D output properties are: ");

      for (size_t i = 0; i < allProperties->size (); ++i)
      {
         const Interface::Property* property = (*allProperties)[ i ];
         LogHandler(LogHandler::DEBUG_SEVERITY) << "########################################################";
         LogHandler(LogHandler::DEBUG_SEVERITY) << "3D-->" << property->getName() << " computable?";

         if ((property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY) and
               m_propertyManager->formationPropertyIsComputable (property))
         {
            PetscPrintf(PETSC_COMM_WORLD, "%s ",  property->getName ().c_str());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "YES";
         }
         else
         {
            LogHandler(LogHandler::DEBUG_SEVERITY) << "NO";
         }
      }

      PetscPrintf(PETSC_COMM_WORLD, "\n\n");
      PetscPrintf(PETSC_COMM_WORLD, "Available 2D output properties are: ");

      for (size_t i = 0; i < allProperties->size (); ++i) {
         const Interface::Property* property = (*allProperties)[ i ];
         LogHandler(LogHandler::DEBUG_SEVERITY) << "########################################################";
         LogHandler(LogHandler::DEBUG_SEVERITY) << "2D-->" << property->getName();

         if (property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
              m_propertyManager->formationMapPropertyIsComputable (property))
         {
            PetscPrintf(PETSC_COMM_WORLD, "%s ", property->getName ().c_str());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "YES";

         }
         else if ((property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY) and
              m_propertyManager->surfacePropertyIsComputable (property))
         {
            PetscPrintf(PETSC_COMM_WORLD, "%s ", property->getName ().c_str());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "YES";

         }
         else if (property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              m_propertyManager->formationSurfacePropertyIsComputable (property))
         {

            PetscPrintf(PETSC_COMM_WORLD, "%s ", property->getName ().c_str());
            LogHandler(LogHandler::DEBUG_SEVERITY) << "YES";
         }
         else
         {
            LogHandler(LogHandler::DEBUG_SEVERITY) << "NO";
         }
      }

      PetscPrintf(PETSC_COMM_WORLD, "\n\n");

      delete allProperties;
   }
}

//------------------------------------------------------------//
bool AbstractPropertiesCalculator::setFastcauldronActivityName() {

   if (m_projectHandle->getDetailsOfLastSimulation ("fastcauldron") == 0 ||
       m_projectHandle->getDetailsOfLastSimulation ("fastcauldron")->getSimulatorMode () == "NoCalculation")
   {
      return false;
   }

   m_simulationMode = m_projectHandle->getDetailsOfLastSimulation ("fastcauldron")->getSimulatorMode ();

   if (m_simulationMode == "HydrostaticDecompaction" ||
       m_simulationMode == "HydrostaticTemperature" ||
       m_simulationMode == "Overpressure")
   {
      m_activityName = m_simulationMode;
   }
   else if (m_simulationMode == "CoupledPressureAndTemperature")
   {
      m_activityName = "PressureAndTemperature";
   }
   else if (m_simulationMode == "LooselyCoupledTemperature")
   {
      m_activityName = "OverpressuredTemperature";
   }
   else if (m_simulationMode == "CoupledHighResDecompaction" or m_simulationMode == "HydrostaticHighResDecompaction")
   {
      m_activityName = "HighResDecompaction";
   }
   else
   {
      m_activityName = "Fastproperties";
   }
   m_decompactionMode = (m_activityName == "HydrostaticDecompaction") or (m_activityName == "HighResDecompaction");

   return true;
}

//------------------------------------------------------------//
bool AbstractPropertiesCalculator::parseCommandLine(int argc, char ** argv) {

   const int MaximumLengh = 4000;

   PetscBool parameterDefined = PETSC_FALSE;
   char * parametersArray[MaximumLengh];
   char parameter[MaximumLengh];

   PetscBool uDefined = PETSC_FALSE;

   PetscOptionsHasName (PETSC_NULL, "-help", &parameterDefined);
   PetscOptionsHasName (PETSC_NULL, "-usage", &uDefined);

   if (parameterDefined or uDefined)
   {
      showUsage (argv[0], " Standard usage.");

      return false;
   }

   PetscInt numberOfParameters = MaximumLengh;
   PetscOptionsGetString (PETSC_NULL, "-project", parameter, numberOfParameters, &parameterDefined);
   if (not parameterDefined)
   {
      showUsage (argv[0], "No project file specified");
      return false;
   } else {
      m_projectFileName = string(parameter);
   }

   PetscOptionsHasName (PETSC_NULL, "-properties", &parameterDefined);
   if (parameterDefined)
   {
      numberOfParameters = MaximumLengh;
      PetscOptionsGetStringArray (PETSC_NULL, "-properties", parametersArray, &numberOfParameters, &parameterDefined);
      if (not parameterDefined)
      {
         showUsage (argv[0], "Argument for '-properties' is missing");

         return false;
      }
      m_propertyNames.clear();
      for (int i = 0; i < numberOfParameters; ++i)
      {
         m_propertyNames.push_back (parametersArray[i]);
      }
   }

   double agesArray[MaximumLengh];
   PetscOptionsHasName (PETSC_NULL, "-ages", &parameterDefined);
   if (parameterDefined)
   {
      numberOfParameters = MaximumLengh;
      PetscOptionsGetRealArray (PETSC_NULL, "-ages", agesArray, &numberOfParameters, &parameterDefined);
      if (numberOfParameters == 0)
      {
         showUsage (argv[0], "Argument for '-ages' is missing");

         return false;
      }
      else
      {
         m_ages.clear();
         for (int i = 0; i < numberOfParameters; ++i)
         {
            m_ages.push_back (agesArray[i]);
            m_ages.push_back (-1); //separator
         }
         m_ages.push_back (-1); //separator
      }
   }

   PetscOptionsHasName(PETSC_NULL, "-formations", &parameterDefined);
   if (parameterDefined)
   {
      numberOfParameters = MaximumLengh;
      PetscOptionsGetStringArray (PETSC_NULL, "-formations", parametersArray, &numberOfParameters, &parameterDefined);
      if (not parameterDefined)
      {
         showUsage (argv[0], "Argument for '-formations' is missing");

         return false;
      }
      m_formationNames.clear();
      for (int i = 0; i < numberOfParameters; ++i)
      {
         m_formationNames.push_back (parametersArray[i]);
      }
   }
   PetscOptionsHasName (PETSC_NULL, "-nobasement", &parameterDefined);
   if (parameterDefined) m_basement = false;

   PetscOptionsHasName (PETSC_NULL, "-minor", &parameterDefined);
   if (parameterDefined) m_snapshotsType = MAJOR | MINOR;

   PetscOptionsHasName (PETSC_NULL, "-all-2D-properties", &parameterDefined);
   if (parameterDefined) m_all2Dproperties = true;

   PetscOptionsHasName (PETSC_NULL, "-all-3D-properties", &parameterDefined);
   if (parameterDefined) m_all3Dproperties = true;

   PetscOptionsHasName (PETSC_NULL, "-list-properties", &parameterDefined);
   if (parameterDefined) m_listProperties = true;

   PetscOptionsHasName (PETSC_NULL, "-list-snapshots", &parameterDefined);
   if (parameterDefined) m_listSnapshots = true;

   PetscOptionsHasName (PETSC_NULL, "-list-stratigraphy", &parameterDefined);
   if (parameterDefined) m_listStratigraphy = true;

   PetscOptionsHasName (PETSC_NULL, "-debug", &parameterDefined);
   if (parameterDefined) m_debug = true;

   PetscOptionsHasName (PETSC_NULL, "-primaryPod", &parameterDefined);
   if (parameterDefined) m_primaryPod = true;

   PetscOptionsHasName (PETSC_NULL, "-extract2D", &parameterDefined);
   if (parameterDefined) m_extract2D = true;


   if (m_projectFileName == "")
   {
      showUsage (argv[ 0 ], "No project file specified");

      return false;
   }

   for (int i = 1; i < argc; ++i)
   {
      m_commandLine += std::string (argv [ i ]) + (i == argc - 1 ? "" : " ");
   }

   return true;
}

//------------------------------------------------------------//
bool AbstractPropertiesCalculator::isBasementProperty(const string& propertyName) const {

   return m_basementPropertiesList.count(propertyName) != 0;
}

//------------------------------------------------------------//
void AbstractPropertiesCalculator::startTimer() {
   PetscTime (&m_startTime);

}

//------------------------------------------------------------//

void AbstractPropertiesCalculator::showUsage (const char* command, const char* message)
{
   if (m_rank == 0) {
      cout << endl;

      if (message != 0)
      {
         cout << command << ": " << message << endl;
      }

      cout << "Usage (case sensitive!!): " << command << endl << endl
           << "\t[-properties name1,name2...]               properties to produce output for" << endl
           << "\t[-ages age1[-age2],...]                    select snapshot ages using single values and/or ranges" << endl << endl
           << "\t[-formations formation1,formation2...]     produce output for the given formations" << endl
           << "\t                                           the four options above can include Crust or Mantle" << endl << endl
           << "\t[-nobasement]                              do not produce output for the basement" << endl
           << "\t                                           only needed if none of the three options above have been specified" << endl << endl
           << "\t[-project] projectname                     name of 3D Cauldron project file to produce output for" << endl
           << "\t[-save filename]                           name of file to save output (*.csv format) table to, otherwise save to stdout" << endl
           << "\t[-verbosity level]                         verbosity level of the log file (s): minimal|normal|detailed|diagnostic. Default value is 'normal'." << endl
           << endl
           << "\t[-all-3D-properties]                       produce output for all 3D properties" << endl
           << "\t[-all-2D-properties]                       produce output for all 2D primary properties" << endl
           << "\t[-project-properties]                      produce output for the properties selected for output in the project file" << endl
           << "\t[-extract2D]                               produce output for all 2D properties (use with -all-2D-properties)" << endl
           << "\t[-list-properties]                         print a list of available properties and exit" << endl
           << "\t[-list-snapshots]                          print a list of available snapshots and exit" << endl
           << "\t[-list-stratigraphy]                       print a list of available surfaces and formations and exit" << endl << endl
           << "\t[-minor]                                   output major and minor snapshots" << endl << endl
           << "\t[-pminor]                                  output major and primary properties at minor snapshots" << endl << endl
           << "Options for visualization format:" << endl << endl
           << "\t[-viz]                                     calculate the properties and convert to visialization format." << endl << endl
           << "\t[-genex]                                   produce output for Genex/Shale Gas properties only" << endl << endl
           << "\t[-fastcauldron]                            produce output for Fastcauldron properties only" << endl << endl
           << "\t[-mig]                                     produce output for Fastmig and tracktraps properties only" << endl << endl
           << "\t[-savexml]                                 name of xml file in which to save visualization format results" << endl << endl
           << "\t[-xml]                                     load existing xml" << endl << endl
           << "\t[-hdfonly]                                 convert visulization format back to hdf format." << endl << endl
           << "\t[-listXml]                                 list of properties in xml file." << endl << endl
           << endl
           << "\t[-help]                                    print this message and exit" << endl << endl
           << "Options for shared cluster storage:" << endl << endl
           << "\t[-primaryPod <dir>]                        use if the fastcauldron data are stored in the shared <dir> on the cluster" << endl << endl
           << "\t[-copy]                                    use in combination with -primaryPod. Copy the results to the local dir and " << endl
           << "\t                                           remove them from the shared dir on the cluster" << endl << endl
           << "\t[-noremove]                                use in combination with -primaryPod and -copy. Don't remove results from " << endl
           << "\t                                           the shared dir on the cluster" << endl << endl;
      cout << "If names in an argument list contain spaces, put the list between double or single quotes, e.g:"
           << "\t-formations \"Dissolved Salt,Al Khalata\"" << endl;
      cout << "Bracketed options are optional and options may be abbreviated" << endl << endl;
   }
}

//------------------------------------------------------------//

bool snapshotIsEqual(const Snapshot * snapshot1, const Snapshot * snapshot2)
{
   return snapshot1->getTime () == snapshot2->getTime ();
}

bool snapshotSorter(const Snapshot * snapshot1, const Snapshot * snapshot2)
{
   return snapshot1->getTime () > snapshot2->getTime ();
}

void displayProgress(const string & fileName, double startTime, const string & message)
{
   if (PetscGlobalRank != 0)
      return;
   double EndTime;

   PetscTime (&EndTime);

   double CalculationTime = EndTime - startTime;
   long remainder = (long) CalculationTime;

   long secs = remainder % 60;
   remainder -= secs;
   long mins = (remainder / 60) % 60;
   remainder -= mins * 60;
   long hrs = remainder / (60 * 60);

   char time[124];
   sprintf (time, "%2.2ld:%2.2ld:%2.2ld", hrs, mins, secs);

   ostringstream buf;
   buf.precision(4);
   buf.setf(ios::fixed);

   buf << message << fileName << " : " << setw(8) << " Elapsed: " << time;;

   cout << buf.str() << endl;
   cout << flush;
}

void displayTime (const double timeToDisplay, const char * msgToDisplay)
{
   int hours   = (int)(timeToDisplay / 3600.0);
   int minutes = (int)((timeToDisplay - (hours * 3600.0)) / 60.0);
   int seconds = (int)(timeToDisplay - hours * 3600.0 - minutes * 60.0);

   PetscPrintf (PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
   PetscPrintf (PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds);
   PetscPrintf (PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

}
