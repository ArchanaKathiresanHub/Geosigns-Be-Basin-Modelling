#ifndef _DATA_ACCESS__MESSAGE_HANDLER__H_
#define _DATA_ACCESS__MESSAGE_HANDLER__H_

#include <string>

namespace DataAccess {

   namespace Interface {


      /// \brief A class for handling output messages from simulators.
      class MessageHandler {

      public :

         MessageHandler () {}

         virtual ~MessageHandler () {}


         /// \brief Print a string.
         virtual void print ( const std::string& str ) const = 0;

         /// \brief Print an integer.
         virtual void print ( const int          val ) const = 0;

         /// \brief Print a double.
         virtual void print ( const double       val ) const = 0;


         /// \brief Print a string.
         virtual void synchronisedPrint ( const std::string& str ) const = 0;

         /// \brief Print an integer.
         virtual void synchronisedPrint ( const int          val ) const = 0;

         /// \brief Print a double.
         virtual void synchronisedPrint ( const double       val ) const = 0;


         /// \brief Print a string and add a new line.
         virtual void printLine ( const std::string& str ) const = 0;

         /// \brief Print a string and add a new line.
         virtual void synchronisedPrintLine ( const std::string& str ) const = 0;


         /// \brief Print a new line.
         virtual void newLine () const = 0;
         
         /// \brief Print a new line.
         virtual void synchronisedNewLine () const = 0;
         

         /// \brief Flush the output stream.
         virtual void flush () const = 0;


      };

   }

}


#endif // _DATA_ACCESS__MESSAGE_HANDLER__H_
