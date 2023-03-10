//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_PROJECTHANDLE_H
#define INTERFACE_PROJECTHANDLE_H

//hdf5 3rdparty
#include "hdf5.h"

#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <list>

//DataAccess
#include "Interface.h"
#include "ProjectFileHandler.h"
#include "PropertyAttribute.h"
#include "MessageHandler.h"
#include "ApplicationGlobalOperations.h"
#include "Local2DArray.h"
#include "Validator.h"
// DataAccess library table classes
// fastctc
#include "TableCTC.h"
#include "TableCTCRiftingHistory.h"
// alc
#include "TableOceanicCrustThicknessHistory.h"

//TableIo
#include "database.h"

/*! \mainpage The Cauldron Distributed Data Access Framework
 * \section intro Introduction
 * The Cauldron Distributed Data Access Framework is a framework provides
 * access to the objects that make up a Cauldron model.
 * In addition, this framework provides the possibility to generate new 2D and 3D PropertyValue objects
 * and save these objects to the appropriate files. It is also possible to save the entire modified Cauldron
 * model.
 *
 * The framework contains a number of classes that make up the Cauldron 4D Earth Model: Formation, Surface,
 * Lithology, Snapshot, Reservoir, Trap and others.
 * The framework handles properties through the Property and PropertyValue classes.
 * The framework supports the possibility to produce new PropertyValue objects.
 *
 * The framework is called distributed because the 2D maps and 3D volumes are processed in data parallel mode based on Petsc and
 * MPI.
 * To that end the framework provides the Grid and GridMap classes.
 *
 * To support application-specific subclassing of the framework classes all framework objects are created via
 * an object factory that can be replaced to produce objects of derived classes of the framework classes.
 */

namespace migration
{
   class Migrator;
}

namespace DataAccess
{
   using Interface::MINOR;
   using Interface::MAJOR;

   using Interface::SURFACE;
   using Interface::FORMATION;
   using Interface::FORMATIONSURFACE;
   using Interface::RESERVOIR;

   using Interface::MAP;
   using Interface::VOLUME;


   namespace Interface
   {
      /// Create a project from a project file with the given name
      /// return the associated ProjectHandle
      ProjectHandle * OpenCauldronProject(const std::string & name,
                                           const ObjectFactory* objectFactory,
                                           const std::vector<std::string>& outputTableNames = NoTableNames);

      /// @brief Create TableIO database object from project file. This function is used by OpenCauldronProject()
      /// @param name project file name
      /// @return Database object pointer which must be deleted by the caller
      database::ProjectFileHandlerPtr CreateDatabaseFromCauldronProject( const std::string& name,
                                                                         const std::vector<std::string>& outputTableNames = NoTableNames );

      /// A ProjectHandle contains references to the entities in a Project.
      /// Objects of this class are created by OpenCauldronProject()
      class ProjectHandle
      {

      typedef Local2DArray <bool> BooleanLocal2DArray;

      public:
         /// Constructor
         ProjectHandle( database::ProjectFileHandlerPtr projectFileHandler, const std::string & name, const ObjectFactory* objectFactory );

         /// Destructor
         virtual ~ProjectHandle( void );

         int GetNumberOfSpecies( void );
         std::string GetSpeciesName( int i );

         /// Get a handle to the Table with the given name
         database::Table * getTable( const std::string & tableName ) const;


         /// \brief Set the table to be saved in the table output file.
         void setAsOutputTable ( const std::string& tableName );

         /// return the ObjectFactory
         const ObjectFactory* getFactory( void ) const;

         /// save the project to the specified file
         bool saveToFile( const std::string & fileName );

         /// \brief Set details about the current simulation.
         void setSimulationDetails ( const std::string& simulatorName,
                                     const std::string& simulatorMode,
                                     const std::string& simulatorCommandLineParams );

         /// \brief Get list of simulation details.
         SimulationDetailsListPtr getSimulationDetails () const;

         /// \brief Get the details of the last simulation for a particular simulator.
         const SimulationDetails* getDetailsOfLastSimulation ( const std::string& simulatorName ) const;

         /// \brief Get the details of the last fastcauldron simulation other than HighResDecompaction
         const SimulationDetails* getDetailsOfLastFastcauldron () const;

         /// Return the full file name of the project
         virtual const std::string & getName( void ) const;
         /// Return the directory of the project
         virtual const std::string & getProjectPath( void ) const;
         /// Return the name of the project without the .project extension
         virtual const std::string & getProjectName( void ) const;
         /// Return the file name of the project without the directory path
         virtual const std::string & getFileName( void ) const;

         /// set the Grid that is used to produce new PropertyValues
         bool setActivityOutputGrid( const Grid * grid );

         /// start a new activity
         virtual bool startActivity( const std::string & name, const Grid * grid, bool saveAsInputGrid = false, bool createResultsFile = true, bool append = false );

         /// Restart an activity.
         bool restartActivity( void );

         /// continue an activity
         bool continueActivity( void );

         /// abort an activity
         /// identical to finishActivity (false)
         bool abortActivity( void );

