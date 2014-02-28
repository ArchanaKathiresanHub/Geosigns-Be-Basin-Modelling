#ifndef INVERSION_PARAMETER_H
#define INVERSION_PARAMETER_H

#include <iosfwd>
#include <vector>

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

   /// Returns true if parameter is continious (float)
   virtual bool isContinuous() const = 0;

   /// Return vector of values for parameter if it could be converted to vector of doubles
   virtual std::vector<double> toDblVector() const = 0;

   /// Set values for parameter if it could be set from vector of doubles
   virtual void fromDblVector( const std::vector<double> & prms ) = 0;

};

#endif

