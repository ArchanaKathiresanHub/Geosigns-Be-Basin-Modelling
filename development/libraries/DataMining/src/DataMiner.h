#ifndef _MINING__DATA_MINER_H_
#define _MINING__DATA_MINER_H_

#include <vector>

#include "Property.h"

#include "ElementPosition.h"
#include "DataMiningProjectHandle.h"
#include "DomainPropertyCollection.h"
#include "InterpolatedPropertyValues.h"

namespace DataAccess {

   namespace Mining {

      /// \brief Class extracting results from a Cauldron project.
      ///
      /// These results will mostly be interpolated, some properties however will be
      /// calculated, perhaps using some interpolated results.
      class DataMiner {

      public :

         typedef std::vector<const Interface::Property*> PropertySet;

         typedef std::vector<InterpolatedPropertyValues> ResultValues;



         DataMiner ( Mining::ProjectHandle& projectHandle,
                     DerivedProperties::DerivedPropertyManager& propertyManager );

         ~DataMiner ();

         /// \brief Set which properties are to be extracted.
         void setProperties ( const PropertySet&          properties );

         /// \brief Compute result for a property at a position.
         ///
         /// Result is added to result sequence.
         void compute ( const ElementPosition&            position,
                        const Interface::Property*        property,
                              InterpolatedPropertyValues& result );

         /// \brief Compute result for a set of properties at a position.
         ///
         /// Results are added to result sequence.
         void compute ( const ElementPosition&            position,
                        const PropertySet&                properties,
                              InterpolatedPropertyValues& result );

         /// \brief Compute results for a set of properties at many positions.
         ///
         /// Results are added to result sequence.
         void compute ( const ElementPositionSequence&    positions,
                        const PropertySet&                properties,
                              ResultValues&               results );

         /// \brief Compute result for a property at many position.
         ///
         /// Results are added to result sequence.
         void compute ( const ElementPositionSequence&    positions,
                        const Interface::Property*        property,
                              ResultValues&               results );

         /// \brief Compute result for all previously defined property at many position.
         ///
         /// Results are added to result sequence.
         void compute ( const ElementPositionSequence&    positions,
                              ResultValues&               results );

      private :

         DerivedProperties::DerivedPropertyManager& m_propertyManager;
         DomainPropertyCollection*                  m_collection;

      };

   }

}


#endif // _MINING__DATA_MINER_H_
