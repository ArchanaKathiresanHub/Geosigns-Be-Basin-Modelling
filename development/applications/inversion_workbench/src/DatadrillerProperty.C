#include "DatadrillerProperty.h"
#include "ProjectResultsReader.h"
#include "ScalarRange.h"

void DatadrillerProperty::readResults( const std::string & projectFile, std::vector< double > & results) const
{
  ProjectResultsReader project(projectFile);

  std::vector<double> zs;
  ScalarRange zRange( m_positionBegZ, m_positionEndZ, m_stepZ);

  while (!zRange.isPastEnd())
  {
     zs.push_back( zRange.getValue() );
     zRange.nextValue();
  }

  project.read( m_retrievedVariable, m_snapshotTime, m_positionX, m_positionY, zs, results );
}
