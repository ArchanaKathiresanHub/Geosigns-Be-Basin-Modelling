// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_TORNADOSENSITIVITIES_H
#define SUMLIB_TORNADOSENSITIVITIES_H

#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"


namespace SUMlib {

class ParameterPdf;
class Proxy;


/// Tornado sensitivities of the screening parameters for a single observable are calculated.
/// The response of the selected observable is represented by the associated proxy model.
///
/// An ordinal parameter can be at 2 extremes and thus provides 2 scalar sensitivities.
///
/// A categorical parameter that can take n different values (i.e. n categorical states),
/// corresponds to n scalar sensitivities (relative to the base case).
/// One of the categorical states, say the second one, is equal to the base value.
/// As a result, the second scalar sensitivity will be zero by definition.

/// IMPORTANT: This class handles all (frozen and non-frozen) parameters in a consistent order:
/// first the continuous parameters (ordinal), then the discrete parameters (also ordinal),
/// and finally the categorical states (first for categorical parameter 1, then for 2, etc.).
/// The corresponding sensitivities assume the same order; 2 sensitivities for each ordinal
/// parameter, and 1 sensitivity for each categorical state.
///
/// Obviously, sensitivies are zero if they correspond to a frozen parameter.
class INTERFACE_SUMLIB TornadoSensitivities
{
public:

   /// Constructor
   TornadoSensitivities();

   /// Destructor
   virtual ~TornadoSensitivities();

   /// Get proxy-based sensitivities
   /// @param in  proxy          proxy corresponding to the selected observable
   /// @param in  priorPar       scaled parameter pdf to retrieve parameter settings from
   /// @param out refObsValue    reference observable value associated with the base/default case
   /// @param out sensitivity    sensitivities, all expressed as observable value - refObsValue
   /// @param out relSensitivity relative sensitivities between 0 and 100 %
   void getSensitivities( const Proxy* proxy, const ParameterPdf& priorPar,
        double& refObsValue, std::vector<std::vector<double> >& sensitivity,
        std::vector<std::vector<double> >& relSensitivity ) const;

};


} // namespace SUMlib

#endif // SUMLIB_TORNADOSENSITIVITIES_H
