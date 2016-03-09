//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CAULDRONIO_H_
#define _CAULDRONIO_H_

#include "VisualizationAPIFwDecl.h"

#include <vector>
#include <string>
#include <utility>
#include <stdexcept>

/// \namespace CauldronIO
/// \brief The namespace for the visualization IO library related classes
namespace CauldronIO
{
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
    typedef std::pair<  boost::shared_ptr<const Formation>, boost::shared_ptr<Volume> >     FormationVolume;
    typedef std::vector<FormationVolume >                                                   FormationVolumeList;
    typedef std::vector<boost::shared_ptr<SnapShot > >                                      SnapShotList;
    typedef std::vector<boost::shared_ptr<Surface > >                                       SurfaceList;
    typedef std::vector<boost::shared_ptr<Volume > >                                        VolumeList;
    typedef std::vector<boost::shared_ptr<Trapper > >                                       TrapperList;
    typedef std::vector<boost::shared_ptr<const Property > >                                PropertyList;
    typedef std::vector<boost::shared_ptr<const Formation> >                                FormationList;
    typedef std::vector<boost::shared_ptr<const Reservoir> >                                ReservoirList;
    typedef std::pair<  boost::shared_ptr<const Property>, boost::shared_ptr<SurfaceData> > PropertySurfaceData;
    typedef std::pair<  boost::shared_ptr<const Property>, boost::shared_ptr<VolumeData> >  PropertyVolumeData;
    typedef std::vector < PropertySurfaceData >                                             PropertySurfaceDataList;
    typedef std::vector < PropertyVolumeData >                                              PropertyVolumeDataList;

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
        Project(const std::string& name, const std::string& description, const std::string& team, const std::string& version,
            ModellingMode mode, int xmlVersionMajor, int xmlVersionMinor);

        /// \brief Destructor
        ~Project();

		/// \brief Adds a snapshot to the current project
		void addSnapShot(boost::shared_ptr<SnapShot>& snapShot);
        /// \brief Adds a property to the current project
        void addProperty(boost::shared_ptr<const Property>& property);
        /// \brief Adds a formation to the current project
        void addFormation(boost::shared_ptr<const Formation>& formation);
        /// \brief Adds a reservoir to the current project
        void addReservoir(boost::shared_ptr<const Reservoir>& newReservoir);
        /// \brief Retrieve all data in project
        void retrieve();
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
        /// \returns the property (if existing) for the given name
        boost::shared_ptr<const Property> findProperty(std::string propertyName) const;
        /// \returns a list of all formations
        const FormationList& getFormations() const;
        /// \returns the formation (if existing) for the given name
        boost::shared_ptr<const Formation> findFormation(std::string formationName) const;
        /// \returns a list of reservoirs
        const ReservoirList& getReservoirs() const;
        /// \returns the reservoir (if existing) for the given name [we assume there are no reservoirs with identical names and different otherwise]
        boost::shared_ptr<const Reservoir> findReservoir(std::string reservoirName) const;
        /// \returns a list of strings containing the names of all surfaces
        const std::vector<std::string>& getSurfaceNames();
        /// \returns the xml version number
        int getXmlVersionMajor() const;
        /// \returns the xml version number
        int getXmlVersionMinor() const;

    private:
        SnapShotList m_snapShotList;
        std::string m_name, m_description, m_team, m_version;
        ModellingMode m_mode;
        PropertyList m_propertyList;
        FormationList m_formationList;
        ReservoirList m_reservoirList;
        std::vector<std::string> m_surfaceNames;
        int m_xmlVersionMajor, m_xmlVersionMinor;
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
	    SnapShot(double age, SnapShotKind kind, bool isMinorShapshot);
        ~SnapShot();

        /// \brief Retrieve all data in snapshot: 
        /// This will iterate over all contained data (surfaces, volumes, etc.)
        void retrieve();
        /// \brief Release all data in the snapshot
        void release();

	    /// \brief Add a surface to the snapshot; ownership is transfered
	    void addSurface(boost::shared_ptr<Surface>& surface);
 	    /// \brief Add a volume to the snapshot; ownership is transfered
        void setVolume(boost::shared_ptr<Volume>& volume);
        /// \brief Add a discontinuous volume to the snapshot; ownership is transfered
        void addFormationVolume(FormationVolume& formVolume);
        /// \brief Add a trapper to the snapshot; ownership is transfered
        void addTrapper(boost::shared_ptr<Trapper>& trapper);

