#ifndef _INTERFACE_PROJECTHANDLE_H_
#define _INTERFACE_PROJECTHANDLE_H_

#include "hdf5.h"

#include <string>
#include <vector>
using namespace std;

#include "Interface.h"

#include "database.h"

#include "MessageHandler.h"
#include "ApplicationGlobalOperations.h"

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
	   
      /// Create a project from a project file with the given name and access mode ("r" or "rw") and
      /// return the associated ProjectHandle
      ProjectHandle * OpenCauldronProject (const string & name, const string & accessMode);

      void InitializeSerializedIO ();


      /// Close the project associated with the given ProjectHandle
      void CloseCauldronProject (ProjectHandle * projectHandle);

      /// A ProjectHandle contains references to the entities in a Project.
      /// Objects of this class are created and destroyed via the
      /// functions OpenCauldronProject() and CloseCauldronProject(), respectively.
      class ProjectHandle
      {
	 public:
	    /// Constructor
	    ProjectHandle (database::Database * database, const string & name, const string & accessMode);

	    /// Destructor
	    virtual ~ProjectHandle (void);

	    int GetNumberOfSpecies (void);
	    const string & GetSpeciesName (int i);

	    database::Database * getDataBase (void) const;

	    /// Get a handle to the Table with the given name
	    database::Table * getTable (const string & tableName) const;

	    /// Specify a new ObjectFactory
	    static bool UseFactory (ObjectFactory * factory);

	    /// return the ObjectFactory
	    ObjectFactory * getFactory (void) const;

	    /// save the project to the specified file
	    bool saveToFile (const string & fileName);

	    /// Return the full file name of the project
	    virtual const string & getName (void) const;
	    /// Return the directory of the project
	    virtual const string & getProjectPath (void) const;
	    /// Return the name of the project without the .project extension
	    virtual const string & getProjectName (void) const;
	    /// Return the file name of the project without the directory path
	    virtual const string & getFileName (void) const;

	    /// start a new activity
	    bool startActivity (const string & name, const Grid * grid);

	    /// report completion of an activity to the in-core project file.
	    bool reportActivityCompletion (void);

	    /// Restart an activity.
	    bool restartActivity (void);

	    /// continue an activity
	    bool continueActivity (void);

	    /// abort an activity
	    /// identical to finishActivity (false)
	    bool abortActivity (void);

	    /// finish an activity
	    bool finishActivity (bool isComplete = true);

	    /// Returns the Grid in which the input maps of this project are specified.
	    virtual const Grid * getInputGrid (void) const;

	    /// Returns the Grid of the output GridMaps that were produced by either a migration run or
	    /// a high resolution decompaction run. This grid is a subgrid of the input grid.
	    virtual const Grid * getHighResolutionOutputGrid (void) const;

	    /// Returns the Grid of the output GridMaps that were not produced by either a migration run or
	    /// a high resolution decompaction run. This grid is a subgrid of the high resolution output grid.
	    virtual const Grid * getLowResolutionOutputGrid (void) const;

	    /// Find the Snapshot with the given time
	    virtual const Snapshot * findSnapshot (double time, int type = MAJOR) const;
	    /// Find the LithoType with the given name
	    virtual const LithoType * findLithoType (const string & name) const;
	    /// Find the Formation with the given name
	    virtual const Formation * findFormation (const string & name) const;
	    /// Find the Surface with the given name
	    virtual const Surface * findSurface (const string & name) const;
	    /// Find the Reservoir with the given name
	    virtual const Reservoir * findReservoir (const string & name) const;

	    /// Find the Property  with the given name
	    virtual const Property * findProperty (const string & name) const;

            /// Find and return a const pointer to the property int the time-filter.
            ///
            /// If the name is not found then a null value will be returned.
	    virtual const OutputProperty * findTimeOutputProperty (const std::string & propertyName) const;

	    /// Find the InputValue  with the given attributes
	    virtual const InputValue * findInputValue (const string & tableName, const string & mapName) const;
	    /// Find the SourceRock with the given layer name
	    virtual const SourceRock * findSourceRock (const string & name) const;
	    /// return the AllochthonousLithology of the specified formation.
	    /// if formation equals 0, then null is returned.
	    virtual const AllochthonousLithology * findAllochthonousLithology (const string& formationName) const;
            /// Find the FluidType with the given name.
            virtual const FluidType* findFluid ( const string& name ) const;
         
	    /// return the list of LithoType objects.
	    virtual LithoTypeList * getLithoTypes (void) const;
	    /// return the list of snapshots.
	    virtual SnapshotList * getSnapshots (int type = MAJOR) const;
	    /// return the list of formations, all or for a given snapshot.
	    virtual FormationList * getFormations (const Snapshot * snapshot = 0,
                                                              const bool                  includeBasement = false ) const;
	    /// return the list of surfaces, all or for a given snapshot.
	    virtual SurfaceList * getSurfaces (const Snapshot * snapshot = 0,
                                                          const bool                  includeBasement = false ) const;
	    /// return the list of TouchstoneMaps
	    virtual TouchstoneMapList * getTouchstoneMaps (void) const;
	    /// return the list of reservoirs
	    virtual ReservoirList * getReservoirs (const Formation * formation = 0) const;
	    /// return the list of MobileLayers
	    virtual MobileLayerList * getMobileLayers (const Formation * formation) const;

            /// Return a list of all fluids in project.
            virtual FluidTypeList* getFluids () const;


            virtual PaleoFormationPropertyList * getCrustPaleoThicknessHistory () const;

            virtual PaleoFormationPropertyList * getMantlePaleoThicknessHistory () const;

            /// return the list of surface temperature history.
            // Would it be better to have a ModelTopSurface kind of object, that also had the top boundary conditions?
            virtual PaleoPropertyList * getSurfaceTemperatureHistory () const;

            /// return the list of surface depth history.
            // Would it be better to have a ModelTopSurface kind of object, that also had the top boundary conditions?
            virtual PaleoPropertyList * getSurfaceDepthHistory () const;

	    /// return the list of heat-flow paleo-surface properties.
	    virtual PaleoSurfacePropertyList * getHeatFlowHistory () const;

	    /// return the list of AllochthonousLithologyDistributions of the specified AllochthonousLithology.
	    /// If allochthonousLithology equals 0, then null is returned.
	    virtual AllochthonousLithologyDistributionList * getAllochthonousLithologyDistributions (const AllochthonousLithology * allochthonousLithology = 0) const;

	    /// return the list of AllochthonousLithologyInterpolations from the specified formation.
	    /// If formation equals 0, then all allochthonous lithology interpolations are returned.
	    virtual AllochthonousLithologyInterpolationList * getAllochthonousLithologyInterpolations (const AllochthonousLithology * allochthonousLithology = 0) const;

            /// Return a list of the time-output properties for a current modelling mode.
            virtual OutputPropertyList * getTimeOutputProperties () const;

            /// Return a list of the depth-output properties for a current modelling mode.
            virtual OutputPropertyList * getDepthOutputProperties () const;


            /// Return a list to the heat-capacity samples for the lithology.
            ///
            /// If litho == 0 then all heat capacity samples will be returned.
            virtual LithologyHeatCapacitySampleList * getLithologyHeatCapacitySampleList ( const LithoType* litho = 0 ) const;

            /// Return a list to the thermal-conductivity samples for the lithology.
            ///
            /// If litho == 0 then all thermal-conductivity samples will be returned.
            virtual LithologyThermalConductivitySampleList * getLithologyThermalConductivitySampleList ( const LithoType* litho = 0 ) const;

            /// Return a list to the heat-capacity samples for the fluid.
            ///
            /// If fluid == 0 then all heat-capacity samples will be returned.
            virtual FluidHeatCapacitySampleList * getFluidHeatCapacitySampleList ( const FluidType* fluid ) const;

            /// Return a list to the heat-capacity samples for the fluid.
            ///
            /// If fluid == 0 then all heat-capacity samples will be returned.
            virtual FluidThermalConductivitySampleList * getFluidThermalConductivitySampleList ( const FluidType* fluid ) const;

            /// Return a list to the density samples for the fluid.
            ///
            /// If fluid == 0 then all density samples will be returned.
            virtual FluidDensitySampleList * getFluidDensitySampleList ( const FluidType* fluid ) const;

            /// Return a list to the related projects.
            virtual RelatedProjectList * getRelatedProjectList () const;

	    /// return the list of FaultCollections for a Formation
	    virtual FaultCollectionList * getFaultCollections (const Formation * formation) const;

	    /// Find a fault collection
	    virtual FaultCollection * findFaultCollection (const string & name) const;

	    /// Load and process the fault descriptions
	    virtual bool loadFaults (void);

            virtual ConstrainedOverpressureIntervalList* getConstrainedOverpressureIntervalList ( const Formation* formation ) const;

            virtual bool loadConstrainedOverpressureIntervals ();

	    /// return a list of Migration objects based on the given arguments.
	    /// if an argument equals 0, it is used as a wildcard.
	    virtual MigrationList * getMigrations (const string & process, const Formation * sourceFormation,
		  const Snapshot * sourceSnapshot, const Reservoir * sourceReservoir, const Trapper * sourceTrapper,
		  const Snapshot * destinationSnapshot, const Reservoir * destinationReservoir, const Trapper * destinationTrapper
		  ) const;

	    /// return a list of Trapper objects based on the given arguments.
	    /// if an argument equals 0, it is used as a wildcard.
	    virtual TrapperList * getTrappers (const Reservoir * reservoir,
		  const Snapshot * snapshot, unsigned int id, unsigned int persistentId) const;

	    /// return a list of Trap objects based on the given arguments.
	    /// if an argument equals 0, it is used as a wildcard.
	    virtual TrapList * getTraps (const Reservoir * reservoir,
		  const Snapshot * snapshot, unsigned int id) const;

	    /// return a Trapper that meets the given condition arguments.
	    /// If an argument equals 0, it is used as a wildcard.
	    virtual const Trapper * findTrapper (const Reservoir * reservoir,
		  const Snapshot * snapshot, unsigned int id, unsigned int persistentId) const;

	    /// return a Trap that meets the given condition arguments.
	    /// If an argument equals 0, it is used as a wildcard.
	    /// If all arguments are non-0, at most one trap can meet the conditions.
	    virtual const Trap * findTrap (const Reservoir * reservoir,
		  const Snapshot * snapshot, unsigned int id) const;

	    /// return the list of Leads
	    virtual LeadList * getLeads (void) const;

	    /// return a Lead with the given id
	    virtual const Lead * findLead (unsigned int id) const;

	    /// return the list of (Mapped)InputValues
	    virtual InputValueList * getInputValues (void) const;

	    /// Return a list of Properties which match the given arguments.
            ///
            /// Return a list of Property classes which match the given arguments.
            ///
            /// @param[in] all =  SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR
            /// @param[in] selectionFlags:
	    /// 	   SURFACE = surface property which per definition is 2D.
            /// 	   FORMATION = formation property which can be 2D and 3D
            /// 	   FORMATIONSURFACE = a surface property that is not continuous over the surface.
            /// 	   RESERVOIR = properties which apply to a reservoir and are therefore 2D.
            /// 		  const Snapshot * snapshot = 0,
            /// @param[in] snapshot: properties belonging to this snapshot. If not specified, return 
            ///            properties for all snapshots.
	    /// @param[in] reservoir: properties belonging to this reservoir.
            /// @param[in] formation: properties belonging to this formation.
	    /// @param[in] surface: properties belonging to this surface.
            /// @param[in] propertyTypes: whether the properties should be 2D (MAP) or 3D (VOLUME)
            /// @return    a list of the selected properties
	    virtual PropertyList * getProperties (bool all = false, int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
		  const Snapshot * snapshot = 0,
		  const Reservoir * reservoir = 0, const Formation * formation = 0,
		  const Surface * surface = 0, int propertyTypes = MAP | VOLUME) const;

	    /// Return a list of property values based on the given arguments.
            ///
            /// @param[in] all =  SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR
            /// @param[in] selectionFlags:
	    /// 	   SURFACE = surface property which per definition is 2D.
            /// 	   FORMATION = formation property which can be 2D and 3D
            /// 	   FORMATIONSURFACE = a surface property that is not continuous over the surface.
            /// 	   RESERVOIR = properties which apply to a reservoir and are therefore 2D.
            /// 		  const Snapshot * snapshot = 0,
            /// @param[in] snapshot: properties belonging to this snapshot. If not specified, return 
            ///            properties for all snapshots.
	    /// @param[in] reservoir: properties belonging to this reservoir.
            /// @param[in] formation: properties belonging to this formation.
	    /// @param[in] surface: properties belonging to this surface.
            /// @param[in] propertyTypes: whether the properties should be 2D (MAP) or 3D (VOLUME)
            /// @return    a list of the selected properties
	    virtual PropertyValueList * getPropertyValues (int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
		  const Property * property = 0, const Snapshot * snapshot = 0,
		  const Reservoir * reservoir = 0, const Formation * formation = 0,
		  const Surface * surface = 0, int propertyTypes = MAP | VOLUME) const;

	    virtual unsigned int deletePropertyValueGridMaps (int selectionFlags = SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR,
		  const Property * property = 0, const Snapshot * snapshot = 0,
		  const Reservoir * reservoir = 0, const Formation * formation = 0,
		  const Surface * surface = 0, int propertyTypes = MAP | VOLUME) const;

	    void deletePropertyValues (int selectionFlags,
		  const Property * property = 0, const Snapshot * snapshot = 0,
		  const Reservoir * reservoir = 0, const Formation * formation = 0,
		  const Surface * surface = 0, int propertyTypes = MAP | VOLUME);


	    /// return a list of PropertyValues based on the given arguments.
	    /// if an argument equals 0, it is used as a wildcard
	    virtual bool hasPropertyValues (int selectionFlags,
		  const Property * property, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation,
		  const Surface * surface, int propertyType) const;


	    /// Create a Map-based PropertyValue
	    virtual PropertyValue * createMapPropertyValue (const string & propertyValueName, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation, const Surface * surface);

	    /// Create a Volume-based PropertyValue
	    virtual PropertyValue * createVolumePropertyValue (const string & propertyValueName, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation, unsigned int depth);

	    void printPropertyValues (PropertyValueList * propertyValues) const;

	    void cleanUpPropertyValues (database::Table * timeIoTbl,
		  database::Transaction * transaction,
		  PropertyValueList * propertyValuesToRetain) const;


	    /// Save a project to a directory in a format that the BPA server can handle
	    virtual bool saveForBPA (const string & filePath, PropertyValueList * propertyValues,
		  vector<string> & inputMapFileNames, vector<string> & outputMapFileNames) const;

	    /// update the original project file with upload information
	    virtual bool updateOriginal (const string & serverName, const string & serverProjectName,
		  const string & ownerOrgName, const string & checksum, PropertyValueList * propertyValues) const;

	    // virtual const string & getBPAServer (void) const;
	    // virtual const string & getBPAName (void) const;
	    // virtual const string & getBPAOwnerOrg (void) const;
	    // virtual const string & getBPAChecksum (void) const;

	    void computeInputChecksums (const string & rootDirectory, database::Transaction * transaction) const;

	    void computeOutputChecksums (database::Transaction * transaction, const string & rootDirectory,
		  PropertyValueList * propertyValues) const;

	    virtual string computeChecksum (const string & fileName) const;


	    void addProperty (Property * property);
	    // Function supporting the implementation
	    PropertyValue * addPropertyValue (database::Record * record, const string & name, const Property * property, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage);

            /// Utilities to parse a HDF5 file
	    void setCurrentSnapshot (const Snapshot * snapshot);
	    const Snapshot * getCurrentSnapshot (void);

            /// Utilities to parse a HDF5 file
	    void setCurrentProperty (const Property * property);
	    const Property * getCurrentProperty (void);

            /// Utilities to parse a HDF5 file
	    void setCurrentPropertyValueName (const string & name);
	    const string & getCurrentPropertyValueName (void);

            /// Utilities to parse a HDF5 file
	    //void setLayerName (const string & layerName);
	    //const string & getLayerName (void);

            /// Create a PropertyValue GridMap from the data found in the HDF file specified by fileName at the location specified by propertyId and
            /// add it to the specified parent at the specified index
	    GridMap * loadOutputMap (const Parent * parent, unsigned int childIndex, const string & fileName, const string & propertyId);

	    /// In the given file find the name of the dataset that starts with "Layer=" and ends with the given layerIndex
	    //const string & findLayerName (const string & filePathName, int layerIndex);

            /// Create a GridMap from the data found in the HDF file specified by filePathName at the location specified by dataSetName and
            /// add it to the specified parent at the specified index
	    GridMap * loadGridMap (const Parent * parent, unsigned int childIndex, const string & filePathName, const string & dataSetName);

            /// Get the name of the output directory
	    virtual const string & getOutputDir (void) const;

	    bool setOutputDir (database::Transaction * transaction, const string & fileOrDirName) const;

	    void resetSnapshotIoTbl (database::Transaction * transaction) const;

            const string & getCrustIoTableName();
            database::Table* getCrustIoTable(void);

            virtual bool loadCrustThinningHistory (void);
            virtual bool loadMantleThicknessHistory (void);
            virtual bool loadHeatFlowHistory (void);
            virtual bool loadRelatedProjects ();

            bool addCrustThinningHistoryMaps();

	    /// Load the input map specified by the given arguments
	    GridMap * loadInputMap (const string & referringTable, const string & mapName);

	    static ObjectFactory * GetFactoryToUse (void);

	    Grid * findOutputGrid (int numI, int numJ);
	    Grid * findGrid (int numI, int numJ);

	    /// return the Grid that is used to produce new PropertyValues
	    const Grid * getActivityOutputGrid (void) const;

	    virtual void printOn (ostream & ostr) const;

	    virtual void asString (string &) const;

	    virtual const BiodegradationParameters* getBiodegradationParameters() const;

	    virtual const FracturePressureFunctionParameters* getFracturePressureFunctionParameters() const;

	    virtual const DiffusionLeakageParameters* getDiffusionLeakageParameters() const;

	    virtual const CrustFormation* getCrustFormation () const;

	    virtual const MantleFormation* getMantleFormation () const;

	    virtual const RunParameters* getRunParameters () const;

            virtual const ProjectData* getProjectData () const;

            virtual BottomBoundaryConditions getBottomBoundaryConditions () const;

            virtual const string & getCrustPropertyModel () const;
            virtual const string & getMantlePropertyModel () const;
            virtual double getBottomMantleTemperature () const;

	    virtual ModellingMode getModellingMode () const;

            virtual PointAdsorptionHistoryList* getPointAdsorptionHistoryList ( const std::string& sourceRockFormationName = "" ) const;

            virtual LangmuirAdsorptionIsothermSampleList* getLangmuirAdsorptionIsothermSampleList ( const std::string& functionName ) const;

            virtual const LangmuirAdsorptionTOCEntry* getLangmuirAdsorptionTOCEntry ( const std::string& langmuirName ) const;

            virtual const IrreducibleWaterSaturationSample* getIrreducibleWaterSaturationSample () const;

            virtual const SGDensitySample* getSGDensitySample () const;

            /// \brief Determine if any source rock layer contains sulphur.
            virtual bool containsSulphur () const;

            virtual int getRank () const;

            virtual int getSize () const;

            /// \brief Added a barrier function.
            ///
            /// A process will wait here until all processes have called the barrier function.
            /// In serial programmes this is a skip operation, in MPI programmes this calls the mpi-barrier.
            void barrier () const;

            /// \brief Function to get global min/max values
            ///
            /// In serial programmes this is a skip operation, in MPI programmes this calls the mpi-allReduce.
            void getMinValue ( double * localMin, double * globalMin ) const;
            void getMaxValue ( double * localMax, double * globalMax ) const;

           const DataAccess::Interface::MessageHandler& getMessageHandler () const;

           const DataAccess::Interface::ApplicationGlobalOperations& getGlobalOperations () const;

	 protected:
	    friend ProjectHandle * OpenCauldronProject (const string & name, const string & accessMode);

	    typedef enum { READONLY, READWRITE } AccessMode;
	    //1DComponent
	    bool loadModellingMode(void);
	    ModellingMode m_modellingMode;

	    /// Pointer to the TableIO handler
	    database::Database * const m_database;

            /// the full path of the project
	    const string m_name;

            /// the directory in which the project is to be found
	    string m_projectPath;
            /// The name of the project file
	    string m_fileName;
            /// The name of the project
	    string m_projectName;
            /// The name of the output directory
	    mutable string m_outputDir;

            /// A function to extract the project directory and the project file name
	    void splitName (void);

	    /// The name of the current activity producing output values
	    string m_activityName;

	    const AccessMode m_accessMode;

	    ObjectFactory * m_factory;

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
            MutablePaleoFormationPropertyList m_mantlePaleoThicknesses;
            MutableCrustalThicknessDataList   m_crustalThicknessData;

            // Should really be a list of PaleoSurfaceProperty's, 
            // but there is no surface defined for the top surface.
	    MutablePaleoPropertyList m_surfaceDepthHistory;
	    MutablePaleoPropertyList m_surfaceTemperatureHistory;

	    MutableTouchstoneMapList m_touchstoneMaps;
	    MutableAllochthonousLithologyList m_allochthonousLithologies;
	    MutableAllochthonousLithologyDistributionList m_allochthonousLithologyDistributions;
            MutableAllochthonousLithologyInterpolationList m_allochthonousLithologyInterpolations;
	    MutableTrapList m_traps;
	    MutableTrapperList m_trappers;
	    MutableMigrationList m_migrations;
	    MutableLeadList m_leads;
	    MutableInputValueList m_inputValues;
	    MutablePropertyList m_properties;
	    MutablePropertyValueList m_propertyValues;
	    MutablePropertyValueList m_recordLessMapPropertyValues;
	    MutablePropertyValueList m_recordLessVolumePropertyValues;
            MutableOutputPropertyList m_timeOutputProperties;
            MutableOutputPropertyList m_depthOutputProperties;
            MutableRelatedProjectList m_relatedProjects;
            MutableConstrainedOverpressureIntervalList m_constrainedOverpressureIntervals;

            BiodegradationParameters* m_biodegradationParameters;
            FracturePressureFunctionParameters* m_fracturePressureFunctionParameters;
	    DiffusionLeakageParameters* m_diffusionLeakageParameters;

            RunParameters* m_runParameters;
            ProjectData* m_projectData;

            /// The crust- and mantle-formations do not have to be deallocated directly. Since they are added
            /// to the list of formations in the model they will be deleted when this object is destroyed.
            CrustFormation* m_crustFormation;
            MantleFormation* m_mantleFormation;

            BottomBoundaryConditions m_bottomBoundaryConditions;
            string m_crustPropertyModel;
            string m_mantlePropertyModel;

            /// The surfaces of the basement formations.
            /// The crust- and mantle-surfaces do not have to be deallocated directly. Since they are added
            /// to the list of surfaces in the model they will be deleted when this object is destroyed.
            BasementSurface* m_crustBottomSurface;
            BasementSurface* m_mantleBottomSurface;

            MutableLithologyHeatCapacitySampleList m_lithologyHeatCapacitySamples;
            MutableLithologyThermalConductivitySampleList m_lithologyThermalConductivitySamples;

            MutableFluidDensitySampleList             m_fluidDensitySamples;
            MutableFluidThermalConductivitySampleList m_fluidThermalConductivitySamples;
            MutableFluidHeatCapacitySampleList        m_fluidHeatCapacitySamples;

	    MutableFaultCollectionList m_faultCollections;

	    Grid * m_inputGrid;
	    Grid * m_highResOutputGrid;
	    Grid * m_lowResOutputGrid;
            
	    const Grid * m_activityOutputGrid;

	    MapWriter * m_mapPropertyValuesWriter;

            int m_rank;
            int m_size;

         void mapFileCacheConstructor (void);
         void mapFileCacheDestructor  (void);

         enum { auxiliary, hrdecompaction, genex, fastcauldron };

         void * m_mapFileCache;
		
         /// Variables used in the traversal of HDF5 files.
	    const Snapshot * m_currentSnapshot;
	    const Property * m_currentProperty;
	    string m_currentPropertyValueName;

	    void resetActivityName (void);
	    /// set the name of the data production activity
	    bool setActivityName (const string & name);
	    const string & getActivityName (void) const;

	    void resetActivityOutputGrid (void);
	    /// set the Grid that is used to produce new PropertyValues
	    bool setActivityOutputGrid (const Grid * grid);

	    bool loadSnapshots (void);
	    bool createSnapshotsAtGeologicalEvents(void);
	    bool createSnapshotsAtRiftEvents(void);
	    static bool isEqualTime (double t1, double t2);
	    bool loadGrids (void);
	    bool loadLithoTypes (void);
	    bool loadTouchstoneMaps (void);

#if 0
	    bool loadSourceRocks (void);
#endif
	    bool loadSurfaces (void);
	    bool loadFormations (void);
	    bool loadReservoirs (void);
	    bool loadMobileLayers (void);
	    bool loadAllochthonousLithologies (void);
	    bool loadAllochthonousLithologyDistributions (void);
            bool loadAllochthonousLithologyInterpolations (void);
	    bool loadTraps (void);
	    bool loadTrappers (void);
	    bool loadMigrations (void);
	    bool loadLeads (void);
	    bool loadInputValues (void);
	    bool loadProperties (void);
	    bool loadMapPropertyValues (void);
	    bool loadVolumePropertyValues (void);


            bool loadCrustFormation ();
            bool loadMantleFormation ();
            bool loadBasementSurfaces ();

	    bool loadRunParameters (void);
	    bool loadProjectData (void);
            bool loadSurfaceDepthHistory (void);
            bool loadSurfaceTemperatureHistory (void);
            bool loadTimeOutputProperties (void);
            bool loadDepthOutputProperties (void);

            bool loadFluidDensitySamples ();
            bool loadFluidThermalConductivitySamples ();
            bool loadFluidHeatCapacitySamples ();
            bool loadLithologyHeatCapacitySamples ();
            bool loadLithologyThermalConductivitySamples ();

            // There is nothing to delete here.
            bool loadBottomBoundaryConditions ();

	    bool loadBiodegradationParameters (void);
	    bool loadFracturePressureFunctionParameters (void);
	    bool loadDiffusionLeakageParameters (void);

	    bool loadFaultCollections (void);
	    bool connectFaultCollections (void);
	    bool loadFaultEvents (void);

            bool loadFluidTypes ();

            bool loadCrustalThicknessData ();

            void computeMantlePaleoThicknessHistory () const;

	    Snapshot * createSnapshot (database::Record record);

	    bool initializeMapPropertyValuesWriter (void);
	    bool finalizeMapPropertyValuesWriter (void);

	    bool saveCreatedMapPropertyValues (void);

	    //1DComponent
	    bool saveCreatedMapPropertyValuesMode1D (void);

	    bool saveCreatedMapPropertyValuesMode3D (void);

	    virtual bool saveCreatedVolumePropertyValues (void);

	    bool saveCreatedVolumePropertyValuesMode1D (void);

	    bool saveCreatedVolumePropertyValuesMode3D (void);

            /// connect Surfaces to Formations
	    bool connectSurfaces (void);

            /// connect Reservoirs to Formations
	    bool connectReservoirs (void);

	    /// check of the m_trappers is not empty
	    bool trappersAreAvailable ();

            /// connect Traps to Reservoirs and Snapshots
	    bool connectTraps (void);

            /// connect Traps to Reservoirs and Snapshots
	    bool connectTrappers (void);

	    bool connectMigrations (void);

	    /// Connect spilling traps
	    bool connectUpAndDownstreamTrappers (void) const;

	    bool connectUpAndDownstreamTrappers (TrapperList * trappers, Reservoir * reservoir, const Snapshot * snapshot) const;
	  
	    const Trapper * findTrapper (TrapperList & trappers, const Reservoir * reservoir,
		  const Snapshot * snapshot, unsigned int id, unsigned int persistentId) const;

	    void numberInputValues (void);

	    void convertInputValuesToBPA (database::Transaction * transaction) const;
	    void saveInputValues (const string & directory, vector<string> & fileNames) const;
	    void savePropertyValues (const string & rootDirectory, const string & subDirectory, PropertyValueList * propertyValues,
		  vector<string> & fileNames) const;

	    void loadInputGridMaps (void) const;
	    void loadPropertyGridMaps (PropertyValueList * propertyValues) const;

            void loadLangmuirIsotherms ();
            void loadLangmuirTOCEntries ();
            void loadPointHistories ();
            void loadIrreducibleWaterSaturationSample ();
            void loadSGDensitySample ();

	    void deleteSnapshots (void);
	    void deleteSurfaces (void);
	    void deleteFormations (void);
	    void deleteLithoTypes (void);
	    void deleteSourceRocks (void);
	    void deleteReservoirs (void);
	    void deleteMobileLayers (void);
	    void deleteTouchstoneMaps (void);
	    void deleteAllochthonousLithologies (void);
	    void deleteAllochthonousLithologyDistributions (void);
	    void deleteAllochthonousLithologyInterpolations (void);
	    void deleteTraps (void);
	    void deleteTrappers (void);
	    void deleteMigrations (void);
	    void deleteLeads (void);
	    void deleteInputValues (void);
	    void deleteProperties (void);
	    void deletePropertyValues (void);
            void deleteFluidTypes ();

            void deleteBiodegradationParameters (void);
            void deleteFracturePressureFunctionParameters (void);
            void deleteDiffusionLeakageParameters (void);

            void deleteCrustFormation ();
            void deleteMantleFormation ();
            void deleteBasementSurfaces ();

            void deleteCrustalThicknessData ();
            void deleteHeatFlowHistory (void);
            void deleteCrustThinningHistory (void);
            void deleteMantleThicknessHistory (void);

            void deleteSurfaceDepthHistory (void);
            void deleteSurfaceTemperatureHistory (void);
	    void deleteRunParameters (void);
	    void deleteProjectData (void);

            void deleteTimeOutputProperties (void);
            void deleteDepthOutputProperties (void);
            void deleteLithologyHeatCapacitySamples (void);
            void deleteLithologyThermalConductivitySamples (void);
            void deleteFluidDensitySamples ();
            void deleteFluidThermalConductivitySamples ();
            void deleteFluidHeatCapacitySamples ();
            void deleteRelatedProjects ();
            void deleteConstrainedOverpressureIntervals ();

            void deleteLangmuirIsotherms ();
            void deleteLangmuirTOCEntries ();
            void deletePointHistories ();
            void deleteIrreducibleWaterSaturationSample ();
            void deleteSGDensitySample ();


            /// Check whether the results of this project were produced by fastcauldron
	    bool wasProducedByFastCauldron (void) const;
            /// Check whether the output maps are in HDF5 format.
	    bool containsHDF5OutputMaps (void) const;
	    
	    /*
	    ///Mapping from a Grid to another with different resolution
	    bool mapGridMapAtoGridMapB( GridMap *mapA, const Grid *GridA, GridMap *mapB, const Grid *GridB);

	    ///Transforms a high res local grid map to a low res local grid map 
	    bool transformHighRes2LowRes( GridMap *mapA, const Grid *GridA,  GridMap *mapB, const Grid *GridB);
	    ///Transforms a low res local grid map to a high res local grid map 
	    bool transformLowRes2HighRes( GridMap *mapA, const Grid *GridA,  GridMap *mapB, const Grid *GridB);
	    //utility for transformLowRes2HighRes
	    bool findLowResElementCoordinates(const int HighResI, const int HighResJ, const int depth,
                                		 const GridMap *lowResMap,  const Grid *lowResGrid,  
                                		 const Grid *HighResGrid, 
                                		 int lowResElementCoordinates[], int lowResElementCoordinatesInHighRes[],
                                		 double nodalValuesInLowResElement[], bool useGhostNodesInLowRes);
	    //utility for findLowResElementCoordinates
	    bool isHighResNodeInLowResElement(const int &HighResI, const int &HighResJ, int lowResElementCoordinatesInHighRes[]);
	    //utility for transformLowRes2HighRes
	    bool isHighResNodeInLowResGrid(const int &HighResI, const int &HighResJ, const GridMap *lowResMap, const Grid *lowResGrid, const Grid *highResGrid);
	    */


            MutableLangmuirAdsorptionIsothermSampleList m_langmuirIsotherms;
            MutableLangmuirAdsorptionTOCEntryList m_langmuirTocAdsorptionEntries;
            MutablePointAdsorptionHistoryList m_adsorptionPointHistoryList;

            IrreducibleWaterSaturationSample* m_irreducibleWaterSample;
            SGDensitySample* m_sgDensitySample;
  private:
            static float GetUndefinedValue (hid_t fileId);

            /// \brief Allocate architecture related clases.
            ///
            /// E.g. Message-handler, global-operations.
            void allocateArchitectureRelatedParameters ();


            DataAccess::Interface::MessageHandler* m_messageHandler;
            DataAccess::Interface::ApplicationGlobalOperations* m_globalOperations;

      };
   }
}
#endif // _INTERFACE_PROJECTHANDLE_H_