         /// finish an activity
         bool finishActivity( bool isComplete = true );

         bool saveAsInputGrid( void ) const;

         /// Returns the Grid in which the input maps of this project are specified.
         virtual const Grid * getInputGrid( void ) const;

         /// Returns the Grid of the output GridMaps that were produced by either a migration run or
         /// a high resolution decompaction run. This grid is a subgrid of the input grid.
         virtual const Grid * getHighResolutionOutputGrid() const;

         /// Returns the Grid of the output GridMaps that were not produced by either a migration run or
         /// a high resolution decompaction run. This grid is a subgrid of the high resolution output grid.
         virtual const Grid * getLowResolutionOutputGrid() const;

         /// print the snapshot table
         void printSnapshotTable () const;

         /// Find the Snapshot with the given time
         virtual const Snapshot * findSnapshot( double time, int type = MAJOR ) const;
         /// Find the Snapshot with the given time
         virtual const Snapshot * findNextSnapshot( double time, int type = MAJOR ) const;
         /// Find the Snapshot with the given time
         virtual const Snapshot * findPreviousSnapshot( double time, int type = MAJOR ) const;
         /// Find the LithoType with the given name
         virtual const LithoType * findLithoType( const std::string & name ) const;
         /// Find the Formation with the given name
         virtual const Formation * findFormation( const std::string & name ) const;
         /// Find the Surface with the given name
         virtual const Surface * findSurface( const std::string & name ) const;
         /// Find the Reservoir with the given name
         virtual const Reservoir * findReservoir( const std::string & name ) const;

         /// Find the Property  with the given name
         virtual const Property * findProperty( const std::string & name ) const;

         /// Find and return a const pointer to the property int the time-filter.
         ///
         /// If the name is not found then a null value will be returned.
         virtual const OutputProperty * findTimeOutputProperty( const std::string & propertyName ) const;

         /// Find the InputValue  with the given attributes
         virtual const InputValue * findInputValue( const std::string & tableName, const std::string & mapName ) const;
         /// Find the SourceRock with the given layer name
         virtual const SourceRock * findSourceRock( const std::string & name ) const;
         /// return the AllochthonousLithology of the specified formation.
         /// if formation equals 0, then null is returned.
         virtual const AllochthonousLithology * findAllochthonousLithology( const std::string& formationName ) const;
         /// Find the FluidType with the given name.
         virtual const FluidType* findFluid( const std::string& name ) const;

         /// return the list of LithoType objects.
         virtual LithoTypeList * getLithoTypes( void ) const;
         /// return the list of snapshots.
         virtual SnapshotList * getSnapshots( int type = MAJOR ) const; // initialized on stack so delete after use
         /// return the list of formations, all or for a given snapshot.
         virtual FormationList * getFormations( const Snapshot * snapshot = nullptr, const bool includeBasement = false ) const;
         /// return the list of surfaces, all or for a given snapshot.
         // Do we need to include igneous intrusions here, as they will have the same snapshot time as the surface below.
         virtual SurfaceList * getSurfaces( const Snapshot * snapshot = nullptr, const bool includeBasement = false ) const;
         /// return the list of reservoirs
         virtual ReservoirList * getReservoirs( const Formation * formation = nullptr ) const;
         /// @return the global reservoir options
         std::shared_ptr<const ReservoirOptions> getReservoirOptions () const;
         /// add a detected reservoir to the list of reservoirs
         virtual Reservoir* addDetectedReservoirs (database::Record * record, const Formation * formation);
         /// return the list of MobileLayers
         virtual MobileLayerList * getMobileLayers( const Formation * formation ) const;

         /// Return a list of all fluids in project.
         virtual FluidTypeList* getFluids() const;

         virtual PaleoFormationPropertyList * getCrustPaleoThicknessHistory() const;

         virtual PaleoFormationPropertyList * getMantlePaleoThicknessHistory() const;

         virtual PaleoFormationPropertyList* getOceaCrustPaleoThicknessHistory() const;

         /// return the list of surface temperature history.
         // Would it be better to have a ModelTopSurface kind of object, that also had the top boundary conditions?
         virtual PaleoPropertyList * getSurfaceTemperatureHistory() const;

         /// return the list of surface depth history.
         // Would it be better to have a ModelTopSurface kind of object, that also had the top boundary conditions?
         virtual PaleoPropertyList * getSurfaceDepthHistory() const;

         /// return the list of heat-flow paleo-surface properties.
         virtual PaleoSurfacePropertyList * getHeatFlowHistory() const;

         /// return the list of AllochthonousLithologyDistributions of the specified AllochthonousLithology.
         /// If allochthonousLithology equals 0, then null is returned.
         virtual AllochthonousLithologyDistributionList * getAllochthonousLithologyDistributions( const AllochthonousLithology * allochthonousLithology = nullptr ) const;

         /// return the list of AllochthonousLithologyInterpolations from the specified formation.
         /// If formation equals 0, then all allochthonous lithology interpolations are returned.
         virtual AllochthonousLithologyInterpolationList * getAllochthonousLithologyInterpolations( const AllochthonousLithology * allochthonousLithology = nullptr ) const;

