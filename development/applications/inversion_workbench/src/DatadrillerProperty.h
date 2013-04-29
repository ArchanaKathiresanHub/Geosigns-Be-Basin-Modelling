#ifndef INVERSION_DATADRILLER_PROPERTY_H
#define INVERSION_DATADRILLER_PROPERTY_H

#include <string>
#include <vector>

class DatadrillerProperty
{

public:
  DatadrillerProperty(const std::string & RetrievedVariable, double SnapshotTime = 0.0, double PositionX = 460001, double PositionY = 6750001, double PositionBegZ = 0.0, double PositionEndZ = 1000.0, double StepZ = 0.0)
    : m_RetrievedVariable(RetrievedVariable)
    , m_SnapshotTime(SnapshotTime)
    , m_PositionX(PositionX)
    , m_PositionY(PositionY)
    , m_PositionBegZ(PositionBegZ)
    , m_PositionEndZ(PositionEndZ)
    , m_StepZ(StepZ)
  {}

  const std::string & getName() const
  { return m_RetrievedVariable; }

  double getTime() const
  { return m_SnapshotTime; }

  double getPositionX() const
  { return m_PositionX; }

  double getPositionY() const
  { return m_PositionY; }

  double getPositionBegZ() const
  { return m_PositionBegZ; }

  double getPositionEndZ() const
  { return m_PositionEndZ; }

  double getStepZ() const
  { return m_StepZ; }


private:
  std::string m_RetrievedVariable;
  double m_SnapshotTime;
  double m_PositionX;
  double m_PositionY;
  double m_PositionBegZ;
  double m_PositionEndZ;
  double m_StepZ;


};

#endif
