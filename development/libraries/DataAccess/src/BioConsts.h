//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DISTRIBUTEDDATAACCESS_BIOCONSTS_H
#define DISTRIBUTEDDATAACCESS_BIOCONSTS_H

#include <iostream>
#include <vector>
#include <assert.h>

#include "Interface.h"

namespace DataAccess {

   namespace Interface {

      class BioConsts
      {
      
      public:

         BioConsts( const double& tempFactor, const std::vector<double>& bioConsts ) :
            m_tempFactor( tempFactor ),
            m_bioConsts( bioConsts )
         {
            // No biodegradation allowed for CO2
            /* For May 2016 release the code below is commented, as in v2015.1104. Reactivate for NGBA releases 
            if (bioConsts[COX] != 1.0)
            {
               cerr << "Warning: you try to biodegrade CO2. The CO2 cannot be biodegraded, its biodegradation coefficient has been reset to 1.0" << endl;
               m_bioConsts[COX] = 1.0;
            }
            */            
         }

         size_t size() const { return m_bioConsts.size(); }

         double operator[]( int index ) const
         {
         
            assert( index >= 0 && index < (int)m_bioConsts.size() );
            /* For May 2016 release the code below is commented, as in v2015.1104. Reactivate for NGBA releases
            if (m_bioConsts[index] < 0.0)
            {
               cerr << "Warning: the biodegradation coefficient of the component " << index << " is negative: " << m_bioConsts[index] << ". No biodegradation computed for this component" << endl;
               return -199999;
            }
            if (m_tempFactor < 0.0)
            {
               cerr << "Warning: The temperature factor coefficient used for biodegradation is negative: " << m_tempFactor << ". No biodegradation computed" << endl;
               return -199999;
            }
            */
         
            return m_bioConsts[ index ] * m_tempFactor;
         }
      
      private:

         double const m_tempFactor;
         std::vector<double> m_bioConsts;
      };
   } // namespace DataAccess::Interface
} // namespace DataAccess

#endif
