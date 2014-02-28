// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "Exception.h"
#include "ReferenceProxy.h"

namespace SUMlib {

const double ReferenceProxy::defaultStdDev = 1.0;

ReferenceProxy::ReferenceProxy( Proxy const& proxy, double reference, double stddev /* defaultStdDev */ ) :
   m_proxy( proxy ),
   m_active( true ),
   m_hasReference( true ),
   m_reference( reference ),
   m_stddev( stddev )
{
   // empty
}

ReferenceProxy::ReferenceProxy( Proxy const& proxy ) :
   m_proxy( proxy ),
   m_active( false ),
   m_hasReference( false )
{
   // empty
}

ReferenceProxy::~ReferenceProxy( )
{
   // empty
}

void ReferenceProxy::setUsed( bool active )
{
   m_active = active;
}

bool ReferenceProxy::isUsed() const
{
   return m_active;
}

bool ReferenceProxy::hasReference() const
{
   return m_hasReference;
}

void ReferenceProxy::setReference( double reference, double stddev /* defaultStdDev */ )
{
   m_hasReference = true;
   m_reference = reference;
   m_stddev = stddev;
}

double ReferenceProxy::getReferenceValue() const
{
   if ( ! m_hasReference )
   {
      THROW2( InvalidState, "No reference value available" );
   }
   return m_reference;
}

double ReferenceProxy::getStdDeviation() const
{
   if ( ! m_hasReference )
   {
      THROW2( InvalidState, "No reference value available" );
   }
   return m_stddev;
}

double ReferenceProxy::getProxyValue(Parameter const& p, KrigingType ) const
{
   return m_proxy.getProxyValue( p );
}

unsigned int ReferenceProxy::size() const
{
   return m_proxy.size();
}

} // namespace SUMlib
