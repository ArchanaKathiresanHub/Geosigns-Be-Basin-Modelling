// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "MC.h"

#include <algorithm>

using std::vector;

namespace SUMlib {

bool MC::convergenceImpl( vector<vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int)
{
   return McmcBase::convergenceImpl_MCMC_MC( sampleVar, stddev, lambda );
}

void MC::iterateOnceImpl()
{
   calcModel( extendSampleToProxyCase( m_pSample ), m_ySample, m_proxyKrigingType );
   CopySampleAndResponse( m_pSample, m_ySample, m_sample_copy );
   updateStatistics();
   m_bestMatches.clear();
   const int n = CountUsed( m_proxies );
   for ( unsigned int i = 0; i < m_sampleSize; ++i )
   {
      double rmse = sqrt( sumOfSquaredErrors( m_ySample[i] )/std::max( 1, n ) );
      m_bestMatches.insert( std::pair<double, const vector<double> > ( rmse, extendSampleToProxyCase( m_pSample[i], i ) ) );
   }
}

void MC::stepImpl( vector<double>& yNew, double& logLhNew, const size_t i )
{
   yNew = m_y[i]; //because here m_y = m_yImpr by definition
   logLhNew = m_logLh[i]; //copy corresponding log likelihood
}



}
