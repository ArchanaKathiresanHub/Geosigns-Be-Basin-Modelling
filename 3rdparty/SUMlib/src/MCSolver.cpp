// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "MCSolver.h"

using std::vector;

namespace SUMlib {

bool MCSolver::convergenceImpl( vector<vector<double> >&, double&, const double, const unsigned int maxNbOfTrialsPerCycle )
{
   if ( m_acceptanceRate > 50.0 / maxNbOfTrialsPerCycle ) return false;
   return true;
}

void MCSolver::stepImpl( vector<double>& yNew, double& logLhNew, const size_t i )
{
   if ( m_krigingUsage == SmartMcmcKriging )
   {
      calcModel( extendSubSampleToProxyCase( m_pSubSample[i], i) , yNew, m_krigingType ); //calculate expensive yNew
      logLhNew = calcLh( yNew ); //calculate corresponding log likelihood
   }
   else
   {
      yNew = m_y[i]; //because here m_y = m_yImpr by definition
      logLhNew = m_logLh[i]; //copy corresponding log likelihood
   }
}

double MCSolver::proposeStepImpl1( const vector<double>& pStar, vector<double>& yStar, unsigned int i )
{
   KrigingType proxyKriging = m_krigingUsage == FullMcmcKriging ? m_proxyKrigingType : NoKriging;

   calcModel( extendSubSampleToProxyCase( pStar, i ), yStar, proxyKriging ); // not needed for MC
   return calcLh( yStar ); // not needed for MC
}

void MCSolver::proposeStepImpl3( const size_t i )
{
   if ( m_searchStatus[i] == Random ) m_searchStatus[i] = Tornado;
   else m_searchStatus[i] = Terminated;
}

}
