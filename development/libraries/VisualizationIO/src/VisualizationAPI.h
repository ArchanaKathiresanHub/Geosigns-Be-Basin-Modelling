//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _VisualizationAPI_h__
#define _VisualizationAPI_h__

#include "VisualizationAPIFwDecl.h"

#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <memory>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4290)
#endif

/// \namespace CauldronIO
/// \brief The namespace for the visualization IO library related classes
namespace CauldronIO
{
    // From ProjectHandle.C 
    const float DefaultUndefinedValue = 99999;
    const float DefaultUndefinedScalarValue = -9999;
    // maximum length for string parameters
    const int maxStringLength = 256;

    // number of points for 2D interpolation
    const int numberOf2DPoints = 4;
    // number of points for 3D interpolation
    const int numberOf3DPoints = 8;

    // \brief An element in which 3D point lies
    struct Element
    {
        // the reference coordinates of the point in the element
        // reference x coordinate
        double xi = DefaultUndefinedValue;
        // reference y coordinate
        double eta = DefaultUndefinedValue;
        // reference z coordinate
        double zeta = DefaultUndefinedValue;

        // the actual point coordinates
        size_t x = (size_t)DefaultUndefinedValue;
        size_t y = (size_t)DefaultUndefinedValue;
        size_t z = (size_t)DefaultUndefinedValue;

        // the formation in which the point lies
        std::string formationName = "";
    };

    /// \class CauldronIOException
    /// \brief The VisualizationIO exception class

   class CauldronIOException : public std::runtime_error
    {
    public:
        /// \brief Exception from string
        CauldronIOException(const std::string& message) : std::runtime_error(message) {}
        /// \brief Exception from char*
        CauldronIOException(const char* message) : std::runtime_error(message) {}
    };

    /// type definitions
    typedef std::pair<  std::shared_ptr<const Formation>, std::shared_ptr<Volume> >     FormationVolume;
    typedef std::vector<FormationVolume >                                               FormationVolumeList;
    typedef std::vector<std::shared_ptr<SnapShot > >                                    SnapShotList;
    typedef std::vector<std::shared_ptr<Surface > >                                     SurfaceList;
    typedef std::vector<std::shared_ptr<Trapper > >                                     TrapperList;
    typedef std::vector<std::shared_ptr<Trap > >                                        TrapList;
    typedef std::vector<std::shared_ptr<const Property > >                              PropertyList;
    typedef std::vector<std::shared_ptr<Formation> >                                    FormationList;
    typedef std::vector<std::shared_ptr<const Reservoir> >                              ReservoirList;
    typedef std::pair<  std::shared_ptr<const Property>, std::shared_ptr<SurfaceData> > PropertySurfaceData;
    typedef std::pair<  std::shared_ptr<const Property>, std::shared_ptr<VolumeData> >  PropertyVolumeData;
    typedef std::vector < PropertySurfaceData >                                         PropertySurfaceDataList;
    typedef std::vector < PropertyVolumeData >                                          PropertyVolumeDataList;
    typedef std::vector<std::shared_ptr<const Geometry2D> >                             GeometryList;
    typedef std::vector<std::shared_ptr<MigrationEvent> >                               MigrationEventList;
    typedef std::vector<StratigraphyTableEntry>                                         StratigraphyTableEntryList;

    typedef std::vector<std::shared_ptr<DisplayContour> >                               DisplayContourList;
    typedef std::vector<std::shared_ptr<DepthIo> >                                      DepthIoList;
    typedef std::vector<std::shared_ptr<TimeIo1D> >                                     TimeIo1DList;
    typedef std::vector<std::shared_ptr<IsoEntry> >                                     TemperatureIsoList;
    typedef std::vector<std::shared_ptr<IsoEntry> >                                     VrIsoList;
    typedef std::vector<std::shared_ptr<FtSample> >                                     FtSampleList;
    typedef std::vector<std::shared_ptr<FtGrain> >                                      FtGrainList;
    typedef std::vector<std::shared_ptr<FtPredLengthCountsHist> >                       FtPredLengthCountsHistList;
    typedef std::vector<std::shared_ptr<FtPredLengthCountsHistData> >                   FtPredLengthCountsHistDataList;
    typedef std::vector<std::shared_ptr<SmectiteIllite> >                               SmectiteIlliteList;
    typedef std::vector<std::shared_ptr<Biomarkerm> >                                   BiomarkermList;
    typedef std::vector<std::shared_ptr<FtClWeightPercBins>  >                          FtClWeightPercBinsList;

    /// \class Project
    /// \brief Highest level class containing all surface and volume data within a Cauldron project
    class Project
    {
    public:
        /// \brief Create an empty project
        /// \param [in] name Name of the project
        /// \param [in] description Description of the project
        /// \param [in] team Team working on the project
        /// \param [in] version Cauldron simulator version
        /// \param [in] mode Modeling mode
        /// \param [in] xmlVersionMajor 
        /// \param [in] xmlVersionMinor
        explicit Project(const std::string& name, const std::string& description, const std::string& team, const std::string& version,
            ModellingMode mode, int xmlVersionMajor, int xmlVersionMinor);

        /// \brief Destructor
        ~Project();

        /// \brief Adds a snapshot to the current project: exception thrown if it exists
        void addSnapShot(std::shared_ptr<SnapShot>& snapShot) throw (CauldronIOException);
        /// \brief Adds a property to the current project: exception thrown if it exists
        void addProperty(std::shared_ptr<const Property>& property);
        /// \brief Adds a formation to the current project: exception thrown if it exists
        void addFormation(std::shared_ptr<Formation>& formation) throw (CauldronIOException);
        /// \brief Adds a reservoir to the current project: exception thrown if it exists
        void addReservoir(std::shared_ptr<const Reservoir>& newReservoir) throw (CauldronIOException);
        /// \brief Adds a geometry to the current project: no exception thrown if it exists
        /// Note: when a new surface or volume is added to the project without registering its geometry, this will fail...
        void addGeometry(const std::shared_ptr<const Geometry2D>& geometry) throw (CauldronIOException);
        /// \brief Finds the index of this geometry in the list of geometries; throws an exception if index not found
        /// \returns the index of this geometry in the list of geometries
        size_t getGeometryIndex(const std::shared_ptr<const Geometry2D>& newGeometry, bool addWhenNotFound = false) throw (CauldronIOException);
        /// \brief Release all data in project
        void release();

        /// \returns Name of the project
        const std::string& getName() const;
        /// \returns Description of the project
        const std::string& getDescription() const;
        /// \returns Team working on this project (?)
        const std::string& getTeam() const;
        /// \returns Obsolete
        const std::string& getProgramVersion() const;
        /// \returns Modeling mode for this run
        ModellingMode getModelingMode() const;
        /// \returns The list of snapshots
        const SnapShotList& getSnapShots() const;
        /// \returns A list of all unique properties
        const PropertyList& getProperties() const;
        /// \returns A list of all unique geometries
        const GeometryList& getGeometries() const;
        /// \returns true if the project descriptions are equal
        bool operator==(const Project& other) const;

        /// \returns the property (if existing) for the given name
        std::shared_ptr<const Property> findProperty(std::string propertyName) const;
        /// \returns a list of all formations
        const FormationList& getFormations() const;
        /// \returns the formation (if existing) for the given name
        std::shared_ptr<Formation> findFormation(std::string formationName) const;
        /// \returns a list of reservoirs
        const ReservoirList& getReservoirs() const;
        /// \returns the reservoir (if existing) for the given name [we assume there are no reservoirs with identical names and different otherwise]
        std::shared_ptr<const Reservoir> findReservoir(std::string reservoirName) const;
        /// \returns the xml version number
        int getXmlVersionMajor() const;
        /// \returns the xml version number
        int getXmlVersionMinor() const;
        /// \returns the input stratigraphy table
        const StratigraphyTableEntryList& getStratigraphyTable() const;
        /// \brief Adds an entry to the input stratigraphy table
        /// \param [in] entry a new entry for the input stratigraphy table
        void addStratigraphyTableEntry(StratigraphyTableEntry entry);
        /// \brief Retrieve all data in the stratigraphy table
        void retrieveStratigraphyTable();
        /// \returns the migration events table
        const MigrationEventList& getMigrationEventsTable() const;
        /// \brief Adds an entry to the migration events table
        /// \param [in] entry a new entry for the migration events table
        void addMigrationEvent(std::shared_ptr<MigrationEvent> event);
        /// \brief Adds an entry to the trapper table
        /// \param [in] entry a new entry for the trapper table
        void addTrapper(std::shared_ptr<Trapper>& event) throw (CauldronIOException);
        /// \returns the trapper table
        const TrapperList& getTrapperTable() const;
        /// \returns the trapper record for trapId and snapshot age
        std::shared_ptr<const Trapper> findTrapper(int trapId, float snapsotAge) const;
        /// \brief Adds an entry to the trap table
        /// \param [in] entry a new entry for the trap table
        void addTrap(std::shared_ptr<Trap>& event) throw (CauldronIOException);
        /// \returns the trap table
        const TrapList& getTrapTable() const;
        /// \returns the list of  genex history files
        const std::vector<std::string>& getGenexHistoryList();
        /// \brief Adds an entry to the genex history list
        ///        Genex history files are produced by genex simulator and contain 
        ///        the simulation output (genex or shale-gas) at every time-step at history location 
        ///        defined in the project file
        /// \param [in] entry a new entry for the genex history list
        void addGenexHistoryRecord(std::string historyRecord);
        /// \returns the mass balance file name produced by fastmig simulator
        const std::string& getMassBalance();
        ///  \brief Set the mass balance file name
        void setMassBalance(const std::string name);
        /// \brief Adds an entry to the burial history list
        /// \param [in] entry a new entry for the burial history list
        void addBurialHistoryRecord(std::string historyRecord);
        /// \returns the list of burial history files - the BHF files produced by cauldron2bhf application.
        const std::vector<std::string>& getBurialHistoryList();

