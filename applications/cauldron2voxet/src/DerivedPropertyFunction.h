#ifndef _DERIVED_PROPERTY_FUNCTION_H_
#define _DERIVED_PROPERTY_FUNCTION_H_

#include <string>
#include <vector>

#include "DerivedPropertyFormationFunction.h"

using namespace DataAccess;
using namespace Interface;

#include "database.h"

#include "VoxetProjectHandle.h"

#include "fparser.hh"

/// The derived property function.
///
/// At the moment all functions derived from this have a calculate function taking ves and bulk-density, 
/// this should be changed if required.
class DerivedPropertyFunction {

public :

   DerivedPropertyFunction ( database::Record* record, DerivedPropertyFunctionList & derivedPropertyFunctions);

   virtual ~DerivedPropertyFunction () {}

   /// Calculate the derived property.
   virtual float calculate (std::vector<Parameter *> & inputParameters, std::vector<float> & inputValues) const;

   /// The name of the derived property.
   virtual const std::string& getName () const;

   /// Return a string representation of the derived property.
   virtual std::string image () const;

   inline FunctionParser & getFunctionParser ();

private :
   std::string m_name;
   std::string m_function;
   std::string m_functionParameters;

   mutable FunctionParser m_fparser;

   void handleParserError (int parserError);
   void handleEvalError ( std::vector<float> & inputValues) const;
}; 

inline FunctionParser & DerivedPropertyFunction::getFunctionParser ()
{
   return m_fparser;
}

#endif // _DERIVED_PROPERTY_FUNCTION_H_
