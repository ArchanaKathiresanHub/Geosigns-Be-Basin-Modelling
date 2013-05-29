#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "propinterface.h"
#include "snapshotdata.h"


double getPropValOfFirstNodeOnSurface(DA a_DA,
                                      Vec* a_vec,
                                      const SurfacePosition surfacePosition, 
                                      const double factor,
                                      const Boolean2DArray& validNeedle );

void getPropValsOfFirstColumnInLayer(DA a_DA,
                                     Vec* a_vec,
                                     const double factor,
                                     const Boolean2DArray& validNeedle,
                                     list<double>& returnValList);

#endif /* _OUTPUT_H_ */
