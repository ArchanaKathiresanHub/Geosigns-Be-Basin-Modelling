// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARAMETERPDF_H
#define SUMLIB_PARAMETERPDF_H

#include <map>
#include <vector>

#include "BaseTypes.h"
#include "ParameterBounds.h"
#include "ParameterSpace.h"
#include "Case.h"
#include "SUMlib.h"
#include "ISerializer.h"

namespace SUMlib {

class ParameterSpace;

/// @class ParameterPdf provides the basics of a mixed parameter PDF.
///
/// It automatically removes "fixed" parameters (low = high).
/// The PDF can also scale itself to the [-1:1] domain.
///
/// Generated copy constructor and copy assignment are OK
class INTERFACE_SUMLIB ParameterPdf : public ISerializable
{
public:
   /// Default constructor.
   ParameterPdf();

   /// Constructor. PDF properties, if not provided, are calculated from parameter space.
   /// @param [in] space       parameter space providing bounds and categorical info
   /// @param [in] minLow      lowest possible lower bounds (originally set by the user)
   /// @param [in] maxHigh     highest possible upper bounds (originally set by the user)
   /// @param [in] base        base case
   /// @param [in] covariance  matrix with covariances for the continuous parameters
   /// @param [in] disWeights  weights for all possible values of each discrete parameter
   /// @param [in] catWeights  weights for all possible values of each categorical parameter
   ParameterPdf(
         ParameterSpace const& space,
         Case const&           minLow,
         Case const&           maxHigh,
         Case const&           base,
         RealMatrix const&     covariance,
         RealMatrix const&     disWeights,
         RealMatrix const&     catWeights );

   /// Destructor
   virtual ~ParameterPdf();

   /// Initialisation
   /// @param [in] space       parameter space providing bounds and categorical info
   /// @param [in] minLow      lowest possible lower bounds (originally set by the user)
   /// @param [in] maxHigh     highest possible upper bounds (originally set by the user)
   /// @param [in] base        base case
   /// @param [in] covariance  matrix with covariances for the continuous parameters
   /// @param [in] disWeights  weights for all possible values of each discrete parameter
   /// @param [in] catWeights  weights for all possible values of each categorical parameter
   void initialise(
         ParameterSpace const& space,
         Case const&           minLow,
         Case const&           maxHigh,
         Case const&           base,
         RealMatrix const&     covariance,
         RealMatrix const&     disWeights,
         RealMatrix const&     catWeights );

   /// Getter for the number of parameters
   /// @returns the number of parameters (actually the size of m_low)
   unsigned int size() const { return m_low.size(); }

   /// Getter for the number of continuous parameters
   /// @returns the number of continuous parameters
   unsigned int sizeCon() const { return m_low.sizeCon(); }

   /// Getter for the number of discrete parameters
   /// @returns the number of discrete parameters
   unsigned int sizeDis() const { return m_low.sizeDis(); }

   /// Get number of ordinal parameters (i.e. combined continuous and discrete parameters)
   /// @returns the number of ordinal parameters
   unsigned int sizeOrd() const { return m_low.sizeOrd(); }

   /// Getter for the number of categorical parameters
   /// @returns the number of categorical parameters
   unsigned int sizeCat() const { return m_low.sizeCat(); }

   /// Getter for the lower bounds
   /// @returns the lower bounds
   const Case& low() const { return m_low; }

   /// Getter for the upper bounds
   /// @returns the upper bounds
   const Case& high() const { return m_high; }

   /// Getter for the base values (doubles) of the discrete parameters
   /// @returns the base values of the discrete parameters
   const std::vector<double>& discreteBase() const { return m_discreteBase; }

   /// Getter for the most likely continuous parameter values
   /// @returns the most likely continuous parameter values
   const std::vector<double>& mostLikely() const { return m_mostLikely; }

   /// Getter for the scaled base values of the ordinal parameters
   /// @returns the scaled base values of the ordinal parameters
   const std::vector<double>& scaledOrdinalBase() const { return m_scaledOrdinalBase; }

   /// Getter for the base values of the categorical parameters
   /// @returns the base values of the categorical parameters
   const std::vector<unsigned int>& categoricalBase() const { return m_categoricalBase; }

   /// Getter for the covariance
   /// @returns the covariance
   const RealMatrix& covariance() const { return m_covariance; }

   /// Getter for the minimum standard deviation values
   /// @returns the minimum standard deviation values
   const std::vector<double>& minStddev() const { return m_minStddev; }

   /// Get the weights for the values of the non-fixed discrete parameters
   /// @returns the weights for the values of the discrete parameters
   const RealMatrix& disWeights() const { return m_disWeights; }

   /// Get the weights for the values of the non-fixed categorical parameters
   /// @returns the weights for the values of the categorical parameters
   const RealMatrix& catWeights() const { return m_catWeights; }

   /// Available values of each non-fixed categorical parameter
   const std::vector<IndexList>& catValues() const { return m_catValues; }

   /// Getter for the lowest bounds of the non-frozen ordinal parameters
   std::vector<double> lowestNonFrozenOrdParams() const;
   
   /// Getter for the highest bounds of the non-frozen ordinal parameters
   std::vector<double> highestNonFrozenOrdParams() const;

   /// Getter for the upper bounds of the total parameter space
   const Case& maxParValues() const { return m_maxHigh; }

   /// Getter for the indices of the non-frozen parameters
   const IndexList& varParIdx() const { return m_varParIdx; }

   /// Scale ordinal part of PDF to [-1:1] domain as input for MC kind of algorithms.
   /// An instance of ParameterPDF is properly scaled after the first call. A
   /// subsequent call will not have any effect! So set the standard deviations
   /// first (if needed) before scaling the instance.
   void scale();

