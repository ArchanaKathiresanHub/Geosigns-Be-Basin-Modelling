/// The Components Graph inherits from Interactive Graph
/// adding PVT calculation functionaliy and a graph options panel
/// so the user can switch between various graph calculations
/// causing PVT to be performed where necessary

#ifndef __componentgraphform__
#define __componentgraphform__

#include "interactivegraphform.h"
#include "componentgraphoptionsframe.h"

class QPushButton;

class CreateUserRequest;
class DataReader;
class ComponentsReader;
class ComponentsReaderPVT;

class ComponentGraphForm : public InteractiveGraphForm
{
public:
   ComponentGraphForm (QWidget* parent = 0, const char* name = 0, WFlags fl = 0)
      : InteractiveGraphForm (parent, name, fl),  m_componentReaderPVT (0),
      m_currentReader (0), m_componentReader (0)
   {
      setGeometry (x(), y(), width() + 150, height());      
      createGraphOptions ();
   }
   ~ComponentGraphForm() { deleteReaders (); }
      
   DataReader* getReader() { return m_currentReader; }
   void setReader (DataReader *reader) { InteractiveGraphForm::setReader (reader); }
   void setReader (ComponentsReader *reader); 
   void updateGraph ();
   
public slots:
//   virtual void pvtSelectedSlot(bool on);
   
protected:
   virtual void createGraphOptions ();

protected slots:
   void resizePlot ();
   
private:
   ComponentsReader *m_componentReader;
   ComponentsReaderPVT *m_componentReaderPVT;
   DataReader *m_currentReader;
   const CreateUserRequest *m_userRequest;
   ComponentGraphOptionsFrame *m_optionsFrame;
   
   void deleteReaders ();
   void rereadData();
   ComponentsReader* getComponentReader ();
   ComponentsReaderPVT* getPvtReader ();
   void setToComponentReader ();
   void setToPvtReader ();
};

#endif