         /// Return a list of the time-output properties for a current modeling mode.
         virtual OutputPropertyList * getTimeOutputProperties() const;

         /// Return a list to the heat-capacity samples for the lithology.
         ///
         /// If litho == 0 then all heat capacity samples will be returned.
         virtual LithologyHeatCapacitySampleList * getLithologyHeatCapacitySampleList( const LithoType* litho = nullptr ) const;

         /// Return a list to the thermal-conductivity samples for the lithology.
         ///
         /// If litho == 0 then all thermal-conductivity samples will be returned.
         virtual LithologyThermalConductivitySampleList * getLithologyThermalConductivitySampleList( const LithoType* litho = nullptr ) const;

         /// Return a list to the heat-capacity samples for the fluid.
         ///
         /// If fluid == 0 then all heat-capacity samples will be returned.
         virtual FluidHeatCapacitySampleList * getFluidHeatCapacitySampleList( const FluidType* fluid ) const;

         /// Return a list to the heat-capacity samples for the fluid.
         ///
         /// If fluid == 0 then all heat-capacity samples will be returned.
         virtual FluidThermalConductivitySampleList * getFluidThermalConductivitySampleList( const FluidType* fluid ) const;

         /// Return a list to the related projects.
         virtual RelatedProjectList * getRelatedProjectList() const;

         /// return the list of FaultCollections for a Formation
         virtual FaultCollectionList * getFaultCollections( const Formation * formation ) const;

         /// Load and process the fault descriptions
         virtual bool loadFaults( void );

         virtual ConstrainedOverpressureIntervalList* getConstrainedOverpressureIntervalList( const Formation* formation ) const;

         virtual bool loadConstrainedOverpressureIntervals();

         /// return a list of Migration objects based on the given arguments.
         /// if an argument equals 0, it is used as a wild card.
         virtual MigrationList * getMigrations( const std::string & process, const Formation * sourceFormation,
            const Snapshot * sourceSnapshot, const Reservoir * sourceReservoir, const Trapper * sourceTrapper,
            const Snapshot * destinationSnapshot, const Reservoir * destinationReservoir, const Trapper * destinationTrapper
            ) const;

         /// return a list of Trap objects based on the given arguments.
         /// if an argument equals 0, it is used as a wild card.
         virtual TrapList * getTraps( const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id ) const;

         /// return a Trapper that meets the given condition arguments.
         /// If an argument equals 0, it is used as a wild card.
         virtual Trapper * findTrapper( const Reservoir * reservoir,
            const Snapshot * snapshot, unsigned int id, unsigned int persistentId ) const;

         // return a list of Trapper objects based on the given arguments.
         // if an argument equals 0, it is used as a wild card.
         virtual TrapperList* getTrappers(const Reservoir* reservoir,
           const Snapshot* snapshot, unsigned int id, unsigned int persistentId) const;

         /// return a Trap that meets the given condition arguments.
         /// If an argument equals 0, it is used as a wild card.
         /// If all arguments are non-0, at most one trap can meet the conditions.
         virtual const Trap * findTrap( const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id ) const;

         /// return the list of (Mapped)InputValues
         virtual InputValueList * getInputValues( void ) const;

         /// @brief Return a list of Properties which match the given arguments.
         ///
         /// @param[in] all if true it means: SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR
         /// @param[in] selectionFlags is logical OR for the following flags:
         ///                           SURFACE = surface property which per definition is 2D.
         ///                             FORMATION = formation property which can be 2D and 3D
         ///                             FORMATIONSURFACE = a surface property that is not continuous over the surface.
         ///                             RESERVOIR = properties which apply to a reservoir and are therefore 2D.
         /// @param[in] snapshot      properties belonging to this snapshot. If not specified, return properties for all snapshots.
         /// @param[in] reservoir     properties belonging to this reservoir.
         /// @param[in] formation     properties belonging to this formation.
         /// @param[in] surface       properties belonging to this surface.
         /// @param[in] propertyTypes whether the properties should be 2D (MAP) or 3D (VOLUME)
         /// @return                  a list of the selected properties
         virtual PropertyList * getProperties( bool all = false, int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
            const Snapshot * snapshot = nullptr,
            const Reservoir * reservoir = nullptr, const Formation * formation = nullptr,
            const Surface * surface = nullptr, int propertyTypes = MAP | VOLUME ) const;

         /// \brief Get the list of properties that have the particular PropertyAttribute.
         virtual PropertyListPtr getProperties ( const DataModel::PropertyAttribute attr ) const;

         /// \brief Get the list of properties that have the particular PropertyOutputAttribute.
         virtual PropertyListPtr getProperties ( const DataModel::PropertyOutputAttribute attr ) const;

