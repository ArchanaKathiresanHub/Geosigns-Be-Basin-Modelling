#include <iostream>
using namespace std;

#include "Interface/Parent.h"
#include "Interface/Child.h"

using namespace DataAccess;
using namespace Interface;

Child::Child (void) : m_parent (0), m_index (0)
{
}

Child::Child (const Parent * parent, unsigned int index) : m_parent (parent), m_index (index)
{
   if (m_parent) m_parent->setChild (this, index);
}

Child::~Child (void)
{
   if (m_parent) m_parent->detachChild (m_index);
}

void Child::release (void) const
{
   if (m_parent) m_parent->releaseChild (m_index);
}

const Parent * Child::getParent (void) const
{
   return m_parent;
}
