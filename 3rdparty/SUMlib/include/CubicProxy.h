// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_CUBICPROXY_H
#define SUMLIB_CUBICPROXY_H

// std
#include <map>

// SUMlib
#include "BaseTypes.h"
#include "ISerializer.h"
#include "SUMlib.h"

namespace SUMlib {

/// @class CubicProxy holds proxy model coefficients for a cubic polynomial model.
/// The class assumes parameters to be scaled to the [-1:1] domain.
class CubicProxy : public ISerializable, public ISerializationVersion
{
   public:

      /// CubicProxy monomials and their coefficients and standard errors
      typedef std::map< MonomialKey, std::pair< double, double > > CoefficientsMap;

      /// Calculate coefficients for a proxy model based on SVD data (a, w, and v) and target values
      /// @param [in] stat    integer status of SVD (0 = success)
      /// @param [in] a       m x n orthonormal matrix
      /// @param [in] w       n x 1 vector of singular values
      /// @param [in] v       n x n orthonormal matrix
      /// @param [in] b       m x 1 vector of target values
      /// @param [out] coef   n x 1 vector of coefficients corresponding to the model monomials
      static void calculateCoefficients( int stat, std::vector<std::vector<double> > const& a, std::vector<double> const& w, std::vector<std::vector<double> > const& v, std::vector<double> const& b, std::vector<double> & coef );

      /// Calculates the index list corresponding to the initial
      /// monomials (none, linear, or up to second order) contained in the proxy
      /// @param [in]  nPars     the size of the case vector
      /// @param [in]  order     order of the initial polynomial proxy (0, 1, or 2)
      /// @param [in]  partition identifies parameters to take into account
      /// @returns the index list
      static IndexList initialVarList( unsigned int nPars, unsigned int order,
                                       Partition const& partition );

      /// Calculates a special index list corresponding to the initial monomials
      /// contained in the proxy if at least one categorical parameter is involved.
      /// Note that a categorical parameter is modeled by dummy parameters.
      /// As a result, a monomial of a certain order (from a user perspective) can be of
      /// higher order under the hood due to terms like "dummy par times ordinal par".
      /// @param [in]  nPars     the size of the case vector
      /// @param [in]  nOrdPars  the number of ordinal parameters
      /// @param [in]  order     order of the initial polynomial proxy (0, 1, or 2) set by the user
      /// @param [in]  partition identifies parameters to take into account
      /// @returns the index list
      static IndexList initialVarList( unsigned int nPars, unsigned int nOrdPars,
                                       unsigned int order, Partition const& partition );

      /// Calculate an array of parameter index lists that captures
      /// the monomials to be calculated for the cubic proxy.
      /// The datastructure to be filled is cleared and resized.
      /// @param [in]  nPars the number of parameters to calculate the monomials for
      /// @param [out] code  vector of monomial index vectors
      static void monomial_code( unsigned int nPars, std::vector<IndexList>& code );

      /// Calculate an array of parameter index lists that captures
      /// the monomials to be calculated for the cubic proxy
      /// The datastructure to be filled is cleared and resized
      /// @param [in]  nPars the size of a case to calculate the monomials for
      /// @param [in]  vars  index list of active variables
      /// @param [out] code  vector of monomial index vectors for the active variables
      static void monomial_code( unsigned int nPars, IndexList const& vars, std::vector<IndexList>& code );

      /// Calculate a single monomial indentified by an index list of parameters
      /// to be multiplied
      /// @param [in] index  list of index numbers to the parameter identifying
      ///                    elements to be multiplied
      /// @param [in] c      the case to calculate the monomial for
      /// @returns the monomial value
      static double monomial( MonomialKey const& index, Parameter const& par );

      /// Calculate all cubic monomials for a parameter P
      /// The result vector is cleared and resized
      /// @param [in]  c case to calculate the monomials for
      /// @param [out] r result vector of monomials
      static void monomials( Parameter const& p, RealVector & r );

