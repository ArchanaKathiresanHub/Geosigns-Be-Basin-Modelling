#ifndef _IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_
#define _IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_

#include "database.h"

class AdsorptionProjectHandle;


class IrreducibleWaterSaturationSample {

public :

   IrreducibleWaterSaturationSample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                      database::Record*        record );

   double getCoefficientA () const;

   double getCoefficientB () const;

private :

   double m_coeffA;
   double m_coeffB;

};

#endif // _IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_
