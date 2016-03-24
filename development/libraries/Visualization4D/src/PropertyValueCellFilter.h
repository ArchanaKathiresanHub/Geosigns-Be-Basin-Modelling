#ifndef PROPERTYVALUECELLFILTER_H_INCLUDED
#define PROPERTYVALUECELLFILTER_H_INCLUDED

#include <MeshVizXLM/mesh/cell/MiCellFilterIjk.h>

template<class T>
class MiDataSetIjk;

class PropertyValueCellFilter : public MiCellFilterIjk
{
	double m_minValue;
	double m_maxValue;

	const MiDataSetIjk<double>* m_dataSet;
    size_t m_timestamp;

public:

	PropertyValueCellFilter();

	virtual bool acceptCell(size_t i, size_t j, size_t k) const;

    virtual size_t getTimeStamp() const;

	void setDataSet(const MiDataSetIjk<double>* dataSet);

	void setRange(double minValue, double maxValue);

	double getMinValue() const;

	double getMaxValue() const;
};

#endif