       /// \returns the DisplayContour table
       const DisplayContourList& getDisplayContourTable() const;
       /// \brief Adds an entry to the list of contour settings for a property to display
       /// \param [in] entry a new entry
       void addDisplayContour(std::shared_ptr<DisplayContour> entry);
       /// \returns the DepthIo table
       const DepthIoList& getDepthIoTable() const;
       /// \brief Adds an entry to the list of property values for a depth/time combination
       /// \param [in] entry a new entry
       void addDepthIo(std::shared_ptr<DepthIo> entry);
       const TimeIo1DList& get1DTimeIoTable() const;
       /// \brief Adds an entry to the list of 1D property values
       /// \param [in] entry a new entry
       void add1DTimeIo(std::shared_ptr<TimeIo1D> entry);    
       /// \returns the TemperatureIso table
       const TemperatureIsoList& getTemperatureIsoTable() const;
       /// \brief Adds an entry to the list of temperature Iso depth that will be displayed in the burial graph
       /// \param [in] entry a new entry
       void addTemperatureIsoEntry(std::shared_ptr<IsoEntry> entry);
       /// \returns the VrIso table
       const VrIsoList& getVrIsoTable() const;
       /// \brief Adds an entry to the list of Vr Iso depth that will be displayed in the burial graph
       /// \param [in] entry a new entry
       void addVrIsoEntry(std::shared_ptr<IsoEntry> entry);
       /// \returns the FtSampleIo table
       const FtSampleList& getFtSampleTable() const;
       /// \brief Adds an entry to the list of Fission track sample data
       /// \param [in] entry a new entry
       void addFtSample(std::shared_ptr<FtSample> entry);
       /// \returns the FtGrainIo table
       const FtGrainList& getFtGrainTable() const;
       /// \brief Adds an entry to the list of spontaneous and induced track count per grain
       /// \param [in] entry a new entry
       void addFtGrain(std::shared_ptr<FtGrain> entry);
       /// \returns the FtPredLengthCountsHistIo table
       const FtPredLengthCountsHistList& getFtPredLengthCountsHistTable() const;
       /// \brief Adds an entry to the list of layout for the histograms containing the predicted fission track length counts
       /// \param [in] entry a new entry
       void addFtPredLengthCountsHist(std::shared_ptr<FtPredLengthCountsHist> entry);
       /// \returns the FtPredLengthCountsHistDataIo table
       const FtPredLengthCountsHistDataList& getFtPredLengthCountsHistDataTable() const;
       /// \brief Adds an entry to the list of histogram data (predicted counts per track length bin) of histograms
       /// \param [in] entry a new entry
       void addFtPredLengthCountsHistData(std::shared_ptr<FtPredLengthCountsHistData> entry);
       /// \returns the SmectiteIlliteIo table
       const SmectiteIlliteList& getSmectiteIlliteTable() const;
       /// \brief Adds an entry to the list of measured smectite illite data for well
       /// \param [in] entry a new entry
       void addSmectiteIllite(std::shared_ptr<SmectiteIllite> entry);
       /// \returns the BiomarkermIo table
       const BiomarkermList& getBiomarkermTable() const;
       /// \brief Adds an entry to the list of Biomarker measurements for a well
       /// \param [in] entry a new entry
       void addBiomarkerm(std::shared_ptr<Biomarkerm> entry);
       /// \returns the FtClWeightPercBins table
       const FtClWeightPercBinsList& getFtClWeightPercBinsTable() const;
       /// \brief Adds the layout of Cl Weight Bins used in the fission track prediction algorithms
       /// \param [in] entry a new entry
       void addFtClWeightPercBins(std::shared_ptr<FtClWeightPercBins> entry);
 
       /// \brief Get a property value at the x, y, z coordinates. 
       ///        If the z coordinate is not defined the value is taken at surface or formation map.
       /// \param [in] snapshotTime Snapshot Age
       /// \param [in] propertyName Name of the property
       /// \param [in] xCoord X coordinate 
       /// \param [in] yCoord Y coordinate 
       /// \param [in] zCoord Z coordinate
       /// \param [in] reservoirName The name of reservoir to find the property
       /// \param [in] surfaceName The name of surface to find a property map
       /// \param [in] formationName The name of formation to find a property map
       /// \returns The property value.  
       float getPropertyAtLocation(double snapshotTime, const std::string & propertyName,
                                   double xCoord, double yCoord, double zCoord, 
                                   const std::string& reservoirName, const std::string& surfaceName, 
                                   const std::string& formationName ) const throw (CauldronIOException);
        
   private:
        SnapShotList m_snapShotList;
        std::string m_name, m_description, m_team, m_version;
        ModellingMode m_mode;
        PropertyList m_propertyList;
        FormationList m_formationList;
        ReservoirList m_reservoirList;
        GeometryList m_geometries;
        MigrationEventList m_migrationEventList;
        TrapperList m_trapperList;
        TrapList m_trapList;
        int m_xmlVersionMajor, m_xmlVersionMinor;
        StratigraphyTableEntryList m_stratTable;
        std::vector<std::string> m_genexHistoryList;
        std::vector<std::string> m_burialHistoryList;
        std::string m_massBalance;

        // 1D tables
       DisplayContourList m_displayContour;
       DepthIoList m_depthIo;
       TimeIo1DList m_timeIo1D;
       TemperatureIsoList m_temperatureIso;
       VrIsoList m_vrIso;
       FtSampleList m_ftSample;
       FtGrainList m_ftGrain;
       FtPredLengthCountsHistList m_ftPredHist;
       FtPredLengthCountsHistDataList m_ftPredHistData;
       SmectiteIlliteList m_smectiteIllite;
       BiomarkermList m_biomarkerm;
       FtClWeightPercBinsList m_ftClWeightPercBins;
       
    };

    /// \class StratigraphyTableEntry
    /// \brief This class holds the input data surface or formation entry to build a stratigraphy table
    class StratigraphyTableEntry
    {
    public:
        /// \brief Constructor
        StratigraphyTableEntry() {};
        /// \returns The surface, can be null
        const std::shared_ptr<Surface>& getSurface() const;
        /// \returns The formation, can be null
        const std::shared_ptr<Formation>& getFormation() const;
        /// \brief set a surface, this will nullify the formation
        void setSurface(std::shared_ptr<Surface>& surface);
        /// \brief set a formation, this will nullify the surface
        void setFormation(std::shared_ptr<Formation>& formation);

    private:
        std::shared_ptr<Surface> m_surface;
        std::shared_ptr<Formation> m_formation;
    };

    /// \class SnapShot 
    /// \brief container class holding all surfaces and volumes for a snapshot
    class SnapShot
    {
    public:
        /// \brief create an empty snapshot
        /// \param [in] age Snapshot age in MY
        /// \param [in] kind Snapshot kind
        /// \param [in] isMinorShapshot If true, this is a minor snapshot
        explicit SnapShot(double age, SnapShotKind kind, bool isMinorShapshot) throw (CauldronIOException);
        ~SnapShot();

        /// \brief Retrieve all data in snapshot: 
        /// This will iterate over all contained data (surfaces, volumes, etc.)
        void retrieve();
        /// \brief Release all data in the snapshot
        void release();

        /// \brief Add a surface to the snapshot; ownership is transfered
        void addSurface(std::shared_ptr<Surface>& surface) throw (CauldronIOException);
        /// \brief Add a volume to the snapshot; ownership is transfered
        void setVolume(std::shared_ptr<Volume>& volume);
        /// \brief Add a discontinuous volume to the snapshot; ownership is transfered
        void addFormationVolume(FormationVolume& formVolume) throw (CauldronIOException);
        /// \brief Add a trapper to the snapshot; ownership is transfered
        void addTrapper(std::shared_ptr<Trapper>& trapper) throw (CauldronIOException);

        /// \returns Age of snapshot
        double getAge() const;
        /// \returns kind of snapshot
        SnapShotKind getKind() const;
        /// \returns True if this is a minor snapshot
        bool isMinorShapshot() const;

        /// \returns the list of surfaces
        const SurfaceList& getSurfaceList() const;
        /// \returns the list of volumes
        const std::shared_ptr<Volume>& getVolume() const;
        /// \returns the list of discontinuous volumes
        const FormationVolumeList& getFormationVolumeList() const;
        /// \returns the list of trappers
        const TrapperList& getTrapperList() const;
        /// \returns a list of all data not yet retrieved for this snapshot
        std::vector < VisualizationIOData* > getAllRetrievableData() const;

        /// \brief Get a property value at the x, y, z coordinates
        ///        If z coordinate is not defined and the property is discontinous, find a top or bottom formation for a surface.
        ///        
        /// \param [in] formations The list of formations in the project to find a formation for the surface
        /// \param [in] property The property in the project
        /// \param [in] xCoord X coordinate 
        /// \param [in] yCoord Y coordinate 
        /// \param [in] zCoord Z coordinate
        /// \param [in] reservoirName The name of reservoir to find the property
        /// \param [in] surfaceName The name of surface to find a property map
        /// \param [in] formationName The name of formation to find a property map
        /// \returns The property value
        float getPropertyAtLocation(const FormationList& formations,
                                   double xCoord, double yCoord, double zCoord, std::shared_ptr<const Property>& property, 
                                   const std::string& reservoirName, const std::string& surfaceName, 
                                   const std::string& formationName ) const throw (CauldronIOException);
    private:
        SurfaceList m_surfaceList;
        std::shared_ptr<Volume> m_volume;
        FormationVolumeList m_formationVolumeList;
        TrapperList m_trapperList;
        SnapShotKind m_kind;
        bool m_isMinor;
        double m_age;

       /// \brief Get a continuous property value at the element location
       float getValueAtLocation(std::shared_ptr<const Property>& property, std::shared_ptr<CauldronIO::Element> &element) const;
       /// \brief Get a surface or formation map property value at the element location
       float getValueAtLocation(std::shared_ptr<const Property>& property, const std::string& surfaceName, const std::string& formationName, std::shared_ptr<CauldronIO::Element> &element) const;
       /// \brief Find an element in Depth property for x, y, z coordinates in the domain
       std::shared_ptr<CauldronIO::Element> getDepthElementAtLocation(double xCoord, double yCoord, double zCoord, const bool highRes = false) const throw (CauldronIOException);
       /// \brief Find the formation for a surface
       bool findFormationForSurface(const FormationList& formations,
                                    std::shared_ptr<const Property>& property,  const std::string& surfaceName,
                                    const std::string& formation, std::shared_ptr<CauldronIO::Element> &element) const;
    };

    /// \class Reservoir
    /// \brief container class holding some information about a reservoir
    class Reservoir
    {
    public:
        /// \brief Reservoir constructor
        /// \param [in] reservoirName name of reservoir
        /// \param [in] formation the formation for this reservoir
        explicit Reservoir(const std::string& reservoirName, const std::shared_ptr<const Formation>& formation);

        /// \returns the name of this reservoir
        const std::string& getName() const;
        /// \returns the formation of this reservoir
        const std::shared_ptr<const Formation>& getFormation() const;
        /// \returns true if two reservoirs are equal
        bool operator==(const Reservoir& other) const;

