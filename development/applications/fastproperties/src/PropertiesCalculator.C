//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <petsc.h>

#include "Interface/SimulationDetails.h"
#include "h5_parallel_file_types.h"
#include "Interface/OutputProperty.h"
#include "Interface/RunParameters.h"
#include "Interface/ProjectData.h"
#include "Interface/MantleFormation.h"
#include "Interface/Reservoir.h"
#include "GeoPhysicsFormation.h"
#include "PropertiesCalculator.h"
#include "FilePath.h"
#include "Utilities.h"
#include "ImportFromXML.h"
#include "ExportToHDF.h"
#include "FolderPath.h"

// utilities library
#include "LogHandler.h"
#include "StatisticsHandler.h"

using namespace Utilities::CheckMemory;

static bool splitString (char * string, char separator, char * & firstPart, char * & secondPart);
static bool parseStrings (StringVector & strings, char * stringsString);
static bool parseAges (DoubleVector & ages, char * agesString);

static bool snapshotSorter (const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2);
static bool snapshotIsEqual (const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2);

void displayTime (const double timeToDisplay, const char * msgToDisplay);
void displayProgress (const string & fileName, double startTime, const string & message);


//------------------------------------------------------------//

PropertiesCalculator::PropertiesCalculator(int aRank) {
   
   m_rank = aRank;
   
   m_debug            = false;
   m_copy             = false;
   m_basement         = true;
   m_all2Dproperties  = false;
   m_all3Dproperties  = false;
   m_listProperties   = false;
   m_listSnapshots    = false;
   m_listStratigraphy = false;
   m_vizFormat        = false;
   m_vizFormatHDF     = false;
   m_vizFormatHDFonly = false;
   m_vizListXml       = false;
   m_primaryPod       = false;
   m_extract2D        = false;
   m_no3Dproperties   = false;
   m_minorPrimary     = false;
   m_projectProperties = false;
   
   m_snapshotsType = MAJOR;
   
   m_projectFileName = "";
   m_simulationMode  = "";
   m_activityName    = "";
   m_decompactionMode = false;
   
   m_projectHandle   = 0;
   m_propertyManager = 0;
   m_activeGrid      = 0;
   m_export          = 0;
}

//------------------------------------------------------------//

PropertiesCalculator::~PropertiesCalculator() {
   
   if (m_propertyManager != 0) delete m_propertyManager;
   m_propertyManager = 0;
   
   if (m_export != 0) delete m_export;
   m_export = 0;
}

//------------------------------------------------------------//

GeoPhysics::ProjectHandle* PropertiesCalculator::getProjectHandle() const {
   
   return m_projectHandle;
}
//------------------------------------------------------------//

DerivedPropertyManager * PropertiesCalculator::getPropertyManager() const {
   
   return m_propertyManager;
}
//------------------------------------------------------------//

