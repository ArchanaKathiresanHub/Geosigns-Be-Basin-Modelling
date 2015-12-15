// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCMCPROXY_H
#define SUMLIB_MCMCPROXY_H

#include "Proxy.h"
#include "SUMlib.h"

namespace SUMlib {


/// a Proxy with associated reference value and weight for use in Mcmc
class INTERFACE_SUMLIB McmcProxy : public Proxy
{
   public:

      /// Destructor
      virtual ~McmcProxy(){}

      /// Returns wether the McmcProxy should actually be used
      /// @returns true if the proxy should be used
      virtual bool isUsed( void ) const = 0;

      /// Returns wether the McmcProxy has reference data
      /// @returns true if reference data is available
      virtual bool hasReference( ) const = 0;

      /// The reference value for the proxy. This may be a synthetic value
      /// or an actual measurement value
      /// @returns the reference value
      virtual double getReferenceValue() const = 0;

      /// The standard deviation for the reference value
      /// @returns the standard deviation value
      virtual double getStdDeviation() const = 0;
      
      /// Set whether the reference value of the proxy should be used
      /// @param [in] active  true if the reference value must be used.
      virtual void setUsed( bool active ) = 0;
      
      /// Set the reference value and the standard deviation
      /// @param [in] reference reference value (actual or synthetic measurement)
      /// @param [in] stddev    standard deviation of the measurement
      virtual void setReference( double reference, double stddev ) = 0;

      /// Calculate the error of a specified value with respect to the reference value,
      /// scaled by the standard deviation
      /// @param [in] value to calculate scaled error for
      /// @returns the calculated error value
      inline double getScaledError( double value ) const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int /*version*/ )
      {return false;}
      virtual bool save( ISerializer*, unsigned int /*version*/ ) const
      {return false;}

};

// Inline function implementations
double McmcProxy::getScaledError( double value ) const { return ( value - getReferenceValue() )/ getStdDeviation(); }

} // namespace SUMlib

#endif // SUMLIB_MCMCPROXY_H
