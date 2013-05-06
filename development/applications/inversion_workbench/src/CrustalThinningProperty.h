#ifndef INVERSION_CRUSTALTHINNINGPROPERTY_H
#define INVERSION_CRUSTALTHINNINGPROPERTY_H

#include <vector>

#include "Property.h"
#include "InputParameterDataRow.h"

class Case;

class CrustalThinningProperty : public Property
{
public:
   typedef double Time; // in mA (million of years ago)
   typedef double Thickness; // in meters.
   typedef std::pair<Time, Thickness> ThicknessAtTime;

   CrustalThinningProperty(std::vector<InputParameterDataRow> crustalThinningData);
   virtual void createParameter(Case & project, double value);

   void fromUserInputParameterToCrustalIoTable();

private:
   std::vector<InputParameterDataRow> m_crustalThinningData;
   std::vector< std::vector< ThicknessAtTime > > m_crustalThinningTableForProjectFile;
};


#endif
