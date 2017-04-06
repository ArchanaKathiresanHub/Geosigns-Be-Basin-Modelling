//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
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

    private:
        SnapShotList m_snapShotList;
        std::string m_name, m_description, m_team, m_version;
        ModellingMode m_mode;
        PropertyList m_propertyList;
        FormationList m_formationList;
        ReservoirList m_reservoirList;
        GeometryList m_geometries;
        MigrationEventList m_migrationEventList;
        int m_xmlVersionMajor, m_xmlVersionMinor;
        StratigraphyTableEntryList m_stratTable;
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

    private:
        SurfaceList m_surfaceList;
        std::shared_ptr<Volume> m_volume;
        FormationVolumeList m_formationVolumeList;
        TrapperList m_trapperList;
        SnapShotKind m_kind;
        bool m_isMinor;
        double m_age;
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

    /// \class Trapper 
    /// \brief container class holding all some information related to a Trapper
    class Trapper
    {
    public:
        /// \brief Property constructor
        /// \param [in] ID the ID of the trapper
        /// \param [in] persistentID the persistent ID of the trapper
        explicit Trapper(int ID, int persistentID);
        /// \returns the reservoir name
        const std::string& getReservoirName() const;
        /// \param [in] reservoirName the name of the reservoir
        void setReservoirName(const std::string& reservoirName);
        /// \returns the spillpoint depth
        float getSpillDepth() const;
        /// \param [in] depth the spill depth to assign to this trapper
        void setSpillDepth(float depth);
        /// \param [out] spillPointPosX the spillpoint X position
        /// \param [out] spillPointPosY the spillpoint Y position
        void getSpillPointPosition(float& spillPointPosX, float& spillPointPosY) const;
        /// \param [in] posX the spillpoint X position
        /// \param [in] posY the spillpoint Y position
        void setSpillPointPosition(float posX, float posY);
        /// \returns the leakage point depth
        float getDepth() const;
        /// \param [in] depth the leakage point depth
        void setDepth(float depth);
        /// \param [out] positionPosX the leakage point X position
        /// \param [out] positionPosY the leakage point Y position
        void getPosition(float& positionPosX, float& positionPosY) const;
        /// \param [in] posX the leakage point X position
        /// \param [in] posY the leakage point Y position
        void setPosition(float posX, float posY);
        /// \returns the ID
        int getID() const;
        /// \returns the persistent ID
        int getPersistentID() const;
        /// \brief Assign a downstream trapper for this trapper
        /// \returns the downstream trapper (if any)
        std::shared_ptr<const Trapper> getDownStreamTrapper() const;
        /// \param [in] trapper the downstreamtrapper for this trapper
        void setDownStreamTrapper(std::shared_ptr<const Trapper> trapper);
        /// \param [in] persistentID the downstreamtrapper persistent ID for this trapper; temporary state - do not use
        void setDownStreamTrapperID(int persistentID);
        /// \returns the downstream trapper ID; temporary state - do not use
        int getDownStreamTrapperID() const;
        /// \brief set the gas-oil contact depth
        void setGOC(float goc);
        /// \returns the gas-oil contact depth
        float getGOC() const;
        /// \brief sets the oil-water contact depth
        void setOWC(float goc);
        /// \returns the oil-water contact depth
        float getOWC() const;

    private:
        int m_ID, m_persistentID, m_downstreamTrapperID;
        float m_depth, m_positionX, m_positionY;
        float m_spillDepth, m_spillPositionX, m_spillPositionY;
        float m_goc, m_owc;
        std::shared_ptr<const Trapper> m_downstreamTrapper;
        std::string m_reservoir;
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

    private:

        void updateMinMax() throw (CauldronIOException);
        void setData(float* data, bool setValue = false, float value = 0) throw (CauldronIOException);

        float* m_internalData;
        float m_constantValue;
        bool m_isConstant;
        bool m_updateMinMax;
        float m_minValue, m_maxValue;
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
        static const int m_maxStringLength = 256;

        // We store the strings as char[] so we can serialize the entire class easily
        char m_migrationProcess[m_maxStringLength];
        char m_SourceRockName[m_maxStringLength];
        char m_SourceReservoirName[m_maxStringLength];
        char m_DestinationReservoirName[m_maxStringLength];

        float m_destX, m_destY, m_srcX, m_srcY, m_srcAge, m_destAge;
        int m_srcTrapID, m_destTrapID;

        double m_MassC1, m_MassC2, m_MassC3, m_MassC4, m_MassC5, m_MassN2, m_MassCOx, m_MassH2S, m_MassC6_14Aro, m_MassC6_14Sat;
        double m_MassC15Aro, m_MassC15Sat, m_MassLSC, m_MassC15AT, m_MassC15AroS, m_MassC15SatS, m_MassC6_14BT, m_MassC6_14DBT;
        double m_MassC6_14BP, m_MassC6_14SatS, m_MassC6_14AroS, m_Massresins, m_Massasphaltenes;
    };
}


#ifdef _MSC_VER
#pragma warning (pop)
#endif

#endif

