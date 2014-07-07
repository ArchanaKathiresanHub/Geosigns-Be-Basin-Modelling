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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class INTERFACE_SUMLIB ParameterTransforms : public ISerializable
{
   public:
      typedef std::auto_ptr< ParameterTransforms > ptr;

      enum TransformType
      {
         transformNone,    //! No, or trivial transform:     x -> x.
         transformLog,     //! Log transform:                x -> log(x).
         transformInv,     //! Inverse/reciprocal transform: x -> 1/x.
         transformSqrt     //! Square-root transform:        x -> sqrt(x).
      };

      /// Constructor.
      /// @param [in] transformDef        for every continuous parameter the transform type is specified.
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

      /// Check if the transform is valid.
      /// @param [in] transformType    the transform type enumeration.
      /// @param [in] min              minimum value of the physical parameter space.
      /// @param [in] max              maximum value of the physical parameter space.
      /// @param [in] reason           this string will contain the reason in case the transform is not valid.
      /// @return true iff the transform type with the attributes is a valid transform.
      static bool isValidTransform( TransformType transformType, double min, double max, std::string* reason = 0 );

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
      std::vector< IParameterComponentTransform* > m_transforms;        //! Transformations for every continuous parameter component.
      std::vector< TransformType >                 m_transformDefs;     //! Transformation definitions.
      ParameterSpace                               m_parSpace;          //! Parameter space.
};

} /// namespace SUMlib

#endif