    private:
        std::string m_reservoirName;
        std::shared_ptr<const Formation> m_formation;
    };


    /// \class Property 
    /// \brief Little information class holding information about a Cauldron property
    class Property
    {
    public:
        /// \brief Property constructor
        /// \param [in] name Name of the property
        /// \param [in] username User name of the property
        /// \param [in] cauldronName Cauldron name of the property
        /// \param [in] unit the Unit of the property
        /// \param [in] type the property type
        /// \param [in] attrib the property attribute
        explicit Property(const std::string& name, const std::string& username, const std::string& cauldronName, const std::string& unit,
            PropertyType type, PropertyAttribute attrib) throw (CauldronIOException);
        /// \returns the name of this property
        const std::string& getName() const;
        /// \returns the user name of this property
        const std::string& getUserName() const;
        /// \returns the cauldron name of this property
        const std::string& getCauldronName() const;
        /// \return the unit of this property
        const std::string& getUnit() const;
        /// \returns the PropertyType
        PropertyType getType() const;
        /// \returns the PropertyAttribute
        PropertyAttribute getAttribute() const;
        /// \returns true if two properties are equal
        bool operator==(const Property& other) const;
        /// \return true if this property is high-resolution
        bool isHighRes() const;

    private:
        std::string m_name, m_username, m_cauldronName, m_unit;
        PropertyType m_type;
        PropertyAttribute m_attrib;
    };

    /// \class Formation
    /// \brief Container class to hold information about a Cauldron Formation
    class Formation
    {
    public:
        /// \brief Creates a new formation covering a depth range and name
        /// \param [in] kStart start depth index into the bigger volume (if relevant)
        /// \param [in] kEnd end depth index (inclusive) into bigger volume
        /// \param [in] name name of the formation
        explicit Formation(int kStart, int kEnd, const std::string& name) throw (CauldronIOException);
        /// \brief Destructor
        ~Formation();

        /// \returns true if depth (or k) range is defined
        bool isDepthRangeDefined() const;
        /// \brief Update the k-range of an existing formation
        /// \param [in] kStart start depth index into the bigger volume (if relevant)
        /// \param [in] kEnd end depth index (inclusive) into bigger volume
        void updateK_range(int kStart, int kEnd);
        /// \returns the depth range; only sensible output if isDepthRangeDefined returns true
        void getK_Range(int & k_range_start, int& k_range_end) const;
        /// \returns the formation name
        const std::string& getName() const;
        /// \returns true if two formations are equal
        bool operator==(const Formation& other) const;
        /// \param [in] isMobileLayer true if this formation is a mobile layer
        void setIsMobileLayer(bool isMobileLayer);
        /// \returns true if this formation is a mobile layer
        bool isMobileLayer() const;
        /// \param [in] isSourceRock true if this formation is a source rock
        void setIsSourceRock(bool isSourceRock);
        /// \returns true if this formation is a source rock
        bool isSourceRock() const;
        /// \brief Assigns the source rock 1 name
        void setSourceRock1Name(const std::string& name);
        /// \returns Returns the source rock 1 name
        const std::string& getSourceRock1Name() const;
        /// \brief Assigns the source rock 2 name
        void setSourceRock2Name(const std::string& name);
        /// \returns Returns the source rock 2 name
        const std::string& getSourceRock2Name() const;
        /// \brief Enables source rock mixing
        void setEnableSourceRockMixing(bool enable);
        /// \returns Returns true if source rock mixing has been enabled
        bool getEnableSourceRockMixing() const;
        /// \returns Returns true if this formation has an allochthonous lithology
        bool hasAllochthonousLithology() const;
        /// \brief Enables allochthonous lithology for this formation
        void setAllochthonousLithology(bool value);
        /// \returns Returns the allochthonous lithology name
        const std::string& getAllochthonousLithologyName() const;
        /// \brief Assigns the allochthonous lithology name
        void setAllochthonousLithologyName(const std::string& value);
        /// \returns Returns true if this formation is an ignious intrusion
        bool isIgneousIntrusion() const;
        /// \brief Enables ignious intrusion for this formation
        void setIgneousIntrusion(bool value);
        /// \returns Returns the ignious intrusion age
        double getIgneousIntrusionAge() const;
        /// \brief Assigns the ignious intrusion age
        void setIgneousIntrusionAge(double age);
        /// \returns Returns the deposition sequence number 
        int getDepoSequence() const;
        /// \brief Assigns the deposition sequence number 
        void setDepoSequence(int number);
        /// \returns Returns the fluid type string
        const std::string& getFluidType() const;
        /// \brief Assigns the fluid type string
        void setFluidType(const std::string& type);
        /// \returns Returns true if this formation has a constrained overpressure
        bool hasConstrainedOverpressure() const;
        /// \brief Enables constrained overpressure for this formation
        void setConstrainedOverpressure(bool value);
        /// \returns Returns true if this formation is subject to chemical compaction
        bool hasChemicalCompaction() const;
        /// \brief Enable chemical compaction for this formation
        void setChemicalCompaction(bool value);
        /// \returns Returns the element refinement in z-direction
        int getElementRefinement() const;
        /// \brief Assigns the element refinement in z-direction
        void setElementRefinement(int value);
        /// \returns Returns the mixingmodel string
        const std::string& getMixingModel() const;
        /// \brief Assigns the mixingmodel
        void setMixingModel(const std::string& model);
        /// \brief assign the lithotype1 name
        void setLithoType1Name(const std::string& name);
        /// \returns the lithotype1 name
        const std::string& getLithoType1Name() const;
        /// \brief assign the lithotype2 name
        void setLithoType2Name(const std::string& name);
        /// \returns the lithotype2 name
        const std::string& getLithoType2Name() const;
        /// \brief assign the lithotype3 name
        void setLithoType3Name(const std::string& name);
        /// \returns the lithotype3 name
        const std::string& getLithoType3Name() const;

        /// \brief get the list of all property-surfaceData pairs contained in this formation
        const PropertySurfaceDataList& getPropertySurfaceDataList() const;

        /// \returns true if this formation has a thickness map defined
        bool hasThicknessMap() const;
        /// \brief Assigns a thickness map to this formation
        void setThicknessMap(PropertySurfaceData& thicknessMap);
        /// \returns Returns the thickness map; can be empty
        const PropertySurfaceData& getThicknessMap() const throw (CauldronIOException);

        /// \returns true if this formation has a mixingHI map defined
        bool hasSourceRockMixingHIMap() const;
        /// \brief Assigns a mixingHI map to this formation
        void setSourceRockMixingHIMap(PropertySurfaceData& map);
        /// \returns Returns the mixingHI map; can be empty
        const PropertySurfaceData& getSourceRockMixingHIMap() const throw (CauldronIOException);

        /// \brief Assign a lithotype percentagemap
        void setLithoType1PercentageMap(PropertySurfaceData& map);
        /// \brief Assign a lithotype percentagemap
        void setLithoType2PercentageMap(PropertySurfaceData& map);
        /// \brief Assign a lithotype percentagemap
        void setLithoType3PercentageMap(PropertySurfaceData& map);
        /// \brief Retrieve a lithotype percentagemap
        const PropertySurfaceData& getLithoType1PercentageMap() const throw(CauldronIOException);
        /// \brief Retrieve a lithotype percentagemap
        const PropertySurfaceData& getLithoType2PercentageMap() const throw (CauldronIOException);
        /// \brief Retrieve a lithotype percentagemap
        const PropertySurfaceData& getLithoType3PercentageMap() const throw (CauldronIOException);
        /// \returns Returns true if this formation has a lithotype1 percentagemap
        bool hasLithoType1PercentageMap() const;
        /// \returns Returns true if this formation has a lithotype2 percentagemap
        bool hasLithoType2PercentageMap() const;
        /// \returns Returns true if this formation has a lithotype3 percentagemap
        bool hasLithoType3PercentageMap() const;

        void setTopSurface(std::shared_ptr<CauldronIO::Surface>& surface);
        const std::shared_ptr<CauldronIO::Surface>& getTopSurface();

        void setBottomSurface(std::shared_ptr<CauldronIO::Surface>& surface);
        const std::shared_ptr<CauldronIO::Surface>& getBottomSurface();

        /// \brief Release the formation's data
        void release();
        /// \brief Retrieve all data related to this formation
        void retrieve();
        /// \brief Returns true if the data has been retrieved
        bool isRetrieved() const;

    private:
        std::string m_name, m_lithoType1name, m_lithoType2name, m_lithoType3name, m_fluidTypeName;
        std::string m_mixingmodelname, m_sourceRock1name, m_sourceRock2name, m_allochthonousLithologyName;
        int m_kstart, m_kend; // these can be negative if not defined, for example if output data is not available
        bool m_isSourceRock, m_isMobileLayer, m_chemicalcompaction, m_constrainedOverpressure, m_igniousintrusion, m_enableSourceRockMixing, m_hasAllochthonousLithology;
        double m_igniousintrusionAge;
        int m_depoSequence, m_elementRefinement;
        int m_thicknessMap_index, m_mixingHI_index, m_lithPerc1_index, m_lithPerc2_index, m_lithPerc3_index;
        PropertySurfaceDataList m_propSurfaceList;
        std::shared_ptr<CauldronIO::Surface> m_topSurface, m_bottomSurface;
    };

    /// \class Surface
    /// \brief Container class to hold meta information about a surface. The actual data is contained in \class Map
    class Surface
    {
    public:
        /// \brief Construct a new surface
        /// \param [in] name Name of the surface
        /// \param [in] kind kind of surface
        explicit Surface(const std::string& name, SubsurfaceKind kind);
        /// \brief Destructor
        ~Surface();

