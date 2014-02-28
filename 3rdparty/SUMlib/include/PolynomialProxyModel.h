// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_POLYNOMIALPROXYMODEL_H
#define SUMLIB_POLYNOMIALPROXYMODEL_H

#include <map>
#include <string>
#include <vector>

#include "BaseTypes.h"
#include "Proxy.h"
#include "SUMlib.h"

namespace SUMlib {

/// @class PolynomialProxyModel calculates a polynomial model upto second order terms
/// the coefficients are calculated for use with unscaled parameters.
/// unscaled parameters are used for the Proxy::getProxyValue call
class INTERFACE_SUMLIB PolynomialProxyModel : public Proxy
{
   public:

      /// Type of polynomial model
      enum Type
      {
         FULL_QUADRATIC,
         LINEAR_CROSSTERMS,
         LINEAR_QUADRATIC,
         LINEAR
      };

      /// Constructor
      PolynomialProxyModel( bool quadratic = false, bool interaction = false );

      /// Constructor
      PolynomialProxyModel( Type type );

      /// Destructor
      virtual ~PolynomialProxyModel();

      /// Determine the type of polynomial model given the number of parameters and the number of runs
      static Type               determineType( int nrOfParameters, int nrOfRuns );

      // Get the type of polynomial model given a string
      static Type               getType( const std::string& typeText );

      // Get the type of polynomial model as a string
      static const std::string& getTypeText( Type type );

      /// Set values for the type of proxy model
      /// @param [in]  quadratic flag whether quadratic terms are taken into account
      /// @param [in]  interaction flag whether interaction terms are taken into account
      void initialise( bool quadratic, bool interaction );

      /// Convenience function to set the type
      void initialise( Type type );

      /// Return the size of parameterTuples for which the coefficients are calculated
      /// @returns the size of the coefficients arrays
      virtual unsigned int size() const; // Override from Proxy

      /// Determine whether the proxy model has quadratic terms
      /// @returns true if the proxy model has quadratic terms
      bool hasQuadraticTerms() const;

      /// Determine whether the proxy model has interaction terms
      /// @returns true if the proxy model has interaction terms
      bool hasInteractionTerms() const;

      /// Calculate the proxy coefficients based on a simulated responses and the parameter sets
      /// that produced those responses
      void calculateProxyCoefficients(
            ParameterData const& pMin,
            ParameterData const& pMax,
            ParameterSet const& parameterSet,
            TargetSet const& target );

      /// Get the constant coefficient
      double getConstCoefficient() const;

      /// Get the first order coefficients
      /// @returns a vector with coefficients, one for each parameter factor
      const std::vector<double>& getFirstOrderCoefficients() const;

      /// Get the second order coefficients
      /// @returns a vector with coefficients, for each parameter first the quadratic term (one for each parameter factor),
      /// then the interaction (above-diagonal) terms
      const std::vector<double>& getSecondOrderCoefficients() const;

      /// Get the response model value.
      /// @param [in] parameter The set of factor values for which to calculate the response.
      /// @return The response model result.
      /// Implements Proxy
      virtual double getProxyValue( Parameter const& parameter, KrigingType t = GlobalKriging ) const;

      /// Use the default implementation of the getProxyValue call with kriging weights
      using Proxy::getProxyValue;

      /// Calculate the entries for a response matrix (see class mcmc)
      /// @param [in] cp the central parameter
      /// @param [in] stdDev the standard deviation
      /// @param [out] row the row of matrix entries
      void getResponseMatrixRow( std::vector<double> const& cp, double stdDev, std::vector<double>& row ) const;

      /// Write the coefficients to a string
      /// @returns a string with proxy coefficients {{const_coeff}{firstOrderCoeff}{secondOrderCoeff}}
      std::string toString() const;

      // [TODO] methods setConstCoefficient, addFirstOrderTerm and addSecondOrderTerm are required
      // for SUM using SUMDB, but should not belong to the SUMlib implementation

      /// Set the constant coefficient of the proxy
      /// @param [in] coefficient the value of the constant coefficient
      void setConstCoefficient( double coefficient );

      /// Add a first order term
      /// @param [in] coefficient the value of the first order coefficient to add
      /// @param [in] index the index (id) of the parameter factor to which the coefficient applies
      void addFirstOrderTerm( double coefficient, unsigned int index );

      /// @param [in] coefficient the value of the second order coefficient to add
      /// @param [in] index1 the index (id) of the first parameter factor to which the coefficient applies
      /// @param [in] index2 the index (id) of the second parameter factor to which the coefficient applies
      void addSecondOrderTerm( double coefficient, unsigned int index1, unsigned int index2 );

      static const std::map<std::string, PolynomialProxyModel::Type>& getTypes();

   private:

      /// Calculate the matrix of raw (i.e. unscaled) parameter values for the full matrix
      /// @param [in]     parameterSet full list of Parameters
      /// @param [out]    matrix       full matrix of raw coefficients
      // TYPE_DEPENDENT
      void calculateRawMatrix( ParameterSet const& parameterSet, RealMatrix& matrix ) const;

      /// Select active coeffients from the raw matrix
      /// @param [in]     cases  list of flags identifying active cases
      /// @param [in]     vars   list of flags identifying active variables
      /// @param [out]    matrix matrix of raw coefficients for active case/var combinations
      // TYPE_INDEPENDENT
      void getActiveRawMatrix( CaseList const& cases, VarList const& vars, RealMatrix& matrix ) const;

      /// Calculate a scaled row for the design matrix used in the coefficients calculation
      /// Calculate the matrix of raw (i.e. unscaled) parameter value monomials for the matrix used
      /// in the coefficients calculation
      /// @param [in] pMin parameter minimum values
      /// @param [in] pMin parameter range values
      /// @param [in] values parameter values
      /// @param [out] targets
      void calculateRawTargets(
         ParameterData const& pMin,
         ParameterData const& pRange,
         Parameter const& values,
         TargetSet& targets ) const;

      /// Calculate a scaled row for the design matrix used in the coefficients calculation
      virtual void calculateRow(
            DataTuple const& pMin,
            DataTuple const& pRange,
            Parameter const& values,
            std::vector<double>& row ) const;

      /// @brief flags whether quadratic terms are added
      bool m_quadratic;

      /// @brief flags whether interaction terms are added
      bool m_interaction;

      /// Constant coefficient of the response model
      double m_constCoefficient;

      ///
      /// First order coefficients of the response model.
      /// The coefficient at index i is the first order coefficient for factor
      /// index1[i].
      ///
      std::vector<double> m_firstOrderCoefficients;

      /// Factor indices for first order coefficients coef1.
      std::vector<unsigned int> m_firstOrderIndexes;

      ///
      /// Second order coefficients of the response model.
      ///
      /// The coefficient at index i is the second order coefficient for factors
      /// qindex1[i] and qindex2[i].
      ///
      std::vector<double> m_secondOrderCoefficients;

      std::vector<unsigned int> m_secondOrderIndexes1;

      std::vector<unsigned int> m_secondOrderIndexes2;


}; // class PolynomialProxyModel

} // namespace SUMlib

#endif // SUMLIB_POLYNOMIALPROXYMODEL_H
