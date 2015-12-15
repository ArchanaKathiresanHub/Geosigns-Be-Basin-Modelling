#ifndef HPC_GENERALEXCEPTION_H
#define HPC_GENERALEXCEPTION_H

#include <exception>
#include <sstream>

namespace hpc {

class GeneralException : public std::exception
{
public:
  GeneralException();
  GeneralException(const GeneralException & other);

  GeneralException & operator=( const GeneralException & other);

  // Overriding std::exception functions
  virtual ~GeneralException() throw () ;
  virtual const char * what() const throw ()  ;

  // Allow nice notation
  template <typename T>
  GeneralException & operator<<(const T & x )
  {
    m_message << x;
    return *this;
  }

private:
  std::ostringstream m_message;
  mutable std::string m_formatted;
};

template <typename T>
struct BaseException : GeneralException
{
  template <typename U>
  T & operator<< (const U & x) 
  {
    return static_cast<T &>( GeneralException::operator<<( x ) );
  }
};

}

#endif
