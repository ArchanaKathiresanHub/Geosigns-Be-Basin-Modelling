#ifndef _DATA_ACCESS__LANGMUIR_ADSORPTION_TOC_ENTRY_H_
#define _DATA_ACCESS__LANGMUIR_ADSORPTION_TOC_ENTRY_H_

#include "DAObject.h"
#include "Interface.h"



namespace DataAccess {

   namespace Interface {

      /// \brief Class containing the contents a single line of the Langmuir adsorption isotherm table.
      class LangmuirAdsorptionTOCEntry : public DAObject {

      public :

         LangmuirAdsorptionTOCEntry ( ProjectHandle*    projectHandle,
                                      database::Record* record );

         /// \brief Destructor.
         virtual ~LangmuirAdsorptionTOCEntry ();

         /// \brief Return the temperature, C.
         virtual double getReferenceTemperature () const;

         /// \brief Return the temperature gradient.
         virtual double getTemperatureGradient () const;

         /// \brief Coefficient for the .
         virtual double getCoeffA () const;

         /// \brief Coefficient for the .
         virtual double getCoeffB () const;

         /// \brief Return the pressure, MPa.
         virtual double getLangmuirPressure () const;

         /// \brief Get the name of the formation to which the function is to be applied.
         virtual const std::string& getLangmuirName () const;

         /// \brief Return a string representation of the object.
         virtual std::string image () const;

      private :

         double m_referenceTemperature;
         double m_temperatureGradient;
         double m_coeffA;
         double m_coeffB;
         double m_langmuirPressure;
         std::string m_langmuirName;

      };

   }

}

#endif // _DATA_ACCESS__LANGMUIR_ADSORPTION_TOC_ENTRY_H_
