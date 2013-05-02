START-properties


  Identification : SimpleParameter
  Type       : string

  Identification : StartValue
  Type       : double

  Identification : EndValue
  Type       : double

  Identification : StepValue
  Type       : double

  Identification : ProjectTemplatePath
  Type       : string

  Identification : OutputDirectory
  Type       : string

  Identification : OutputFileName
  Type       : string

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


END-properties

START-property-tables

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

  Identification : RuntimeConfiguration
  Column     : ProjectTemplatePath
  Column     : OutputDirectory
  Column     : OutputFileName
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

END-property-tables


