#ifndef HPC_CUDAARRAY_H
#define HPC_CUDAARRAY_H

#include <vector>

namespace hpc
{


template <typename T>
class CudaArray;


template <>
class CudaArray<void>
{
public:
  typedef int SizeType;

  CudaArray()
    : m_size(0)
    , m_array(0)
  {}

  explicit CudaArray(SizeType size)
    : m_size(0)
    , m_array(0)
  {
    resize(size);
  }

  ~CudaArray()
  {
    try
    {
      clear();
    }
    catch(...)
    {
      /* TODO: log exception */
    }
  }
  
  CudaArray( const CudaArray & other);

  void swap( CudaArray & other)
  {
    using std::swap;
    swap( this->m_size, other.m_size);
    swap( this->m_array, other.m_array);
  }

  CudaArray & operator=(const CudaArray & other)
  {
    CudaArray tmp(other);
    this->swap(tmp);
    return *this;
  }

  void resize(SizeType size);
  void clear();

  int size() const
  { return m_size; }

protected:
  void getData(void * dst, SizeType srcOffset, SizeType length) const;
  void setData(const void * src, SizeType dstOffset, SizeType length);

  void * devicePointer() 
  { return m_array; }

  const void * devicePointer() const
  { return m_array; }

private:
  SizeType m_size;
  void * m_array;
};

template <typename T>
class CudaArray : public CudaArray<void>
{
public:
  CudaArray()
    : CudaArray<void>()
  {}

  explicit CudaArray(SizeType size)
    : CudaArray<void>(sizeof(T)*size)
  {}
  
  CudaArray( const std::vector<T> & other)
    : CudaArray<void>(sizeof(T) * other.size())
  {
    this->setData( &other[0], 0, sizeof(T)*other.size());
  }

  CudaArray( const T * begin, const T * end)
    : CudaArray<void>(sizeof(T) * std::distance(begin, end))
  {
    this->setData( begin, 0, sizeof(T)*(end-begin));
  }

  operator std::vector<T> () const
  {
    std::vector<T> result( this->size() );
    this->getData( &result[0], 0, CudaArray<void>::size() );
    return result;
  }

  std::vector<T> get(SizeType begin, SizeType end) const
  {
    std::vector<T> result( end - begin );
    this->getData( &result[0], begin*sizeof(T), (end-begin)*sizeof(T));
    return result;
  }

  void resize(SizeType size) 
  { CudaArray<void>::resize( size * sizeof(T) ); } 

  int size() const
  { return CudaArray<void>::size() / sizeof(T); }
      
  T operator[](SizeType i) const
  {
    T x;
    this->getData(&x, sizeof(T)*i, sizeof(T));
    return x;
  }

  T * devicePointer() 
  { return static_cast<T *>(CudaArray<void>::devicePointer()); }

  const T * devicePointer() const
  { return static_cast<const T *>(CudaArray<void>::devicePointer()); }
};



}

#endif
