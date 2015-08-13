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

namespace CauldronIO
{
    // enums

    /// \brief FastCauldron modeling modes
    enum ModellingMode { MODE1D, MODE3D };
    /// \brief Type of snapshot
    enum SnapShotKind { SYSTEM, USERDEFINED, NONE };
    /// \brief Type of property
    enum PropertyAttribute { Continuous3DProperty, Discontinuous3DProperty, Surface2DProperty, Formation2DProperty, Other};
    /// \brief Type of property
    enum PropertyType { ReservoirProperty, FormationProperty, TrapProperty };
    /// \brief Type of subsurface
    enum SubsurfaceKind { Sediment, Basement, None };

    // Our own exception class
    class CauldronIOException : public std::runtime_error
    {
    public:
        CauldronIOException(const std::string& message) : std::runtime_error(message) {}
        CauldronIOException(const char* message) : std::runtime_error(message) {}
    };

    // forward declarations
    class SnapShot;
    class Map;
    class Property;
    class Formation;
    class Volume;
    class Surface;
    class DiscontinuousVolume;
    // type definitions
    typedef std::pair<boost::shared_ptr<const Formation>, boost::shared_ptr<const Volume> > FormationVolume;
    /// the disadvantage is that when an object has been added to the list, it cannot be modified any longer...
    typedef std::vector<boost::shared_ptr<const FormationVolume > > FormationVolumeList;
    typedef std::vector<boost::shared_ptr<const SnapShot > > SnapShotList;
    typedef std::vector<boost::shared_ptr<const Surface > > SurfaceList;
    typedef std::vector<boost::shared_ptr<const Volume > > VolumeList;
    typedef std::vector<boost::shared_ptr<const DiscontinuousVolume > > DiscontinuousVolumeList;
    
    /// \brief Main entry: project class
    class Project
    {
    public:
        /// \brief Create an empty project
        Project(const std::string& name, const std::string& description, const std::string& team, const std::string& version,
            ModellingMode mode);

        ~Project();

		/// \brief Adds a snapshot to the current project
		void AddSnapShot(const boost::shared_ptr<const SnapShot>& snapShot);

        /// Basic info: 
         /// \brief Name of the project
        const std::string& GetName() const;
        /// \brief Description of the project
        const std::string& GetDescription() const;
        /// \brief Team working on this project (?)
        const std::string& GetTeam() const;
        /// \brief Obsolete
        const std::string& GetProgramVersion() const;
        /// \brief Modeling mode for this run
        ModellingMode GetModelingMode() const;

        /// \brief Gets the list of snapshots
        const boost::shared_ptr<const SnapShotList> GetSnapShots() const;

    private:
	    boost::shared_ptr<SnapShotList> _snapShotList;
        std::string _name, _description, _team, _version;
        ModellingMode _mode;
    };

    /// \brief Snapshot container class
    class SnapShot
    {
	public:
	    /// \brief create an empty snapshot
	    SnapShot(double age, SnapShotKind kind, bool isMinorShapshot);
        ~SnapShot();

	    /// \brief Add a surface to the snapshot; ownership is transfered
	    void AddSurface(const boost::shared_ptr<const Surface>& surface);
 	    /// \brief Add a volume to the snapshot; ownership is transfered
        void AddVolume(const boost::shared_ptr<const Volume>& volume);
        /// \brief Add a discontinuous volume to the snapshot; ownership is transfered
        void AddDiscontinuousVolume(const boost::shared_ptr<const DiscontinuousVolume>& discVolume);

	    // Basic info
        double GetAge () const;
        SnapShotKind GetKind() const;
	    bool IsMinorShapshot() const;

        /// \brief Get the list of surfaces
        const boost::shared_ptr<const SurfaceList> GetSurfaceList() const;
        /// \brief Get the list of volumes
        const boost::shared_ptr<const VolumeList> GetVolumeList() const;
        /// \brief Get the list of discontinuous volumes
        const boost::shared_ptr<const DiscontinuousVolumeList> GetDiscontinuousVolumeList() const;
		
    private:
	    boost::shared_ptr<SurfaceList> _surfaceList;
        boost::shared_ptr<VolumeList> _volumeList;
        boost::shared_ptr<DiscontinuousVolumeList> _discVolumeList;
        SnapShotKind _kind;
        bool _isMinor;
        double _age;
	};
    
	/// \brief Property information class
	class Property
	{
	public:
        /// \brief 
        Property(const std::string& name, const std::string& username, const std::string& cauldronName, const std::string& unit,
            PropertyType type, PropertyAttribute attrib);
	    /// \brief Returns the name of this property
        const std::string& GetName() const;
	    /// \brief Returns the user name of this property
        const std::string& GetUserName() const;
	    /// \brief Returns the cauldron name of this property
        const std::string& GetCauldronName() const;
	    /// \brief Return the unit of this property
        const std::string& GetUnit() const;
	    /// \brief Returns the PropertyType
        PropertyType GetType() const;
	    /// \brief Returns the PropertyAttribute
        PropertyAttribute GetAttribute() const;

