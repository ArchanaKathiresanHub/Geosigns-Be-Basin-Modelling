#ifndef PROPERTY_H_INCLUDED
#define PROPERTY_H_INCLUDED

#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>

namespace DataAccess
{
  namespace Interface
  {
    class GridMap;
  }
}

class ScalarProperty : public MiDataSetIjk<double>
{
  const DataAccess::Interface::GridMap* m_values;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  double m_minValue;
  double m_maxValue;

  size_t m_timestamp;

public:

  ScalarProperty(const std::string& name, const DataAccess::Interface::GridMap* values);

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

class VectorProperty : public MiDataSetIjk<MbVec3d>
{
  const DataAccess::Interface::GridMap* m_values[3];

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  //size_t m_numI;
  //size_t m_numJ;
  //size_t m_numK;

  size_t m_timestamp;

public:

  VectorProperty(const std::string& name, const DataAccess::Interface::GridMap* values[3]);

  virtual MbVec3d get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

#endif
