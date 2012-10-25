#ifndef _DATA_ACCESS__LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_
#define _DATA_ACCESS__LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"



namespace DataAccess {

   namespace Interface {

      /// Class containing the contents a single line of the Langmuir adsorption isotherm table.
      class LangmuirAdsorptionIsothermSample : public DAObject {

      public :

         /// \brief Constructor.
         LangmuirAdsorptionIsothermSample ( ProjectHandle*    projectHandle,
                                            database::Record* record );

         /// \brief Destructor.
         virtual ~LangmuirAdsorptionIsothermSample ();

         /// \brief Return the temperature, C.
         virtual double getLangmuirTemperature () const;

         /// \brief Return the pressure, MPa.
         virtual double getLangmuirPressure () const;

         /// \brief Return the volume, cc/g.
         virtual double getLangmuirVolume () const;

         /// \brief Get the name of the isotherm set this isotherm belongs to
         virtual const std::string& getLangmuirName () const;

         /// \brief Return a string representation of the object.
         virtual std::string image () const;


      private :

         double m_langmuirTemperature;
         double m_langmuirPressure;
         double m_langmuirVolume;
         std::string m_langmuirName;

      };


      /// \brief Functor class for ordering langmuir-adsorption-isotherms.
      class LangmuirAdsorptionIsothermSampleLessThan {

      public :

         bool operator ()( const LangmuirAdsorptionIsothermSample* s1, 
                           const LangmuirAdsorptionIsothermSample* s2 ) const;

      };

   }

}

#endif // _DATA_ACCESS__LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_
