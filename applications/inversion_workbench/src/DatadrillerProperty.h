#ifndef INVERSION_DATADRILLER_PROPERTY_H
#define INVERSION_DATADRILLER_PROPERTY_H

#include <string>
#include <vector>

/// Represents a probe for a specific Cauldron project file result property.
class DatadrillerProperty
{

public:
  DatadrillerProperty(const std::string & retrievedVariable, double snapshotTime = 0.0, double positionX = 460001, double positionY = 6750001, double positionBegZ = 0.0, double positionEndZ = 1000.0, double stepZ = 0.0)
     : m_retrievedVariable(retrievedVariable)
     , m_snapshotTime(snapshotTime)
     , m_positionX(positionX)
     , m_positionY(positionY)
     , m_positionBegZ(positionBegZ)
     , m_positionEndZ(positionEndZ)
     , m_stepZ(stepZ)
   {}

   const std::string & getName() const
   { return m_retrievedVariable; }

   void readResults( const std::string & projectFile, 
         double & x, double & y,  std::vector<double> & zs, double & age, 
         std::vector<double> & results) const;

private:
   std::string m_retrievedVariable;
   double m_snapshotTime;
   double m_positionX;
   double m_positionY;
   double m_positionBegZ;
   double m_positionEndZ;
   double m_stepZ;
};

#endif
