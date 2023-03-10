#include "../src/EosPackCAPI.h"
#include "PVTCfgFileMgr.h"

#include <fstream>
#include <stdexcept>

const char * PVTCfgFileMgr::s_cfgFileName = "./PVT_properties.cfg" ;

// Singleton factory
const PVTCfgFileMgr & PVTCfgFileMgr::instance( const char * nm )
{
   static PVTCfgFileMgr cfgFile( nm );
   return cfgFile;
}


// Destructor
PVTCfgFileMgr::~PVTCfgFileMgr()
{
   std::remove( m_cfgFileName );
}

// Constructor - creates PVT_properties.cfg file in current folder
PVTCfgFileMgr::PVTCfgFileMgr( const char * nm )
{
   m_cfgFileName = nm == 0 ? s_cfgFileName : nm;
   std::ofstream ofs( m_cfgFileName, std::ios_base::out | std::ios_base::trunc );
   if ( !ofs.good() )
   {
      throw std::runtime_error( "Can not write PVT configuration file" );
   }

   pvtFlash::SetPvtPropertiesConfigFile( m_cfgFileName ); // Set configuration file name

   ofs << "///component based and general data for PVT" << "\n";
   ofs << "///" << "\n";
   ofs << "///This file contains tables describing 6 COMPONENT-based properties and additionally GENERAL properties " << "\n";
   ofs << "///for PVT (see table headers)." << "\n";
   ofs << "///" << "\n";
   ofs << "///All tables have comma-separated columns. " << "\n";
   ofs << "///" << "\n";
   ofs << "///COMPONENT TABLES:" << "\n";
   ofs << "///" << "\n";
   ofs << "///Every table with component data contains the component names in the first column followed by the property columns." << "\n";
   ofs << "///" << "\n";
   ofs << "///The order of the 6 component property columns must be maintained. " << "\n";
   ofs << "///However, it is possible to split the component tables, i.e. one can use one table containing all component property columns," << "\n";
   ofs << "///or use up to 6 separate tables." << "\n";
   ofs << "///" << "\n";
   ofs << "///GENERAL TABLES:" << "\n";
   ofs << "///" << "\n";
   ofs << "///General data must be contained in one table." << "\n";
   ofs << "///" << "\n";
   ofs << "///All (numerical) data entries to be prescribed (either component or general data) " << "\n";
   ofs << "///are generally interpreted as a piecewise polynomial depending on the " << "\n";
   ofs << "///GORM (mass based gas/oil ratio), i.e. every data entry may be either: " << "\n";
   ofs << "///a constant value," << "\n";
   ofs << "///a polynomial (e.g. 2.0 + 4.0x^3 + 3.4x)," << "\n";
   ofs << "///a piecewise polynomial with an arbitrary number of semicolon separated pieces, each consisting " << "\n";
   ofs << "///of a range [x_a:x_b] followed by a constant or polynomial." << "\n";
   ofs << "///Example:" << "\n";
   ofs << "///[*:1.2]   1.7e-05 ; [1.2:1.8]   -2.7E-05x + 5.1E-05 ;   [1.8:*]   0.0" << "\n";
   ofs << "///(in the above example, the '*'s indicate positive/negative infinity)" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,MolecularWeight,AcentricFactor,VCrit" << "\n";
   ofs << "N2,         28.01352	         ,   0.04000   ,   8.980e-02" << "\n";
   ofs << "COx,        44.00980	         ,   0.22500   ,   9.400e-02" << "\n";
   ofs << "C1,         16.04288	         ,   0.00800   ,   9.900e-02" << "\n";
   ofs << "C2,         30.06982	         ,   0.09800   ,   1.480e-01" << "\n";
   ofs << "C3,         44.09676	         ,   0.15200   ,   2.030e-01" << "\n";
   ofs << "C4,         58.12370	         ,   0.19300   ,   2.550e-01" << "\n";
   ofs << "C5,         72.15064	         ,   0.25100   ,   3.040e-01" << "\n";
   ofs << "H2S,        34.08000	         ,   0.10000   ,   9.850e-02" << "\n";
   ofs << "C6-14Sat,   [0.0:2.5]   103.09 -   0.57x  ;   [2.5:*] 101.665,   0.37272   ,   4.7060e-01" << "\n";
   ofs << "C6-14Aro,   [0.0:2.5]   158.47 - 2.0552x  ;   [2.5:*] 153.332,   0.54832   ,   6.7260e-01" << "\n";
   ofs << "C15+Sat,    [0.0:2.5]   281.85 - 17.194x  ;   [2.5:*] 238.865,   0.86828   ,   1.19620e+00" << "\n";
   ofs << "C15+Aro,    [0.0:2.5]   474.52 - 11.129x  ;   [2.5:*] 446.6975,   1.14380   ,   2.12800e+00" << "\n";
   ofs << "resins,     [0.0:2.5]   618.35 - 7.7908x  ;   [2.5:*] 598.873,   1.16772   ,   2.89200e+00" << "\n";
   ofs << "asphaltenes,[0.0:2.5]   795.12 +  2.785x  ;   [2.5:*] 802.0825,   0.89142   ,   3.84400e+00" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,VolumeShift" << "\n";
   ofs << "N2,                           -4.2300000E-03" << "\n";
   ofs << "COx,                          -1.6412000E-03" << "\n";
   ofs << "C1,                           -5.2000000E-03" << "\n";
   ofs << "C2,                           -5.7900010E-03" << "\n";
   ofs << "C3,                           -6.3499980E-03" << "\n";
   ofs << "C4,                           -6.4900000E-03" << "\n";
   ofs << "C5,                           -5.1200000E-03" << "\n";
   ofs << "H2S,                          -3.840000E-03" << "\n";
   ofs << "C6-14Sat,    [0.0:1.242]   1.737999554e-02 ; [1.242:1.8694]   -2.770163E-02x + 5.178542E-02 ;   [1.8694:*]   0.0   " << "\n";
   ofs << "C6-14Aro,    [0.0:1.242]   2.214000542e-02 ; [1.242:1.8694]   -3.528849E-02x + 6.596831E-02 ;   [1.8694:*]   0.0  " << "\n";
   ofs << "C15+Sat,     [0.0:1.242]   2.573999668e-02 ; [1.242:1.8694]   -4.102646E-02x + 7.669486E-02 ;   [1.8694:*]   0.0  " << "\n";
   ofs << "C15+Aro,                                     [0.0:2.5]        -3.687332E-02x + 1.244384E-03 ;   [2.5:*] -9.09389E-02" << "\n";
   ofs << "resins,                                      [0.0:2.5]         -4.229062E-02x - 3.814919E-02;   [2.5:*] -1.43876E-01" << "\n";
   ofs << "asphaltenes,                                 [0.0:2.5]        -5.542006E-02x - 9.001542E-02 ;   [2.5:*] -2.28566E-01" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,CritPressure" << "\n";
   ofs << "N2,            3394387" << "\n";
   ofs << "COx,           7376462" << "\n";
   ofs << "C1,            4600155" << "\n";
   ofs << "C2,            4883865" << "\n";
   ofs << "C3,            4245519" << "\n";
   ofs << "C4,            3799688" << "\n";
   ofs << "C5,            3374122" << "\n";
   ofs << "H2S,           8936864" << "\n";
   ofs << "C6-14Sat,      [0.0:2.0]   2.81e+06 + 8.71e+04x  +  8.24e+05x^2  -  1.15e+06x^3 +  3.68e+05x^4; [2.0:*]  2.97e+06" << "\n";
   ofs << "C6-14Aro,      [0.0:2.0]   2.07e+06 + 2.51e+05x  +  5.79e+05x^2  -  1.07e+06x^3 +  3.85e+05x^4; [2.0:*]  2.49e+06" << "\n";
   ofs << "C15+Sat,       [0.0:2.0]   1.64e+06 - 2.66e+05x  +  1.27e+06x^2  -  1.48e+06x^3 +  4.83e+05x^4; [2.0:*]  2.08e+06" << "\n";
   ofs << "C15+Aro,       [0.0:2.0]   1.34e+06 - 1.85e+05x  +  1.26e+06x^2  -  1.57e+06x^3 +  5.22e+05x^4; [2.0:*]  1.80e+06" << "\n";
   ofs << "resins,        [0.0:2.0]   1.29e+06 - 2.98e+05x  +  1.43e+06x^2  -  1.67e+06x^3 +  5.43e+05x^4; [2.0:*]  1.74e+06" << "\n";
   ofs << "asphaltenes,   [0.0:2.0]   1.25e+06 - 3.43e+05x  +  1.45e+06x^2  -  1.67e+06x^3 +  5.42e+05x^4; [2.0:*]  1.68e+06" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,CritTemperature" << "\n";
   ofs << "N2,             126.200" << "\n";
   ofs << "COx,            304.200" << "\n";
   ofs << "C1,             190.600" << "\n";
   ofs << "C2,             305.400" << "\n";
   ofs << "C3,             369.800" << "\n";
   ofs << "C4,             425.200" << "\n";
   ofs << "C5,             469.600" << "\n";
   ofs << "H2S,            373.200" << "\n";
   ofs << "C6-14Sat,       [0.0:2.0]    582.13 +  15.00x -	160.73x^2 + 189.96x^3 -  61.28x^4 ; [2.0:*]	 508.426" << "\n";
   ofs << "C6-14Aro,       [0.0:2.0]    686.69 -  41.72x -	101.39x^2 + 179.72x^3 -  65.72x^4 ; [2.0:*]	 583.956" << "\n";
   ofs << "C15+Sat,        [0.0:2.0]    857.09 - 121.02x -	 56.60x^2 + 197.60x^3 -  79.33x^4 ; [2.0:*]	 700.218" << "\n";
   ofs << "C15+Aro,        [0.0:2.0]   1037.50 -  81.42x -	208.72x^2 + 351.63x^3 - 123.74x^4 ; [2.0:*]	 872.986" << "\n";
   ofs << "resins,         [0.0:2.0]   1160.30 -  41.11x -	319.82x^2 + 444.11x^3 - 148.68x^4 ; [2.0:*]	 972.8" << "\n";
   ofs << "asphaltenes,    [0.0:2.0]   1351.30 - 206.25x -	105.01x^2 + 341.23x^3 - 136.23x^4 ; [2.0:*]	 1068.92" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-General-Properties]" << "\n";
   ofs << "EOS,OmegaA,OmegaB,LBC0,LBC1,LBC2,LBC3,LBC4" << "\n";
   ofs << "PR,   4.57240000E-01,   7.77960000E-02,   1.02300000E-01,   2.33640000E-02,   5.85330000E-02,   -4.07580000E-02,   9.33240000E-03" << std::endl;
   ofs << "EndOfTable" << std::endl;
   ofs.close();
}

