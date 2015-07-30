#ifndef PROPERTY_H_INCLUDED
#define PROPERTY_H_INCLUDED

#include <MeshVizInterface/mesh/data/MiDataSetI.h>

namespace DataAccess
{
  namespace Interface
  {
    class GridMap;
  }
}

class ScalarProperty : public MiDataSetI<double>
{
  const DataAccess::Interface::GridMap* m_values;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double m_minValue;
  double m_maxValue;

  size_t m_timestamp;

public:

  ScalarProperty(const std::string& name, const DataAccess::Interface::GridMap* values);

  virtual double get(size_t i) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;
};

#endif
