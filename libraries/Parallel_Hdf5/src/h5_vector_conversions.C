#ifdef SEEK_SET
#undef SEEK_SET
#endif
#include "mpi.h"
//
// Class to create dataset boundaries 
// based on DA Local Vectors
//

#include "h5_vector_conversions.h"

H5_VectorBoundaries::H5_VectorBoundaries (DALocalInfo &vecInfo)
{
   hGlobal = Size  (vecInfo.dim, vecInfo.mx, vecInfo.my, vecInfo.mz);
   hLocal  = Size  (vecInfo.dim, vecInfo.xm, vecInfo.ym, vecInfo.zm);
   hOffset = Start (vecInfo.dim, vecInfo.xs, vecInfo.ys, vecInfo.zs);
} 

