#include "BrooksCorey.h"

// Declare some static variables; they are defined in the header 

const double BrooksCorey::Sir = 0.1;        //connnate saturation -- irreducible saturaiton
const double BrooksCorey::Pe = 1000000;         //entry pressure = 1 MPa
const double BrooksCorey::Sor = 0.3;        //residual oil saturation
const double BrooksCorey::Krwor = 1.0;      // end point relative permeability of Brine
const double BrooksCorey::Krocw = 1.0;      // end point relative permeability of HC
const double BrooksCorey::Adjustment = 1.0e-4;
