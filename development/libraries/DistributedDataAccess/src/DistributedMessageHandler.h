#ifndef _DATA_ACCESS__DISTRIBUTED_MESSAGE_HANDLER__H_
#define _DATA_ACCESS__DISTRIBUTED_MESSAGE_HANDLER__H_

#include <string>

#include "MessageHandler.h"

namespace DataAccess {

   namespace Interface {


      /// \brief A class for handling output messages from simulators.
      class DistributedMessageHandler : public MessageHandler {

      public :


         ~DistributedMessageHandler ();


         /// \brief Print a string.
         void print ( const std::string& str ) const;

         /// \brief Print an integer.
         void print ( const int          val ) const;

         /// \brief Print a double.
         void print ( const double       val ) const;


         /// \brief Print a string.
         void synchronisedPrint ( const std::string& str ) const;

         /// \brief Print an integer.
         void synchronisedPrint ( const int          val ) const;

         /// \brief Print a double.
         void synchronisedPrint ( const double       val ) const;


         /// \brief Print a string and add a new line.
         void printLine ( const std::string& str ) const;

         /// \brief Print a string and add a new line.
         void synchronisedPrintLine ( const std::string& str ) const;


         /// \brief Print a new line.
         void newLine () const;
         
         /// \brief Print a new line.
         void synchronisedNewLine () const;
         

         /// \brief Flush the output stream.
         void flush () const;

      };

   }

}


#endif // _DATA_ACCESS__DISTRIBUTED_MESSAGE_HANDLER__H_
