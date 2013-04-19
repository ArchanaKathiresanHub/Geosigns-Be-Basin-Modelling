#ifndef ITEMINFO_H
#define ITEMINFO_H

#include <vector>
#include <string>

#include "BitField.h"

const unsigned int OwnerOrgsDownloaded = 1 << 0;
const unsigned int RegionsDownloaded = 1 << 1;
const unsigned int CountriesDownloaded = 1 << 2;
const unsigned int BasinsDownloaded = 1 << 3;
const unsigned int ProjectsDownloaded = 1 << 4;
const unsigned int CoordinateSystemsDownloaded = 1 << 5;
const unsigned int GeoTransformsDownloaded = 1 << 6;

class LoginDialog;
class CauldronBPA;
class OwnerOrg;
class Country;
class BpaRegion;
class Basin;
class CauldronProject;

class QListBoxItem;
class QListViewItem;

class ItemInfoBase : public BitField
{
 public:
   enum InclusionMode { Including, Excluding };

   ItemInfoBase (int type);
   ItemInfoBase (const ItemInfoBase & orig);
   virtual ~ItemInfoBase (void);

   ItemInfoBase * deepCopy ();
   virtual void deepCopyElements ();
   virtual ItemInfoBase * createCopy () = 0;

   void clearCopy ();
   void * getCopy ();

   void addParent (ItemInfoBase * parent);
   bool removeParent (ItemInfoBase * child);
   bool hasParents (void);
   bool parentsSelected (void);
   bool parentsSelected (int type);

   void selectParents (void);
   void selectParents (InclusionMode mode, unsigned int types);
   void deselectParents (void);

   ItemInfoBase * findChild (const std::string & name);

   bool hasChildren (void);
   bool hasChild (ItemInfoBase * child);

   void highlight (void);
   void dehighlight (void);
   void highlightParents (void);
   void dehighlightParents (void);
   
   void setListBoxItem (QListBoxItem * listBoxItem);
   QListBoxItem * getListBoxItem (void);
   
   void setListViewItem (QListViewItem * listViewItem);
   QListViewItem * getListViewItem (void);
   
   int getType (void);

   bool isVisible (void);
   bool isSelected (void);
   void setSelected (bool select);

   void setCurrent ();

   virtual long getId () = 0;
   virtual const std::string & getName () = 0;

   void addChild (ItemInfoBase * child);

   bool removeChild (ItemInfoBase * child);
   void removeChildren (void);

   int numChildren (void);
   int numParents (void);

 private:
   std::vector <ItemInfoBase *> m_parents;
   std::vector <ItemInfoBase *> m_children;

   bool m_downloaded;

   QListBoxItem *m_listBoxItem;
   QListViewItem *m_listViewItem;

   int m_type;

   ItemInfoBase * m_copy;
};

template <class ItemType> class ItemInfo : public ItemInfoBase
{
 public:
   ItemInfo (int type);
   ItemInfo (const ItemInfo<ItemType> & orig);
   virtual ~ItemInfo (void);

   virtual ItemInfo<ItemType> * createCopy ();
   virtual void deepCopyElements ();

   virtual long getId ();
   virtual const std::string & getName ();

   void setItem (ItemType * item);
   ItemType * getItem (void);
   
 private:
   ItemType * m_item;
};



#endif // ITEMINFO_H
