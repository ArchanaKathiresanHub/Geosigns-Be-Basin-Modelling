#ifndef _DISTRIBUTEDDATAACCESS_INTERFACE_DIFFUSIONLEAKAGEPARAMETERS_H_
#define _DISTRIBUTEDDATAACCESS_INTERFACE_DIFFUSIONLEAKAGEPARAMETERS_H_

#include "DAObject.h"
#include "ProjectHandle.h"

namespace DataAccess { namespace Interface {

enum OverburdenModel {
  FullOverburden,
  LimitedOverburden
};

enum TransientModel {
  Transient,
  SteadyState
};

class DiffusionLeakageParameters: public DAObject
{
public:
   DiffusionLeakageParameters(ProjectHandle* projecthandle, database::Record* record);
   ~DiffusionLeakageParameters();

   double salinity() const;

   OverburdenModel overburdenModel() const;
   double maximumSealThickness() const;

   TransientModel transientModel() const;

   vector<double> diffusionConsts() const;
   double activationEnergy() const;

   vector<double> concentrationConsts() const;
};

} } // namespace DataAccess::Interface

#endif