        /// \brief get the list of property-surfaceData pairs contained in this surface
        const PropertySurfaceDataList& getPropertySurfaceDataList() const;
        /// \brief Method to replace a PropertySurfaceData object by another one in the list
        /// \param[in] index Index in to the PropertySurfaceDataList
        /// \param[in] index data the new PropertySurfaceData for that element
        void replaceAt(size_t index, PropertySurfaceData& data) throw (CauldronIOException);
        /// \brief Add a property-surfaceData pair to the list
        void addPropertySurfaceData(PropertySurfaceData& data) throw (CauldronIOException);
        /// \returns true if this surface has a depth surface
        bool hasDepthSurface() const;
        /// \returns the depth surface data; can be null
        std::shared_ptr<SurfaceData> getDepthSurface() const;
        /// \brief Get the name of this surface
        const std::string& getName() const;
        /// \returns the subsurface kind
        SubsurfaceKind getSubSurfaceKind() const;
        /// \brief Associate a formation with this map
        /// \param [in] formation the formation to be associated with this map. Optional.
        /// \param [in] isTop
        void setFormation(std::shared_ptr<Formation>& formation, bool isTop);
        /// \returns the associated top formation for this map. Can be null
        const std::shared_ptr<Formation>& getTopFormation() const;
        /// \returns the associated bottom formation for this map. Can be null
        const std::shared_ptr<Formation>& getBottomFormation() const;
        /// \brief Retrieve actual data into memory
        void retrieve();
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        void release();
        /// \returns true if data is available
        bool isRetrieved() const;
        /// \returns true if two Surfaces are equal: note: the PropertySurfaceDataList is not compared 
        bool operator==(const Surface& other) const;
        /// \returns true if Age is defined
        bool isAgeDefined() const;
        /// \brief Assigns an age to this surface
        /// \param [in] age deposition age
        void setAge(float age);
        /// \returns Age of this surface. Only makes sense if isAgeDefined is true
        float getAge() const;

    private:
        SubsurfaceKind m_subSurfaceKind;
        std::shared_ptr<Formation> m_Topformation;
        std::shared_ptr<Formation> m_Bottomformation;
        std::string m_name;
        std::string m_reservoirName;
        PropertySurfaceDataList m_propSurfaceList;
        float m_age;
    };

    class Geometry2D
    {
    public:
        /// \brief Constructor
        /// \param [in] numI number of elements in i-direction
        /// \param [in] numJ number of elements in j-direction
        /// \param [in] deltaI spacing in i-direction
        /// \param [in] deltaJ spacing in j-direction
        /// \param [in] minI map origin in i-direction
        /// \param [in] minJ map origin in j-direction
        explicit Geometry2D(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ, bool cellCentered = false);
        /// \returns the local horizontal resolution
        size_t getNumI() const;
        /// \returns  the local vertical resolution
        size_t getNumJ() const;
        /// \returns  the distance between two vertical grid lines
        double getDeltaI() const;
        /// \returns  the distance between two horizontal grid lines
        double getDeltaJ() const;
        /// \returns the leftmost grid coordinate value
        double getMinI() const;
        /// \returns  the bottommost grid coordinate value
        double getMinJ() const;
        /// \returns  the rightmost grid coordinate value
        double getMaxI() const;
        /// \returns  the topmost grid coordinate value
        double getMaxJ() const;
        /// \return the total number of elements
        virtual size_t getSize() const;
        /// \returns true if two geometries are equal
        bool operator==(const Geometry2D& other) const;
        /// \returns true if properties are cell-centered
        bool isCellCentered() const;
        /// \brief mark this surface cell-centered
        /// \param [in] cellCentered true if the surface should be cell-centered
        void setCellCentered(bool cellCentered);

    protected:
        double m_deltaI, m_deltaJ, m_minI, m_minJ, m_maxI, m_maxJ;
        size_t m_numI, m_numJ;
        bool m_isCellCentered;
    };

    class Geometry3D : public Geometry2D
    {
    public:
        /// \brief assign a geometry: the k-offset is the index of the first k-element (can be zero)
        /// \param [in] numI number of elements in i-dimension
        /// \param [in] numJ number of elements in j-dimension
        /// \param [in] numK number of elements in k-dimension
        /// \param [in] offsetK the offset in k-dimension; if non-zero, subsequent index should assume this offset
        /// \param [in] deltaI spacing in i-dimension
        /// \param [in] deltaJ spacing in j-dimension
        /// \param [in] minI the volume origin in i-dimension
        /// \param [in] minJ the volume origin in j-dimension
        explicit Geometry3D(size_t numI, size_t numJ, size_t numK, size_t offsetK,
            double deltaI, double deltaJ, double minI, double minJ, bool cellCentered = false);

        /// \returns the number of k elements in this volume
        size_t getNumK() const;
        /// \returns the index of the first k element
        size_t getFirstK() const;
        /// \returns the index of the last k element (inclusive)
        size_t getLastK() const;
        /// \return the total number of elements
        virtual size_t getSize() const;
        /// \returns true if two geometries are equal
        bool operator==(const Geometry3D& other) const;
        /// \brief updates the k-range [use with care!!!]
        /// \param [in] firstK index in k-dimension
        /// \param [in] numK number of elements in k-dimension
        void updateK_range(size_t firstK, size_t numK);
        /// \brief updates the IJ-range [use with care!!!]
        /// \param [in] numI number of elements in i-dimension
        /// \param [in] numJ number of elements in j-dimension
        void updateIJ_range(size_t numI, size_t numJ);

    private:
        size_t m_numK, m_firstK;
    };

    /// \class HDFinfo
    /// \brief little struct to hold HDF related information
    struct HDFinfo
    {
    public:
        void setData(float* data) { m_data = data; }
        float* getData() { return m_data; }

        std::string dataSetName;
        std::string filepathName;
        size_t numI, numJ, numK;
        VisualizationIOData* parent;
        int indexMain; // index into allReadData
        int indexSub;  // index into propertyvalue array

    private:
        float* m_data;
    };

    /// \class VisualizationIOData
    /// \brief interface class for surface and volume data
    class VisualizationIOData
    {
    public:
        /// \brief Retrieve the data: throws on failure. Note: The VisualizationIO_projectHandle implementation requires that HDF data is read before retrieve is called.
        /// \brief See CauldronIO::VisualizationUtils::retrieveAllData.The VisualizationIO_native implementation can handle this itself.
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release() = 0;
        /// \brief Prefetch any data: load from disk, do not decompress yet (for this call retrieve)
        virtual void prefetch() = 0;
        /// \returns true if data is available
        virtual bool isRetrieved() const = 0;

        /// HDF support calls (relevant for VisualizationIO_projectHandle only)
        /////////////////////////////////////

        /// \returns a list of HDFinfo holding the data; can be null; ownership with this class
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() = 0;
        /// \brief Method to signal new HDF data has been loaded
        /// \returns true if all data needed is now ready (prefetch done)
        virtual bool signalNewHDFdata() = 0;
    };

    /// \class SurfaceData 
    /// \brief container class to hold data for a Surface
    class SurfaceData : public VisualizationIOData
    {
    public:
        /// \brief Create a surface-data object
        explicit SurfaceData(const std::shared_ptr<const Geometry2D>& geometry, float minValue = DefaultUndefinedValue, float maxValue = DefaultUndefinedValue);
        virtual ~SurfaceData();

        /// VisualizationIOData implementation
        //////////////////////////////////////////////////////////////////////////
        /// \brief Retrieve the data
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns true if data is available
        virtual bool isRetrieved() const;
        /// \brief Prefetch any data: load from disk, do not decompress yet (for this call retrieve)
        virtual void prefetch() = 0;
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() = 0;

        /// \returns the geometry
        const std::shared_ptr<const Geometry2D>& getGeometry() const;
        /// \brief Assign data to the map : geometry must have been assigned
        /// \param [in] data pointer to the xy data, ordered row-wise
        /// \note data is copied so data ownership is not transferred; data should be deleted by client if obsolete
        void setData_IJ(float* data) throw (CauldronIOException);
        /// \returns  true if data is represented per row
        bool canGetRow() const;
        /// \returns true if data is represented per column
        bool canGetColumn() const;
        /// \brief Gets the value at the specified grid coordinate
        /// \param [in] i row index
        /// \param [in] j column index
        /// \returns value at the given coordinate
        float getValue(size_t i, size_t j) const throw (CauldronIOException);
        /// \brief Gets an entire row; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] j column index
        /// \returns the row values for the given column
        const float* getRowValues(size_t j) throw (CauldronIOException);
        /// \brief Gets an entire column; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] i row index
        /// \returns the column values for the given row
        const float* getColumnValues(size_t i) throw (CauldronIOException);
        /// \returns pointer to entire data 
        const float* getSurfaceValues() throw (CauldronIOException);
        /// \brief Convenience function to get an index into the flat surface data 
        /// \param [in] i row index
        /// \param [in] j column index
        /// \returns the index into the 2D array
        size_t getMapIndex(size_t i, size_t j) const;
        /// \returns true if surface is constant
        bool isConstant() const;
        /// \brief Set a constant value for this surface
        /// \param [in] constantValue
        void setConstantValue(float constantValue);
        /// \returns the constant value
        float getConstantValue() const throw (CauldronIOException);
        /// \param [in] i row index
        /// \param [in] j column index
        /// \returns true if grid coordinate is undefined
        bool isUndefined(size_t i, size_t j) const throw (CauldronIOException);
        /// \returns the undefined value
        float getUndefinedValue() const;
        /// \param [in] formation the formation to be associated with this map. Optional.
        void setFormation(const std::shared_ptr<const Formation>& formation);
        /// \returns the associated top formation for this map. Can be null
        const std::shared_ptr<const Formation>& getFormation() const;
        /// \brief Set the reservoir of this surface
        void setReservoir(std::shared_ptr<const Reservoir> reservoir);
        /// \brief Get the name of the reservoir associated with the surface. Optional.
        const std::shared_ptr<const Reservoir>& getReservoir() const;
        /// \returns the minimum value of this surface
        float getMinValue();
        /// \returns the maximum value of this surface
        float getMaxValue();
        /// \returns Returns the deposition sequence number 
        int getDepoSequence() const;
        /// \brief Assigns the deposition sequence number 
        void setDepoSequence(int number);
        /// \brief Interpolate a property in a 2d map
        float interpolate( std::shared_ptr<CauldronIO::Element> & element) const;
        bool findPlaneLocation(double xCoord, double yCoord, std::shared_ptr<CauldronIO::Element> &element);
   private:

        void updateMinMax() throw (CauldronIOException);
        void setData(float* data, bool setValue = false, float value = 0) throw (CauldronIOException);

        float* m_internalData;
        float m_constantValue;
        bool m_isConstant;
        bool m_updateMinMax;
        float m_minValue, m_maxValue;
        int m_depoSequence;
        std::shared_ptr<const Formation> m_formation;
        std::shared_ptr<const Reservoir> m_reservoir;
        std::shared_ptr<const Geometry2D> m_geometry;

    protected:
        bool m_retrieved;
        size_t m_numI, m_numJ;
        double m_deltaI, m_deltaJ, m_minI, m_minJ, m_maxI, m_maxJ;
    };

