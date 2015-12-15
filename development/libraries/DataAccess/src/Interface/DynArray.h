#ifndef _DYNARRAY_
#define _DYNARRAY_

#include <stdlib.h>
#include <assert.h>


// forward declaration so that we can declare the DynArrays
template < class ObjType > class DynArrayIterator;

/// Array that grows dynamically to store objects at the requested index.
template < class ObjType > class DynArray
{

 public:
   typedef DynArrayIterator < ObjType > iterator;

   DynArray (unsigned int initialSize = 5, float resizeFactor = 1.3);

   ~DynArray ();

   /// return number of elements in the array, O(N).
   inline unsigned int size () const;

   // return largest possible number of elements in the array, O(1).
   inline unsigned int capacity () const
   {
      return m_capacity;
   }

   /// index of the next non-zero array entry following the entry of the given index, O(1).
   inline int next (const int index) const;

   /// returns the element at the index.
   inline const ObjType & get (const int index) const;

   /// puts an element at the index and returns the old one.
   inline ObjType & put (const int index, const ObjType & value);

   /// array operator for retrieval and assignment.
   inline ObjType & operator[](const int index);

   /// remove all elements
   inline void clear (void);

   /// return an iterator for the array positioned at the first element of the array
   inline DynArrayIterator< ObjType > begin ();
   /// return an iterator for the array positioned beyond the last element of the array
   inline DynArrayIterator< ObjType > end ();


 private:
   //Member variables
   unsigned int m_capacity;            //the size of the table.
   float m_resizeFactor;
   ObjType *m_table;
   ObjType nullObject;
   //Helper functions
   inline void resize (unsigned int index);
};


template < class ObjType > class DynArrayIterator
{
 private:
   typedef DynArray < ObjType > PDynArray;

   PDynArray * m_array;

   unsigned int m_index;

 public:

   DynArrayIterator (PDynArray & hashtable);
   DynArrayIterator (const DynArrayIterator<ObjType> & rhs);
   DynArrayIterator<ObjType> & operator= (const DynArrayIterator<ObjType> & rhs);
   bool first (void);
   bool next (void);
   DynArrayIterator<ObjType> & operator++ (void);
   bool operator!= (DynArrayIterator<ObjType> rhs);
   bool reset (void);

   ObjType & value (void);
   unsigned int index (void);
};


/// Implementation of constructor, destructor, and member functions of DynArray
template < class ObjType >
DynArray < ObjType >::DynArray (unsigned int initialSize, float resizeFactor)
{
   m_capacity = initialSize;
   m_resizeFactor = resizeFactor > 1 ? resizeFactor : 1;

   /// initialize and set to null
   m_table = new ObjType [m_capacity];

   nullObject = 0;
   
   for (unsigned int i = 0; i < m_capacity; i++)
      m_table[i] = 0;
}

template < class ObjType >
DynArray < ObjType >::~DynArray ()
{
   delete [] m_table;
}

template < class ObjType >
unsigned int DynArray < ObjType >::size (void) const
{
   int sz = 0;
   for (int i = 0; i < m_capacity; i++)
      if (m_table[i]) ++sz;
   return sz;
}

template < class ObjType >
int DynArray < ObjType >::next (const int index) const
{
   for (int i = index + 1; i < m_capacity; i++)
      if (m_table[i]) return i;
   return m_capacity;
}

template < class ObjType >
const ObjType & DynArray < ObjType >::get (const int index) const
{
   if (index < m_capacity)
      return m_table[index];
   else
      return nullObject;
}

template < class ObjType >
void DynArray < ObjType >::resize (unsigned int index)
{
   unsigned int oldCapacity = m_capacity;
   ObjType *oldTable = m_table;
   unsigned int i;

   m_capacity = static_cast<unsigned int>(static_cast<float>(index) * m_resizeFactor);
   m_table = new ObjType [m_capacity];

   for (i = 0; i < oldCapacity; i++)
      m_table[i] = oldTable[i];

   for (i = i; i < m_capacity; i++)
      m_table[i] = 0;

   delete [] oldTable;
}

template < class ObjType >
ObjType & DynArray < ObjType >::put (const int index, const ObjType & value)
{
   ObjType null = 0;
   ObjType & old = null;

   if (index >= m_capacity)
      resize (index + 1);
   else
      old = m_table[index];

   m_table[index] = value;

   return old;
}

template < class ObjType >
ObjType & DynArray < ObjType >::operator[](const int index)
{
   if (index >= m_capacity)
      resize (index + 1);
   return m_table[index];
}

template < class ObjType >
void DynArray < ObjType >::clear (void)
{
   for (unsigned int i = 0; i < m_capacity; i++)
      m_table[i] = 0;
}

template < class ObjType >
DynArrayIterator< ObjType > DynArray < ObjType >::begin (void)
{
   DynArrayIterator< ObjType > theIterator (* this);
   theIterator.first ();

   return theIterator;
}

template < class ObjType >
DynArrayIterator< ObjType > DynArray < ObjType >::end (void)
{
   DynArrayIterator< ObjType > theIterator (* this);

   return theIterator;
}

//Implementaion of DynArrayIterator methods
template < class ObjType >
DynArrayIterator < ObjType >::DynArrayIterator (PDynArray & rhs)
{
   m_array = & rhs;
   m_index = -1;
}

template < class ObjType >
DynArrayIterator < ObjType >::DynArrayIterator (const DynArrayIterator<ObjType> & rhs)
{
   m_array = rhs.m_array;
   m_index = rhs.m_index;
}

template < class ObjType >
DynArrayIterator<ObjType> & DynArrayIterator < ObjType >::operator= (const DynArrayIterator<ObjType> & rhs)
{
   m_array = rhs.m_array;
   m_index = rhs.m_index;

   return * this;
}

template < class ObjType >
bool DynArrayIterator < ObjType >::first (void)
{
   ObjType *tab = (* m_array).m_table;

   for (unsigned int i = 0; i < (* m_array).m_capacity; i++)
   {
      if (tab[i] != 0)
      {
         m_index = i;
         return true;
      }
   }
   return false;
}

template < class ObjType >
bool DynArrayIterator < ObjType >::next (void)
{

   ObjType * tab = (* m_array).m_table;

   for (unsigned int i = m_index + 1; i < (* m_array).m_capacity; i++)
   {
      if (tab[i] != 0)
      {
         m_index = i;
         return true;
      }
   }

   reset ();
   return false;
}

template < class ObjType >
DynArrayIterator<ObjType> & DynArrayIterator < ObjType >::operator++ (void)
{
   next ();
   return * this;
}

template < class ObjType >
bool DynArrayIterator < ObjType >::operator!= (DynArrayIterator<ObjType> rhs)
{
   if (m_array != rhs.m_array ||
       m_index != rhs.m_index)
      return true;
   else
      return false;
}

template < class ObjType >
bool DynArrayIterator < ObjType >::reset (void)
{
   m_index = -1;
   return true;
}

template < class ObjType >
ObjType & DynArrayIterator < ObjType >::value (void)
{
   return (*m_array)[m_index];
}

template < class ObjType >
unsigned int DynArrayIterator < ObjType >::index (void)
{
   return m_index;
}

#endif // _DYNARRAY_
