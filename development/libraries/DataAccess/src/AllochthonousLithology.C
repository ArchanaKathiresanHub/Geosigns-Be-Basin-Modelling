#include <assert.h>

#include <iostream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Formation.h"
#include "LithoType.h"
#include "AllochthonousLithology.h"

using namespace DataAccess;
using namespace Interface;

AllochthonousLithology::AllochthonousLithology (ProjectHandle * projectHandle, database::Record * record) : DAObject (projectHandle, record) {
  m_formation = 0;
  m_lithoType = 0;
}

AllochthonousLithology::~AllochthonousLithology (void)
{
}

const string & AllochthonousLithology::getFormationName (void) const
{
   return database::getLayerName (m_record);
}

const string & AllochthonousLithology::getLithologyName (void) const
{
   return database::getLithotype (m_record);
}

const Formation * AllochthonousLithology::getFormation (void) const
{

   if ( m_formation == 0 ) {
      m_formation = ( const Interface::Formation* ) m_projectHandle->findFormation ( getFormationName ());
   }

   return m_formation;
}

const LithoType * AllochthonousLithology::getLithoType (void) const
{

   if ( m_lithoType == 0 ) {
      m_lithoType = ( const Interface::LithoType* ) m_projectHandle->findLithoType ( getLithologyName ());
   }

  return m_lithoType;
}

AllochthonousLithologyDistributionList * AllochthonousLithology::getAllochthonousLithologyDistributions (void) const
{
   return m_projectHandle->getAllochthonousLithologyDistributions (this);
}


AllochthonousLithologyInterpolationList * AllochthonousLithology::getAllochthonousLithologyInterpolations (void) const
{
   return m_projectHandle->getAllochthonousLithologyInterpolations (this);
}

void AllochthonousLithology::printOn (ostream & ostr) const
{

  ostr << "AllochthonousLithology:";
  ostr << "   formation name = " << getFormationName ();
  ostr << ", lithology name = " << getLithologyName ();
  ostr << endl;

}
