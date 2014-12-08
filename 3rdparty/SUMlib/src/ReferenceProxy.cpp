// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "Exception.h"
#include "ReferenceProxy.h"
#include "SerializerUtils.h"

namespace SUMlib {

ReferenceProxy::ReferenceProxy( Proxy const& proxy, double reference, double stddev ) :
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

void ReferenceProxy::setReference( double reference, double stddev )
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

double ReferenceProxy::getProxyValue(Parameter const& p, KrigingType t /* DefaultKriging */ ) const
{
   return m_proxy.getProxyValue( p, t );
}

unsigned int ReferenceProxy::size() const
{
   return m_proxy.size();
}

// Complicated situation: m_proxy is const, it is loaded and saved outside
bool ReferenceProxy::load( IDeserializer* deserializer, unsigned int /*version*/ )
{
   bool ok = true;

   ok = ok && deserialize( deserializer, m_active );
   ok = ok && deserialize( deserializer, m_hasReference );
   ok = ok && deserialize( deserializer, m_reference );
   ok = ok && deserialize( deserializer, m_stddev );

   return ok;
}

bool ReferenceProxy::save( ISerializer* serializer, unsigned int /*version*/ ) const
{
   bool ok = true;

   ok = ok && serialize( serializer, m_active );
   ok = ok && serialize( serializer, m_hasReference );
   ok = ok && serialize( serializer, m_reference );
   ok = ok && serialize( serializer, m_stddev );

   return ok;
}

} // namespace SUMlib
