##########################################################################
# $Header: /nfs/rvl/users/ibs/RCS/./EPT-HM/BB/RubyPropModel/BasinModellerProperty.Spec,v 26.15 2006/11/08 13:46:44 nlwse0 Stable $
##########################################################################
#
#    Property specification file for the Ruby 1D/3D project
#    Created June 10 1994
#    compiled by Lidia Lonergen
#    
##########################################################################

START-application-attributes

    Identification      : Ruby
    Comment		: No
    Parsing             : No
    Pre-conditions      : Yes     
    Post-conditions     : Yes     
    Class-invariants    : Yes
    Persistency         : TimeDependentOnly
    Property I/O        : Yes
    DynamicInterface    : Yes

END-application-attributes

##########################################################################

START-properties

#### General data --------------------------------------------------------

    Identification      : Age
    Description         : Geological age of an event, entry in a table etc. 
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : StartAge
    Description         : Geological start age of a fault event, entry in the PressureFaultcutIoTbl
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : EndAge
    Description         : End of some particular geological event
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Description
    Description         : string to further specify a project, lithotype, etc.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Label
    Description         : A piece of text attached to a measurement
                          in any calibration data type
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DefinedBy
    Description         : Person that defined the lithotype, etc.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Selected
    Description         : Whether or not it is selected
    Unit of measurement : 
    Type                : boolean
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UserDefined
    Description         : May be updated by the local user if 1
    Unit of measurement : 
    Type                : boolean
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DefinitionDate
    Description         : date at which the the lithtotype etc is created
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LastChangedBy
    Description         : Person that last changed the lithotype, etc.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LastChangedDate
    Description         : date at which the the lithtotype etc is last changed 
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent
    Description         : volume fraction in percentage (at STP)
    Unit of measurement : volume%
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent1
    Description         : volume fraction of lithotype1 in rock matrix at STP
    Unit of measurement : volume%
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent1Grid
    Description         : volume fraction of lithotype1 in rock matrix at STP
    Unit of measurement : volume%
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent2
    Description         : volume fraction of lithotype2 in rock matrix at STP
    Unit of measurement : volume%
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent2Grid
    Description         : volume fraction of lithotype2 in rock matrix at STP
    Unit of measurement : volume%
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percent3
    Description         : volume fraction of lithotype3 in rock matrix at STP
    Unit of measurement : volume% 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyName
    Description         : Name of property 
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyClass
    Description         : Type of property {Physical, Formation, FormationTop,
                          FormationBottom, Source rock}
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Time
    Description         : Snapshot time of a property value
    Unit of measurement : Ma
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : IsMinorSnapshot
    Description         : 0 if major, 1 if minor
    Unit of measurement :
    Type                : integer
    Default             : 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Depth_
    Description         : The depth for a property
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Average
    Description         : Average property value
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : StandardDev
    Description         : Standard Deviation
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Minimum
    Description         : Minimum property value
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Maximum
    Description         : Maximum property value
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Sum
    Description         : Sum of the property values for all iterations
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Sum2
    Description         : Sum of (property values * property values)
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NP
    Description         : Number of property values
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 12-11-1997
    # Increment 4.4
    Identification      : P15
    Description         : P15 distribution percentile
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 12-11-1997
    # Increment 4.4
    Identification      : P50
    Description         : P50 distribution percentile
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 12-11-1997
    # Increment 4.4
    Identification      : P85
    Description         : P85 distribution percentile
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : SumFirstPower
    Description         : Mean
    Unit of measurement :  
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : SumSecondPower
    Description         : Sum of second power about mean
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : SumThirdPower
    Description         : Sum of third power about mean
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : SumFourthPower
    Description         : Sum of fourth power about mean
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : Skewness
    Description         : Degree of asymmetry of distribution
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No 

    # Added by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.6
    Identification      : Kurtosis
    Description         : Degree of peakedness of distribution
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RunNumber
    Description         : Monte Carlo analysis run number
                          Used in DepthMCResultIoTbl and TimeMCResultIoTbl
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Value
    Description         : In a Monte Carlo run calculated property value
                          Used in DepthMCResultIoTbl and TimeMCResultIoTbl
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyGrid
    Description         : MapName of the output grid
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Filename
    Description         : Filename within extension
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Fileextension
    Description         : Extension of a filename
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Dataset
    Description         : Logical name to identify dataset
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#    Identification      : Xlocation
#    Description         : x coordinate of a point in the basinmodel
#    Unit of measurement : m
#    Type                : float
#    Error Field         : No 
#    Time dependent      : No
#    Property I/O        : No

#    Identification      : Ylocation
#    Description         : y coordinate of a point in the basinmodel
#    Unit of measurement : m
#    Type                : float
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No

    Identification      : BricksizeEast
    Description         : Bricksize in model East direction in 1D modelling mode
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BricksizeNorth
    Description         : Bricksize in model North direction in 1D modelling mode
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PrefSnapstep
    Description         : Prefered step between two snapshot times 
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 10.0

    Identification      : PrefReconstep
    Description         : Initial temperature calculation step
                          at the start of a Snapstep
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.1

    Identification      : OptimalTotalPresDiff
    Description         : Optimal maximum pressure difference within
                          a calculation step for the whole basin
    Unit of measurement : MPa
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.1

    Identification      : OptimalTotalTempDiff
    Description         : Optimal maximum temperature difference within
                          a calculation step for the whole basin
    Unit of measurement : DegreesCelcius
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 4.0

    Identification      : OptimalSourceRockTempDiff
    Description         : Optimal maximum temperature difference within
                          a calculation step for the source rock only
    Unit of measurement : DegreesCelcius
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 2.0

    Identification      : MaxTimeStepIncreaseFactor
    Description         : Max factor where the time step may be
                          increased with (per time step)
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1.5

    Identification      : MinTimeStepDecreaseFactor
    Description         : Min factor where the time step may be
                          descreased with (per time step)
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.5

#### Project data --------------------------------------------------------

    Identification      : ProjectName
    Description         : Name of whole Project
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProjectSize
    Description         : Size of the project in relation to the
                          size of the machine. Possible values:
			  "" or "Small" -> The whole modelling can
			  be run within virtual memory
			  "Big" Only a very limitted number of
			  snapshots can be held within virtual memory
			  This field is a 3D only field.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProjectNotes
    Description         : text field where user can write notes about project
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProjectTeam
    Description         : People involved in the Project
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProjectStartDate
    Description         : date of start of project
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProjectEndDate
    Description         : date of end of project
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#    Identification      : BPAServer
#    Description         : BPA server to which this project has been uploaded
#    Unit of measurement :
#    Type                : string
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No

    Identification      : BPAKey
    Description         : key into BPA database
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#    Identification      : BPAChecksum
#    Description         : Hexadecimal checksum of the uploaded project file
#    Unit of measurement :
#    Type                : string
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No
#    Identification      : BPAName
#    Description         : BPA Name under which this project has been uploaded
#    Unit of measurement :
#    Type                : string
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No
#    Identification      : BPAOwnerOrg
#    Description         : BPA OwnerOrg to which this project belongs
#    Unit of measurement :
#    Type                : string
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No
    Identification      : PTCouplingMode
    Description         : One of "Hydrostatic", "LooselyCoupled", "ItCoupled"
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WorkflowType
    Description         : One of "StandardCauldron", "DarcyFlow"
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WktGeometryOutline
    Description         : An XML fragment with an entity name of <WktGeometryOutline> defined by the BPA <WktGeometry> class
                          containing three entities with names as follows:
                          <CoordinateOperationName> contains the identifier for the transform of the Crs to/from WGS84
                          <SrsName> contains the identifier of the Crs, the Shell Geodetic Registry urn
                          <Wkt> contains the well known text description of the project outline polygon, if it exists.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ProgramVersion
    Description         : version of Ruby basin modelling program used
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ModellingMode
    Description         : Allowed values: "1d", "3d" or "Multiple 1d"
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OutputOption
    Description         : Allowed values for the "DepthIoTbl":
                          and for the "DepthMCResultIoTbl":
                          "None", "SedimentsOnly" or "SedimentsPlusBasement".
                          Allowed values for the "TimeIoTbl":
                          and for the "TimeMCResultIoTbl":
                          "None", "SourceRockOnly", "SedimentsOnly" or
			  "SedimentsPlusBasement".
			  This field specifies the level of detail at 
                          which property values are saved. "None" means
			  that no property value of this type will be
			  saved. The other options mean that property
			  values will be saved for the source rock
			  layer(s), the sediment layers or
			  sediment + basement layers.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    # Added by S. Mathijssen (ORIGIN) 05-11-1997
    # Increment 1.2
    Identification      : ResultOption
    Description         : Allowed values for the "DepthIoTbl",
                          for the "DepthMCResultIoTbl", for the
                          "TimeIoTbl", and for the "TimeMCResultIoTbl":
                          "Simple" and "Full".
			  "Simple" means average, minimum, maximum, standard
                          deviation, sum, sum^2, NP, and Skewness and Kurtosis.
                          "Full" means all MC results, P15, P50 and P85, and
                          all simple distribution results. 
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumberX
    Description         : Number of nodes in the X direction (Easting)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 2

    Identification      : NumberY
    Description         : Number of nodes in the Y direction (Northing)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 2
    
    Identification      : NumberZ
    Description         : Number of nodes in the Z direction
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 2
    
    Identification      : ScaleX
    Description         : Division factor in the X direction (Easting)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 1

    Identification      : ScaleY
    Description         : Division factor in the Y direction (Northing)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 1

    Identification      : ElementRefinementZ
    Description         : Refinement of elements in the z-direction for each formation.
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 1

    Identification      : OffsetX
    Description         : Offset of nodes in the X direction (Easting)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : OffsetY
    Description         : Offset of nodes in the Y direction (Northing)
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0
    
    Identification      : DeltaX
    Description         : Same as attribute "BricksizeEast"
    Unit of measurement : m
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 1

    Identification      : DeltaY
    Description         : Same as attribute "BricksizeNorth"
    Unit of measurement : m
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 1

    Identification	: WindowXMin
    Description         : Lower X index for Grid Window
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: -1

    Identification	: WindowXMax
    Description         : Upper X index for Grid Window
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: -1

    Identification	: WindowYMin
    Description         : Lower Y index for Grid Window
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: -1

    Identification	: WindowYMax
    Description         : Upper Y index for Grid Window
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: -1

#### Well Location data ---------------------------------------------------

    Identification      : WellName
    Description         : Name of well
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Datum
    Description         : The surface from which all depths in the model are
                          measured. It can be above or at the same depth as
                          the top_surface (water/sediment or air/sediment 
                          interface), at which the top boundary condition 
                          is applied. The datum can be derrick floor,sea level,
                          sea bottom or ground level.
    Unit of measurement : 
    Type                : string 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : XCoord
    Description         : grid x coordinate of a well
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : YCoord
    Description         : grid Y coordinate of a well
    Unit of measurement : m
    Type                : double
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : ZCoord
    Description         : z (depth) coordinate
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : SpillPointXCoord
    Description         : grid x coordinate of a trap spill point
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : SpillPointYCoord
    Description         : grid Y coordinate of a trap spill point
    Unit of measurement : m
    Type                : double
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : BHFName
    Description         : Name of burial history file in output directory
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : XLeft
    Description         : bottom left of a grid
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : YBottom
    Description         : bottom left of a grid
    Unit of measurement : m
    Type                : double
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No
    Default		: 0

    Identification      : Latitude
    Description         : Latitude of a well
    Unit of measurement : degs,minutes
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Longitude
    Description         : Longitude of a well
    Unit of measurement : degs,minutes
    Type                : float
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No

    Identification      : DerFloorElev
    Description         : Derrick floor elevation in a well (may be datum for 
                          horizon depth measurements)
    Unit of measurement : m 
    Type                : float
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No

    Identification      : WellElevation
    Description         : elevation above sea level for a well 
    Unit of measurement : m 
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Waterdepth
    Description         : Water depth in a well (may be datum for horizon depth 
                          measurements, if sealevel chosen as datum)
    Unit of measurement : m 
    Type                : float
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No

#### Grid Map properties ---------------------------------------------------

    Identification      : ReferredBy
    Description         : Name of the table which refer to this map,
                          e.g. StratIoTbl, ...
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MapName
    Description         : Name of the grid map. Will be refered by one or
                          more *Grid attributes (logical link).
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MapType
    Description         : Type of the grid map, possible values are:
                            "DECASCII", "DECBINARY", "ZYCOR", "CPS3",
			    "EPIRUS", "XYZ"
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SnapshotFileName
    Description         : Filename of the snapshot file
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TypeOfSnapshot
    Description         : Type of snapshot, possible values:
                          "System Generated" is a snapshot needed for 
			  for the decompaction. This type of snapshot
			  may not be deleted by the user
                          "User Defined" --> is a snapshot added by
			  the user.
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MapFileName
    Description         : Filename of the grid map (with extension)
    Unit of measurement : 
    Type                : string
    Default		: ""
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MapSeqNbr
    Description         : Sequence number of the grid map, within the
                          grid loader (Starting with 0). This attribute
			  is only relevant for multiple map files.
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HDF5FileName
    Description         : Filename of the grid map (with extension)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DataSetName
    Description         : DataSet name in the file
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GroupName
    Description         : Group name in the file
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FileId
    Description         : inode number identifying the file
    Unit of measurement : 
    Type                : integer
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MD5Checksum
    Description         : Hexadecimal checksum of the file specified by HDF5FileName
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BPAPresence
    Description         : Whether this map has been uploaded to BPA
    Unit of measurement : 
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#### Geology / Stratigraphic column properties -----------------------------
# surface/ formation properties:

    Identification      : SurfaceName
    Description         : Name of surface (horizon or other) (input by user)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BottomFormationName
    Description         : Name of surface (horizon or other) (input by user)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FormationName
    Description         : Name of formation (input by user)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NodeIndex
    Description         : Index of the node in the formation
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DepoAge
    Description         : Age of deposition of a horizon/formation
    Unit of measurement : Ma
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

#    Identification      : ErosionAge
#    Description         : Age of erosion of a horizon/formation
#    Unit of measurement : s (to User Ma)
#    Type                : float
#    Error Field         : Yes
#    Time dependent      : No
#    Property I/O        : No

#    Identification      : ErosionAgeVec2
#    Description         : Age of erosion  of a top and bottom of formation
#    Unit of measurement : s (to User Ma)
#    Type                : floatvector 2
#    Error Field         : Yes
#    Time dependent      : No
#    Property I/O        : No

    Identification      : TwoWayTtime
    Description         : one way travel time (two-way-travel time/2)
    Unit of measurement : s
    Type                : float 
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

# layer properties:

    Identification      : LayerName
    Description         : Name of layer (formation or other) (input by user)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DummyLayerIndicator
    Description         : TRUE if layer is a dummy layer, FALSE otherwise
    Unit of measurement : 
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DepoSequence
    Description         : depositional sequence of formations from model
                          bottom up. For the TimeIoTbl:
			  1000*deposeg_layer + deposeq_surface
    Unit of measurement :
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Lithotype
    Description         : name of rock type used to identify a set of material 
                          properties of that rock type
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Fluidtype
    Description         : name of fluid type used to identify a set of 
                          material properties of that fluid type (In the
                          first version we only have one fluid = water)
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : HeatCaptype
    Description         : name of fluid type used to identify a set of 
                          material properties of that fluid type (In the
                          first version we only have one fluid = water)
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : ThermCondtype
    Description         : name of fluid type used to identify a set of 
                          material properties of that fluid type (In the
                          first version we only have one fluid = water)
     Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : Denstype
    Description         : name of fluid type used to identify a set of 
                          material properties of that fluid type (In the
                          first version we only have one fluid = water)
     Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Viscotype
    Description         : name of fluid type used to identify a set of 
                          material properties of that fluid type (In the
                          first version we only have one fluid = water)

    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
   
    Identification      : FaultLithology
    Description         : name of lithology type used to model faults
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Lithotype1
    Description         : name of first rock type used to identify a set of 
                          material properties of that rock type
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Lithotype2
    Description         : name of second rock type used to identify a set of
                          material properties of that rock type
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Lithotype3
    Description         : name of third rock type used to identify a set of
                          material properties of that rock type
    Unit of measurement :  
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MixModel
    Description         : Model used for mixing the different lithotypes to a
                          lithology; either "Homogeneous" or "Layered", could
                          be extended number of cycles (when layered)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourceRock
    Description         : Flag to indicate whether a layer is a Source Rock
			  within the model
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MobileLayer
    Description         : Flag to indicate whether a layer is a Mobile Layer
			  within the model (i.e. salt or ignious intrusion)
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HasAllochthonLitho
    Description         : Flag to indicate whether a layer has an allochthonous lithology
			  within the model (i.e. salt or ignious intrusion)
    Type                : boolean
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HydroSand
    Description         : Flag to indicate whether a layer has hydrostatic or overpressure constraint
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : IncludeInDarcy
    Description         : Flag to indicate whether a layer should be included in the Darcy subdomain.
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0


    Identification      : IsIgneousIntrusion
    Description         : Flag to indicate whether a layer is an igneous intrusion or not.
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : false

    Identification      : IgneousIntrusionAge
    Description         : The age at which the intrusion is inflated
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : -9999.0

    Identification      : IgneousIntrusionTemperature
    Description         : The initial temperature of the intruted material,
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : -9999.0




#### Source Rock properties -------------------------------------------------

    Identification      : SourceRockType
    Description         : default source rock type
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TocIni
    Description         : Initial TOC
    Type                : float
    Unit of measurement : weight %
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : TocIniGrid
    Description         : Initial TOC
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : S1Ini
    Description         : S1 initial value: The mass of hydrocarbons retained 
                          per mass of rock.
    Type                : float
    Unit of measurement : kg/tonne
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : S1IniGrid
    Description         : S1 initial value: The mass of hydrocarbons retained 
                          per mass of rock.
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : S2Ini
    Description         : S2 initial value: The remaining generative potential 
                          of the rock which has not yet been transformed into 
			  hydrocarbons
    Type                : float
    Unit of measurement : kg/tonne
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : S2IniGrid
    Description         : S2 initial value: The remaining generative potential 
                          of the rock which has not yet been transformed into 
			  hydrocarbons
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : S3Ini
    Description         : S3 initial value: Rough measurements of the oxygen 
                          content of the organic matter
    Type                : float
    Unit of measurement : kg/tonne
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : S3IniGrid
    Description         : S3 initial value: Rough measurements of the oxygen 
                          content of the organic matter
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HCInputMode
    Description         : Method of inputting the H/C (possible values are "HC", "HI" or "S2")
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcIni
    Description         : H/C initial ratio
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No
    Default             : 1.56

    Identification      : HcVRe05
    Description         : H/C initial ratio. H/C at VRe 0.5.
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1.56

    Identification      : ScVRe05
    Description         : S/C initial ratio. S/C at VRe 0.5.
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : HiIni
    Description         : HI initial ratio
    Type                : float
    Unit of measurement : kg/tonne
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No
    Default             : 773.5

    Identification      : HcIniGrid
    Description         : H/C initial ratio
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OcIni
    Description         : O/C initial ratio
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : OcIniGrid
    Description         : O/C initial ratio
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NcIni
    Description         : N/C initial ratio
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : NcIniGrid
    Description         : N/C initial ratio
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CharLength
    Description         : Characteristic length of the source Rock. 
			  Measurement for the ease of pimary migration
    Type                : float
    Unit of measurement : m
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CharLengthGrid
    Description         : Characteristic length of the source Rock. 
			  Measurement for the ease of pimary migration
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UpperBiot
    Description         : Biot number for the top boundary of the source rock.
			  A value to indicate the seal/carrier capacity of the
			  top of source rock/lithology boundary
    Type                : float
    Unit of measurement : 
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : UpperBiotGrid
    Description         : Biot number for the top boundary of the source rock.
			  A value to indicate the seal/carrier capacity of the
			  top of source rock/lithology boundary
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LowerBiot
    Description         : Biot number for the bottom boundary of the source rock.
			  A value to indicate the seal/carrier capacity of the
			  bottom of source rock/lithology boundary
    Type                : float
    Unit of measurement : 
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : LowerBiotGrid
    Description         : Biot number for the bottom boundary of the source rock.
			  A value to indicate the seal/carrier capacity of the
			  bottom of source rock/lithology boundary
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : KerogenStartAct
    Description         : The activation energy limit for which the kerogen
			  cracking starts
    Type                : float
    Unit of measurement : kJ/mol
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : KerogenStartActGrid
    Description         : The activation energy limit for which the kerogen
			  cracking starts
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : KerogenEndAct
    Description         : The activation energy limit where the kerogen
			  cracking ends
    Type                : float
    Unit of measurement : kJ/mol
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : KerogenEndActGrid
    Description         : The activation energy limit where the kerogen
			  cracking ends
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PreAsphaltStartAct
    Description         : The activation energy limit for which the PreAsphalt
			  cracking starts
    Type                : float
    Unit of measurement : kJ/mol
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No
    Default             : 216.0

    Identification      : PreAsphaltStartActGrid
    Description         : The activation energy limit for which the PreAsphalt
			  cracking starts
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PreAsphaltEndAct
    Description         : The activation energy limit where the PreAsphalt
			  cracking ends
    Type                : float
    Unit of measurement : kJ/mol
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : PreAsphaltEndActGrid
    Description         : The activation energy limit where the PreAsphalt
			  cracking ends
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NettThickIni
    Description         : Netto initial thickness of the source rock. This
                          is defined as the gross - non-source rock
    Type                : float
    Unit of measurement : m
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : NettThickIniGrid
    Description         : Netto initial thickness of the source rock. This
                          is defined as the gross - non-source rock
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NGenexTimeSteps
    Description         : Number of time steps to use in the Genex generation
                          and expulsion calculations
    Type                : smallint
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NGenexTimeStepsGrid
    Description         : Number of time steps to use in the Genex generation
                          and expulsion calculations
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NGenexSlices
    Description         : Number of slices in which the source rock will be 
                          modelled with the Genex generation and expulsion 
			  calculations
    Type                : smallint
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NGenexSlicesGrid
    Description         : Number of slices in which the source rock will be 
                          modelled with the Genex generation and expulsion 
			  calculations
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VREini
    Description         : Vitrinite reflectance of the source rock                      
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default		: 0.5
 
    Identification      : AsphalteneDiffusionEnergy
    Description         : diffusion energy for asphaltne 
    Type                : float
    Unit of measurement : kJ
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 88.0

    Identification      : ResinDiffusionEnergy
    Description         : diffusion energy for resin 
    Type                : float
    Unit of measurement : kJ
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 85.0

    Identification      : C15AroDiffusionEnergy
    Description         : diffusion energy for C15+ Aromatics 
    Type                : float
    Unit of measurement : kJ
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 80.0

    Identification      : C15SatDiffusionEnergy
    Description         : diffusion energy for C15+ Saturates  
    Type                : float
    Unit of measurement : kJ
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 75.0

    Identification      : VREoptimization
    Description         : Flag for enabling the VRE optimization which states that the Genex5 kernel should not perform any computation at points with VRE at present day below the VREthreshold      
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "No"

    Identification      : VREthreshold
    Description         : Value to be used if VREoptimization equals "Yes"                     
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.5

    Identification      : VESLimitIndicator
    Description         : Flag for enabling the VES limit used in hydrostatic calculations. Admissible values are "Yes" and "No".
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "No"

    Identification      : VESLimit
    Description         : Value of VES to be used for a hydrostatic calculation, if VESLimitIndicator equals "Yes"
    Type                : float
    Unit of measurement : MPa
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 20.0

    Identification      : EnableSourceRockMixing
    Description         : Whether to perform Source Rock mixing
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : SourceRockType1
    Description         : First Source Rock Type end member
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : ""

    Identification      : BaseSourceRockType
    Description         : Name of predefined SourceRock type
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : ""

    Identification      : SourceRockType2
    Description         : Second Source Rock Type end member
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : ""

    Identification      : SourceRockMixingHI
    Description         : Value of uniform HI scalar value
    Type                : float
    Unit of measurement : kg/tonne C
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : SourceRockMixingHIGrid
    Description         : Map of HI values
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourceRockEndMember1
    Unit of measurement : %
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourceRockEndMember2
    Unit of measurement : %
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