         /// @brief Return a list of recorded property values based on the given arguments.
         ///
         /// @param[in] selectionFlags is logical OR for the following flags:
         ///                            SURFACE = surface property which per definition is 2D.
         /// 	                         FORMATION = formation property which can be 2D and 3D
         /// 	                         FORMATIONSURFACE = a surface property that is not continuous over the surface.
         /// 	                         RESERVOIR = properties which apply to a reservoir and are therefore 2D.
         /// @param property
         /// @param[in] snapshot properties belonging to this snapshot. If not specified, return
         ///            properties for all snapshots.
         /// @param[in] reservoir properties belonging to this reservoir.
         /// @param[in] formation properties belonging to this formation.
         /// @param[in] surface properties belonging to this surface.
         /// @param[in] propertyTypes whether the properties should be 2D (MAP) or 3D (VOLUME)
         /// @return    a list of the selected recorded properties
         PropertyValueList * getPropertyValues( int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
            const Property * property = nullptr, const Snapshot * snapshot = nullptr,
            const Reservoir * reservoir = nullptr, const Formation * formation = nullptr,
            const Surface * surface = nullptr, int propertyTypes = MAP | VOLUME ) const;

         /// @brief Return a map of properties to property values, based on the given arguments.
         ///
         /// @param[in] selectionFlags is logical OR for the following flags:
         ///                            SURFACE = surface property which per definition is 2D.
         /// 	                         FORMATION = formation property which can be 2D and 3D
         /// 	                         FORMATIONSURFACE = a surface property that is not continuous over the surface.
         /// 	                         RESERVOIR = properties which apply to a reservoir and are therefore 2D.
         /// @param[in] propertyTypes whether the properties should be 2D (MAP) or 3D (VOLUME)
         /// @return    a map of properties to property values, based on the given arguments
         PropertyPropertyValueListMap getPropertyPropertyValuesMap( int selectionFlags, int propertyType );

         /// @brief Return a list of unrecorded property values based on the given arguments.
         ///
         /// @param[in] selectionFlags is logical OR for the following flags:
         ///                            SURFACE = surface property which per definition is 2D.
         /// 	                         FORMATION = formation property which can be 2D and 3D
         /// 	                         FORMATIONSURFACE = a surface property that is not continuous over the surface.
         /// 	                         RESERVOIR = properties which apply to a reservoir and are therefore 2D.
         /// @param property
         /// @param[in] snapshot properties belonging to this snapshot. If not specified, return
         ///            properties for all snapshots.
         /// @param[in] reservoir properties belonging to this reservoir.
         /// @param[in] formation properties belonging to this formation.
         /// @param[in] surface properties belonging to this surface.
         /// @param[in] propertyTypes whether the properties should be 2D (MAP) or 3D (VOLUME)
         /// @return    a list of the selected unrecorded properties
         PropertyValueList * getPropertyUnrecordedValues( int selectionFlags,
            const Interface::Property * property, const Interface::Snapshot * snapshot,
            const Interface::Reservoir * reservoir, const Interface::Formation * formation, const Interface::Surface * surface,
            int propertyType ) const;

         /// @brief Return a list of property values based on the given arguments.
         /// @param[in] list The property value list (recorded or not)
         /// @param[in] selectionFlags is logical OR for the following flags:
         ///                            SURFACE = surface property which per definition is 2D.
         /// 	                         FORMATION = formation property which can be 2D and 3D
         /// 	                         FORMATIONSURFACE = a surface property that is not continuous over the surface.
         /// 	                         RESERVOIR = properties which apply to a reservoir and are therefore 2D.
         /// @param property
         /// @param[in] snapshot properties belonging to this snapshot. If not specified, return
         ///            properties for all snapshots.
         /// @param[in] reservoir properties belonging to this reservoir.
         /// @param[in] formation properties belonging to this formation.
         /// @param[in] surface properties belonging to this surface.
         /// @param[in] propertyTypes whether the properties should be 2D (MAP) or 3D (VOLUME)
         /// @return    a list of the selected properties inside the property value list
         PropertyValueList * getPropertyValuesForList( MutablePropertyValueList list,
            int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
            const Property * property = nullptr, const Snapshot * snapshot = nullptr,
            const Reservoir * reservoir = nullptr, const Formation * formation = nullptr,
            const Surface * surface = nullptr, int propertyTypes = MAP | VOLUME ) const;

         virtual unsigned int deletePropertyValueGridMaps( int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
            const Property * property = nullptr, const Snapshot * snapshot = nullptr,
            const Reservoir * reservoir = nullptr, const Formation * formation = nullptr,
            const Surface * surface = nullptr, int propertyTypes = MAP | VOLUME ) const;

         void deletePropertyValues( int selectionFlags,
            const Property * property = nullptr, const Snapshot * snapshot = nullptr,
            const Reservoir * reservoir = nullptr, const Formation * formation = nullptr,
            const Surface * surface = nullptr, int propertyTypes = MAP | VOLUME );

         void deletePropertyValues( void );
         void deleteRecordLessMapPropertyValues( void );
         void deleteRecordLessVolumePropertyValues( void );
         unsigned int deletePropertiesValuesMaps ( const Snapshot * snapshot );

