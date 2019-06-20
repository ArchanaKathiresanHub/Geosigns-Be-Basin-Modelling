#ifndef _IBS_FAULT_FILE_READER_H
#define _IBS_FAULT_FILE_READER_H

#include "FaultFileReader.h"
#include "auxiliaryfaulttypes.h"


namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The identifier and object allocator for the fault file reader factory.
      ///
      const std::string IBSFaultFileReaderID = "FLT";

      FaultFileReader* allocateIBSFaultFileReader ();

      ///
      /// Reads faults from the IBS fault file format.
      /// This format is basically a sequence of points in 2D space 
      /// ended by a special marker point (999.999 999.999), unless
      /// the point is the last point in the file when it may be omitted.
      /// Must each point be on its own line?
      /// Comments begin with an '!'.
      ///
      class IBSFaultFileReader : public ASCIIFaultFileReader {

	 public :

	    IBSFaultFileReader ();

	    void open ( const std::string& fileName,
		  bool&        fileIsOpen );

	    void preParseFaults ();


	 private :

	    static const double IBSFaultNullPoint;

	    static const int    BufferSize = 4096;


	    void readFault ( std::string&   faultName,
		  PointSequence& faultLine,
		  bool&          done );

	    void readFault2 ( std::string&   faultName,
		  PointSequence& faultLine,
		  bool&          done );

	    bool isCommentLine ( const char* buffer ) const;

	    ///
	    /// Used to create the fault name
	    ///
	    int faultCount;

      };
   }
}

#endif // _IBS_FAULT_FILE_READER_H