;    Identification      : DepoAgeIni
;    Description         : Initial deposition age of the source rock.
;    Type                : float
;    Unit of measurement : m
;    Error Field         : Yes
;    Time dependent      : No
;    Property I/O        : No

;    Identification      : Tmaxi
;    Description         : Maximum temperature to use
;                          NOT USED (YET) BY GENEX4 KERNEL
;    Type                : float
;    Unit of measurement : 
;    Error Field         : Yes
;    Time dependent      : No
;    Property I/O        : No

;    Identification      : FractIni
;    Description         : Fractionation initialisation
;                          NOT USED (YET) BY GENEX4 KERNEL
;    Type                : float
;    Unit of measurement : 
;    Error Field         : Yes
;    Time dependent      : No
;    Property I/O        : No

;    Identification      : FractFact
;    Description         : Fractionation factor alpha
;                          NOT USED (YET) BY GENEX4 KERNEL
;    Type                : float
;    Unit of measurement : 
;    Error Field         : Yes
;    Time dependent      : No
;    Property I/O        : No


 #### Geometry properties -------------------------------------------------

    Identification      : Depth
    Description         : depth with respect to datum
    Unit of measurement : m
    Type                : float
    Error Field         : No  
    Time dependent      : Yes
    Property I/O        : No

    Identification      : ErrDepth
    Description         : Error in Depth
    Unit of measurement : m
    Type                : float
    Error Field         : No  
    Time dependent      : Yes
    Property I/O        : No


    Identification      : DepthGrid
    Description         : MapName of the "Depth" property
    Unit of measurement : m
    Type                : string
    Property I/O        : No

    Identification      : DepthIndex
    Description         : depth with respect to datum used as index in tables
    Unit of measurement : m
    Type                : float
    Error Field         : No  
    Time dependent      : No
    Property I/O        : No

    Identification      : Thickness
    Description         : Formation thickness 
    Unit of measurement : m
    Type                : float
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : SpillDepth
    Description         : depth at which a trap spills
    Unit of measurement : m
    Type                : float
    Error Field         : No  
    Time dependent      : Yes
    Property I/O        : No

    Identification      : CalibThickness
    Description         : Calibrated value for 'Thickness'
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimThickness
    Description         : 0 = No optimization
                          1 = Optimize formation thickness
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ThicknessGrid
    Description         : MapName of the "Thickness" property
    Unit of measurement : m
    Type                : string

    Identification      : DistributionGrid
    Description         : MapName of the distribution of the allochthonous lithology
    Unit of measurement : 
    Type                : string

    Identification      : FullCompThickness
    Description         : Formation fully compacted thickness 
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BrickHeightSediment
    Description         : Maximum present day brick height of the bricks 
                          in the sedimentary layers, also used for the depth
			  interval of the output results
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

;    Identification      : BrickHeightBasement
;    Description         : Maximum present day brick height of the 
;                          bricks in the basement layers (superseded by 
;			  BrickHeightCrust and BrickHeightMantle)
;    Unit of measurement : m
;    Type                : float
;    Error Field         : No
;    Time dependent      : No
;    Property I/O        : No

    Identification      : BrickHeightCrust
    Description         : Maximum present day brick height of the 
                          bricks in the crust layers 
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 4000.0

    Identification      : BrickHeightMantle
    Description         : Maximum present day brick height of the 
                          bricks in the manlte layers 
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 20000.0

    Identification      : MaxNumberOfRunOverpressure
    Description         : Maximum Number of Overpressure runs
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 10

    Identification      : FracturePressureFunction
    Description         : Fracture-pressure funtion to be used
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "None        "

    Identification      : FractureType
    Description         : Fracture-pressure Function Type to be used
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "None"

    Identification      : FractureModel
    Description         : Fracture-pressure model type to be used
    Unit of measurement : 
    Type                : smallint
    Default		: 1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CalcAllDepthIoProps
    Description         : Calculate all DepthIoProps through time and store
                          them in the DepthIoTbl --> 1
                          Only calculate and store the present day DepthIoProps
			  and store them in the DepthIoTbl <> 1
    Unit of measurement : 
    Type                : smallint
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LithoMantleThickness
    Description         : Lithospheric Mantle thickness 
    Unit of measurement : m
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CalibLithoMantleThickness
    Description         : Calibrated value for 'LithoMantleThickness'
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InitialLthMntThickns
    Description         : Initial Litho Mantle thickness 
    Unit of measurement : m
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CalibInitialLthMntThickns
    Description         : Calibrated value for 'InitialLthMntThickns'
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FixedCrustThickness
    Description         : Fixed Crust Thickness 
    Unit of measurement : m
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CalibFixedCrustThickness
    Description         : Calibrated value for 'FixedCrustThickness'
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InitialLithosphericMantleThickness
    Description         : Initial Lithospheric Mantle thickness 
    Unit of measurement : m
    Type                : float
    Default		: 115000
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : CrustPropertyModel
    Description         : Crust physical properties type
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "Low Conductivity Crust"
    
    Identification      : MantlePropertyModel
    Description         : Mantle physical properties type
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "Standard Conductivity Mantle"
    
    Identification      : BasaltThickness
    Description		: present-day basalt thickness
    Unit of measurement : m
    Type                : float
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification 	: BasaltThicknessGrid
    Description    	: MapName of the Basalt Thickness
    Unit of measurement : 
    Type                : string
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No
    
    Identification      : CrustThicknessMeltOnset
    Description		: crust thickness at melt onset
    Unit of measurement : m
    Type                : float
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification 	: CrustThicknessMeltOnsetGrid
    Description    	: MapName of the Crust Thickness at melt onset
    Unit of measurement : 
    Type                : string
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No
    
    Identification      : SurfaceNormal
    Description         : Normal of a point on a surface
    Unit of measurement : 
    Type                : floatvector 3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FaultcutsMap
    Description         : MapName of the "Faultcuts" property
    Unit of measurement : m
    Type                : string
    Property I/O        : No

    Identification      : FaultName
    Description         : Name of the Fault
    Unit of measurement : 
    Type                : string
    Property I/O        : No

    Identification      : FaultcutStatus
    Description         : Status indicator of the faultcut
    Unit of measurement : 
    Type                : string
    Property I/O        : No

    Identification      : UsedInOverpressure
    Description         : Whether the fault is used in overpressure or not
    Unit of measurement : 
    Type                : smallint
    Property I/O        : No
    Default             : 0

    Identification      : AgeSeqNbr
    Description         : Age sequence number in reconstruction of faultcuts
    Unit of measurement : 
    Type                : smallint
    Property I/O        : No

#### Rock (lithotype and fluidtype matrix and bulk) volume properties --------

    Identification      : FgColour
    LastChangedDate
    Description         : foreground colour, used to  display the lithotype
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BgColour
    LastChangedDate
    Description         : foreground colour, used to display the lithotype
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Pixmap
    LastChangedDate
    Description         : Pixmap, used to display the lithotype
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Density
    Description         : density of the basic lithotype, fluidtype, matrix,
                          fluid or bulk at STP (until TExpansionCoef is taken
                          into account)
    Unit of measurement : kg/m3
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : Salinity
    Description         : Salinity of the basic fluidtype,fluid
    Unit of measurement : ppm
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DensModel
    Description         : Densoty Model for the fluidtype,fluid
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : SeisVelocityModel
    Description         : SeisVelocityModel Model for the fluidtype,fluid
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DensXHeatCap
    Description         : density x Heat Capacity 
    Unit of measurement : J/m3K
    Type                : float
    Error Field         : No  (only in calculation)
    Time dependent      : No
    Property I/O        : No

    Identification      : DepoPerm
    Description         : The depositional permeability (K0) in milli Darcy
    Unit of measurement : mD
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : PermDecrStressCoef
    Description         : User defined dimensionless coefficient (q) controlling
                          the decrease in permeability with stress.
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PermIncrRelaxCoef
    Description         : User defined dimensionless coefficient (q*) controlling
                          the increase in permeability on unloading
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PermMixModel
    Description         : Mixing model for permeability
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No



    Identification      : Porosity_Model
    Description         : Method of porosity used in computation (exponential or soil mechanics)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "Exponential"

    Identification      : Compaction_Coefficient_SM
    Description         : Compaction coefficient used in soil mechanics porosities, ...
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0



    Identification      : Number_Of_Data_Points
    Description         : Number of data points
    Type                : smallint
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Multipoint_Porosity
    Description         : array of characters contining the porosities
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Multipoint_Permeability
    Description         : array of characters contining the permeabilities
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : Viscosity
    Description         : The ease in which fluid will flow in Pascal times Second
    Unit of measurement : PaS
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
#    Identification      : TExpansionCoef
#    Description         : coefficent of thermal expansion
#    Unit of measurement : 10-5C-1 
#    Type                : float
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No

    Identification      : HeatCapacity
    Description         : Specific heat capacity of lithotype and fluidtypes
                          (temperature dependent, see tables), of matrix,
                          fluids and bulk
    Unit of measurement : J/kgK 
    Type                : float
    Error Field         : No 
    Time dependent      : No 
    Property I/O        : No

    Identification      : ThCond
    Description         : Thermal conductivity (vertical)
    Unit of measurement : W/mK
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : ThCondVec2
    Description         : Thermal conductivity  
    Unit of measurement : W/mK
    Type                : floatvector 2
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : PermeabilityVec2
    Description         : Permeability
    Unit of measurement : mD
    Type                : floatvector 2
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : DiffusivityVec2
    Description         : Diffusivity: ThCond/DensityXHeatCap  
    Unit of measurement : 
    Type                : floatvector 2
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HeatFlow_
    Description         : Heat flow (calculated)
    Unit of measurement : mW/m2
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HeatFlowVec8
    Description         : Heat flow (calculated), corner fluxes as calculated
                          by the solver one value for each corner of a brick
    Unit of measurement : mW/m2
    Type                : doublevector 8
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : HeatFlowX
    Description         : Horizontal Heat flow (calculated)
    Unit of measurement : mW/m2
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HeatFlowY
    Description         : Horizontal Heat flow (calculated)
    Unit of measurement : mW/m2
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HeatFlowZ
    Description         : Vertical Heat flow (calculated)
    Unit of measurement : mW/m2
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : FluidVelocityX
    Description         : Horizontal fluid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : FluidVelocityY
    Description         : Horizontal fluid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : FluidVelocityZ
    Description         : Vertical fluid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No


    Identification      : IsoStaticMass
    Description         : isostatic mass of the sediments plus basement
    Unit of measurement : kg/m3
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : IsoStaticDeflection
    Description         : isostatic deflection of the sediments plus basement
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : IsoStaticWaterBottom
    Description         : isostatic water bottom depth derived from the present day
                          basement depth and the deflection corrected with sediment height
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : StpThCond
    Description         : (Measured) Thermal conductivity of a bulk lithotype 
                          (perpendicular to bedding) at a STP (standard
                          temperature  and pressure) (and porosity 0 ??????)
    Unit of measurement : W/mK 
    Type                : float 
    Error Field         : Yes
    Time dependent      : No 
    Property I/O        : No

    Identification      : ThCondAnisotropy
    Description         : Ratio of matrix thermal conductivity parallel
                          to bedding over conductivity perpendicular to
                          bedding (kh/kv)
    Unit of measurement : (ratio)
    Type                : float 
    Error Field         : Yes
    Time dependent      : No 
    Property I/O        : No

    Identification      : PermAnisotropy
    Description         : Ratio of matrix permeability parallel
                          to bedding over conductivity perpendicular to
                          bedding (kh/kv)
    Unit of measurement : (ratio)
    Type                : float 
    Error Field         : Yes
    Time dependent      : No 
    Property I/O        : No

    Identification      : CapC1
    Description         : Factor in a formula to determine the capillary
                          entry pressure from the permeability
    Unit of measurement : (unknown)
    Type                : float 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 0.33

    Identification      : CapC2
    Description         : Factor in a formula to determine the capillary
                          entry pressure from the permeability
    Unit of measurement : (unknown)
    Type                : float 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 0.2611
    
    Identification      : PcKrModel
    Description         : Capillary pressure, relative permeability model
                          name (e.g Brooks Corey, etc)
    Unit of measurement : (unknown)
    Type                : string 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : "Brooks_Corey"

    Identification      : LambdaPc
    Description         : Exponent in Brooks-Corey constitutive relation
                          for capillary pressure
    Unit of measurement : (unknown)
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 1.0

    Identification      : LambdaPcWater
    Description         : Exponent in Brooks-Corey constitutive relation
                          for capillary pressure
    Unit of measurement : (unknown)
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : -9999
    
    Identification      : LambdaKr
    Description         : Exponent in Brooks-Corey constitutive relation
                          for relative permeability
    Unit of measurement : (unknown)
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 1.0

    Identification      : LambdaKrWater
    Description         : Exponent in Brooks-Corey constitutive relation
                          for relative permeability
    Unit of measurement : (unknown)
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : -9999

    Identification      : ThCondParallel
    Description         : Thermal conductivity parallel to bedding at a
                          specific temperature (= ThCond * ThCondAnisotrpy)  
    Unit of measurement : Wm-1C-1 
    Type                : float
    Error Field         : No 
    Time dependent      : Yes
    Property I/O        : No

#    Identification      : UraniumConcent
#    Description         : Uranium (U) concentration of a lithotype, used to
#                          calculate the radiogenic heat production
#    Unit of measurement : weigthppm 
#    Type                : float
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No

#    Identification      : ThoriumConcent
#    Description         : Thorium (Th) concentration of a lithotype, used to
#                          calculate the radiogenic heat production
#    Unit of measurement : weigthppm 
#    Type                : float
#    Error Field         : No
#    Time dependent      : No
#    Property I/O        : No

