// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "ParameterComponentTransforms.h"
#include "ParameterSpace.h"
#include "ParameterTransforms.h"
#include "SerializerUtils.h"

#include <cassert>
#include <memory>

namespace
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// serialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool serialize( SUMlib::ISerializer* serializer, const std::vector< SUMlib::ParameterTransforms::TransformType >& transformDefs )
{
   std::vector< unsigned int > transformDefsUInt( transformDefs.size() );
   for ( size_t i = 0; i < transformDefs.size(); ++i )
   {
      assert( transformDefs[ i ] >= 0 );
      transformDefsUInt[ i ] = transformDefs[ i ];
   }

   bool ok = SUMlib::serialize( serializer, transformDefsUInt );
   return ok;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// deserialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool deserialize( SUMlib::IDeserializer* deserializer, std::vector< SUMlib::ParameterTransforms::TransformType >& transformDefs )
{
   std::vector< unsigned int > transformDefsUInt;
   bool ok = SUMlib::deserialize( deserializer, transformDefsUInt );

   transformDefs.resize( transformDefsUInt.size() );
   for ( size_t i = 0; i < transformDefsUInt.size(); ++i )
   {
      transformDefs[ i ] = static_cast< SUMlib::ParameterTransforms::TransformType >( transformDefsUInt[ i ] );
   }
   return ok;
}

} /// anonymous namespace

namespace SUMlib
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterTransforms::ParameterTransforms( const std::vector< TransformType >& transformDefs, const ParameterSpace& parSpace ) :
   m_transformDefs( transformDefs ),
   m_parSpace( parSpace )
{
   initialise();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterTransforms::ParameterTransforms()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterTransforms::~ParameterTransforms()
{
   for ( size_t i = 0; i < m_transforms.size(); ++i )
   {
      delete m_transforms[ i ];
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// initialise
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterTransforms::initialise()
{
   m_isTrivial = true;

   assert( m_transforms.size() == 0 );
   m_transforms.resize( m_transformDefs.size() );

   const Case& caseLow = m_parSpace.low();
   const Case& caseHigh = m_parSpace.high();

   size_t iConParNonFixed = 0;

   for ( size_t iConPar = 0; iConPar < m_parSpace.sizeCon(); ++iConPar )
   {
      /// Skip fixed parameters
      if ( m_parSpace.isFixed( iConPar ) )
      {
         continue;
      }

      IParameterComponentTransform* parTransform = buildTransform( m_transformDefs[ iConParNonFixed ], caseLow.continuousPar( iConPar ), caseHigh.continuousPar( iConPar ) );
      assert( parTransform );
      if ( !dynamic_cast< TrivialParameterTransform* >( parTransform ) )
      {
         m_isTrivial = false;
      }
      m_transforms[ iConParNonFixed ] = parTransform;

      ++iConParNonFixed;
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// isTrivial
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::isTrivial() const
{
   return m_isTrivial;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// isValidTransform
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::isValidTransform( TransformType transformType, double min, double max, std::string* reason )
{
   IParameterComponentTransform::ptr transform( buildTransform( transformType, min, max ) );
   bool result = transform->isValid( reason );
   return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// buildTransform
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IParameterComponentTransform* ParameterTransforms::buildTransform( TransformType transformType, double min, double max )
{
   switch ( transformType )
   {
      case transformNone:
      {
         return new TrivialParameterTransform();
      }

      case transformLog:
      {
         return new LogParameterTransform( min, max );
      }

      case transformInv:
      {
         return new InverseParameterTransform( min, max );
      }

      case transformSqrt:
      {
         return new SqrtParameterTransform( min, max );
      }

      default:
      {
         assert( false );
      }
   }
   /// Suppress compiler warning
   return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// apply
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Parameter ParameterTransforms::apply( const Parameter& parameter ) const
{
   Parameter result( parameter );
   for ( size_t i = 0; i < m_transforms.size(); ++i )
   {
      result[ i ] = m_transforms[ i ]->apply( parameter[ i ] );
   }
   return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// load
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::load( IDeserializer* deserializer, unsigned int /*version*/ )
{
   bool ok = true;

   ok &= ok && ::deserialize( deserializer, m_transformDefs );
   ok &= ok && deserialize( deserializer, m_parSpace );

   initialise();

   return ok;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// save
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::save( ISerializer* serializer, unsigned int /*version*/ ) const
{
   bool ok = true;

   ok &= ok && ::serialize( serializer, m_transformDefs );
   ok &= ok && serialize( serializer, m_parSpace );

   return ok;
}

} /// namespace SUMlib
