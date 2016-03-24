#include "PropertyValueCellFilter.h"

#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>
#include <MeshVizXLM/MxTimeStamp.h>

PropertyValueCellFilter::PropertyValueCellFilter()
	: m_minValue(0.0)
	, m_maxValue(1.0)
	, m_dataSet(nullptr)
    , m_timestamp(MxTimeStamp::getTimeStamp())
{
}

bool PropertyValueCellFilter::acceptCell(size_t i, size_t j, size_t k) const
{
  if (!m_dataSet)
    return true;

  double val = m_dataSet->get(i, j, k);

  return val >= m_minValue && val <= m_maxValue;
}

size_t PropertyValueCellFilter::getTimeStamp() const
{
  return m_timestamp;
}

void PropertyValueCellFilter::setDataSet(const MiDataSetIjk<double>* dataSet)
{
  if (dataSet != m_dataSet)
  {
    m_dataSet = dataSet;
    m_timestamp = MxTimeStamp::getTimeStamp();
  }
}

void PropertyValueCellFilter::setRange(double minValue, double maxValue)
{
  if (minValue != m_minValue || maxValue != m_maxValue)
  {
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_timestamp = MxTimeStamp::getTimeStamp();
  }
}

double PropertyValueCellFilter::getMinValue() const
{
  return m_minValue;
}

double PropertyValueCellFilter::getMaxValue() const
{
  return m_maxValue;
}
