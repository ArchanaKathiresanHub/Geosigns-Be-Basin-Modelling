#ifndef SAUA_PARAMETERS
#define SAUA_PARAMETERS

#include <string>

class SAUAParameters
{
   public:
      SAUAParameters() {;}
   
      void setDoE( const std::string & doe, const std::string & prms ) { m_nameDoE = doe; m_prmsDoE = prms; }

      std::string getDoE()     const { return m_nameDoE; }
      std::string getDoEPrms() const { return m_prmsDoE; }

   private:
      std::string m_nameDoE;
      std::string m_prmsDoE;
};

#endif
