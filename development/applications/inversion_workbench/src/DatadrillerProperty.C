#include "DatadrillerProperty.h"
#include "ProjectResultsReader.h"
#include "ScalarRange.h"

void DatadrillerProperty::readResults( const std::string & projectFile, 
      double & x, double & y,  std::vector<double> & zs, double & age, 
      std::vector< double > & results) const
{
  ProjectResultsReader project(projectFile);

  ScalarRange zRange( m_positionBegZ, m_positionEndZ, m_stepZ);
  zs.clear();
  while (!zRange.isPastEnd())
  {
     zs.push_back( zRange.getValue() );
     zRange.nextValue();
  }

  project.read( m_retrievedVariable, m_snapshotTime, m_positionX, m_positionY, zs, results );
  x = m_positionX;
  y = m_positionY;
  age = m_snapshotTime;
}


