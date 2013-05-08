#ifndef INVERSION_PARAMETER_H
#define INVERSION_PARAMETER_H

#include <iosfwd>

class Project;

/// A Parameter is an instantiation of a Property. It's main function,
/// changeParameter, gives specific value to a specific set of fields in
/// a Cauldron project file.
class Parameter
{
public:
   virtual ~Parameter() {}

   /// Prints the parameter description and it's value
   virtual void print(std::ostream & output ) = 0;

   /// Updates a Cauldron project file with this specific parameter setting.
   virtual void changeParameter(Project & project)=0;
};

#endif

