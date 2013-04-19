#include <string>
#include <sstream>
#include <stdexcept>
using namespace std;

template <class T>
string addToStream (ostream & aStream, const string & aString, const T & arg)
{
      size_t lastpos = 0;
      
      lastpos = aString.find_first_of ('%', 0);
      if (lastpos == string::npos) return aString;
      aStream << aString.substr (0, lastpos);
      aStream << arg;

      return aString.substr (lastpos + 1, string::npos);
}

class RecordException : public exception {
   public:
   RecordException(const string & error) : exception ()
   {
      ostringstream errorStream;
      errorStream << error;
      m_errorString = errorStream.str ();
   }

   template <class T>
   RecordException(const string & error, const T & arg1) : exception ()
   {
      ostringstream errorStream;

      string remainder = error;

      remainder = addToStream (errorStream, remainder, arg1);

      errorStream << remainder;

      m_errorString = errorStream.str ();
   }

   template <class T1, class T2>
   RecordException(const string & error, const T1 & arg1, const T2 & arg2) : exception ()
   {
      ostringstream errorStream;

      string remainder = error;

      remainder = addToStream (errorStream, remainder, arg1);
      remainder = addToStream (errorStream, remainder, arg2);

      errorStream << remainder;

      m_errorString = errorStream.str ();
   }

   template <class T1, class T2, class T3>
   RecordException(const string & error, const T1 & arg1, const T2 & arg2, const T3 & arg3) : exception ()
   {
      ostringstream errorStream;

      string remainder = error;

      remainder = addToStream (errorStream, remainder, arg1);
      remainder = addToStream (errorStream, remainder, arg2);
      remainder = addToStream (errorStream, remainder, arg3);

      errorStream << remainder;

      m_errorString = errorStream.str ();
   }

   virtual ~RecordException () throw ()
   {
   }

   virtual const char* what() const throw()
   {
      return m_errorString.c_str ();
   }

   protected:
   string m_errorString;
};
 


