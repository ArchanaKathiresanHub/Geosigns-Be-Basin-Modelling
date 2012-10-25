#ifndef __simplesmartpointer__
#define __simplesmartpointer__

// 
// This header file contains classes for a 
// simple Smart Pointer to be used as a container for 
// pointing to an object or an array of objects
//
// The SimpleSmartPointer class inherits from a pointer Policy class
// So, when inheriting from the Array1D policy, it should be used to contain 1d arrays
// and when inheriting from the Pointer policy, it should be used with normal pointers
//
// The policy classes each have a deletePointer method and protected ctor/dtor as want 
// to avoid needing virtual dtor and are for inheriting only

//
// Pointer Policy
// treats pointer as noraml pointer
// calling normal delete
//
template <typename PointerType>
struct PointerPolicy
{
   void deletePointer (PointerType *p)
   { 
      if (p) delete p;
   }
protected:
   PointerPolicy () {}
   ~PointerPolicy () {}
};

//
// Array1dPolicy class treats the pointer as
// a 1D array, using delete []
//
template <typename PointerType>
struct Array1dPolicy
{
   void deletePointer (PointerType *p)
   {
      if (p) delete [] p;
   }
protected:
  Array1dPolicy () {}
   ~Array1dPolicy () {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Simple Smart Pointer class
// takes two template arguments
//
// PointerType : type of pointer
// PolicyType: type of policy regarding pointer (normal pointer or array)
//

// The following two lines are not allowed with CC. But are allowed with other
// compilers and are useful as you can pass an uninstantiated template parameter
// and then instantiate it with the first parameter, so the user of the class doesn't
// have to specify the template type twice
//
//template <typename PointerType, template <typename> PolicyType>
//      class SimpleSmartPointer : public PolicyType<PointerType> {}

template <typename PointerType, typename PolicyType>
class SimpleSmartPointer : public PolicyType 
{
public:
   // ctor / dtor
   explicit SimpleSmartPointer (PointerType* p = 0)
      : m_p (p), PolicyType ()
   {}
   
   ~SimpleSmartPointer () throw ()
   {
      deletePointer (m_p);
   }
   
   // public methods
   PointerType* release ()
   {
      PointerType *p = m_p;
      m_p = 0;
      return p;
   }
   
   PointerType* get ()
   {
      return m_p;
   }
   
private:
   // forbidden copy ctor and assignment
   SimpleSmartPointer<PointerType, PolicyType> (const SimpleSmartPointer<PointerType, PolicyType>& rhs);
   PointerType* operator= (const SimpleSmartPointer<PointerType, PolicyType> &rhs);
   
    // member variables
   PointerType *m_p;
};

#endif