    /// \class Volume
    /// \brief This is a continuous volume: it is continuous across formation boundaries. It can still be (legacy) node centered as well as cell centered.
    /// A DiscontinuousVolume is not continuous across formation boundaries and uses multiple instances of this class to contain its data.
    /// Formation data of continuous properties (e.g. temperature) can be merged into a single Volume class. Also, upon converting to cell centered data,
    /// all DiscontinuousVolume's can be merged into Volume's
    class Volume
    {
    public:
        /// \brief Constructor
        /// \param [in] kind the SubsurfaceKind of this volume
        explicit Volume(SubsurfaceKind kind);
        /// \brief Destructor
        ~Volume();

        /// \returns the SubsurfaceKind
        SubsurfaceKind getSubSurfaceKind() const;
        /// \brief get the list of property-volumeData pairs contained in this surface
        PropertyVolumeDataList& getPropertyVolumeDataList();
        //// \brief Removes a propertyvolume data from the list and disposes it        
        void removeVolumeData(PropertyVolumeData& data) throw (CauldronIOException);
        /// \brief Add a property-surfaceData pair to the list
        void addPropertyVolumeData(PropertyVolumeData& data) throw (CauldronIOException);
        /// \brief Method to replace a PropertyVolumeData object by another one in the list
        /// \param[in] index Index in to the PropertyVolumeDataList
        /// \param[in] index data the new PropertyVolumeData for that element
        void replaceAt(size_t index, PropertyVolumeData& data) throw (CauldronIOException);
        /// \returns true if this surface has a depth surface
        bool hasDepthVolume() const;
        /// \returns the depth volume data; can be null
        std::shared_ptr<VolumeData> getDepthVolume() const;

        /// \brief Retrieve the data
        void retrieve();
        /// \returns true if data is available
        bool isRetrieved() const;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        void release();

    private:
        SubsurfaceKind m_subSurfaceKind;
        PropertyVolumeDataList m_propVolumeList;
    };

    class VolumeData : public VisualizationIOData
    {
    public:
        /// \brief Create a surface-data object
        explicit VolumeData(const std::shared_ptr<Geometry3D>& geometry, float minValue = DefaultUndefinedValue, float maxValue = DefaultUndefinedValue);
        virtual ~VolumeData();

        /// \returns the geometry
        const std::shared_ptr<Geometry3D>& getGeometry() const;
        /// \brief Assign data to the volume as a 1D array: K fastest, then I, then J
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void setData_KIJ(float* data, bool setValue = false, float value = 0) throw (CauldronIOException);
        /// \brief Assign data to the volume as a 1D array: I fastest, then J, then K
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void setData_IJK(float* data, bool setValue = false, float value = 0) throw (CauldronIOException);
        /// \returns true if IJK data is present (false if not present or constant data)
        bool hasDataIJK() const;
        /// \returns true if KIJ data is present (false if not present or constant data)
        bool hasDataKIJ() const;
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns if value is undefined
        bool isUndefined(size_t i, size_t j, size_t k) const throw (CauldronIOException);
        /// \returns the undefined value
        float getUndefinedValue() const;
        /// \returns true if surface is constant
        bool isConstant() const;
        /// \brief Set a constant value for this volume
        /// \param [in] constantValue the value to assign to all data of this volume
        void setConstantValue(float constantValue);
        /// \returns the constant value
        float getConstantValue() const throw (CauldronIOException);
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns the value at the specified grid coordinate
        float getValue(size_t i, size_t j, size_t k) const throw (CauldronIOException);
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire row of data; can be null if this volume is not row-ordered (or throw an exception)
        const float* getRowValues(size_t j, size_t k) throw (CauldronIOException);
        /// \param [in] i index in i-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire column of data; can be null if this volume is not row-ordered (or throw an exception)
        const float* getColumnValues(size_t i, size_t k) throw (CauldronIOException);
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \throws CauldronIOException
        /// \returns an entire needle of data; can be null if this volume is not needle-ordered (or throw an exception)
        const float* getNeedleValues(size_t i, size_t j) throw (CauldronIOException);
        /// \brief Find the actual coordinates and the reference coordinates of the element
        /// \param [in] xCoord x coordinate of the well location
        /// \param [in] yCoord y coordinate of the well location
        /// \returns the element with the acual and reference coordinates
        bool findPlaneLocation(double xCoord, double yCoord, std::shared_ptr<CauldronIO::Element> &element);
        /// \brief Interpolate a property in a 3d map
        float interpolate(std::shared_ptr<CauldronIO::Element>& element);
        float interpolate(std::shared_ptr<CauldronIO::Element>& element, size_t k);
        /// \param [in] k index in k-dimension
        /// \returns pointer to entire data for the surface at depth k; can be null if data is not stored per ij surface
        const float* getSurface_IJ(size_t k) throw (CauldronIOException);
        /// \returns pointer to entire data: can be NULL
        const float* getVolumeValues_IJK() throw (CauldronIOException);
        /// \returns pointer to entire data: can be NULL 
        const float* getVolumeValues_KIJ() throw (CauldronIOException);

        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        size_t computeIndex_IJK(size_t i, size_t j, size_t k) const;
        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        size_t computeIndex_KIJ(size_t i, size_t j, size_t k) const;

        /// \returns the minimum value of this volume
        float getMinValue();
        /// \returns the maximum value of this volume
        float getMaxValue();

        /// VisualizationIOData implementation
        //////////////////////////////////////////////////////////////////////////
        /// \brief Retrieve the data, returns true on success
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns true if data is available
        virtual bool isRetrieved() const;
        /// \brief Prefetch any data
        virtual void prefetch() = 0;
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() = 0;

    private:
        
        void updateMinMax() throw (CauldronIOException);
        void setData(float* data, float** internalData, bool setValue = false, float value = 0) throw (CauldronIOException);
        
        float* m_internalDataIJK;
        float* m_internalDataKIJ;
        float m_minValue, m_maxValue;
        bool m_updateMinMax;
        float m_constantValue;
        bool m_isConstant;
        SubsurfaceKind m_subSurfaceKind;
        std::shared_ptr<const Property> m_property;
        std::shared_ptr<const Volume> m_depthVolume;

    protected:
        void updateGeometry();
        std::shared_ptr<Geometry3D> m_geometry;
        bool m_retrieved;
        size_t m_numI, m_numJ, m_firstK, m_lastK, m_numK;
        double m_deltaI, m_deltaJ, m_minI, m_minJ, m_maxI, m_maxJ;
    };

    /// \class MigrationEvent
    /// \brief This class holds all data related to one migration event
    class MigrationEvent
    {
    public:
        /// \brief Constructor
        MigrationEvent();

        /// \brief set migration process name
        void setMigrationProcess(const std::string& name);
        //// \brief get migration process name
        std::string getMigrationProcess() const;
        /// \brief get source rock age
        float getSourceAge() const;
        /// \brief set source rock age
        void setSourceAge(float val);
        /// \brief set source rock name
        void setSourceRockName(const std::string& name);
        /// \brief get source rock name
        std::string getSourceRockName() const;
        /// \brief set source reservoir name
        void setSourceReservoirName(const std::string& name);
        /// \brief get source reservoir name
        std::string getSourceReservoirName() const;
        /// \brief get source trap ID
        int getSourceTrapID() const;
        /// \brief set source trap ID
        void setSourceTrapID(int val);
        /// \brief get source point X
        float getSourcePointX() const;
        /// \brief set source point X
        void setSourcePointX(float val);
        /// \brief get source point Y
        float getSourcePointY() const;
        /// \brief set source point Y
        void setSourcePointY(float val);
        /// \brief get the destination age
        float getDestinationAge() const;
        /// \brief set the destination age
        void setDestinationAge(float val);
        /// \brief set the destination reservoir name
        void setDestinationReservoirName(const std::string& name);
        /// \brief get the destination reservoir name
        std::string getDestinationReservoirName() const;
        /// \brief set the destination trap ID
        void setDestinationTrapID(int val);
        /// \brief get the destination trap ID
        int getDestinationTrapID();
        /// \brief set the destination point X
        void setDestinationPointX(float val);
        /// \brief get the destination point X
        float getDestinationPointX() const;
        /// \brief set the destination point Y
        void setDestinationPointY(float val);
        /// \brief get the destination point Y
        float getDestinationPointY() const;

        /// \brief get Mass C1
        double getMassC1() const;
        /// \brief get Mass C2
        double getMassC2() const;
        /// \brief get Mass C3
        double getMassC3() const;
        /// \brief get Mass C4
        double getMassC4() const;
        /// \brief get Mass C5
        double getMassC5() const;
        /// \brief get Mass N2
        double getMassN2() const;
        /// \brief get Mass COx
        double getMassCOx() const;
        /// \brief get Mass H2S
        double getMassH2S() const;
        /// \brief get Mass C6_14Aro
        double getMassC6_14Aro() const;
        /// \brief get Mass C6_14Sat
        double getMassC6_14Sat() const;
        /// \brief get Mass C15Aro
        double getMassC15Aro() const;
        /// \brief get Mass C5Sat
        double getMassC15Sat() const;
        /// \brief get Mass LSC
        double getMassLSC() const;
        /// \brief get Mass C15AT
        double getMassC15AT() const;
        /// \brief get Mass C15AroS
        double getMassC15AroS() const;
        /// \brief get Mass C15SatS
        double getMassC15SatS() const;
        /// \brief get Mass C6_14DBT
        double getMassC6_14BT() const;
        /// \brief get Mass C6_14DBT
        double getMassC6_14DBT() const;
        /// \brief get Mass C6_14BP
        double getMassC6_14BP() const;
        /// \brief get Mass C6_14SatS
        double getMassC6_14SatS() const;
        /// \brief get Mass C46_14AroS
        double getMassC6_14AroS() const;
        /// \brief get Mass resins
        double getMassresins() const;
        /// \brief get Mass asphaltenes
        double getMassasphaltenes() const;
        /// \brief set Mass C1
        void setMassC1(double val);
        /// \brief set Mass C2
        void setMassC2(double val);
        /// \brief set Mass C3
        void setMassC3(double val);
        /// \brief set Mass C4
        void setMassC4(double val);
        /// \brief set Mass C5
        void setMassC5(double val);
        /// \brief set Mass N2
        void setMassN2(double val);
        /// \brief set Mass COx
        void setMassCOx(double val);
        /// \brief set Mass H2S
        void setMassH2S(double val);
        /// \brief set Mass C6_14Aro
        void setMassC6_14Aro(double val);
        /// \brief set Mass C6_14Sat
        void setMassC6_14Sat(double val);
        /// \brief set Mass C15Aro
        void setMassC15Aro(double val);
        /// \brief set Mass C15Sat
        void setMassC15Sat(double val);
        /// \brief set Mass LSC
        void setMassLSC(double val);
        /// \brief set Mass C15AT
        void setMassC15AT(double val);
        /// \brief set Mass C15AroS
        void setMassC15AroS(double val);
        /// \brief set Mass C15SatS
        void setMassC15SatS(double val);
        /// \brief set Mass C6_14BT
        void setMassC6_14BT(double val);
        /// \brief set Mass C6_14DBT
        void setMassC6_14DBT(double val);
        /// \brief set Mass C6_14BP
        void setMassC6_14BP(double val);
        /// \brief set Mass C6_14SatS
        void setMassC6_14SatS(double val);
        /// \brief set Mass C6_14AroS
        void setMassC6_14AroS(double val);
        /// \brief set Mass resins
        void setMassresins(double val);
        /// \brief set Mass asphaltenes
        void setMassasphaltenes(double val);

