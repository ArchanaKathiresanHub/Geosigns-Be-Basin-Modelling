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
      typedef std::auto_ptr< IParameterComponentTransform > ptr;

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
      bool isValid( std::string* /*reason*/ ) const;
};



/// @class LogParameterTransform.
/// @brief A log transformation.
class LogParameterTransform : public IParameterComponentTransform
{
   public:
      LogParameterTransform( double parameterBoundMin, double parameterBoundMax );
      double apply( double original ) const;
      bool isValid( std::string* reason = 0 ) const;

   private:
      double         m_parScale;
      double         m_parMin;
};



/// @class InverseParameterTransform.
/// @brief A 1/x transform.
class InverseParameterTransform : public IParameterComponentTransform
{
   public:
      InverseParameterTransform( double parameterBoundMin, double parameterBoundMax );
      double apply( double original ) const;
      bool isValid( std::string* reason = 0 ) const;

   private:
      double         m_parScale;
      double         m_parMin;
};



/// @class SqrtParameterTransform.
/// @brief A square-root transform.
class SqrtParameterTransform : public IParameterComponentTransform
{
   public:
      SqrtParameterTransform( double parameterBoundMin, double parameterBoundMax );
      double apply( double original ) const;
      bool isValid( std::string* reason = 0 ) const;

   private:
      double         m_parScale;
      double         m_parMin;
};

} /// namespace SUMlib

#endif // SUMLIB_PARAMETERCOMPONENTTRANSFORMS_H