    private:
	    std::string _name, _username, _cauldronName, _unit;
	    PropertyType _type;
	    PropertyAttribute _attrib;
    };

    /// \brief Formation container class
    class Formation
    {
    public:
        /// \brief Creates a new formation covering a depth range and name
        Formation(size_t kStart, size_t kEnd, const std::string& name);
        /// \brief Get the depth range
        void GetDepthRange(size_t &start, size_t &end) const;
        /// \brief Get the formation name
        const std::string& GetName() const;

    private:
        std::string _name;
        size_t _kstart, _kend;
    };

    /// \brief Surface container class
    class Surface
    {
    public:
        /// \brief Construct a new surface. Ownership of all objects is transfered to the surface
        /// Depthmap can be null, for example for thickness or erosionfactor. 
        /// These are formation maps, not associated with a particular surface
        Surface(const std::string& name, SubsurfaceKind kind, boost::shared_ptr<const Property> property, 
            boost::shared_ptr<const Map> valueMap);

        /// \brief Get the name of this surface
        const std::string& GetName() const;
        /// \brief Get the values for this surface
        const boost::shared_ptr<const Map> GetValueMap() const;
        /// \brief Returns the subsurface kind
        SubsurfaceKind GetSubSurfaceKind() const;
        /// \brief return the associate property with this grid
        const boost::shared_ptr<const Property> GetProperty() const;
        /// \brief Associate a formation with this map
        void SetFormation(boost::shared_ptr<const Formation> formation);
        /// \brief Get the associated formation for this map
        const boost::shared_ptr<const Formation> GetFormation() const;
        /// \brief Set the associated Depth surface for this surface
        void SetDepthSurface(boost::shared_ptr<const Surface> surface);
        /// \brief Get the associated Depth surface for this surface; CAN BE NULL
        const boost::shared_ptr<const Surface> GetDepthSurface() const;

    private:
        SubsurfaceKind _subSurfaceKind;
        boost::shared_ptr<const Property> _property;
        boost::shared_ptr<const Formation> _formation;
        boost::shared_ptr<const Map> _valueMap;
        boost::shared_ptr<const Surface> _depthSurface;
        std::string _name;
    };

    /// \brief Map container class
    class Map
    {
    public:
        /// \brief Create a new map specifying cell-centricity and the undefined value
        Map(bool cellCentered, float undefined);
        ~Map();

        /// \brief Assign data to the map : geometry must have been assigned
        void SetData_IJ(float* data);
        /// \brief assign a geometry
        void SetGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ);

        /// \brief return true if this map is cell centered
        bool IsCellCentered() const;
        /// \brief return true if this map is node centered
        bool IsNodeCentered() const { return !IsCellCentered(); }
        /// \brief return the local horizontal resolution
        size_t GetNumI() const;
        /// \brief return the local vertical resolution
        size_t GetNumJ() const;
        /// \brief return the distance between two vertical grid lines
        double GetDeltaI() const;
        /// \brief return the distance between two horizontal grid lines
        double GetDeltaJ() const;
        /// return the leftmost grid coordinate value
        double GetMinI() const;
        /// \brief return the bottommost grid coordinate value
        double GetMinJ() const;
        /// \brief return the rightmost grid coordinate value
        double GetMaxI() const;
        /// \brief return the topmost grid coordinate value
        double GetMaxJ() const;
        /// \brief Returns true if data is represented per row
        bool CanGetRow() const;
        /// \brief Returns true if data is represented per column
        bool CanGetColumn() const;
        /// \brief 
        bool IsUndefined(size_t i, size_t j) const;
        /// \brief 
        float GetValue(size_t i, size_t j) const;
        /// \brief Gets an entire row; can be null if this map is not row-ordered (or throw an exception)
        float const * GetRowValues(size_t j);
        /// \brief Gets an entire column; can be null if this map is not row-ordered (or throw an exception)
        float const * GetColumnValues(size_t i);
        /// \brief Returns pointer to entire data 
        float const * GetSurfaceValues();
        /// \brief Convenience function to get an index into the 1D volume data 
        inline size_t GetMapIndex(size_t i, size_t j) const;

        bool IsConstant() const;
        void SetConstantValue(float constantValue);
        float GetUndefinedValue() const;