         /// return a list of PropertyValues based on the given arguments.
         /// if an argument equals 0, it is used as a wild card
         bool hasPropertyValues( int selectionFlags,
            const Property * property, const Snapshot * snapshot,
            const Reservoir * reservoir, const Formation * formation,
            const Surface * surface, int propertyType ) const;


         /// Create a Map-based PropertyValue
         virtual PropertyValue * createMapPropertyValue( const std::string & propertyValueName, const Snapshot * snapshot,
            const Reservoir * reservoir, const Formation * formation, const Surface * surface );

         /// Create a Volume-based PropertyValue
         virtual PropertyValue * createVolumePropertyValue( const std::string & propertyValueName, const Snapshot * snapshot,
            const Reservoir * reservoir, const Formation * formation, unsigned int depth, const std::string & fileName = "" );

         void printPropertyValues( PropertyValueList * propertyValues ) const;

         void addProperty( Property * property );
         void addPropertyToFront( Property * property );

         // Function supporting the implementation
         PropertyValue * addPropertyValue( database::Record * record, const std::string & name, const Property * property, const Snapshot * snapshot,
            const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage, const std::string & fileName = "" );

         /// Utilities to parse a HDF5 file
         void setCurrentSnapshot( const Snapshot * snapshot );
         const Snapshot * getCurrentSnapshot( void );

         /// Utilities to parse a HDF5 file
         void setCurrentProperty( const Property * property );
         const Property * getCurrentProperty( void );

         /// Utilities to parse a HDF5 file
         void setCurrentPropertyValueName( const std::string & name );
         const std::string & getCurrentPropertyValueName( void );

         /// Create a PropertyValue GridMap from the data found in the HDF file specified by fileName at the location specified by propertyId and
         /// add it to the specified parent at the specified index
         GridMap * loadOutputMap( const Parent * parent, unsigned int childIndex, const std::string & fileName, const std::string & propertyId );

         /// Create a GridMap from the data found in the HDF file specified by filePathName at the location specified by dataSetName and
         /// add it to the specified parent at the specified index
         GridMap * loadGridMap( const Parent * parent, unsigned int childIndex, const std::string & filePathName, const std::string & dataSetName );

         /// Get the name of the output directory
         virtual std::string getOutputDir( void ) const;
         virtual const std::string  getFullOutputDir( void ) const;
         virtual bool makeOutputDir() const;

         void resetSnapshotIoTbl(  ) const;

         const std::string & getCrustIoTableName();
         database::Table* getCrustIoTable( void );

         virtual bool loadCrustThinningHistory( void );
         virtual bool loadMantleThicknessHistory( void );
         virtual bool loadHeatFlowHistory( void );
         virtual bool loadRelatedProjects();

         
         bool addCrustThinningHistoryMaps(void);

         //-----------------------------------------------------//
         bool correctCrustThicknessHistory();

         /// Load the input map specified by the given arguments
         GridMap * loadInputMap( const std::string & referringTable, const std::string & mapName );

         const Grid * findOutputGrid( int numI, int numJ ) const;
         const Grid * findGrid( int numI, int numJ ) const;

         /// return the Grid that is used to produce new PropertyValues
         const Grid * getActivityOutputGrid( void ) const;

         virtual const BiodegradationParameters* getBiodegradationParameters() const;

         virtual const FracturePressureFunctionParameters* getFracturePressureFunctionParameters() const;

         virtual const DiffusionLeakageParameters* getDiffusionLeakageParameters() const;

         virtual const CrustFormation* getCrustFormation() const;

         virtual const MantleFormation* getMantleFormation() const;

         virtual const RunParameters* getRunParameters() const;

         virtual const ProjectData* getProjectData() const;

         BottomBoundaryConditions getBottomBoundaryConditions() const;

				 bool isALC() const;
		 bool isFixedTempBasement() const;
		 bool isFixedHeatFlow() const;

         const std::string & getCrustPropertyModel() const;
         const std::string & getMantlePropertyModel() const;
         const std::string & getCrustLithoName() const;
         const std::string & getMantleLithoName() const;
         double getTopAsthenosphericTemperature() const;
         int getMaximumNumberOfMantleElements() const;

         virtual PointAdsorptionHistoryList* getPointAdsorptionHistoryList( const std::string& sourceRockFormationName = "" ) const;

         virtual LangmuirAdsorptionIsothermSampleList* getLangmuirAdsorptionIsothermSampleList( const std::string& functionName ) const;

         virtual const LangmuirAdsorptionTOCEntry* getLangmuirAdsorptionTOCEntry( const std::string& langmuirName ) const;

         virtual const IrreducibleWaterSaturationSample* getIrreducibleWaterSaturationSample() const;

         virtual const SGDensitySample* getSGDensitySample() const;

         /// \brief Determine if any source rock layer contains sulphur.
         virtual bool containsSulphur() const;

         virtual int getRank() const;

         virtual int getSize() const;

         /// \brief Added a barrier function.
         ///
         /// A process will wait here until all processes have called the barrier function.
         /// In serial programs this is a skip operation, in MPI programs this calls the MPIBarrier.
         void barrier() const;

