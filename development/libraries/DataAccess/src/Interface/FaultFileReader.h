#ifndef _FAULT_FILE_READER_H
#define _FAULT_FILE_READER_H

#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
    #include<fstream>
    using std::ifstream;
  #else // !_STANDARD_C_PLUS_PLUS
    #include<fstream.h>
  #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
  #include <fstream>
  using std::ifstream;
#endif // sgi

#include <string>
#include <vector>

#include "auxiliaryfaulttypes.h"


namespace DataAccess
{
   namespace Interface
   {
      class FaultFileReader {
         public :
            struct FaultDataItem {
               std::string   faultName;
               PointSequence faultLine;
            };

            typedef std::vector <FaultDataItem> FaultDataSet;
            typedef FaultDataSet::const_iterator FaultDataSetIterator;

            FaultFileReader ();

            virtual ~FaultFileReader ();

            virtual void open ( const std::string& fileName,
                  bool&        fileIsOpen ) = 0;

            virtual void close () = 0;

            ///
            /// Load fault from the fault file into the FaultDataSet
            ///
            virtual void preParseFaults () = 0;


            const std::string&    faultName ( const FaultDataSetIterator& Iter ) const;

            const PointSequence&  faultLine ( const FaultDataSetIterator& Iter ) const;


            ///
            /// Return an iterator pointing to the start of the fault sequence
            ///
            FaultDataSetIterator begin () const;

            ///
            /// Return an iterator pointing to the end of the fault sequence
            ///
            FaultDataSetIterator end   () const;

      private:
         FaultDataSet faultData;


      protected:

         void addFault(const std::string&   newFaultName,
            const PointSequence& newFaultLine);

      };



      class ASCIIFaultFileReader : public FaultFileReader {

         public :

            ASCIIFaultFileReader ();

            ~ASCIIFaultFileReader ();

            void open ( const std::string& fileName,
                  bool&        fileIsOpen );

            void close ();

         protected :

            ifstream faultFile;

            ///
            /// Only reason for this is because, when using the IRIX compiler it doesn't seem to
            /// have the member function "is_open".
            ///
            bool     isOpen;

      };
   }
}


#endif // _FAULT_FILE_READER_H
