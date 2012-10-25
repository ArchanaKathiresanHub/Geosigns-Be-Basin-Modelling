#include "DerivedPropertyFormationFunction.h"
#include "voxetschemafuncs.h"

#include "DerivedProperty.h"
#include "DerivedPropertyFunction.h"
#include "VoxetProjectHandle.h"
#include "ProjectHandle.h"

DerivedPropertyFormationFunction::DerivedPropertyFormationFunction ( VoxetProjectHandle* voxetHandle,
                                                                     database::Record*   record ) :
   m_voxetHandle ( voxetHandle ),
   m_record ( record ) {

   std::string functionCallParameters = database::getFunctionCallParameters (record);
   splitFunctionCallParameters (functionCallParameters);

}

void DerivedPropertyFormationFunction::splitFunctionCallParameters (const std::string & functionCallParameters)
{
   std::string::size_type index = 0;
   string localFunctionCallParameters = functionCallParameters;

   // remove spaces from the function call parameters string
   while (index != std::string::npos)
   {
      index = localFunctionCallParameters.find (' ', index);
      if (index != std::string::npos)
      {
	 localFunctionCallParameters.erase (index, 1);
      }
   }

   if ( localFunctionCallParameters.length () == 0 ) {
      return;
   }

   std::string::size_type firstIndex = 0;
   std::string::size_type secondIndex = 0;

   // split the comma separated function call parameters string into individual parameters.
   while (secondIndex != std::string::npos)
   {
      secondIndex = localFunctionCallParameters.find (',', firstIndex);
      std::string parameter = localFunctionCallParameters.substr (firstIndex, secondIndex == std::string::npos ? std::string::npos : secondIndex - firstIndex);
      Parameter * aParameter;
      if (isdigit (parameter[0]) || parameter[0] == '.' && isdigit (parameter[1]))
      {
	 aParameter = new Parameter (atof (parameter.c_str ()));
      }
      else
      {
	 const Property * property = m_voxetHandle->getCauldronProjectHandle ()->findProperty (parameter);
	 if (!property)
	 {
	    cerr << "Cannot find function call parameter: " << parameter << " in " << getFunctionName () << endl;
	    cerr << "Function call parameters are: " << functionCallParameters << endl;
	    exit (-1);
	 }

	 aParameter = new Parameter (property);
      }

      m_functionCallParameters.push_back (aParameter);
      firstIndex = secondIndex + 1;
   }
}

const std::string& DerivedPropertyFormationFunction::getFormationName () const {
   return database::getCauldronFormationName ( m_record );
}

const std::string& DerivedPropertyFormationFunction::getDerivedPropertyName () const {
   return database::getDerivedPropertyName ( m_record );
}

const std::string& DerivedPropertyFormationFunction::getFunctionName () const {
   return database::getFunctionName ( m_record );
}

const DerivedProperty* DerivedPropertyFormationFunction::getDerivedProperty () const {
   return m_voxetHandle->getDerivedProperty ( getDerivedPropertyName ());
}

const DerivedPropertyFunction* DerivedPropertyFormationFunction::getDerivedPropertyFunction () const {
   return m_voxetHandle->getDerivedPropertyFunction ( getFunctionName ());
}
