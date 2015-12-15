#ifndef LIBRARIES_PARAVIEWREADER_PARAVIEWREADER_H
#define LIBRARIES_PARAVIEWREADER_PARAVIEWREADER_H

#include <vtkMultiBlockDataSetAlgorithm.h>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

#include "datatype.h"

class vtkInformation;
class vtkInformationVector;
class vtkTable;

namespace DataAccess { namespace Interface {
   class ProjectHandle; 
} }

namespace database {
   class Table;
   class Record;
}

class ParaViewReader : public vtkMultiBlockDataSetAlgorithm 
{
public:
  vtkTypeMacro( ParaViewReader, vtkMultiBlockDataSetAlgorithm );

  ParaViewReader();

  void PrintSelf(std::ostream& os, vtkIndent indent);
  static ParaViewReader *New();

  // Description:
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  int RequestInformation( vtkInformation * request, 
                          vtkInformationVector ** inputVector,
                          vtkInformationVector * outputVector);

  int RequestData( vtkInformation * request, 
                   vtkInformationVector ** inputVector,
                   vtkInformationVector * outputVector);

  int RequestUpdateExtent( vtkInformation * request, 
                           vtkInformationVector ** inputVector,
                           vtkInformationVector * outputVector);
  

private:
  ParaViewReader(const ParaViewReader &); // copying prohibited
  ParaViewReader & operator=(const ParaViewReader & ); // assignment prohibited

  // Internal functions
  static void fillVtkTable(vtkTable * table, database::Table * cauldronTable);
  static vtkAbstractArray * createVtkArray( datatype::DataType type );
  static void copyFieldFromRecord( vtkAbstractArray * array, datatype::DataType type, database::Record * record, int column);

  // VTK Properties
  char * FileName;

  // Cauldron properties
  boost::shared_ptr< DataAccess::Interface::ProjectHandle > m_project;
};

#endif