#    Identification      : PotassiumConcent
#    Description         : Potassium (K) concentration of a lithotype, used to
#                          calculate the radiogenic heat production
#    Unit of measurement : weigth% 
#    Type                : float
#    Error Field         : No
#    Time dependent      : No
#   Property I/O        : No

    Identification      : HeatProd
    Description         : Radiogenic heat production of a lythotype 
    Unit of measurement : uW/m3 (Lidia's report is wrong with mW/m3 ref Matthew)
    Type                : float
    Error Field         : Yes 
    Time dependent      : No
    Property I/O        : No

    Identification      : TopCrustHeatProd
    Description         : The surface Radiogenic heat production of the basement 
    Unit of measurement : uW/m3 
    Type                : float
    Error Field         : Yes 
    Time dependent      : No
    Property I/O        : No

    Identification      : TopCrustHeatProdGrid
    Description         : The surface Radiogenic heat production of the basement  grid name
    Unit of measurement : uW/m3 
    Type                : string
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No

    Identification      : CalibTopCrustHeatProd
    Description         : Calibrated 'TopCrustHeatProd' value
    Unit of measurement : uW/m3 
    Type                : float
    Error Field         : No 
    Time dependent      : No
    Property I/O        : No

    Identification      : CrustHeatPDecayConst
    Description         : The decay constant of the surface Radiogenic heat 
                          production of the basement 
    Unit of measurement : m
    Type                : float
    Error Field         : Yes 
    Time dependent      : No
    Property I/O        : No

    Identification      : PorosityVec2
    Description         : volumetric fraction of pore space in the rock/fluid 
                          mixture 
    Unit of measurement : %
    Type                : floatvector 2
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : SonicVec2
    Description         : sonic velocity of rock/fluid segment 
    Unit of measurement : us/m
    Type                : floatvector 2
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : VelocityVec2
    Description         : velocity of rock/fluid segment 
    Unit of measurement : m/s
    Type                : floatvector 2
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : ReflectivityVec2
    Description         : reflectivity of rock/fluid segment 
    Unit of measurement : 
    Type                : floatvector 2
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : BulkDensityVec2
    Description         : Bulk Density of rock/fluid segment 
    Unit of measurement : kg/m3
    Type                : floatvector 2
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : SurfacePorosity
    Description         : volumetric fraction of pore space in the rock/fluid 
                          mixture at the surface for a lithotype or a lithology
    Unit of measurement : %
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : SeisVelocity
    Description         : seismic velocity of a lithotype, fluidtype, matrix,
                          fluid or bulk 
    Unit of measurement : m/s
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : HydraulicFracturingPercent
    Description         : Lithostatic pressure % to trigger hydraulic fracturing
    Unit of measurement : %
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 100

    Identification      : ReferenceSolidViscosity
    Description         : Reference solid viscosity
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1e17

    Identification      : ActivationEnergy
    Description         : Activation Energy
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MinimumPorosity
    Description         : Minimum porosity
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0


    Identification      : SpecSurfArea
    Description         : Specific surface area, factor in a formula to determine
                          the capillary pressure from the porosity
    Unit of measurement : m2/g
    Type                : float 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 0.06

    Identification      : GeoVariance
    Description         : Geometric variance, factor in a formula to determine
                          the capillary pressure from the porosity
    Unit of measurement : 
    Type                : float 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No
    Default             : 1.7
#### Rock properties Algorithm parameters ---------------------------------

    Identification      : CompacCoefSC
    Description         : Sclater & Christie compaction coefficient for a 
                          lithotype
    Unit of measurement : km-1
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CompacCoefFM
    Description         : Falvey & Middleton compaction coefficient
    Unit of measurement : km-1
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : CompacCoefES
    Description         : Vertical Effective Stress compaction coefficient 
    Unit of measurement : 10-8Pa-1
    Type                : float
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : PresFuncName
    Description         : Fracture pressure curve name
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CoefA
    Description         : Fracture pressure curve parameter
    Unit of measurement : MPa
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CoefB
    Description         : Fracture pressure curve parameter
    Unit of measurement : MPa/m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CoefC
    Description         : Fracture pressure curve parameter
    Unit of measurement : MPa/m^2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CoefD
    Description         : Fracture pressure curve parameter
    Unit of measurement : MPa/m^3
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#### Point properties -----------------------------------------------------


    Identification      : DummyLayerSegment
    Description         : if property is set at segment the segment 
                          belongs to a dummy layer
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HeatFlow
    Description         : User supplied heat flow for bottom boundary condition
                          (i.e mantle).
    Unit of measurement : mW/m2  
    Type                : float 
    Error Field         : Yes
    Time dependent      : No (time variations supplied via table)
    Property I/O        : No

    Identification      : HeatFlowGrid
    Description         : User supplied heat flow grid name
                          for bottom boundary condition (i.e mantle).
    Unit of measurement : mW/m2
    Type                : string 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Ves
    Description         : vertical effective stress
                          overburden pressure - fluid pressure
    Unit of measurement : Pa (to user psi, bar)
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : MaxVes
    Description         : maximum vertical effective stress from time of 
                          depostion to specified time t
    Unit of measurement : Pa
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


#### (over)pressure related properties ---------------------------

    Identification      : MassFlux_
    Description         : MassFlux (calculated)
    Unit of measurement : kg/(s.m2)
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : MassFluxVec8
    Description         : MassFlux (calculated), corner fluxes as calculated
                          by the solver one value for each corner of a brick
    Unit of measurement : kg/(s.m2)
    Type                : doublevector 8
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : MassFluxX
    Description         : Horizontal MassFlux (calculated)
    Unit of measurement : kg/(s.m2)
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : MassFluxY
    Description         : Horizontal MassFlux (calculated)
    Unit of measurement : kg/(s.m2)
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : MassFluxZ
    Description         : Vertical MassFlux (calculated)
    Unit of measurement : kg/(s.m2)
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : Pressure
    Description         : The pressure 
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : FracturePressure
    Description         : The pressure 
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : Buoyancy
    Description         : The pressure 
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


    Identification      : CEPGas
    Description         : The Capillary Entry Pressure for Gas
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Default		: -1
    Property I/O        : No

    Identification      : CEPOil
    Description         : The Capillary Entry Pressure for Oil
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Default		: -1
    Property I/O        : No

    Identification      : CEPVapour
    Description         : The Capillary Entry Pressure for Gas
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Default		: -1
    Property I/O        : No

    Identification      : CEPLiquid
    Description         : The Capillary Entry Pressure for Oil
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Default		: -1
    Property I/O        : No

    Identification      : FractSealStrength
    Description         : The Fracture Seal Strength
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Default		: -1
    Property I/O        : No

    Identification      : HydroStaticPressure
    Description         : The hydrostatic pressure, which follows a simple
                          depth curve.
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


    Identification      : LithoStaticPressure
    Description         : The lythostatic pressure, which follows a simple
                          depth curve.
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


    Identification      : OverPressure (Pressure - HydroStaticPressure)
    Description         : The overpressure
    Unit of measurement : MPa
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : FCTCorrection
    Description         : Factor with which to scale the FCT history.
    Unit of measurement :
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No
    Default             : 1.0

    Identification      : FaultElements
    Description         : Indicates whether fault intersects element or not
    Unit of measurement :
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No
    Default             : 0.0

    Identification      : ThicknessError
    Description         : Error is calculated thickness, applies only to overpressure calculations with geometric loop.
    Unit of measurement : %
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No
    Default             : 1.0

    Identification      : AllochthonousLithology
    Description         : The distribution of the allochthonous lithology for the particular snapshot time.
    Unit of measurement : %
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No
    Default             : 0.0

    Identification      : SaveFCTCorrection
    Description         : Factor with which to scale the FCT history.
    Unit of measurement :
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1.0

    Identification      : C1Adsorped
    Unit of measurement : scf/ton
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : C1AdsorpedExpelled
    Unit of measurement : scf/ton
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : C1AdsorpedFree
    Unit of measurement : scf/ton
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : FractionOfAdsorptionCap
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : AdsorptionCapacity
    Unit of measurement : scf/ton
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : HcVapourSat
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : HcLiquidSat
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : HcSaturation
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : PorosityLossFromPyroBitumen
    Unit of measurement : %
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : H2SRisk
    Unit of measurement : kg/m2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : OverChargeFactor
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : ImmobileWaterSat
    Unit of measurement : frac
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : RetainedCgrSR
    Unit of measurement : bbl/mcf
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : RetainedGorSR
    Unit of measurement : scf/bbl
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : RetainedCondensateApiSR
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : RetainedOilApiSR
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0


    Identification      : RetainedGasVolumeST
    Unit of measurement : bcf/km2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : RetainedOilVolumeST
    Unit of measurement : mmbbl/km2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : GasExpansionRatio_Bg
    Unit of measurement : m3/m3
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : Oil2GasGeneratedCumulative
    Unit of measurement : kg/m2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0

    Identification      : TotalGasGeneratedCumulative
    Unit of measurement : kg/m2
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.0



#### Calibration / Thermal Indicator properties ---------------------------
# Measured Pressure
    Identification      : MPressure
    Description         : 
    Unit of measurement : MPa 
    Type                : double 
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : FormPressureType
    Description         : Text label which allows user to classify their 
                          press. data as RFT or LOP
    Unit of measurement : 
    Type                : string
#   Enum options:        : "RFT" "LPO"
    Error Field         : No
    Time dependent      : No

# temperature / BHT (bottom hole temperatures)

    Identification      : Temperature
    Description         : 
    Unit of measurement : C (degCentigrade)
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ErrTemperature
    Description         : Error in Temperature
    Unit of measurement : C (degCentigrade)
    Type                : double 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


    Identification      : TemperatureGrid
    Description         : 
    Unit of measurement : C (degCentigrade)
    Type                : string 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TempIndex   
    Description         : temperature as used as index in
                          tables  
    Unit of measurement : C (degCentigrade)
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TempGradient
    Description         : temperature gradient per formation.
                          If this option is selected by the user for their
                          calculation then lithotypes, conductivities etc.
                          are ignored, and temperature gradient is an input 
                          parameter. Alternatively in a heatflow option
                          run temperature gradient is calulated for a segment.
    Unit of measurement : C/m 
    Type                : float
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : FormTempType
    Description         : Text label which allows user to classify their 
                          temp. data as BHT, DST, RFT or other
    Unit of measurement : 
    Type                : string
#   Enum options:        : "BHT" "RFT" "GEOfile"
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CirculationTime
    Description         : Mud circulation time (for BhtTable)
    Unit of measurement : min
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TimeSinceCirStopped
    Description         : Lenght of time betweem end of mud circulation 
                          and logging tool on the bottom (for BhtTable)
    Unit of measurement : min
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Theta
    Description         : Value calculated in bht correction.
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BitDiameter
    Description         : Diameter of bit (for BhtTable) (frequently in inches)
    Unit of measurement : inch
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MarsdenAcoef
    Description         : a coefficient of Marsden & Schreurs bht correction
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : MarsdenBcoef
    Description         : b coefficient of Marsden & Schreurs bht correction
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : MarsdenCcoef
    Description         : c coefficient of Marsden & Schreurs bht correction
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

 
# vitrinite reflectance

    Identification      : Vr
    Description         : Vitrinite Reflectance Value for a VRM (measurement)
                          or for a VRE model (calculated)
    Unit of measurement : % reflectivity
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : VrmType
    Description         : Text label which allows user to classify their VRM 
                          data as one of 3 types (tellocollinite, 
                          desmocoll. & other)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

 
# Sonic velocity (Sonic log from Logic)

    Identification      : Dt
    Description         : Sonic Velocity Value for Logic (measurement)
                          or for a Sonic velocity model (calculated)
    Unit of measurement : us/m
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

 
#  Bulk Density (Density log from Logic)

    Identification      : BDensity
    Description         : Bulk Density Value for Logic (measurement)
                          or for a Bulk Density model (calculated)
    Unit of measurement : kg/m^3
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

# Biomarker properties

    Identification      : HopaneIsomerisation
    Description         : 
    Unit of measurement : 
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : SteraneIsomerisation
    Description         : 
    Unit of measurement : 
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : SteraneAromatisation
    Description         : 
    Unit of measurement : 
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No


    Identification      : HopIsoActEnergy
    Description         : 
    Unit of measurement : J/mol
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SteIsoActEnergy
    Description         : 
    Unit of measurement : J/mol
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SteAroActEnergy
    Description         : 
    Unit of measurement : J/mol
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : HopIsoFreqFactor
    Description         : 
    Unit of measurement : sec-1
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SteIsoFreqFactor
    Description         : 
    Unit of measurement : sec-1
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SteAroFreqFactor
    Description         : 
    Unit of measurement : sec-1
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HopIsoGamma
    Description         : 
    Unit of measurement : 
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SteIsoGamma
    Description         : 
    Unit of measurement : 
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


# fission track properties

    Identification      : FtGrainId
    Description         : Fission track grain identifier in sample
    Unit of measurement : 
    Type                : integer 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtSpontTrackNo
    Description         : Number of spontaneous tracks in grain
    Unit of measurement : 
    Type                : integer 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtInducedTrackNo
    Description         : Number of induces tracks in grain
    Unit of measurement : 
    Type                : integer 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtLength
    Description         : Fission track length
    Unit of measurement : microns
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtClWeightPerc
    Description         : Chlorine weight percentage
    Unit of measurement : %
    Type                : float 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtSampleId
    Description         : Fission track sample identifier
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtZeta
    Description         : Calibration factor used for fission track age calculation
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtUstglTrackDensity
    Description         : Track density of uranium standard glass 
    Unit of measurement : cm-2
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtGrainAge
    Description         : Fission track age of a grain
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtGrainAgeErr
    Description         : Error in fission track age of a grain
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtPooledAge
    Description         : Fission track age of sample
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtPooledAgeErr
    Description         : error in fission track age of sample
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtPredictedAge
    Description         : Fission track age 
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtAgeChi2
    Description         : ChiSquared of the grain fission track ages of a sample
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtDegreeOfFreedom
    Description         : Degree of freedom in chi square calculation
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtPAgeChi2
    Description         : P(Chi Squared) of the grain fission track ages of a sample
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtCorrCoeff
    Description         : Correlation coefficient Ns and Ni
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtVarianceSqrtNs
    Description         : Variance of square root of Ns
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtVarianceSqrtNi
    Description         : Variance of square root of Ni
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtNsDivNi
    Description         : Ns divided by Ni
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtNsDivNiErr
    Description         : Ns divided by Ni
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtMeanRatio
    Description         : Mean ratio 
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtMeanRatioErr
    Description         : Error in mean ration
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtCentralAge
    Description         : Central age
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtCentralAgeErr
    Description         : Error in central age
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtMeanAge
    Description         : Mean age
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtMeanAgeErr
    Description         : Error in mean age
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtLengthChi2
    Description         : ChiSquared of the fission track lengths distributions 
                          (calculated distribution and observed distribution)
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtApatiteYield
    Description         : Fission track sample identifier
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

   #Fission track new data entries for Cauldron/BPA integration

    Identification      : FtPredLengthHistId
    Description         : Histogram identifier
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtPredLengthBinStart
    Description         : Start length of first bin in this histograms
    Unit of measurement : 
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtPredLengthBinWidth
    Description         : Width of each historgram bin
    Unit of measurement : 
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtPredLengthBinNum
    Description         : Number of bins in histogram
    Unit of measurement : 
    Type                : integer 
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtPredLengthBinIndex
    Description         : Index of bin according to line layout defined in FtPredLengthCountsHistIoTbl
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtPredLengthBinCount
    Description         : Bin count (=number of predicted tracks). due to the prediction algorithm this might be a floating point number (instead of an integer)
    Unit of measurement : 
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtClWeightBinStart
    Description         : Start of first Cl weight percent bin
    Unit of measurement : 
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

    Identification      : FtClWeightBinWidth
    Description         : Width of  Cl weight percent bins
    Unit of measurement : 
    Type                : double 
    Error Field         : No
    Time dependent      : No
    Property I/O        : Yes

# smectite illite properties

    Identification      : IlliteFraction
    Description         : Illite Fraction
    Unit of measurement : 
    Type                : float
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : ActEnergy1
    Description         : Phase 1 (growth of illite layers within flaky
                          smectite particles) activation energy
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreqFactor1
    Description         : Phase 1 (growth of illite layers within flaky
                          smectite particles) frequency factor
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : ActEnergy2
    Description         : Phase 2 (dissilution of smectite and the 
                          precipitation of illite-rich crystals) activation 
			  energy
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreqFactor2
    Description         : Phase 2 (dissilution of smectite and the 
                          precipitation of illite-rich crystals) frequency 
			  factor
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InitIlliteFraction
    Description         : Initial Illite fraction in the sediments of the column
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


#### Run Options / Algorithm choice Properties -----------------------------

    Identification      : FileVersioning
    Description         : File versioning policy:
                          1 = no file versioning
			  2 = IBS - like version numbers
			  3 = .bak files
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : AutoSaving
    Description         : Automatically saves the projectfile after a run
                          0 or NULL = no autosaving
			  1 = autosaving
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SaveBeforeEachRun
    Description         : Saves the whole projectfile before each 
                          montecarlo iteration
                          0 or NULL = no autosaving
			  1 = autosaving
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MaxFileWidth
    Description         : Max width for the iosets, if a table
                          exceeds this limit then a block format will be used
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ConsoleLevel
    Description         : Level at which messages are send to the console
                          < 0 means error msssages
			  > 0 means trace msssages
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PopupLevel
    Description         : Level at which messages are popped up
                          < 0 means error msssages
			  > 0 means trace msssages
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LogfileLevel
    Description         : Level at which messages are send to the logfile
                          < 0 means error msssages
			  > 0 means trace msssages
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CompactionAlgorithm
    Description         : Active compaction algorithm
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BottomBoundaryModel
    Description         : Active Bottom boundary model, 
                          (Fixed Temperature, Fixed HeatFlow)
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VreAlgorithm
    Description         : Active vitrinite reflectance model
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VelAlgorithm
    Description         : Active velocity model
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : Optimisation_Level
    Description         : Level of optimisation in run
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : "Level 4 (default)"

    Identification      : Temperature_Gradient
    Description         : The rate of temperature (C) increase WRT depth 
                          Measured in kilometres in the project file, but metres in the code. 
                          It is converted in the Project class (Project::Set_Temperature_Gratient)
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 30

    Identification      : TempDiffBasedStepping
    Description         : use temp. diff. based timestepping instead of segment burial rate.
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1

    Identification      : StepsPerSegment
    Description         : segment burial rate.
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1

    Identification      : Constrain_VES
    Description         : Constrain VES to be greater than zero/Reference VES (0 or 1.0e5)(Should be Boolean)
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1


    Identification      : HeatFlowSolution
    Description         : Heat flow solution model
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TopAsthenoTemp
    Description         : Temperature at the top of the asthenospheric mantle
                          (bottom of lithospheric mantle) i.e. melting
                          temperature.
    Type                : float
    Unit of measurement : C (degCentigrade)
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UncertaintyAnalysis
    Description         : Application of uncertainty analysis;  (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SensitivityAnalysis
    Description         : Application of sensitivity analysis;  (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Him
    Description         : Values: 1st, 2nd, 3nd, nth
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Hims
    Description         : Values: single
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumEstimates
    Description         : Values: 2,2 or 4,8
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CoupledMode
    Description         : Whether or not running Coupled Pressure and Temperature 
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ChemicalCompaction
    Description         : Whether or not using Chemical Compaction
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ConvectiveTerm
    Description         : Whether or not using Convective Term
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NonGeometricLoop
    Description         : Whether or not using non-geometric loop calculation
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AllochthonousModelling
    Description         : Whether or not to use the allochthonous modelling.
    Type                : boolean
    Unit of measurement :
    Default             : FALSE
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ResponseSurfaceModeling
    Description         : Application of response surface modeling;  (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PrimaryRow
    Description         : Name of the first identification row in the table
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SecondaryRow
    Description         : Name of the second identification row in the table
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RSMparameter
    Description         : Name of the parameter to change
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RSMnumber
    Description         : the ID number in %param[ID]%
    Type                : integer
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RSMparameterValue
    Description         : The identification for response surface modelling
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : McHistOutput
    Description         : Switch for MonteCarlo Histogram output generation  (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RSMxmlOutput
    Description         : Switch for response surface output generation  (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ApplyOtgcToDarcy
    Description         : Indicate whether otgc should be applied to Darcy simulation
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 1

    Identification      : DarcyMaxTimeStep
    Description         : Maximum time step in Darcy simulation
    Type                : double
    Unit of measurement : Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0.001


    Identification      : Optimization
    Description         : Application of Optimization; boolean value (0 or 1)
    Type                : boolean
    Unit of measurement : 
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FormTempOptimWeight
    Description         : Relative weight for the optimization function
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VrmOptimWeight
    Description         : Relative weight for the optimization function
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BiomOptimWeight
    Description         : Relative weight for the optimization function
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FtOptimWeight
    Description         : Relative weight for the optimization function
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SmiOptimWeight
    Description         : Relative weight for the optimization function
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimTolerance
    Description         : Optimality Tolerance for the calibration procedure
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimTopCrustHeatProd
    Description         : 0 = No optimization
                          1 = Optimize crustal heat production (A0)
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimCrustalThickness
    Description         : 0 = No optimization
                          1 = Optimize crustal thicknesses
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimMantleThickness
    Description         : 0 = No optimization
                          1 = Optimize the mantle thickness
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OptimUnconfThickness
    Description         : 0 = No optimization
                          1 = Optimize unconformity thicknesses
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NrMCLoops
    Description         : Number of monte Carlo loops to be run 
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NrMCSaveEvery
    Description         : If NrMCSaveEvery > 0 then the project file
                          together with the output results will be
			  saved every "NrMCSaveEvery" iterations. These
			  files can be used for intermediate purposes
			  and to be able to perform a restart and
			  continue in case of problems.
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NrSensSaveEvery
    Description         : If NrSensSaveEvery > 0 then the project file
                          together with the output results will be
			  saved every "NrSensSaveEvery" iterations. These
			  files can be used for intermediate purposes
			  and to be able to perform a restart and
			  continue in case of problems.
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NrMCLoopsCompleted
    Description         : Number of Monte Carlo loops completed,
                          this information is needed for the
			  restart facility.
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MCStatusOfLastRun
    Description         : Status of the last run,
                          this information is needed for the
			  restart facility and for "Big" 3D projects
			  Possible values are:
			     "Initial", "CalculatingGeometry", "GeometryCalculated",
			     "CalculatingTemperatures", "TemperaturesCalculated", 
			     "CalculatingGenex", "GenexCalculated"
			     "CalculatingXmig",  "XmigCalculated", etc.
    Type                : string
    Default		: "Initial"
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : MCStatusOfLastPressureRun
    Description         : Status of the last run,
                          this information is needed for the
			  restart facility and for "Big" 3D projects
			  Possible values are:
			     "Initial", "CalculatingPressures", "PressuresCalculated",
    Type                : string
    Default		: "Initial"
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : MCCalculationScope
    Description         : Target of the last run,
                          this information is needed for the
			  restart facility
			  Possible values are:
			  "DECOMPACT", "HIGHRESDECOMPACT", "TEMPERATURE",
                          "GENEX", "XMIG", "FULLRUN", "PRESSURE".
    Type                : string
    Default		: "FULLRUN"
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MCStatusOfLastHighResRun
    Description         : Status of the last run,
                          this information is needed for the
			  restart facility and for "Big" 3D projects
			  Possible values are:
			     "Initial", "CalculatingHighResDecompaction",
                             "HighResDecompactionCalculated",
    Type                : string
    Default		: "Initial"
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : OutputDirOfLastRun
    Description         : Output directory of the last run, needed to save
                          the output of a continued run within the save 
			  directory
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OutputDirCreatedBy
    Description         : The name of the projectfile (without extension),
                          the outputdirectory belongs to.
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MCCurrentSeedNumber
    Description         : Internal seed number for the montecarlo pseudo
                          random generator. This number is needed generate
			  the same results in case of a re-start + continue.
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RestartTempCalcTimeStep
    Description         : The first temperature calculation timestep that need to 
                          be taken in case of a restart for 3D projects.
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RestartPresCalcTimeStep
    Description         : The first pressure calculation timestep that need to 
                          be taken in case of a restart for 3D projects.
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AnalysisIdentification
    Description         : Unique identification for the sensitivity analysis
                          to be perform. For more details see "RBYmontecarlo.dat"
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AnalysisIndicator
    Description         : Switch for sensitivity analysis (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyDescription
    Description         : Description for a property application
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TypeOfDistribution
    Description         : Values: Uniform, Normal_1SD, Normal_2SD, Normal_3SD
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SGRIndicator
    Description         : Switch for SGR analysis (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BioDegradInd
    Description         : Switch for Bio Degradation (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilToGasCrackingInd
    Description         : Switch for Bio Degradation (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Default		: 1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DiffusionInd
    Description         : Switch for Diffusion (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ActivityMode
    Description         : Reservoir activityMode
    Type                : string
    Unit of measurement : 
    Default		: "AlwaysActive"
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ActivityStart
    Description         : Reservoir start of activity
    Type                : float
    Unit of measurement : Ma
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BlockingInd
    Description         : Switch for vertical migration blocking (1 = On, 0 = Off)
    Type                : boolean
    Unit of measurement : 
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BlockingPermeability
    Description         : Permeability at which to block vertical migration
    Type                : float
    Unit of measurement : mD
    Default		: 1e-9
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PermafrostInd
    Description         : Indicate whether or not include permafrost modelling (1 = yes, 0 = no)
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : PressureTerm
    Description         : Indicate whether or not include pressure term in permafrost modelling (1 = yes, 0 = no)
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : SalinityTerm
    Description         : Indicate whether or not include salinity term in permafrost modelling (1 = yes, 0 = no)
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

### Boundary conditions
###  Geological Boundary Io Tbl

    Identification         : GeologicalObjectName
    Description         : Name of the layer or formation on which a boundary condition is applied
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
                          
    Identification      : GeologicalObjectType
    Description         : Type of the geological object ( surface 0  or volume 1 )
    Type                : string 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumberOfProperties
    Description         : Number of properties boundaries assigned to the geological object
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TypeOfProperty
    Description         : Type of properties boundaries assigned to the geological object ( Flow1 or Fixed 0)
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No 

    Identification      : NameOfProperty
    Description         : Name of properties boundaries assigned to the geological object
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FirstIndexInPropertyIoTbl
    Description         : FirstIndex in the PropertyBoundaryIoTbl  for the current PropertyBoundary
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

## Property BoudaryIoTbl
    Identification      : FirstIndexInPropertyBoundaryValuesIoTbl
    Description         : FirstIndex in the BoundareyValuesUoTbl for the current PropertyBoundary
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumberOfTimeIntervals
    Description         : Number of Time intervals when the property is active and has aassigned value
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

# BoudaryValue IoTbl
    Identification      : BeginTimeValues
    Description         : Beginning of toime interval
    Type                : double 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : EndTimeValues
    Description         : End of toime interval
    Type                : double 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyValueGrid
    Description         : Grid of values
    Type                : string 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PropertyValue
    Description         : Assigned value on the boundary
    Type                : double 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No  
  
 

   
#### User Interface properties:-----------------------------------------------

    Identification      : ContourColour
    Description         : colour to contour a set of property values
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ContourValue
    Description         : Contour value (or milestones) for the property  
                          set in the DisplayContourIoTbl 
    Unit of measurement :  
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UseDefault
    Description         : Must default be used in Property Display
    Type                : integer
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LowerboundDefault
    Description         : Default lowerbound value
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LowerboundValue
    Description         : Lowerbound value
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RedLowerbound
    Description         : Red component for lowerbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GreenLowerbound
    Description         : Green component for lowerbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BlueLowerbound
    Description         : Blue component for lowerbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UpperboundDefault
    Description         : Default upperbound value
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UpperboundValue
    Description         : Upperbound value
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : RedUpperbound
    Description         : Red component for upperbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GreenUpperbound
    Description         : Green component for upperbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BlueUpperbound
    Description         : Blue component for upperbound colour
    Type                : float
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PlotLineId
    Description         : Identification of line in PlotTbl
    Type                : string
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PlotXcrd
    Description         : X value for plot point in graph
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PlotYcrd
    Description         : Y value for plot point in graph
    Type                : double 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BoundsEnabled
    Description         : Property Boundaries on/off for plotting
    Unit of measurement :  
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LowerBound
    Description         : Lower Property Boundary for plotting
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : UpperBound
    Description         : Upper Property Boundary for plotting
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BinStart
    Description         : Start of bin in histogram
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BinEnd
    Description         : End of bin in histogram
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : BinCount
    Description         : Number of items in Bin of Histogram
    Type                : double
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


#
#### Genex output properties: start --------------------------------------
#
#
# Oil
#

    Identification      : InstantaneousExpulsionApi
    Description         : InstantaneousExpulsionApi
    Type                : float
    Unit of measurement : API
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CumulativeExpulsionApi
    Description         : CumulativeExpulsionApi
    Type                : float
    Unit of measurement : API
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InstantaneousExpulsionCondensateGasRatio
    Description         : InstantaneousExpulsionCondensateGasRatio
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CumulativeExpulsionCondensateGasRatio
    Description         : CumulativeExpulsionCondensateGasRatio
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InstantaneousExpulsionGasOilRatio
    Description         : InstantaneousExpulsionGasOilRatio
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CumulativeExpulsionGasOilRatio
    Description         : CumulativeExpulsionGasOilRatio
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InstantaneousExpulsionGasWetness
    Description         : InstantaneousExpulsionGasWetness
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CumulativeExpulsionGasWetness
    Description         : CumulativeExpulsionGasWetness
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InstantaneousExpulsionAromaticity
    Description         : InstantaneousExpulsionAromaticity
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CumulativeExpulsionAromaticity
    Description         : CumulativeExpulsionAromaticity
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : KerogenConversionRatio
    Description         : KerogenConversionRatio
    Type                : float
    Unit of measurement :  
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilGeneratedCumulative
    Description         : OilGeneratedCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilGeneratedRate
    Description         : OilGeneratedRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilExpelledCumulative
    Description         : OilExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilExpelledRate
    Description         : OilExpelledRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcGasGeneratedCumulative
    Description         : HcGasGeneratedCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcGasGeneratedRate
    Description         : HcGasGeneratedRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcGasExpelledCumulative
    Description         : HcGasExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcGasExpelledRate
    Description         : HcGasExpelledRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DryGasGeneratedCumulative
    Description         : DryGasGeneratedCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DryGasGeneratedRate
    Description         : DryGasGeneratedRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DryGasExpelledCumulative
    Description         : DryGasExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DryGasExpelledRate
    Description         : DryGasExpelledRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WetGasGeneratedCumulative
    Description         : WetGasGeneratedCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WetGasGeneratedRate
    Description         : WetGasGeneratedRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WetGasExpelledCumulative
    Description         : WetGasExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WetGasExpelledRate
    Description         : WetGasExpelledRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : asphaltenesExpelledCumulative
    Description         : asphaltenesExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : resinsExpelledCumulative
    Description         : resinsExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AroExpelledCumulative
    Description         : C15AroExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15SatExpelledCumulative
    Description         : C15SatExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14AroExpelledCumulative
    Description         : C6_14AroExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14SatExpelledCumulative
    Description         : C6_14SatExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SbearingHCsGeneratedCumulative
    Description         : SbearingHCsGeneratedCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SbearingHCsGeneratedRate
    Description         : SbearingHCsGeneratedRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SbearingHCsExpelledCumulative
    Description         : SbearingHCsExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SbearingHCsExpelledRate
    Description         : SbearingHCsExpelledRate
    Type                : float
    Unit of measurement : kg/m2/Ma
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


##
## The following are used on for units in UI
##


    Identification      : C15+AroExpelledCumulative
    Description         : C15+AroExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15+SatExpelledCumulative
    Description         : C15+SatExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14AroExpelledCumulative
    Description         : C6-14AroExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14SatExpelledCumulative
    Description         : C6-14SatExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

##
## until here
##

    Identification      : C5ExpelledCumulative
    Description         : C5ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C4ExpelledCumulative
    Description         : C4ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C3ExpelledCumulative
    Description         : C3ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C2ExpelledCumulative
    Description         : C2ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C1ExpelledCumulative
    Description         : C1ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : COxExpelledCumulative
    Description         : COxExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : N2ExpelledCumulative
    Description         : N2ExpelledCumulative
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AroRetained
    Description         : C15AroRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15SatRetained
    Description         : C15SatRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14AroRetained
    Description         : C6_14AroRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14SatRetained
    Description         : C6_14SatRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15+AroRetained
    Description         : C15+AroRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15+SatRetained
    Description         : C15+SatRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14AroRetained
    Description         : C6-14AroRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14SatRetained
    Description         : C6-14SatRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C5Retained
    Description         : C5Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C4Retained
    Description         : C4Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C3Retained
    Description         : C3Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C2Retained
    Description         : C2Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C1Retained
    Description         : C1Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : COxRetained
    Description         : COxRetained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : N2Retained
    Description         : N2Retained
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AroExpelledCumulativeSR
    Description         : C15AroExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15SatExpelledCumulativeSR
    Description         : C15SatExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14AroExpelledCumulativeSR
    Description         : C6_14AroExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14SatExpelledCumulativeSR
    Description         : C6_14SatExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15+AroExpelledCumulativeSR
    Description         : C15+AroExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15+SatExpelledCumulativeSR
    Description         : C15+SatExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14AroExpelledCumulativeSR
    Description         : C6-14AroExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6-14SatExpelledCumulativeSR
    Description         : C6-14SatExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C5ExpelledCumulativeSR
    Description         : C5ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C4ExpelledCumulativeSR
    Description         : C4ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C3ExpelledCumulativeSR
    Description         : C3ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C2ExpelledCumulativeSR
    Description         : C2ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C1ExpelledCumulativeSR
    Description         : C1ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : COxExpelledCumulativeSR
    Description         : COxExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : N2ExpelledCumulativeSR
    Description         : N2ExpelledCumulativeSR
    Type                : float
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


##
##
##

    Identification      : HcLiquidVolume
    Description         : HcLiquidVolume not really a volume but a ratio of volumes
    Type                : float
    Unit of measurement : m^3/m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourVolume
    Description         : HcVapourVolume not really a volume but a ratio of volumes
    Type                : float
    Unit of measurement : m^3/m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ElementVolume
    Description         : ElementVolume volume of element
    Type                : float
    Unit of measurement : m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ElementPoreVolume
    Description         : ElementPoreVolume Pore volume in element.
    Type                : float
    Unit of measurement : m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : TransportedMass
    Description         : Mass that has been transported during a snapshot interval (for debugging only).
    Type                : float
    Unit of measurement : kg
    Error Field         : No
    Time dependent      : No
    Property I/O        : No



##
## Concentration of HCs
##

    Identification      : asphaltenesConcentration
    Description         : asphaltenesConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : resinsConcentration
    Description         : resinsConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AroConcentration
    Description         : C15AroConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15SatConcentration
    Description         : C15SatConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14AroConcentration
    Description         : C6_14AroConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14SatConcentration
    Description         : C6_14SatConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C5Concentration
    Description         : C5Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C4Concentration
    Description         : C4Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C3Concentration
    Description         : C3Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C2Concentration
    Description         : C2Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C1Concentration
    Description         : C1Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : COxConcentration
    Description         : COxConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : N2Concentration
    Description         : N2Concentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : H2SConcentration
    Description         : H2SConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


    Identification      : LSCConcentration
    Description         : LSCConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AtConcentration
    Description         : C15AtConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14BtConcentration
    Description         : C6_14BtConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14DbtConcentration
    Description         : C6_14DbtConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14BpConcentration
    Description         : C6_14BpConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15AroSConcentration
    Description         : C15AroSConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C15SatSConcentration
    Description         : C15SatSConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14SatSConcentration
    Description         : C6_14SatSConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : C6_14AroSConcentration
    Description         : C6_14AroSConcentration
    Type                : float
    Unit of measurement : kg/m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No



##
## Saturation of phases.
##

    Identification      : BrineSaturation
    Description         : Water/brine Saturation
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcLiquidSaturation
    Description         : LiquidSaturation of HC
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourSaturation
    Description         : VapourSaturation of HC
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AverageBrineSaturation
    Description         : Averaged Water/brine Saturation
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AverageHcLiquidSaturation
    Description         : Averaged LiquidSaturation of HC
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AverageHcVapourSaturation
    Description         : Averaged VapourSaturation of HC
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ImmobileSaturation
    Description         : Immobile species saturation
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AverageImmobileSaturation
    Description         : Averaged immobile species saturation
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

##
## Relative permeability of phases.
##

    Identification      : BrineRelativePermeability
    Description         : Water/brine RelativePermeability
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcLiquidRelativePermeability
    Description         : LiquidRelativePermeability of HC
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourRelativePermeability
    Description         : VapourRelativePermeability of Hc
    Type                : float
    Unit of measurement : frac
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

##
## Capillary pressure between phases.
##

    Identification      : HcLiquidBrineCapillaryPressure
    Description         : PC between Hc liquid and brine
    Type                : float
    Unit of measurement : Pa
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourBrineCapillaryPressure
    Description         : PC between Hc vapour and brine
    Type                : float
    Unit of measurement : Pa
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcLiquidBrineCapillaryEntryPressure
    Description         : PC between Hc liquid and brine
    Type                : float
    Unit of measurement : Pa
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourBrineCapillaryEntryPressure
    Description         : PC between Hc vapour and brine
    Type                : float
    Unit of measurement : Pa
    Error Field         : No
    Time dependent      : No
    Property I/O        : No


##
##
##

    Identification      : HcLiquidDensity
    Description         : HcLiquidDensity
    Type                : float
    Unit of measurement : kg/m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourDensity
    Description         : HcVapourDensity
    Type                : float
    Unit of measurement : kg/m^3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcLiquidViscosity
    Description         : HcLiquidViscosity
    Type                : float
    Unit of measurement : PaS
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HcVapourViscosity
    Description         : HcVapourViscosity
    Type                : float
    Unit of measurement : PaS
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

##
## HC phase Velocities.
##

    Identification      : HcVapourVelocityX
    Description         : Horizontal hc-vapour velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcVapourVelocityY
    Description         : Horizontal hc-vapour velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcVapourVelocityZ
    Description         : Vertical hc-vapour velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcVapourVelocityMagnitude
    Description         : Magnitude of the hc-vapour velocity
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No


    Identification      : HcLiquidVelocityX
    Description         : Horizontal hc-liquid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcLiquidVelocityY
    Description         : Horizontal hc-liquid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcLiquidVelocityZ
    Description         : Vertical hc-liquid velocity (calculated)
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No

    Identification      : HcLiquidVelocityMagnitude
    Description         : Magnitude of the hc-liquid velocity
    Unit of measurement : mm/y
    Type                : float
    Error Field         : No 
    Time dependent      : Yes 
    Property I/O        : No


#
#### Genex output properties: end --------------------------------------
#
#
# Colin's extensions ------------------------------------------------------
#
#    Identification      : Head
#    Description         : Fluid Potential
#    Unit of measurement : C (degCentigrade)
#    Type                : float 
#    Error Field         : No
#    Time dependent      : Yes
#    Property I/O        : No

#    Identification      : FluidFlow
#    Description         : The fluid flow vector
#    Unit of measurement : 
#    Type                : floatvector 3
#    Error Field         : No
#    Time dependent      : Yes
#    Property I/O        : No

#### Reservoir Properties -----------------------------

    Identification      : ReservoirName
    Description         : Name of reservoir (input by user)
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : SourceAge
    Description         : Geological age of an event, entry in a table etc. 
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DestinationAge
    Description         : Geological age of an event, entry in a table etc. 
    Unit of measurement : Ma
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourceRockName
    Description         : Name of source rock layer
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : SourceReservoirName
    Description         : Name of reservoir
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : DestinationReservoirName
    Description         : Name of reservoir
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : TrapCapacity
    Description         : Minimal volume of a trap to be of interest
    Unit of measurement : m3
    Type                : float
    Default		: 5e5
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : Capacity
    Description         : Possible volume of a trap
    Unit of measurement : m3
    Type                : float
    Default		: 5e5
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : DepthOffset
    Description         : offset depth from the top of a formation 
    Unit of measurement : m
    Type                : double
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : DepthOffsetGrid
    Description         : MapName of the "DepthOffset" property
    Unit of measurement : m
    Type                : string

    Identification      : NetToGross
    Description         : Net to Gross Percentage  
    Unit of measurement : percent
    Type                : double
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : NetToGrossGrid
    Description         : MapName of the "NetToGross" property
    Unit of measurement : percent
    Type                : string

    Identification      : MicroTraps
    Description         : microtraps density at the top of the reservoir 
    Unit of measurement : m3/m2
    Type                : double
    Error Field         : Yes
    Time dependent      : No
    Property I/O        : No

    Identification      : LeakProperty
    Description         : switch to tell what is leaking gas/oil with without a specified rate (NoLeak/GasLeak/OilLeak/GasLeakRate/OilLeakRate).
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LeakRate
    Description         : rate at which the oil/gas is leaking. 
    Unit of measurement : kg/m2/ma
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LayerFrequency 
    Description         : Sand/Shale Layer Frequency  
    Unit of measurement : km 
    Type                : double 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : LayerFrequencyGrid
    Description         : MapName of the "LayerFrequency" property
    Unit of measurement : 
    Type                : string

    Identification      : OilSlope 
    Description         : Oil SGR slope
    Unit of measurement : km 
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : GasSlope 
    Description         : Gas SGR slope
    Unit of measurement : 
    Type                : float 
    Error Field         : Yes
    Time dependent      : Yes
    Property I/O        : No

    Identification      : MinimumSGR 
    Description         : Minimun SGR value 
    Unit of measurement : 
    Type                : float 
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

#### BioDegradation Constants -----------------------------

    Identification      : MaxBioTemp 
    Description         : Maximum temperature for biodegradation to take place
    Unit of measurement : C (degCentigrade)
    Type                : double 
    Default		: 80.0 
    Error Field         : Yes
    Time dependent      : No 
    Property I/O        : No

    Identification      : TempConstant 
    Description         : Bio Degrad. temperature constant 
    Unit of measurement : 1/C 
    Type                : double 
    Default		: 70.0 
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : TimeConstant 
    Description         : Bio Degrad. time Constant 
    Unit of measurement : 1/Ma 
    Type                : double 
    Default		: 0.5
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C1_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0008 
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C2_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0005 
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C3_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0003
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C4_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0008
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C5_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.001
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : N2_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.001
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : COx_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C6_14Aro_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.007
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C6_14Sat_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.009
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C15Aro_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.05
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C15Sat_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.011
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : resins_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.061
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : asphaltenes_BioFactor 
    Description         : Bio Degradation factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.1
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

#### Diffusion Constants -----------------------------

    Identification      : DiffusionModel
    Description         : Diffusion Model, one of: 'Transient', 'Steady State'
    Type                : string
    Unit of measurement : 
    Default		: "Transient"
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SealThicknessModel
    Description         : Seal Thickness Model, one of: 'FullOverburden', 'LimitedOverburden State'
    Type                : string
    Unit of measurement : 
    Default		: "FullOverburden"
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MaximumSealThickness 
    Description         : maximum seal thickness in case of LimitedOverburden SealThicknessModel
    Unit of measurement : m
    Type                : double 
    Default		: 0
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : DiffSalinity 
    Description         : Salinity
    Unit of measurement : kg/kg
    Type                : double 
    Default		: 0.15
    Error Field         : Yes
    Time dependent      : No 
    Property I/O        : No

    Identification      : ActEnergy 
    Description         : Activation Energy
    Unit of measurement : J
    Type                : double 
    Default		: 18360.0 
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : GasRatio 
    Description         : Diffusion constant
    Unit of measurement : ???? 
    Type                : double 
    Default		: 8.31451
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C1_DiffFactor 
    Description         : Diffusion factor 
    Unit of measurement : 
    Type                : double 
    Default		: 3.047e-06
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C2_DiffFactor 
    Description         : Diffusion factor 
    Unit of measurement : 
    Type                : double 
    Default		: 2.484e-6
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C3_DiffFactor 
    Description         : Diffusion factor 
    Unit of measurement : 
    Type                : double 
    Default		: 2.266e-6
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C4_DiffFactor 
    Description         : Diffusion factor 
    Unit of measurement : 
    Type                : double 
    Default		: 2.036e-6
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C5_DiffFactor 
    Description         : Diffusion factor 
    Unit of measurement : 
    Type                : double 
    Default		: 1.908e-6
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C1_ConcFactor 
    Description         : Concentration factor 
    Unit of measurement : 
    Type                : double 
    Default		: 1
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C2_ConcFactor 
    Description         : Concentration factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.222
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C3_ConcFactor 
    Description         : Concentration factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.089
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C4_ConcFactor 
    Description         : Concentration factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.020
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

    Identification      : C5_ConcFactor 
    Description         : Concentration factor 
    Unit of measurement : 
    Type                : double 
    Default		: 0.0067
    Error Field         : Yes 
    Time dependent      : No 
    Property I/O        : No

#### Diffusion results -----------------------------

    Identification      : MassC1Diffused
    Description         : diffused mass of the specified component. 
    Unit of measurement : kg
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : MassC2Diffused
    Description         : diffused mass of the specified component. 
    Unit of measurement : kg
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : MassC3Diffused
    Description         : diffused mass of the specified component. 
    Unit of measurement : kg
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : PenetrationC1
    Description         : penetration depth of the diffusion front
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : PenetrationC2
    Description         : penetration depth of the diffusion front
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : PenetrationC3
    Description         : penetration depth of the diffusion front
    Unit of measurement : m
    Type                : double 
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : DiffCoefC1
    Description         : Diffusion coefficient
    Unit of measurement : m^2/s
    Type                : double 
    default		: -9999
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

    Identification      : DiffCoefC2
    Description         : Diffusion coefficient
    Unit of measurement : m^2/s
    Type                : double 
    default		: -9999
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DiffCoefC3
    Description         : Diffusion coefficient
    Unit of measurement : m^2/s
    Type                : double
    default		: -9999
    Error Field         : No
    Time dependent      : No 
    Property I/O        : No

#### Reservoir Out Properties -----------------------------

    Identification      : ResRockTrapID
    Description         : trap id to group nodes that belong to the same trap
    Type                : float 
    Unit of measurement : index
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockTop
    Description         : depth of the top of the trap
    Type                : float 
    Unit of measurement : m
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockPorosity
    Description         : top porosity of the reservoir
    Type                : float 
    Unit of measurement : %
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockSealPermeability
    Description         : Permeability of the reservoir seal
    Unit of measurement : mD
    Type                : float
    Error Field         : No 
    Time dependent      : No 
    Property I/O        : No

    Identification      : ResRockFlow
    Description         : HC movement on the top seal of the reservoir
    Type                : float 
    Unit of measurement : log(kg)
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFlux
    Description         : HC movement on the top seal of the reservoir
    Type                : float 
    Unit of measurement : kg/m^2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFillGasPhaseQuantity
    Description         : quantity of gas at each node (trap condition)
    Type                : float 
    Unit of measurement : kg
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFillFluidPhaseQuantity
    Description         : quantity of oil at each node (trap condition)
    Type                : float 
    Unit of measurement : kg
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFillGasPhaseDensity
    Description         : Density of gas at each node (trap condition)
    Type                : float 
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFillFluidPhaseDensity
    Description         : Density of oil at each node (trap condition)
    Type                : float 
    Unit of measurement : kg/m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockFaultCutEdges
    Description         : fault cut index description (see code...)
    Type                : float 
    Unit of measurement : index
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ResRockBottom
    Description         : depth of the bottom of the reservoir
    Type                : float 
    Unit of measurement : m
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
#### Trap Properties -----------------------------

    Identification      : TrapID
    Description         : Reservoir and snapshot specific trap id number
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourceTrapID
    Description         : Reservoir and snapshot specific trap id number
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MigrationProcess
    Description         : Process through which charge was lost: "Spill", ....
    Type                : string
    Unit of measurement :
    Default		: "Spill"
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DestinationTrapID
    Description         : Reservoir and snapshot specific trap id number
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourcePointX
    Description         : Reservoir and snapshot specific trap spill id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SourcePointY
    Description         : Reservoir and snapshot specific trap spill id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DestinationPointX
    Description         : Reservoir and snapshot specific trap spill id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O	: No

    Identification      : DestinationPointY
    Description         : Reservoir and snapshot specific trap spill id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O	: No

    Identification      : LeakPointX
    Description         : Reservoir and snapshot specific reservoir leak id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O	: No

    Identification      : LeakPointY
    Description         : Reservoir and snapshot specific reservoir leak id
    Type                : float 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O	: No
   
    Identification      : WasteLeakPoint
    Description         : Reservoir and snapshot specific reservoir leak id
    Type                : double 
    Unit of measurement : kg
    Error Field         : No
    Time dependent      : No
    Property I/O	: No
   
    Identification      : WCSurface
    Description         : Surface of the OWC/GWC (Oil water contact or Gas water contact)
    Type                : float 
    Unit of measurement : m2
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VolumeOil
    Description         : Volume of the gas phase in a trap
    Type                : float 
    Unit of measurement : m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VolumeGas
    Description         : Volume of the gas phase in a trap
    Type                : float 
    Unit of measurement : m3
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GOC
    Description         : Depth of the Gas/Oil or Water contact
    Type                : float 
    Unit of measurement : m
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OWC
    Description         : Depth of the Oil/Water contact
    Type                : float 
    Unit of measurement : m
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InterfacialTensionGas
    Description         : InterfacialTensionGas
    Unit of measurement : N/m
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InterfacialTensionOil
    Description         : InterfacialTensionOil
    Unit of measurement : N/m
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CriticalTemperatureGas
    Description         : Critical temperature of the vapour hydrocarbon phase
    Unit of measurement : C
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CriticalTemperatureOil
    Description         : Critical temperature of the liquid hydrocarbon phase
    Unit of measurement : C
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InterfacialTensionVapour
    Description         : InterfacialTensionVapour
    Unit of measurement : N/m
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InterfacialTensionLiquid
    Description         : InterfacialTensionLiquid
    Unit of measurement : N/m
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CriticalTemperatureVapour
    Description         : Critical temperature of the vapour hydrocarbon phase
    Unit of measurement : C
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CriticalTemperatureLiquid
    Description         : Critical temperature of the liquid hydrocarbon phase
    Unit of measurement : C
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Permeability
    Description         : seal permeability of the trap formation
    Unit of measurement : mD
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SealPermeability
    Description         : seal permeability of the trap formation
    Unit of measurement : mD
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Porosity
    Description         : porosity of the trap formation
    Unit of measurement : volume%
    Type                : double
    Default		: -1
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MassC1
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC3
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC4
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC5
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassN2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassCOx
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassH2S
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC15Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC15Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassLSC
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC15AT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC15AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC15SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14BT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14DBT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14BP
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassC6_14AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : Massresins
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : Massasphaltenes
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMass
    Description         : mass of the specified phase. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreeGasMassC1
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0


    Identification      : FreeGasMassC3
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0


    Identification      : FreeGasMassC4
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0


    Identification      : FreeGasMassC5
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0


    Identification      : FreeGasMassN2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0


    Identification      : FreeGasMassCOx
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreeGasMassH2S
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC15Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC15Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassLSC
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC15AT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC15AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC15SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14BT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14DBT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14BP
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassC6_14AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassresins
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : FreeGasMassasphaltenes
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMass
    Description         : mass of the specified phase. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC1
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC3
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC4
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC5
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassN2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassCOx
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassH2S
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC15Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC15Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassresins
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassasphaltenes
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassLSC
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC15AT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC15AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC15SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14BT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14DBT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14BP
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : CondensateMassC6_14AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMass
    Description         : mass of the specified phase. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC1
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC3
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC4
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC5
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassN2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassCOx
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassH2S
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC15Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC15Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassresins
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassasphaltenes
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassLSC
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC15AT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC15AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC15SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14BT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14DBT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14BP
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : SolutionGasMassC6_14AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMass
    Description         : mass of the specified phase. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC1
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC3
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC4
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC5
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassN2
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassCOx
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassH2S
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC15Aro
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC15Sat
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassresins
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassasphaltenes
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassLSC
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC15AT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC15AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC15SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14BT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14DBT
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14BP
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14SatS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : StockTankOilMassC6_14AroS
    Description         : mass of the specified component. 
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
	Default				: 0

    Identification      : MassOilSpilled
    Description         : mass of oil spilled from a trap.
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MassGasSpilled
    Description         : mass of gas spilled from a trap.
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MassOilLost
    Description         : mass of oil lost from a trap.
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MassGasLost
    Description         : mass of gas lost from a trap.
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

# Properties for calculating histograms of MC results

    Identification      : Index
    Description         : Index 
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumberOfBins
    Description         : Number of bins in a histogram 
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Key
    Description         : Key  
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MarkerHeight
    Description         : Location on x-axis of bin with index 'Index'
    Unit of measurement : 
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MarkerLocation
    Description         : Horizontal location of marker
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

# Lead and Trapper Properties

    Identification      : LeadID
    Description         : Identifies series of vertically connected traps
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PersistentTrapID
    Description         : Identifies series of vertically connected traps
    Type                : integer 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ReservoirPhase
    Description         : Phase in the reservoir
    Type                : string 
    Unit of measurement : 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GasVolume
    Description         : gass phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilVolume
    Description         : oil phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VolumeVapour
    Description         : vapour phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : VolumeLiquid
    Description         : liquid phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GasDensity
    Description         : gass phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilDensity
    Description         : oil phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DensityVapour
    Description         : vapour phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DensityLiquid
    Description         : liquid phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreeGasVolume
    Description         : free gas phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CondensateVolume
    Description         : condensate phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SolutionGasVolume
    Description         : solution gas phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : StockTankOilVolume
    Description         : stock tank oil phase volume
    Unit of measurement : m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreeGasDensity
    Description         : free gas phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CondensateDensity
    Description         : condensate phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SolutionGasDensity
    Description         : solution gas phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : StockTankOilDensity
    Description         : stock tank oil phase density
    Unit of measurement : kg/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FreeGasViscosity
    Description         : free gas phase density
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CondensateViscosity
    Description         : condensate phase density
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : SolutionGasViscosity
    Description         : solution gas phase density
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : StockTankOilViscosity
    Description         : stock tank oil phase density
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ViscosityVapour
    Description         : vapour phase viscosity
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ViscosityLiquid
    Description         : liquid phase viscosity
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GasViscosity
    Description         : gass phase viscosity
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilViscosity
    Description         : oil phase viscosity
    Unit of measurement : Pa*s
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GasGORM
    Description         : gass phase Gas/Oil Ratio
    Unit of measurement : kg/kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilGORM
    Description         : Oil phase Gas/Oil Ratio
    Unit of measurement : kg/kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GOR
    Description         : Gas/Oil volume factor
    Unit of measurement : m^3/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : FormVolFactor
    Description         : Gas/Oil formation volume factor
    Unit of measurement : m^3/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : API
    Description         : API Gravity
    Unit of measurement : degreesAPI
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilAPI
    Description         : API Gravity
    Unit of measurement : degreesAPI
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : CGR
    Description         : Condensate Gas Ratio
    Unit of measurement : m^3/m^3
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : N2GasRatio
    Description         : Nitrogen C1-C5 Ratio
    Unit of measurement : mass%
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : GasWetness
    Description         : Gas phase wetness 
    Unit of measurement : mole%
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : OilMass
    Description         : mass of oil
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : GasMass
    Description         : mass of gas
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MassLiquid
    Description         : mass of liquid phase
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : MassVapour
    Description         : mass of vapour phase
    Unit of measurement : kg
    Type                : double
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : 0

    Identification      : PropertyUnit
    Description         : name of property
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : ""

    Identification      : GridName
    Description         : Name of a grid
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : ""

# Touchstone Map Properties

    Identification      : TcfName
    Description         : Touchstone tcf filename for output Map
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Category
    Description         : Touchstone Category for output Map
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Format
    Description         : Touchstone Format for output Map
    Unit of measurement : 
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : Percentile
    Description         : Touchstone percent for output Map
    Unit of measurement : 
    Type                : float
    Default		: 100
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PercentUsed
    Description         : Touchstone percent for output Map
    Unit of measurement : 
    Type                : boolean
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : MapDone
    Description         : Touchstone percent for output Map
    Unit of measurement : 
    Type                : boolean
    Default		: 0
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

# Sensitivity User Options

    Identification      : GroupCombination
    Description         : Sensitivity Analysis Sobol input group combinations
    Unit of measurement : 
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
# Obsolete property definitions

    Identification      : FTlength
    Description         : Fission track length
    Unit of measurement : micron
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No

    Identification      : FTage
    Description         : Fission track age
    Unit of measurement : Ma
    Type                : float 
    Error Field         : No
    Time dependent      : Yes
    Property I/O        : No


#### Allochthonous lithology property data --------------------------------


    Identification      : InterpFileName
    Description         : Filename of the interpolation results (with extension)
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : NumberOfPoints
    Description         : The number of points in the interpolation-point set
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : PolynomialDegree
    Description         : The degree of the polynomial used.
    Type                : integer
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : InterpGroupName
    Description         : Name of the group containing the interpolation points, coefficients and rhs in the HDF file.
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#### Crustal Thickness Calculator input

    Identification      : T0Ini
    Description         : Age of onset of rifting
    Type                : double
    Unit of measurement : Ma

    Identification      : T0IniGrid
    Description         : Age of onset of rifting
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TRIni
    Description         : Age of ending of rifting
    Type                : double
    Unit of measurement : Ma
    Default             : 

    Identification      : TRIniGrid
    Description         : Age of ending of rifting
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HCuIni
    Description         : Initial continental crustal thickness
    Type                : double
    Unit of measurement : m
    Default             : 35000

    Identification      : HCuIniGrid
    Description         : Initial continental crustal thickness
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HLMuIni
    Description         : Pre-rifted lithospheric mantle thickness
    Type                : double
    Unit of measurement : m
    Default             : 115000

    Identification      : HLMuIniGrid
    Description         : Pre-rifted lithospheric mantle thickness
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : HBu
    Description         : Maximum thickness of basalt melt
    Type                : double
    Unit of measurement : m
    Default             : 7000

    Identification      : HBuGrid
    Description         : Maximum thickness of basalt melt
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : DeltaSL
    Description         : Relative sealevel adjustment
    Type                : double
    Unit of measurement : m
    Default             : 0.0

    Identification      : DeltaSLGrid
    Description         : Relative sealevel adjustment
    Type                : string
    Unit of measurement :
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

#### Crustal Thickness Calculator and Advanced Lithosphere Calculator output properties 

    Identification      : Moho
    Description		: present-day Moho depth 
    Unit of measurement : m
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AdjustedWaterLoadedSubsidence
    Description		: present-day water loaded subsidence
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : AdjustedResidualDepthAnomaly
    Description		: present-day calculated residual depth anomaly
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ContinentalCrustalThickness
    Description		: present-day continental crustal thickness
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : EffectiveCrustalThickness
    Description		: present-day effective crust thickness
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : TopBasaltUnderplate
    Description		: present-day depth to top basalt
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : ALCMaxAsthenoMantleDepth
    Description		: maximum depth of asthenospheric mantle
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCStepTopBasaltDepth
    Description		: depth of top of lithoswitched basalt
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCStepMohoDepth
    Description		: Moho, base of lithoswitched basalt or continental crust
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCStepContCrustThickness
    Description		: thickness of continental crust after lithoswitching
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCStepBasaltThickness
    Description		: calculated after lithoswitching thickness of basalt
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCSmBasaltThickness
    Description		: smooth basalt thickness
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
 
    Identification      : ALCSmContCrustThickness
    Description		: interpolated continental crust thickness
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCSmTopBasaltDepth
    Description		: calculated depth of top of basalt
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCSmMohoDepth
    Description		: calculated depth of Moho
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
    Identification      : ALCOrigLithMantleDepth
    Description		: base of uplifted initial mantle
    Unit of measurement : m
    Type                : float
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    
#### Shale Gas properties

START-properties

    Identification      : ApplyAdsorption
    Description         : Whether to apply adsorption on a source rock
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : FALSE

    Identification      : AdsorptionTOCDependent
    Description         : Whether to apply a TOC-dependent adsorption capacity function
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : FALSE

    Identification      : ComputeOTGC
    Description         : Whether to also perform otgc i adsorption is applied
    Type                : boolean
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Default             : FALSE

    Identification      : AdsorptionCapacityFunctionName
    Description         : Name of the adsorption function to use
    Type                : string 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : WhichAdsorptionSimulator
    Description         : Name of the adsorption simulator to use
    Type                : string 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LangmuirName
    Description         : Name of a Langmuir object
    Type                : string 
    Error Field         : No
    Time dependent      : No
    Property I/O        : No

    Identification      : LangmuirTemperature
    Description         : 
    Unit of measurement : C (degCentigrade)
    Type                : double 

    Identification      : ReferenceTemperature
    Description         : 
    Unit of measurement : C (degCentigrade)
    Type                : double 

    Identification      : VLTemperatureGradient
    Description         : 
    Unit of measurement : cc/g/C (cubic-centimetre/gram-centigrade)
    Type                : double 

    Identification      : VLCoeffA
    Description         : 
    Unit of measurement : 
    Type                : double 

    Identification      : VLCoeffB
    Description         : 
    Unit of measurement : 
    Type                : double 

    Identification      : LangmuirPressure
    Description         : Pressure at which half the Langmuir volume is filled.
    Unit of measurement : MPa
    Type                : double 

    Identification      : LangmuirVolume
    Description         : 
    Unit of measurement : cc
    Type                : double 

    Identification      : XPlanePosition
    Description         : Position of x-point in plane
    Unit of measurement : m
    Type                : double 

    Identification      : YPlanePosition
    Description         : Position of y-point in plane
    Unit of measurement : m
    Type                : double 

    Identification      : AdsorptionFormationName
    Description         : Name of formation for which history is required. 
    Type                : string

    Identification      : CoefficientA
    Description         : Slope (linear term) of irreducible water saturation line.
    Type                : double
    Default             : -0.187

    Identification      : CoefficientB
    Description         : Constant term of irreducible water saturation line.
    Type                : double
    Default             : 1.18

    Identification      : SGMeanBulkDensity
    Description         : The density of the inorganic part
    Type                : double
    Default             : 2300.0


#    Identification      : MapRowIndex
#    Description         : Indicate which row of the map is required
#    Type                : Integer

#    Identification      : MapColumnIndex
#    Description         : Indicate which column of the map is required
#    Type                : Integer

#    Identification      : AdsorptionNodeOutputSubscript
#    Description         : The name of the output subscript
#    Type                : string

#### BPA Name/Description/etc mapping table to preserve BPA-specific properties on roundtripped projects

    Identification      : TblIoMappingEncode
    Description         : Mapping encode of the table/column containing a value that needs a BPA name/description
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Identification      : BPAColumnName
    Description         : The BPA column name for the BPAColumnValue
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
    Identification      : BPAColumnValue
    Description         : The BPA value to use (preserve)
    Type                : string
    Error Field         : No
    Time dependent      : No
    Property I/O        : No
END-properties

##########################################################################
 
START-property-tables

#### Project tables ----------------------------------------------------

    Identification : ProjectIoTbl
    Description    : table with general project data
    Column         : ProjectName
    Column         : ProjectSize
    Column         : Description
    Column         : ProjectTeam
    Column         : ProjectStartDate
    Column         : ProjectEndDate
#    Column         : BPAServer
#    Column         : BPAName
#    Column         : BPAOwnerOrg
#    Column         : BPAChecksum
    Column         : ProgramVersion
    Column         : ModellingMode
    Column         : XCoord
    Column         : YCoord
    Column         : NumberX
    Column         : NumberY
    Column         : DeltaX
    Column         : DeltaY
    Column         : ScaleX
    Column         : ScaleY
    Column         : OffsetX
    Column         : OffsetY
    Column	   : WindowXMin
    Column	   : WindowXMax
    Column	   : WindowYMin
    Column	   : WindowYMax
    Column         : WktGeometryOutline
    Header         : Project data
    Time dependent : No
    Property I/O   : Yes

    Identification : ProjectNotesIoTbl
    Description    : table with the project notes (multi line!!)
    Column         : ProjectNotes
    Header         : Project data
    Time dependent : No
    Property I/O   : Yes

    Identification : RelatedProjectsIoTbl
    Description    : table with the filenames of related projects (3d --o 1d)
    Column         : Filename
    Header         : Project data
    Time dependent : No
    Property I/O   : Yes
    Identification : BPANameMapping
    Column         : TblIoMappingEncode
    Column         : BPAColumnName
    Column         : BPAColumnValue

    Identification : DefaultFileIoTbl
    Description    : default filenames + extensions per (logical) dataset
    Column         : Filename
    Column         : Fileextension
    Column         : Dataset
    Header         : Default files  
    Time dependent : No
    Property I/O   : Yes

    Identification : IoOptionsIoTbl
    Description    : table with file io options
    Column         : MapType
    Column         : FileVersioning
    Column         : AutoSaving
    Column         : SaveBeforeEachRun
    Column         : MaxFileWidth
    Column         : ConsoleLevel
    Column         : PopupLevel
    Column         : LogfileLevel
    Time dependent : No
    Property I/O   : Yes

    Identification : RunOptionsIoTbl
    Description    : table with run options data
    Column         : CompactionAlgorithm
    Column         : VreAlgorithm
    Column         : VelAlgorithm
    Column         : Optimisation_Level
    Column         : Temperature_Gradient
    Column         : Constrain_VES
    Column         : HeatFlowSolution
    Column         : PrefSnapstep
    Column         : StepsPerSegment
    Column         : TempDiffBasedStepping
    Column         : PrefReconstep
    Column         : OptimalTotalPresDiff
    Column         : OptimalTotalTempDiff
    Column         : OptimalSourceRockTempDiff
    Column         : MaxTimeStepIncreaseFactor
    Column         : MinTimeStepDecreaseFactor
    Column         : BricksizeEast
    Column         : BricksizeNorth
    Column         : BrickHeightSediment  
#			BrickHeightBasement -> replaced by the 2 below:
    Column         : BrickHeightCrust
    Column         : BrickHeightMantle
    Column         : MaxNumberOfRunOverpressure
    Column         : FracturePressureFunction
    Column         : FractureType
    Column         : FractureModel
    Column         : CalcAllDepthIoProps
    Column         : UncertaintyAnalysis
    Column         : SensitivityAnalysis
    Column         : ResponseSurfaceModeling
    Column         : NrMCLoops
    Column         : NrMCSaveEvery
    Column         : NrSensSaveEvery
    Column         : Optimization
    Column         : OptimTopCrustHeatProd
    Column         : OptimCrustalThickness
    Column         : OptimMantleThickness
    Column         : OptimUnconfThickness
    Column         : FormTempOptimWeight
    Column         : VrmOptimWeight
    Column         : BiomOptimWeight
    Column         : FtOptimWeight
    Column         : SmiOptimWeight
    Column         : OptimTolerance
    Column         : Hims
    Column         : Him
    Column         : NumEstimates
    Column         : WorkflowType
    Column         : PTCouplingMode
    Column         : CoupledMode
    Column         : ChemicalCompaction
    Column         : ConvectiveTerm
    Column         : NonGeometricLoop
    Column         : AllochthonousModelling
    Column         : McHistOutput
    Column         : RSMxmlOutput
    Column         : ApplyOtgcToDarcy
    Column         : DarcyMaxTimeStep
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

    Identification : SnapshotIoTbl
    Description    : table with snapshot data (3D only)
    Column         : Time
    Column         : IsMinorSnapshot
    Column         : TypeOfSnapshot
    Column         : SnapshotFileName
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

    # Changed by S. Mathijssen (ORIGIN) 24-11-1997
    # Increment 1.1 (3D)
    # New column ModellingMode added.
    Identification : SensAnalysisIoTbl
    Description    : table with parameters for the sensitivity analysis
    Column         : AnalysisIdentification
    Column         : AnalysisIndicator
    Column         : PropertyDescription
    Column         : TypeOfDistribution
    Column         : ModellingMode
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

    Identification : RespSurfModelParamsIoTbl
    Description    : table with parameters for response surface modelling
    Column         : AnalysisIdentification
    Column         : PrimaryRow
    Column         : SecondaryRow
    Column         : RSMparameter
    Column         : RSMnumber
    Column         : RSMparameterValue
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes


    # Changed by S. Mathijssen (ORIGIN) 05-11-1997
    # Increment 1.2
    Identification : FilterDepthIoTbl
    Description    : Determines which properties needs to be present
                     within the DepthIoTbl
                     and the DepthMCResultIoTbl
    Column         : PropertyName
    Column         : ModellingMode
    Column         : OutputOption
    Column         : ResultOption
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

    # Changed by S. Mathijssen (ORIGIN) 05-11-1997
    # Increment 1.2
    Identification : FilterTimeIoTbl
    Description    : Determines which properties needs to be present
                     within the TimeIoTbl
                     and the TimeMCResultIoTbl
    Column         : PropertyName
    Column         : ModellingMode
    Column         : OutputOption
    Column         : ResultOption
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

    Identification : FilterTimeDepthIoTbl
    Description    : Determines which properties needs to be present
                     within the HistogramMcResultIoTbl
    Column         : PropertyName
    Column         : PropertyClass
    Column         : SurfaceName
    Column         : FormationName
    Column         : Time
    Column         : ResultOption
    Column         : NumberOfBins
    Header         : Run Options
    Time dependent : No
    Property I/O   : Yes

#### Project - 1d tables --------------------------------------------------

    Identification : WellLocIoTbl
    Description    : table with location data for IO and UI
    Column         : WellName
    Column         : XCoord
    Column         : YCoord
    Column         : Longitude
    Column         : Latitude
    Column         : Datum
    Column         : WellElevation
    Column         : DerFloorElev
    Column         : Waterdepth
    Header         : Location Data  
    Time dependent : No
    Property I/O   : Yes

#### Project - 1d tables --------------------------------------------------

    Identification : TouchstoneWellIoTbl
    Description    : Table with Well location data for Touchstone
    Column         : WellName
    Column         : XCoord
    Column         : YCoord    
    Column         : BHFName    
    Header         : Touchstone Well Data  
    Time dependent : No
    Property I/O   : Yes

#### Project - 3d tables --------------------------------------------------

    Identification : GridMapIoTbl
    Description    : table with grid map data, KEYS: ReferredBy+MapName
    Column         : ReferredBy
    Column         : MapName
    Column         : MapType
    Column         : MapFileName
    Column         : FileId
    Column         : MD5Checksum
    Column         : MapSeqNbr
    Column         : HDF5FileName
    Header         : Grid Map Data
    Time dependent : No
    Property I/O   : Yes

#### Stratigraphic data --------------------------------------------------

    Identification : StratIoTbl
    Description    : table with lithostratigraphic data for IO and UI
#  Surface properties:
    Column         : SurfaceName
    Column         : DepoAge
    Column         : Depth
    Column         : ErrDepth
    Column         : DepthGrid
    Column         : TwoWayTtime
#  Layer properties:
    Column         : LayerName
    Column         : DepoSequence
    Column         : Thickness
#  The next two entries for unconformities only
    Column         : CalibThickness
    Column         : OptimThickness
    Column         : ThicknessGrid
#leave ui and io limited to 3 layers only:
    Column         : Lithotype1
    Column         : Percent1
    Column         : Percent1Grid
    Column         : Lithotype2
    Column         : Percent2
    Column         : Percent2Grid
    Column         : Lithotype3
    Column         : Percent3
    Column         : MixModel
    Column         : SeisVelocity
    Column         : Fluidtype
    Column         : TempGradient
    Column         : SourceRock
    Column         : MobileLayer
    Column         : HasAllochthonLitho
    Column         : HydroSand
    Column         : ChemicalCompaction
#  SourceRock Properties:
    Column         : EnableSourceRockMixing
    Column         : SourceRockType1
    Column         : SourceRockType2
    Column         : SourceRockMixingHI
    Column         : SourceRockMixingHIGrid
    Column         : ElementRefinementZ
    Column         : IncludeInDarcy
    Column         : IsIgneousIntrusion
    Column         : IgneousIntrusionAge
    Column         : IgneousIntrusionTemperature

    Header         : Stratigraphic Data  
    Time dependent : No
    Property I/O   : Yes

    Identification : MobLayThicknIoTbl
    Description    : Table of 'mobile' layer thicknesses through time
    Column         : LayerName
    Column         : Age
    Column         : Thickness
    Column         : ThicknessGrid
    Header         : Stratigraphic Data  
    Time dependent : No
    Property I/O   : Yes

    Identification : AllochthonLithoIoTbl
    Description    : Table of formations involved in the modelling of allochthonous lithologies
    Column         : LayerName
    Column         : Lithotype
    Header         : Formation Data  
    Time dependent : No
    Property I/O   : Yes

    Identification : AllochthonLithoDistribIoTbl
    Description    : Table of maps involved in the modelling of allochthonous lithologies for all layers
    Column         : LayerName
    Column         : DistributionGrid
    Column         : Age
    Header         : Formation Data  
    Time dependent : No
    Property I/O   : Yes

    Identification : AllochthonLithoInterpIoTbl
    Description    : Table of results of allochthonous lithology interpolation.
    Column         : LayerName
    Column         : InterpFileName
    Column         : NumberOfPoints
    Column         : PolynomialDegree
    Column         : InterpGroupName
    Column         : StartAge
    Column         : EndAge
    Header         : Formation Data  
    Time dependent : No
    Property I/O   : Yes


    Identification : CrustIoTbl
    Description    : table with Crust thickness data for IO and UI
    Column         : Age
    Column         : Thickness
    Column         : ThicknessGrid
    Column         : CalibThickness
    Column         : OptimThickness
    #limited to 1 layer only:
    Header         : Crust Thickness Data  
    Time dependent : No
    Property I/O   : Yes
    
    Identification : ContCrustalThicknessIoTbl
    Description    : table with Crust thickness data for IO and UI
    Column         : Age
    Column         : Thickness
    Column         : ThicknessGrid
    Column         : CalibThickness
    Column         : OptimThickness
    #limited to 1 layer only:
    Header         : Continental Crust Thickness Data  
    Time dependent : No
    Property I/O   : Yes
    
    Identification : BasaltThicknessIoTbl
    Description    : table with Basalt thickness data for IO and UI
    Column         : BasaltThickness
    Column         : BasaltThicknessGrid
    Column         : CrustThicknessMeltOnset
    Column         : CrustThicknessMeltOnsetGrid
    #limited to 1 layer only:
    Header         : Basalt Thickness Data  
    Time dependent : No
    Property I/O   : Yes

 # Source Rock data IO:

    Identification : SourceRockLithoIoTbl
    Description    : Table with general and material (single) properties of
                     each source rock lithotype
    Column         : LayerName
    Column         : SourceRockType
    Column         : BaseSourceRockType
    Column         : TocIni
    Column         : TocIniGrid
    Column         : HCInputMode
    Column         : S1Ini
    Column         : S1IniGrid
    Column         : S2Ini
    Column         : S2IniGrid
    Column         : S3Ini
    Column         : S3IniGrid
    Column         : HcVRe05
    Column         : ScVRe05
    Column         : HcIni
    Column         : HcIniGrid
    Column         : HiIni
    Column         : OcIni
    Column         : OcIniGrid
    Column         : NcIni
    Column         : NcIniGrid
    Column         : CharLength
    Column         : CharLengthGrid
    Column         : UpperBiot
    Column         : UpperBiotGrid
    Column         : LowerBiot
    Column         : LowerBiotGrid
    Column         : KerogenStartAct
    Column         : KerogenStartActGrid
    Column         : KerogenEndAct
    Column         : KerogenEndActGrid
    Column         : PreAsphaltStartAct
    Column         : PreAsphaltStartActGrid
    Column         : PreAsphaltEndAct
    Column         : PreAsphaltEndActGrid
    Column         : NettThickIni
    Column         : NettThickIniGrid
    Column         : NGenexTimeSteps
    Column         : NGenexTimeStepsGrid
    Column         : NGenexSlices
    Column         : NGenexSlicesGrid
    Column         : VREini
    Column         : AsphalteneDiffusionEnergy
    Column         : ResinDiffusionEnergy
    Column         : C15AroDiffusionEnergy
    Column         : C15SatDiffusionEnergy
    Column         : VREoptimization
    Column         : VREthreshold
    Column         : VESLimitIndicator
    Column         : VESLimit
    Column         : ApplyAdsorption
    Column         : AdsorptionTOCDependent
    Column         : ComputeOTGC
    Column         : AdsorptionCapacityFunctionName
    Column         : WhichAdsorptionSimulator
    Header         : Stratigraphic Data  
    Time dependent : No
    Property I/O   : Yes

# Data for CrustalThicknessCalculator

    Identification : CTCIoTbl 
    Description    : Table of user defined data for calculation of MOHO and basalt thickness
    Column         : SurfaceName
    Column         : T0Ini	  
    Column         : T0IniGrid
    Column         : TRIni
    Column         : TRIniGrid
    Column         : HCuIni
    Column         : HCuIniGrid
    Column         : HLMuIni
    Column         : HLMuIniGrid
    Column         : HBu
    Column         : HBuGrid
    Column         : DeltaSL
    Column         : DeltaSLGrid
    Header         : Crustal Thickness Calculator input data  
    Time dependent : No
    Property I/O   : Yes

# Boundary condition data -------------------------------------------------
    
    Identification : GeologicalBoundaryIoTbl
    Description    : Table of boundary values Through Time 
    Column         : GeologicalObjectName
    Column         : GeologicalObjectType
    Column         : TypeOfProperty
    Column         : FirstIndexInPropertyIoTbl
    Column         : NumberOfProperties
    Header         : Geological Boundary datas
    Time dependent : No
    Property I/O   : Yes

    Identification : PropertyBoundaryIoTbl
    Description    : Table of boundary values Through Time 
    Column         : NameOfProperty
    Column         : FirstIndexInPropertyBoundaryValuesIoTbl
    Column         : NumberOfTimeIntervals
    Header         : Property Boundary datas
    Time dependent : No
    Property I/O   : Yes

    Identification : BoundaryValuesIoTbl
    Description    : Table of boundary values Through Time 
    Column         : BeginTimeValues
    Column         : EndTimeValues	
    Column         : PropertyValueGrid
    Column         : PropertyValue
    Header         : Boundary values datas
    Time dependent : No
    Property I/O   : Yes

    Identification : SurfaceTempIoTbl
    Description    : Table of surface temperatures through time 
    Column         : Age
    Column         : Temperature
    Column         : ErrTemperature
    Column         : TemperatureGrid
    Header         : Surface Temperatures
    Time dependent : No
    Property I/O   : Yes

    Identification : PermafrostIoTbl
    Description    : Table of Permafrost simulation parameters 
    Column         : PermafrostInd
    Column         : PressureTerm
    Column	   : SalinityTerm
    Header         : Permafrost parameters
    Time dependent : No
    Property I/O   : Yes

    Identification : SurfaceDepthIoTbl
    Description    : Table of surface depths through time 
    Column         : Age
    Column         : Depth
    Column         : DepthGrid
    Header         : Surface Depths
    Time dependent : No
    Property I/O   : Yes

    Identification : MntlHeatFlowIoTbl
    Description    : Table of mantle heat flow through time 
                     (for transient program option)
    Column         : Age
    Column         : HeatFlow
    Column         : HeatFlowGrid
    Header         : Mantle Heat Flows
    Time dependent : No
    Property I/O   : Yes

    Identification : BasementIoTbl
    Description    : basement
    Column         : BottomBoundaryModel
    Column         : TopAsthenoTemp
    Column         : TopCrustHeatProd
    Column         : TopCrustHeatProdGrid
    Column         : CalibTopCrustHeatProd
    Column         : CrustHeatPDecayConst
    Column         : LithoMantleThickness
    Column         : CalibLithoMantleThickness
    Column         : InitialLthMntThickns
    Column         : CalibInitialLthMntThickns
    Column         : FixedCrustThickness
    Column         : CalibFixedCrustThickness
    Column         : InitialLithosphericMantleThickness
    Column	   : CrustPropertyModel
    Column	   : MantlePropertyModel
    Header         : Basement
    Time dependent : No
    Property I/O   : Yes

#    Identification : SealevelIoTbl
#    Description    : Table of sealevel through time 
#    Column         : Sealevel
#    Column         : Age
#    Header         : Sea Levels
#    Time dependent : No
#    Property I/O   : Yes


#### Calibration / Thermal Indicator Data -----------------------------------
# Measured Pressures:

    Identification : FormPressIoTbl
    Description    : Measured Pressures for a well
                     (no correction or after correction) 
    Column         : DepthIndex
    Column         : MPressure
    Column         : FormPressureType
    Column         : Label
    Header         : Measured Pressures 
    Time dependent : No
    Property I/O   : Yes

   

# temperatures /BHT:



    Identification : FormTempIoTbl
    Description    : Corrected temperature data for a well
                     (no correction or after correction) 
    Column         : DepthIndex
    Column         : Temperature
    Column         : ErrTemperature
    Column         : FormTempType
    Column         : Label
    Column         : Optimization
    Header         : Formation Temperatures (corrected)
    Time dependent : No
    Property I/O   : Yes

    Identification : RawBhtIoTbl
    Description    : Measuered Bottom hole temperature data at depth for well
                     after ciculation time, before correction (CTCYM/Horner) 
    Column         : DepthIndex
    Column         : TimeSinceCirStopped
    Column         : Temperature
    Column         : ErrTemperature
    Column         : Theta
    Header         : Bottom Hole Temperatures (uncorrected)
    Time dependent : No
    Property I/O   : Yes

    Identification : RawBhtBitsizeIoTbl
    Description    : Additional data at depth used for the calculation of
                     corrected well temperatures from the Bottom hole
                     temperature 
    Column         : DepthIndex
    Column         : CirculationTime
    Column         : BitDiameter
    Column         : FormTempType
    Header         : Bottom Hole Temperatures (uncorrected)
    Time dependent : No
    Property I/O   : Yes

# Bht correction with Marsden & Schreurs polynomial trend fitting
    
    Identification : MarsdenCoefIoTbl
    Description    : Data for Marsden & Schreurs polynomial trend 
                     fitting method for bottom hole temperature correction
    Column         : MarsdenAcoef
    Column         : MarsdenBcoef
    Column         : MarsdenCcoef
    Header         : Marsden & Schreurs polynom coefficients
    Time dependent : No
    Property I/O   : Yes

# Vitrinite reflectance:

    Identification : VrmIoTbl
    Description    : Measured Vitrinite Reflectance for well
    Column         : DepthIndex
    Column         : Vr
    Column         : VrmType
    Column         : Label
    Column         : Optimization
    Header         : Vitrinite Reflectance Measurements
    Time dependent : No
    Property I/O   : Yes

# Sonic Velocity:

    Identification : SonicIoTbl
    Description    : Measured Sonic Velocity for well from LOGIC application 
    Column         : Depth
    Column         : Dt
    Header         : Sonic Velocity Measurements
    Time dependent : No
    Property I/O   : Yes

# Bulk Density:

    Identification : BulkDensityIoTbl
    Description    : Measured Density log for well from LOGIC application 
    Column         : Depth
    Column         : BDensity
    Header         : Bulk Density Measurements
    Time dependent : No
    Property I/O   : Yes
# Biomarker  :

    Identification : BiomarkerKinIoTbl
    Description    : 
    Column         : HopIsoActEnergy
    Column         : SteIsoActEnergy
    Column         : SteAroActEnergy
    Column         : HopIsoFreqFactor
    Column         : SteIsoFreqFactor
    Column         : SteAroFreqFactor
    Column         : HopIsoGamma
    Column         : SteIsoGamma
    Header         : Biomarker Kinetics
    Time dependent : No
    Property I/O   : Yes

    Identification : BiomarkermIoTbl
    Description    : Measured biomarker data for well
    Column         : DepthIndex
    Column         : HopaneIsomerisation
    Column         : SteraneIsomerisation
    Column         : SteraneAromatisation
    Column         : Optimization
    Header         : Biomarker Measurements
    Time dependent : No
    Property I/O   : Yes

# Smectite Illite  :

    Identification : SmectiteIlliteKinIoTbl
    Description    : 
    Column         : ActEnergy1
    Column         : FreqFactor1
    Column         : ActEnergy2
    Column         : FreqFactor2
    Column         : InitIlliteFraction
    Header         : Smectite Illite Kinetics
    Time dependent : No
    Property I/O   : Yes

    Identification : SmectiteIlliteIoTbl
    Description    : Measured smectite illite data for well
    Column         : DepthIndex
    Column         : IlliteFraction
    Column         : Label
    Column         : Optimization
    Header         : Smectite Illite Measurements
    Time dependent : No
    Property I/O   : Yes


# Fission track :

    Identification : FtGrainIoTbl
    Description    : Spontaneous and induced track count per grain
    Column         : FtSampleId
    Column         : FtGrainId
    Column         : FtSpontTrackNo
    Column         : FtInducedTrackNo
    Column         : FtClWeightPerc
    Column         : FtGrainAge
    Column         : FtGrainAgeErr
    Header         : Fission track counts
    Time dependent : No
    Property I/O   : Yes

    Identification : FtLengthIoTbl
    Description    : Fission track length measurements
    Column         : FtSampleId
    Column         : FtGrainId
    Column         : FtLength
    Header         : Fission track lengths
    Time dependent : No
    Property I/O   : Yes

    Identification : FtSampleIoTbl
    Description    : Fission track sample data
    Column         : FtSampleId
    Column         : DepthIndex
    Column         : FtZeta
    Column         : FtUstglTrackDensity
    Column         : FtPredictedAge
    Column         : FtPooledAge
    Column         : FtPooledAgeErr
    Column         : FtAgeChi2
    Column         : FtDegreeOfFreedom
    Column         : FtPAgeChi2
    Column         : FtCorrCoeff
    Column         : FtVarianceSqrtNs
    Column         : FtVarianceSqrtNi
    Column         : FtNsDivNi
    Column         : FtNsDivNiErr
    Column         : FtMeanRatio
    Column         : FtMeanRatioErr
    Column         : FtCentralAge
    Column         : FtCentralAgeErr
    Column         : FtMeanAge
    Column         : FtMeanAgeErr
    Column         : FtLengthChi2
    Column         : FtApatiteYield
    Column         : Optimization
    Header         : Fission track sample data
    Time dependent : No
    Property I/O   : Yes

    #Fission track new tables for Cauldron/BPA integration

    Identification : FtPredLengthCountsHistIoTbl
    Description    : Description of layout for the histograms containing the predicted fission track length counts
    Column         : FtPredLengthHistId 
    Column         : FtSampleId
    Column         : FtClWeightPerc
    Column         : FtPredLengthBinStart 
    Column         : FtPredLengthBinWidth 
    Column         : FtPredLengthBinNum 
    Header         : Layout of fission track predicted histograms 
    Time dependent : No
    Property I/O   : Yes

    Identification : FtPredLengthCountsHistDataIoTbl
    Description    : Histogram data (predicted counts per track length bin) of histograms defined in FtPredLengthCountsHistIoTbl
    Column         : FtPredLengthHistId 
    Column         : FtPredLengthBinIndex
    Column         : FtPredLengthBinCount
    Header         : Histogram data (predicted counts per track length bin) of histograms defined in FtPredLengthCountsHistIoTbl 
    Time dependent : No
    Property I/O   : Yes

    Identification : FtClWeightPercBinsIoTbl
    Description    : Layout of the Cl Weight Bins used in the fission track prediction algorithms
    Column         : FtClWeightBinStart 
    Column         : FtClWeightBinWidth
    Header         : Layout of the Cl Weight Bins used in the fission track prediction algorithms
    Time dependent : No
    Property I/O   : Yes


# XMIG stuff

    Identification : ReservoirIoTbl
    Description    : Table listing the list of reservoirs and
                     their properties
    Column         : ReservoirName
    Column         : FormationName
    Column         : TrapCapacity
    Column         : ActivityMode
    Column         : ActivityStart
    Column         : DepthOffset
    Column         : DepthOffsetGrid
    Column         : Thickness
    Column         : ThicknessGrid
    Column         : NetToGross
    Column         : NetToGrossGrid
    Column         : MicroTraps
    Column         : LeakProperty
    Column         : LeakRate
    Column         : Lithotype1
    Column         : Percent1
    Column         : Percent1Grid
    Column         : Lithotype2
    Column         : Percent2
    Column         : Percent2Grid
    Column         : Lithotype3
    Column         : LayerFrequency 
    Column         : LayerFrequencyGrid
    Column         : BioDegradInd 
    Column         : OilToGasCrackingInd 
    Column         : DiffusionInd 
    Column         : BlockingInd 
    Column         : BlockingPermeability 
    Header         : Reservoir Data
    Time dependent : No
    Property I/O   : Yes

    Identification : BioDegradIoTbl
    Description    : This table holds the bio degradation parameters 
    Column         : MaxBioTemp 
    Column         : TempConstant 
    Column         : TimeConstant 
    Column         : C1_BioFactor 
    Column         : C2_BioFactor 
    Column         : C3_BioFactor 
    Column         : C4_BioFactor 
    Column         : C5_BioFactor 
    Column         : N2_BioFactor 
    Column         : COx_BioFactor 
    Column         : C6_14Aro_BioFactor 
    Column         : C6_14Sat_BioFactor 
    Column         : C15Aro_BioFactor 
    Column         : C15Sat_BioFactor 
    Column         : resins_BioFactor 
    Column         : asphaltenes_BioFactor 
    Header         : Bio Degradation Data 
    Time dependent : No
    Property I/O   : Yes

    Identification : DiffusionIoTbl
    Description    : This table holds the diffusion parameters 
    Column         : DiffusionModel 
    Column         : SealThicknessModel 
    Column         : MaximumSealThickness
    Column         : DiffSalinity 
    Column         : ActEnergy 
    Column         : GasRatio 
    Column         : C1_DiffFactor 
    Column         : C2_DiffFactor 
    Column         : C3_DiffFactor 
    Column         : C4_DiffFactor 
    Column         : C5_DiffFactor 
    Column         : C1_ConcFactor 
    Column         : C2_ConcFactor 
    Column         : C3_ConcFactor 
    Column         : C4_ConcFactor 
    Column         : C5_ConcFactor 
    Header         : Diffusion Data 
    Time dependent : No
    Property I/O   : Yes

    Identification : PalinspasticIoTbl
    Description    : Table listing the palinspastic maps and
                     their properties
    Column         : DepthGrid
    Column         : FaultcutsMap
    Column         : SurfaceName
    Column         : BottomFormationName
    Column         : Age
    Column         : Depth
    Column         : MapSeqNbr
    Column         : OilSlope 
    Column         : GasSlope 
    Column         : MinimumSGR 
    Column         : SGRIndicator 
    Header         : Palinspastic Map Data
    Time dependent : No
    Property I/O   : Yes

    Identification : FaultcutIoTbl
    Description    : Table listing faultcuts and their status
    Column         : SurfaceName
    Column         : FaultName
    Column         : FaultcutStatus
    Column         : Age
    Header         : Migration Faultcut status data
    Time dependent : No
    Property I/O   : Yes

    Identification : PressureFaultcutIoTbl
    Description    : Table listing of the faults used in overpressure
    Column         : FaultcutsMap
    Column         : FaultName
    Column         : StartAge
    Column         : FaultLithology
    Column         : UsedInOverpressure
    Header         : Overpressure Faultcut status data
    Time dependent : No
    Property I/O   : Yes

    Identification : PressureFuncIoTbl
    Description    : Table listing fracture pressure functions
    Column         : PresFuncName
    Column         : CoefA
    Column         : CoefB
    Column         : CoefC
    Column         : CoefD
    Column         : Selected
    Column         : UserDefined
    Header         : Fracture Pressure Functions against Depth
    Time dependent : No
    Property I/O   : Yes

    Identification : TrapIoTbl
    Description    : Table listing traps (information through time, read-only)
    Column         : ReservoirName
    Column         : Age
    Column         : TrapID
    Column         : XCoord
    Column         : YCoord
    Column         : TrapCapacity
    Column         : Depth
    Column         : SpillDepth
    Column         : SpillPointXCoord
    Column         : SpillPointYCoord
    Column         : WCSurface
    Column         : Temperature
    Column         : Pressure
    Column         : Permeability
    Column         : SealPermeability
    Column         : NetToGross
    Column         : CEPGas
    Column         : CEPOil
    Column         : CriticalTemperatureGas
    Column         : CriticalTemperatureOil
    Column         : InterfacialTensionGas
    Column         : InterfacialTensionOil
    Column         : FractSealStrength
    Column         : FracturePressure

    Column         : GOC
    Column         : OWC
    Column         : VolumeGas
    Column         : VolumeOil
    Column         : MassC1
    Column         : MassC2
    Column         : MassC3
    Column         : MassC4
    Column         : MassC5
    Column         : MassN2
    Column         : MassCOx
    Column         : MassH2S
    Column         : MassC6_14Aro
    Column         : MassC6_14Sat
    Column         : MassC15Aro
    Column         : MassC15Sat
    Column         : MassLSC
    Column         : MassC15AT
    Column         : MassC15AroS
    Column         : MassC15SatS
    Column         : MassC6_14BT
    Column         : MassC6_14DBT
    Column         : MassC6_14BP
    Column         : MassC6_14SatS
    Column         : MassC6_14AroS
    Column         : Massresins
    Column         : Massasphaltenes
    Header         : Trap Data
    Time dependent : No
    Property I/O   : Yes

    Identification : LeadIoTbl
    Description    : Table listing of traps organized by leads, present day only
    Column         : LeadID
    Column         : ReservoirName
    Column         : TrapID
    Column         : ReservoirPhase
    Column         : XCoord
    Column         : YCoord
    Column         : Depth
    Column         : WCSurface
    Column         : GOC
    Column         : OWC
    Column         : Temperature
    Column         : Pressure
    Column         : Porosity
    Column         : SealPermeability
    Column         : CEPGas
    Column         : CEPOil
    Column         : GasMass
    Column         : OilMass
    Column         : GasVolume
    Column         : OilVolume
    Column         : GasDensity
    Column         : OilDensity
    Column         : GasViscosity
    Column         : OilViscosity
    Column         : GasGORM
    Column         : OilGORM
    Column         : FormVolFactor
    Column         : CGR
    Column         : N2GasRatio
    Column         : GasWetness
    Column         : API
    Header         : Lead Data
    Time dependent : No
    Property I/O   : Yes

    Identification : TrapperIoTbl
    Description    : Table listing of traps for trapper
    Column         : PersistentTrapID
    Column         : ReservoirName
    Column         : Age
    Column         : TrapID
    Column         : XCoord
    Column         : YCoord
    Column         : TrapCapacity
    Column         : Depth
    Column         : SpillDepth
    Column         : SpillPointXCoord
    Column         : SpillPointYCoord
    Column         : Pressure
    Column         : Temperature
    Column         : Permeability
    Column         : SealPermeability
    Column         : Porosity
    Column         : NetToGross
    Column         : InterfacialTensionVapour
    Column         : CriticalTemperatureVapour
    Column         : CEPVapour
    Column         : MassVapour
    Column         : VolumeVapour
    Column         : ViscosityVapour
    Column         : DensityVapour
    Column         : InterfacialTensionLiquid
    Column         : CriticalTemperatureLiquid
    Column         : CEPLiquid
    Column         : MassLiquid
    Column         : VolumeLiquid
    Column         : ViscosityLiquid
    Column         : DensityLiquid
    Column         : OilAPI
    Column         : CGR
    Column         : GOR
    Column         : FracturePressure
    Column         : Buoyancy
    Column         : WCSurface
    Column         : GOC
    Column         : OWC
    Column         : FreeGasMass
    Column         : FreeGasVolume
    Column         : FreeGasDensity
    Column         : FreeGasViscosity
    Column         : FreeGasMassC1
    Column         : FreeGasMassC2
    Column         : FreeGasMassC3
    Column         : FreeGasMassC4
    Column         : FreeGasMassC5
    Column         : FreeGasMassN2
    Column         : FreeGasMassCOx
    Column         : FreeGasMassH2S
    Column         : FreeGasMassC6_14Aro
    Column         : FreeGasMassC6_14Sat
    Column         : FreeGasMassC15Aro
    Column         : FreeGasMassC15Sat
    Column         : FreeGasMassLSC
    Column         : FreeGasMassC15AT
    Column         : FreeGasMassC15AroS
    Column         : FreeGasMassC15SatS
    Column         : FreeGasMassC6_14BT
    Column         : FreeGasMassC6_14DBT
    Column         : FreeGasMassC6_14BP
    Column         : FreeGasMassC6_14SatS
    Column         : FreeGasMassC6_14AroS
    Column         : FreeGasMassresins
    Column         : FreeGasMassasphaltenes
    Column         : CondensateMass
    Column         : CondensateVolume
    Column         : CondensateDensity
    Column         : CondensateViscosity
    Column         : CondensateMassC1
    Column         : CondensateMassC2
    Column         : CondensateMassC3
    Column         : CondensateMassC4
    Column         : CondensateMassC5
    Column         : CondensateMassN2
    Column         : CondensateMassCOx
    Column         : CondensateMassH2S
    Column         : CondensateMassC6_14Aro
    Column         : CondensateMassC6_14Sat
    Column         : CondensateMassC15Aro
    Column         : CondensateMassC15Sat
    Column         : CondensateMassresins
    Column         : CondensateMassasphaltenes
    Column         : CondensateMassLSC
    Column         : CondensateMassC15AT
    Column         : CondensateMassC15AroS
    Column         : CondensateMassC15SatS
    Column         : CondensateMassC6_14BT
    Column         : CondensateMassC6_14DBT
    Column         : CondensateMassC6_14BP
    Column         : CondensateMassC6_14SatS
    Column         : CondensateMassC6_14AroS
    Column         : SolutionGasMass
    Column         : SolutionGasVolume
    Column         : SolutionGasDensity
    Column         : SolutionGasViscosity
    Column         : SolutionGasMassC1
    Column         : SolutionGasMassC2
    Column         : SolutionGasMassC3
    Column         : SolutionGasMassC4
    Column         : SolutionGasMassC5
    Column         : SolutionGasMassN2
    Column         : SolutionGasMassCOx
    Column         : SolutionGasMassH2S
    Column         : SolutionGasMassC6_14Aro
    Column         : SolutionGasMassC6_14Sat
    Column         : SolutionGasMassC15Aro
    Column         : SolutionGasMassC15Sat
    Column         : SolutionGasMassresins
    Column         : SolutionGasMassasphaltenes
    Column         : SolutionGasMassLSC
    Column         : SolutionGasMassC15AT
    Column         : SolutionGasMassC15AroS
    Column         : SolutionGasMassC15SatS
    Column         : SolutionGasMassC6_14BT
    Column         : SolutionGasMassC6_14DBT
    Column         : SolutionGasMassC6_14BP
    Column         : SolutionGasMassC6_14SatS
    Column         : SolutionGasMassC6_14AroS
    Column         : StockTankOilMass
    Column         : StockTankOilVolume
    Column         : StockTankOilDensity
    Column         : StockTankOilViscosity
    Column         : StockTankOilMassC1
    Column         : StockTankOilMassC2
    Column         : StockTankOilMassC3
    Column         : StockTankOilMassC4
    Column         : StockTankOilMassC5
    Column         : StockTankOilMassN2
    Column         : StockTankOilMassCOx
    Column         : StockTankOilMassH2S
    Column         : StockTankOilMassC6_14Aro
    Column         : StockTankOilMassC6_14Sat
    Column         : StockTankOilMassC15Aro
    Column         : StockTankOilMassC15Sat
    Column         : StockTankOilMassresins
    Column         : StockTankOilMassasphaltenes
    Column         : StockTankOilMassLSC
    Column         : StockTankOilMassC15AT
    Column         : StockTankOilMassC15AroS
    Column         : StockTankOilMassC15SatS
    Column         : StockTankOilMassC6_14BT
    Column         : StockTankOilMassC6_14DBT
    Column         : StockTankOilMassC6_14BP
    Column         : StockTankOilMassC6_14SatS
    Column         : StockTankOilMassC6_14AroS
    Header         : Trapper Data
    Time dependent : No
    Property I/O   : Yes

    Identification : DataMiningIoTbl
    Description    : Table listing for datadriller
    Column         : BPAKey
    Column         : Time
    Column         : XCoord
    Column         : YCoord
    Column         : ZCoord
    Column         : ReservoirName
    Column         : FormationName
    Column         : SurfaceName
    Column         : PropertyName
    Column         : PropertyUnit
    Column         : Value
    Header         : datadriller Data
    Time dependent : No
    Property I/O   : Yes


    Identification : SourceRockPropIoTbl
    Description    : Contains present day source rock properties
    Column         : LayerName
    Column         : PropertyName
    Column         : PropertyUnit
    Column         : MapFileName
    Column         : MapType
    Header         : Present day source rock properties
    Time dependent : No
    Property I/O   : Yes

    Identification : ReservoirPropIoTbl
    Description    : Contains present day reservoir properties
    Column         : ReservoirName
    Column         : PropertyName
    Column         : PropertyUnit
    Column         : MapFileName
    Column         : MapType
    Header         : Present day reservoir properties
    Time dependent : No
    Property I/O   : Yes

    Identification : GridIoTbl
    Description    : Contains grid descriptions
    Column         : GridName
    Column         : XLeft
    Column         : YBottom
    Column         : NumberX
    Column         : NumberY
    Column         : DeltaX
    Column         : DeltaY
    Header         : Grid descriptions
    Time dependent : No
    Property I/O   : Yes

    Identification : DiffusionResultIoTbl
    Description    : Table listing the diffusion values per trap
    Column         : ReservoirName
    Column         : Age
    Column         : TrapID
    Column         : XCoord
    Column         : YCoord
    Column         : MassC1Diffused
    Column         : PenetrationC1
    Column         : DiffCoefC1
    Column         : MassC2Diffused
    Column         : PenetrationC2
    Column         : DiffCoefC2
    Column         : MassC3Diffused
    Column         : PenetrationC3
    Column         : DiffCoefC3

    Identification : MigrationIoTbl
    Description    : Table listing migrations (information through time, read-only)
    Column         : MigrationProcess
    Column         : SourceAge
    Column         : SourceRockName
    Column         : SourceReservoirName
    Column         : SourceTrapID
    Column         : SourcePointX
    Column         : SourcePointY
    Column         : DestinationAge
    Column         : DestinationReservoirName
    Column         : DestinationTrapID
    Column         : DestinationPointX
    Column         : DestinationPointY
    Column         : MassC1
    Column         : MassC2
    Column         : MassC3
    Column         : MassC4
    Column         : MassC5
    Column         : MassN2
    Column         : MassCOx
    Column         : MassH2S
    Column         : MassC6_14Aro
    Column         : MassC6_14Sat
    Column         : MassC15Aro
    Column         : MassC15Sat
    Column         : MassLSC
    Column	   : MassC15AT
    Column	   : MassC15AroS
    Column	   : MassC15SatS
    Column	   : MassC6_14BT
    Column	   : MassC6_14DBT
    Column	   : MassC6_14BP
    Column	   : MassC6_14SatS
    Column	   : MassC6_14AroS
    Column         : Massresins
    Column         : Massasphaltenes

    Header         : Trap Loss Data
    Time dependent : No
    Property I/O   : Yes

    Identification : ReservoirLeakIoTbl
    Description    : Table listing reservoir leakage
    Column         : Age
    Column         : SourceReservoirName
    Column         : SourceTrapID
    Column         : LeakPointX
    Column         : LeakPointY
    Column         : MassOilSpilled
    Column         : MassGasSpilled
    Column         : WasteLeakPoint
    Header         : Reservoir Leak Data
    Time dependent : No
    Property I/O   : Yes
    
 # Lithotype data IO:

    Identification : LithotypeIoTbl
    Description    : Table with general and material (single) propeties of
                     each lithotype
    Column         : Lithotype
    Column         : Description
    Column         : DefinedBy
    Column         : UserDefined
    Column         : DefinitionDate
    Column         : LastChangedBy
    Column         : LastChangedDate
    Column         : FgColour
    Column         : BgColour
    Column         : Pixmap
    Column         : Density
#    Column         : TExpansionCoef
    Column         : HeatProd
    Column         : SurfacePorosity
    Column         : CompacCoefES
    Column         : CompacCoefSC
    Column         : CompacCoefFM
    Column         : StpThCond
    Column         : ThCondAnisotropy
    Column         : DepoPerm
    Column         : PermDecrStressCoef
    Column         : PermIncrRelaxCoef
    Column         : PermAnisotropy
    Column         : PermMixModel
    Column         : SeisVelocity
    Column         : CapC1
    Column         : CapC2
    Column         : PcKrModel
    Column         : LambdaPc
    Column         : LambdaPcWater
    Column         : LambdaKr
    Column         : LambdaKrWater
    Column         : Porosity_Model
    Column         : Compaction_Coefficient_SM
    Column         : Number_Of_Data_Points
    Column         : Multipoint_Porosity
    Column         : Multipoint_Permeability
    Column         : HydraulicFracturingPercent
    Column         : ReferenceSolidViscosity
    Column         : ActivationEnergy
    Column         : MinimumPorosity
    Column         : SpecSurfArea
    Column         : GeoVariance

    Header         : Lithotype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : LitMixIoTbl
    Description    : Table that specifies how a lithotype was defined in terms
                     of a mix of other lithotypes
    Column         : Lithotype
    Column         : Lithotype1
    Column         : Percent1
    Header         : Lithotype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : LitThCondIoTbl
    Description    : Table of Thermal conductivity of lithotype matrix at a 
                     specific temperature (see table) (perpendicular to 
                     bedding)
    Column         : Lithotype
    Column         : TempIndex
    Column         : ThCond
    Header         : Lithotype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : LitHeatCapIoTbl
    Description    : Table of specific heat capacity of a lithottype at
                     a certain temperatures 
    Column         : Lithotype
    Column         : TempIndex
    Column         : HeatCapacity
    Header         : Lithotype Data
    Time dependent : No
    Property I/O   : Yes

#    Identification : LitUraThoPotIoTbl
#    Description    : Table of Uranium, Thorium and Potassium concentration for
#                     a specific lithotype, these values can be used 
#                     calculate the 
#    Column         : Lithotype
#    Column         : UraniumConcent
#    Column         : ThoriumConcent
#    Column         : PotassiumConcent
#    Header         : Lithotype Data
#    Time dependent : No
#    Property I/O   : Yes

#   MODEL:

    Identification : LitMixTbl
    Description    : table with each lithotype and its percentage (the column
                     of PercentLith must sum to 100). 
    Column         : Lithotype
    Column         : Percent
    Time dependent : No
    Property I/O   : No

#   perhaps move to two vectors LithotypeID and PercentLith

    Identification : ThCondTbl
    Description    : Table of Thermal conductivity at a 
                     specific temperature (see table) (perpendicular to 
                     bedding)
    Column         : TempIndex
    Column         : ThCond
    Column         : ThCondParallel
    Time dependent : No
    Property I/O   : No

    Identification : HeatCapTbl
    Description    : Table of specific heat capacity  at
                     a certain temperatures 
    Column         : TempIndex
    Column         : HeatCapacity
    Time dependent : No
    Property I/O   : No

    Identification : DensXHeatCapTbl
    Description    : Table of density x specific heat capacity  at
                     a certain temperatures 
    Column         : TempIndex
    Column         : DensXHeatCap
    Time dependent : No
    Property I/O   : No

#   Identification : FltHeatCapTbl
#   Description    : Table of specific heat capacity  at
#                    a certain temperature and pressure 
#   Column         : TempIndex
#   Column         : Pressure
#   Column         : HeatCapacity
#   Time dependent : No
#   Property I/O   : No

#   Identification : FltDensityTbl
#   Description    : Table of density  at a certain temperature and pressure 
#   Column         : TempIndex
#   Column         : Pressure
#   Column         : Density
#   Time dependent : No
#   Property I/O   : No

#   Identification : FltDensXHeatCapTbl
#   Description    : Table of density x specific heat capacity  at
#                    a certain temperature and pressure 
#   Column         : TempIndex
#   Column         : Pressure
#   Column         : DensXHeatCap
#   Time dependent : No
#   Property I/O   : No

# Fluidtype data ----------------------------------------------------------

    Identification : FluidtypeIoTbl
    Description    : table with general and material properties of each
                     fluidtype (Current version only 1 row i.e only 1 fluid =
                     water) 
    Column         : Fluidtype
    Column         : Description
    Column         : DefinedBy
    Column         : UserDefined
    Column         : DefinitionDate
    Column         : LastChangedDate
    Column         : LastChangedBy
    Column         : Density 
    Column         : DensModel
    Column         : SeisVelocity
    Column         : SeisVelocityModel
    Column         : Salinity
    Column         : HeatCaptype
    Column         : ThermCondtype
    Column         : Denstype
    Column         : Viscotype
    Header         : Fluidtype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : FltThCondIoTbl
    Description    : Table of thermal conductivites at different temperatures 
                     per Fluidtype
    Column         : Fluidtype
    Column         : TempIndex
    Column         : ThCond
    Header         : Fluidtype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : FltHeatCapIoTbl
    Description    : Table of temperature dependence of specific heat 
                     capacitiesper fluidtype
    Column         : Fluidtype
    Column         : TempIndex
    Column         : Pressure
    Column         : HeatCapacity
    Header         : Fluidtype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : FltDensityIoTbl
    Description    : Table of temperature dependence of specific heat 
                     capacitiesper fluidtype
    Column         : Fluidtype
    Column         : TempIndex
    Column         : Pressure
    Column         : Density
    Header         : Fluidtype Data
    Time dependent : No
    Property I/O   : Yes

    Identification : FltViscoIoTbl
    Description    : Table which specifies the decrease and increase coefficients
                     for the permeability for a specific lithology type
    Column         : Fluidtype
    Column         : TempIndex
    Column         : Viscosity
    Header         : Fluidtype Data
    Time dependent : No
    Property I/O   : Yes

#### User Interface tables ------------------------------------------------

    Identification : DisplayContourIoTbl
    Description    : contour settings for any calculated property (e.g
                     temperature isotherms, VRE milestones 
    Column         : PropertyName
    Column         : ContourValue
    Column         : ContourColour
    Header         : Countour settings 
    Time dependent : No
    Property I/O   : Yes

    Identification : PlotTbl 
    Description    : Plot (X,Y) points for plotting in graphs
    Column         : PlotLineId
    Column         : PlotXcrd
    Column         : PlotYcrd
    Time dependent : No
    Property I/O   : No

    Identification : HistogramTbl 
    Description    : Histogram bins for plotting histograms
    Column         : BinStart
    Column         : BinEnd
    Column         : BinCount
    Time dependent : No
    Property I/O   : No

    Identification : SnapshotTbl 
    Description    : Used to extract shapshot times from project
    Column         : Time
    Time dependent : No
    Property I/O   : No

    Identification : PlotBoundsTbl
    Description    : Used to specify fixed bounds for plotting of properties.
    Column         : PropertyName
    Column         : BoundsEnabled
    Column         : LowerBound
    Column         : UpperBound
    Time dependent : No
    Property I/O   : Yes

# Output results  ------------------------------------------------------

    Identification : NeedlePropsTbl
    Description    : Table with properties of active nodes on a needle
    Column         : Depth
    Column         : SurfaceName
    Column         : LayerName
    Column         : DummyLayerIndicator
    Column         : Temperature
    Column         : ErrTemperature
    Column         : Pressure
    Column         : HydroStaticPressure
    Column         : LithoStaticPressure
    Column         : OverPressure
    Column         : Ves
    Column         : MaxVes
    Column         : Vr
    Column         : HopaneIsomerisation
    Column         : SteraneIsomerisation
    Column         : SteraneAromatisation
    Column         : IlliteFraction
    Column         : Thickness
    Column         : PorosityVec2
    Column         : SonicVec2
    Column         : VelocityVec2
    Column         : ReflectivityVec2
    Column         : BulkDensityVec2
    Column         : ThCondVec2
    Column         : PermeabilityVec2
    Column         : HeatFlowZ
    Column         : HeatFlow_
    Column         : MassFluxZ
    Column         : MassFlux_
    Column         : DiffusivityVec2
    Header         : Output results
    Time dependent : No
    Property I/O   : No

    Identification : RunStatusIoTbl
    Description    : Table which contains the most imported status information
                     of the last run. Important for re-start + continue
		     behaviour for montecarlo 1d-runs or big 3d-runs
    Column         : NrMCLoopsCompleted
    Column         : MCCurrentSeedNumber
    Column         : MCStatusOfLastRun
    Column         : MCCalculationScope
    Column         : OutputDirOfLastRun
    Column         : OutputDirCreatedBy
    Column         : RestartTempCalcTimeStep
    Column         : RestartPresCalcTimeStep
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    # Changed by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.4
    # Added P15, P50 and P85
    # Increment 4.6
    # Added sums of nth power about the mean and
    # skewness and kurtosis
    Identification : DepthIoTbl
    Description    : Table listing the statistics of property values
                     for serveral depth/time combinations, 
		     KEYS: PropertyName+Time+Depth_
    Column         : PropertyName
    Column         : Time
    Column         : Depth_
    Column         : Average
    Column         : StandardDev
    Column         : Minimum
    Column         : Maximum
    Column         : Sum
    Column         : Sum2
    Column         : NP
    Column         : P15
    Column         : P50
    Column         : P85
    Column         : SumFirstPower
    Column         : SumSecondPower
    Column         : SumThirdPower
    Column         : SumFourthPower
    Column         : Skewness
    Column         : Kurtosis
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    Identification : DepthMCResultIoTbl
    Description    : Table listing the values of property values
                     for all MC-runs and serveral depth/time combinations, 
		     KEYS: PropertyName+Time+Depth_+RunNumber
    Column         : PropertyName
    Column         : Time
    Column         : Depth_
    Column         : RunNumber
    Column         : Value
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    Identification : ExportDepthIoTbl
    Description    : This is a strip down version of 'DepthIoTbl' used to
                     export a depth io plot to file
    Column         : PropertyName
    Column         : Time
    Column         : Depth_
    Column         : Average
    Column         : StandardDev
    Column         : Minimum
    Column         : Maximum

    # Added by S. Mathijssen (ORIGIN) 15-12-1997
    # PR 12
    Column         : P15
    Column         : P50
    Column         : P85

    # Added by S. Mathijssen (ORIGIN) 15-12-1997
    # PR 18
    Column         : Skewness
    Column         : Kurtosis


    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    # Changed by S. Mathijssen (ORIGIN) 14-11-1997
    # Increment 4.4
    # Added P15, P50 and P85
    # Increment 4.6
    # Added sums of nth power about the mean and
    # skewness and kurtosis
    Identification : TimeIoTbl
    Description    : Table listing the statistics of property values
                     for serveral time/surface combinations,
		     KEYS: PropertyName+Time+SurfaceName+FormationName
    Column         : PropertyName
    Column         : Time
    Column         : SurfaceName
    Column         : FormationName
    Column         : MapFileName
    Column         : BPAPresence
    Column         : MD5Checksum
    Column         : Average
    Column         : StandardDev
    Column         : Minimum
    Column         : Maximum
    Column         : Sum
    Column         : Sum2
    Column         : NP
    Column         : PropertyGrid
    Column         : DepoSequence
    Column         : P15
    Column         : P50
    Column         : P85
    Column         : SumFirstPower
    Column         : SumSecondPower
    Column         : SumThirdPower
    Column         : SumFourthPower
    Column         : Skewness
    Column         : Kurtosis
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    Identification : 3DTimeIoTbl
    Description    : Table listing for 3D property values
    Column         : Time
    Column         : PropertyName
    Column         : FormationName
    Column         : MapFileName
    Column         : GroupName
    Column         : DataSetName
    Column         : NumberX
    Column         : NumberY
    Column         : NumberZ
    Column         : Average
    Column         : Minimum
    Column         : Maximum
    Column         : Sum
    Column         : Sum2
    Column         : NP
    Header         : 3D output results
    Time dependent : No
    Property I/O   : Yes

    Identification : 1DTimeIoTbl
    Description    : Table listing for 1D property values
    Column         : Time
    Column         : PropertyName
    Column         : FormationName
    Column         : NodeIndex
    Column         : SurfaceName
    Column         : Value
    Header         : 1D output results
    Time dependent : No
    Property I/O   : Yes

    Identification : TimeMCResultIoTbl
    Description    : Table listing the property values
                     for all MC-runs and several time/surface combinations,
		     KEYS: PropertyName+Time+SurfaceName+FormationName+RunNumber
    Column         : PropertyName
    Column         : Time
    Column         : SurfaceName
    Column         : FormationName
    Column         : RunNumber
    Column         : Value
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    Identification : TimeSensResultIoTbl
    Description    : Table for holding Cauldron results when using Sample C
                     SOBOL results. This table is not saved to file
		     KEYS: PropertyName+Time+SurfaceName+FormationName+RunNumber
    Column         : PropertyName
    Column         : Time
    Column         : SurfaceName
    Column         : FormationName
    Column         : RunNumber
    Column         : Value
    Header         : Output results
    Time dependent : No
    Property I/O   : Yes
    Property I/O   : Yes

    Identification : ExportTimeIoTbl
    Description    : This is a strip down version of TimeIoTbl used to
                     export a time io plot to file
    Column         : PropertyName
    Column         : Time
    Column         : SurfaceName
    Column         : FormationName
    Column         : Average
    Column         : StandardDev
    Column         : Minimum
    Column         : Maximum

    # Added by S. Mathijssen (ORIGIN) 15-12-1997
    # PR 12
    Column         : P15
    Column         : P50
    Column         : P85

    # Added by S. Mathijssen (ORIGIN) 15-12-1997
    # PR 18
    Column         : Skewness
    Column         : Kurtosis


    Header         : Output results
    Time dependent : No
    Property I/O   : Yes

    Identification : HistogramMCResultIoTbl
    Description    : Table listing the PropertyName,SurfaceName,FormationName
                     combinations for which histograms are calculated in 
                     MC-mode. Key is a reference to entries in the 
                     MarkerMCResultIoTbl, NumberOfBins is the number of bins
                     in the histogram.
		     KEYS: PropertyName+SurfaceName+FormationName
    Column         : PropertyName
    Column         : SurfaceName
    Column         : FormationName
    Column         : DepoSequence
    Column         : Time
    Column         : NumberOfBins
    Column         : Key
    Column         : NP
    Header         : Histograms of MC output
    Time dependent : No
    Property I/O   : Yes

    Identification : MarkerMCResultIoTbl
    Description    : Table listing the markerheight and markerlocation to
                     allow calculation of histograms. From the MarkerHeight
                     values a histogram is calculated, MarkerLocation is needed
                     for restart of a run.
		     KEYS: Key+Index
    Column         : Key
    Column         : Index
    Column         : MarkerHeight
    Column         : MarkerLocation
    Header         : Histogram data
    Time dependent : No
    Property I/O   : Yes

    Identification : TemperatureIsoIoTbl
    Description    : Table to hold the temperature Iso depth that will be
                     displayed in the burial graph. The depth is calculated 
                     and stored after each time step in the Sum. Number of Points
                     is stored to accomodate MC runs. 
                     KEYS: ContourValue + Age
    Column         : Age
    Column         : ContourValue
    Column         : Sum
    Column         : NP
    Header         : Iso temperature data for burial graph
    Time dependent : No
    Property I/O   : Yes

    Identification : VrIsoIoTbl
    Description    : Table to hold the vr Iso depth that will be
                     displayed in the burial graph. The depth is calculated 
                     and stored after each time step in the Sum.  Number of Points
                     is stored to accomodate MC runs. 
                     KEYS: ContourValue + Age
    Column         : Age
    Column         : ContourValue
    Column         : Sum
    Column         : NP
    Header         : Iso vr data for burial graph
    Time dependent : No
    Property I/O   : Yes
 
    Identification : GenericIsoIoTbl
    Description    : Table to hold the Iso depth data that will be
                     displayed in the burial graph. The depth is calculated
                     and stored after each time step.  Number of Points is stored
                     to accomodate MC runs.
                     This table is used to perfrom operations on the 
                     TemperatureIsoIoTbl and VrIsoIoTbl (inheritance)
                     KEYS: ContourValue + Age
    Column         : Age
    Column         : ContourValue
    Column         : Sum
    Column         : NP
    Header         : Iso vr or temperature data for burial graph
    Time dependent : No
    Property I/O   : No

    Identification : TouchstoneMapIoTbl
    Description    : List of map choices made by user
    Column         : TcfName
    Column         : SurfaceName
    Column         : FormationName
    Column         : Category
    Column         : Format
    Column         : Percentile
    Header         : List of Touchstone Map Combinations
    Time dependent : No
    Property I/O   : Yes

# Sensitivity tables

    Identification : GrpCombIoTbl
    Description    : Sensitivity Analysis Group Input combinations
    Column         : GroupCombination
    Header         : Sensitivity Analysis Sobol Input Group Combinations
    Time dependent : No
    Property I/O   : Yes

# Obsolete tables

    Identification : FTmIoTbl
    Description    : Obsolete fission track table
    Column         : DepthIndex
    Column         : FTlength
    Column         : FTage
    Column         : Label
    Column         : Optimization
#    Header         : Fission track measurements
    Time dependent : No
    Property I/O   : Yes

# Genex History location Table

    Identification : GenexHistoryLocationIoTbl
    Column         : FormationName
    Column         : WellName
    Column         : XPlanePosition
    Column         : YPlanePosition
    Column         : Filename
    Header         : Node simulation history

# Shale Gas Tables

    Identification : LangmuirAdsorptionCapacityIsothermSetIoTbl
    Column         : LangmuirName
    Column         : LangmuirTemperature
    Column         : LangmuirPressure
    Column         : LangmuirVolume
    Header         : Langmuir Isotherm
    Property I/O   : Yes

    Identification : LangmuirAdsorptionCapacityTOCFunctionIoTbl
    Column         : LangmuirName
    Column         : ReferenceTemperature
    Column         : VLCoeffA
    Column         : VLCoeffB
    Column         : VLTemperatureGradient
    Column         : LangmuirPressure
    Header         : Lamgmuir TOC Adsorption Isotherm IO Tbl
    Property I/O   : Yes

    Identification : AdsorptionHistoryIoTbl
    Column         : AdsorptionFormationName
    Column         : XPlanePosition
    Column         : YPlanePosition
    Header         : Node simulation history

    Identification : IrreducibleWaterSaturationIoTbl
    Column         : CoefficientA
    Column         : CoefficientB
    Header         : Irreducible water saturation coefficients

    Identification : SGDensityIoTbl
    Column         : SGMeanBulkDensity

#    Identification : AdsorptionNodeOutputIoTbl
#    Column         : AdsorptionNodeOutputSubscript
#    Column         : MapRowIndex
#    Column         : MapColumnIndex
#    Header         : Node output for debugging.

END-property-tables

##########################################################################

START-application-classes

    Identification : Project
    # Identification : Node
    # Identification : Segment
    # Identification : Brick
    # Identification : Lithotype
    # Identification : Fluidtype
    # Identification : Lithomatrix
    # Identification : Fluid

END-application-classes

##########################################################################

START-application-class-properties

    Class    : Project
    Property : ProjectIoTbl
    Property : ProjectNotesIoTbl
    Property : RelatedProjectsIoTbl
    Property : DefaultFileIoTbl
    Property : IoOptionsIoTbl
    Property : RunOptionsIoTbl
    Property : SnapshotIoTbl
    Property : SensAnalysisIoTbl
    Property : RespSurfModelParamsIoTbl
    Property : WellLocIoTbl
    Property : TouchstoneWellIoTbl
    Property : TouchstoneMapIoTbl
    Property : GrpCombIoTbl
    Property : GridMapIoTbl
    Property : StratIoTbl
    Property : SourceRockLithoIoTbl
    Property : MobLayThicknIoTbl
    Property : AllochthonLithoIoTbl
    Property : AllochthonLithoDistribIoTbl
    Property : AllochthonLithoInterpIoTbl
    Property : CrustIoTbl
    Property : ContCrustalThicknessIoTbl
    Property : BasaltThicknessIoTbl
    Property : BasementIoTbl
    Property : FormPressIoTbl
    Property : FormTempIoTbl
    Property : DisplayContourIoTbl
    Property : SurfaceTempIoTbl
    Property : PermafrostIoTbl
    Property : GeologicalBoundaryIoTbl
    Property : PropertyBoundaryIoTbl
    Property : BoundaryValuesIoTbl
    Property : SurfaceDepthIoTbl
    Property : MntlHeatFlowIoTbl
    Property : RawBhtIoTbl
    Property : RawBhtBitsizeIoTbl
    Property : MarsdenCoefIoTbl
    Property : VrmIoTbl
    Property : SonicIoTbl
    Property : BulkDensityIoTbl
    Property : BiomarkermIoTbl
    Property : BiomarkerKinIoTbl
    Property : SmectiteIlliteIoTbl
    Property : SmectiteIlliteKinIoTbl
    Property : ReservoirIoTbl
    Property : BioDegradIoTbl
    Property : DiffusionIoTbl
    Property : PalinspasticIoTbl
    Property : FaultcutIoTbl
    Property : PressureFaultcutIoTbl
    Property : PressureFuncIoTbl
    Property : TrapIoTbl
    Property : MigrationIoTbl
    Property : TrapperIoTbl
    Property : DataMiningIoTbl
    Property : ReservoirLeakIoTbl
    Property : FtGrainIoTbl
    Property : FtLengthIoTbl
    Property : FtSampleIoTbl
    Property : FtPredLengthCountsHistIoTbl
    Property : FtPredLengthCountsHistDataIoTbl
    Property : FtClWeightPercBinsIoTbl
    Property : LithotypeIoTbl
    Property : LitMixIoTbl
    Property : LitThCondIoTbl
    Property : LitHeatCapIoTbl
    Property : FluidtypeIoTbl
    Property : FltThCondIoTbl
    Property : FltHeatCapIoTbl
    Property : FltDensityIoTbl
    Property : FltViscoIoTbl
    Property : FilterDepthIoTbl
    Property : RunStatusIoTbl
    Property : FilterTimeIoTbl
    Property : FilterTimeDepthIoTbl
    #Property : PlotTbl
    #Property : HistogramTbl
    #Property : SnapshotTbl
    #Property : PlotBoundsTbl
    #Property : NeedlePropsTbl
    Property : DepthIoTbl
    Property : DepthMCResultIoTbl
    Property : ExportDepthIoTbl
    Property : TimeIoTbl
    Property : 3DTimeIoTbl
    Property : 1DTimeIoTbl
    Property : TimeMCResultIoTbl
    Property : TimeSensResultIoTbl
    Property : ExportTimeIoTbl
    Property : HistogramMCResultIoTbl
    Property : MarkerMCResultIoTbl
    Property : DiffusionResultIoTbl
    Property : TemperatureIsoIoTbl
    Property : VrIsoIoTbl
    #Property : GenericIsoIoTbl
    Property : FTmIoTbl
# Genex History Location 
    Property : GenexHistoryLocationIoTbl
# CrustalThicknessCalculator data
    Property : CTCIoTbl
# Shale Gas properties
    Property : LangmuirAdsorptionCapacityIsothermSetIoTbl
    Property : LangmuirAdsorptionCapacityTOCFunctionIoTbl
    Property : AdsorptionHistoryIoTbl
    Property : IrreducibleWaterSaturationIoTbl
    Property : SGDensityIoTbl
#    Property : AdsorptionNodeOutputIoTbl
    Property : BPANameMapping
    
    # Class    : Node
    # Property : Depth
    # Property : SurfaceNormal
    # Property : Ves
    # Property : MaxVes
    # Property : Pressure
    # Property : HydroStaticPressure
    # Property : LithoStaticPressure
    # Property : OverPressure
    # Property : MassFluxX 
    # Property : MassFluxY
    # Property : MassFluxZ 
    # Property : MassFlux_
    # Property : Temperature
    # Property : HeatFlowX 
    # Property : HeatFlowY
    # Property : HeatFlowZ 
    # Property : HeatFlow_
    # Property : Vr
    # Property : HopaneIsomerisation
    # Property : SteraneIsomerisation
    # Property : SteraneAromatisation
    # Property : IlliteFraction
    # Property : FCTCorrection
    # Property : FaultElements
    # Property : SaveFCTCorrection
    # Property : IsoStaticMass
    # Property : IsoStaticDeflection
    # Property : IsoStaticWaterBottom
#    Property : Xlocation
#    Property : Ylocation
#    Property : DepoAge
#    Property : ErosionAge
#    Property : HeatProd
#    Property : Head


    # Class    : Segment
    # Property : DummyLayerSegment
    # Property : Thickness
#    Property : FullCompThickness   superseded by FCThicknessHistory attribute
    # Property : PorosityVec2
    # Property : SonicVec2
    # Property : VelocityVec2
    # Property : ReflectivityVec2
    # Property : BulkDensityVec2
    # Property : ThCondVec2
    # Property : PermeabilityVec2
    # Property : DiffusivityVec2
    

    # Class    : Brick
    # Property : HeatFlowVec8
    # Property : MassFluxVec8
#    Property : FluidFlow

    # Class    : Lithotype
    # Property : DepoPerm
    # Property : PermDecrStressCoef
    # Property : PermIncrRelaxCoef
    # Property : PermAnisotropy
    # Property : PermMixModel
    # Property : HeatCapTbl
    # Property : ThCondTbl
    # Property : Density
    # Property : HeatProd
    # Property : SurfacePorosity
    # Property : CompacCoefES
    # Property : CompacCoefSC
    # Property : CompacCoefFM
    # Property : StpThCond
    # Property : ThCondAnisotropy
    # Property : SeisVelocity
    # Property : CapC1
    # Property : CapC2
    # Property : Porosity_Model
    # Property : Compaction_Coefficient_SM
    # Property : Number_Of_Data_Points
    # Property : Multipoint_Porosity
    # Property : Multipoint_Permeability 
    # Property : HydraulicFracturingPercent
    # Property : ReferenceSolidViscosity
    # Property : ActivationEnergy
    # Property : MinimumPorosity

    # Class    : Lithomatrix
    # Property : DepoPerm
    # Property : PermDecrStressCoef
    # Property : PermIncrRelaxCoef
    # Property : PermAnisotropy
    # Property : PermMixModel
    # Property : LitMixTbl
    # Property : MixModel
    # Property : Density
    # Property : DensXHeatCapTbl
    # Property : ThCondTbl
    # Property : HeatProd
    # Property : SurfacePorosity
    # Property : CompacCoefES
    # Property : CompacCoefSC 
    # Property : CompacCoefFM
    # Property : StpThCond
    # Property : ThCondAnisotropy
    # Property : SeisVelocity
    # Property : CapC1
    # Property : CapC2
    # Property : Porosity_Model
    # Property : Compaction_Coefficient_SM
    # Property : Number_Of_Data_Points
    # Property : Multipoint_Porosity
    # Property : Multipoint_Permeability
    # Property : HydraulicFracturingPercent
    # Property : ReferenceSolidViscosity
    # Property : ActivationEnergy
    # Property : MinimumPorosity

    # Class    : Fluidtype
    # Property : Density
    # Property : SeisVelocity
    # # Property : FltHeatCapTbl
    # # Property : FltDensityTbl
    # Property : ThCondTbl

    # Class    : Fluid
    # Property : Density
    # Property : SeisVelocity
    # Property : FltDensXHeatCapTbl
    # Property : ThCondTbl


END-application-class-properties

##########################################################################

