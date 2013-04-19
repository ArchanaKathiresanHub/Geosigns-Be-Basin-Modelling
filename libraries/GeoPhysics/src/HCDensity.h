#ifndef _GEOPHYSICS__HC_DENSITY_H_
#define _GEOPHYSICS__HC_DENSITY_H_

#include "IBSinterpolator2d.h"

#include "database.h"

namespace GeoPhysics {

#define TEMP_NUM 80
#define PRESSURE_NUM 160

   class HCDensity {

  public :

    // Constructor.
    HCDensity ();
    
    // Destructor.
    ~HCDensity ();
       
    // Compute the hydrocarbon density.
    //
    double density ( unsigned int phaseId, const double temperature, const double pressure ) const { 
      return densityFromTable( phaseId, temperature, pressure );
    }
    
    // Interpolate the hydrocarbon density for given temperature and pressure
    double densityFromTable ( unsigned int phaseId, const double temperature, const double pressure ) const;

    void createPropertyTables ();

    double computeLiquid( const double temperature, 
                          const double pressure ) const;
    double computeVapour( const double temperature, 
                          const double pressure ) const;
 
  private :
    /// The interpolator for the density table.
    ///
    /// It depends on both temperature and pressure.

    mutable ibs::Interpolator2d m_vapourDensityTbl;
    mutable ibs::Interpolator2d m_liquidDensityTbl;

    double m_startTempValue;
    double m_startPressValue;
    double m_stepTempValue;
    double m_stepPressValue;

    bool m_isComputed;
  };

} // end GeoPhysics

#endif // _GEOPHYSICS__HC_DENSITY_H_