    private:

        // We store the strings as char[] so we can serialize the entire class easily
        char m_migrationProcess[maxStringLength];
        char m_SourceRockName[maxStringLength];
        char m_SourceReservoirName[maxStringLength];
        char m_DestinationReservoirName[maxStringLength];

        float m_destX, m_destY, m_srcX, m_srcY, m_srcAge, m_destAge;
        int m_srcTrapID, m_destTrapID;

        double m_MassC1, m_MassC2, m_MassC3, m_MassC4, m_MassC5, m_MassN2, m_MassCOx, m_MassH2S, m_MassC6_14Aro, m_MassC6_14Sat;
        double m_MassC15Aro, m_MassC15Sat, m_MassLSC, m_MassC15AT, m_MassC15AroS, m_MassC15SatS, m_MassC6_14BT, m_MassC6_14DBT;
        double m_MassC6_14BP, m_MassC6_14SatS, m_MassC6_14AroS, m_Massresins, m_Massasphaltenes;
    };
 
   /// \class Trap
   /// \brief container class holding all some information related to a Trap
   class Trap
   {
   public:
      /// \brief Constructor
      Trap();
      /// \brief Constructor
      /// \param [in] ID the trap ID
      Trap(int ID);
      /// \param [in] reservoirName the name of the reservoir
      void setReservoirName(const std::string& reservoirName);
      /// \param [in] depth the spill depth to assign to this trapper
      void setSpillDepth(float depth);
      /// \param [in] posX the spillpoint X position
      /// \param [in] posY the spillpoint Y position
      void setSpillPointPosition(float posX, float posY);
      /// \param [in] depth the leakage point depth
      void setDepth(float depth);
      /// \param [in] posX the leakage point X position
      /// \param [in] posY the leakage point Y position
      void setPosition(float posX, float posY);
      /// \brief set the gas-oil contact depth
      void setGOC(float goc);
      /// \brief sets the oil-water contact depth
      void setOWC(float goc);
      /// \brief set oil volume
      void setVolumeOil(float volumeLiquid);
      /// \brief set gas volume
      void setVolumeGas(float volumeVapour);
      /// \brief set liquid CEP
      void setCEPOil(float cepLiquid);
      /// \brief set vapour CEP
      void setCEPGas(float cepVapour);
      /// \brief set liquid critical temerature
      void setCriticalTemperatureOil(float criticalTemperatureLiquid);
      /// \brief set vapour critical temerature
      void setCriticalTemperatureGas(float criticalTemperatureVapour);
      /// \brief set liquid interfacialTension
      void setInterfacialTensionOil(float interfacialTensionLiquid);
      /// \brief set vapour interfacialTension
      void setInterfacialTensionGas(float interfacialTensionVapour);
      /// \brief set fracture pressure
      void setFracturePressure(float fracturePressure);
      /// \brief set WC Surface
      void setWCSurface(float wcSurface);
      /// \brief set trap capacity
      void setTrapCapacity(float trapCapacity);
      /// \brief set pressure
      void setPressure(float pressure);
      /// \brief set temperature
      void setTemperature(float temperature);
      /// \brief set permeability
      void setPermeability(float permeability);
      /// \brief set seal permeability
      void setSealPermeability(float sealPermeability);
      /// \brief set net to gross
      void setNetToGross(float netToGross);
      /// \brief set age
      void setAge(float age);
      /// \brief set trap ID
      void setID(int id);
      /// \brief set fracture seal strengt
      void setFractSealStrength(float fractSealStrength);
      /// \brief set mass for one species
      void setMass(double mass, SpeciesNamesId compId);
      
      /// \returns the gas-oil contact depth
      float getOWC() const;
      /// \returns the oil-water contact depth
      float getGOC() const;
      /// \returns the ID
      int getID() const;
      /// \param [out] positionPosX the leakage point X position
      /// \param [out] positionPosY the leakage point Y position
      void getPosition(float& positionPosX, float& positionPosY) const;
      /// \returns the leakage point depth
      float getDepth() const;
      /// \param [out] spillPointPosX the spillpoint X position
      /// \param [out] spillPointPosY the spillpoint Y position
      void getSpillPointPosition(float& spillPointPosX, float& spillPointPosY) const;
      /// \returns the spillpoint depth
      float getSpillDepth() const;
      /// \brief get liquid volume
      float getVolumeOil() const;
      /// \brief get vapour volume
      float getVolumeGas() const;
      /// \brief get liquid CEP
      float getCEPOil() const;
      /// \brief get vapour CEP
      float getCEPGas() const;
      /// \brief get liquid critical temerature
      float getCriticalTemperatureOil() const;
      /// \brief get vapour critical temerature
      float getCriticalTemperatureGas() const;
      /// \brief get liquid interfacialTension
      float getInterfacialTensionOil() const;
      /// \brief get vapour interfacialTension
      float getInterfacialTensionGas() const;
      /// \brief get fracture pressure
      float getFracturePressure() const;
      /// \brief get WC Surface
      float getWCSurface() const;
      /// \brief get trap capacity
      float getTrapCapacity() const;
      /// \brief get pressure
      float getPressure() const;
      /// \brief get temperature
      float getTemperature() const;
      /// \brief get permeability
      float getPermeability() const;
      /// \brief get seal permeability
      float getSealPermeability() const;
      /// \brief get net to gross
      float getNetToGross() const;
      /// \brief get age
      float getAge() const;
      /// \returns the reservoir name
      const std::string getReservoirName() const;
      /// \brief get fracture seal strengt
      float getFractSealStrength() const;
      /// \brief get mass of one species
      double getMass(SpeciesNamesId compId) const;
      
   private:
      int m_ID;
      float m_fractSealStrength;
      
      double m_masses[NUMBER_OF_SPECIES];
      
      float m_depth, m_positionX, m_positionY;
      float m_spillDepth, m_spillPositionX, m_spillPositionY;
      float m_goc, m_owc;
      
      float m_volumeLiquid, m_volumeVapour;
      float m_cepLiquid, m_cepVapour;
      float m_criticalTemperatureLiquid, m_criticalTemperatureVapour;
      float m_interfacialTensionLiquid, m_interfacialTensionVapour;
      float m_fracturePressure;
      float m_wcSurface;
      float m_trapCapacity;
      float m_pressure, m_temperature, m_permeability, m_sealPermeability;
      float m_netToGross;
      float m_age;
      
      char m_reservoir[maxStringLength];
   };
   
   
   /// \class Trapper 
   /// \brief container class holding all some information related to a Trapper
	class Trapper 
	{
	public:
           /// \brief Constructor
           Trapper();
           /// \brief Property constructor
           /// \param [in] ID the ID of the trapper
           /// \param [in] persistentID the persistent ID of the trapper
           explicit Trapper(int ID, int persistentID);
           /// \returns the persistent ID
           int getPersistentID() const;
           /// \returns the trapper ID
           int getID() const;
           /// \brief Assign a downstream trapper for this trapper
           /// \returns the downstream trapper (if any)
           std::shared_ptr<const Trapper> getDownStreamTrapper() const;
           /// \param [in] trapper the downstreamtrapper for this trapper
           void setDownStreamTrapper(std::shared_ptr<const Trapper> trapper);
           /// \param [in] persistentID the downstreamtrapper persistent ID for this trapper; temporary state - do not use
           void setDownStreamTrapperID(int persistentID);
           /// \returns the downstream trapper ID; temporary state - do not use
           int getDownStreamTrapperID() const;
           
           /// \returns get Value by name
           float getValue(const std::string valueName) const;
           /// \brief set the solution gas volume
           void setSolutionGasVolume(float solutionGasVolume);
           /// \brief get the solution gas volume
           float getSolutionGasVolume() const;
           /// \brief set the solution gas density
           void setSolutionGasDensity(float solutionGasDensity);
           /// \brief get the solution gas density
           float getSolutionGasDensity() const;
           /// \brief set the solution gas viscosity
           void setSolutionGasViscosity(float solutionGasViscosity);
           /// \brief get the solution gas viscosity
           float getSolutionGasViscosity() const;
           /// \brief set the one component solution gas mass 
           void setSolutionGasMass(double solutionGasMass, SpeciesNamesId compId);
           /// \brief get the one component solution gas mass 
           double getSolutionGasMass(SpeciesNamesId compId) const;
           /// \brief set thesolution gas mass 
           void setSolutionGasMass(float solutionGasMass);
           /// \brief get the solution gas mass 
           float getSolutionGasMass() const;
           
           /// \brief set the free gas volume
           void setFreeGasVolume(float freeGasVolume);
           /// \brief get the free gas volume
           float getFreeGasVolume() const;
           /// \brief set the free gas density
           void setFreeGasDensity(float freeGasDensity);
           /// \brief get the free gas density
           float getFreeGasDensity() const;
           /// \brief set the free gas viscosity
           void setFreeGasViscosity(float freeGasViscosity);
           /// \brief get the free gas viscosity
           float getFreeGasViscosity() const;
           /// \brief set the one component free gas mass 
           void setFreeGasMass(double freeGasMass, SpeciesNamesId compId);
           /// \brief get the one component free gas mass 
           double getFreeGasMass(SpeciesNamesId compId) const;
           /// \brief set the  free gas mass 
           void setFreeGasMass(float freeGasMass);
           /// \brief get the free gas mass 
           float getFreeGasMass() const;
           
           /// \brief set the condensate volume
           void setCondensateVolume(float condensateVolume);
           /// \brief get the condensate volume
           float getCondensateVolume() const;
           /// \brief set the condensate density
           void setCondensateDensity(float condensateDensity);
           /// \brief get the condensate density
           float getCondensateDensity() const;
           /// \brief set the condensate viscosity
           void setCondensateViscosity(float condensateViscosity);
           /// \brief get the condensate viscosity
           float getCondensateViscosity() const;
           /// \brief set the one component condensate mass 
           void setCondensateMass(double condensateMass, SpeciesNamesId compId);
           /// \brief get the one component condensate mass 
           double getCondensateMass(SpeciesNamesId compId) const;
           /// \brief set the condensate mass 
           void setCondensateMass(float condensateMass);
           /// \brief get the condensate mass 
           float getCondensateMass() const;
           
