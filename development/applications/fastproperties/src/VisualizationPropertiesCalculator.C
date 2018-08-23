//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sys/stat.h>

#include "VisualizationPropertiesCalculator.h"

#include "ExportToHDF.h"
#include "FolderPath.h"

// DataAccess library
#include "Interface/ProjectData.h"
#include "Interface/Reservoir.h"
#include "Interface/Trapper.h"

// VisualizationIO library
#include "VisualizationAPI.h"
#include "ImportProjectHandle.h"
#include "ImportFromXML.h"

//-------------------------------------------------------------//
VisualizationPropertiesCalculator::VisualizationPropertiesCalculator(int aRank) : AbstractPropertiesCalculator (aRank)
{
   m_vizFormatHDFonly = false;
   m_vizListXml       = false;
   m_minorPrimary     = false;

   m_export = 0;

   m_localIndices[0]  = m_localIndices[1] = 0;
   m_globalIndices[0] = m_globalIndices[1] = 0;
   m_minValue = m_maxValue = CauldronIO::DefaultUndefinedValue;

   m_attribute = DataModel::UNKNOWN_PROPERTY_OUTPUT_ATTRIBUTE;
}

//------------------------------------------------------------//
VisualizationPropertiesCalculator::~VisualizationPropertiesCalculator() {

   if (m_export != 0) delete m_export;
   m_export = 0;

   m_existingProperties.clear();
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::createXML() {
   
   if (not m_fileNameExistingXml.empty()) 
   { 
      // import existing xml
      m_vizProject = CauldronIO::ImportFromXML::importFromXML(m_fileNameExistingXml);

   } else {
      // create new xml
      m_vizProject = createStructureFromProjectHandle(false);
   }
   
   if (m_rank == 0) 
   {
      ibs::FilePath folderPath(m_filePathXml);
      folderPath << m_outputNameXml;
      
      // Create output directory if not existing
      if (!folderPath.exists())
      {
         FolderPath(folderPath.path()).create();
      }
      m_pt = m_doc.append_child("project");
      
      m_export = new CauldronIO::ExportToXML(m_filePathXml, m_outputNameXml, 1, false);
      
      if (not m_fileNameExistingXml.empty()) 
      { 
         m_vizProjectExisting = CauldronIO::ImportFromXML::importFromXML(m_fileNameExistingXml);
      }
      // Create xml property tree and write datastores
      m_export->addProjectDescription(m_pt, m_vizProject, m_vizProjectExisting);
      m_snapShotNodes = m_pt.append_child("snapshots");
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::saveXML() {
   
   if (m_rank == 0) 
   {
      // Add/replace trap, trapper, migration, .. tables
      addTables();

      displayProgress(m_fileNameXml, m_startTime, "Writing to visualization format ");
      
      // snapshots are already written - add the project data
      m_export->addProjectData(m_pt, m_vizProject, false);

      m_doc.save_file(m_fileNameXml.c_str());
      
      displayProgress("", m_startTime, "Writing to visualization format done ");
   }
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::acquireExistingProperties ()
{
   if (m_vizProject)
   {
      shared_ptr<const CauldronIO::Property> vizProperty;
      for (auto& property : m_vizProject->getProperties()) 
      {
         m_existingProperties.push_back (property->getName());
      }
   }
   return false;
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::propertyExisting (const string &propertyName)
{
   vector<string>::iterator it = find(m_existingProperties.begin(), m_existingProperties.end(), propertyName);
   if (it != m_existingProperties.end()) return true;

   return false;
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::refinePropertyNames(Interface::PropertyList& properties)
{
   for (Interface::PropertyList::iterator propertyIter = properties.begin(); propertyIter != properties.end(); )
   {
      const Interface::Property * property = *propertyIter;
      
      if (propertyExisting (property->getName()))
      {
         propertyIter = properties.erase (propertyIter);
      }
      else
      {
         ++ propertyIter;
      }
   }
 
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::calculateProperties(FormationSurfaceVector& formationItems, Interface::PropertyList properties, 
                                                            Interface::SnapshotList & snapshots)  {
   
   if (properties.size () == 0) 
   {
      return;
   }

   m_sharedProjectHandle.reset(m_projectHandle);
   
   if (m_rank == 0)
   {
      createXML();
   }

   if (not m_fileNameExistingXml.empty()) 
   {
      // create a list of existing XML properties, if there are any
      acquireExistingProperties();
      
      // remove the existing properties from the list of the properties to be calculated
      refinePropertyNames (properties);
   }
   
   MPI_Op_create((MPI_User_function *)minmaxint_op, true, & m_ind);
   MPI_Op_create((MPI_User_function *)minmax_op, true, & m_op);
   
   Interface::SnapshotList::iterator snapshotIter;
   
   Interface::PropertyList::iterator propertyIter;
   FormationSurfaceVector::iterator formationIter;
   
   SnapshotFormationSurfaceOutputPropertyValueMap allOutputPropertyValues;
   
   if (snapshots.empty()) 
   {
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot(0);
      
      snapshots.push_back(zeroSnapshot);
   }
   
   updateFormationsKRange();
   
   for (snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter)
   {
      const Interface::Snapshot * snapshot = *snapshotIter;
      
      displayProgress(snapshot->getFileName (), m_startTime, "Start computing ");
      
      m_formInfoList.reset();
      m_formInfoList = getDepthFormations(m_projectHandle, snapshot);
      if (m_formInfoList->size() < 1) 
      {
         continue;
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
         
         createVizSnapshotFormationData(snapshot, * formationIter, properties, allOutputPropertyValues);
      }
      
      removeProperties(snapshot, allOutputPropertyValues);
      m_propertyManager->removeProperties(snapshot);
      
      
      if (m_rank == 0) 
      {
         std::shared_ptr<CauldronIO::SnapShot> snap = DerivedProperties::getSnapShot(m_vizProject, snapshot->getTime());
         
         createTrappers(snapshot, snap);
         
         // updateConstantValue updates a constant value for snapshots volume data, which is fastcauldron output only
         updateConstantValue(snap);

         pugi::xml_node node = m_snapShotNodes.append_child("snapshot");
         m_export->addSnapShot(snap, node);

         // release snapshot
         DerivedProperties::getSnapShot(m_vizProject, snapshot->getTime())->release();
      }
      
      m_projectHandle->deletePropertiesValuesMaps (snapshot);
      
      StatisticsHandler::update ();
   }
   
   saveXML();
   
   MPI_Op_free (&m_op);  
   MPI_Op_free (&m_ind);
     
   PetscLogDouble End_Time;
   PetscTime(&End_Time);
   
   displayTime(End_Time - m_startTime, "Total derived properties saving: ");
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::updateConstantValue( std::shared_ptr< CauldronIO::SnapShot>& snapshot )
{
   std::shared_ptr<CauldronIO::Volume> snapshotVolume = snapshot->getVolume();
   if( snapshotVolume ) {
      for (size_t i = 0; i <  snapshotVolume->getPropertyVolumeDataList().size(); ++i ) {
         CauldronIO::PropertyVolumeData& pdata =  snapshotVolume->getPropertyVolumeDataList().at(i); 
         
         if (propertyExisting (pdata.first->getName())) continue;

         shared_ptr<CauldronIO::VolumeData> volDataNew = pdata.second;
         if(volDataNew->isRetrieved()) {
            DerivedProperties::updateVolumeDataConstantValue( volDataNew );
         }
      }
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::listXmlProperties() {
   
   std::shared_ptr< CauldronIO::Project> vizProject(CauldronIO::ImportFromXML::importFromXML(m_fileNameXml));

   cout << "List of properties in " << m_fileNameXml << " : " << endl;

   SnapShotList snapShotList = vizProject->getSnapShots();
   for (auto& snapShot : snapShotList) 
   {
      cout << "Snapshot " << snapShot->getAge() << endl;
      DerivedProperties::listProperties(snapShot, vizProject);
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::writeToHDF() {

   std::shared_ptr< CauldronIO::Project> vizProject(CauldronIO::ImportFromXML::importFromXML(m_fileNameXml));

   cout << "Writing to HDF from visualization format " << m_fileNameXml << endl;

   CauldronIO::ExportToHDF::exportToHDF(vizProject, m_fileNameXml, 1, m_basement, m_projectHandle, this);
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::acquireAttributeProperties (DataModel::PropertyOutputAttribute outputAttribute) {

   Interface::PropertyListPtr simProperties = m_projectHandle->getProperties(outputAttribute);
   for (size_t i = 0; i < simProperties->size (); ++i) 
   {
      const Interface::Property* property = (*simProperties)[ i ];
      
      m_propertyNames.push_back(property->getName());
      LogHandler(LogHandler::DEBUG_SEVERITY) << "   #" << property->getName();
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::acquireSimulatorProperties () {
   
   if (m_attribute != DataModel::UNKNOWN_PROPERTY_OUTPUT_ATTRIBUTE and m_propertyNames.empty()) 
   {
      acquireAttributeProperties (m_attribute);
      
      // add genex properties if the last fastcauldron simulation was Darcy
      if (m_attribute == DataModel::FASTCAULDRON_PROPERTY)
      { 
         // check for Darcy
         const std::string& commLine = m_projectHandle->getDetailsOfLastSimulation ("fastcauldron")->getSimulatorCommandLineParameters();
         
         if (commLine.find ("-mcf") != string::npos)
         {
            acquireAttributeProperties (DataModel::FASTGENEX_PROPERTY);
         }
      }
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::updateVizSnapshotsConstantValue() {

   SnapShotList snapShotList = m_vizProject->getSnapShots();
   for (auto& snapShot : snapShotList)  
   {
      updateConstantValue(snapShot);
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::addTables ()
{
   ImportProjectHandle import(false, m_vizProject, m_sharedProjectHandle);

   if (m_rank == 0) 
   {
      if (m_attribute == DataModel::FASTMIG_PROPERTY or m_all2Dproperties)
      {
         // Add migration_io data (replace existing)
         database::Table* table = m_projectHandle->getTable("MigrationIoTbl");
         if (table and table->size() > 0) 
         {
            m_vizProject->clearMigrationEventsTable();
            import.addMigrationIO();
         }

         // Add trapper_io data (replace existing)
         if (m_projectHandle->trappersAreAvailable())
         {
            m_vizProject->clearTrapperTable();
            import.addTrapperIO();
         } 

         // Add trap_io data (replace existing)
         if (m_projectHandle->trapsAreAvailable())
         {
            m_vizProject->clearTrapTable();
            import.addTrapIO();
         }

         // Add reference to massBalance file  (replace existing)
         import.addMassBalance();
      }
      if (m_attribute == DataModel::FASTGENEX_PROPERTY or m_all2Dproperties)
      {
         // Find genex/shale-gas history files (replace existing)
         import.addGenexHistory();
      }

      if (m_attribute == DataModel::FASTTOUCH_PROPERTY or m_all2Dproperties)
      {
         // Find burial history files (replace existing)
         import.addBurialHistory();
      }
      
      // Add 1D tables
      import.add1Ddata();    // todo
   }
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::computeMinMax () {

   float localValues[2];
   float globalValues[2];
   
   localValues[0] = m_minValue;
   localValues[1] = m_maxValue;
      
   MPI_Reduce(localValues, globalValues, 2, MPI_FLOAT, m_op, 0, PETSC_COMM_WORLD);
   if (m_rank == 0) 
   {
      m_minValue = globalValues[0];
      m_maxValue = globalValues[1];
   }
}
//------------------------------------------------------------//
void  VisualizationPropertiesCalculator::computeVolumeBounds (const DataModel::AbstractGrid* grid, int firstK, int lastK, int minK) {

   int numI = grid->numIGlobal();
   int numJ = grid->numJGlobal();

   m_localIndices[0] = grid->firstI (false) + grid->firstJ(false) * numI + (firstK - minK) * numI * numJ; // first index
   m_localIndices[1] = grid->lastI (false) + grid->lastJ(false) * numI + (lastK - minK) * numI * numJ; // last index
   
   MPI_Allreduce(m_localIndices, m_globalIndices, 2, MPI_INT, m_ind, PETSC_COMM_WORLD); 
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::computeFormationVolume (OutputPropertyValuePtr propertyValue, const DataModel::AbstractGrid* grid, 
                                                                int firstK, int lastK, int minK, int depthK, bool computeMinMax) {

   propertyValue->retrieveData();

   unsigned int dataSize = grid->numIGlobal()*grid->numJGlobal()*depthK;
   
   if (dataSize > m_data.size()) 
   {
      m_data.resize(dataSize);
   }

   float *data = &m_data[0];
   memset(data, 0, sizeof(float) * dataSize);

   int numI = grid->numIGlobal();
   int numJ = grid->numJGlobal();
   m_minValue = std::numeric_limits<float>::max();
   m_maxValue = -m_minValue;
   
   for (int j = grid->firstJ (false); j <= grid->lastJ (false); ++j) 
   {
      for (int i = grid->firstI(false); i <= grid->lastI (false); ++i) 
      {
         unsigned int pk = 0;
         for (int k = lastK; k >= firstK; --k, ++ pk) 
         {
            float value = static_cast<float>(propertyValue->getValue(i, j, pk));
            if (computeMinMax and value != CauldronIO::DefaultUndefinedValue)
            {
               m_minValue = std::min(m_minValue, value);
               m_maxValue = std::max(m_maxValue, value);
            }
            size_t index =  i + j * numI + (k - minK ) * numI * numJ;
            m_data[index] = value;
         }
      }
   }
   propertyValue->restoreData ();

   return true;
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::computeFormationMap (OutputPropertyValuePtr propertyValue, const DataModel::AbstractGrid* grid, 
                                                             int kIndex, float * dest) {

   propertyValue->retrieveData();
 
   unsigned int dataSize = grid->numIGlobal() * grid->numJGlobal();
   
   if (dataSize > m_data.size()) 
   {
      m_data.resize(dataSize);
   }

   float *data = &m_data[0];
   memset(data, 0, sizeof(float) * dataSize);

   int numI = grid->numIGlobal();
   
   for (int j = grid->firstJ (false); j <= grid->lastJ (false); ++j) 
   {
      for (int i = grid->firstI(false); i <= grid->lastI (false); ++i) 
      {
         size_t index =  j * numI + i;
         float value = static_cast<float>(propertyValue->getValue(i, j, kIndex));
         m_data[index] = value;
      }
   }

   propertyValue->restoreData ();
   MPI_Reduce((void *)data, (void *)dest,  dataSize, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);

   return true;
}

//------------------------------------------------------------//
std::shared_ptr<CauldronIO::Project> VisualizationPropertiesCalculator::createStructureFromProjectHandle(bool verbose) {

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
   ImportProjectHandle import(false, project, m_sharedProjectHandle);

   import.checkInputValues();

 
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
bool VisualizationPropertiesCalculator::createVizSnapshotResultPropertyValue (OutputPropertyValuePtr propertyValue, 
                                                                              const Snapshot* snapshot, const Interface::Formation * formation,
                                                                              const Interface::Surface * surface) {

   // This check is needed because some of the derived properties are creating null properties
   // instead of using the isComputable() function
   if (not propertyValue->hasProperty())
   {
      return true;
   }

   unsigned int p_depth = propertyValue->getDepth();
   bool status = true;

   if (p_depth > 1 and surface == 0) 
   {
      //if (not propertyValue->isPrimary()) { // uncomment to convert only derived properties
      if (propertyValue->getProperty ()->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY) 
      {
         status = createVizSnapshotResultPropertyValueContinuous (propertyValue, snapshot, formation);
      }
      else if (propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY) 
      {
         status = createVizSnapshotResultPropertyValueDiscontinuous (propertyValue, snapshot, formation);
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
      status = createVizSnapshotResultPropertyValueMap (propertyValue, snapshot, formation, surface);
   }

   if (not status) {
      PetscPrintf(PETSC_COMM_WORLD, " Basin_Error: cannot convert property %s for formation %s at snapshot %lf\n", 
                  (propertyValue->getName()).c_str(), (formation->getName()).c_str(), snapshot->getTime());
   }

   return true;

}
//------------------------------------------------------------//
void VisualizationPropertiesCalculator::createVizSnapshotFormationData(const Snapshot * snapshot, const FormationSurface & formationItem, 
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
bool VisualizationPropertiesCalculator::createVizSnapshotResultPropertyValueContinuous (OutputPropertyValuePtr propertyValue, 
                                                                                        const Snapshot* snapshot, const Interface::Formation * daFormation) {
   

   const string propName = propertyValue->getName();
   
   if (daFormation->kind() == Interface::BASEMENT_FORMATION and not allowBasementOutput (propName)) 
   {
      return true;
   }

   //find info and geometry for the formation
   std::shared_ptr<CauldronIO::FormationInfo> info;
   size_t maxK = 0;
   size_t minK = std::numeric_limits<size_t>::max();
   size_t maxSedimentK = 0;

   for (size_t i = 0; i < m_formInfoList->size(); ++i)  
   {
      std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = m_formInfoList->at(i);
      if (depthInfo->formation->kind() == Interface::SEDIMENT_FORMATION) 
      {
         maxSedimentK = max (maxSedimentK, depthInfo->kEnd);
      }
      maxK = max(maxK, depthInfo->kEnd);
      minK = min(minK, depthInfo->kStart);
      if (depthInfo->formation == daFormation) 
      {
         info = m_formInfoList->at(i);
      }
   }
   if (not info) 
   {
      PetscPrintf(PETSC_COMM_WORLD, "Continuous property %s: %s formation not found\n", propName.c_str(), (daFormation->getName()).c_str());
      return false;
   } 

   size_t depthK = 1 + maxK - minK;
 
   int firstK = static_cast<int>(info->kStart);
   int lastK  = static_cast<int>(info->kEnd);
   int fK = (daFormation->kind () == BASEMENT_FORMATION and firstK > 0 ? firstK + 1 : firstK);
   unsigned int dataIsAllocated;

   const DataModel::AbstractGrid* grid = propertyValue->getGrid();

   if(m_rank != 0) {
      // collective check for an error on the rank 0
      MPI_Bcast (&dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
      if (dataIsAllocated == 0) 
      {
         return false;
      }

      if (not allowBasementOutput (propName)) 
      {
         depthK = 1 + maxSedimentK - minK;
      }

      computeFormationVolume(propertyValue, grid, fK, lastK, minK, depthK, true);
      computeVolumeBounds(grid, fK, lastK, minK);
     
      float *data = &m_data[m_globalIndices[0]];

      MPI_Reduce((void *)data, NULL,  m_globalIndices[1] - m_globalIndices[0] + 1, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);

      if (daFormation->kind () == SEDIMENT_FORMATION)
      {
         computeMinMax ();
      }
  
      return true;
   }

   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty(propertyValue, CauldronIO::Continuous3DProperty);

   // find snapshot
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, snapshot->getTime());
   
   // find or create formation
   if (snapshot->getTime() == 0) 
   {
      std::shared_ptr<CauldronIO::Formation> vizFormation = findOrCreateFormation (info);
   } 
   
   // find or create snapshot volume and geometry
   std::shared_ptr<CauldronIO::Volume> snapshotVolume = vizSnapshot->getVolume();
   std::shared_ptr< CauldronIO::Geometry3D> geometry;
   
   shared_ptr<CauldronIO::VolumeData> volDataNew;
   bool propertyVolumeExisting = false;
   
   if (not snapshotVolume) 
   {
      // create volume and geometry
      snapshotVolume.reset(new CauldronIO::Volume(CauldronIO::None));
      vizSnapshot->setVolume(snapshotVolume);
      geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
      m_vizProject->addGeometry(geometry);
   } 
   else 
   {
      // find volume of the property
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
   
   if (not propertyVolumeExisting) 
   {
      if (not allowBasementOutput (propName)) 
      {
         depthK = 1 + maxSedimentK - minK;
         geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                   grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
      }
      volDataNew.reset(new CauldronIO::VolumeDataNative(geometry, CauldronIO::DefaultUndefinedValue));
           
      volDataNew->setData_IJK(nullptr, true, 0);
   }

   float * internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());

   dataIsAllocated = (not internalData ? 0 : 1);
   MPI_Bcast (&dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
   if (dataIsAllocated == 0) 
   {
      return false;
   }

   computeFormationVolume(propertyValue, grid, fK, lastK, minK, depthK, true);
   computeVolumeBounds(grid, fK, lastK, minK);

   float * data = &m_data[m_globalIndices[0]];
   float * firstInd = &internalData[m_globalIndices[0]];

   MPI_Reduce((void *)data, (void *)firstInd, m_globalIndices[1] - m_globalIndices[0] + 1, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);

   if (daFormation->kind() == SEDIMENT_FORMATION)
   {
      computeMinMax ();

      float sedimentMinValue = volDataNew->getSedimentMinValue();
      float sedimentMaxValue = volDataNew->getSedimentMaxValue();
      
      if (
         sedimentMinValue == DefaultUndefinedValue && 
         sedimentMaxValue == DefaultUndefinedValue)
      {
          volDataNew->setSedimentMinMax(m_minValue, m_maxValue);
     }
      else
      {
         volDataNew->setSedimentMinMax(
                                       std::min(m_minValue, sedimentMinValue),
                                       std::max(m_maxValue, sedimentMaxValue));
      }
   }

   if (not propertyVolumeExisting) 
   {
      CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
      snapshotVolume->addPropertyVolumeData(propVolDataNew);
   }

   return true;
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::createVizSnapshotResultPropertyValueDiscontinuous (OutputPropertyValuePtr propertyValue, 
                                                                                           const Snapshot* snapshot, const Interface::Formation * daFormation) {
      
   const string propName = propertyValue->getName();

   if (daFormation->kind() == Interface::BASEMENT_FORMATION and not allowBasementOutput (propName)) 
   {
      return true;
   }
   
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

   const DataModel::AbstractGrid* grid = propertyValue->getGrid();
   unsigned int dataSize = grid->numIGlobal() * grid->numJGlobal() * propertyValue->getDepth();
   int firstK = static_cast<int>(info->kStart);
   int lastK  = static_cast<int>(info->kStart +  propertyValue->getDepth() - 1);
   unsigned int dataIsAllocated;
 
   if(m_rank != 0) {
      computeFormationVolume (propertyValue, grid, firstK, lastK, info->kStart, propertyValue->getDepth(), false);

      // collective check for an error on the rank 0
      MPI_Bcast( &dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
      if (dataIsAllocated == 0) 
      {
         return false;
      }

      MPI_Reduce((void *)(&m_data[0]), NULL, dataSize, MPI_FLOAT, MPI_SUM, 0, PETSC_COMM_WORLD);

      return true;
   }
   
   // find snapshot
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot(m_vizProject, snapshot->getTime());

   // find or create formation
   std::shared_ptr< CauldronIO::Formation> vizFormation = findOrCreateFormation (info);

   //create geometry
   const std::shared_ptr<CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), propertyValue->getDepth(), info->kStart,
                                                                                     grid->deltaI(), grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
   m_vizProject->addGeometry(geometry);

   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty (propertyValue, CauldronIO::Discontinuous3DProperty);
   
   // create volume data
   shared_ptr<CauldronIO::VolumeData> volDataNew(new CauldronIO::VolumeDataNative(geometry));

   computeFormationVolume (propertyValue, grid, firstK, lastK, info->kStart, propertyValue->getDepth(), false);

   volDataNew->setData_IJK(nullptr, true, 0);

   float * internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());

   dataIsAllocated = (not internalData ? 0 : 1);
   MPI_Bcast (&dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
   if (dataIsAllocated == 0) 
   {
      return false;
   }
   MPI_Reduce((void *)(&m_data[0]), (void *)internalData, dataSize, MPI_FLOAT, MPI_SUM, 0,  PETSC_COMM_WORLD);

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
bool VisualizationPropertiesCalculator::createVizSnapshotResultPropertyValueMap (OutputPropertyValuePtr propertyValue, 
                                                                                 const Snapshot* snapshot, const Interface::Formation * daFormation,
                                                                                 const Interface::Surface * daSurface) {
   
   const string propName = propertyValue->getName();

   if (daSurface != 0 and daSurface->kind() == BASEMENT_SURFACE and not allowBasementOutput(propName))
   {
      return true;
   }
   const DataModel::AbstractGrid* grid = propertyValue->getGrid();
   unsigned int p_depth = propertyValue->getDepth();
   unsigned int kIndex = 0;
   unsigned int dataIsAllocated;

   assert(p_depth == 1);
 
   if(m_rank != 0) {
      // collective check for an error on the rank 0
      MPI_Bcast (&dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
      if (dataIsAllocated == 0) 
      {
         return false;
      }
      
      computeFormationMap(propertyValue, grid, kIndex, nullptr);

      return true;
   }
   const string surfaceName = (daSurface != 0 ? daSurface->getName() : "");

   // Create geometry
   std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(grid->numIGlobal(), grid->numJGlobal(), grid->deltaI(), 
                                                                                     grid->deltaJ(), grid->minIGlobal(), grid->minJGlobal()));
   m_vizProject->addGeometry(geometry);
   
   // find/create a property
   PropertyAttribute attrib = (daSurface != 0 ? CauldronIO::Surface2DProperty : CauldronIO::Formation2DProperty);
   shared_ptr<const CauldronIO::Property> vizProperty = findOrCreateProperty (propertyValue,  attrib);

   // find snapshot
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
      } 
   }

   //assign surface map
   std::shared_ptr< CauldronIO::SurfaceData> valueMap(new CauldronIO::MapNative(geometry));
   valueMap->setData_IJ(nullptr); 
  
   float * internalData = const_cast<float *>(valueMap->getSurfaceValues());

   dataIsAllocated = (not internalData ? 0 : 1);
   MPI_Bcast (&dataIsAllocated, 1, MPI_UNSIGNED, 0, PETSC_COMM_WORLD);
   if (dataIsAllocated == 0) 
   {
      return false;
   }
   
   computeFormationMap(propertyValue, grid, kIndex, internalData);

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

   return true;
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::updateFormationsKRange() {

   // Compute k-range for all stratigraphy formations at present day
   const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot(0);
   std::shared_ptr<CauldronIO::FormationInfoList> depthFormations = getDepthFormations(m_projectHandle, zeroSnapshot);

   if(m_rank == 0) {
      // Capture global k-range
      for (unsigned int i = 0; i < depthFormations->size(); ++i)
      {
         std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
         std::shared_ptr<CauldronIO::Formation> vizFormation = findOrCreateFormation (info);
      }
   }
}

//------------------------------------------------------------//
shared_ptr<CauldronIO::Formation> VisualizationPropertiesCalculator::findOrCreateFormation(std::shared_ptr<CauldronIO::FormationInfo>& info) {

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
shared_ptr<const CauldronIO::Property> VisualizationPropertiesCalculator::findOrCreateProperty(OutputPropertyValuePtr propertyValue, PropertyAttribute attrib) {

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
std::shared_ptr<const CauldronIO::Reservoir> VisualizationPropertiesCalculator::findOrCreateReservoir(const Interface::Reservoir* reserv, 
                                                                                                      std::shared_ptr<const CauldronIO::Formation> formationIO)
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
void VisualizationPropertiesCalculator::createTrappers(const Interface::Snapshot * snapShot, std::shared_ptr<CauldronIO::SnapShot>& snapShotIO)
{  
   if (m_attribute == DataModel::FASTMIG_PROPERTY or m_all2Dproperties)
   {
      // Find trappers
      std::shared_ptr<Interface::TrapperList> trapperList(m_projectHandle->getTrappers(0, snapShot, 0, 0));
      for (size_t i = 0; i < trapperList->size(); ++i)
      {
         const Interface::Trapper* trapper = trapperList->at(i);
         
         // Get some info
         int downstreamTrapperID = -1;
         if (trapper->getDownstreamTrapper())
            downstreamTrapperID = trapper->getDownstreamTrapper()->getPersistentId();
         int persistentID = trapper->getPersistentId();
         int ID = trapper->getId();
         double spillPointDepth = trapper->getSpillDepth();
         double spillPointX, spillPointY;
         trapper->getSpillPointPosition(spillPointX, spillPointY);
         double depth = trapper->getDepth();
         double pointX, pointY;
         trapper->getPosition(pointX, pointY);
         double GOC = trapper->getGOC();
         double OWC = trapper->getOWC();
         
         const Interface::Reservoir* reservoir = trapper->getReservoir();
         assert(reservoir);
         
         // Create a new Trapper and assign some values
         std::shared_ptr<CauldronIO::Trapper> trapperIO(new CauldronIO::Trapper(ID, persistentID));
         trapperIO->setReservoirName(reservoir->getName());
         trapperIO->setSpillDepth((float)spillPointDepth);
         trapperIO->setSpillPointPosition((float)spillPointX, (float)spillPointY);
         trapperIO->setDepth((float)depth);
         trapperIO->setPosition((float)pointX, (float)pointY);
         trapperIO->setDownStreamTrapperID(downstreamTrapperID);
         trapperIO->setOWC((float)OWC);
         trapperIO->setGOC((float)GOC);
         
         snapShotIO->addTrapper(trapperIO);
      }
   }
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::parseCommandLine(int argc, char ** argv) {

   bool status = AbstractPropertiesCalculator::parseCommandLine (argc, argv);
   if (status) 
   {
      const int MaximumLengh = 4000;
      
      PetscBool parameterDefined = PETSC_FALSE;
     
      PetscOptionsHasName (PETSC_NULL, "-hdfonly", &parameterDefined);
      if (parameterDefined) m_vizFormatHDFonly = true;
      
      PetscOptionsHasName (PETSC_NULL, "-listXml", &parameterDefined);
      if (parameterDefined) m_vizListXml = true;
      
      PetscOptionsHasName (PETSC_NULL, "-pminor", &parameterDefined);
      if (parameterDefined)
      {
         m_snapshotsType = MAJOR | MINOR;
         m_minorPrimary = true;
      }

      PetscInt numberOfParameters = MaximumLengh;
      char parameter[MaximumLengh];
      
      PetscOptionsGetString (PETSC_NULL, "-sim", parameter, numberOfParameters, &parameterDefined);
      if (parameterDefined)
      {
         string simulatorName = string (parameter);
         if (simulatorName.empty())
         {
            showUsage (argv[0], "Argument for '-sim' is missing");
            return false;
         }
         else
         {
            if (simulatorName == "fastcauldron")
            {
               m_attribute = DataModel::FASTCAULDRON_PROPERTY;
            }
            else if (simulatorName == "fastmig")
            {
               m_attribute = DataModel::FASTMIG_PROPERTY;
            }
            else if (simulatorName == "fastgenex6")
            {
               m_attribute = DataModel::FASTGENEX_PROPERTY;
            }
            else if (simulatorName == "fastctc")
            {
               m_attribute = DataModel::FASTCTC_PROPERTY;
            }
            else if (simulatorName == "fasttouch7")
            {
               m_attribute = DataModel::FASTTOUCH_PROPERTY;
            }
            else
            {
               showUsage (argv[0], "Unknown argument for '-sim'");
               return false;
            }
         }
      }
     
      PetscOptionsGetString (PETSC_NULL, "-xml", parameter, numberOfParameters, &parameterDefined);
      if (parameterDefined)
      {
         m_fileNameExistingXml = string (parameter);
         if (m_fileNameExistingXml.empty())
         {
            showUsage (argv[0], "Argument for '-xml' is missing");
            return false;
         }
      }

      PetscOptionsGetString (PETSC_NULL, "-savexml", parameter, numberOfParameters, &parameterDefined);
      if (parameterDefined)
      {
         m_fileNameXml = string (parameter);
         if (m_fileNameXml.empty())
         {
            showUsage (argv[0], "Argument for '-xml' is missing");
            return false;
         }
      }
      
      status = checkParameters();
   }
   
   return status;
}

//------------------------------------------------------------//
bool VisualizationPropertiesCalculator::checkParameters() {

   if (m_attribute != DataModel::UNKNOWN_PROPERTY_OUTPUT_ATTRIBUTE)
   {
      m_all3Dproperties = false;
      m_all2Dproperties = false;

      if (m_attribute == DataModel::FASTCAULDRON_PROPERTY)
      {
         m_extract2D = true;
      } else {
         m_extract2D = false;
      }
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

   setXmlName();

   if (not m_fileNameExistingXml.empty())
   {
      // Check the existing XML file
      ibs::FilePath existingXMLFileName(m_fileNameExistingXml);
      if (not existingXMLFileName.exists())
      {
         PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: XML file %s does not exist.\n", m_fileNameExistingXml.c_str());
         return false;
      }
  
      // Check that the existing XML file and a new file have differnt paths/names
      boost::filesystem::path thePath =  boost::filesystem::canonical(existingXMLFileName.filePath());
      string existingXMLpath = thePath.string(); 
 
      ibs::FolderPath existingXMLOutputDir (existingXMLpath);
      string existingXMLOutputDirName = existingXMLFileName.fileNameNoExtension() + "_vizIO_output";
      existingXMLOutputDir << existingXMLOutputDirName;

      if (not existingXMLOutputDir.exists())
      {
         PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: Output dir %s does not exist.\n", existingXMLOutputDirName.c_str());
         return false;
      }

      ibs::FilePath newXMLFileName(m_fileNameXml);
      thePath = boost::filesystem::canonical(newXMLFileName.filePath());
      string newXMLpath = thePath.string(); 

      if (newXMLpath == existingXMLpath and m_fileNameExistingXml == m_fileNameXml)
      {
         PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: Existing XML file and a new XML file should have different absolute paths.\n");
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------//
void VisualizationPropertiesCalculator::setXmlName() {

   if (m_fileNameXml.empty())
   {
      string projectName;

      ibs::FilePath ppath (m_projectFileName);
      if ( ppath.size() > 0 ) // number path elements more the one
      {
         m_filePathXml = ppath.filePath();
         projectName = ppath.fileName();
      }
      else
      {
         m_filePathXml = ".";
         projectName = m_projectFileName;
      }

      string::size_type dotPos = projectName.rfind( ".project" );
      if ( dotPos != string::npos )
      {
         projectName.erase( dotPos, string::npos );
      }
      
      m_fileNameXml = projectName + ".xml";
      m_outputNameXml = projectName + "_vizIO_output";
   }
   else
   {
      ibs::FilePath ppath (m_fileNameXml);

      if ( ppath.size() > 0 ) // number path elements more the one
      {
         m_filePathXml = ppath.filePath();
      }
      else
      {
         m_filePathXml = ".";
      }
      m_outputNameXml = ppath.fileNameNoExtension() + "_vizIO_output";
   }
}
