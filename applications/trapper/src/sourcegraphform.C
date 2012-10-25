/// Class SourceGraphForm inherits from InteractiveGraphForm and 
/// adds functionality to show source rock origin data for one component
/// at a time. The graph adds a drop down list with the component options

#include "sourcegraphform.h"
#include "sourcereader.h"

#include <qcombobox.h>
#include <qlabel.h>

#include "globalstrings.h"
using namespace Graph_Properties;

#include "ComponentManager.h"
using namespace CBMGenerics;


DataReader* SourceGraphForm::getReader()
{
   return m_sourceReader;
}

//
// PRIVATE METHODS
//
/// SourceGraphForm::createComboBox creates a component drop down list adding all
/// components to it
void SourceGraphForm::createComboBox ()
{
   ComponentManager &tcm = ComponentManager::getInstance();

   SelectComponentBox = new QComboBox( FALSE, this, "SelectComponentBox" );
   SelectComponentBox->setGeometry( QRect( compBoxX(), compBoxY(), 100, 25 ) );
   
   connect( SelectComponentBox, SIGNAL( activated(int) ), this, SLOT( componentSelectedSlot() ) );
   
   SelectComponentBox->clear();
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C1).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C2).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C3).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C4).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C5).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::N2).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::COx).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C6Minus14Aro).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C6Minus14Sat).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C15PlusAro).c_str()) ); 
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::C15PlusSat).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::resin).c_str()) );
   SelectComponentBox->insertItem( tr(tcm.GetSpeciesName(ComponentManager::asphaltene).c_str()) );

   SelectComponentBox->insertItem( tr(WetGas) );
   SelectComponentBox->insertItem( tr(TotalGas) );
   SelectComponentBox->insertItem( tr(TotalOil) );
}

/// SourceGraphForm::componentSelectedSlot is called when the user selectes a new component
/// the data is the re-read from the source reader and the graph is refreshed
void SourceGraphForm::componentSelectedSlot()
{
   resetSelectedComponent();
   rereadData();
   refreshGraph ();
}

void SourceGraphForm::resetSelectedComponent()
{
   m_sourceReader->setSelectedComponent (SelectComponentBox->currentItem());
}

void SourceGraphForm::rereadData()
{
   // re read table
   m_sourceReader->clearData ();
   m_sourceReader->readData ("MigrationIoTbl");
}

string SourceGraphForm::refreshGraph ()
{
   // redraw graph
   clearPlotData ();
   resetGeneralColourIndex ();
   return InteractiveGraphForm::refreshGraph();
}

void SourceGraphForm::resizeEvent (QResizeEvent *size)
{
   InteractiveGraphForm::resizeEvent(size);
   SelectComponentBox->move (compBoxX(), compBoxY());
}

int SourceGraphForm::compBoxX()
{
   return width()-280;
}

int SourceGraphForm::compBoxY()
{
   return height()-41;
}