           /// \brief set the StockTankOil volume
           void setStockTankOilVolume(float stockTankOilVolume);
           /// \brief set the StockTankOil density
           void setStockTankOilDensity(float stockTankOilDensity);
           /// \brief set the StockTankOil viscosity
           void setStockTankOilViscosity(float stockTankOilViscosity);
           /// \brief get the StockTankOil volume
           float getStockTankOilVolume() const;
           /// \brief get the StockTankOil density
           float getStockTankOilDensity() const;
           /// \brief get the StockTankOil viscosity
           float getStockTankOilViscosity() const;
           /// \brief set the one component StockTankOil mass 
           void setStockTankOilMass(double stockTankOilMass, SpeciesNamesId compId);
           /// \brief get the one component StockTankOil mass 
           double getStockTankOilMass(SpeciesNamesId compId) const;
           /// \brief set the StockTankOil mass 
           void setStockTankOilMass(float stockTankOilMass);
           /// \brief get the StockTankOil mass 
           float getStockTankOilMass() const;
           
           /// \brief set liquid mass
           void setMassLiquid(float massLiquid);
           /// \brief set vapour mass
           void setMassVapour(float massVapour);
           /// \brief set liquid viscosity
           void setViscosityLiquid(float viscosityLiquid);
           /// \brief set vapour viscosity
           void setViscosityVapour(float viscosityVapour);
           /// \brief set liquid density
           void setDensityLiquid(float densityLiquid);
           /// \brief set vapour density
           void setDensityVapour(float densityVapour);
           /// \brief set oil API
           void setOilAPI(float oilAPI);
           /// \brief set CGR
           void setCGR(float cgr);
           /// \brief set GOR
           void setGOR(float gor);
           /// \brief set buoyancy
           void setBuoyancy(float buoyancy);
           /// \brief set porosily
           void setPorosity(float porosity);
           /// \brief set persistent trap ID
           void setPersistentId(int id);
           /// \param [in] reservoirName the name of the reservoir
           void setReservoirName(const std::string& reservoirName);
           /// \param [in] depth the spill depth to assign to this trapper
           void setSpillDepth(float depth);
           /// \param [in] posX the spillpoint X position
           /// \param [in] posY the spillpoint Y position
           void setSpillPointPosition(float posX, float posY);
           /// \param [in] depth the leakage point depth
           void setDepth(float depth);
           /// \param [in] posX the leakage point X position
           /// \param [in] posY the leakage point Y position
           void setPosition(float posX, float posY);
           /// \brief set the gas-oil contact depth
           void setGOC(float goc);
           /// \brief sets the oil-water contact depth
           void setOWC(float goc);
           /// \brief set oil volume
           void setVolumeOil(float volumeLiquid);
           /// \brief set gas volume
           void setVolumeGas(float volumeVapour);
           /// \brief set liquid CEP
           void setCEPOil(float cepLiquid);
           /// \brief set vapour CEP
           void setCEPGas(float cepVapour);
           /// \brief set liquid critical temerature
           void setCriticalTemperatureOil(float criticalTemperatureLiquid);
           /// \brief set vapour critical temerature
           void setCriticalTemperatureGas(float criticalTemperatureVapour);
           /// \brief set liquid interfacialTension
           void setInterfacialTensionOil(float interfacialTensionLiquid);
           /// \brief set vapour interfacialTension
           void setInterfacialTensionGas(float interfacialTensionVapour);
           /// \brief set fracture pressure
           void setFracturePressure(float fracturePressure);
           /// \brief set WC Surface
           void setWCSurface(float wcSurface);
           /// \brief set trap capacity
           void setTrapCapacity(float trapCapacity);
           /// \brief set pressure
           void setPressure(float pressure);
           /// \brief set temperature
           void setTemperature(float temperature);
           /// \brief set permeability
           void setPermeability(float permeability);
           /// \brief set seal permeability
           void setSealPermeability(float sealPermeability);
           /// \brief set net to gross
           void setNetToGross(float netToGross);
           /// \brief set age
           void setAge(float age);
           /// \brief set trap ID
           void setID(int id);
           
           /// \brief get liquid mass
           float getMassLiquid() const;
           /// \brief get vapour mass
           float getMassVapour() const;
           /// \brief get liquid viscosity
           float getViscosityLiquid() const;
           /// \brief get vapour viscosity
           float getViscosityVapour() const;
           /// \brief get liquid density
           float getDensityLiquid() const;
           /// \brief get vapour density
           float getDensityVapour() const;
           /// \brief get oil API
           float getOilAPI() const;
           /// \brief get CGR
           float getCGR() const;
           /// \brief get GOR
           float getGOR() const;
           /// \brief get buoyancy
           float getBuoyancy() const;
           /// \brief get porosily
           float getPorosity() const;
           /// \returns the gas-oil contact depth
           float getOWC() const;
           /// \returns the oil-water contact depth
           float getGOC() const;
           /// \param [out] positionPosX the leakage point X position
           /// \param [out] positionPosY the leakage point Y position
           void getPosition(float& positionPosX, float& positionPosY) const;
           /// \returns the leakage point depth
           float getDepth() const;
           /// \param [out] spillPointPosX the spillpoint X position
           /// \param [out] spillPointPosY the spillpoint Y position
           void getSpillPointPosition(float& spillPointPosX, float& spillPointPosY) const;
           /// \returns the spillpoint depth
           float getSpillDepth() const;
           /// \brief get liquid volume
           float getVolumeOil() const;
           /// \brief get vapour volume
           float getVolumeGas() const;
           /// \brief get liquid CEP
           float getCEPOil() const;
           /// \brief get vapour CEP
           float getCEPGas() const;
           /// \brief get liquid critical temerature
           float getCriticalTemperatureOil() const;
           /// \brief get vapour critical temerature
           float getCriticalTemperatureGas() const;
           /// \brief get liquid interfacialTension
           float getInterfacialTensionOil() const;
           /// \brief get vapour interfacialTension
           float getInterfacialTensionGas() const;
           /// \brief get fracture pressure
           float getFracturePressure() const;
           /// \brief get WC Surface
           float getWCSurface() const;
           /// \brief get trap capacity
           float getTrapCapacity() const;
           /// \brief get pressure
           float getPressure() const;
           /// \brief get temperature
           float getTemperature() const;
           /// \brief get permeability
           float getPermeability() const;
           /// \brief get seal permeability
           float getSealPermeability() const;
           /// \brief get net to gross
           float getNetToGross() const;
           /// \brief get age
           float getAge() const;
           /// \returns the reservoir name
           const std::string getReservoirName() const;
        private:
           int m_ID;
           int m_persistentID, m_downstreamTrapperID;
           float m_depth, m_positionX, m_positionY;
           float m_spillDepth, m_spillPositionX, m_spillPositionY;
           float m_goc, m_owc;
           
           float m_volumeLiquid, m_volumeVapour;
           float m_cepLiquid, m_cepVapour;
           float m_criticalTemperatureLiquid, m_criticalTemperatureVapour;
           float m_interfacialTensionLiquid, m_interfacialTensionVapour;
           float m_fracturePressure;
           float m_wcSurface;
           float m_trapCapacity;
           float m_pressure, m_temperature, m_permeability, m_sealPermeability;
           float m_netToGross;
           float m_age;
           
           float m_massLiquid, m_massVapour;
           float m_viscosityLiquid, m_viscosityVapour;
           float m_densityLiquid, m_densityVapour;
           float m_oilAPI, m_cgr, m_gor;
           float m_buoyancy;
           float m_porosity;
           
           float m_solutionGasVolume;
           float m_solutionGasDensity;
           float m_solutionGasViscosity;
           float m_solutionGasMass;
           double m_solutionGasMasses[NUMBER_OF_SPECIES];
           
           float m_freeGasVolume;
           float m_freeGasDensity;
           float m_freeGasViscosity;
           float m_freeGasMass;
           double m_freeGasMasses[NUMBER_OF_SPECIES];
           
           float m_condensateVolume;
           float m_condensateDensity;
           float m_condensateViscosity;
           float m_condensateMass;
           double m_condensateMasses[NUMBER_OF_SPECIES];
           
           float m_stockTankOilVolume;
           float m_stockTankOilDensity;
           float m_stockTankOilViscosity;
           float m_stockTankOilMass;
           double m_stockTankOilMasses[NUMBER_OF_SPECIES];
           
           char m_reservoir[maxStringLength];
           
           std::shared_ptr<const Trapper> m_downstreamTrapper;
	};

   /// \class IsoEntry
   /// \brief container class holding an information stored in one record of TemperatureIsoIoTbl or VrIsoIoTbl- Iso data for burial graph.

   class IsoEntry
   {
   public:
      /// \brief Constructor
      IsoEntry();
 
      /// \brief set the value
      void setContourValue(float value);
      /// \brief set age
      void setAge(float age);
      /// \brief set the number of points
      void setNP(int np);
      /// \brief the depth
      void setSum(double sum);

      /// \returns the contour value
      float getContourValue() const;
      /// \returns the age
      float getAge() const;
      /// \returns the number of points
      int getNP() const;
      /// \returns the depth
      double getSum() const;
    private:
      float m_age;
      float m_contourValue;
      int   m_numberOfPoints;
      double m_sum;
   };

   /// \class DisplayContour
   /// \brief container class holding an information stored in one record of DisplayContourIoTbl - contour settings for any calculated property.
   class DisplayContour
   {
   public:
      /// \brief Constructor
      DisplayContour();
 
      /// \brief set the property name
      void setPropertyName(const std::string& name);
      /// \brief set the contour color
      void setContourColour(const std::string& colour);
      /// \brief set the property name
      void setContourValue(float value);

      /// \returns the property name
      const std::string getPropertyName() const;
      /// \returns the contour color
      const std::string getContourColour() const;
      /// \returns the contour value
      float getContourValue() const;

   private:
      char m_name[maxStringLength];
      char m_contourColour[maxStringLength];
      float m_contourValue;
   };

   /// \class Biomarkerm
   /// \brief container class holding Biomarker measurements for a well
   class Biomarkerm
   {
   public:
      /// \brief Constructor
      Biomarkerm();
 
