#include "DerivedPropertyFunction.h"
#include "voxetschemafuncs.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>

extern bool debug;

DerivedPropertyFunction::DerivedPropertyFunction (database::Record * record,
   DerivedPropertyFunctionList & derivedPropertyFunctions)
{
   m_name = database::getFunctionName (record);
   m_function = database::getFunction (record);
   m_functionParameters = database::getFunctionParameters (record);

   DerivedPropertyFunctionList::iterator dpfIterator;

   for (dpfIterator = derivedPropertyFunctions.begin (); dpfIterator != derivedPropertyFunctions.end (); ++dpfIterator)
   {
      m_fparser.AddConstant("pi", 3.14159265358979323846);
      m_fparser.AddConstant("e", 2.71828182845904523536);

      m_fparser.AddFunction ((*dpfIterator)->getName (), (*dpfIterator)->getFunctionParser ());
   }

   if (debug)
      std::cout << "Adding function: " << m_name << "(" << m_functionParameters << ") = " << m_function << endl;

   int parserError = m_fparser.Parse (m_function, m_functionParameters);
   handleParserError (parserError);
}

void DerivedPropertyFunction::handleParserError (int parserError)
{
   if (parserError >= 0)
   {
      std::cerr << "Error in function: \"" << m_function << "\" at position " << parserError + 1 << ": ";
      std::cerr << m_fparser.ErrorMsg () << endl;
      std::cerr << "                    ";
      for (int i = 0; i < parserError; ++i) std::cerr << " ";
      std::cerr << "^" << endl;
   }
}

float DerivedPropertyFunction::calculate (std::vector<Parameter *> & inputParameters, std::vector<float> & inputValues) const
{
   double values[inputValues.size ()];

   if (debug) std::cout << "(" << image () << ") (";

   for (int i = 0; i < inputValues.size (); ++i)
   {
      values[i] = inputValues[i];

      if (debug)
      {

         if ( inputParameters[i] != 0 ) {

            if (inputParameters[i]->isConstant ())
            {
               std::cout << inputParameters[i]->getConstant ();
            }
            else
            {
               std::cout << inputParameters[i]->getProperty ()->getName () << " = " << inputValues[i];
            }

         } else {
            std::cout << "?? = " << inputValues[i];
         }

         if (i == inputValues.size () - 1)
            std::cout << ") = ";
         else
            std::cout << ", ";
      }
   }

   double result = m_fparser.Eval (values);
   if (debug)
      std::cout << (float) result << std::endl;

   handleEvalError (inputValues);
   return (float) result;
}

void DerivedPropertyFunction::handleEvalError ( std::vector<float> & inputValues) const
{
   int error = m_fparser.EvalError ();
   if (error != 0)
   {
      std::cerr << "Error in function call (" << image () << ") (";

      for (int i = 0; i < inputValues.size (); ++i)
      {
	 std::cerr << inputValues[i];
	 if (i != inputValues.size () - 1)
	    std::cerr << ", ";
         else
            std::cerr << ")";
      }
      std::cerr << endl << "\t";

      switch (error)
      {
	 case 1:
	    std::cerr << "division by zero" << endl;
	    break;
	 case 2:
	    std::cerr << "sqrt error (sqrt of a negative value)" << endl;
	    break;
	 case 3:
	    std::cerr << "log error (logarithm of a negative value)" << endl;
	    break;
	 case 4:
	    std::cerr << "trigonometric error (asin or acos of illegal value)" << endl;
	    break;
	 case 5:
	    std::cerr << "maximum recursion level in eval() reached " << endl;
	    break;
	 default:
	    break;
      }
   }
}

const std::string & DerivedPropertyFunction::getName () const 
{
   return m_name;
}

std::string DerivedPropertyFunction::image () const
{
   std::stringstream buffer;

   buffer << getName () << " (";
   buffer << m_functionParameters << ") = ";
   buffer << m_function;

   return buffer.str ();
}
