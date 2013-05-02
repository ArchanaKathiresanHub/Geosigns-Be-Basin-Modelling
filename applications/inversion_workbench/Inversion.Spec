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

END-properties

START-property-tables

  Identification : SimpleParametersDefinition
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

END-property-tables
