START-properties

  Identification : DepoFormationName
  Type       : string

  Identification : Lithology1
  Type       : string

  Identification : Percentage1
  Type       : double

  Identification : Lithology2
  Type       : string

  Identification : Percentage2
  Type       : double

  Identification : Lithology3
  Type       : string

  Identification : Percentage3
  Type       : double


  Identification : SimpleParameter
  Type       : string

  Identification : StartValue
  Description    : Start value of a range (inclusive)
  Type           : double

  Identification : EndValue
  Description    : End value of a range (inclusive)
  Type           : double

  Identification : StepValue
  Description    : Step value of a range
  Type           : double

  Identification : ProjectTemplatePath
  Description    : Path to template project file.
  Type       : string

  Identification : OutputDirectory
  Description    : Directory in which all generated Project3d files are saved, are processed by Cauldron, and calculated properties are saved.
  Type           : string

  Identification : OutputFileName
  Description    : Format of generated Project3d files names.
  Type           : string

  Identification : CauldronVersion
  Description    : Version of Fastcauldron to use
  Type           : string

  Identification : OutputTableFieldSeparator
  Description    : Field separator that is used when outputting table with resulting variables. The field separator can only consist of a single character
  Type           : string

  Identification : OutputTableFieldWidth
  Description    : Field width that is used when outputting table with resulting variables. A field width of 0 disables it.
  Default        : 0
  Type           : integer

  Identification : RetrievedVariable
  Type       : string

  Identification : SnapshotTime
  Type       : double

  Identification : PositionX
  Type       : double

  Identification : PositionY
  Type       : double

  Identification : PositionBegZ
  Type       : double

  Identification : PositionEndZ
  Type       : double

  Identification : StepZ
  Type       : double

  Identification : AlgoType 
  Description    : Name of the stage in SA/UA
  Type           : string

  Identification : AlgoName
  Description    : Name of the algorithm which will be used in corresponded stage of SA/UA
  Type           : string

  Identification : AlgoParameters
  Description    : Set of parameters for algorith in SA/UA
  Type           : string

END-properties

START-property-tables

  Identification : UnconformityLithology
  Column     : DepoFormationName
  Column     : Lithology1
  Column     : Percentage1
  Column     : Lithology2
  Column     : Percentage2
  Column     : Lithology3
  Column     : Percentage3
  Property I/O : Yes

  Identification : BasementProperty
  Column     : SimpleParameter
  Column     : StartValue
  Column     : EndValue
  Column     : StepValue
  Header     : Project Data
  Property I/O : Yes

  Identification : CrustalThinningProperty
  Column     : SimpleParameter
  Column     : StartValue
  Column     : EndValue
  Column     : StepValue
  Header     : Project Data
  Property I/O : Yes

  Identification : UnconformityProperty
  Column     : DepoFormationName
  Column     : SimpleParameter
  Column     : StartValue
  Column     : EndValue
  Column     : StepValue
  Property I/O : Yes

  Identification : SurfaceTemperatureProperty
  Column     : StartValue
  Column     : EndValue
  Column     : StepValue
  Property I/O: Yes

  Identification : RuntimeConfiguration
  Column     : ProjectTemplatePath
  Column     : OutputDirectory
  Column     : OutputFileName
  Column     : CauldronVersion
  Column     : OutputTableFieldSeparator
  Column     : OutputTableFieldWidth
  Header     : Project Data
  Property I/O : Yes

  Identification : DatadrillerProperty
  Column     : RetrievedVariable
  Column     : SnapshotTime
  Column     : PositionX
  Column     : PositionY
  Column     : PositionBegZ
  Column     : PositionEndZ
  Column     : StepZ
  Header     : Project Data
  Property I/O : Yes

  Identification : SAUAParameters
  Column         : AlgoType
  Column         : AlgoName
  Column         : AlgoParameters

END-property-tables


