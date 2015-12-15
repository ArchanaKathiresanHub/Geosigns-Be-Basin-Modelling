#ifndef CLD_ELEMENT_ASSEMBLY_H
#define CLD_ELEMENT_ASSEMBLY_H

#include "ElementAssembly.h"

#include "../../src/FiniteElementTypes.h"

namespace FiniteElementMethod
{
  ///////////////////////////////////////////////////////////////////////////////
  /// @brief Class which implements element matrix assembly using cauldron FiniteElements library
  ///////////////////////////////////////////////////////////////////////////////
  class CldElementAssembly : public ElementAssembly
  {
  public:
      ElementMatrix         Element_Jacobian;
      ElementVector         Element_Residual;
      ElementGeometryMatrix geometryMatrix;
      ElementGeometryMatrix previousGeometryMatrix;

      ElementVector         Current_Element_VES;
      ElementVector         Current_Element_Max_VES;
      ElementVector         Current_Ph;
      ElementVector         Current_Po;
      ElementVector         Current_Lp;
      ElementVector         Previous_Element_Temperature;
      ElementVector         Current_Element_Temperature;
      ElementVector         Current_Element_Chemical_Compaction;
      ElementVector         Element_Heat_Production;

      void AssembleElement();
   };
};

#endif
