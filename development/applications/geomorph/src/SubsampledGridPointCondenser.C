#include "SubsampledGridPointCondenser.h"

AllochMod::SubsampledGridPointCondenser::SubsampledGridPointCondenser () {

  xSubSampling = DefaultSubSampling;
  ySubSampling = DefaultSubSampling;
}

AllochMod::SubsampledGridPointCondenser::SubsampledGridPointCondenser ( const int initialXSubSampling,
                                                                        const int initialYSubSampling ) {

  xSubSampling = initialXSubSampling;
  ySubSampling = initialYSubSampling;

}

void AllochMod::SubsampledGridPointCondenser::setSubSampling ( const int newXSubSampling,
                                                               const int newYSubSampling ) {

  xSubSampling = newXSubSampling;
  ySubSampling = newYSubSampling;
}

bool AllochMod::SubsampledGridPointCondenser::operator ()( const int i,
                                                           const int j ) const {

  return ( i % xSubSampling == 0 ) && ( j % ySubSampling == 0 );
}