    private:
        double _deltaI, _deltaJ, _minI, _minJ, _maxI, _maxJ;
        size_t _numI, _numJ;
        bool _geometryAssigned;
        float _constantValue, _undefinedValue;
        bool _isConstant, _isCellCentered;
        void SetData(float* data, bool setValue = false, float value = 0);
        float* _internalData;
    };

    /// \brief A continuous volume is continuous across formation boundaries. It can still be (legacy) node centered as well as cell centered.
    class Volume
    {
    public:
        /// \brief Constructor
        Volume(bool cellCentered, float undefined, SubsurfaceKind kind, boost::shared_ptr<const Property> property);
        ~Volume();

        /// \brief Returns the subsurface kind
        SubsurfaceKind GetSubSurfaceKind() const;
        /// \brief return the associate property with this grid
        const boost::shared_ptr<const Property> GetProperty() const;

        /// \brief Assign data to the volume as a 1D array: K fastest, then I, then J
        void SetData_KIJ(float* data, bool setValue = false, float value = 0);
        /// \brief Assign data to the volume as a 1D array: I fastest, then J, then K
        void SetData_IJK(float* data, bool setValue = false, float value = 0);

        /// \brief assign a geometry: the k-offset is the index of the first k-element (can be zero)
        void SetGeometry(size_t numI, size_t numJ, size_t numK, size_t offsetK,
            double deltaI, double deltaJ, double minI, double minJ);

        /// \brief return true if this volume is cell centered
        bool IsCellCentered() const;
        /// \brief return true if this volume is node centered
        bool IsNodeCentered() const { return !IsCellCentered(); }
        /// \brief return the local horizontal resolution
        size_t GetNumI() const;
        /// \brief return the local vertical resolution
        size_t GetNumJ() const;
        /// \brief return the number of k elements in this volume
        size_t GetNumK() const;
        /// \brief return the index of the first k element
        size_t GetFirstK() const;
        /// \brief return the index of the last k element (inclusive)
        size_t GetLastK() const;
        /// \brief return the distance between two vertical gridlines
        double GetDeltaI() const;
        /// \brief return the distance between two horizontal gridlines
        double GetDeltaJ() const;
        /// return the leftmost grid coordinate value
        double GetMinI() const;
        /// \brief return the bottommost grid coordinate value
        double GetMinJ() const;

        /// \brief return the rightmost grid coordinate value
        double GetMaxI() const;
        /// \brief return the topmost grid coordinate value
        double GetMaxJ() const;
        double GetMaxK() const;

        /// \brief Returns true if data is represented per row, for constant k
	    bool CanGetRow() const;
	    /// \brief Returns true if data is represented per column, for constant k
	    bool CanGetColumn() const;
	    /// \brief Returns true if data is represented per needle, for constant ij
        bool CanGetNeedle() const;
	    /// \brief Returns true if data is represented per surface, for constant k
        bool CanGetSurface_IJ() const;
	    /// \brief 
	    bool IsUndefined(size_t i, size_t j, size_t k) const;
        bool IsConstant() const;
        void SetConstantValue(float constantValue);
        float GetUndefinedValue() const;

	    /// \brief 
	    float GetValue(size_t i, size_t j, size_t k) const;
	    /// \brief Gets an entire row; can be null if this volume is not row-ordered (or throw an exception)
	    float const * GetRowValues(size_t j, size_t k);
	    /// \brief Gets an entire column; can be null if this volume is not row-ordered (or throw an exception)
	    float const * GetColumnValues(size_t i, size_t k);
        /// \brief Gets an entire needle; can be 
        float const * GetNeedleValues(size_t i, size_t j);
	    /// \brief Returns pointer to entire data 
        float const * GetSurface_IJ(size_t k);
	    /// \brief Returns pointer to entire data: can be NULL
	    float const * GetVolumeValues_IJK();
        /// \brief Returns pointer to entire data: can be NULL 
        float const * GetVolumeValues_KIJ();

        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        size_t ComputeIndex_IJK(size_t i, size_t j, size_t k) const;
        /// \brief Convenience function to get an index into the 1D volume data : indexing is through the full-k range, corresponding to the depth volume
        size_t ComputeIndex_KIJ(size_t i, size_t j, size_t k) const;

    private:
        void SetData(float* data, float* internalData, bool setValue = false, float value = 0);
        inline bool IsDataAvailable() const;
        float* _internalDataIJK;
        float* _internalDataKIJ;
        double _deltaI, _deltaJ, _minI, _minJ, _maxI, _maxJ;
        size_t _numI, _numJ, _numK, _firstK;
        bool _geometryAssigned;
        float _constantValue, _undefinedValue;
        bool _isConstant, _isCellCentered;
        SubsurfaceKind _subSurfaceKind;
        boost::shared_ptr<const Property> _property;
    };

    /// \brief This is a legacy volume. It is node centered and therefore, most properties are discontinuous across formation boundaries
    class DiscontinuousVolume
    {
    public:
        /// \brief Create a new discontinuous volume by providing the first subvolume
        DiscontinuousVolume();
        ~DiscontinuousVolume();
        /// \brief This type of volume is NOT cell centered
        bool IsCellCentered() const { return false; }
        /// \brief Adds a volume to the list
        void AddVolume(boost::shared_ptr<const Formation> formation, boost::shared_ptr<const Volume> volume);
        /// \brief Get the entire list of (sub)volumes
        const boost::shared_ptr<const FormationVolumeList> GetVolumeList() const;

    private:
        boost::shared_ptr<FormationVolumeList> _volumeList;
    };
}

#endif