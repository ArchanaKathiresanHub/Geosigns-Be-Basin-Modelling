/*
 * Copyright (C) 2015 Shell International Exploration & Production.
 * All rights reserved.
 *
 * Developed under license for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell
 */

#ifndef LOOKUP_H
#define LOOKUP_H

#include <vector>

namespace TSR_Tables { 

class LookUp {
public:
    LookUp(std::vector<double> const &pressure /* rows */, std::vector<double> const &temperature /* colums */);

    // Initialize a look up table, fill-in temparature, pressure and the data array of mol fractions.
    LookUp(std::vector<double> const &pressure /* rows */, std::vector<double> const &temperature /* colums */,
           std::vector<double> const &values );


public:
    // calculate a mol fraction value for the pressure and temperature. Interpolate if needed.
    bool get(double pressure, double temperature, double &value) const;

    // get a value from the array of the lookup data
    double getValue( int i, int j ) const;

    void printTable() const;
    void printValue( const double p1, const double t1 ) const; 
    // check the sizes and undefined values (-1) in m_data
    bool checkValidity() const;
private:
    std::vector<double> m_pressure;
    std::vector<double> m_temperature;
    std::vector<double> m_data;
};

}

#endif 

