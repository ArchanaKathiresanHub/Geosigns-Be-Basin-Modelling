//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_CONST_H_
#define _CBMGENERICS_CONST_H_

namespace CBMGenerics {

const double Pa2MPa = 1e-6;
const double MPa2Pa = 1e6;

const double C2K = 273.15;
const double K2C = -273.15;

const double Fraction2Percentage = 100;

const double Gravity = 9.81;

const double PressureAtSeaLevel = 1.0e5;

const double BoltzmannConstant = 1.38064852e-23; //m2 kg s-2 K-1

const double StockTankPressureMPa  = 101325.0 * 1e-6; // surface conditions pressure in MPa
const double StockTankTemperatureC = 15.0;            // surface conditions termperature in grad Celsius

const double gasRadii[5] = { 1.89e-10, 2.21e-10, 2.46e-10, 2.66e-10, 2.84e-10}; // radius of gas molecules in m (Molecular volumes and the Stokes-Einstein equation, 1970, Journal of Chemical Education 47, no. 4: 261)

} // namespace CBMGenerics

#endif
