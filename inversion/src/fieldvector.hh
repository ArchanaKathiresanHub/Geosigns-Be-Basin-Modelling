#ifndef FIELDVECTOR_H
#define FIELDVECTOR_H
#include<iostream>
#include <assert.h>
#include <cmath>
template<class T, int SIZE>
class FieldVector
{
 public:
  typedef T fieldType;
  FieldVector()
	{//values set to zero
	  for(int i=0;i<SIZE;++i)
		p[i]=0;
	}
  FieldVector(const T& n)
	{//values set to const
	  for(int i=0;i<SIZE;++i)
		p[i]=n;
	}
  
  const fieldType& operator[](int i) const
  {
	assert(i<SIZE);
	if(i>=SIZE)
	  {
		std::cout<<"ERROR Dimension exceeded!"<<std::cout;
		exit(0);
	  }
	return p[i];
  }
  
  fieldType& operator[](int i)
	{
	  assert(i<SIZE);
	  if(i>=SIZE)
        {
          std::cout<<"ERROR Dimension exceeded!"<<std::cout;
          exit(0);
        }
	  return p[i];
	}
  //scalar assignment
  FieldVector<T,SIZE>& operator=(const fieldType& scalar);
  //scalar multiplication
  FieldVector<T,SIZE>& operator*(const fieldType& scalar);
//vector compnent wise multiplication
  FieldVector<T,SIZE>& operator*(const FieldVector<T,SIZE>& v);

  double two_norm () const;
  
  
  ~FieldVector()
	{}
 private:
  T p[SIZE];
};

//scalar assignment
template<class T,int SIZE>
  FieldVector<T,SIZE>& FieldVector<T,SIZE>::operator=(const T& number)
{
  for(int i=0;i<SIZE;++i)
	p[i]=number;
  return *this;
}
//scalar multiplication
  template<class T,int SIZE>
  FieldVector<T,SIZE>& FieldVector<T,SIZE>::operator*(const T& scalar)
{
  for(int i=0;i<SIZE;++i)
	p[i]*=scalar;
   return *this;
}

//vector component wise multiplication
  template<class T,int SIZE>
  FieldVector<T,SIZE>& FieldVector<T,SIZE>::operator*(const FieldVector<T,SIZE>& v)
{
  for(int i=0;i<SIZE;++i)
	p[i]*=v[i];
   return *this;
}



  template<class T,int SIZE>
  std::ostream& operator<<(std::ostream& os, const FieldVector<T,SIZE>& v)
  {
	for (int i=0; i<SIZE; i++)
	  os << ((i>0) ? " " : "") << v[i];
	return os;
  }


template< class T, int SIZE >
	inline std::istream& operator>>(std::istream& is, FieldVector<T,SIZE>& v)
{
  //FieldVector<T,SIZE> w;
  for( int i = 0; i < SIZE; ++i)
	is >> v[i];
  //if(is)
  //v = w;
  return is;
}




//! two norm sqrt(sum over squared values of entries)
template< class T, int SIZE >
double FieldVector<T,SIZE>::two_norm () const
	{
          double result = 0;
          for (int i=0; i<SIZE; i++)
              result += (p[i]*p[i]);
          return std::sqrt(result);
	}



#endif 
