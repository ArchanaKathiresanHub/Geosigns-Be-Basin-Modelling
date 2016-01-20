#ifndef DATASET_H_INCLUDED
#define DATASET_H_INCLUDED

#include <MeshVizXLM/MxTimeStamp.h>
#include <MeshVizXLM/mesh/data/MiDataSetIj.h>
#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>

#include <algorithm>

class DataSetIj : public MiDataSetIj<double>
{
  size_t m_numI;
  size_t m_numJ;

  double* m_values;
  double  m_minValue;
  double  m_maxValue;

  size_t m_timestamp;

public:

  DataSetIj(double* values, size_t ni, size_t nj)
    : m_values(values)
    , m_numI(ni)
    , m_numJ(nj)
    , m_timestamp(MxTimeStamp::getTimeStamp())
  {
    m_minValue = std::numeric_limits<double>::infinity();
    m_maxValue = -m_minValue;

    size_t n = ni * nj;
    for (size_t i = 0; i < n; ++i)
    {
      double v = m_values[i];
      if (v != 99999.0)
      {
        m_minValue = std::min(m_minValue, v);
        m_maxValue = std::max(m_maxValue, v);
      }
    }
  }

  DataSetIj(const DataSetIj&) = delete;
  DataSetIj& operator=(const DataSetIj&) = delete;

  virtual ~DataSetIj()
  {
    delete[] m_values;
  }

  virtual double get(size_t i, size_t j) const
  {
    return m_values[j * m_numI + i];
  }

  virtual MiMeshIjk::StorageLayout getLayout() const
  {
    return MiMeshIjk::LAYOUT_KJI;
  }

  virtual DataBinding getBinding() const
  {
    return MiDataSet::PER_CELL;
  }

  virtual size_t getTimeStamp() const
  {
    return m_timestamp;
  }

  virtual std::string getName() const { return "DataSet"; }
  virtual double getMin() const { return m_minValue; }
  virtual double getMax() const { return m_maxValue; }
};


class DataSetIjk : public MiDataSetIjk<double>
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double* m_values;
  double m_minValue;
  double m_maxValue;

  size_t m_timestamp;

public:

  DataSetIjk(double* values, size_t ni, size_t nj, size_t nk)
    : m_values(values)
    , m_numI(ni)
    , m_numJ(nj)
    , m_numK(nk)
    , m_timestamp(MxTimeStamp::getTimeStamp())
  {
    m_minValue = std::numeric_limits<double>::infinity();
    m_maxValue = -m_minValue;

    size_t n = ni * nj;
    for (size_t i = 0; i < n; ++i)
    {
      double v = m_values[i];
      if (v != 99999.0)
      {
        m_minValue = std::min(m_minValue, v);
        m_maxValue = std::max(m_maxValue, v);
      }
    }
  }

  DataSetIjk(const DataSetIjk&) = delete;
  DataSetIjk& operator=(const DataSetIjk&) = delete;

  virtual ~DataSetIjk()
  {
    delete[] m_values;
  }

  virtual double get(size_t i, size_t j, size_t k) const
  {
    return m_values[k * m_numI * m_numJ + j * m_numI + i];
  }

  virtual MiMeshIjk::StorageLayout getLayout() const
  {
    return MiMeshIjk::LAYOUT_KJI;
  }

  virtual DataBinding getBinding() const
  {
    return MiDataSet::PER_CELL;
  }

  virtual size_t getTimeStamp() const
  {
    return m_timestamp;
  }

  virtual std::string getName() const { return "DataSet"; }
  virtual double getMin() const { return 0.0; }
  virtual double getMax() const { return 3.0; }
};

#endif
