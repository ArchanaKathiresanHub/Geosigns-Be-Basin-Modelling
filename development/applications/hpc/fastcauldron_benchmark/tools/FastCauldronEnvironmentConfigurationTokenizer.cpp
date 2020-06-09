#include "FastCauldronEnvironmentConfigurationTokenizer.h"

   
hpc :: FastCauldronEnvironmentConfigurationTokenizer
   :: FastCauldronEnvironmentConfigurationTokenizer(const std::string & text)
   : m_text(text)
   , m_posLeft(0), m_posRight(0)
{}

void
hpc :: FastCauldronEnvironmentConfigurationTokenizer
   :: next( std::string & token, std::string & marker )
{
   const std::string sepMarkerLeft = "{";
   const std::string sepMarkerRight = "}";

   // the next token is all text up to the next '{' or end of string
   m_posLeft = m_text.find( sepMarkerLeft, m_posLeft );

   std::string::size_type tokenLength = 0;
   if (m_posLeft == std::string::npos)
      tokenLength = std::string::npos;
   else
      tokenLength = m_posLeft - m_posRight;

   token = m_text.substr( m_posRight, tokenLength );

   if (m_posLeft == std::string::npos)
   {  // we have reached the end of string
      marker.clear();
      return;
   }

   // otherwise, there is a marker
   m_posRight = m_text.find(sepMarkerRight, m_posLeft);
   if (m_posRight == std::string::npos)
      throw Exception() << "Missing '" << sepMarkerRight << "' after '" << sepMarkerLeft << "'";

   marker = m_text.substr(m_posLeft + sepMarkerLeft.size(), m_posRight - m_posLeft - sepMarkerLeft.size());

   m_posRight += sepMarkerRight.size();
   m_posLeft = m_posRight;
}

bool
hpc :: FastCauldronEnvironmentConfigurationTokenizer
   :: hasMore() const
{
   return m_posLeft < std::string::npos;
}

