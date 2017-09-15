/*
 * Copyright (C) 2015 Shell International Exploration & Production.
 * All rights reserved.
 *
 * Developed under license for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell
 */

#ifndef LOOKUP_DIRECTORY_H
#define LOOKUP_DIRECTORY_H

#include <map>

#include "EosPack.h"
#include "LookUp.h"

using namespace std;

namespace TSR_Tables { 

enum TSR_Table { 
    H2S_H2Om, H2S_H2O_NaCl1m, H2S_H2O_NaCl2m, H2S_H2O_NaCl4m, H2S_H2O_NaCl6m,
    CO2_H2Om, CO2_H2O_NaCl1m, CO2_H2O_NaCl2m, CO2_H2O_NaCl4m, CO2_H2O_NaCl6m,
    NumberOfTables };

typedef map < TSR_Table, const double* > TSR_TablesData;

const double invMolarMass = 17.1115674; // 1000 / 58.44 ) (molarMass NaCl = 58.44 g/mol)

class LookUpDirectory {
public:
    LookUpDirectory();
    ~LookUpDirectory();


    const LookUp * getTable( TSR_Table tableName );
    TSR_Table getTableName ( const double salinity, ComponentId id );

    // interpolate a value for salinity
    bool getValueForSalinity( const double salinity, ComponentId id, 
                              const double pressure, const double temperature, double & value );

private:
    TSR_TablesData m_tableData;
    const TSR_Tables::LookUp * m_lookupTables [TSR_Tables::NumberOfTables];
    
    void allocateTable( TSR_Table tableName );
    double convertSalinityToMolal ( const double salinity ) const;

    TSR_Table getLowTableName ( const double mol, ComponentId id );
    TSR_Table getUpTableName ( const double mol, ComponentId id );
};

inline double TSR_Tables::LookUpDirectory::convertSalinityToMolal( const double salinity ) const {

   // salinity in kg/kg NaCl. Convert to molal (mol/kg H2O). NaCl molar mass = 58.44 g/mol
   return salinity * TSR_Tables::invMolarMass;
   
}

}
// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:

#endif 

