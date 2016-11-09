// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <vector>

#include "DataStructureUtils.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "ParameterPdf.h"
#include "TornadoSensitivities.h"
#include "Proxy.h"
#include "Case.h"
#include "SUMlib.h"

using std::vector;

namespace SUMlib {

   TornadoSensitivities::TornadoSensitivities()
   {
   }

   TornadoSensitivities::~TornadoSensitivities()
   {
   }

   void TornadoSensitivities::getSensitivities( const Proxy* proxy, const ParameterPdf& priorPar,
         double& refObsValue, vector<vector<double> >& sensitivity, vector<vector<double> >& relSensitivity ) const
   {
      // Initialise sensitivities
      Case parInfo = priorPar.maxParValues();
      const unsigned int totNbOrdPar = parInfo.sizeOrd(); //total number of ordinal parameters
      const unsigned int totNbPar = parInfo.size(); // total number of parameters
      unsigned int temp = 0;
      for ( unsigned int i = 0; i < parInfo.sizeCat(); ++i )
      {
         temp += ( parInfo.categoricalPar( i ) + 1 );
      }
      const unsigned int totNbCatStates = temp;
      const unsigned int totNbSensPar = totNbOrdPar + totNbCatStates;
      sensitivity.clear();
      sensitivity.resize( totNbSensPar );
      for ( unsigned int i = 0; i < totNbOrdPar; ++i )
      {
         sensitivity[i].assign( 2, 0.0 );
      }
      for ( unsigned int i = totNbOrdPar; i < totNbSensPar; ++i )
      {
         sensitivity[i].assign( 1, 0.0 );
      }
      relSensitivity = sensitivity;
      vector<unsigned int> unfrozenParIdx = priorPar.varParIdx();
      double sumOfAbsSensitivities = 0.0;

      // Calculate the reference observable value
      const unsigned int nbOrdPar = priorPar.sizeOrd(); //number of non-frozen ordinal parameters
      const unsigned int nbCatPar = priorPar.sizeCat(); //number of non-frozen categorical parameters
      vector<unsigned int> catCase = priorPar.categoricalBase(); //get categorical base case
      const vector<double> cp = priorPar.scaledOrdinalBase();
      assert( cp.size() == nbOrdPar );
      vector<double> proxyCase = cp;
      priorPar.extendToProxyCase( catCase, proxyCase );
      refObsValue = proxy->getProxyValue( proxyCase );

      // Calculate sensitivities for the ordinal parameters
      unsigned int j;
      for ( j = 0; ( j < unfrozenParIdx.size() ) && ( unfrozenParIdx[j] < totNbOrdPar ); ++j ) //loop over ordinals
      {
         unsigned int i = unfrozenParIdx[j];
         proxyCase[j] = -1.0;
         sensitivity[i][0] = proxy->getProxyValue( proxyCase ) - refObsValue;
         proxyCase[j] = 1.0;
         sensitivity[i][1] = proxy->getProxyValue( proxyCase ) - refObsValue;
         if ( sensitivity[i][0]*sensitivity[i][1] < 0.0 )
         {
            sumOfAbsSensitivities += ( fabs( sensitivity[i][0] ) + fabs( sensitivity[i][1] ) );
         }
         else
         {
            sumOfAbsSensitivities += ( std::max( fabs( sensitivity[i][0] ), fabs( sensitivity[i][1] ) ) );
         }
         proxyCase[j] = cp[j]; //back to base case
      }
      assert( j == nbOrdPar );

      // Calculate sensitivity for each dummy parameter
      unsigned int sensIdx = totNbOrdPar; //sensitivity index corresponding to a categorical state
      for ( unsigned int i = totNbOrdPar; i < totNbPar; ++i ) //loop over all categorical parameters
      {
         const unsigned int tempIdx = sensIdx;
         if ( ( j < unfrozenParIdx.size() ) && ( i == unfrozenParIdx[j] ) )
         {
            vector<unsigned int> catVals = priorPar.catValues()[j - nbOrdPar];
            unsigned int baseCatValue = catCase[j - nbOrdPar];
            for ( unsigned int k = 0; k < catVals.size(); ++k )
            {
               if ( k == 0 ) sensIdx += catVals[k];
               else sensIdx += ( catVals[k] - catVals[k - 1] );
               catCase[j - nbOrdPar] = catVals[k];
               proxyCase = cp;
               priorPar.extendToProxyCase( catCase, proxyCase );
               sensitivity[sensIdx][0] = proxy->getProxyValue( proxyCase ) - refObsValue;
               sumOfAbsSensitivities += ( fabs( sensitivity[sensIdx][0] ) );
               catCase[j - nbOrdPar] = baseCatValue; //back to base (default) value
            }
            j++;
         }
         sensIdx = tempIdx + parInfo.categoricalPar( i - totNbOrdPar ) + 1; //point to right categorical state
      }
      assert( j == nbOrdPar + nbCatPar );
      assert( sensIdx == totNbSensPar );

      // Calculate the relative sensitivities (in %)
      if ( sumOfAbsSensitivities > MachineEpsilon() )
      {
         for ( unsigned int i = 0; i < totNbOrdPar; ++i ) //loop over all ordinal parameters
         {
            relSensitivity[i][0] = 100*sensitivity[i][0]/sumOfAbsSensitivities;
            relSensitivity[i][1] = 100*sensitivity[i][1]/sumOfAbsSensitivities;
         }
         for ( unsigned int i = totNbOrdPar; i < totNbSensPar; ++i ) //loop over all dummy parameters
         {
            relSensitivity[i][0] = 100*sensitivity[i][0]/sumOfAbsSensitivities;
         }
      }
   }

} // namespace SUMlib
