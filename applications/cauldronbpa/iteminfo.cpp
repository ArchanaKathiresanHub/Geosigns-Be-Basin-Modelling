#include "BPAClientLibrary.hpp"

#include "iteminfo.h"
#include <qlistbox.h>
#include <qlistview.h>

ItemInfoBase::ItemInfoBase (int type) :
   BitField (), m_listBoxItem (0), m_listViewItem (0), m_type (type), m_copy (0)
{
}

ItemInfoBase::ItemInfoBase (const ItemInfoBase & orig) : BitField (orig)
{
   m_children = orig.m_children;
   m_parents = orig.m_parents;

   m_downloaded = orig.m_downloaded;
   m_listBoxItem = 0;
   m_listViewItem = 0;

   m_type = orig.m_type;
   m_copy = 0;
}

ItemInfoBase::~ItemInfoBase (void)
{
   // cerr << "removing: " << this << endl;
   removeChildren ();

   vector<ItemInfoBase *>::iterator iter;

   while ((iter = m_parents.begin ()) != m_parents.end ())
   {
      (* iter)->removeChild (this);
   }

   if (m_listBoxItem) delete m_listBoxItem;
   if (m_listViewItem) delete m_listViewItem;
}


ItemInfoBase * ItemInfoBase::deepCopy ()
{
   // static int depth = 0;

   // cerr << "DeepCopying " << getName () << " (" << this << ")"  << "(" << depth << ")" << endl;
   // ++depth;

   if (!m_copy)
   {
      m_copy = createCopy ();
      // cerr << "Copied " << m_copy->getName () << " (" << m_copy << ")" << endl;
      m_copy->deepCopyElements ();
   }

   // --depth;
   // cerr << "Returning " << m_copy->getName ()  << " (" << m_copy << ")" << "(" << depth << ")" << endl;

   return m_copy;
}
      
void ItemInfoBase::deepCopyElements ()
{
   vector < ItemInfoBase * >::iterator iter;

   // cerr << "DeepCopying parents of " << getName () << endl;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      ItemInfoBase * parent = * iter;
      *iter = parent->deepCopy ();
   }

   // cerr << "DeepCopying children of " << getName () << endl;
   for (iter = m_children.begin (); iter != m_children.end (); ++iter)
   {
      ItemInfoBase * child = * iter;
      *iter = child->deepCopy ();
   }
}

void ItemInfoBase::clearCopy ()
{
   m_copy = 0;
}

void * ItemInfoBase::getCopy ()
{
   return m_copy;
}

template <class ItemType>
ItemInfo <ItemType>::ItemInfo (int type) :
   ItemInfoBase (type), m_item (0)
{
}

template <class ItemType>
ItemInfo <ItemType>::ItemInfo (const ItemInfo <ItemType> & orig) :
   ItemInfoBase (orig)
{
   m_item = orig.m_item;
   // cerr << "Copying m_item " << m_item->getName () << endl;
}

template <class ItemType>
ItemInfo <ItemType>::~ItemInfo (void)
{
   // if (m_item) delete m_item;
}

template <class ItemType>
ItemInfo<ItemType> * ItemInfo<ItemType>::createCopy ()
{
   // cerr << "Copying " << getName () << endl;
   return new ItemInfo<ItemType> (* this);
}

template <class ItemType>
void ItemInfo<ItemType>::deepCopyElements ()
{
   ItemInfoBase::deepCopyElements ();
}

template <class ItemType>
void ItemInfo <ItemType>::setItem (ItemType * item)
{
   m_item = item;
}

template <class ItemType>
ItemType * ItemInfo <ItemType>::getItem (void)
{
   return m_item;
}

template <class ItemType> long ItemInfo <ItemType>::getId (void)
{
   if (m_item) return m_item->getId ();
   else return -1;
}

template <class ItemType> const std::string & ItemInfo <ItemType>::getName (void)
{
   static const string empty = "";
   if (m_item) return m_item->getName ();
   else return empty;
}

void ItemInfoBase::addParent (ItemInfoBase * parent)
{
   m_parents.push_back (parent);
}

bool ItemInfoBase::removeParent (ItemInfoBase * parent)
{
   vector<ItemInfoBase *>::iterator iter;

   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      if (* iter == parent)
      {
	 m_parents.erase (iter);
	 return true;
      }
   }
   return false;
}

bool ItemInfoBase::hasParents (void)
{
   return (m_parents.size () != 0);
}

bool ItemInfoBase::parentsSelected (void)
{
   if (!hasParents ()) return true;

   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      if ((* iter)->isSelected ())
      {
	 return true;
      }
   }
   return false;
}

bool ItemInfoBase::parentsSelected (int type)
{
   if (!hasParents ()) return false;

   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      if ((* iter)->isSelected () && (* iter)->getType () == type)
      {
	 return true;
      }
   }
   return false;
}

void ItemInfoBase::selectParents (void)
{
   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      ItemInfoBase * parent = * iter;
      // cerr << "Selecting parent: " << parent->getName () << endl;
      parent->selectParents ();
      parent->setSelected (true);
   }
}

void ItemInfoBase::selectParents (InclusionMode mode, unsigned int types)
{
   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      ItemInfoBase * parent = * iter;

      if ((mode == Excluding && (parent->getType () & types) == 0) ||
	    (mode == Including && (parent->getType () & types) != 0))
      {
	 parent->selectParents (mode, types);
	 // cerr << "Selecting parent: " << parent->getName () << endl;
	 parent->setSelected (true);
      }
   }
}

