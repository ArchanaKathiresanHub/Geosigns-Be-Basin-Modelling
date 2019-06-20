//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_VALIDATOR_H
#define INTERFACE_VALIDATOR_H

// DataModel
#include "AbstractValidator.h"
#include "Local2DArray.h"

// std
#include <vector>

namespace DataAccess {
   namespace Interface {
      class ProjectHandle;
      class GridMap;
      class Formation;
      class MantleFormation;
      class CrustFormation;
      class PropertyValue;
      
      /// @class Validator class which defines if one node is valid or not according to DataAccess project handle
      class Validator : public DataModel::AbstractValidator  {

         typedef Local2DArray <bool> BooleanLocal2DArray;
         typedef std::vector<Formation *> FormationList;

         public:

            explicit Validator( ProjectHandle& projectHandle):
               m_projectHandle( projectHandle ){};
            ~Validator() final = default;

            /// Initialise the valid-node array.
            ///
            /// This function uses only the input data maps and is sufficient to construct
            /// all of the necessary items in the cauldron model, e.g. sea-surface temperature,
            /// crust-thickness history, etc. It may be necessary to restrict further the
            /// valid nodes with other input data, e.g. fct-correction maps, or ves property.
            virtual bool initialiseValidNodes( const bool readSizeFromVolumeData );

            /// Add undefined areas of a grid-map to the undefined-node array.
            void addUndefinedAreas( const GridMap* theMap );

            /// @return true if (i,j) is a valid node, else return false
            bool isValid( const unsigned int i, const unsigned int j ) const final;

         private:

            /// Add undefined areas of formation input maps to the undefined-node array.
            ///
            /// Input maps include:
            ///   o Lithology maps;
            ///   o Mobile layer thickness maps;
            ///   o Allochthonous lithology distribution maps;
            ///   o All reservoir maps.
            void addFormationUndefinedAreas( const Formation* formation );

            /// Add undefined areas of the mantle-formation input maps to the undefined-node array.
            ///
            /// Input maps include:
            ///   o Paleo-thickness history;
            ///   o Heat-flow history maps.
            void addMantleUndefinedAreas( const MantleFormation* mantle );

            /// Add undefined areas of the crust-formation input maps to the undefined-node array.
            ///
            /// Input maps are paleo-thickness history.
            void addCrustUndefinedAreas( const CrustFormation* crust );

            /// Valid isolated nodes will be removed.
            /// A valid node is marked as "isolated" when it does not belong to any entirely valid element,
            /// eg. a 4-points element with all valid nodes.
            void filterValidNodesByValidElements();

            ProjectHandle& m_projectHandle;

            /// @brief Indicates whether a node is valid or not.
            /// @details whether it is included in the calculation.
            BooleanLocal2DArray m_validNodes;
      };
   }
}

#endif

