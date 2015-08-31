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

#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>         

/// \namespace CauldronIO
/// \brief The namespace for the visualization IO library related classes
namespace CauldronIO
{
    // enums

    /// \enum ModellingMode
    /// \brief FastCauldron modeling modes
    enum ModellingMode { MODE1D, MODE3D };
    /// \enum SnapShotKind 
    /// \brief Type of snapshot
    enum SnapShotKind { SYSTEM, USERDEFINED, NONE };
    /// \enum PropertyAttribute
    /// \brief Type of property
    enum PropertyAttribute { Continuous3DProperty, Discontinuous3DProperty, Surface2DProperty, Formation2DProperty, Other};
    /// \enum PropertyType
    /// \brief Type of property
    enum PropertyType { ReservoirProperty, FormationProperty, TrapProperty };
    /// \enum SubsurfaceKind
    /// \brief Type of subsurface
    enum SubsurfaceKind { Sediment, Basement, None };

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

    /// forward declarations
    class SnapShot;
    class Map;
    class Property;
    class Formation;
    class Volume;
    class Surface;
    class DiscontinuousVolume;
    /// type definitions
    typedef std::pair<boost::shared_ptr<const Formation>, boost::shared_ptr<Volume> > FormationVolume;
    typedef std::vector<boost::shared_ptr<FormationVolume > > FormationVolumeList;
    typedef std::vector<boost::shared_ptr<SnapShot > > SnapShotList;
    typedef std::vector<boost::shared_ptr<Surface > > SurfaceList;
    typedef std::vector<boost::shared_ptr<Volume > > VolumeList;
    typedef std::vector<boost::shared_ptr<DiscontinuousVolume > > DiscontinuousVolumeList;
    
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
            ModellingMode mode);

        /// \brief Destructor
        ~Project();

		/// \brief Adds a snapshot to the current project
		void AddSnapShot(boost::shared_ptr<SnapShot>& snapShot);

        /// \brief Retrieve all data in project
        void Retrieve();

        /// \returns Name of the project
        const std::string& GetName() const;
        /// \returns Description of the project
        const std::string& GetDescription() const;
        /// \returns Team working on this project (?)
        const std::string& GetTeam() const;
        /// \returns Obsolete
        const std::string& GetProgramVersion() const;
        /// \returns Modeling mode for this run
        ModellingMode GetModelingMode() const;
        /// \returns The list of snapshots
        const SnapShotList& GetSnapShots() const;

    private:
        SnapShotList _snapShotList;
        std::string _name, _description, _team, _version;
        ModellingMode _mode;
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
        void Retrieve();

	    /// \brief Add a surface to the snapshot; ownership is transfered
	    void AddSurface(boost::shared_ptr<Surface>& surface);
 	    /// \brief Add a volume to the snapshot; ownership is transfered
        void AddVolume(boost::shared_ptr<Volume>& volume);
        /// \brief Add a discontinuous volume to the snapshot; ownership is transfered
        void AddDiscontinuousVolume(boost::shared_ptr<DiscontinuousVolume>& discVolume);

	    /// \returns Age of snapshot
        double GetAge () const;
        /// \returns kind of snapshot
        SnapShotKind GetKind() const;
	    /// \returns True if this is a minor snapshot
        bool IsMinorShapshot() const;

        /// \returns the list of surfaces
        const SurfaceList& GetSurfaceList() const;
        /// \returns the list of volumes
        const VolumeList& GetVolumeList() const;
        /// \returns the list of discontinuous volumes
        const DiscontinuousVolumeList& GetDiscontinuousVolumeList() const;
		
    private:
	    SurfaceList _surfaceList;
        VolumeList _volumeList;
        DiscontinuousVolumeList _discVolumeList;
        SnapShotKind _kind;
        bool _isMinor;
        double _age;
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
        const std::string& GetName() const;
	    /// \returns the user name of this property
        const std::string& GetUserName() const;
	    /// \returns the cauldron name of this property
        const std::string& GetCauldronName() const;
	    /// \return the unit of this property
        const std::string& GetUnit() const;
	    /// \returns the PropertyType
        PropertyType GetType() const;
	    /// \returns the PropertyAttribute
        PropertyAttribute GetAttribute() const;

    private:
	    std::string _name, _username, _cauldronName, _unit;
	    PropertyType _type;
	    PropertyAttribute _attrib;
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
        Formation(size_t kStart, size_t kEnd, const std::string& name);
        /// \returns the depth range
        void GetDepthRange(size_t &start, size_t &end) const;
        /// \returns the formation name
        const std::string& GetName() const;

    private:
        std::string _name;
        size_t _kstart, _kend;
    };

    /// \class Surface
    /// \brief Container class to hold meta information about a surface. The actual data is contained in \class Map
    class Surface
    {
    public:
        /// \brief Construct a new surface. Ownership of all objects is transfered to the surface
        /// Depthmap can be null, for example for thickness or erosionfactor. 
        /// These are formation maps, not associated with a particular surface
        /// \param [in] name Name of the surface
        /// \param [in] kind kind of surface
        /// \param [in] property of this surface
        /// \param [in] valueMap the Map to associate with this surface
        Surface(const std::string& name, SubsurfaceKind kind, boost::shared_ptr<const Property> property, 
            boost::shared_ptr<Map> valueMap);

        /// \brief Get the name of this surface
        const std::string& GetName() const;
        /// \brief Get the values for this surface
        const boost::shared_ptr<Map> GetValueMap() const;
        /// \returns the subsurface kind
        SubsurfaceKind GetSubSurfaceKind() const;
        /// \returns the associate property with this grid
        const boost::shared_ptr<const Property> GetProperty() const;
        /// \brief Associate a formation with this map
        /// \param [in] formation the formation to be associated with this map. Optional
        void SetFormation(boost::shared_ptr<const Formation> formation);
        /// \returns the associated formation for this map. Can be null
        const boost::shared_ptr<const Formation> GetFormation() const;
        /// \brief Set the associated Depth surface for this surface
        /// \param [in] surface the depth-surface to associate with this 
        void SetDepthSurface(boost::shared_ptr<const Surface> surface);
        /// \returns the associated Depth surface for this surface; CAN BE NULL
        const boost::shared_ptr<const Surface> GetDepthSurface() const;
        /// \brief Retrieve actual data into memory
        void Retrieve();
        /// \returns true if data is available
        bool IsRetrieved() const;

    private:
        SubsurfaceKind _subSurfaceKind;
        boost::shared_ptr<const Property> _property;
        boost::shared_ptr<const Formation> _formation;
        boost::shared_ptr<const Surface> _depthSurface;
        boost::shared_ptr<Map> _valueMap;
        std::string _name;
    };

    /// \class Map 
    /// \brief container class to hold data for a Surface
    class Map
    {
    public:
        /// \brief Create a new map specifying cell-centricity and the undefined value
        Map(bool cellCentered);
        ~Map();

        /// \returns the currently assigned UUID; this can be useful for referencing other maps, not in memory
        const boost::uuids::uuid& GetUUID() const;
        /// \brief Sets the currently assigned UUID; this can be useful for referencing other maps, not in memory
        /// \param [in] uuid the uuid to associate this map with
        void SetUUID(const boost::uuids::uuid& uuid);

        /// \brief Assign data to the map : geometry must have been assigned
        /// \param [in] data pointer to the xy data, ordered row-wise
        void SetData_IJ(float* data);
        /// \brief assign a geometry
        /// \param [in] numI number of elements in i-direction
        /// \param [in] numJ number of elements in j-direction
        /// \param [in] deltaI spacing in i-direction
        /// \param [in] deltaJ spacing in j-direction
        /// \param [in] minI map origin in i-direction
        /// \param [in] minJ map origin in j-direction
        void SetGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ);
        /// \brief Retrieve the data
        virtual void Retrieve() = 0;
        /// \returns true if data is available
        bool IsRetrieved() const;
        /// \returns true if this map is cell centered
        bool IsCellCentered() const;
        /// \returns true if this map is node centered
        bool IsNodeCentered() const { return !IsCellCentered(); }
        /// \returns the local horizontal resolution
        size_t GetNumI() const;
        /// \returns  the local vertical resolution
        size_t GetNumJ() const;
        /// \returns  the distance between two vertical grid lines
        double GetDeltaI() const;
        /// \returns  the distance between two horizontal grid lines
        double GetDeltaJ() const;
        /// \returns the leftmost grid coordinate value
        double GetMinI() const;
        /// \returns  the bottommost grid coordinate value
        double GetMinJ() const;
        /// \returns  the rightmost grid coordinate value
        double GetMaxI() const;
        /// \returns  the topmost grid coordinate value
        double GetMaxJ() const;
        /// \returns  true if data is represented per row
        bool CanGetRow() const;
        /// \returns true if data is represented per column
        bool CanGetColumn() const;
        /// \brief Gets the value at the specified grid coordinate
        /// \param [in] i row index
        /// \param [in] j column index
        /// \returns
        float GetValue(size_t i, size_t j) const;
        /// \brief Gets an entire row; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] j column index
        /// \returns
        float const * GetRowValues(size_t j);
        /// \brief Gets an entire column; can be null if this map is not row-ordered (or throw an exception)
        /// \param [in] i row index
        /// \returns
        float const * GetColumnValues(size_t i);
        /// \returns pointer to entire data 
        float const * GetSurfaceValues();
        /// \brief Convenience function to get an index into the 1D volume data 
        /// \param [in] i row index
        /// \param [in] j column index
        inline size_t GetMapIndex(size_t i, size_t j) const;
        /// \returns true if surface is constant
        bool IsConstant() const;
        /// \brief Set a constant value for this surface
        /// \param [in] constantValue
        void SetConstantValue(float constantValue);
        /// \returns the constant value
        float GetConstantValue() const;
        /// \returns true if grid coordinate is undefined
        /// \param [in] i row index
        /// \param [in] j column index
        bool IsUndefined(size_t i, size_t j) const;
        /// \returns the undefined value
        float GetUndefinedValue() const;
        /// \brief Set the undefined value
        /// \param [in] undefined the undefined value value
        void SetUndefinedValue(float undefined);

    private:
        double _deltaI, _deltaJ, _minI, _minJ, _maxI, _maxJ;
        size_t _numI, _numJ;
        float _constantValue, _undefinedValue;
        bool _isConstant, _isCellCentered;
        void SetData(float* data, bool setValue = false, float value = 0);
        float* _internalData;
        boost::uuids::uuid _uuid;

    protected:
        bool _retrieved;
        bool _geometryAssigned;
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
        /// \param [in] cellCentered if true, this volume is cell centered: its associated depth volume will be one larger in each dimension
        /// \param [in] kind the SubsurfaceKind of this volume
        /// \param [in] property the property to assign to this volume
        Volume(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property);
        /// \brief Destructor
        ~Volume();

        /// \returns the SubsurfaceKind
        SubsurfaceKind GetSubSurfaceKind() const;
        /// \returns the associate property with this grid
        const boost::shared_ptr<const Property> GetProperty() const;
        /// \brief Assigns a related depth volume to this volume
        /// \param [in] depthVolume the depth volume to assign to this volume
        void SetDepthVolume(boost::shared_ptr<const Volume> depthVolume);
        /// \returns an associated depth volume; should not be null
        boost::shared_ptr<const Volume> GetDepthVolume() const;

        /// \returns the currently assigned UUID; this can be useful for referencing other maps, not in memory
        const boost::uuids::uuid& GetUUID() const;
        /// \brief Sets the currently assigned UUID; this can be useful for referencing other maps, not in memory
        /// \param [in] uuid the uuid to associate with this volume
        void SetUUID(const boost::uuids::uuid& uuid);
        /// \brief Retrieve the data: to be implemented by inherited class
        virtual void Retrieve() = 0;
        /// \returns true if data is available
        bool IsRetrieved() const;

        /// \brief Assign data to the volume as a 1D array: K fastest, then I, then J
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void SetData_KIJ(float* data, bool setValue = false, float value = 0);
        /// \brief Assign data to the volume as a 1D array: I fastest, then J, then K
        /// \param [in] data a pointer to the data; it will be copied, no ownership is transferred
        /// \param [in] setValue if true, a constant value will be assigned to the data
        /// \param [in] value the value to assign to the data if setValue is true
        void SetData_IJK(float* data, bool setValue = false, float value = 0);

        /// \brief assign a geometry: the k-offset is the index of the first k-element (can be zero)
        /// \param [in] numI number of elements in i-dimension
        /// \param [in] numJ number of elements in j-dimension
        /// \param [in] numK number of elements in k-dimension
        /// \param [in] offsetK the offset in k-dimension; if non-zero, subsequent index should assume this offset
        /// \param [in] deltaI spacing in i-dimension
        /// \param [in] deltaJ spacing in j-dimension
        /// \param [in] minI the volume origin in i-dimension
        /// \param [in] minJ the volume origin in j-dimension
        void SetGeometry(size_t numI, size_t numJ, size_t numK, size_t offsetK,
            double deltaI, double deltaJ, double minI, double minJ);

        /// \returns true if this volume is cell centered
        bool IsCellCentered() const;
        /// \returns true if this volume is node centered
        bool IsNodeCentered() const { return !IsCellCentered(); }
        /// \returns the local horizontal resolution
        size_t GetNumI() const;
        /// \returns the local vertical resolution
        size_t GetNumJ() const;
        /// \returns the number of k elements in this volume
        size_t GetNumK() const;
        /// \returns the index of the first k element
        size_t GetFirstK() const;
        /// \returns the index of the last k element (inclusive)
        size_t GetLastK() const;
        /// \returns the distance between two vertical gridlines
        double GetDeltaI() const;
        /// \returns the distance between two horizontal gridlines
        double GetDeltaJ() const;
        /// return the leftmost grid coordinate value
        double GetMinI() const;
        /// \returns the bottommost grid coordinate value
        double GetMinJ() const;
        /// \returns the rightmost grid coordinate value
        double GetMaxI() const;
        /// \returns the topmost grid coordinate value
        double GetMaxJ() const;
        /// \returns true if IJK data is present (false if not present or constant data)
        bool HasDataIJK() const;
        /// \returns true if KIJ data is present (false if not present or constant data)
        bool HasDataKIJ() const;

        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns if value is undefined
        bool IsUndefined(size_t i, size_t j, size_t k) const;
        /// \brief Set the undefined value
        void SetUndefinedValue(float undefined);
        /// \returns the undefined value
        float GetUndefinedValue() const;
        /// \returns true if surface is constant
        bool IsConstant() const;
        /// \brief Set a constant value for this volume
        /// \param [in] constantValue the value to assign to all data of this volume
        void SetConstantValue(float constantValue);
        /// \returns the constant value
        float GetConstantValue() const;

        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns the value at the specified grid coordinate
        float GetValue(size_t i, size_t j, size_t k) const;
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire row of data; can be null if this volume is not row-ordered (or throw an exception)
        float const * GetRowValues(size_t j, size_t k);
        /// \param [in] i index in i-dimension
        /// \param [in] k index in k-dimension
        /// \returns an entire column of data; can be null if this volume is not row-ordered (or throw an exception)
        float const * GetColumnValues(size_t i, size_t k);
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \throws CauldronIOException
        /// \returns an entire needle of data; can be null if this volume is not needle-ordered (or throw an exception)
        float const * GetNeedleValues(size_t i, size_t j);
        /// \param [in] k index in k-dimension
        /// \returns pointer to entire data for the surface at depth k; can be null if data is not stored per ij surface
        float const * GetSurface_IJ(size_t k);
        /// \returns pointer to entire data: can be NULL
        float const * GetVolumeValues_IJK();
        /// \returns pointer to entire data: can be NULL 
        float const * GetVolumeValues_KIJ();

        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        size_t ComputeIndex_IJK(size_t i, size_t j, size_t k) const;
        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        /// \param [in] i index in i-dimension
        /// \param [in] j index in j-dimension
        /// \param [in] k index in k-dimension
        size_t ComputeIndex_KIJ(size_t i, size_t j, size_t k) const;

    private:
        void SetData(float* data, float** internalData, bool setValue = false, float value = 0);
        float* _internalDataIJK;
        float* _internalDataKIJ;
        double _deltaI, _deltaJ, _minI, _minJ, _maxI, _maxJ;
        size_t _numI, _numJ, _numK, _firstK;
        float _constantValue, _undefinedValue;
        bool _isConstant, _isCellCentered;
        SubsurfaceKind _subSurfaceKind;
        boost::shared_ptr<const Property> _property;
        boost::shared_ptr<const Volume> _depthVolume;
        boost::uuids::uuid _uuid;

    protected:
        bool _retrieved, _geometryAssigned;
    };

    /// \class DiscontinuousVolume
    /// \brief This is a legacy volume facilitating to hold properties discontinuous across formation boundaries
    class DiscontinuousVolume
    {
    public:
        /// \brief Constructor; create a new discontinuous volume
        DiscontinuousVolume();
        /// \brief Destructor
        ~DiscontinuousVolume();
        /// \returns false; this volume is NOT cell centered
        bool IsCellCentered() const { return false; }
        /// \brief Adds a volume/formation pair to the list
        /// \param [in] formation Formation instance
        /// \param [in] volume Volume instance
        void AddVolume(boost::shared_ptr<Formation> formation, boost::shared_ptr<Volume> volume);
        /// \brief Get the entire list of (sub)volumes
        const FormationVolumeList& GetVolumeList() const;
        /// \brief Assigns a related depth volume to this volume
        /// \param [in] depthVolume the depth volume to associate with this DiscontinuousVolume
        void SetDepthVolume(boost::shared_ptr<const Volume> depthVolume);
        /// \returns The associated depth volume 
        boost::shared_ptr<const Volume> GetDepthVolume() const;
        /// \brief Load all data in this volume
        void Retrieve();

    private:
        FormationVolumeList _volumeList;
        boost::shared_ptr<const Volume> _depthVolume;
    };
}

#endif