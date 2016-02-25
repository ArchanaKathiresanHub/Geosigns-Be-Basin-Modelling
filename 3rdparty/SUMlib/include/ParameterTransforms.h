// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARAMETER_TRANSFORMS_H
#define SUMLIB_PARAMETER_TRANSFORMS_H

#include "BaseTypes.h"
#include "ParameterSpace.h"
#include "SUMlib.h"

#include <memory>
#include <vector>

/// Forward declares
namespace SUMlib
{
class ParameterSpace;
class IParameterComponentTransform;
}

namespace SUMlib
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ParameterTransforms
/// @brief Class handling the transformations on parameters. Transformations can only be applied on the continuous
///        parameters. For all non-trivial transforms the original bounds, as given in the parameter space, are used.
///
///        Notes:
///        * Transforms for all continous Parameters should be given. The fixed Parameters are ignored.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class INTERFACE_SUMLIB ParameterTransforms : public ISerializable
{
   public:
      typedef std::shared_ptr< ParameterTransforms > ptr;

      enum TransformType
      {
         transformConst = 0,  //! Constant transform:           x -> 0.
         transformNone,       //! No, or trivial transform:     x -> x.
         transformLog,        //! Log transform:                x -> log(x).
         transformInv,        //! Inverse/reciprocal transform: x -> 1/x.
         transformSqrt,       //! Square-root transform:        x -> sqrt(x).
         numTransforms        //! Number of transforms.
      };


      /// Constructor.
      /// @param [in] transformDef        for every continuous parameter the transform type is specified. This includes the
      ///                                 transforms for the fixed parameters. These will be filtered out.
      /// @param [in] parSpace            the parameter space contains information about the scaling of the transforms.
      ParameterTransforms( const std::vector< TransformType >& transformDef, const ParameterSpace& parSpace );

      /// Destructor.
      ~ParameterTransforms();

      /// Returns true iff all parameters are transformed trivially (i.e. transformNone).
      bool isTrivial() const;

      /// Applies the transform on a parameter.
      /// @param [in] parameter        the parameter to be transformed. It is assumed that the parameter is a prepared
      ///                              SUMlib case (i.e. scaled between [-1, 1]).
      /// @return the transformed parameter.
      Parameter apply( const Parameter& parameter ) const;


      /// Check if the transforms contained in this instance are all valid.
      /// @param [in] parIndicesWithInvalidTransforms the parameter indices that have invalid transforms.
      /// @param [in] reason                          this string will contain the reason in case the transform is not valid.
      /// @return true iff the transform type with the attributes is a valid transform.
      bool isValid( std::vector< size_t >& parIndicesWithInvalidTransforms, std::vector< std::string >& reasons ) const;

      /// Check if the transform is valid.
      /// @param [in] transformType    the transform type enumeration.
      /// @param [in] min              minimum value of the physical parameter space.
      /// @param [in] max              maximum value of the physical parameter space.
      /// @param [in] reason           this string will contain the reason in case the transform is not valid.
      /// @return true iff the transform type with the attributes is a valid transform.
      static bool isValidTransform( TransformType transformType, double min, double max, std::string* reason = 0 );

      /// Get the parameter indices that have a constant transform.
      /// @return the parameter indices
      IndexList getConstTransformedParameters() const;

      /// ISerializable interface implementation. @see ISerializable.
      bool load( IDeserializer* deserializer, unsigned int version );
      bool save( ISerializer* serializer, unsigned int version ) const;

      /// Default constructor.
      /// Only to be called in the context of serialization.
      ParameterTransforms();

   private:
      /// Build a parameter transform for a single component.
      static IParameterComponentTransform* buildTransform( TransformType tansformType, double min, double max );

      /// Initialises the transforms. The members m_transformDefs and m_parSpace need to be set.
      void initialise();

   private:
      bool                                         m_isTrivial;         //! True iff all components have trivial transformations.
      std::vector< IParameterComponentTransform* > m_transforms;        //! Transformations for every non-fixed continuous parameter component.
      std::vector< TransformType >                 m_transformDefs;     //! Transformation definitions for all continuous parameters (including fixed).
                                                                        //! Note that this implies m_transformDefs.size() != m_transforms.size() in general.
      ParameterSpace                               m_parSpace;          //! Parameter space.
};

/// Readability typedef
typedef std::vector< ParameterTransforms::TransformType > ParameterTransformTypeVector;

} /// namespace SUMlib

#endif