bool  PropertiesCalculator::finalise (bool isComplete) {
   
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

bool PropertiesCalculator::CreateFrom (DataAccess::Interface::ObjectFactory* factory) {
   
   if (m_projectHandle == 0) 
   {
      m_projectHandle = (GeoPhysics::ProjectHandle*)(OpenCauldronProject(m_projectFileName, "r", factory));
      
      if (m_projectHandle != 0) 
      {
         if (m_vizFormat) 
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

bool PropertiesCalculator::startActivity() {
   
   if (!setFastcauldronActivityName()) 
   {
      return false;
   }
   
   const Interface::Grid * grid = m_projectHandle->getLowResolutionOutputGrid();
   
   bool started = m_projectHandle->startActivity (m_activityName, grid, false, true, m_activityName != "Fastproperties");
   
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
bool PropertiesCalculator::showLists() {
   
   return m_listProperties || m_listSnapshots || m_listStratigraphy;
   
}

//------------------------------------------------------------//
void PropertiesCalculator::createXML() {
   
   if (m_vizFormat) 
   {
      
#if 0
      // Works if the xml file with the primary properties converted  exists
      boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
      pathToxml /= m_projectHandle->getProjectName();
      m_fileNameXml = pathToxml.string() + xmlExt;
      ibs::FilePath vizFileName(m_fileNameXml);
      
      if (vizFileName.exists()) 
      {
         m_vizProject = CauldronIO::ImportFromXML::importFromXML(m_fileNameXml);
      } 
      else 
      {
         // create new xml
         m_vizProject = createStructureFromProjectHandle(false);
      }
#endif
      m_vizProject = createStructureFromProjectHandle(false);
      
      if (m_rank == 0) 
      {
         boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
         pathToxml /= m_projectHandle->getProjectName();
         string absPath = pathToxml.string() + ".xml";
         
         // Create empty property tree object
         ibs::FilePath outputPath(absPath);
         ibs::FilePath folderPath = outputPath.filePath();
         std::string filename = outputPath.fileName();
         std::string filenameNoExtension = outputPath.fileNameNoExtension();
         filenameNoExtension += "_vizIO_output";
         folderPath << filenameNoExtension;

         // Create output directory if not existing
         if (!folderPath.exists())
         {
            FolderPath(folderPath.path()).create();
         }
         m_pt = m_doc.append_child("project");
         m_export = new CauldronIO::ExportToXML(outputPath.filePath(), filenameNoExtension, 1, false);

         // Create xml property tree and write datastores
         std::shared_ptr<CauldronIO::Project> projectExisting;
         m_export->addProjectDescription(m_pt,  m_vizProject, projectExisting);
         m_snapShotNodes = m_pt.append_child("snapshots");
      }
   }
   if (false and m_rank == 0) 
   {
      const shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, 0);
      DerivedProperties::listProperties(vizSnapshot, m_vizProject);
   }
}

//------------------------------------------------------------//
void PropertiesCalculator::saveXML() {
   
   if (m_vizFormat and m_rank == 0) 
   {
      std::shared_ptr<CauldronIO::Project> projectExisting;
#if 0
      // Works if the xml file with the primary properties converted  exists.
      // the derived properties will be saved in a different folder and a new xml will be created.
      ibs::FilePath vizFileName(m_fileNameXml);
      if (vizFileName.exists()) 
      {
         projectExisting = CauldronIO::ImportFromXML::importFromXML(m_fileNameXml);;
      }
#endif
      boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
      pathToxml /= m_projectHandle->getProjectName();
      string fileNameXml = pathToxml.string() + ".xml";
      
      displayProgress(fileNameXml, m_startTime, "Writing to visualization format ");
      
      m_export->addProjectData(m_pt,  m_vizProject, false);
      
      ibs::FilePath outputPath(fileNameXml);
      ibs::FilePath xmlFileName(outputPath.filePath());
      xmlFileName << outputPath.fileNameNoExtension() + ".xml";
      m_doc.save_file(xmlFileName.cpath());
      
      displayProgress("", m_startTime, "Writing to visualization format done ");
   }
}

//------------------------------------------------------------//
void PropertiesCalculator::calculateProperties(FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots)  {
   
   if (properties.size () == 0) 
   {
      return;
   }

   m_sharedProjectHandle.reset(m_projectHandle);
   
   if (m_vizFormat) 
   {
      createXML();
      
      MPI_Op_create((MPI_User_function *)minmax_op, true, & m_op);
   }
   
   Interface::SnapshotList::iterator snapshotIter;
   
   Interface::PropertyList::iterator propertyIter;
   FormationSurfaceVector::iterator formationIter;
   
   SnapshotFormationSurfaceOutputPropertyValueMap allOutputPropertyValues;
   
   if (snapshots.empty()) 
   {
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot(0);
      
      snapshots.push_back(zeroSnapshot);
   }
   
   if (m_vizFormat) 
   {
      updateFormationsKRange();
   }
   
   struct stat fileStatus;
   int fileError;
   
   for (snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter)
   {
      const Interface::Snapshot * snapshot = *snapshotIter;
      
      displayProgress(snapshot->getFileName (), m_startTime, "Start computing ");
      
      if (snapshot->getFileName () != "") 
      {
         ibs::FilePath fileName(m_projectHandle->getFullOutputDir ());
         fileName << snapshot->getFileName ();
         fileError = stat (fileName.cpath(), &fileStatus);
         
         ((Snapshot *)snapshot)->setAppendFile (not fileError);
      }
      if (m_vizFormat) 
      {
         m_formInfoList.reset();
         m_formInfoList = getDepthFormations(m_projectHandle, snapshot);
         if (m_formInfoList->size() < 1) 
         {
            continue;
         }
      }
      for (formationIter = formationItems.begin(); formationIter != formationItems.end(); ++formationIter)
      {
         const Interface::Formation * formation = (*formationIter).first;
         const Interface::Surface   * surface   = (*formationIter).second;
         const Interface::Snapshot  * bottomSurfaceSnapshot = (formation->getBottomSurface() != 0 ? formation->getBottomSurface()->getSnapshot() : 0);
         
         if (snapshot->getTime() != 0.0 and surface == 0 and bottomSurfaceSnapshot != 0) 
         {
            const double depoAge = bottomSurfaceSnapshot->getTime();
            if (snapshot->getTime() > depoAge or fabs(snapshot->getTime() - depoAge) < snapshot->getTime() * 1e-9) 
            {
               continue;
            }
         }
         
         for (propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter)
         {
            const Interface::Property * property = *propertyIter;
            if (m_no3Dproperties and surface == 0 and property->getPropertyAttribute() != DataModel::FORMATION_2D_PROPERTY) 
            {
               continue;
            }
            
            if (not m_extract2D and surface != 0 and property->getPropertyAttribute() != DataModel::SURFACE_2D_PROPERTY) 
            {
               continue;
            }
            
            if (not m_projectProperties or (m_projectProperties and allowOutput(property->getCauldronName(), formation, surface))) 
            {
               resetProjectActivityGrid (property);
               OutputPropertyValuePtr outputProperty = DerivedProperties::allocateOutputProperty (* m_propertyManager, property, snapshot, * formationIter, m_basement);
               resetProjectActivityGrid ();

               if (outputProperty != 0) 
               {
                  if (m_debug && m_rank == 0) 
                  {
                     LogHandler(LogHandler::INFO_SEVERITY) << "Snapshot: " << snapshot->getTime() <<
                        " allocate " << property->getName() << " " << (formation != 0 ? formation->getName() : "") << " " <<
                        (surface != 0 ? surface->getName() : "");
                  }
                  allOutputPropertyValues [ snapshot ][ * formationIter ][ property ] = outputProperty;
               } 
               else 
               {
                  if (m_debug && m_rank == 0) 
                  {
                     LogHandler(LogHandler::INFO_SEVERITY) << "Could not calculate derived property " << property->getName()
                                                             << " @ snapshot " << snapshot->getTime() << "Ma for formation " <<
                        (formation != 0 ? formation->getName() : "") << " " <<  (surface != 0 ? surface->getName() : "") << ".";
                  }
               }
            }
         }
         
         if (not m_vizFormat) 
         { 
            DerivedProperties::outputSnapshotFormationData(m_projectHandle, snapshot, * formationIter, properties, allOutputPropertyValues);
         } 
         else 
         {
            createVizSnapshotFormationData(snapshot, * formationIter, properties, allOutputPropertyValues);
         }
      }
      
      removeProperties(snapshot, allOutputPropertyValues);
      m_propertyManager->removeProperties(snapshot);
      
      if (not m_vizFormat) 
      {
         displayProgress(snapshot->getFileName (), m_startTime, "Start saving ");
         
         m_projectHandle->continueActivity();
         
         displayProgress(snapshot->getFileName (), m_startTime, "Saving is finished for ");
      } 
      else 
      {
         collectVolumeData(DerivedProperties::getSnapShot(m_vizProject, snapshot->getTime()));
         
         if (m_rank == 0) 
         {
            std::shared_ptr<CauldronIO::SnapShot> snap = DerivedProperties::getSnapShot(m_vizProject, snapshot->getTime());

            DerivedProperties::updateConstantValue(snap);
            pugi::xml_node node = m_snapShotNodes.append_child("snapshot");
            m_export->addSnapShot(snap, node);
         }
         DerivedProperties::getSnapShot(m_vizProject, snapshot->getTime())->release();
      }
      // m_projectHandle->deleteRecordLessMapPropertyValues();
      // m_projectHandle->deleteRecordLessVolumePropertyValues();
      
      m_projectHandle->deletePropertiesValuesMaps (snapshot);
      
      StatisticsHandler::update ();
   }
   
   if (m_vizFormat) 
   {
      saveXML();

      MPI_Op_free (&m_op);  
   }
   
   if (m_vizFormatHDF and m_rank == 0) 
   {
      writeToHDF();
   }
   
   PetscLogDouble End_Time;
   PetscTime(&End_Time);
   
   displayTime(End_Time - m_startTime, "Total derived properties saving: ");
}

//------------------------------------------------------------//
void PropertiesCalculator::resetProjectActivityGrid(const DataAccess::Interface::Property * property) {

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
void PropertiesCalculator::listXmlProperties() {
   
   boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
   pathToxml /= m_projectHandle->getProjectName();
   string fileNameXml = pathToxml.string() + ".xml";
   std::shared_ptr< CauldronIO::Project> vizProject(CauldronIO::ImportFromXML::importFromXML(fileNameXml));

   cout << "List of properties in " << fileNameXml << " : " << endl;


   SnapShotList snapShotList = vizProject->getSnapShots();
   for (auto& snapShot : snapShotList) 
   {
      cout << "Snapshot " << snapShot->getAge() << endl;
      DerivedProperties::listProperties(snapShot, vizProject);
   }
}

//------------------------------------------------------------//
void PropertiesCalculator::writeToHDF() {

   boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
   pathToxml /= m_projectHandle->getProjectName();
   string fileNameXml = pathToxml.string() + ".xml";
   std::shared_ptr< CauldronIO::Project> vizProject(CauldronIO::ImportFromXML::importFromXML(fileNameXml));

   cout << "Writing to HDF from visualization format " << fileNameXml << endl;

   CauldronIO::ExportToHDF::exportToHDF(vizProject, fileNameXml, 1, m_basement, m_projectHandle);
}
//------------------------------------------------------------//

PropertyOutputOption PropertiesCalculator::checkTimeFilter3D (const string & name) const {

   if (name == "AllochthonousLithology" or  name == "Lithology" or name == "BrineDensity" or name == "BrineViscosity") 
   {
      return Interface::SEDIMENTS_ONLY_OUTPUT;
   }
   if (name == "FracturePressure") 
   {
      if (m_projectHandle->getRunParameters ()->getFractureType () == "None") 
      {
         return Interface::NO_OUTPUT;
      }
   }
   if (name == "FaultElements") 
   {
      if (m_projectHandle->getBasinHasActiveFaults ()) 
      {
         return Interface::SEDIMENTS_ONLY_OUTPUT;
      } 
      else 
      {
         return Interface::NO_OUTPUT;
      }
   }
   if (name == "HorizontalPermeability") 
   {
      const Interface::OutputProperty* permeability = m_projectHandle->findTimeOutputProperty ("PermeabilityVec");
      const Interface::PropertyOutputOption permeabilityOption = (permeability == 0 ? Interface::NO_OUTPUT : permeability->getOption ());
      const Interface::OutputProperty* hpermeability = m_projectHandle->findTimeOutputProperty ("HorizontalPermeability");
      const Interface::PropertyOutputOption hpermeabilityOption = (hpermeability == 0 ? Interface::NO_OUTPUT : hpermeability->getOption ());
      
      if (hpermeabilityOption  == Interface::NO_OUTPUT and permeability != 0) 
      {
         return permeabilityOption;
      }
      if (permeability == 0) 
      {
         return hpermeabilityOption;
      }
   }

   const Interface::OutputProperty * property = m_projectHandle->findTimeOutputProperty(name);

   if (property != 0) 
   {
      if (m_simulationMode == "HydrostaticDecompaction" and name == "LithoStaticPressure" and
          property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT) 
      {
         return Interface::SEDIMENTS_ONLY_OUTPUT;
      }
      if (name == "HydroStaticPressure" and
          property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT) 
      {
         return Interface::SEDIMENTS_ONLY_OUTPUT;
      }
      
      return property->getOption ();
   }
   
   return Interface::NO_OUTPUT;
}

//------------------------------------------------------------//

bool PropertiesCalculator::allowOutput (const string & propertyName3D,
                                         const Interface::Formation * formation, const Interface::Surface * surface) const {


   string propertyName = propertyName3D;
   
   if (propertyName.find("HeatFlow") != string::npos) 
   {
      propertyName = "HeatFlow";
   } 
   else if (propertyName.find("FluidVelocity") != string::npos) 
   {
      propertyName = "FluidVelocity";
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

   if ((propertyName == "BrineDensity" or  propertyName == "BrineViscosity") and surface != 0) 
   {
      return false;
   }
   if (m_decompactionMode and (propertyName == "BulkDensity") and surface == 0) 
   {
      return false;
   }

   bool basementFormation = formation->kind () == DataAccess::Interface::BASEMENT_FORMATION;

   // The top of the crust is a part of the sediment
   if (basementFormation and surface != 0 and (propertyName == "Depth" or propertyName == "Temperature")) 
   {
      if (dynamic_cast<const GeoPhysics::Formation*>(formation)->isCrust()) 
      {
         if (formation->getTopSurface() and (formation->getTopSurface() == surface)) 
         {
            return true;
         }
      }
   }
   PropertyOutputOption outputOption = checkTimeFilter3D (propertyName);

   if (outputOption == Interface::NO_OUTPUT) 
   {
      return false;
   }
   if (basementFormation) 
   {
      if (outputOption < Interface::SEDIMENTS_AND_BASEMENT_OUTPUT) 
      {
         return false;
      }
   }

   return true;

}

//------------------------------------------------------------//

bool PropertiesCalculator::acquireSnapshots(SnapshotList & snapshots)
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
                     Swap(firstAge, secondAge);
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
void PropertiesCalculator::acquireProperties(Interface::PropertyList & properties) {

   if (m_propertyNames.size() != 0) 
   {
      // remove duplicated names
      std::sort(m_propertyNames.begin(), m_propertyNames.end());
      m_propertyNames.erase(std::unique(m_propertyNames.begin(), m_propertyNames.end(), DerivedProperties::isEqualPropertyName), m_propertyNames.end());

      DerivedProperties::acquireProperties(m_projectHandle, * m_propertyManager, properties, m_propertyNames);
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::acquireFormationsSurfaces(FormationSurfaceVector& formationSurfaceItems) {

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
void PropertiesCalculator::acquireAll2Dproperties() {

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

void PropertiesCalculator::acquireAll3Dproperties() {
   
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
void PropertiesCalculator::printListSnapshots ()  {

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
void PropertiesCalculator::printListStratigraphy () {

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

void PropertiesCalculator::printOutputableProperties () {

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

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
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
bool PropertiesCalculator::setFastcauldronActivityName() {

   if (m_projectHandle->getDetailsOfLastSimulation ("fastcauldron") == 0 ||
       m_projectHandle->getDetailsOfLastSimulation ("fastcauldron")->getSimulatorMode () == "NoCalculaction") 
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


// Methods for visualization format output
//------------------------------------------------------------//

void PropertiesCalculator::updateVizSnapshotsConstantValue() {

   SnapShotList snapShotList = m_vizProject->getSnapShots();
   for (auto& snapShot : snapShotList)  
   {
      DerivedProperties::updateConstantValue(snapShot);
   }
}

//------------------------------------------------------------//
std::shared_ptr<CauldronIO::Project> PropertiesCalculator::createStructureFromProjectHandle(bool verbose) {

   // Get modeling mode
   Interface::ModellingMode modeIn = m_sharedProjectHandle->getModellingMode();
   CauldronIO::ModellingMode mode = modeIn == Interface::MODE1D ? CauldronIO::MODE1D : CauldronIO::MODE3D;

   // setActivityGrid
   if (mode == CauldronIO:: MODE1D) 
   {
      m_sharedProjectHandle->setActivityOutputGrid(m_sharedProjectHandle->getLowResolutionOutputGrid ());
   }
   // Read general project data
   const Interface::ProjectData* projectData = m_sharedProjectHandle->getProjectData();

   // Create the project
   std::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(projectData->getProjectName(), projectData->getDescription(),
                                                                        projectData->getProgramVersion(), mode,
                                                                        xml_version_major, xml_version_minor));
   // Import all snapshots
   ImportProjectHandle import(verbose, project, m_sharedProjectHandle);

   import.checkInputValues();
      
   if (m_rank == 0) 
   {
      // Add migration_io data
      import.addMigrationIO();

      // Add trapper_io data
      import.addTrapperIO();

      // Add trap_io data
      import.addTrapIO();

      // Find genex/shale-gas history files
      import.addGenexHistory();

      // Find burial history files
      import.addBurialHistory();

      // Add reference to massBalance file
      import.addMassBalance();

      // Add 1D tables
      import.add1Ddata();
   }

   if (verbose)
      cout << "Create empty snapshots" << endl;


   std::shared_ptr<Interface::SnapshotList> snapShots;
   snapShots.reset(m_projectHandle->getSnapshots(Interface::MAJOR | Interface::MINOR));

   for (size_t i = 0; i < snapShots->size(); i++)
   {
      const Interface::Snapshot* snapShot = snapShots->at(i);

      // Create a new empty snapshot
      std::shared_ptr<CauldronIO::SnapShot> snapShotIO(new CauldronIO::SnapShot(snapShot->getTime(), DerivedProperties::getSnapShotKind(snapShot), snapShot->getType() == MINOR));

      // Add to project
      project->addSnapShot(snapShotIO);
   }

   return project;
}

//------------------------------------------------------------//
bool PropertiesCalculator::copyFiles() {

   if (not H5_Parallel_PropertyList::isPrimaryPodEnabled () or (H5_Parallel_PropertyList::isPrimaryPodEnabled () and not m_copy)) 
   {
      return true;
   }

   int rank;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

   if (rank != 0) return true;

   PetscBool noFileRemove = PETSC_FALSE;
   PetscOptionsHasName(PETSC_NULL, "-noremove", &noFileRemove);

   PetscLogDouble StartMergingTime;
   PetscTime(&StartMergingTime);
   bool status = true;

   const std::string& directoryName = m_projectHandle->getOutputDir ();

   PetscPrintf (PETSC_COMM_WORLD, "Copy output files ...\n");

   SnapshotList * snapshots = m_projectHandle->getSnapshots(MAJOR | MINOR);
   SnapshotList::iterator snapshotIter;

   for (snapshotIter = snapshots->begin(); snapshotIter != snapshots->end(); ++snapshotIter) 
   {
      const Interface::Snapshot * snapshot = *snapshotIter;

      if (snapshot->getFileName () == "") 
      {
         continue;
      }
      ibs::FilePath filePathName(m_projectHandle->getProjectPath ());
      filePathName << directoryName << snapshot->getFileName ();

      displayProgress(snapshot->getFileName (), StartMergingTime, "Copy ");

      status = H5_Parallel_PropertyList::copyMergedFile(filePathName.path(), false);

      // delete the file in the shared scratch
      if (status and not noFileRemove) 
      {
         ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName());
         fileName << filePathName.cpath ();

         int status = std::remove(fileName.cpath()); //c_str ());
         if (status == -1)
            cerr << fileName.cpath() << " Basin_Warning:  Unable to remove snapshot file, because '"
                 << std::strerror(errno) << "'" << endl;
      }
   }

   string fileName = m_activityName + "_Results.HDF" ;
   ibs::FilePath filePathName(m_projectHandle->getProjectPath ());
   filePathName <<  directoryName << fileName;

   displayProgress(fileName, StartMergingTime, "Copy ");

   status = H5_Parallel_PropertyList::copyMergedFile(filePathName.path(), false);

   // remove the file from the shared scratch
   if (status and  not noFileRemove) 
   {

    ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName());
      fileName << filePathName.cpath ();
      int status = std::remove(fileName.cpath());

      if (status == -1) 
      {
         cerr << fileName.cpath () << " Basin_Warning:  Unable to remove file, because '"
              << std::strerror(errno) << "'" << endl;
      }

     // remove the output directory from the shared scratch
      ibs::FilePath dirName(H5_Parallel_PropertyList::getTempDirName());
      dirName << directoryName;

      displayProgress(dirName.path(), StartMergingTime, "Removing remote output directory ");
      status = std::remove(dirName.cpath());

      if (status == -1)
         cerr << dirName.cpath () << " Basin_Warning:  Unable to remove the directory, because '"
              << std::strerror(errno) << "'" << endl;
   }

   if (status) 
   {
      displayTime(StartMergingTime, "Total merging time: ");
   }
   else 
   {
      PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: Could not merge the file %s.\n", filePathName.cpath());
   }

   delete snapshots;

   return status;
}
//------------------------------------------------------------//
bool PropertiesCalculator::createVizSnapshotResultPropertyValue (OutputPropertyValuePtr propertyValue, 
                                                                 const Snapshot* snapshot, const Interface::Formation * formation,
                                                                 const Interface::Surface * surface) {
   
   if (not propertyValue->hasMap ()) 
   {
      return true;
   }

   unsigned int p_depth = propertyValue->getDepth();
   
   if (p_depth > 1 and surface == 0) 
   {
      //if (not propertyValue->isPrimary()) { // uncomment to convert only derived properties
      if (propertyValue->getProperty ()->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY) 
      {
         createVizSnapshotResultPropertyValueContinuous (propertyValue, snapshot, formation);
      }
      else if (propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY) 
      {
         createVizSnapshotResultPropertyValueDiscontinuous (propertyValue, snapshot, formation);
      } 
      else 
      {
         return false;
      }
      // else {//  the property is already in the output file }
      
   //} else if (not propertyValue->isPrimary() and not (surface == 0 and propertyValue->isPrimary())) { // uncomment to convert only derived properties
   } 
   else 
   {
      return createVizSnapshotResultPropertyValueMap (propertyValue, snapshot, formation, surface);
   }

   return true;

}
//------------------------------------------------------------//

void PropertiesCalculator::createVizSnapshotFormationData(const Snapshot * snapshot, const FormationSurface & formationItem, 
                                                           DataAccess::Interface::PropertyList & properties,
                                                           SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues) {
   
   Interface::PropertyList::iterator propertyIter;

   const Interface::Formation * formation = formationItem.first;
   const Interface::Surface * surface = formationItem.second;


   for (propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter) 
   {
      const Interface::Property * property = *propertyIter;

      OutputPropertyValuePtr propertyValue = allOutputPropertyValues[ snapshot ][ formationItem ][ property ];

      if (propertyValue != 0)  
      {
         if (not m_minorPrimary or (snapshot->getType() == DataAccess::MAJOR or (snapshot->getType() == DataAccess::MINOR and property->isPrimary()))) 
         {
            printDebugMsg (" Output property avaiable for" , property, formation, surface,  snapshot);
 
            createVizSnapshotResultPropertyValue (propertyValue, snapshot, formation, surface);
         }
      }
      else
      {
         printDebugMsg (" No property avaiable for" , property, formation, surface, snapshot);
      }
   }
}
//------------------------------------------------------------//

bool PropertiesCalculator::createVizSnapshotResultPropertyValueContinuous (OutputPropertyValuePtr propertyValue, 
                                                                           const Snapshot* snapshot, const Interface::Formation * daFormation) {
   

   bool debug = false;

   const string propName = propertyValue->getName();
   //find info and geometry for the formation
   std::shared_ptr<CauldronIO::FormationInfo> info;
   size_t maxK = 0;
   size_t minK = std::numeric_limits<size_t>::max();
   
   for (size_t i = 0; i < m_formInfoList->size(); ++i)  
   {
      std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = m_formInfoList->at(i);

      maxK = max(maxK, depthInfo->kEnd);
      minK = min(minK, depthInfo->kStart);
      if (depthInfo->formation == daFormation) 
      {
         info = m_formInfoList->at(i);
      }
   }
   size_t depthK = 1 + maxK - minK;
   if (not info) 
   {
      PetscPrintf(PETSC_COMM_WORLD, "Continuous property %s: %s formation not found\n", propName.c_str(), (daFormation->getName()).c_str());
      return false;
   } 
   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty(propertyValue, CauldronIO::Continuous3DProperty);

   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, snapshot->getTime());
    
   if (snapshot->getTime() == 0) 
   {
      std::shared_ptr<CauldronIO::Formation> vizFormation = findOrCreateFormation (info);
   } 
   
   // find or create snapshot volume and geometry
   std::shared_ptr<CauldronIO::Volume> snapshotVolume = vizSnapshot->getVolume();
   std::shared_ptr< CauldronIO::Geometry3D> geometry;

   shared_ptr<CauldronIO::VolumeData> volDataNew;
   bool propertyVolumeExisting = false;
   
   const DataModel::AbstractGrid* grid = propertyValue->getGrid();
   if (not snapshotVolume) 
   {
      // create volume and geometry
      snapshotVolume.reset(new CauldronIO::Volume(CauldronIO::None));
      vizSnapshot->setVolume(snapshotVolume);
      geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
      m_vizProject->addGeometry(geometry);
      if (debug) 
      {
         cout << "Adding Geomerty and volume " << " depth = " << depthK << " mink= " << minK << endl;
      }
   } 
   else 
   {
      // find volume the property
      for (size_t i = 0; i <  snapshotVolume->getPropertyVolumeDataList().size(); ++i) 
      {
         CauldronIO::PropertyVolumeData& pdata = snapshotVolume->getPropertyVolumeDataList().at(i); 
         if (pdata.first->getName() == propName) 
         {
            volDataNew = pdata.second;
            if (volDataNew) 
            {
               propertyVolumeExisting = true;
            }
            break;
         }
      }
      if (propertyVolumeExisting) 
      {
         // get geomerty for the existing property volume
         geometry = volDataNew->getGeometry();
      }
      else 
      {
         // get geometry for the first property (assume the same geometry for derived properties)
         if (snapshotVolume->getPropertyVolumeDataList().size() > 0) 
         {
            geometry = snapshotVolume->getPropertyVolumeDataList().at(0).second->getGeometry();
         }
         else 
         {
            geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                      grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
            m_vizProject->addGeometry(geometry);
         }
      }
   }

   if (debug) 
   {
      cout << info->numI << " " << info->numJ << " " << info->kEnd  << " " << info->kStart << " " <<  
         info->deltaI << " " <<  info->deltaJ << " " <<  info->minI << " " <<  info->minJ << endl;
      cout << daFormation->getName() << " snapshot " << snapshot->getTime() << " geometry3d size = " << geometry->getSize() << " numk = " << geometry->getNumK() << endl;
   }
   
   float * internalData = 0;

   int firstK = static_cast<int>(info->kStart);
   int lastK  = static_cast<int>(info->kEnd);

   if (not propertyVolumeExisting) 
   {
      volDataNew.reset(new CauldronIO::VolumeDataNative(geometry, CauldronIO::DefaultUndefinedValue));

      unsigned int dataSize = geometry->getNumI() * geometry->getNumJ() * geometry->getNumK();
      if (dataSize > m_data.size()) 
      {
         m_data.resize(dataSize);
      }
      float *data = &m_data[0];
      memset(data, 0, sizeof(float) * dataSize);
      
      volDataNew->setData_IJK(data);
      internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      
   } 
   else 
   {
      internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      if (debug) 
      {
         cout << "Adding to existing voldata" << snapshot->getTime() << " " << propName << " " << daFormation->getName() << " kstart= " << info->kStart << " kend= " << info->kEnd << endl;
      }
   }
   if (not internalData) 
   {
      return false;
   }

   propertyValue->retrieveData();

   float minValue = std::numeric_limits<float>::max();
   float maxValue = -minValue;

   // Write the formation part of a continuous property

   // Do not re-write the bottom value of the sediment - some continous properties are not calculated for the basement
   int fK = (daFormation->kind () == BASEMENT_FORMATION and firstK > 0 ? firstK + 1 : firstK);

   for (int j = grid->firstJ (false); j <= grid->lastJ (false); ++j) 
   {
      for (int i = grid->firstI(false); i <= grid->lastI (false); ++i) 
      {
         unsigned int pk = 0;
         for (int k = lastK; k >= fK; --k, ++ pk) 
         {
            float value = static_cast<float>(propertyValue->getValue(i, j, pk));
            
            if (value != CauldronIO::DefaultUndefinedValue)
            {
               minValue = std::min(minValue, value);
               maxValue = std::max(maxValue, value);
            }

            internalData[volDataNew->computeIndex_IJK(i, j, k)] = value;
         }
      }
   }
   propertyValue->restoreData ();
   
   if (daFormation->kind() == SEDIMENT_FORMATION)
   {
      float sedimentMinValue = volDataNew->getSedimentMinValue();
      float sedimentMaxValue = volDataNew->getSedimentMaxValue();
      
      if (
         sedimentMinValue == DefaultUndefinedValue && 
         sedimentMaxValue == DefaultUndefinedValue)
      {
         volDataNew->setSedimentMinMax(minValue, maxValue);
      }
      else
      {
         volDataNew->setSedimentMinMax(
                                       std::min(minValue, sedimentMinValue),
                                       std::max(maxValue, sedimentMaxValue));
      }
   }
   
   if (not propertyVolumeExisting) 
   {
      CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
      snapshotVolume->addPropertyVolumeData(propVolDataNew);
      
      if (debug) 
      {
         cout << "Adding new voldata to " << snapshot->getTime() << " " << propName << " " << daFormation->getName() << " kstart= " << info->kStart << " kend= " << info->kEnd << endl;
         cout << "kstart= " << info->kStart << " kend= " << info->kEnd <<  endl;
      }
   }


   return true;
}
//------------------------------------------------------------//
bool PropertiesCalculator::createVizSnapshotResultPropertyValueDiscontinuous (OutputPropertyValuePtr propertyValue, 
                                                                              const Snapshot* snapshot, const Interface::Formation * daFormation) {
      
   bool debug = false;

   const string propName = propertyValue->getName();
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, snapshot->getTime());
   
   std::shared_ptr<CauldronIO::FormationInfo> info;

   for (size_t i = 0; i < m_formInfoList->size(); ++i)  
   {
      std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = m_formInfoList->at(i);

      if (depthInfo->formation == daFormation) 
      {
         info = m_formInfoList->at(i);
      }
   }
   if (not info) 
   {
      PetscPrintf(PETSC_COMM_WORLD, "Discontinuous property %s: %s formation not found\n", propName.c_str(), (daFormation->getName()).c_str());
      return false;
   } 

   std::shared_ptr< CauldronIO::Formation> vizFormation = findOrCreateFormation (info);

   //create geometry
   const DataModel::AbstractGrid* grid = propertyValue->getGrid();
   const std::shared_ptr<CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), propertyValue->getDepth(), info->kStart,
                                                                                     grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
   m_vizProject->addGeometry(geometry);

   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty (propertyValue, CauldronIO::Discontinuous3DProperty);
   
   // create volume data
   shared_ptr<CauldronIO::VolumeData> volDataNew(new CauldronIO::VolumeDataNative(geometry));

   unsigned int dataSize = geometry->getNumI() * geometry->getNumJ() * geometry->getNumK();
   if (m_data.size() < dataSize) 
   {
      m_data.resize(dataSize);
   }
   float * data = &m_data[0];
   memset(data, 0, sizeof(float) * dataSize);

   propertyValue->retrieveData();

   volDataNew->setData_IJK(data);
   float * internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      
   if (not internalData) 
   {
      return false;
   }

   int firstK = static_cast<int>(geometry->getFirstK());
   int lastK  = static_cast<int>(geometry->getFirstK() + geometry->getNumK() - 1);

   for (int i = grid->firstI(false); i <= grid->lastI (false); ++i) 
   {
      for (int j = grid->firstJ (false); j <= grid->lastJ (false); ++j) 
      {
         unsigned int pk = 0;
         for (int k = lastK; k >= firstK; --k, ++ pk) 
         {
            internalData[volDataNew->computeIndex_IJK(i, j, k)] = static_cast<float>(propertyValue->getValue(i, j, pk));
         }
      }
   }

   propertyValue->restoreData ();

   int rank;
   MPI_Comm_rank (PETSC_COMM_WORLD, &rank);

   MPI_Reduce((void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);

   if (rank == 0) 
   {
      std::memcpy(internalData, data, dataSize * sizeof(float));
   }

   DerivedProperties::updateVolumeDataConstantValue(volDataNew);
   
   CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
   CauldronIO::SubsurfaceKind formationKind = (daFormation->kind() == Interface::BASEMENT_FORMATION ? CauldronIO::Basement : CauldronIO::Sediment);

   CauldronIO::FormationVolumeList& formVolList = const_cast<CauldronIO::FormationVolumeList&>(vizSnapshot->getFormationVolumeList());
   shared_ptr<CauldronIO::Volume> vol;

   bool existingVolume = false;
   if (formVolList.size() > 0) 
   {
      for (auto& volume : formVolList) 
      {
         
         if (volume.first != 0 and volume.first->getName() == daFormation->getName()) 
         {
            vol = volume.second;
            existingVolume = true;
         }
      }
   }
   if (not vol) 
   {
      vol = shared_ptr<CauldronIO::Volume>(new CauldronIO::Volume(formationKind));
   }
   vol->addPropertyVolumeData(propVolDataNew);
   
   if (not existingVolume) 
   {
      CauldronIO::FormationVolume formVol = CauldronIO::FormationVolume(vizFormation, vol);
      vizSnapshot->addFormationVolume(formVol);
   }
   return true;
}

//------------------------------------------------------------//
bool PropertiesCalculator::createVizSnapshotResultPropertyValueMap (OutputPropertyValuePtr propertyValue, 
                                                                    const Snapshot* snapshot, const Interface::Formation * daFormation,
                                                                    const Interface::Surface * daSurface) {
   
   bool debug = false;

   const string propName = propertyValue->getName();
   const string surfaceName = (daSurface != 0 ? daSurface->getName() : "");

   // Create geometry
   const DataModel::AbstractGrid* grid = propertyValue->getGrid();
   std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(grid->numIGlobal(), grid->numJGlobal(), grid->deltaI(), 
                                                                                     grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
   m_vizProject->addGeometry(geometry);
   
   // find/create a property
   PropertyAttribute attrib = (daSurface != 0 ? CauldronIO::Surface2DProperty : CauldronIO::Formation2DProperty);
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty (propertyValue,  attrib);


   // find/create snapshot
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, snapshot->getTime());
   if (not vizSnapshot) 
   {
      //create snapshot
      CauldronIO::SnapShotKind kind = DerivedProperties::getSnapShotKind(snapshot);
      
      vizSnapshot.reset(new CauldronIO::SnapShot(snapshot->getTime(), kind, snapshot->getType() == MINOR));
      m_vizProject->addSnapShot(vizSnapshot);
   } 

   // find/create snapshot surface
   std::shared_ptr<CauldronIO::Surface> vizSurface;
   CauldronIO::SubsurfaceKind kind = CauldronIO::None;
   
   std::shared_ptr< CauldronIO::Formation> vizFormation = m_vizProject->findFormation(daFormation->getName());
 
   if (not vizFormation->isDepthRangeDefined()) 
   {
      //find info and geometry for the formation
      std::shared_ptr<CauldronIO::FormationInfo> info;
      for (size_t i = 0; i < m_formInfoList->size(); ++i)  
      {
         std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = m_formInfoList->at(i);
         
         if (depthInfo->formation == daFormation) 
         {
            info = m_formInfoList->at(i);
         }
      }
      vizFormation->updateK_range(static_cast<int>(info->kStart), static_cast<int>(info->kEnd));
      if (debug) 
      {
         cout << "update krange map " <<  daFormation->getName()<< " " << info->kStart << " " << info->kEnd << endl;
      }
   }
  
   if (daSurface != 0) 
   {
      vizSurface = getSurface(vizSnapshot, surfaceName);
      if (not vizSurface) 
      {
         if (daSurface->kind() == BASEMENT_SURFACE) kind = CauldronIO::Basement;
         else kind = CauldronIO::Sediment;
         vizSurface.reset(new CauldronIO::Surface(surfaceName,  kind));
         // assign formations
         if (daSurface->getTopFormation() != 0) 
         {
            std::shared_ptr<CauldronIO::Formation> topFormation = m_vizProject->findFormation(daSurface->getTopFormation()->getName());
            vizSurface->setFormation(topFormation.get(), true);
          }
         if (daSurface->getBottomFormation() != 0) 
         {
            std::shared_ptr<CauldronIO::Formation> bottomFormation = m_vizProject->findFormation(daSurface->getBottomFormation()->getName());
            vizSurface->setFormation(bottomFormation.get(), false);
         }
         
         vizSnapshot->addSurface(vizSurface);
         if (debug) 
         {
           cout << "Adding surface " << surfaceName << endl;
         }
      }
      else 
      {
         if (debug) 
         {
            cout << "Found surface " << surfaceName << endl;
         }
      }
   }
   else 
   {
      // find/create surface for formation map
      vizSurface = getSurface(vizSnapshot, vizFormation);
      if (not vizSurface) 
      {
         vizSurface.reset(new CauldronIO::Surface(surfaceName,  kind));
         
         vizSurface->setFormation(vizFormation.get(), true);
         vizSurface->setFormation(vizFormation.get(), false);
         vizSnapshot->addSurface(vizSurface);
         if (debug) 
         {
            cout << "Adding surface " << surfaceName << " for formation " << daFormation->getName() << endl;
         }
      } 
      else 
      {
         if (debug) 
         {
            cout << "Found surface " << surfaceName << " as formation " << daFormation->getName() << endl;
         }
      }
   }

   // copy property data
   unsigned int p_depth = propertyValue->getDepth();
   unsigned int kIndex = 0;
   if (daSurface != 0 and daSurface == daFormation->getTopSurface ()) 
   {
      kIndex = p_depth - 1;
   }
   
   assert(p_depth == 1);

   unsigned int dataSize = geometry->getNumI() * geometry->getNumJ();
   if (m_data.size() < dataSize) 
   {
      m_data.resize(dataSize);
   }

   float * data = &m_data[0]; 
   memset(data, 0, sizeof(float) * dataSize);

   // assign surface map
   std::shared_ptr< CauldronIO::SurfaceData> valueMap(new CauldronIO::MapNative(geometry));
   valueMap->setData_IJ(data); 

   float * internalData = const_cast<float *>(valueMap->getSurfaceValues());
   propertyValue->retrieveData();
   for (int i = grid->firstI(false); i <= grid->lastI (false); ++i) 
   {
      for (int j = grid->firstJ (false); j <= grid->lastJ (false); ++j) 
      {
         internalData[valueMap->getMapIndex(i, j)] = static_cast<float>(propertyValue->getValue(i, j, kIndex));
      }
   }
   propertyValue->restoreData ();

   int rank;
   MPI_Comm_rank (PETSC_COMM_WORLD, &rank);

   MPI_Reduce((void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);
   if (rank == 0) 
   {
      std::memcpy(internalData, data, dataSize * sizeof(float));
   }

   CauldronIO::PropertySurfaceData propSurface = CauldronIO::PropertySurfaceData(vizProperty, valueMap);

   if (daSurface == 0 or propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY) 
   {
      valueMap->setFormation(vizFormation);
   }

   if (propertyValue->getReservoir() != 0) 
   {
      const Interface::Reservoir* daReservoir = dynamic_cast<const Interface::Reservoir *>(propertyValue->getReservoir());
     
      if (daReservoir != 0 ) 
      {
         std::shared_ptr<const CauldronIO::Reservoir> vizReservoir = findOrCreateReservoir(daReservoir, vizFormation);
         valueMap->setReservoir(vizReservoir);
      }
   }

   vizSurface->addPropertySurfaceData(propSurface);

   if (debug) 
   {
      if (propertyValue->getProperty ()->getPropertyAttribute () != DataModel::DISCONTINUOUS_3D_PROPERTY) 
      {
         cout << snapshot->getTime() << ": Added " << propName << " " << (daSurface ? daSurface->getName() : daFormation->getName()) << 
            " count= " << vizSurface->getPropertySurfaceDataList().size() << endl;
      } 
      else 
      {
         cout << snapshot->getTime() << ": Added " << propName << " " << daSurface->getName() << " and " <<  daFormation->getName() << 
            " count= " << vizSurface->getPropertySurfaceDataList().size() << endl;
      }
   }

   return true;
}

//------------------------------------------------------------//
void PropertiesCalculator::updateFormationsKRange() {

   // Compute k-range for all stratigraphy formations at present day
   const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot(0);
   std::shared_ptr<CauldronIO::FormationInfoList> depthFormations = getDepthFormations(m_projectHandle, zeroSnapshot);

   // Capture global k-range
   for (unsigned int i = 0; i < depthFormations->size(); ++i)
   {
       std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
       std::shared_ptr<CauldronIO::Formation> vizFormation = findOrCreateFormation (info);
    }
}
//------------------------------------------------------------//
void PropertiesCalculator::collectVolumeData(const std::shared_ptr<CauldronIO::SnapShot>& snapshot) {

   const std::shared_ptr<Volume> volume = snapshot->getVolume();
   if (volume)  
   {
      PropertyVolumeDataList& propVolList = volume->getPropertyVolumeDataList();
      if (propVolList.size() > 0) 
      {
         int rank;
         MPI_Comm_rank (PETSC_COMM_WORLD, &rank);
 
         for (auto& propVolume : propVolList) 
         {
            std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
            if (valueMap->isRetrieved()) 
            {
               std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();
               unsigned int dataSize = geometry->getNumI() * geometry->getNumJ() * geometry->getNumK();
               if (dataSize > m_data.size()) 
               {
                  m_data.resize(dataSize);
               }
               float * data = &m_data[0];
               
               float * internalData = const_cast<float *>(valueMap->getVolumeValues_IJK());
               
               // Collect the data from all processors on rank 0
               MPI_Reduce((void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);
              
               if (rank == 0) 
               {
                  std::memcpy(internalData, data, dataSize * sizeof(float));
               }

               // Find the global min and max sediment values and set on rank 0
               float globalValues[2];
               float localValues[2];

               localValues[0] = valueMap->getSedimentMinValue();
               localValues[1] = valueMap->getSedimentMaxValue();

               MPI_Reduce(localValues, globalValues, 2, MPI_FLOAT, m_op, 0, PETSC_COMM_WORLD);
               if (rank == 0) 
               {
                  valueMap->setSedimentMinMax(globalValues[0], globalValues[1]);
               }
            } 
         }
      }
   }
}

//------------------------------------------------------------//
shared_ptr<CauldronIO::Formation> PropertiesCalculator::findOrCreateFormation(std::shared_ptr<CauldronIO::FormationInfo>& info) {

   std::shared_ptr<CauldronIO::Formation> vizFormation = m_vizProject->findFormation (info->formation->getName());
   if (vizFormation) 
   {
      if (not vizFormation->isDepthRangeDefined()) 
      {
         vizFormation->updateK_range (static_cast<int>(info->kStart), static_cast<int>(info->kEnd));
      }
   }
   else
   {
      vizFormation.reset(new CauldronIO::Formation (static_cast<int>(info->kStart), static_cast<int>(info->kEnd), info->formation->getName()));
      m_vizProject->addFormation(vizFormation);         
   }
   return vizFormation;
}

//------------------------------------------------------------//
shared_ptr<const CauldronIO::Property> PropertiesCalculator::findOrCreateProperty(OutputPropertyValuePtr propertyValue, PropertyAttribute attrib) {

   const Interface::Property* daProperty = dynamic_cast<const Interface::Property *>(propertyValue->getProperty());
   const string propName = propertyValue->getName();
   string propertyMapName = (daProperty != 0 ? daProperty->getCauldronName() : propName);
   string propertyUserName = (daProperty != 0 ? daProperty->getUserName() : propName);
   const string unit = (daProperty != 0 ? daProperty->getUnit() : "");

   if (propertyMapName == "HorizontalPermeability") propertyMapName = "PermeabilityHVec2";
   if (propertyValue->getName() == "Reflectivity")  propertyMapName = "Reflectivity";
  
   shared_ptr<const CauldronIO::Property> vizProperty;
   for (auto& property : m_vizProject->getProperties()) 
   {
      if (property->getName() == propName) 
      {
         vizProperty = property;
         break;
      }
   }
   if (not vizProperty) 
   {
      CauldronIO::PropertyType type = (daProperty->getType() == RESERVOIRPROPERTY ? CauldronIO::ReservoirProperty : CauldronIO::FormationProperty);
      vizProperty.reset(new CauldronIO::Property(propName, propertyUserName, propertyMapName, unit, type, attrib));
      m_vizProject->addProperty(vizProperty);
   }

   return vizProperty;
}

//------------------------------------------------------------//
std::shared_ptr<const CauldronIO::Reservoir> PropertiesCalculator::findOrCreateReservoir(const Interface::Reservoir* reserv, std::shared_ptr<const CauldronIO::Formation> formationIO)
{
   for (auto& reservoir : m_vizProject->getReservoirs())
   {
      if (reservoir->getName() == reserv->getName() && reservoir->getFormation() == formationIO)
         return reservoir;
   }

   std::shared_ptr<const CauldronIO::Reservoir> reservoirIO(new CauldronIO::Reservoir(reserv->getName(), formationIO));
   m_vizProject->addReservoir(reservoirIO);
   
   return reservoirIO;
}

//------------------------------------------------------------//

bool PropertiesCalculator::parseCommandLine(int argc, char ** argv) {

   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp(argv[ arg ], "-properties", Max(5, (int)strlen(argv[ arg ]))) == 0)
      {
         if (arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-')
         {
            showUsage(argv[ 0 ], "Argument for '-properties' is missing");

            return false;
         }
         if (!parseStrings(m_propertyNames, argv[ ++arg ]))
         {
            showUsage(argv[ 0 ], "Illegal argument for '-properties'");

            return false;
         }
      }
      else if (strncmp(argv[ arg ], "-ages", Max(2, (int)strlen(argv[ arg ]))) == 0)
      {
         if (arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-')
         {
            showUsage(argv[ 0 ], "Argument for '-ages' is missing");

            return false;
         }
         if (!parseAges(m_ages, argv[ ++arg ]))
         {
            showUsage(argv[ 0 ], "Illegal argument for '-ages'");

            return false;
         }
      }
      else if (strncmp(argv[ arg ], "-formations", Max(5, (int)strlen(argv[ arg ]))) == 0)
      {
         if (arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-')
         {
            showUsage(argv[ 0 ], "Argument for '-formations' is missing");

            return false;
         }
         if (!parseStrings(m_formationNames, argv[ ++arg ]))
         {
            showUsage(argv[ 0 ], "Illegal argument for '-formations'");

            return false;
         }
      }
      else if (strncmp(argv[ arg ], "-nobasement", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {
         m_basement = false;
      }
      else if (strncmp(argv[ arg ], "-minor", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {
         m_snapshotsType = MAJOR | MINOR;
      }
      else if (strncmp(argv[ arg ], "-pminor", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {
         m_snapshotsType = MAJOR | MINOR;
         m_minorPrimary = true;
      }
      else if (strncmp(argv[ arg ], "-major", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {
      }
      else if (strncmp(argv[ arg ], "-all-2D-properties", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_all2Dproperties = true;
      }
      else if (strncmp(argv[ arg ], "-all-3D-properties", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_all3Dproperties = true;
      }
      else if (strncmp(argv[ arg ], "-project-properties", Max(12, (int)strlen(argv[ arg ]))) == 0)
      {
         m_projectProperties = true;
      }
      else if (strncmp(argv[ arg ], "-list-properties", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_listProperties = true;
      }
      else if (strncmp(argv[ arg ], "-list-snapshots", Max(8, (int)strlen(argv[ arg ]))) == 0)
      {
         m_listSnapshots = true;
      }
      else if (strncmp(argv[ arg ], "-list-stratigraphy", Max(8, (int)strlen(argv[ arg ]))) == 0)
      {
         m_listStratigraphy = true;
      }
      else if (strncmp(argv[ arg ], "-project", Max(5, (int)strlen(argv[ arg ]))) == 0)
      {
         if (arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-')
         {
            showUsage(argv[ 0 ], "Argument for '-project' is missing");
            return false;
         }
         m_projectFileName = argv[ ++arg ];
      }
      else if (strncmp(argv[ arg ], "-copy", Max(4, (int)strlen(argv[ arg ]))) == 0)
      {
         m_copy = true;
      }
      else if (strncmp(argv[ arg ], "-noremove", Max(4, (int)strlen(argv[ arg ]))) == 0)
      {
      }
      else if (strncmp(argv[ arg ], "-debug", Max(2, (int)strlen(argv[ arg ]))) == 0)
      {
         m_debug = true;
      }
      else if (strncmp(argv[ arg ], "-help", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {
         showUsage(argv[ 0 ], " Standard usage.");

         return false;
      }
      else if (strncmp(argv[ arg ], "-?", Max(2, (int)strlen(argv[ arg ]))) == 0)
      {
         showUsage(argv[ 0 ], " Standard usage.");

         return false;
      }
      else if (strncmp(argv[ arg ], "-usage", Max(2, (int)strlen(argv[ arg ]))) == 0)
      {
         showUsage(argv[ 0 ], " Standard usage.");

         return false;
      }
      else if (strncmp(argv[ arg ], "-", 1) != 0)
      {
         m_projectFileName = argv[ arg ];
      }
      else if (strncmp(argv[ arg ], "-viz", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_vizFormat = true;
      }
      else if (strncmp(argv[ arg ], "-hdf", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_vizFormatHDF = true;
      }
      else if (strncmp(argv[ arg ], "-hdfonly", Max(7, (int)strlen(argv[ arg ]))) == 0)
      {
         m_vizFormatHDFonly = true;
      }
      else if (strncmp(argv[ arg ], "-listXml", Max(6, (int)strlen(argv[ arg ]))) == 0)
      {
         m_vizListXml = true;
      }
      else if (strncmp(argv[ arg ], "-ddd", Max(3, (int)strlen(argv[ arg ]))) == 0)
      {

      }
      else if (strncmp(argv[ arg ], "-primaryPod", Max(8, (int)strlen(argv[ arg ]))) == 0)
      {
         ++ arg;
         m_primaryPod = true;
      }
      else if (strncmp(argv[ arg ], "-extract2D", Max(6, (int)strlen(argv[ arg ]))) == 0)
      {
         m_extract2D = true;
      }
      else if (strncmp(argv[ arg ], "-save", Max(4, (int)strlen(argv[ arg ]))) == 0)
      {
         ++ arg;

      }
      else if (strncmp(argv[ arg ], "-verbosity", Max(4, (int)strlen(argv[ arg ]))) == 0)
      {
         ++ arg;

      }
      else
      {
         LogHandler(LogHandler::ERROR_SEVERITY) << "Unknown or ambiguous option: " << argv[ arg ];
         showUsage(argv[ 0 ]);

         return false;
      }
   }


   if (m_projectFileName == "")
   {
      showUsage(argv[ 0 ], "No project file specified");

      return false;
   }
   if (m_projectProperties) 
   {
      m_all3Dproperties = true;
      m_all2Dproperties = true;
      // Do not extract and save 2D maps from 3d data, define -extract2D option
      // m_extract2D = false;
      m_basement = true;
   }

   if (m_vizFormatHDFonly or m_vizListXml) 
   {
      int numberOfRanks;
      
      MPI_Comm_size (PETSC_COMM_WORLD, &numberOfRanks);
      if (numberOfRanks > 1) 
      {
         PetscPrintf(PETSC_COMM_WORLD, "Unable to convert data to Visualization format. Please select 1 core.\n");
         return false;
      }
   }

   for (int i = 1; i < argc; ++i) 
   {
      m_commandLine += std::string (argv [ i ]) + (i == argc - 1 ? "" : " ");
   }

   return true;
}

//------------------------------------------------------------//
void PropertiesCalculator::startTimer() {
   PetscTime(&m_startTime);

}
//------------------------------------------------------------//

bool PropertiesCalculator::hdfonly() const {
   return m_vizFormatHDFonly;
}
//------------------------------------------------------------//

bool PropertiesCalculator::listXml() const {
   return m_vizListXml;
}

//------------------------------------------------------------//

bool PropertiesCalculator::vizFormat() const {
   return m_vizFormat;
}


//------------------------------------------------------------//

void PropertiesCalculator::showUsage(const char* command, const char* message)
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
           << "\t[-verbosity level]                         verbosity level of the log file(s): minimal|normal|detailed|diagnostic. Default value is 'normal'." << endl
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
           << "\t[-help]                                    print this message and exit" << endl << endl
           << "Options for visualization format:" << endl << endl
           << "\t[-viz]                                     calculate the properties and convert to visialization format." << endl << endl
           << "\t[-hdfonly]                                 convert visulization format back to hdf format." << endl << endl
           << "\t[-listXml]                                 list of properties in xml file." << endl << endl
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

/// destructive!!!
bool splitString(char * string, char separator, char * & firstPart, char * & secondPart)
{
   if (!string || strlen(string) == 0) return false;
   secondPart = strchr(string, separator);
   if (secondPart != 0)
   {
      *secondPart = '\0';
      ++secondPart;
      if (*secondPart == '\0') secondPart = 0;
   }

   firstPart = string;
   if (strlen(firstPart) == 0) return false;
   return true;
}

bool parseStrings(StringVector & strings, char * stringsString)
{
   char * strPtr = stringsString;
   char * section;
   while (splitString(strPtr, ',', section, strPtr))
   {
      strings.push_back(string(section));
   }
   return true;
}


bool parseAges(DoubleVector & ages, char * agesString)
{
   char * strPtr = agesString;
   char * commasection;
   char * section;
   while (splitString(strPtr, ',', commasection, strPtr))
   {
      while (splitString(commasection, '-', section, commasection))
      {
         double number = atof(section);
         ages.push_back(number);
      }
      ages.push_back(-1); // separator
   }
   ages.push_back(-1); // separator
   return true;
}

bool snapshotIsEqual(const Snapshot * snapshot1, const Snapshot * snapshot2)
{
   return snapshot1->getTime() == snapshot2->getTime();
}

bool snapshotSorter(const Snapshot * snapshot1, const Snapshot * snapshot2)
{
   return snapshot1->getTime() > snapshot2->getTime();
}

void displayProgress(const string & fileName, double startTime, const string & message) {

   if (PetscGlobalRank != 0)
      return;
   double EndTime;

   PetscTime(&EndTime);

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

void displayTime (const double timeToDisplay, const char * msgToDisplay) {

   int hours   = (int)(timeToDisplay / 3600.0);
   int minutes = (int)((timeToDisplay - (hours * 3600.0)) / 60.0);
   int seconds = (int)(timeToDisplay - hours * 3600.0 - minutes * 60.0);

   PetscPrintf (PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
   PetscPrintf (PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds);
   PetscPrintf (PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

}
