// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_REFERENCEPROXY_H
#define SUMLIB_REFERENCEPROXY_H

#include "McmcProxy.h"
#include "SUMlib.h"

namespace SUMlib {

/// @class ReferenceProxy implements an McmcProxy
class INTERFACE_SUMLIB ReferenceProxy : public McmcProxy
{
   public:

      /// ReferenceProxy without reference value
      /// @param [in] proxy the proxy instance
      explicit ReferenceProxy( Proxy const & proxy );

      /// ReferenceProxy with reference value
      /// @param [in] proxy     the proxy instance
      /// @param [in] reference the reference value (actual or synthetic measurement)
      /// @param [in] stddev    the standard deviation of the measurement
      ReferenceProxy( Proxy const & proxy, double reference, double stddev );

      /// Destructor
      virtual ~ReferenceProxy();

      /// Size of parameters for this proxy
      /// @returns the size of parameters for this proxy
      /// Implements Proxy
      virtual unsigned int size() const;

      /// Proxy function call
      /// @param [in] p parameter to calculate the proxy reponse for
      /// Implements Proxy
      virtual double getProxyValue( Parameter const& p, KrigingType t = DefaultKriging ) const;

      /// Use the default implementation of the getProxyValue call with kriging weights
      using Proxy::getProxyValue;

      /// Set the reference value and the standard deviation
      /// @param [in] reference reference value (actual or synthetic measurement)
      /// @param [in] stddev    standard deviation of the measurement
      void setReference( double reference, double stddev );

      /// Returns wether the ReferenceProxy has reference data
      /// @returns true if reference data is available
      /// Implements McmcProxy
      virtual bool hasReference( ) const;

      /// The reference value for the proxy. This may be a synthetic value
      /// or an actual measurement value
      /// @returns the reference value
      /// Implements McmcProxy
      virtual double getReferenceValue() const;

      /// The standard deviation for the reference value
      /// @returns the standard deviation value
      /// Implements McmcProxy
      virtual double getStdDeviation() const;

      /// Set whether the ReferenceProxy reference value should be used
      /// @param [in] active true if the reference value should be used.
      void setUsed( bool active );

      /// Returns wether the ReferenceProxy should actually be used
      /// @returns true if the proxy should be used
      /// Implements McmcProxy
      virtual bool isUsed( void ) const;

      virtual bool load( IDeserializer*, unsigned int /*version*/ );
      virtual bool save( ISerializer*, unsigned int /*version*/ ) const;

      const Proxy& getProxy() const { return m_proxy; }

   private:

      // non-copyable
      ReferenceProxy( ReferenceProxy const& );
      ReferenceProxy& operator=( ReferenceProxy const & );

   private:
      Proxy  const&  m_proxy;
      bool           m_active;
      bool           m_hasReference;
      double         m_reference;
      double         m_stddev;
};

} // namespace SUMlib

#endif // SUMLIB_REFERENCEPROXY_H
