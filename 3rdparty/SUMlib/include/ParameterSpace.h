// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARAMETERSETINFO_H
#define SUMLIB_PARAMETERSETINFO_H

#include <vector>

#include "BaseTypes.h"
#include "Case.h"
#include "ParameterBounds.h"
#include "SUMlib.h"

#include "ISerializer.h"

namespace SUMlib {

/// @class ParameterSpace represents the properties of the parameter space.
///
/// It supplies information on the cases and the parameter bounds.
/// It also provides functions to prepare and unprepare cases.
///
/// Preparing an unprepared case consists of the following modifications:
/// 1. Start with an unprepared (raw) case c.
/// 2. Transform the continuous parameter values (optionally).
/// 3. Convert the discrete parameters to continuous parameters.
/// 4. Scale the continuous parameter values to [-1:1].
/// 5. Remove the values of parameters that cannot vary (low = high).
/// 6. Convert the non-default categorical values to binary dummy parameters.
/// 7. Convert the results to a vector v of doubles.
/// Usage: prepare( in:c, out:v )
///
/// Unpreparing v is the reverse operation of preparing c, such that
/// unprepare( in:v, out:c ) retrieves the original raw c regardless what may
/// have happened with c between the prepare and unprepare calls.
///
/// Generated copy constructor and copy assignment are OK.
class INTERFACE_SUMLIB ParameterSpace : public ISerializable
{
   public:

      /// @enum Defines a parameter transformation to apply before scaling
      enum TransformType {
         NoTransform,
         Log10Transform,
         Pwr10Transform,
         SqrtTransform,
         SqrTransform
      };

      /// Transformation types for the continuous parameters
      typedef std::vector<TransformType> TransformationSet;

      /// Default constructor
      /// @param [in] origLow        original lower bounds as specified by the user
      /// @param [in] origHigh       original upper bounds as specified by the user
      /// @param [in] tr             set of continuous parameter transformation types
      ParameterSpace( Case const& origLow = Case(), Case const& origHigh = Case(),
                      TransformationSet const& tr = TransformationSet() );

      /// Destructor
      virtual ~ParameterSpace();

      /// Set the original number of dummy parameters for each categorical parameter,
      /// and set the inverse transformation types for the continuous parameters.
      /// @param [in] origLow        original lower bounds as specified by the user
      /// @param [in] origHigh       original upper bounds as specified by the user
      /// @param [in] tr             set of continuous parameter transformation types
      void initialise( Case const& origLow, Case const& origHigh, TransformationSet const& tr );

      /// Set the bounds of the parameter space
      /// @param [in] bounds         parameter bounds to use
      void setBounds( ParameterBounds const& bounds );

      /// Prepare an unprepared case
      /// @param [in]   cRaw        raw case to prepare
      /// @param [out]  v           prepared case
      void prepare( Case const& cRaw, RealVector &v ) const;

      /// Unprepare a prepared case
      /// @param [in]   v           prepared case to unprepare
      /// @param [out]  c           unprepared case
      void unprepare( RealVector const& v, Case &c ) const;

      /// Prepare all cases in the case set
      /// @param [in]   caseSet    case set to prepare
      /// @param [out]  vSet       prepared cases
      void prepare( std::vector<Case> const& caseSet, RealMatrix &vSet ) const;

      /// Unprepare all cases
      /// @param [in]   vSet       cases to unprepare
      /// @param [out]  caseSet    unprepared case set
      void unprepare( RealMatrix const& vSet, std::vector<Case> &caseSet ) const;

      /// Getter for the total number of parameters
      /// @returns the total number of parameters
      unsigned int size() const { return m_bounds.size(); }

      /// Getter for the total number of continuous parameters
      /// @returns the total number of continuous parameters
      unsigned int sizeCon() const { return m_bounds.sizeCon(); }

      /// Getter for the total number of discrete parameters
      /// @returns the total number of discrete parameters
      unsigned int sizeDis() const { return m_bounds.sizeDis(); }

      /// Getter for the total number of ordinal parameters
      /// @returns the total number of ordinal parameters
      unsigned int sizeOrd() const { return m_bounds.sizeOrd(); }

      /// Getter for the total number of categorical parameters
      /// @returns the total number of categorical parameters
      unsigned int sizeCat() const { return m_bounds.sizeCat(); }

      /// Getter for the lower parameter bounds
      /// @returns the lower bounds
      const Case& low() const { return m_bounds.low(); }

      /// Getter for the upper parameter bounds
      /// @returns the upper bounds
      const Case& high() const { return m_bounds.high(); }

      /// Getter for the transformed lower parameter bounds
      /// @returns the transformed lower bounds
      const Case& transformedLow() const { return m_tr_bounds.low(); }

      /// Getter for the transformed upper parameter bounds
      /// @returns the transformed upper bounds
      const Case& transformedHigh() const { return m_tr_bounds.high(); }