   /// Construct a proxy case including binary values for the dummy parameters.
   /// @param [in]      c     categorical part of a case
   /// @param [in,out]  v     in: ordinal part of scaled case, out: proxy case
   void extendToProxyCase( IndexList const& c, std::vector<double> &v ) const;

   /// Set the standard deviation for a specific continuous parameter.
   /// This setter is preferably applied before calling scale(), otherwise the
   /// standard deviation value (2nd argument) must be scaled by the caller!
   /// @param [in] idx     continuous parameter index
   /// @param [in] stddev  the standard deviation value
   void setStdDev( unsigned int idx, double stddev );

   /// Set the weights for a specific discrete parameter.
   /// If applied, this setter must be used before calling scale()!
   /// @param [in] idx     discrete parameter index
   /// @param [in] w       the weights
   void setDisWeights( unsigned int idx, std::vector<double> const& w );

   /// Set the weights for a specific categorical parameter.
   /// @param [in] idx     categorical parameter index
   /// @param [in] w       the weights
   void setCatWeights( unsigned int idx, std::vector<double> const& w );

   /// Calculate defaults for mean and stdev from the bounds
   /// @param [in] min     minimum values
   /// @param [in] max     maximum values
   /// @param [out] mean   (max + min) / 2
   /// @param [out] stdev  (max - min) / 2
   static void calcDefaultMeanAndStdDev(
         ParameterData const&    min,
         ParameterData const&    max,
         ParameterData           &mean,
         ParameterData           &stdev );

   /// Check whether the supplied matrix is a proper covariance matrix.
   /// An exception is thrown if invalid elements are found.
   /// @param [in] cov     covariance matrix (square and symmetric)
   static void checkCovarianceMatrix( RealMatrix const& cov );

   /// Getter for a specific discrete base value (converted to a double)
   /// @param [in] i       discrete parameter index
   double discreteBase( unsigned int i ) const { return m_discreteBase[i]; }

   /// Getter for a specific categorical base value
   /// @param [in] i       categorical parameter index
   unsigned int categoricalBase( unsigned int i ) const { return m_categoricalBase[i]; }

   /// Some specific getters
   /// @param [in] i       continuous parameter index
   double minStddev( unsigned int i ) const { return m_minStddev[i]; }
   double mostLikely( unsigned int i ) const { return m_mostLikely[i]; }
   double variance( unsigned int i ) const { return m_covariance[i][i]; }

private: //methods
   /// Check bounds
   void checkBounds() const;

   /// Set weights, most likely and/or covariance at default values if they are unknown
   void setDefaultsIfUnknownPDF();

   /// Check dimensions
   void checkDimensions() const;

   /// remove weights for categorical values that are not used
   void removeRedundantCatWeights();

   /// Remove fixed elements
   void removeFixedElements();

   /// Check whether weights are positive
   void checkWeights() const;

   /// Check whether weights for a specific discrete parameter are positive
   /// @param [in] i_p     parameter index
   void checkDisWeights( unsigned int i_p ) const;

   /// Check whether weights for a specific categorical parameter are positive
   /// @param [in] i_p     parameter index
   void checkCatWeights( unsigned int i_p ) const;

   /// Check whether most likely values are within bounds, and
   /// check whether discrete base values are within bounds
   void checkOrdinalBase();

   /// Check whether categorical base values exist in m_catValues
   void checkCategoricalBase();

   /// Calculate the minimum stddev and call checkStddev()
   void calcMinStddev( );

   /// Check whether stddev values are bigger than minimum; if not, set stddev to minimum
   void checkStddev();

   /// Scale continuous part of the PDF to new [p_min:p_max] domain
   /// @param [in] p_min   new minimum values for the continuous parameters
   /// @param [in] p_max   new maximum values for the continuous parameters
   void scaleCon( std::vector<double> const& p_min, std::vector<double> const& p_max );

   /// Scale discrete part of the PDF to new [p_min:p_max] domain
   /// @param [in] p_min   new minimum values for the discrete parameters
   /// @param [in] p_max   new maximum values for the discrete parameters
   void scaleDis( std::vector<int> const& p_min, std::vector<int> const& p_max );

   // ISerializable interface
   virtual bool load( IDeserializer*, unsigned int );
   virtual bool save( ISerializer*, unsigned int ) const;

private: //data

   /// Lower bounds of the transformed prepared space
   Case m_low;

   /// Upper bounds of the transformed prepared space
   Case m_high;

   /// Lowest possible transformed lower bounds of the total space
   Case m_minLow;

   /// Highest possible transformed upper bounds of the total space
   Case m_maxHigh;

   /// Available values of each non-fixed categorical parameter
   std::vector<IndexList> m_catValues;

   /// Indices of non-fixed parameters, also subdivided according to type
   IndexList m_varParIdx, m_conVars, m_disVars, m_catVars;

   /// The base values of the discrete parameters, converted to doubles
   std::vector<double> m_discreteBase;

   /// The most likely continuous parameter values
   std::vector<double> m_mostLikely;

   /// The base values of the ordinal parameters
   std::vector<double> m_scaledOrdinalBase;

   /// The base values of the categorical parameters
   std::vector<unsigned int> m_categoricalBase;

   /// The covariance matrix of the continuous parameters
   RealMatrix m_covariance;

   /// Lower bounds of the stddev
   std::vector<double> m_minStddev;

   /// Weights for the values of the non-fixed discrete parameters
   RealMatrix m_disWeights;

   /// Weights for the values of the non-fixed categorical parameters
   RealMatrix m_catWeights;
};

} // namespace SUMlib

#endif // SUMLIB_PARAMETERPDF_H
