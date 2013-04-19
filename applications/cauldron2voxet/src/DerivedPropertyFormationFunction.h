#ifndef _VOXET_DERIVED_PROPERTY_FORMATION_FUNCTION_H_
#define _VOXET_DERIVED_PROPERTY_FORMATION_FUNCTION_H_

#include <string>
#include <vector>

#include "Property.h"

using namespace DataAccess;
using namespace Interface;

#include "database.h"

// Forward declaration of VoxetProjectHandle.
class VoxetProjectHandle;
class DerivedProperty;
class DerivedPropertyFunction;

class Parameter
{
   public:
      inline Parameter (const Property * property);
      inline Parameter (double constant);
      inline bool isConstant ();
      inline const Property * getProperty ();
      inline double getConstant ();
   private:
      const Property * m_property;
      double m_constant;

      bool m_isConstant;
};

/// Connects the formation---derived-property---derived-property-function.
class DerivedPropertyFormationFunction {

public :

   DerivedPropertyFormationFunction ( VoxetProjectHandle* voxetHandle,
                                      database::Record*   record );

   void splitFunctionCallParameters (const std::string & functionCallParameters);

   inline std::vector<Parameter *> & getFunctionCallParameters ();

   /// The name of the formation.
   const std::string& getFormationName () const;

   /// The name of the derived-property.
   const std::string& getDerivedPropertyName () const;

   /// The name of the function.
   const std::string& getFunctionName () const;

   /// The derived-property object.
   const DerivedProperty* getDerivedProperty () const;

   /// The derived-property-function.
   const DerivedPropertyFunction* getDerivedPropertyFunction () const;

private :

   VoxetProjectHandle* m_voxetHandle;
   database::Record*   m_record;

   std::vector<Parameter *> m_functionCallParameters;
};

std::vector<Parameter *> & DerivedPropertyFormationFunction::getFunctionCallParameters ()
{
   return m_functionCallParameters;
}

Parameter::Parameter (const Property * property) : m_property (property), m_isConstant (false)
{
}

Parameter::Parameter (double constant) : m_constant (constant), m_isConstant (true)
{
}

bool Parameter::isConstant ()
{
   return m_isConstant;
}

const Property * Parameter::getProperty ()
{
   assert (!isConstant ());
   return m_property;
}

double Parameter::getConstant ()
{
   assert (isConstant ());
   return m_constant;
}
#endif // _VOXET_DERIVED_PROPERTY_FORMATION_FUNCTION_H_
