#include "PrescribedGridPointCondenser.h"

AllochMod::PrescribedGridPointCondenser::PrescribedGridPointCondenser (  const int xStart,
                                                                         const int xEnd,
                                                                         const int yStart,
                                                                         const int yEnd ) {

  pointIsRequired.setSize ( xStart, xEnd, yStart, yEnd );
  pointIsRequired.fill ( false );
}

void AllochMod::PrescribedGridPointCondenser::fill ( const bool withTheValue ) {
  pointIsRequired.fill ( withTheValue );
}
