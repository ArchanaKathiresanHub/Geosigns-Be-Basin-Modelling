#ifndef _DISTRIBUTEDDATAACCESS_INTERFACE_BIODEGRADATIONPARAMETERS_H_
#define _DISTRIBUTEDDATAACCESS_INTERFACE_BIODEGRADATIONPARAMETERS_H_

#include "../Interface/ProjectHandle.h"
#include "../Interface/DAObject.h"
#include "../Interface/BioConsts.h"

namespace DataAccess 
{ 
   namespace Interface 
   {
	  class BiodegradationParameters: public DAObject
	  {
     public:
      BiodegradationParameters(ProjectHandle* projectHandle, database::Record* record);
     ~BiodegradationParameters();
	 
      /*!
      * \brief Get back the upper temperature limit at which biodegradation can occur.
      * \return the upper temperature limit at which biodegradation can occur (in °C). Set by default to 80°C.
      */
		double maxBioTemp() const;

      /*!
       * \brief Retrieve all the bioconstants for biodegradation.
       * \return BioConst for each component in the folowing order:
       * Asphaltene, Resins, C15+ Aro, C15+ Sat, C6-14 Aro, C6-14 Sat, C5, C4, C3, C2, C1, COx, N2
       */
	 	BioConsts bioConsts() const;

      /*!
      * \brief Get back the timeFactor for biodegradation.
      * \return timeFactor, a user-tuneable rate scalar for all component class (in 1/Myr). Set by default to 0.5.
      */
	 	double timeFactor() const;
	  };
   } 
} // namespace DataAccess::Interface

#endif