         /// \brief Function to get global min/max values
         ///
         /// In serial program this is a skip operation, in MPI program this calls the mpi-allReduce.
         void getMinValue( double * localMin, double * globalMin ) const;
         void getMaxValue( double * localMax, double * globalMax ) const;

         /// \brief Function to get global min/max values
         ///
         /// In serial programs this returns the local value, in MPI programs this calls the mpi-allReduce to find the global minimum/maximum.
         void getMinValue( int localValue, int& globalValue ) const;

         void getMaxValue( int localValue, int& globalValue ) const;

         const DataAccess::Interface::MessageHandler& getMessageHandler() const;

         const DataAccess::Interface::ApplicationGlobalOperations& getGlobalOperations() const;

         DataAccess::Interface::PermafrostEvent * getPermafrostData() const;

         /// Return whether or not to model permafrost
         bool getPermafrost() const;

         /// Set whether or not to model permafrost
         void setPermafrost( const bool aPermafrost );

         /// Return whether or not it is the beginning of an igneous intrusion
         double getPreviousIgneousIntrusionTime( const double Current_Time );

         /// connect Reservoirs to Formations
         bool connectReservoirs (void);

         /// get primary properties flag
         bool isPrimaryDouble() const;

         /// set primary properties flag
         void setPrimaryDouble( const bool primaryFlag );

         const std::string & getActivityName( void ) const;

         bool isPrimaryProperty( const std::string propertyName ) const;

         void mapFileCacheDestructor( void );
         void mapFileCacheCloseFiles(void);

         database::ProjectFileHandlerPtr getProjectFileHandler () const;

         /// check of the m_trappers is not empty
         bool trappersAreAvailable();

         /// check of the m_traps is not empty
         bool trapsAreAvailable() const;

         /// @defgroup NodesValidation
         /// @{
         /// @brief Initialize the valid-node array.
         /// @details This function uses only the input data maps and is sufficient to construct
         /// all of the necessary items in the cauldron model, e.g. sea-surface temperature,
         /// crust-thickness history, etc. It may be necessary to restrict further the
         /// valid nodes with other input data, e.g. fct-correction maps, or ves property.
         bool initialiseValidNodes( const bool readSizeFromVolumeData );
         /// \return Whether or not the node is defined.
         bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;
         /// \return the validator which contains node validity information
         const Validator& getValidator () const;
         /// @}
         ///

         static bool isEqualTime( double t1, double t2 );

      protected:

         /// the full path of the project
         const std::string m_name;

         /// the directory in which the project is to be found
         std::string m_projectPath;
         /// The name of the project file
         std::string m_fileName;
         /// The name of the project
         std::string m_projectName;

         /// A function to extract the project directory and the project file name
         void splitName( void );

         /// The name of the current activity producing output values
         std::string m_activityName;

         database::ProjectFileHandlerPtr m_projectFileHandler;

         const ObjectFactory * m_factory;

         std::shared_ptr<ReservoirOptions> m_reservoirOptions;

         // All the lists

         MutableSnapshotList m_snapshots;
         MutableLithoTypeList m_lithoTypes;
         MutableSourceRockList m_sourceRocks;
         MutableFormationList m_formations;
         MutableSurfaceList m_surfaces;
         MutableReservoirList m_reservoirs;
         MutableMobileLayerList m_mobileLayers;
         MutableFluidTypeList m_fluidTypes;

         MutablePaleoSurfacePropertyList   m_heatFlowHistory;
         MutablePaleoFormationPropertyList m_crustPaleoThicknesses;
         MutablePaleoFormationPropertyList m_OceaCrustPaleoThicknesses;
         MutablePaleoFormationPropertyList m_mantlePaleoThicknesses;
         const TableCTC                          m_tableCTC;
         const TableCTCRiftingHistory            m_tableCTCRiftingHistory;

         // Should really be a list of PaleoSurfaceProperty's,
         // but there is no surface defined for the top surface.
         MutablePaleoPropertyList m_surfaceDepthHistory;
         MutablePaleoPropertyList m_surfaceTemperatureHistory;

         MutableAllochthonousLithologyList m_allochthonousLithologies;
         MutableAllochthonousLithologyDistributionList m_allochthonousLithologyDistributions;
         MutableAllochthonousLithologyInterpolationList m_allochthonousLithologyInterpolations;
         MutableTrapList m_traps;
         MutableTrapperList m_trappers;
         MutableMigrationList m_migrations;
         MutableInputValueList m_inputValues;
         MutablePropertyList m_properties;
         MutablePropertyValueList m_propertyValues;
         MutablePropertyValueList m_recordLessMapPropertyValues;
         MutablePropertyValueList m_recordLessVolumePropertyValues;
         MutableOutputPropertyList m_timeOutputProperties;
         MutableRelatedProjectList m_relatedProjects;
         MutableConstrainedOverpressureIntervalList m_constrainedOverpressureIntervals;
         MutablePermafrostEventList m_permafrostEvents;

