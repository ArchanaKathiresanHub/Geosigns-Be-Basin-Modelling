#ifndef _GENEX5_POINT_ADSORPTION_HISTORY_H_
#define _GENEX5_POINT_ADSORPTION_HISTORY_H_

#include <string>

#include "database.h"

class AdsorptionProjectHandle;

class PointAdsorptionHistory {

public :

   PointAdsorptionHistory ( AdsorptionProjectHandle* adsorptionProjectHandle,
                            database::Record*        record );

   double getX () const;

   double getY () const;

   const std::string& getFormationName () const;

   const std::string& getMangledFormationName () const;

   std::string image () const;

private :

   AdsorptionProjectHandle* m_adsorptionProjectHandle;
   double m_x;
   double m_y;
   std::string m_formationName;
   std::string m_mangledFormationName;


};

#endif // _GENEX5_POINT_ADSORPTION_HISTORY_H_
