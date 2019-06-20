#ifndef _INTERFACE_CHILD_H_
#define _INTERFACE_CHILD_H_

namespace DataAccess
{
   namespace Interface
   {
      class Parent;

      /// Objects of this class are generally owned by a Parent object which is
      /// ultimately responsible for destroying its Children when it is destroyed itself.
      class Child
      {
    public:
       Child (void);
            /// Spawn a Child with the given Parent
       Child (const Parent * parent, unsigned int index = 0);

       virtual ~Child (void);

       /// detach from its parent and destroy
       virtual void release (void) const;
       /// return the parent
       virtual const Parent * getParent (void) const;

    protected:
            /// The Child's Parent
       mutable const Parent * m_parent;
            /// the index of the Child at the Parent
            const unsigned int m_index;
      };
   }
}

#endif // _INTERFACE_CHILD_H_
