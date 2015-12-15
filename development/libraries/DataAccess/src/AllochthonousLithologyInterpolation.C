#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/AllochthonousLithologyInterpolation.h"

using namespace DataAccess;
using namespace Interface;



const std::string AllochthonousLithologyInterpolation::ScalingDataSetName = "Scaling";
const std::string AllochthonousLithologyInterpolation::TranslationDataSetName = "Translation";
const std::string AllochthonousLithologyInterpolation::PointsDataSetName = "Points";
const std::string AllochthonousLithologyInterpolation::CoefficientsDataSetName = "Coefficients";
const std::string AllochthonousLithologyInterpolation::RHSDataSetName = "RHS";


AllochthonousLithologyInterpolation::AllochthonousLithologyInterpolation (ProjectHandle * projectHandle, database::Record * record) : DAObject (projectHandle, record) {
  m_formation = ( const Formation* ) m_projectHandle->findFormation ( getFormationName ());
  m_startSnapshot = ( const Snapshot* ) m_projectHandle->findSnapshot ( database::getStartAge (m_record));
  m_endSnapshot = ( const Snapshot* ) m_projectHandle->findSnapshot ( database::getEndAge (m_record));;
}


AllochthonousLithologyInterpolation::~AllochthonousLithologyInterpolation (void)
{
}

const string & AllochthonousLithologyInterpolation::getFormationName (void) const
{
   return database::getLayerName (m_record);
}

const Formation * AllochthonousLithologyInterpolation::getFormation (void) const
{
   return m_formation;
}

const Snapshot* AllochthonousLithologyInterpolation::getStartSnapshot (void) const {
  return m_startSnapshot;
}

const Snapshot* AllochthonousLithologyInterpolation::getEndSnapshot (void) const {
  return m_endSnapshot;
}


int AllochthonousLithologyInterpolation::getNumberOfPoints () const {
  return database::getNumberOfPoints (m_record);
}

int AllochthonousLithologyInterpolation::getPolynomialDegree () const {
  return database::getPolynomialDegree (m_record);
}

const std::string& AllochthonousLithologyInterpolation::getInterpFileName () const {
  return database::getInterpFileName (m_record);
}

const std::string& AllochthonousLithologyInterpolation::getInterpGroupName () const {
  return database::getInterpGroupName (m_record);
}

void AllochthonousLithologyInterpolation::printOn (ostream & ostr) const {
  
  ostr << " AllochthonousLithologyInterpolation: ";
  ostr << "     Formation name  : " << getFormationName () << endl;
  ostr << "     HDF5 file group : " << getInterpGroupName () << endl;
  ostr << "     Age range       : " << getStartSnapshot ()->getTime () << " .. " << getEndSnapshot ()->getTime () << endl;
  ostr << endl;

}
