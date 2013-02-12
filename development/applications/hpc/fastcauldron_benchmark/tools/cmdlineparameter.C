#include "cmdlineparameter.h"
#include "parser.h"


namespace hpc
{


boost::shared_ptr< CmdLineParameter >
CmdLineParameter
   :: parse( const std::string & text )
{
   Parser parser(text);

   std::string firstToken = parser.nextToken();

   if (firstToken == "{")
   {  // parse a ChoiceCmdLineParameter
      std::vector< std::string > names;
      std::vector< std::string > options;
      std::string comma;
      do
      {
         names.push_back( parser.nextToken() );
         parser.expect(":");
         options.push_back( parser.nextToken() );

         comma = parser.nextToken();
      } while(comma == ",");

      if (comma != "}")
         throw ParseException() << "Expected a '}' at end of choice command line parameter";

      return boost::shared_ptr<CmdLineParameter>( new ChoiceCmdLineParameter(names, options));
   }
   else
   {  // parse a ParameterizedCmdLineParameter
      return boost::shared_ptr<CmdLineParameter>( new ParameterizedCmdLineParameter(firstToken));
   }
}

ParameterizedCmdLineParameter
   :: ParameterizedCmdLineParameter( const std::string &  option)
   : m_option(option)
{}

std::vector< CmdLineParameter :: Option >
ParameterizedCmdLineParameter
   :: getOptions( const std::vector< Value > & values) const
{
   std::ostringstream result;
   std::ostringstream placeholder;

   enum { PLACEHOLDER, QUOTE, OTHER } state = OTHER;

   for (unsigned i = 0; i < m_option.size(); ++i)
   {
      switch(state)
      {
         case OTHER:
            {
               switch(m_option[i])
               {
                  case '{': 
                     state = PLACEHOLDER; 
                     break;

                  case '\\': 
                     state = QUOTE; 
                     break;

                  default: 
                     result << m_option[i]; 
                     break;
               }
               break;
            }

         case QUOTE:
            result << m_option[i];
            state = OTHER;
            break;

         case PLACEHOLDER:
            {
               switch(m_option[i])
               {
                  case '}': 
                     {
                        // parse placeholder number
                        std::istringstream number(placeholder.str());
                        int n = -1;
                        number >> n;

                        if (!number || n < 0 || n>= values.size() )
                           throw ParseException() << "Placeholder in option string could not be parsed";

                        // fill in placeholder
                        result << values[n];

                        // reset placeholder object
                        placeholder.str("");

                        // reset state to OTHER
                        state = OTHER; 
                        break;
                     }

                  default:
                     placeholder << m_option[i];
                     break;
               }
               break;
            }

         default:
            assert( false && "Unknown state");
      } // switch state
   }

   if (state == QUOTE)
      throw ParseException() << "Unexpected end of string after quote '\\' character";

   if (state == PLACEHOLDER)
      throw ParseException() << "Unexpected end of string inside placholder specification";

   return std::vector<Option>(1, result.str());
}   

ChoiceCmdLineParameter
   :: ChoiceCmdLineParameter( const std::vector< Name > & names, const std::vector< Option > & options)
   : m_names(names)
   , m_options(options)
{}

std::vector< CmdLineParameter::Option >
ChoiceCmdLineParameter
   :: getOptions( const std::vector< Value > & values) const
{
   assert( values.size() >= 1);

   unsigned i = std::distance( m_names.begin(), std::find( m_names.begin(), m_names.end(), values[0]));
   assert( i < m_names.size() );

   return std::vector< CmdLineParameter::Option > (1, m_options[i]);
}


} // namespace
