// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARAMETERCOMPONENTTRANSFORMS_H
#define SUMLIB_PARAMETERCOMPONENTTRANSFORMS_H

#include <memory>
#include <string>

namespace SUMlib
{

/// @interface IParameterComponentTransform.
/// @brief Apply a transform on a component of a parameter vector. The parameter components are assumed to be scaled
///        between [-1, 1].
/// @note Not intended for use outside the ParameterTransforms class (i.e. be careful!).
class IParameterComponentTransform
{
   public:
      /// Readability typedef.
      typedef std::unique_ptr< IParameterComponentTransform > ptr;

   public:
      /// Virtual destructor.
      virtual ~IParameterComponentTransform() {}

      /// Apply transform on parameter component with value x.
      virtual double apply( double x ) const = 0;

      /// Returns true if the transform is well defined.
      /// @param [in] reason     If given, the reason why not valid.
      virtual bool isValid( std::string* reason = 0 ) const = 0;
};



/// @class TrivialParameterTransform.
/// @brief A trivial transformation (@see transformNone in ParameterTransforms).
class TrivialParameterTransform : public IParameterComponentTransform
{
   public:
      double apply( double original ) const;
      /// Trivial transforms cannot be invalid.
      bool isValid( std::string* /*reason*/ ) const;
};



class ConstantParameterTransform : public IParameterComponentTransform
{
   public:
      double apply( double original ) const;
      bool isValid( std::string* /*reason*/ ) const;
};



/// @class NonTrivialParameterTransformBase.
/// @brief Base class for non trivial transforms. It takes care of transforming the prepared (in [-1, 1]) cases to the physical
/// space. The transform should be calculated in the physical space in applyImpl. This base class maps the transformed results 
/// back to [-1, 1]. The following requirement should be met for derived classes:
/// * Provide the applyImpl, isValid and getMonotonicity. 
/// * Call initialise in the constructor.
class NonTrivialParameterTransformBase : public IParameterComponentTransform
{
   public:
      enum Monotonicity
      {
         strictlyIncreasing = 0,
         strictlyDecreasing
      };

   public:
      /// Empty constructor. The method initialise should be called in the derived constructors.
      NonTrivialParameterTransformBase();

      /// Implements IParameterComponentTransform::apply.
      double apply( double original ) const;

   protected:
      /// @brief Implement the non-trivial transform in physical space.
      /// @param [in] x: parameter in physical space.
      virtual double applyImpl( double x ) const = 0;

      /// @brief Get the monotonicity of the transform (@see Monotonicity).
      virtual Monotonicity getMonotonicity() const = 0;

   protected:
      /// Initialisation of the members (@see documentation in source code).
      void initialise( double parameterBoundMin, double parameterBoundMax );

   protected:
      double         m_parScale;
      double         m_parMin;
      double         m_resultScale;
      double         m_resultMin;
};



/// @class LogParameterTransform.
/// @brief A log transformation.
class LogParameterTransform : public NonTrivialParameterTransformBase
{
   public:
      LogParameterTransform( double parameterBoundMin, double parameterBoundMax );
      bool isValid( std::string* reason = 0 ) const;

   protected:
      double applyImpl( double x ) const;
      Monotonicity getMonotonicity() const;
};



/// @class InverseParameterTransform.
/// @brief A 1/x transformation.
class InverseParameterTransform : public NonTrivialParameterTransformBase
{
   public:
      InverseParameterTransform( double parameterBoundMin, double parameterBoundMax );
      bool isValid( std::string* reason = 0 ) const;

   protected:
      double applyImpl( double x ) const;
      Monotonicity getMonotonicity() const;
};



/// @class SqrtParameterTransform.
/// @brief A square root transformation.
class SqrtParameterTransform : public NonTrivialParameterTransformBase
{
   public:
      SqrtParameterTransform( double parameterBoundMin, double parameterBoundMax );
      bool isValid( std::string* reason = 0 ) const;

   protected:
      double applyImpl( double x ) const;
      Monotonicity getMonotonicity() const;
};

} /// namespace SUMlib

#endif // SUMLIB_PARAMETERCOMPONENTTRANSFORMS_H