      /// Calculate the number of monomial variables for a given number
      /// of parameter entries.
      /// @param [in] numPar   the number of entries in a Parameter
      /// @param [in] maxOrder the maximum order of monomials to consider
      /// @returns the number of variables
      static unsigned int numVars( unsigned int numPar, unsigned int maxOrder = 3 );

      /// Calculates whether term (par j, par k) is a valid term if any categorical/dummy
      /// parameter(s) are present. The number of ordinal parameters, nOrds, is also passed.
      /// The polynomial order set by the user is passed because it may be different from the
      /// assumed technical order being equal to 2. For example: dummy_1 * dummy_2 is,
      /// technically, of order 2 but from a user perspective it is of order 0.
      /// k and j are parameter indices with k >= j
      static bool validOrder2Var( unsigned int order, unsigned int nOrds,
                                      unsigned int k, unsigned int j );

      /// Calculates whether term (par j, par k, par l) is a valid term if any categorical/dummy
      /// parameter(s) are present. The number of ordinal parameters, nOrds, is also passed.
      /// The polynomial order set by the user is passed because it may be different from the
      /// assumed technical order being equal to 3. For example: dummy * (p_ord)^2 is,
      /// technically, of order 3 but from a user perspective it is of order 2. So, this term
      /// is a valid order 3 term if the user order is 2 but not if the user order is 1.
      /// l, k, and j are parameter indices with l >= k >= j
      static bool validOrder3Var( unsigned int order, unsigned int nOrds,
                                      unsigned int l, unsigned int k, unsigned int j );

      /// Default constructor
      CubicProxy();

      /// Destructor
      virtual ~CubicProxy();

      /// Constructor for a cubic proxy from a case set
      /// All cases in the set are used
      /// @param [in] caseSet      the cases
      /// @param [in] targetSet    simulator response values for each of the parameters
      /// @param [in] vars         index list of active variables
      CubicProxy(
            ParameterSet const&  parameterSet,
            TargetSet const&     targetSet,
            IndexList const&     vars
            );

      /// Constructor for a cubic proxy from a ProxyBuilder
      /// @param [in] size         number of parameter elements
      /// @param [in] vars         list of active variables
      /// @param [in] code         monomial code of each active variables
      /// @param [in] proxyMean    statistical mean of proxy data
      /// @param [in] targetMean   statistical mean of target values
      /// @param [in] coefficients proxy coefficients
      CubicProxy(
            unsigned int                  size,
            VarList const&                vars,
            std::vector<IndexList> const& code,
            RealVector const&             proxyMean,
            double                        targetMean,
            RealVector const&             coefficients
            );

      /// Initialise all internal data structures from the full proxy data matrix and target set
      /// @param [in] allProxyData a matrix of all cubic monomials of the case set
      /// @param [in] allTargets   a vector of target values for all cases in the case set
      /// @param [in] vars         identifies which variables (i.e. monomials) are active in this proxy
      void initialise(
            ProxyData const&     allProxyData,
            TargetSet const&     allTargets,
            IndexList const&     vars
            );

      /// Initialise all internal data structures
      /// @param [in] size         number of parameters
      /// @param [in] vars         list of active variables
      /// @param [in] code         monomial code of each active variables
      /// @param [in] proxyMean    statistical mean of proxy data
      /// @param [in] targetMean   statistical mean of target values
      /// @param [in] coefficients proxy coefficients
      void initialise(
            unsigned int                  size,
            VarList const&                vars,
            std::vector<IndexList> const& code,
            RealVector const&             proxyMean,
            double                        targetMean,
            RealVector const&             coefficients
            );

