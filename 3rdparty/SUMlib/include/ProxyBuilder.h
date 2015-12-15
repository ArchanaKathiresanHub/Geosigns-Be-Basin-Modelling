// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROXYBUILDER_H
#define SUMLIB_PROXYBUILDER_H

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib
{
class CubicProxy;
class EliminationCriterion;

/// @class ProxyBuilder collects the results of SVD computations for a proxy, given
/// a parameter set, a target set and a list of active variables.
///
/// Generated Copy constructor and copy assignment are safe
class INTERFACE_SUMLIB ProxyBuilder
{
   public:
      /// Constructor
      /// @param [in] parSet   parameterset
      /// @param [in] varList  list of active monomial variables
      ProxyBuilder( ParameterSet const& parSet, TargetSet const& targetSet, VarList const& varList );

      /// Destructor
      virtual                      ~ProxyBuilder();

      /// Progressively eliminates items from the list of active monomial variables.
      /// @param [in] criterion  used to decide which monomial variable to eliminate next and when to stop the elimination process.
      unsigned int eliminate( EliminationCriterion& criterion );

      /// Create a proxy for the base variables
      ///
      /// Note: the cubic proxy must be destroyed by the caller
      CubicProxy                   *create( ) const;

      /// Create a proxy for the base variables and one variable more/less.
      /// @param [in] varIndx variable index to include/exclude
      ///
      /// If varIndx is in the base variables, it is eliminated from the list of variables for the cubic proxy,
      /// if varIndx is not in the base variables it is added to the list of variables for the cubic proxy.
      /// Note: the cubic proxy must be destroyed by the caller
      CubicProxy                   *create( unsigned int varIndx ) const;

      /// get the size of parameters
      /// @returns the size of the parameters that a proxy can handle
      unsigned int                  size() const;

      /// @returns the parameter set
      ParameterSet const&           parSet() const { return m_parSet; }

      /// @returns the target set
      TargetSet const&              targets() const { return m_scaledTargets; }

      /// Base list of active variables. This builder can create proxies for
      /// the base list and any list that has one variable more or less.
      /// @returns the base list of variables
      VarList const&                baseVars() const { return m_vars; }

      /// Monomial codes of all variables of the list of base variables
      std::vector<IndexList> const& code() const { return m_code; }

      /// Model matrix containing monomial values of the cubic proxy.
      /// @returns the model matrix
      ProxyData const&              proxyData() const { return m_proxyData; }

      /// Statistical mean of the parameter set
      /// @returns a vector of size parSet.size()
      RealVector const&             proxyMean() const { return m_proxyMean; }

      /// The diagonal matrix resulting from the SVD
      /// @returns the matrix
      RealVector const&             singularValues() const { return m_singularValues; }

      /// The orthonormal matrices resulting from the SVD
      /// @returns an orthonormal matrix
      RealMatrix const&             orthonormalU() const { return m_orthonormalU; }
      RealMatrix const&             orthonormalV() const { return m_orthonormalV; }

      /// Statistical mean of the target set
      /// @returns the mean
      double                        targetMean() const { return m_targetMean; }

      /// The coefficients for the base variables
      /// @returns the vector
      RealVector const&             coefficients() const { return m_coefficients; }

      /// The mean squared error with respect to the target set
      /// @returns the mean squared error
      double mse() const;

      /// Calculates the standard errors on the coefficients.
      /// @returns the vector  of standard errors
      RealVector calcStdErrors() const;

      unsigned int calcDesignMatrixRank() const;

   private:
      void fit();

      void setMonomialCode( VarList const& vars, std::vector<IndexList> &code ) const;

      void calculateProxyData( ParameterSet const& parSet, ProxyData& data, RealVector &mean ) const;

      /// Calculate extra column v if the model must be augmented with one monomial
      /// @param [in] varIndx   monomial (var) index
      /// @param [out] parIndx  single-monomial code containing the indices to the parameter elements
      /// @param [out] v        column vector scaled with the return value
      /// @returns the average of the unscaled column vector elements
      double addVarColumn( unsigned int varIndx, IndexList& parIndx, RealVector& v ) const;

      /// Calculate coefficients if the model must add one monomial
      /// @param [in] c        added column vector
      /// @param [out] r       updated coefficients corresponding to the base vars
      /// @param [out] s       updated coefficient corresponding to the new var
      void calcAugmentedCoeff( RealVector const& c, RealVector& r, double& s ) const;

      /// Calculate coefficients if the model must remove one monomial
      /// @param [in] nVars   number of monomials
      /// @param [in] pos     position of the monomial that must be removed
      /// @returns the coefficients that correspond to the reduced model
      RealVector calcReducedCoeff( unsigned int nVars, unsigned int pos ) const;

      /// The size of a parameter (number of parameter elements)
      unsigned int            m_size;

      /// The matrix of parameters
      ParameterSet            m_parSet;

      /// The vector of targets
      TargetSet               m_scaledTargets;

      /// This list of active variables for which the SVD is calculated
      VarList                 m_vars;

      /// The coding of parameter monomials (see CubicProxy::monomial_code)
      std::vector<IndexList>  m_code;

      /// Statistical mean of parameter set
      RealVector              m_proxyMean;

      /// Statistical mean of target set
      double                  m_targetMean;

      /// Model matrix containing monomial values of the cubic proxy
      ProxyData               m_proxyData;

      /// SVD applied to m_proxyData yields the following matrices:
      /// Diagonal matrix containing singular values, and orthonormal matrices U and V
      RealMatrix              m_orthonormalU;
      RealVector              m_singularValues;
      RealMatrix              m_orthonormalV;

      /// Proxy coefficients for the base variables
      RealVector              m_coefficients;
};

} // namespace SUMlib

#endif // SUMLIB_PROXYBUILDER_H
