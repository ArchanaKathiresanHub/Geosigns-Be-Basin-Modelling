#include <iostream>
#include <vector>
#include "CrustalThinningProperty.h"
#include "CrustalThinningParameter.h"
#include "case.h"

#include <stdio.h>
#include <math.h>

CrustalThinningProperty::CrustalThinningProperty(std::vector<InputParameterDataRow> crustalThinningData)
    : Property("Crustal Thinning",1,1,1)
    , m_crustalThinningData(crustalThinningData) 
{
   for (int i=0; i < crustalThinningData.size(); ++i )
   {
      m_end *= crustalThinningData[i].calculateNumberOfCases();
   }

   this->fromUserInputParameterToCrustalIoTable();
}


void CrustalThinningProperty::fromUserInputParameterToCrustalIoTable()
{
   std::vector< double > T0;
   std::vector< double > DTime;
   std::vector< double > Lcrusti;
   std::vector< double > Beta;

   for ( int i=0; i< m_crustalThinningData.size(); ++i )
   {
      if  ( m_crustalThinningData[i].getStart() <=  m_crustalThinningData[i].getEnd() )
      {
         for ( double value= m_crustalThinningData[i].getStart(); 
               value<= m_crustalThinningData[i].getEnd(); 
               value+= m_crustalThinningData[i].getStep() )
         {
            if (i==0) 
            {
               T0.push_back(value);
            }
            else if (i==1)
            {
               DTime.push_back(value);
            }
            else if (i==2)
            {
               Lcrusti.push_back(value);
            }
            else if (i==3)
            {
               Beta.push_back(value);
            }
         }
      }
      else if  ( m_crustalThinningData[i].getStart() >  m_crustalThinningData[i].getEnd() )
      {
         for ( double value= m_crustalThinningData[i].getEnd(); 
               value<= m_crustalThinningData[i].getStart();
               value+= m_crustalThinningData[i].getStep() )
         {
            if (i==0) 
            {
               T0.push_back(value);
            }
            else if (i==1)
            {
               DTime.push_back(value);
            }
            else if (i==2)
            {
               Lcrusti.push_back(value);
            }
            else if (i==3)
            {
               Beta.push_back(value);
            }
         }
      }
   }

   std::vector< ThicknessAtTime > CrustIoTable(2);

   for ( int i=0; i<T0.size(); ++i )
   {
      for ( int j=0; j<DTime.size(); ++j )
      {
         for ( int k=0; k<Lcrusti.size(); ++k )
         {
            for ( int l=0; l<Beta.size(); ++l )
            {
               double Time1 = T0[i];
               double Time2 = Time1 - DTime[j];
               double Lcrust1 = Lcrusti[k];
               double Lcrust2 = pow(Beta[l],-1.0) * Lcrust1;

               CrustIoTable[0] = std::make_pair( Time2, Lcrust2 );
               CrustIoTable[1] = std::make_pair( Time1, Lcrust1 );

               m_crustalThinningTableForProjectFile.push_back(CrustIoTable);
            }
         }
      }
   }
}


void CrustalThinningProperty::createParameter(Case & project, double value)
{
   project.addParameter( new CrustalThinningParameter(m_crustalThinningTableForProjectFile[int(value)-1]) );
}
