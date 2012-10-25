#ifndef WRITEPROJECT3D
#define WRITEPROJECT3D
#include "cloneproject3d.hh"




void WriteProject3d(std::string inputProject3d, std::string outputProject3d, double wellHeatProd)
{

   CloneProject3d clone = CloneProject3d();
    clone.ModifyHeatProd(inputProject3d,outputProject3d,wellHeatProd);  
}

#endif 