      /// Getter for the available values of each categorical parameter
      /// @returns the available values of each categorical parameter
      const std::vector<IndexList>& catValues() const { return m_bounds.catValues(); }

      /// Getter for the available values of a specific categorical parameter
      /// @param [in]      idx       categorical parameter index
      /// @returns the available values of the corresponding categorical parameter
      const IndexList& catValues( unsigned int idx ) const { return m_bounds.catValues( idx ); }

      /// Getter for the indices of non-fixed parameters
      /// @returns the indices of non-fixed parameters
      const IndexList& nonFixedParIdx() const { return m_preparedParIdx; }

      /// Getter for the transformation set
      /// @returns the transformation set
      const TransformationSet& transformationSet() const { return m_tr; }

      /// Returns whether the parameter with specified index has equal bounds (fixed)
      /// @param [in]      idx       parameter index
      /// @returns whether the specific parameter has equal bounds (fixed)
      bool isFixed( unsigned int idx ) const { return m_bounds.areEqual( idx ); }

      /// Apply parameter transformations
      /// @param [in]      tr             transformations to apply
      /// @param [in,out]  conPars   continuous parameters to transform
      static void transform( TransformationSet const& tr, std::vector<double> &conPars );

      /// Scale the continuous parameter values of a case to [-1:1]
      /// @param [in]      bounds    parameter bounds
      /// @param [in,out]  c         case to scale
      static void scale( ParameterBounds const& bounds, Case &c );

      /// Prepare a partition: remove the entries of the partition that correspond to
      /// parameters with equal bounds (low = high).
      ///
      /// @param [in,out] partition
      void prepare( Partition& ) const;

      /// Unprepare an index list by modifying the indexes to account for parameters with
      /// equal bounds (low = high): if parameter k has equal bounds in the unprepared
      /// case set, all indexes k and higher are incremented.
      ///
      /// @param [in,out] index list
      void unprepare( IndexList& ) const;

      /// Prepare an index list: modify the indexes, removing those that indicate
      /// parameters with equal bounds (low = high), and modifying the others to indicate
      /// the corresponding prepared parameter.
      ///
      /// @param [in,out] index list
      void prepare( IndexList& ) const;

      /// Convert to universal indexing of proxy coefficients (including dummy pars)
      /// @param [in,out] index list
      void convert2origProxyIdx( IndexList& ) const;

      /// Getter for the number of "non-fixed" ordinal parameters
      /// @returns the number of "non-fixed" ordinal parameters
      unsigned int nbOfNonFixedOrdinalPars() const;

      /// Getter for the number of "non-fixed" continuous parameters
      /// @returns the number of "non-fixed" continuous parameters
      unsigned int nbOfNonFixedContinuousPars() const;

      /// Getter for the categorical parameter values from a prepared case
      /// @param [in] v    a prepared case
      /// @returns the categorical parameter values converted to doubles
      RealVector getCatParValuesFromPreparedCase( RealVector const& v ) const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;

   private: // methods

      /// Remove the values corresponding to parameters with equal bounds (low = high)
      /// @param [in,out] c          case to remove "fixed" parameter values from
      void removeFixed( Case &c ) const;

      /// Add the values corresponding to parameters with equal bounds (low = high)
      /// @param [in,out] c          case to add "fixed" parameter values to
      void addFixed( Case &c ) const;

      /// Add binary values for the dummy parameters to the end of a given vector v
      /// @param [in]     c          case to extract categorical values from
      /// @param [in,out] v          vector to be extended with binary values
      void addBinaryValues( Case const& c, RealVector &v ) const;

      /// Convert binary dummy parameters to original categorical parameter values
      /// @param [in]     v          vector to extract binary values from
      /// @returns the categorical parameter values
      std::vector<unsigned int> getCatParValues( RealVector const& v ) const;

      /// List indices of prepared parameters
      void listPreparedParIdx();

      /// Getter for the number of binary dummy parameters
      /// @returns the number of binary dummy parameters
      unsigned int nbOfDummyPars() const;

   private: // data

      /// transformations
      TransformationSet m_tr;

      /// inverse transformations
      TransformationSet m_tr_inv;

      /// low/high values before transformation
      ParameterBounds   m_bounds;

      /// low/high values after transformation
      ParameterBounds   m_tr_bounds;

      /// list of indices of prepared (i.e. non-fixed) parameters
      IndexList m_preparedParIdx;

      /// list of indices of prepared (i.e. non-fixed) parameters, including dummy parameters
      IndexList m_preparedProxyParIdx;

      /// original number of dummy parameters for each categorical parameter
      std::vector<unsigned int> m_nbOfOrigDummyPars;

};

} // namespace SUMlib

#endif // SUMLIB_PARAMETERSETINFO_H