      /// Initialise all internal data structures
      /// @param [in] size         number of parameters
      /// @param [in] vars         list of active variables
      /// @param [in] code         monomial code of each active variables
      /// @param [in] proxyMean    statistical mean of proxy data
      /// @param [in] targetMean   statistical mean of target values
      /// @param [in] coefficients proxy coefficients
      void initialise(
            unsigned int                  size,
            VarList                       vars,
            std::vector<IndexList>        code,
            RealVector                    proxyMean,
            double                        targetMean,
            RealVector                    coefficients,
            RealVector                    stdErrors,
            unsigned int                  designMatrixRank
            );

      /// Remove the terms that contain a constant-transformed parameter.
      /// The proxy polynomial can include terms that contain a constant-transformed parameter. That is undesirable
      /// because those terms do not affect the proxy evaluations but they do increase the maximum rank in
      /// CubicProxy::isRegressionIllPosed() causing the regression to be unjustly considered ill-posed. All terms that
      /// include a constant-transformed parameter must therefore be removed from the proxy polynomial. The CubicProxy
      /// doesn't know about transforms. The parameter transforms are applied by the CompoundProxy, and only the
      /// resulting values are propagated without the information about the transforms themselves. We therefore request
      /// the indices of the constant-transformed parameters in CompoundProxy and pass those on to the CubicProxy.
      /// Preventing the constant-transformed parameters from being included in the proxy in the first place is not
      /// achievable, because its size (see Proxy::size()) and therefore its proxy calls (see e.g.
      /// Proxy::getProxyValue()) currently include those parameters. So it is not just an internal affair, but also
      /// affects code that uses the proxy.
      /// @param [in] the parameter indices that have a constant transform
      void removeConstTransformedParameters( const IndexList& indices );

      /// The size of the cases of this proxy model
      /// Implements the Proxy abstract base class
      /// @returns the size (number of elements of the cases)
      unsigned int size() const;

      /// Get a map of monomials and their cubic proxy coefficient and standard error.
      /// @param [out] cubic proxy monomials and coefficients map
      void getCoefficientsMap( CoefficientsMap& map ) const;

      /// Calculate the value for given (prepared) case
      /// Implements the Proxy abstract base class
      /// @param [in] p             case
      /// @returns the proxy response value
      double getValue( Parameter const& p ) const;

      /// Get the active variables
      /// @param [out] p_varList a copy of the internal active variables list
      void getVarList( IndexList & p_varList ) const;

      /// Get the active variables
      /// @returns the list of active variables
      IndexList const& variables() const;

      /// Set the standard errors of the proxy coefficients
      /// @param [in] stdErrors standard errors
      void setStdErrors( RealVector const& stdErrors );

      /// Sets the rank of the design matrix.
      void setDesignMatrixRank( unsigned int rank );

      /// Returns the rank of the design matrix.
      unsigned int getDesignMatrixRank() const;

      /// Returns true iff the regression is ill-posed.
      bool isRegressionIllPosed() const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;
      virtual unsigned int getSerializationVersion() const;

   private: // methods

      /// Get the intercept value
      double getIntercept() const;

   private: // data

      /// the size of parameter vectors
      unsigned int            m_size;

      /// The active variables list stored as a vector of indices
      IndexList               m_vars;

      /// The coding of parameter monomials (see monomial_code for documentation)
      std::vector<IndexList>  m_code;

      /// mean of m_proxyData for statistical scaling
      RealVector              m_proxyMean;

      /// mean of targetSet for statistical scaling
      double                  m_targetMean;

      /// The proxy coefficients
      RealVector              m_coefficients;

      /// The standard errors of the proxy coefficients
      RealVector              m_stdErrors;

      /// The rank of the design matrix
      unsigned int            m_designMatrixRank;
};

// Inlined to increase performance.
inline double CubicProxy::monomial( MonomialKey const& parindex, Parameter const& par )
{
   double m = 1.0;
   for ( MonomialKey::const_iterator it=parindex.begin(); it != parindex.end(); ++it )
   {
      m *= par[*it];
   }
   return m;
}

}
#endif // SUMLIB_CUBICPROXY_H
