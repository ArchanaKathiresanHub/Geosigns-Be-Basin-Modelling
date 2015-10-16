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
bool serialize( SUMlib::ISerializer* serializer, const SUMlib::ParameterTransformTypeVector& transformDefs )
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
bool deserialize( SUMlib::IDeserializer* deserializer, SUMlib::ParameterTransformTypeVector& transformDefs )
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
   assert( transformDefs.size() == m_parSpace.size() );
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

   /// Cannot initialise twice.
   assert( m_transforms.size() == 0 );

   m_transforms.resize( m_parSpace.getPreparedCaseSize() );

   const Case& caseLow = m_parSpace.low();
   const Case& caseHigh = m_parSpace.high();

   size_t iParNonFixed = 0;

   for ( size_t iPar = 0; iPar < m_parSpace.size(); ++iPar )
   {
      /// Skip fixed parameters
      if ( m_parSpace.isFixed( iPar ) )
      {
         continue;
      }

      if ( iPar < m_parSpace.sizeOrd() )
      {
         double parameterBoundMin = 0;
         double parameterBoundMax = 0;
         if ( iPar < m_parSpace.sizeCon() )
         {
            parameterBoundMin = caseLow.continuousPar( iPar );
            parameterBoundMax = caseHigh.continuousPar( iPar );
         }
         else
         {
            assert( m_transformDefs[ iPar ] == ParameterTransforms::transformConst || m_transformDefs[ iPar ] == ParameterTransforms::transformNone );
         }

         IParameterComponentTransform* parTransform = buildTransform( m_transformDefs[ iPar ], parameterBoundMin, parameterBoundMax );
         if ( !dynamic_cast< TrivialParameterTransform* >( parTransform ) )
         {
            m_isTrivial = false;
         }
         m_transforms[ iParNonFixed ] = parTransform;
         ++iParNonFixed;
      }
      else
      {
         assert( m_transformDefs[ iPar ] == ParameterTransforms::transformConst || m_transformDefs[ iPar ] == ParameterTransforms::transformNone );

         size_t catIndex = iPar - m_parSpace.sizeOrd();
         for ( size_t i = 0; i < m_parSpace.nbOfDummyParsForCat( catIndex ); ++i )
         {
            IParameterComponentTransform* parTransform = buildTransform( m_transformDefs[ iPar ], 0, 0 );
            if ( !dynamic_cast< TrivialParameterTransform* >( parTransform ) )
            {
               m_isTrivial = false;
            }
            m_transforms[ iParNonFixed ] = parTransform;
            ++iParNonFixed;
         }
      }
   }
   
   /// Check that every component of the prepared case has a transform assigned.
   assert( iParNonFixed == m_transforms.size() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// isTrivial
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::isTrivial() const
{
   return m_isTrivial;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// isValid
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParameterTransforms::isValid( std::vector< size_t >& parIndicesWithInvalidTransforms, std::vector< std::string >& reasons ) const
{
   assert( parIndicesWithInvalidTransforms.size() == 0 );
   assert( reasons.size() == 0 );

   bool isValid = true;

   for ( size_t i = 0; i < m_transforms.size(); ++i )
   {
      std::string reason;
      if ( !m_transforms[ i ]->isValid( &reason ) )
      {
         /// parIndex points to index in the parameter vector with fixed parameters included.
         size_t parIndex = m_parSpace.nonFixedParIdx()[ i ];
         parIndicesWithInvalidTransforms.push_back( parIndex );
         reasons.push_back( reason );
         isValid = false;
      }
   }

   return isValid;
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
      case transformConst:
      {
         return new ConstantParameterTransform();
      }

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
/// getConstTransformedParameters
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IndexList ParameterTransforms::getConstTransformedParameters() const
{
   IndexList indices;
   for ( size_t i = 0; i < m_transforms.size(); ++i )
   {
      if ( dynamic_cast< ConstantParameterTransform* >( m_transforms[ i ] ) )
      {
         indices.push_back( i );
      }
   }
   return indices;
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
