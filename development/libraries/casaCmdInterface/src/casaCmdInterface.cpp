#include "casaCmdInterface.h"

#include "ErrorHandler.h"
#include "StringHandler.h"

#include <regex>

namespace casaCmdInterface
{

std::string stringVecToStringWithNoSpaces(std::vector<std::string> strings, std::string delimiter)
{
   for (auto& p : strings)
   {
      p = std::regex_replace(p, std::regex(" "), delimiter);
   }
   return StringHandler::implode(strings,delimiter.c_str());
}

std::string casaCommandFromStrVec(std::vector<std::string> stringVec)
{
   //If strings contain spaces, add quotation marks:
   for (std::string& str : stringVec)
   {
      if (str.find_first_of(' ') != std::string::npos)
      {
         str = "\"" + str + "\"";
      }
   }

   //Concatenate with space as delimiter and return:
   return StringHandler::implode(stringVec," ");
}

std::vector<std::string> stringVecFromCasaCommand(const std::string& cmdLine, std::string& cmdID, int lineNum)
{
   std::vector<std::string> cmdPrms;

   std::istringstream iss( cmdLine );

   std::string result;
   // get command itself first
   int tokNum = 0;

   while( std::getline( iss, result, ' ') )
   {
      if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue; // ignore spaces
      if ( result[0] == '#' ) break; // ignore all after comment
      if ( 0 == tokNum ) // get cmd name
      {
         cmdID = result;
      }
      else
      {
         if ( result[0] == '"' ) // need to get the full string without any change inside
         {
            std::string opt = result.substr( 1, result.size() - 1 );
            if ( *opt.rbegin() == '"' )
            {
               opt.erase( opt.size() - 1, 1 );
            }
            else if ( std::getline( iss, result, '"' ) )
            {
               opt += " " + result;
            }
            else
            {
               throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can not find closing \" for the string: " << opt <<
                                                                         ", at line: " << lineNum << ", command: " << cmdID;
            }

            cmdPrms.push_back( opt );
            continue;
         }
         else if ( result[0] == '[' ) // get whole array in one go
         {
            std::string opt = result;
            if ( *opt.rbegin() != ']' )
            {
               if ( std::getline( iss, result, ']' ) )
               {
                  opt += " " + result + "]";
               }
               else
               {
                  throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can not find closing ] for the set: " << opt <<
                                                                            ", at line: " << lineNum << ", command: " << cmdID;
               }
            }
            cmdPrms.push_back( opt );
            continue;
         }
         else
         {
            cmdPrms.push_back( result );
         }
      }
      ++tokNum;
   }

   return cmdPrms;
}



}
