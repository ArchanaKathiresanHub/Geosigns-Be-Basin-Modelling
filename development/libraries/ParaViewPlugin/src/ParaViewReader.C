#include <vtkMultiBlockDataSetAlgorithm.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkVariantArray.h>
#include <vtkVariant.h>
#include <vtkStdString.h>
#include <vtkTypeInt8Array.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkStringArray.h>
#include <vtkAbstractArray.h>

#include <iostream>

#include "ParaViewReader.h"

#include "Interface/ProjectHandle.h"

vtkStandardNewMacro(ParaViewReader);

ParaViewReader
  :: ParaViewReader()
   : FileName(0)
   , m_project()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

void ParaViewReader::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
   
    os << indent << "File Name: "
             << (this->FileName ? this->FileName : "(none)") << "\n";
}

int
ParaViewReader
  :: RequestInformation( vtkInformation * request, 
                          vtkInformationVector ** inputVector,
                          vtkInformationVector * outputVector)
{
   std::cout << "Request information " << std::endl;
   return 1;
}

int
ParaViewReader
  :: RequestUpdateExtent( vtkInformation * request, 
                           vtkInformationVector ** inputVector,
                           vtkInformationVector * outputVector)
{
   std::cout << "Request extent ... " << std::endl;
   return 1;
}



int
ParaViewReader
  :: RequestData( vtkInformation * request, 
                   vtkInformationVector ** inputVector,
                   vtkInformationVector * outputVector)
{
   std::cout << "Reading data ... " << std::endl;
  vtkInformation* outputInfo = outputVector->GetInformationObject(0);
  if (!m_project)
     m_project.reset( DataAccess::Interface::OpenCauldronProject( FileName, "r"));

  database::Database * database = m_project->getDataBase();
  int numberOfTables = std::distance( database->begin(), database->end() );

  vtkSmartPointer< vtkMultiBlockDataSet > dataSets 
     = vtkMultiBlockDataSet::GetData( outputVector, 0);

  dataSets->SetNumberOfBlocks( numberOfTables );

  for (int i = 0; i < numberOfTables; ++i)
  {
     vtkSmartPointer< vtkTable > table = vtkTable::New();
     fillVtkTable( table, database->getTable(i) );
     dataSets->SetBlock( i, table);
     dataSets->GetMetaData(i)->Set( vtkMultiBlockDataSet::NAME(), vtkStdString( database->getTable(i)->name() ) );
  }

  return 1;
}

void
ParaViewReader
   :: fillVtkTable( vtkTable * table, database::Table * cauldronTable )
{
  const database::TableDefinition & cauldronTableDef = cauldronTable->getTableDefinition();
  const int nRows = cauldronTable->size();
  const int nCols = cauldronTableDef.size();
  for (int col = 0; col < nCols; ++col)
  {
     const database::FieldDefinition & field = *cauldronTableDef.getFieldDefinition(col);
     datatype::DataType type = field.dataType();
     std::string columnName = field.name(); 
     if (! field.unit().empty() )
        columnName +=  " (" + field.unit() + ")";

     vtkSmartPointer< vtkAbstractArray > columnData = createVtkArray( type );
     columnData->SetName( vtkStdString(columnName) );

     for (int row = 0; row < nRows; ++row)
        copyFieldFromRecord(columnData, type, cauldronTable->getRecord(row), col);
     
     table->AddColumn( columnData );
  }
}

vtkAbstractArray *
ParaViewReader
   :: createVtkArray( datatype::DataType type)
{
   switch( type )
   {
      case datatype::Bool: 
         return vtkTypeInt8Array::New();

      case datatype::Int:
         return vtkIntArray::New();

      case datatype::Long:
         return vtkLongArray::New();

      case datatype::Float:
         return vtkFloatArray::New();

      case datatype::Double:
         return vtkDoubleArray::New();

      case datatype::String:
         return vtkStringArray::New();

      default:
         return vtkStringArray::New();
   }
}

void
ParaViewReader
   :: copyFieldFromRecord( vtkAbstractArray * array, datatype::DataType type, database::Record * record, int column)
{
   switch( type )
   {
      case datatype::Bool: 
         vtkTypeInt8Array::SafeDownCast(array)->InsertNextValue( record->getValue< bool >( column ) );
         break;

      case datatype::Int:
         vtkIntArray::SafeDownCast(array)->InsertNextValue( record->getValue< int >( column ) );
         break;

      case datatype::Long:
         vtkLongArray::SafeDownCast(array)->InsertNextValue( record->getValue< long >( column ) );
         break;

      case datatype::Float:
         vtkFloatArray::SafeDownCast(array)->InsertNextValue( record->getValue< float >( column ) );
         break;

      case datatype::Double:
         vtkDoubleArray::SafeDownCast(array)->InsertNextValue( record->getValue< double >( column ) );
         break;

      case datatype::String:
         vtkStringArray::SafeDownCast(array)->InsertNextValue( record->getValue< std::string >( column ) );
         break;

      default:
         vtkStringArray::SafeDownCast(array)->InsertNextValue( "N/A" );
         break;
   }
}
