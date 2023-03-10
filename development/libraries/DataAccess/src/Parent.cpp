#include "Parent.h"
#include "Child.h"

using namespace DataAccess;
using namespace Interface;

Parent::Parent (void)
{
   m_children = nullptr;
}

Parent::Parent (Child * child, unsigned int index)
{
   m_children = new DynArray<Child *> (index + 1, 1);
   (* m_children)[index] = child;
}

Parent::~Parent (void)
{
   if (m_children)
   {
      for (unsigned int index = 0; index < (* m_children).capacity (); ++index)
      {
         if ((* m_children)[index]) delete (* m_children)[index];
      }
      delete m_children;
   }
}

void Parent::setChild (Child * child, unsigned int index) const
{
   assert (child->getParent () == this);
   if (!m_children) m_children = new DynArray<Child *> (index + 1, 1);

   if ((* m_children)[index] && (* m_children)[index] != child) delete (* m_children)[index];

   (* m_children)[index] = child;
}

void Parent::detachChild (unsigned int index) const
{
   if (m_children) (* m_children)[index] = nullptr;
}

void Parent::releaseChild (unsigned int index) const
{
   if (m_children && (* m_children)[index])
   {
      Child * tmpChild = (* m_children)[index];
      (* m_children)[index] = nullptr;
      delete tmpChild;
   }
   assert (getChild (index) == nullptr);
}

Child * Parent::getChild (unsigned int index) const
{
   if (m_children)
      return (* m_children)[index];
   else
      return nullptr;
}