         /// \brief A set of all of the igneous intrusion contained within a project.
         MutableIgneousIntrusionEventList m_igneousIntrusionEvents;

         BiodegradationParameters* m_biodegradationParameters;
         FracturePressureFunctionParameters* m_fracturePressureFunctionParameters;
         DiffusionLeakageParameters* m_diffusionLeakageParameters;

         RunParameters* m_runParameters;
         ProjectData* m_projectData;
         MutableSimulationDetailsList m_simulationDetails;

         /// The crust- and mantle-formations do not have to be deallocated directly. Since they are added
         /// to the list of formations in the model they will be deleted when this object is destroyed.
         CrustFormation* m_crustFormation;
         MantleFormation* m_mantleFormation;

         BottomBoundaryConditions m_bottomBoundaryConditions;
         std::string m_crustPropertyModel;
         std::string m_mantlePropertyModel;
         std::string m_crustLithoName;
         std::string m_mantleLithoName;
         double m_topAsthenosphericTemperature;
         double m_equilibriumOceanicLithosphereThickness;
         int    m_maximumNumberOfMantleElements;


         /// The surfaces of the basement formations.
         /// The crust- and mantle-surfaces do not have to be deallocated directly. Since they are added
         /// to the list of surfaces in the model they will be deleted when this object is destroyed.
         BasementSurface* m_crustBottomSurface;
         BasementSurface* m_mantleBottomSurface;

         MutableLithologyHeatCapacitySampleList m_lithologyHeatCapacitySamples;
         MutableLithologyThermalConductivitySampleList m_lithologyThermalConductivitySamples;

         MutableFluidThermalConductivitySampleList m_fluidThermalConductivitySamples;
         MutableFluidHeatCapacitySampleList        m_fluidHeatCapacitySamples;

         MutableFaultCollectionList m_faultCollections;

         mutable Grid * m_inputGrid;
         mutable Grid * m_highResOutputGrid;
         mutable Grid * m_lowResOutputGrid;

         Validator m_validator; ///< Contains information about nodes validity

         const Grid * m_activityOutputGrid; // grid in which the output is computed by the activity
         bool m_saveAsInputGrid; // whether to use the input grid to save the computed output


         /// I.e. whether permafrost is being modeled
         bool m_permafrost;

         MapWriter * m_mapPropertyValuesWriter;

         // flag to output primary properties in a double precision
         bool m_primaryDouble;

         int m_rank;
         int m_size;

         void mapFileCacheConstructor( void );

         void checkForValidPartitioning( const std::string & name, int M, int N ) const;

         enum { auxiliary, hrdecompaction, genex, fastcauldron };

         void * m_mapFileCache;

         /// Variables used in the traversal of HDF5 files.
         const Snapshot * m_currentSnapshot;
         const Property * m_currentProperty;
         std::string m_currentPropertyValueName;

         void resetActivityName( void );
         /// set the name of the data production activity
         bool setActivityName( const std::string & name );


         void resetActivityOutputGrid( void );

         void loadSnapshots( );
         bool createSnapshotsAtGeologicalEvents( );
         
         void createSnapshotsAtUserDefinedTimes( );

         bool loadLithoTypes( void );

         bool loadSurfaces( void );
         bool loadFormations( void );
         bool loadReservoirs( void );
         bool loadGlobalReservoirOptions (void);
         bool loadMobileLayers( void );
         bool loadAllochthonousLithologies( void );
         bool loadAllochthonousLithologyDistributions( void );
         bool loadAllochthonousLithologyInterpolations( void );
         bool loadTraps( void );
         bool loadTrappers( void );
         bool loadMigrations( void );
         bool loadInputValues( void );
         bool loadProperties( void );
         bool loadMapPropertyValues( void );
         bool loadVolumePropertyValues( void );
         bool loadVolumePropertyValuesViaSnapshotIoTbl( void );
         bool loadVolumePropertyValuesVia3DTimeIoTbl( void );

         bool loadIgneousIntrusions();

         bool loadCrustFormation();
         bool loadMantleFormation();
         bool loadBasementSurfaces();

         bool loadRunParameters( void );
         bool loadProjectData( void );
         bool loadSurfaceDepthHistory( void );
         bool loadSurfaceTemperatureHistory( void );
         bool loadTimeOutputProperties( void );
         bool loadDepthOutputProperties( void );
         bool loadSimulationDetails ();

         bool loadFluidThermalConductivitySamples();
         bool loadFluidHeatCapacitySamples();
         bool loadLithologyHeatCapacitySamples();
         bool loadLithologyThermalConductivitySamples();

         // There is nothing to delete here.
         bool loadBottomBoundaryConditions();

         bool loadBiodegradationParameters( void );
         bool loadFracturePressureFunctionParameters( void );
         bool loadDiffusionLeakageParameters( void );

         bool connectFaultCollections( FaultCollection* faultCollection ) const;
         bool loadFaultEvents( FaultCollection* faultCollection ) const;

         bool loadFluidTypes();

         void loadPermafrostData();

