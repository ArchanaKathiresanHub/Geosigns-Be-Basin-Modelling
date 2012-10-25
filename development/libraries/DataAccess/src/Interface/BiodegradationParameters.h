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
	 
		double maxBioTemp() const;
	 	BioConsts bioConsts() const;
	 	double timeFactor() const;
	  };
   } 
} // namespace DataAccess::Interface

#endif
