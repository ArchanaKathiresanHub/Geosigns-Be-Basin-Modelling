#ifndef _MIGRATION_DIFFUSIONOVERBURDENPROPERTIES_H_
#define _MIGRATION_DIFFUSIONOVERBURDENPROPERTIES_H_

namespace migration {

class DiffusionOverburdenProperties
{
private:

   vector<DiffusionLeak::OverburdenProp> m_diffusionOverburdenProps;
   double m_sealFluidDensity;

public:

   DiffusionOverburdenProperties(const vector<DiffusionLeak::OverburdenProp>& properties,
      const double& sealFluidDensity):
      m_diffusionOverburdenProps(properties),
      m_sealFluidDensity(sealFluidDensity)
   {}

   const vector<DiffusionLeak::OverburdenProp>& properties() const { return m_diffusionOverburdenProps; }
   const double& sealFluidDensity() const { return m_sealFluidDensity; }
};

}
 
#endif