      /// \brief set the depth index
      void setDepthIndex(float depthIndex);
      /// \brief set the hopane Isomerisation 
      void setHopaneIsomerisation(float value);
      /// \brief set the sterane Isomerisation
      void setSteraneIsomerisation(float value);
      /// \brief set the sterane Aromatisation
      void setSteraneAromatisation(float value);
      /// \brief set optimization flag
      void setOptimization(bool flag);

      /// \returns the depth index
      float getDepthIndex() const;
      /// \returns the hopane Isomerisation
      float getHopaneIsomerisation() const;
      /// \returns the sterane Isomerisation
      float getSteraneIsomerisation() const;
      /// \returns the sterane Aromatisation
      float getSteraneAromatisation() const;
      /// \returns optimization flag
      bool getOptimization() const;
   private:

      float m_depthIndex;
      float m_hopaneIsomerisation;
      float m_steraneIsomerisation;
      float m_steraneAromatisation;
      bool  m_optimization;
   };

   /// \class FtSample
   /// \brief container class holding Fission track sample data
   class FtSample
   {
   public:
      /// \brief Constructor
      FtSample();
      
      /// \brief set ID
      const std::string getFtSampleId() const;
      void setFtSampleId(const std::string& value);
      
      float getDepthIndex() const;
      void setDepthIndex(float value);
      
      float getFtZeta() const;
      void setFtZeta(float value);
      
      float getFtUstglTrackDensity() const;
      void setFtUstglTrackDensity(float value);
      
      float getFtPredictedAge() const;
      void setFtPredictedAge(float value);

      float getFtPooledAge() const;
      void setFtPooledAge(float value);

      float getFtPooledAgeErr() const;
      void setFtPooledAgeErr(float value);

      float getFtAgeChi2() const;
      void setFtAgeChi2(float value);

      int getFtDegreeOfFreedom() const;
      void setFtDegreeOfFreedom(int value);

      float getFtPAgeChi2() const;
      void setFtPAgeChi2(float value);

      float getFtCorrCoeff() const;
      void setFtCorrCoeff(float value);

      float getFtVarianceSqrtNs() const;
      void setFtVarianceSqrtNs(float value);

      float getFtVarianceSqrtNi() const;
      void setFtVarianceSqrtNi(float value);

      float getFtNsDivNi() const;
      void setFtNsDivNi(float value);

      float getFtNsDivNiErr() const;
      void setFtNsDivNiErr(float value);

      float getFtMeanRatio() const;
      void setFtMeanRatio(float value);

      float getFtMeanRatioErr() const;
      void setFtMeanRatioErr(float value);

      float getFtCentralAge() const;
      void setFtCentralAge(float value);

      float getFtCentralAgeErr() const;
      void setFtCentralAgeErr(float value);

      float getFtMeanAge() const;
      void setFtMeanAge(float value);

      float getFtMeanAgeErr() const;
      void setFtMeanAgeErr(float value);

      float getFtLengthChi2() const;
      void setFtLengthChi2(float value);

      const std::string  getFtApatiteYield() const;
      void setFtApatiteYield( const std::string & value );

   private:

      char m_FtSampleId[maxStringLength];
      float m_DepthIndex;
      float m_FtZeta; 
      float m_FtUstglTrackDensity;
      float m_FtPredictedAge; 
      float m_FtPooledAge; 
      float m_FtPooledAgeErr;
      float m_FtAgeChi2; 
      int m_FtDegreeOfFreedom; 
      float m_FtPAgeChi2; 
      float m_FtCorrCoeff; 
      float m_FtVarianceSqrtNs;
      float m_FtVarianceSqrtNi; 
      float m_FtNsDivNi; 
      float m_FtNsDivNiErr; 
      float m_FtMeanRatio; 
      float m_FtMeanRatioErr; 
      float m_FtCentralAge;
      float m_FtCentralAgeErr; 
      float m_FtMeanAge; 
      float m_FtMeanAgeErr; 
      float m_FtLengthChi2; 
      char m_FtApatiteYield[maxStringLength]; 
      
   };
   /// \class FtGrain
   /// \brief container class holding spontaneous and induced track count per grain
   class FtGrain
   {
   public:
      /// \brief Constructor
      FtGrain();

      const std::string getFtSampleId() const;
      void setFtSampleId(const std::string & val);

      int getFtGrainId() const;
      void setFtGrainId(int val);

      int getFtSpontTrackNo() const;
      void setFtSpontTrackNo(int val);

      int getFtInducedTrackNo() const;
      void setFtInducedTrackNo(int val);

      double getFtClWeightPerc() const;
      void setFtClWeightPerc(float val);

      double getFtGrainAge() const;
      void setFtGrainAge(float val);
   private:

      char m_FtSampleId[maxStringLength]; ; 
      int m_FtGrainId; 
      int m_FtSpontTrackNo; 
      int m_FtInducedTrackNo;
      float m_FtClWeightPerc;
      float m_FtGrainAge; 
    }; 

   /// \class FtPredLengthCountsHist
   /// \brief Container class holding description of layout for the histograms containing the predicted fission track length counts
   class FtPredLengthCountsHist
   {
   public:
      /// \brief Constructor
      FtPredLengthCountsHist();

      int getFtPredLengthHistId() const;
      void setFtPredLengthHistId(int val);

      const std::string getFtSampleId() const;
      void setFtSampleId(const std::string & val);

      float getFtClWeightPerc() const;
      void setFtClWeightPerc(float val);

      float getFtPredLengthBinStart() const;
      void setFtPredLengthBinStart(float val);

      float getFtPredLengthBinWidth() const;
      void setFtPredLengthBinWidth(float val);

      int getFtPredLengthBinNum() const;
      void setFtPredLengthBinNum(int val);

   private:

      int m_FtPredLengthHistId; 
      char m_FtSampleId[maxStringLength]; 
      float m_FtClWeightPerc;
      float m_FtPredLengthBinStart;
      float m_FtPredLengthBinWidth; 
      int m_FtPredLengthBinNum; 
  };

   /// \class FtPredLengthCountsHistData
   /// \brief Container class holding description of histogram data (predicted counts per track length bin) of histograms 
   /// defined in FtPredLengthCountsHist
   class FtPredLengthCountsHistData
   {
   public:
      /// \brief Constructor
      FtPredLengthCountsHistData();

      int getFtPredLengthHistId() const;
      void setFtPredLengthHistId(int val);

      int getFtPredLengthBinIndex() const;
      void setFtPredLengthBinIndex(int val);

      float getFtPredLengthBinCount() const;
      void setFtPredLengthBinCount(float val);
   private:

      int m_FtPredLengthHistId;
      int m_FtPredLengthBinIndex; 
      float m_FtPredLengthBinCount; 
   };

   /// \class FtClWeightPercBins
   /// \brief Container class holding description of layout of the Cl Weight Bins used in the fission track prediction algorithms

   class FtClWeightPercBins
   {
   public:
      /// \brief Constructor
      FtClWeightPercBins();

      double getFtClWeightBinStart() const;
      void setFtClWeightBinStart(double val);

      double getFtClWeightBinWidth() const;
      void setFtClWeightBinWidth(double val);
      
   private:
      double m_FtClWeightBinStart; 
      double m_FtClWeightBinWidth; 
   };
   /// \class SmectiteIllite
   /// \brief Container class holding description of measured smectite illite data for well

   class SmectiteIllite
   {
   public:
      /// \brief Constructor
      SmectiteIllite();

      float getDepthIndex() const;
      void setDepthIndex(float val);

      float getIlliteFraction() const;
      void setIlliteFraction(float val);

      const std::string getLabel() const;
      void setLabel(const std::string & val);

      bool getOptimization() const;
      void setOptimization(bool val);

   private:
      float m_DepthIndex;
      float m_IlliteFraction; 
      char m_Label[maxStringLength];
      bool m_Optimization; 

   };
   /// \class DepthIo
   /// \brief Container class listing the statistics of property values
   ///        for a depth/time combinations (output results)

   class DepthIo
   {
   public:
      /// \brief Constructor
      DepthIo();

      const std::string getPropertyName() const;
      void setPropertyName(const std::string & val);

      float getTime() const;
      void setTime(float val);

      float getDepth() const;
      void setDepth(float val);

      float getAverage() const;
      void setAverage(float val);

      float getStandardDev() const;
      void setStandardDev(float val);

      float getMinimum() const;
      void setMinimum(float val);

      float getMaximum() const;
      void setMaximum(float val);

      float getSum() const;
      void setSum(float val);

      float getSum2() const;
      void setSum2(float val);

      int getNP() const;
      void setNP(int val);

      float getP15() const;
      void setP15(float val);

      float getP50() const;
      void setP50(float val);

      float getP85() const;
      void setP85(float val);

      float getSumFirstPower() const;
      void setSumFirstPower(float val);

      float getSumSecondPower() const;
      void setSumSecondPower(float val);

      float getSumThirdPower() const;
      void setSumThirdPower(float val);

      float getSumFourthPower() const;
      void setSumFourthPower(float val);

      float getSkewness() const;
      void setSkewness(float val);

      float getKurtosis() const;
      void setKurtosis(float val);

   private:
      char m_PropertyName[maxStringLength]; 
      float m_Time;
      float m_Depth; 
      float m_Average;
      float m_StandardDev;
      float m_Minimum;
      float m_Maximum; 
      float m_Sum; 
      float m_Sum2;
      int m_NP; 
      float m_P15; 
      float m_P50; 
      float m_P85; 
      float m_SumFirstPower; 
      float m_SumSecondPower; 
      float m_SumThirdPower;
      float m_SumFourthPower;
      float m_Skewness;
      float m_Kurtosis; 
   };

   /// \class TimeIo1D
   /// \brief Container class listing 1D property values (1D output results)

   class TimeIo1D
   {
   public:
      /// \brief Constructor
      TimeIo1D();

      float getTime() const;
      void setTime(float val);

      const std::string getPropertyName() const;
      void setPropertyName(const std::string & val);

      const std::string getFormationName() const;
      void setFormationName(const std::string & val);

      int getNodeIndex() const;
      void setNodeIndex(int val);

      const std::string getSurfaceName() const;
      void setSurfaceName(const std::string & val);

      float getValue() const;
      void setValue(float val);
   private:
      float m_Time; 
      char  m_PropertyName[maxStringLength]; 
      char  m_FormationName[maxStringLength];
      int   m_NodeIndex; 
      char  m_SurfaceName[maxStringLength]; 
      float m_Value;
   };
 
}


#ifdef _MSC_VER
#pragma warning (pop)
#endif

#endif
