#include <iostream>
#include <vector>
#include "BasementProperty.h"
#include <stdio.h>
#include <math.h>

//typedef double Time; // in mA (million of years ago)
//typedef double Thickness; // in meters.
//typedef std::pair<Time, Thickness> ThicknessAtTime;

using namespace std;

void BasementProperty::CreateParameter(Case & project, double value)
{
  project.addParameter( new BasementParameter( this->getName(), value) );
}



CrustalThinningProperty::CrustalThinningProperty(std::vector<InputParameterDataRow> CrustalThinningData)
    : Property("Crustal Thinning",1,1,1)
    , m_CrustalThinningData(CrustalThinningData) 
  {
//    m_name="Crustal Thinning";
//    m_start=1;
//    m_step=1;
//    m_end=1;

   std::cout << "CrustalThinningData.size(): " << CrustalThinningData.size() << std::endl;

   for (int i=0; i<CrustalThinningData.size(); ++i )
    {

     std::cout << "***CrustalThinningData[i].calculate_nb_cases(): " << CrustalThinningData[i].calculate_nb_cases() << std::endl;
     m_end = m_end * CrustalThinningData[i].calculate_nb_cases();
    }

   std::cout << "m_end: " << m_end << std::endl;

   this->FromUserInputParameterToCrustalIoTable();

   std::cout << "Constructor for CrustalThinningProperty..." << std::endl;

   std::cout << "m_start: " << m_start << std::endl;
   std::cout << "m_end: " << m_end << std::endl;
   std::cout << "m_step: " << m_step << std::endl;

  }


void CrustalThinningProperty::FromUserInputParameterToCrustalIoTable()
{
  
std::vector< double > T0;
std::vector< double > DTime;
std::vector< double > Lcrusti;
std::vector< double > Beta;


std::cout << "m_CrustalThinningData.size(): " << m_CrustalThinningData.size() << std::endl;

for ( int i=0; i< m_CrustalThinningData.size(); ++i )
 {

  if  ( m_CrustalThinningData[i].getStart() <=  m_CrustalThinningData[i].getEnd() )
   {

  for ( double value= m_CrustalThinningData[i].getStart(); value<= m_CrustalThinningData[i].getEnd(); value+= m_CrustalThinningData[i].getStep() )
   {

     std::cout << "Value: " << value << " i: " << i << std::endl;

     std::cout << "m_CrustalThinningData[i].getStart(): " << m_CrustalThinningData[i].getStart() << " m_CrustalThinningData[i].getEnd(): " << m_CrustalThinningData[i].getEnd() << std::endl;

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


  else if  ( m_CrustalThinningData[i].getStart() >  m_CrustalThinningData[i].getEnd() )
   {

  for ( double value= m_CrustalThinningData[i].getEnd(); value<= m_CrustalThinningData[i].getStart(); value+= m_CrustalThinningData[i].getStep() )
   {

     std::cout << "Value: " << value << std::endl;

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




/*  else if  ( m_CrustalThinningData[i].getStart() ==  m_CrustalThinningData[i].getEnd() )
   {

     size_t value= m_CrustalThinningData[i].getEnd();

     std::cout << "Value: " << value << std::endl;

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
*/




 }

std::cout << "T0.size(): " << T0.size() << std::endl;
std::cout << "DTime.size(): " << DTime.size() << std::endl;
std::cout << "Lcrusti.size(): " << Lcrusti.size() << std::endl;
std::cout << "Beta.size(): " << Beta.size() << std::endl;

std::cout << "m_CrustalThinningData[i].getStart(): " << m_CrustalThinningData[0].getStart() << std::endl;
std::cout << "m_CrustalThinningData[i].getEnd(): " << m_CrustalThinningData[0].getEnd() << std::endl;
std::cout << "m_CrustalThinningData[i].getStep(): " << m_CrustalThinningData[0].getStep() << std::endl;

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

	std::cout << "Time1: " << Time1 << " Time2: " << Time2 << " Lcrust1: " << Lcrust1 << " Lcrust2: " << Lcrust2 << std::endl;

        CrustIoTable[0] = std::make_pair( Time2, Lcrust2 );
        CrustIoTable[1] = std::make_pair( Time1, Lcrust1 );

        m_CrustalThinningTableforProjectFile.push_back(CrustIoTable);

       }

     }
   }
 }


std::cout << "m_CrustalThinningTableforProjectFile: " << m_CrustalThinningTableforProjectFile.size() << std::endl;



/*  size_t j=0;

  for ( int value=m_CrustalThinningData[i].getStart(); value<=m_CrustalThinningData[i].getEnd(); value+=m_CrustalThinningData[i].getStep() )
   {

     j=j+1;
     double Time1 = T0[j];
     double Time2 = Time1 + DTime[j];
     double Lcrust1 = Lcrusti[j];
     double Lcrust2 = Beta[j] * Lcrusti[j];

     CrustIoTable[1] = std::make_pair( Time1, Lcrust1 );
     CrustIoTable[2] = std::make_pair( Time2, Lcrust2 );

     m_CrustalThinningTableforProjectFile.push_back(CrustIoTable);

   }
 }*/

}



void CrustalThinningProperty::CreateParameter(Case & project, double value)
{

//std::vector< ThicknessAtTime > & series;

  std::cout << int(value) << "****CrustalThinningProperty::CreateParameter " << m_CrustalThinningTableforProjectFile.size() << std::endl;
  project.addParameter( new CrustalThinningParameter(m_CrustalThinningTableforProjectFile[int(value)-1]) );




/*  int index = static_cast<int>(value);

  std::cout << index << "****CrustalThinningProperty::CreateParameter " << m_CrustalThinningTableforProjectFile.size() << std::endl;
  
  assert( index >= 0 );
  assert( index <  m_CrustalThinningTableforProjectFile.size() );

  project.addParameter( new CrustalThinningParameter(m_CrustalThinningTableforProjectFile[index]) );
*/
}