void ItemInfoBase::deselectParents (void)
{
   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      ItemInfoBase * parent = * iter;
      // cerr << "Deselecting parent: " << parent->getName () << endl;
      parent->deselectParents ();
      parent->setSelected (false);
   }
}

bool ItemInfoBase::hasChildren (void)
{
   return (m_children.size () != 0);
}

void ItemInfoBase::highlight (void)
{
   // cerr << "highlighting " << getName () << endl;
   if (m_listBoxItem)
      m_listBoxItem->listBox ()->setCurrentItem (m_listBoxItem);
   if (m_listViewItem)
      m_listViewItem->listView ()->setCurrentItem (m_listViewItem);

}

void ItemInfoBase::dehighlight (void)
{
}

void ItemInfoBase::highlightParents (void)
{
   vector<ItemInfoBase *>::iterator iter;
   for (iter = m_parents.begin (); iter != m_parents.end (); ++iter)
   {
      ItemInfoBase * parent = * iter;
      parent->highlight ();
      parent->highlightParents ();
   }
}

void ItemInfoBase::dehighlightParents (void)
{
}


void ItemInfoBase::setListBoxItem (QListBoxItem * listBoxItem)
{
   m_listBoxItem = listBoxItem;
}

QListBoxItem * ItemInfoBase::getListBoxItem (void)
{
   return m_listBoxItem;
}

void ItemInfoBase::setListViewItem (QListViewItem * listViewItem)
{
   m_listViewItem = listViewItem;
}

QListViewItem * ItemInfoBase::getListViewItem (void)
{
   return m_listViewItem;
}


int ItemInfoBase::getType (void)
{
   return m_type;
}

bool ItemInfoBase::isVisible (void)
{
   return (m_listBoxItem && m_listBoxItem->listBox () != 0 ||
	 m_listViewItem && m_listViewItem->listView () != 0);
}

void ItemInfoBase::setSelected (bool select)
{
   if (m_listBoxItem && m_listBoxItem->listBox () != 0)
   {
      m_listBoxItem->listBox ()->setSelected (m_listBoxItem, select);
   }
   if (m_listViewItem && m_listViewItem->listView () != 0)
   {
      m_listViewItem->listView ()->setSelected (m_listViewItem, select);
   }
}

void ItemInfoBase::setCurrent ()
{
   if (m_listBoxItem && m_listBoxItem->listBox () != 0)
   {
      m_listBoxItem->listBox ()->setCurrentItem (m_listBoxItem);
   }
   if (m_listViewItem && m_listViewItem->listView () != 0)
   {
      m_listViewItem->listView ()->setCurrentItem (m_listViewItem);
   }
}

bool ItemInfoBase::isSelected (void)
{
   return (m_listBoxItem && m_listBoxItem->isSelected () ||
	 m_listViewItem && m_listViewItem->isSelected ());
}

void ItemInfoBase::addChild (ItemInfoBase * child)
{
   if (hasChild (child))
   {
      // cerr << "child: " << child->getName () << " already belongs to parent: " << getName () << endl;
      return;
   }
   // cerr << "Adding child: " << child->getName () << " to parent: " << getName () << endl;

   m_children.push_back (child);
   child->addParent (this);
}

void ItemInfoBase::removeChildren (void)
{
   vector<ItemInfoBase *>::iterator iter;

   for (iter = m_children.begin (); iter != m_children.end (); ++iter)
   {
      ItemInfoBase * child = * iter;
      // cerr << "removing parent: " << this << " from child: " << child << endl;
      child->removeParent (this);
      if (!child->hasParents ()) delete child;
   }
   m_children.clear ();
}

int ItemInfoBase::numChildren (void)
{
   return m_children.size ();
}

int ItemInfoBase::numParents (void)
{
   return m_parents.size ();
}

ItemInfoBase * ItemInfoBase::findChild (const string & name)
{
   vector<ItemInfoBase *>::iterator iter;

   for (iter = m_children.begin (); iter != m_children.end (); ++iter)
   {
      if ((* iter)->getName () == name)
      {
	 return *iter;
      }
   }
   return 0;
}

bool ItemInfoBase::hasChild (ItemInfoBase * child)
{
   vector<ItemInfoBase *>::iterator iter;

   for (iter = m_children.begin (); iter != m_children.end (); ++iter)
   {
      if (* iter == child)
      {
	 return true;
      }
   }
   return false;
}

bool ItemInfoBase::removeChild (ItemInfoBase * child)
{
   vector<ItemInfoBase *>::iterator iter;

   for (iter = m_children.begin (); iter != m_children.end (); ++iter)
   {
      if (* iter == child)
      {
	 child->removeParent (this);
	 // cerr << "removing child: " << child << " from parent: " << this << endl;
	 m_children.erase (iter);
	 // if (!child->hasParents ()) delete child;
	 return true;
      }
   }
   return false;
}

#ifndef linux
#pragma instantiate ItemInfo <const BPA::OwnerOrg>
#pragma instantiate ItemInfo <const BPA::Country>
#pragma instantiate ItemInfo <const BPA::Region>
#pragma instantiate ItemInfo <const BPA::Basin>
#pragma instantiate ItemInfo <BPA::CauldronProject>
#pragma instantiate ItemInfo <const BPA::CoordinateSystem>
#pragma instantiate ItemInfo <const BPA::GeoTransform>
#else
template class ItemInfo <const BPA::OwnerOrg>;
template class ItemInfo <const BPA::Country>;
template class ItemInfo <const BPA::Region>;
template class ItemInfo <const BPA::Basin>;
template class ItemInfo <BPA::CauldronProject>;
template class ItemInfo <const BPA::CoordinateSystem>;
template class ItemInfo <const BPA::GeoTransform>;
#endif
