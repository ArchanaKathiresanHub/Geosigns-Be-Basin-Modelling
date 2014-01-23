#ifndef LIBRARIES_PARAVIEWREADER_PARAVIEWREADER_H
#define LIBRARIES_PARAVIEWREADER_PARAVIEWREADER_H

#include <vtkStructuredGridAlgorithm.h>
#include <iosfwd>

class vtkInformation;
class vtkInformationVector;

class ParaViewReader : public vtkStructuredGridAlgorithm 
{
public:
  vtkTypeMacro( ParaViewReader, vtkStructuredGridAlgorithm );

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

  char * FileName;
};

#endif
