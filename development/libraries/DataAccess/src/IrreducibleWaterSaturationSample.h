#ifndef _DATA_ACCESS__IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_
#define _DATA_ACCESS__IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_

#include "DAObject.h"
#include "Interface.h"


namespace DataAccess {

   namespace Interface {

      class IrreducibleWaterSaturationSample : public DAObject {

      public :

         /// \brief Constructor.
         IrreducibleWaterSaturationSample ( ProjectHandle& projectHandle,
                                            database::Record* record );

         IrreducibleWaterSaturationSample ( ProjectHandle& projectHandle,
                                            const double coeffA,
                                            const double coeffB );

         /// \brief Destructor.
         virtual ~IrreducibleWaterSaturationSample ();

         /// \brief Get coefficient A.
         virtual double getCoefficientA () const;

         /// \brief Get coefficient B.
         virtual double getCoefficientB () const;

      private :

         /// \brief The coefficient A.
         double m_coeffA;

         /// \brief The coefficient B.
         double m_coeffB;

      };

   }

}

#endif // _DATA_ACCESS__IRREDUCIBLE_WATER_SATURATION_SAMPLE_H_