         void computeMantlePaleoThicknessHistory() const;

         bool initializeMapPropertyValuesWriter( const bool append = false );
         bool finalizeMapPropertyValuesWriter( void );

         bool saveCreatedMapPropertyValues( void );

         bool saveCreatedVolumePropertyValues( void );

         /// connect Surfaces to Formations
         bool connectSurfaces( void );

         /// connect Traps to Reservoirs and Snapshots
         bool connectTraps( void );

         /// connect Traps to Reservoirs and Snapshots
         bool connectTrappers( void );

         bool connectMigrations( void );

         /// Connect spilling traps
         bool connectUpAndDownstreamTrappers( void ) const;

         Trapper * findTrapper( const MutableTrapperList & trappers, const Reservoir * reservoir,
            const Snapshot * snapshot, unsigned int id, unsigned int persistentId ) const;

         void numberInputValues( void );

         void loadLangmuirIsotherms();
         void loadLangmuirTOCEntries();
         void loadPointHistories();
         void loadIrreducibleWaterSaturationSample();
         void loadSGDensitySample();

         void deleteSnapshots( void );
         void deleteSurfaces( void );
         void deleteFormations( void );
         void deleteLithoTypes( void );
         void deleteSourceRocks( void );
         void deleteReservoirs( void );
         void deleteMobileLayers( void );
         void deleteAllochthonousLithologies( void );
         void deleteAllochthonousLithologyDistributions( void );
         void deleteAllochthonousLithologyInterpolations( void );
         void deleteTraps( void );
         void deleteTrappers( void );
         void deleteMigrations( void );
         void deleteInputValues( void );
         void deleteProperties( void );
         void deleteFluidTypes();

         void deleteIgneousIntrusions();
         void deletePermafrost();

         void deleteBiodegradationParameters( void );
         void deleteFracturePressureFunctionParameters( void );
         void deleteDiffusionLeakageParameters( void );

         void deleteCrustFormation();
         void deleteMantleFormation();

         void deleteHeatFlowHistory( void );
         void deleteCrustThinningHistory( void );
         void deleteMantleThicknessHistory( void );

         void deleteSurfaceDepthHistory( void );
         void deleteSurfaceTemperatureHistory( void );
         void deleteRunParameters( void );
         void deleteProjectData( void );
         void deleteSimulationDetails ();

         void deleteTimeOutputProperties( void );
         void deleteDepthOutputProperties( void );
         void deleteLithologyHeatCapacitySamples( void );
         void deleteLithologyThermalConductivitySamples( void );
         void deleteFluidThermalConductivitySamples();
         void deleteFluidHeatCapacitySamples();
         void deleteRelatedProjects();
         void deleteConstrainedOverpressureIntervals();

         void deleteLangmuirIsotherms();
         void deleteLangmuirTOCEntries();
         void deletePointHistories();
         void deleteIrreducibleWaterSaturationSample();
         void deleteSGDensitySample();

         void deleteFaultCollections();

         void addUndefinedAreas( const GridMap* theMap );


         MutablePointAdsorptionHistoryList m_adsorptionPointHistoryList;
         MutableLangmuirAdsorptionIsothermSampleList m_langmuirIsotherms;
         IrreducibleWaterSaturationSample* m_irreducibleWaterSample;

      private:
         void fillSnapshotIoTbl( std::list<double>& ages, const bool isUserDefined );
         double getStartAge() const;

         static float GetUndefinedValue( hid_t fileId );

         /// \brief Allocate architecture related classes.
         ///
         /// E.g. Message-handler, global-operations.
         void allocateArchitectureRelatedParameters();

         double m_previousIgneousIntrusionTime;

         MutableLangmuirAdsorptionTOCEntryList m_langmuirTocAdsorptionEntries;

         SGDensitySample* m_sgDensitySample;

         DataAccess::Interface::MessageHandler* m_messageHandler;
         DataAccess::Interface::ApplicationGlobalOperations* m_globalOperations;

		 bool addTheThinningHistoryMaps(const std::string& TableName, PaleoPropertyMapAttributeId propId,
			 MutablePaleoFormationPropertyList& theThicknessEntries,
			 MutablePaleoFormationPropertyList& newCrustalThicknesses);
		 // \the following two methods is be clubbed together
		 bool addContiCrustThinningHistoryMaps(void);
		 bool addOceaCrustThinningHistoryMaps(void);
         /// List of the primary properties
         std::set<std::string> m_primaryList;

         friend class migration::Migrator;
         void getDomainShape(const int windowMinI, const int windowMaxI, const int windowMinJ, const int windowMaxJ, std::vector<std::vector<int> >& domainShape) const;

      };
   }
}

//------------------------------------------------------------//

inline bool DataAccess::Interface::ProjectHandle::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_validator.isValid( i, j );
}

//------------------------------------------------------------//

inline const DataAccess::Interface::Validator& DataAccess::Interface::ProjectHandle::getValidator () const {
   return m_validator;
}

#endif // INTERFACE_PROJECTHANDLE_H
