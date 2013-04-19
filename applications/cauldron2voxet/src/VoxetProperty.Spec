START-properties

    Identification      : Temperature
    Description         : 
    Unit of measurement : C (degCentigrade)
    Type                : double 

    Identification      : Ves
    Description         : 
    Unit of measurement : Pa
    Type                : double 


    Identification      : SnapshotTime
    Description         : snapshot time
    Type                : double
    Default		: 0

    Identification      : NumberOfVoxetNodesX
    Description         : NumberOfVoxetNodesX
    Type                : integer
    Default		: 100

    Identification      : NumberOfVoxetNodesY
    Description         : NumberOfVoxetNodesY
    Type                : integer
    Default		: 100

    Identification      : NumberOfVoxetNodesZ
    Description         : NumberOfVoxetNodesZ
    Type                : integer
    Default		: 50

    Identification      : VoxetOriginX
    Description         : Origin of voxet grid in x direction
    Type                : double
    Default		: 0

    Identification      : VoxetOriginY
    Description         : Origin of voxet grid in y direction
    Type                : double
    Default		: 0

    Identification      : VoxetOriginZ
    Description         : Origin of voxet grid in z direction
    Type                : double
    Default		: 0

    Identification      : VoxetDeltaX
    Description         : Delta of voxet grid in x direction
    Type                : double
    Default		: 1000

    Identification      : VoxetDeltaY
    Description         : Delta of voxet grid in y direction
    Type                : double
    Default		: 1000

    Identification      : VoxetDeltaZ
    Description         : Delta of voxet grid in z direction
    Type                : double
    Default		: 100


    Identification      : CauldronPropertyName
    Type                : string
    Default             : ""

    Identification      : VoxetPropertyName
    Type                : string
    Default             : ""

    Identification      : CauldronFormationName
    Type                : string
    Default             : ""

    Identification      : VoxetOutput
    Type                : smallint
    Default             : 0

    Identification      : VelocityFunctionSelection
    Type                : integer
    Default             : 0

    Identification      : Coeff1
    Type                : double
    Default             : 0

    Identification      : Coeff2
    Type                : double
    Default             : 0

    Identification      : ConversionFactor
    Type                : double
    Default             : 1.0

    Identification      : FunctionName
    Type                : string
    Default             : ""

    Identification      : Function
    Type                : string
    Default             : ""

    Identification      : FunctionCallParameters
    Type                : string
    Default		: ""

    Identification      : FunctionParameters
    Type                : string

    Identification      : DerivedPropertyName
    Type                : string
    Default             : ""

    Identification      : DerivedPropertyUnits
    Type                : string
    Default             : "ft/s"

    Identification      : OutputPropertyUnits
    Type                : string
    Default             : ""





END-properties

START-property-tables

    Identification : CauldronPropertyIoTbl
    Column         : CauldronPropertyName
    Column         : VoxetPropertyName
    Column         : ConversionFactor
    Column         : OutputPropertyUnits
    Column         : VoxetOutput
    Header         : Project data
    Property I/O   : Yes

    Identification : SnapshotTimeIoTbl
    Column         : SnapshotTime
    Header         : snapshot time
    Property I/O   : Yes

    Identification : VoxetGridIoTbl
    Column         : VoxetOriginX
    Column         : VoxetOriginY
    Column         : VoxetOriginZ
    Column         : VoxetDeltaX
    Column         : VoxetDeltaY
    Column         : VoxetDeltaZ
    Column         : NumberOfVoxetNodesX
    Column         : NumberOfVoxetNodesY
    Column         : NumberOfVoxetNodesZ
    Header         : Grid
    Property I/O   : Yes

    Identification : DerivedPropertyIoTbl
    Column         : DerivedPropertyName
    Column         : DerivedPropertyUnits

    Identification : DerivedPropertyFormationFunctionIoTbl
    Column         : CauldronFormationName
    Column         : DerivedPropertyName
    Column         : FunctionName
    Column         : FunctionCallParameters
    Header         : Formation functions
    Property I/O   : Yes

    Identification : DerivedPropertyFunctionIoTbl
    Column         : FunctionName
    Column         : FunctionParameters
    Column         : Function
    Header         : Function definitions
    Property I/O   : Yes


END-property-tables