	    /// \returns Age of snapshot
        double getAge () const;
        /// \returns kind of snapshot
        SnapShotKind getKind() const;
	    /// \returns True if this is a minor snapshot
        bool isMinorShapshot() const;

        /// \returns the list of surfaces
        const SurfaceList& getSurfaceList() const;
        /// \returns the list of volumes
        const boost::shared_ptr<Volume>& getVolume() const;
        /// \returns the list of discontinuous volumes
        const FormationVolumeList& getFormationVolumeList() const;
        /// \returns the list of trappers
        const TrapperList& getTrapperList() const;
		
    private:
	    SurfaceList m_surfaceList;
        boost::shared_ptr<Volume> m_volume;
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
        Reservoir(const std::string& reservoirName, const boost::shared_ptr<const Formation>& formation);

        /// \returns the name of this reservoir
        const std::string& getName() const;
        /// \returns the formation of this reservoir
        const boost::shared_ptr<const Formation>& getFormation() const;
        /// \returns true if two reservoirs are equal
        bool operator==(const Reservoir& other) const;

    private:
        std::string m_reservoirName;
        boost::shared_ptr<const Formation> m_formation;
    };

    /// \class Trapper 
    /// \brief container class holding all some information related to a Trapper
    class Trapper
    {
    public:
        /// \brief Property constructor
        /// \param [in] ID the ID of the trapper
        /// \param [in] persistentID the persistent ID of the trapper
        Trapper(int ID, int persistentID);
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
        boost::shared_ptr<const Trapper> getDownStreamTrapper() const;
        /// \param [in] trapper the downstreamtrapper for this trapper
        void setDownStreamTrapper(boost::shared_ptr<const Trapper> trapper);
        /// \param [in] persistentID the downstreamtrapper persistent ID for this trapper; temporary state - do not use
        void setDownStreamTrapperID(int persistentID);
        /// \returns the downstream trapper ID; temporary state - do not use
        int getDownStreamTrapperID() const;

    private:
        int m_ID, m_persistentID, m_downstreamTrapperID;
        float m_depth, m_positionX, m_positionY;
        float m_spillDepth, m_spillPositionX, m_spillPositionY;
        boost::shared_ptr<const Trapper> m_downstreamTrapper;
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
        Property(const std::string& name, const std::string& username, const std::string& cauldronName, const std::string& unit,
            PropertyType type, PropertyAttribute attrib);
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
        /// \param [in] isSourceRock true if this formation is a source rock
        /// \param [in] isMobileLayer true if this formation is a mobile layer
        Formation(size_t kStart, size_t kEnd, const std::string& name, bool isSourceRock, bool isMobileLayer);
        /// \returns the depth range
        void getK_Range(unsigned int & k_range_start, unsigned int& k_range_end) const;
        /// \returns the formation name
        const std::string& getName() const;
        /// \returns true if two formations are equal
        bool operator==(const Formation& other) const;
        /// \returns true if this formation is a source rock
        bool isSourceRock() const;
        /// \returns true if this formation is a mobile layer
        bool isMobileLayer() const;

    private:
        std::string m_name;
        size_t m_kstart, m_kend;
        bool m_isSourceRock, m_isMobileLayer;
    };

    /// \class Surface
    /// \brief Container class to hold meta information about a surface. The actual data is contained in \class Map
    class Surface
    {
    public:
        /// \brief Construct a new surface
        /// \param [in] name Name of the surface
        /// \param [in] kind kind of surface
        /// \param [in] geometry the geometry to associate with this surface
        Surface(const std::string& name, SubsurfaceKind kind);
        /// \brief Destructor
        ~Surface();
        
        /// \brief assign the geometry
        void setGeometry(boost::shared_ptr<const Geometry2D>& geometry);
        /// \brief assign the high-res geometry
        void setHighResGeometry(boost::shared_ptr<const Geometry2D>& geometry);
        /// \returns the geometry; can be null
        const boost::shared_ptr<const Geometry2D>& getGeometry() const;
        /// \returns the high-res geometry; can be null
        const boost::shared_ptr<const Geometry2D>& getHighResGeometry() const;
        /// \brief get the list of property-surfaceData pairs contained in this surface
        const PropertySurfaceDataList& getPropertySurfaceDataList() const;
        /// \brief Add a property-surfaceData pair to the list
        void addPropertySurfaceData( PropertySurfaceData& data);
        /// \returns true if this surface has a depth surface
        bool hasDepthSurface() const; 
        /// \returns the depth surface data; can be null
        boost::shared_ptr<SurfaceData> getDepthSurface() const;
        /// \brief Get the name of this surface
        const std::string& getName() const;
        /// \returns the subsurface kind
        SubsurfaceKind getSubSurfaceKind() const;
        /// \brief Associate a formation with this map
        /// \param [in] formation the formation to be associated with this map. Optional.
        void setFormation(boost::shared_ptr<const Formation>& formation, bool isTop);
        /// \returns the associated top formation for this map. Can be null
        const boost::shared_ptr<const Formation>& getTopFormation() const;
        /// \returns the associated bottom formation for this map. Can be null
        const boost::shared_ptr<const Formation>& getBottomFormation() const;
        /// \brief Retrieve actual data into memory
        void retrieve();
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        void release();
        /// \returns true if data is available
        bool isRetrieved() const;

    private:
        SubsurfaceKind m_subSurfaceKind;
        boost::shared_ptr<const Formation> m_Topformation;
        boost::shared_ptr<const Formation> m_Bottomformation;
        std::string m_name;
        std::string m_reservoirName;
        PropertySurfaceDataList m_propSurfaceList;
        boost::shared_ptr<const Geometry2D> m_geometry;
        boost::shared_ptr<const Geometry2D> m_highresgeometry;
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
        Geometry2D(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ);
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

    protected:
        double m_deltaI, m_deltaJ, m_minI, m_minJ, m_maxI, m_maxJ;
        size_t m_numI, m_numJ;
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
        Geometry3D(size_t numI, size_t numJ, size_t numK, size_t offsetK,
            double deltaI, double deltaJ, double minI, double minJ);

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
        /// \param [in] first K index in k-dimension
        /// \param [in] numK number of elements in k-dimension
        void updateK_range(size_t firstK, size_t numK);

    private:
        size_t m_numK, m_firstK;
    };

    /// \class VisualizationIOData
    /// \brief interface class for surface and volume data
    class VisualizationIOData
    {
    public:
        /// \brief Retrieve the data
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release() = 0;
        /// \brief Prefetch any data
        virtual void prefetch() = 0;
        /// \returns true if data is available
        virtual bool isRetrieved() const = 0;
    };

    /// \class SurfaceData 
    /// \brief container class to hold data for a Surface
    class SurfaceData : public VisualizationIOData
    {
    public:
        /// \brief Create a surface-data object
        SurfaceData(const boost::shared_ptr<const Geometry2D>& geometry);
        ~SurfaceData();

        /// VisualizationIOData implementation
        //////////////////////////////////////////////////////////////////////////
        /// \brief Retrieve the data
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns true if data is available
        virtual bool isRetrieved() const;
        /// \brief Prefetch any data
        virtual void prefetch() = 0;

        /// \returns the geometry
        const boost::shared_ptr<const Geometry2D>& getGeometry() const;
        /// \brief Assign data to the map : geometry must have been assigned
        /// \param [in] data pointer to the xy data, ordered row-wise
        /// \note data ownership is not transferred; data should be deleted by client if obsolete
        void setData_IJ(float* data);
        /// \returns  true if data is represented per row
        bool canGetRow() const;
        /// \returns true if data is represented per column
        bool canGetColumn() const;
        /// \brief Gets the value at the specified grid coordinate
        /// \param [in] i row index
        /// \param [in] j column index
        /// \returns
        float getValue(size_t i, size_t j) const;
        /// \brief Gets an entire row; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] j column index
        /// \returns
        const float* getRowValues(size_t j);
        /// \brief Gets an entire column; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] i row index
        /// \returns
        const float* getColumnValues(size_t i);
        /// \returns pointer to entire data 
        const float* getSurfaceValues();
        /// \brief Convenience function to get an index into the 1D volume data 
        /// \param [in] i row index
        /// \param [in] j column index
        inline size_t getMapIndex(size_t i, size_t j) const;
        /// \returns true if surface is constant
        bool isConstant() const;
        /// \brief Set a constant value for this surface
        /// \param [in] constantValue
        void setConstantValue(float constantValue);
        /// \returns the constant value
        float getConstantValue() const;
        /// \returns true if grid coordinate is undefined
        /// \param [in] i row index
        /// \param [in] j column index
        bool isUndefined(size_t i, size_t j) const;
        /// \returns the undefined value
        float getUndefinedValue() const;
        /// \brief Set the undefined value
        /// \param [in] undefined the undefined value value
        void setUndefinedValue(float undefined);
        /// \param [in] formation the formation to be associated with this map. Optional.
        void setFormation(boost::shared_ptr<const Formation>& formation);
        /// \returns the associated top formation for this map. Can be null
        const boost::shared_ptr<const Formation>& getFormation() const;
        /// \brief Set the reservoir of this surface
        void setReservoir(boost::shared_ptr<const Reservoir> reservoir);
        /// \brief Get the name of the reservoir associated with the surface. Optional.
        const boost::shared_ptr<const Reservoir>& getReservoir() const;

    private:
        float* m_internalData;
        float m_constantValue, m_undefinedValue;
        bool m_isConstant, m_isCellCentered;
        void setData(float* data, bool setValue = false, float value = 0);
        boost::shared_ptr<const Formation> m_formation;
        boost::shared_ptr<const Reservoir> m_reservoir;
        boost::shared_ptr<const Geometry2D> m_geometry;

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
        Volume(SubsurfaceKind kind);
        /// \brief Destructor
        ~Volume();

        /// \returns the SubsurfaceKind
        SubsurfaceKind getSubSurfaceKind() const;
        /// \brief get the list of property-surfaceData pairs contained in this surface
        const PropertyVolumeDataList& getPropertyVolumeDataList() const;
        /// \brief Add a property-surfaceData pair to the list
        void addPropertyVolumeData(PropertyVolumeData& data);
        /// \returns true if this surface has a depth surface
        bool hasDepthVolume() const;
        /// \returns the depth surface data; can be null
        boost::shared_ptr<VolumeData> getDepthVolume() const;

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
        VolumeData(const boost::shared_ptr<Geometry3D>& geometry);
        ~VolumeData();

        /// \returns the geometry
        const boost::shared_ptr<Geometry3D>& getGeometry() const;
        /// \brief Assign data to the volume as a 1D array: K fastest, then I, then J
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void setData_KIJ(float* data, bool setValue = false, float value = 0);
        /// \brief Assign data to the volume as a 1D array: I fastest, then J, then K
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void setData_IJK(float* data, bool setValue = false, float value = 0);
        /// \returns true if IJK data is present (false if not present or constant data)
        bool hasDataIJK() const;
        /// \returns true if KIJ data is present (false if not present or constant data)
        bool hasDataKIJ() const;
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns if value is undefined
        bool isUndefined(size_t i, size_t j, size_t k) const;
        /// \brief Set the undefined value
        void setUndefinedValue(float undefined);
        /// \returns the undefined value
        float getUndefinedValue() const;
        /// \returns true if surface is constant
        bool isConstant() const;
        /// \brief Set a constant value for this volume
        /// \param [in] constantValue the value to assign to all data of this volume
        void setConstantValue(float constantValue);
        /// \returns the constant value
        float getConstantValue() const;
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns the value at the specified grid coordinate
        float getValue(size_t i, size_t j, size_t k) const;
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire row of data; can be null if this volume is not row-ordered (or throw an exception)
        const float* getRowValues(size_t j, size_t k);
        /// \param [in] i index in i-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire column of data; can be null if this volume is not row-ordered (or throw an exception)
        const float* getColumnValues(size_t i, size_t k);
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \throws CauldronIOException
        /// \returns an entire needle of data; can be null if this volume is not needle-ordered (or throw an exception)
        const float* getNeedleValues(size_t i, size_t j);
        /// \param [in] k index in k-dimension
        /// \returns pointer to entire data for the surface at depth k; can be null if data is not stored per ij surface
        const float* getSurface_IJ(size_t k);
        /// \returns pointer to entire data: can be NULL
        const float* getVolumeValues_IJK();
        /// \returns pointer to entire data: can be NULL 
        const float* getVolumeValues_KIJ();

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

        /// VisualizationIOData implementation
        //////////////////////////////////////////////////////////////////////////
        /// \brief Retrieve the data
        virtual void retrieve() = 0;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns true if data is available
        virtual bool isRetrieved() const;
        /// \brief Prefetch any data
        virtual void prefetch() = 0;

    private:
        void setData(float* data, float** internalData, bool setValue = false, float value = 0);
        float* m_internalDataIJK;
        float* m_internalDataKIJ;
        float m_constantValue, m_undefinedValue;
        bool m_isConstant, m_isCellCentered;
        SubsurfaceKind m_subSurfaceKind;
        boost::shared_ptr<const Property> m_property;
        boost::shared_ptr<const Volume> m_depthVolume;

    protected:
        void updateGeometry();
        boost::shared_ptr<Geometry3D> m_geometry;
        bool m_retrieved;
        size_t m_numI, m_numJ, m_firstK, m_lastK, m_numK;
        double m_deltaI, m_deltaJ, m_minI, m_minJ, m_maxI, m_maxJ;
    };
}

#endif