#ifndef _CAULDRONIO_H_
#define _CAULDRONIO_H_

#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
using namespace std;

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
    class CauldronIOException : public runtime_error
    {
    public:
        CauldronIOException(const string& message) : runtime_error(message) {}
        CauldronIOException(const char* message) : runtime_error(message) {}
    };

    // forward declarations
    class SnapShot;
    class Map;
    class Grid;
    class Property;
    class Formation;
    class Volume;
    // type definitions
    typedef pair<const Formation*, const Volume*> FormationVolume;
    typedef vector<const FormationVolume *> FormationVolumeList;
    typedef vector<const SnapShot *> SnapShotList;
    typedef vector<const Grid *> GridList;
    
    /// \brief Main entry: project class
    class Project
    {
    public:
        /// \brief Create an empty project
        Project(const string& name, const string& description, const string& team, const string& version, 
            ModellingMode mode);

        ~Project();

		/// \brief Adds a snapshot to the current project
		void AddSnapShot(const SnapShot *);

        /// Basic info: 
        /// TODO consider removing some of these properties below; they may not be relevant

        /// \brief Name of the project
        const string& GetName() const;
        /// \brief Description of the project
        const string& GetDescription() const;
        /// \brief Team working on this project (?)
        const string& GetTeam() const;
        /// \brief Obsolete
        const string& GetProgramVersion() const;
        /// \brief Modeling mode for this run
        ModellingMode GetModelingMode() const;

        /// \brief Gets the list of snapshots
        const boost::shared_ptr<SnapShotList> GetSnapShots() const;

    private:
	    boost::shared_ptr<SnapShotList> _snapShotList;
        string _name, _description, _team, _version;
        ModellingMode _mode;
    };

    /// \brief Snapshot container class
    class SnapShot
    {
	public:
	    /// \brief create an empty snapshot
	    SnapShot(double age, SnapShotKind kind, bool isMinorShapshot);
        ~SnapShot();

	    /// \brief Add a grid to the snapshot
	    void AddGrid(const Grid* grid);

	    // Basic info
        double GetAge () const;
        SnapShotKind GetKind() const;
	    bool IsMinorShapshot() const;

        // Contained data
        const boost::shared_ptr<GridList> GetGridList() const;
		
    private:
	    boost::shared_ptr<GridList> _gridList;
        SnapShotKind _kind;
        bool _isMinor;
        double _age;
	};
    
	/// \brief Property information class
	class Property
	{
	public:
        /// \brief 
        Property(const string& name, const string& username, const string& cauldronName, const string& unit,
            PropertyType type, PropertyAttribute attrib);
	    /// \brief Returns the name of this property
        const string& GetName() const;
	    /// \brief Returns the user name of this property
        const string& GetUserName() const;
	    /// \brief Returns the cauldron name of this property
        const string& GetCauldronName() const;
	    /// \brief Return the unit of this property
        const string& GetUnit() const;
	    /// \brief Returns the PropertyType
        PropertyType GetType() const;
	    /// \brief Returns the PropertyAttribute
        PropertyAttribute GetAttribute() const;

    private:
	    string _name, _username, _cauldronName, _unit;
	    PropertyType _type;
	    PropertyAttribute _attrib;
    };

    /// \brief Formation container class
    class Formation
    {
    public:
        /// \brief Creates a new formation covering a depth range and name
        Formation(size_t kStart, size_t kEnd, const string& name);
        /// \brief Get the depth range
        void GetDepthRange(size_t &start, size_t &end) const;
        /// \brief Get the formation name
        const string& GetName() const;

    private:
        string _name;
        size_t _kstart, _kend;
    };

    /// \brief Grid container class: baseclass for both Map and Volume
    class Grid
    {
    public:
        /// \brief return true if this map is cell centered
        bool IsCellCentered() const;
        /// \brief return true if this map is node centered
        bool IsNodeCentered() const { return !IsCellCentered(); }
        /// \brief return the local horizontal resolution
        size_t GetNumI() const;
        /// \brief return the local vertical resolution
        size_t GetNumJ() const;
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

        // Values

        /// \brief returns the undefined value equivalent
        float GetUndefinedValue() const;
        /// \brief returns true if the value is constant for the entire grid
        virtual bool IsConstant() const;
        /// \brief Set a constant value for this Grid
        virtual void SetConstantValue(float value);

        /// \brief Returns the subsurface kind
        SubsurfaceKind GetSubSurfaceKind() const;
        /// \brief Sets the subsurface kind
        void SetSubserfaceKind(SubsurfaceKind kind);

        /// \brief return the associate property with this grid
        const boost::shared_ptr<const Property> GetProperty() const;
        /// \brief set the associated property for this grid
        void SetProperty(const Property* property);

    protected:
        /// \brief Can only be created by a derived class
        Grid(bool cellCentered, float undefined);
    
    private:
		boost::shared_ptr<const Property> _property;
		SubsurfaceKind _subSurfaceKind;
    protected:
        double _deltaI, _deltaJ, _minI, _minJ, _maxI, _maxJ;
        size_t _numI, _numJ;
        bool _geometryAssigned;
        float _constantValue, _undefinedValue;
        bool _isConstant, _isCellCentered;
    };

    /// \brief Map container class
    class Map : public Grid
    {
    public:
        /// \brief Create a new map specifying cell-centricity and the undefined value
        Map(bool cellCentered, float undefined);
        ~Map();

        /// \brief Assign data to the map : geometry must have been assigned
        void SetData_IJ(float* data);

        /// \brief assign a geometry
        void SetGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ, double maxI, double maxJ);

        /// \brief Returns true if data is represented per row
        bool CanGetRow() const;
        /// \brief Returns true if data is represented per column
        bool CanGetColumn() const;
        /// \brief 
        bool IsUndefined(size_t i, size_t j) const;
        /// \brief 
        float GetValue(size_t i, size_t j) const;
        /// \brief Gets an entire row; can be null if this map is not row-ordered (or throw an exception)
        const float* GetRowValues(size_t j);
        /// \brief Gets an entire column; can be null if this map is not row-ordered (or throw an exception)
        const float* GetColumnValues(size_t i);
        /// \brief Returns pointer to entire data 
        const float* GetSurfaceValues();
        /// \brief Associate a formation with this map
        void SetFormation(const Formation* formation);
        /// \brief Get the associated formation for this map
        const boost::shared_ptr<const Formation> GetFormation() const;
        /// \brief Convenience function to get an index into the 1D volume data 
        size_t GetMapIndex(size_t i, size_t j) const;

    private:
        typedef size_t (Map::*ComputeIndexFn)(size_t i, size_t j) const;
        ComputeIndexFn _indexCalc;
        size_t ComputeIndex_IJ(size_t i, size_t j) const;
        void SetData(float* data, bool setValue = false, float value = 0);
        float* _internalData;
        boost::shared_ptr<const Formation> _formation;
    };

    /// \brief A continuous volume is continuous across formation boundaries. It can still be (legacy) node centered as well as cell centered.
    class Volume : public Grid
    {
    public:
        /// \brief Constructor
        Volume(bool cellCentered, float undefined);
        ~Volume();

        /// \brief Assign data to the volume as a 1D array: K fastest, then I, then J
        void SetData_KIJ(float* data, bool setValue = false, float value = 0);
        /// \brief Assign data to the volume as a 1D array: I fastest, then J, then K
        void SetData_IJK(float* data, bool setValue = false, float value = 0);

        // Geometry

        /// \brief assign a geometry
        void SetGeometry(size_t numI, size_t numJ, size_t numK, 
            double deltaI, double deltaJ, 
            double minI, double minJ, double minK, 
            double maxI, double maxJ, double maxK);

        /// \brief return the local vertical resolution
        size_t GetNumK() const;
        double GetDeltaK() const;
        double GetMinK() const;
        double GetMaxK() const;

        // Values

        /// \brief Returns true if data is represented per row, for constant k
	    bool CanGetRow() const;
	    /// \brief Returns true if data is represented per column, for constant k
	    bool CanGetColumn() const;
	    /// \brief Returns true if data is represented per needle, for constant ij
        bool CanGetNeedle() const;
	    /// \brief Returns true if data is represented per surface, for constant k
        bool CanGetSurface() const;
	    /// \brief 
	    bool IsUndefined(size_t i, size_t j, size_t k) const;
	    /// \brief 
	    float GetValue(size_t i, size_t j, size_t k) const;
	    /// \brief Gets an entire row; can be null if this volume is not row-ordered (or throw an exception)
	    const float* GetRowValues(size_t j, size_t k);
	    /// \brief Gets an entire column; can be null if this volume is not row-ordered (or throw an exception)
	    const float* GetColumnValues(size_t i, size_t k);
        /// \brief Gets an entire needle; can be 
        const float* GetNeedleValues(size_t i, size_t j);
	    /// \brief Returns pointer to entire data 
        const float* GetSurface(size_t k);
	    /// \brief Returns pointer to entire data 
	    const float* GetVolumeValues();
        /// \brief Convenience function to get an index into the 1D volume data 
        size_t GetVolumeIndex(size_t i, size_t j, size_t k) const;

    private:
        typedef size_t(Volume::*ComputeVolIndexFn)(size_t i, size_t j, size_t k) const;
        size_t ComputeIndex_IJK(size_t i, size_t j, size_t k) const;
        size_t ComputeIndex_KIJ(size_t i, size_t j, size_t k) const;
        ComputeVolIndexFn _indexCalc;
        void SetData(float* data, bool setValue = false, float value = 0);
        float* _internalData;
        size_t _numK;
        double _minK, _maxK, _deltaK;
    };

    /// \brief This is a legacy volume. It is node centered and therefore, most properties are discontinuous across formation boundaries
    class DiscontinuousVolume : public Grid
    {
    public:
        /// \brief Create a new discontinuous volume by providing the first subvolume
        DiscontinuousVolume(const Formation* formation, const Volume* volume, float undefined);
        ~DiscontinuousVolume();
        /// \brief This type of volume is NOT cell centered
        virtual bool IsCellCentered() const { return false; }
        /// \brief Adds a volume to the list: -- OWNERSHIP of the volume is transfered to this class --
        void AddVolume(const Formation* formation, const Volume* volume);
        /// \brief Get the entire list of (sub)volumes
        const boost::shared_ptr<FormationVolumeList> GetVolumeList() const;

        /// Overrides
        //////////////////////////////////////////////////////////////////////////

        /// \brief returns true if the value is constant for the entire grid
        virtual bool IsConstant() const;
        /// \brief Set a constant value for this Grid
        virtual void SetConstantValue(float value);
       
    private:
        boost::shared_ptr<FormationVolumeList> _volumeList;
    };
}

#endif