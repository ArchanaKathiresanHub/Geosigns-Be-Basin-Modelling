//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file RSProxyImpl.h
/// @brief This file keeps API implementation declarations for response surface proxy

#ifndef CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H
#define CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H

// CASA
#include "RSProxy.h"

// STL
#include <memory>
#include <string>

namespace casa
{
   class VarSpace;
   class ObsSpace;

   // Class to implement handling of response surface proxy
   class RSProxyImpl : public RSProxy
   {
   public:

      // Constructor / Destructor
      RSProxyImpl( const std::string & rspName
                 , const VarSpace    & varSp
                 , const ObsSpace    & obsSp
                 , size_t              rsOrder    = 1
                 , RSKrigingType       rsKrig     = NoKriging
                 , bool                autoSearch = false
                 , double              targedR2   = 0.95
                 , double              confLevel  = 0.5
                 );

      virtual ~RSProxyImpl();

      // Calculate polynomial coefficients for the given cases set
      // caseSet list of cases which keeps simulation results with influential parameters value and observables value
      // return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode calculateRSProxy( const std::vector<const RunCase*> & caseSet );

      // Calculate values of observables for given set of parameters
      // cs case which keeps list of parameters and list of observables to be calculated
      // return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode evaluateRSProxy( RunCase & cs );

      // Get type of kriging interpolation for this proxy
      // return which kriging interpolation is used for the proxy
      virtual RSKrigingType kriging() const { return m_kriging; }

      // Get order of polynomial approximation for the proxy
      // return polynomial order
      virtual int polynomialOrder() const;

      /// @brief Get the coefficient maps for all proxies.
      /// @returns a CoefficientsMapList representing the cubic proxy expressions for all observables.
      ///
      /// SUMlib::CompoundProxyCollection returns a data structure of type
      /// std::vector< std::map< std::vector<unsigned int >, double >  > which contains for each
      /// proxy a map with key-value pairs where the keys are lists of influential parameter indexes,
      /// and the values are the coefficients. The lists of influential parameter indexes
      /// indicate the monomial to which the coefficient is associated. The
      /// polynomial expression for the proxy is the sum of all monomials multiplied
      /// by their coefficients.
      ///
      /// For example, suppose the expression (influential parameters named @f$ p_1, p_2, p_3 @f$) for the @f$ k-th @f$
      /// proxy is:
      ///
      ///      @f$ f_k(p_1,p_2,p_3) = 2e4 -5.1e3 \cdot p_1 + 2.3e2 \cdot p_2 \cdot p3 - 1.2e1 \cdot p1 \cdot p1 \cdot p3 @f$
      ///
      /// then coefficients[k] will represent it by the following map
      ///      { }       ->  2e4      # zero order term
      ///      { 0 }     -> -5.1e3    # first order term (linear) p_1
      ///      { 1,2 }   ->  2.3e2    # second order term (quadratic) p_2 \cdot p_3
      ///      { 0,0,2 } -> -1.2e1    # third order term (cubic) p_1 \cdot p_1 \cdot p_3
      ///
      /// Note that the polynomial is defined for influential parameter values scaled to [-1:1],
      /// and that the influential parameters are indexed in the order in which they are present in
      /// the casa::VarSpace (including multi-dimensional parameters) defining the proxies.
      ///
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      virtual const CoefficientsMapList & getCoefficientsMapList() const;

      /// @brief Get SUMlib proxy object
      /// @return SUMlib proxy object
      virtual SUMlib::CompoundProxyCollection * getProxyCollection() const { return m_collection.get(); }

      // Serialization / Deserialization
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 1; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "RSProxyImpl"; }

      /// @brief Create a new RSProxyImpl instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      RSProxyImpl( CasaDeserializer & inStream, const char * objName );
      /// @}

   protected:
      std::string      m_name;     // proxy name

      const VarSpace * m_varSpace; // set of influential parameters
      const ObsSpace * m_obsSpace; // set of observables definitions

      size_t        m_rsOrder;    // order of the response surface polynomial approximation
      RSKrigingType m_kriging;    // type of kriging interpolation
      bool          m_autosearch; // shall we ignore given order and try to search for order ourself?
      double        m_confLevel;  // @TODO document

      CoefficientsMapList m_coefficients; // set of polynomial appoximation coefficients

      std::unique_ptr<SUMlib::CompoundProxyCollection> m_collection; // SUMlib response proxy

      // compare cases and remove duplicated
      void removeDuplicated( const std::vector<const RunCase*> & caseSet, std::vector< const RunCase*> & filteredCaseSet );

   private:
      // disable copy constructor and copy operator
      RSProxyImpl( const RSProxyImpl & );
      RSProxyImpl & operator = ( const RSProxyImpl & );
   };
}


#endif // CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H
