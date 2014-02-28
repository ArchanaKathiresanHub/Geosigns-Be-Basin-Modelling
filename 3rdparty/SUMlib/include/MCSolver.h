// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCSOLVER_H
#define SUMLIB_MCSOLVER_H

// STD
#include <vector>

// SUMlib
#include "mcmcBase.h"
#include "SUMlib.h"

namespace SUMlib {

class INTERFACE_SUMLIB MCSolver : public mcmcBase {

public:

   // Constructor
   MCSolver( RandomGenerator &rg, std::vector<McmcProxy*>const& ascs, unsigned int sampleSize, const ParameterPdf & pdf, const ParameterBounds & constraints,
         McmcStatistics &statistics, unsigned int maxNbOfIterations = 100 ) : mcmcBase( rg, ascs, sampleSize, pdf, constraints, statistics, maxNbOfIterations )
         {};

   // Destructor
   virtual ~MCSolver(){};

   // Virtual function
   virtual bool acceptProposal( double, double& logAccRatio ) const { return logAccRatio > 1e-18; } //reject negligible improvements!


   // Impl functions
   virtual bool convergenceImpl( vector<vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int maxNbOfTrialsPerCycle );

   virtual void stepImpl( vector<double>& yNew, double& logLhNew, const size_t i );

   virtual double proposeStepImpl1( const vector<double>& pStar, vector<double>& yStar, unsigned int i );

   virtual double proposeStepImpl2( const double logLhStar, const size_t i ){ return (logLhStar - m_logLh[i]); }

   virtual void proposeStepImpl3( const size_t i );

   virtual void updateBestMatchesImpl( double key, Parameter const& p ){ if (uniqueMatch( p ) ) mcmcBase::updateBestMatchesImpl( key, p ); }
};


} // namespace SUMlib

#endif // SUMLIB_MCSOLVER_H
