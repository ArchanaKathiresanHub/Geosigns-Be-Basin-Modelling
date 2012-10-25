/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

//
// PUBLIC METHODS
//

#include "includestreams.h"
#include "globalstrings.h"
using namespace UserSettings_NS;

#include <string>
using namespace std;

void UserSettings::init()
{ 
   m_volRange = new AxisRange (this);
   m_volRange->setHeading ("Oil/Gas Volume Range");
   
   m_ageRange = new AxisRange (this);
   m_ageRange->setHeading ("Snapshot Range");
   
   m_massRange = new AxisRange (this);
   m_massRange->setHeading ("Component Mass Range");
   
   m_tempPressRange = new AxisRange (this);
   m_tempPressRange->setHeading ("Temperature / Pressure Range");
   
   m_settings = 0;
   m_projectName = 0;
   
   createSettingsObject ();
}

void UserSettings::destroy() 
{
   deleteProjectName (); 
   if ( m_settings ) delete m_settings;
}

AxisRange* UserSettings::getOilGasVolRange() const
{
   return m_volRange;
}

AxisRange* UserSettings::getTempPressRange() const
{
   return m_tempPressRange;
}

AxisRange* UserSettings::getMassRange() const
{
   return m_massRange;
}

AxisRange* UserSettings::getSnapshotRange() const
{
   return m_ageRange;
}

bool UserSettings::massRangeEnabled() const
{
   return enableMassRange->isChecked();
}

bool UserSettings::oilGasVolumeRangeEnabled() const
{
   return enableVolRange->isChecked();
}

bool UserSettings::tempPressRangeEnabled() const
{
   return enableTempPressRange->isChecked();
}

bool UserSettings::snapshotRangeEnabled() const
{
   return enableAgeRange->isChecked();
}

void UserSettings::setProjectName(const char* projectName)
{  
   if ( m_projectName ) delete [] m_projectName;
   
   m_projectName = extractProjectName (projectName);
  
   readSettingsFromFile ();
   
   enableVolSlot();
   enableMassSlot();
   enableTempPressSlot();
   enableAgeSlot();
}

//
// PROTECTED SLOTS
//
void UserSettings::snapshotButtonSlot()
{
   m_ageRange->show ();
}

void UserSettings::volButtonSlot()
{
   m_volRange->show ();
}

void UserSettings::tempPressButtonSlot()
{
   m_tempPressRange->show ();
}

void UserSettings::massButtonSlot()
{
   m_massRange->show ();
}

void UserSettings::enableVolSlot()
{
   oilgasvolButton->setEnabled (enableVolRange->isChecked());
}

void UserSettings::enableMassSlot()
{
   massButton->setEnabled (enableMassRange->isChecked());
}

void UserSettings::enableAgeSlot()
{
   ageRangeButton->setEnabled (enableAgeRange->isChecked());
}

void UserSettings::enableTempPressSlot()
{ 
   tempPressureButton->setEnabled (enableTempPressRange->isChecked());
}

void UserSettings::saveSettingsSlot()
{
   if ( m_projectName != NULL )
   {
      writeSettingsToFile ();
   }
}

//
// PROTECTED METHODS
//
void UserSettings::createSettingsObject()
{ 
   m_settings = new QSettings ();
}

void UserSettings::writeSettingsToFile()
{ 
   writeOilGasVolRange (m_settings);
   writeMassRange (m_settings);
   writeTempPressRange (m_settings);
   writeAgeRange (m_settings);
}

void UserSettings::writeOilGasVolRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
      
   // write max
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeMax);
   settings->writeEntry (tempStr, m_volRange->getMax());
      
   // write min
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeMin);
   settings->writeEntry (tempStr, m_volRange->getMin());
   
   // write enabled
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeEnabled);
   settings->writeEntry (tempStr, oilGasVolumeRangeEnabled());
      
   delete [] tempStr;
}

void UserSettings::writeMassRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
      
   // write max
   sprintf(tempStr, "%s%s", m_projectName, MassRangeMax);
   settings->writeEntry (tempStr, m_massRange->getMax());
      
   // write min
   sprintf(tempStr, "%s%s", m_projectName, MassRangeMin);
   settings->writeEntry (tempStr, m_massRange->getMin());
   
   // write enabled
   sprintf(tempStr, "%s%s", m_projectName, MassRangeEnabled);
   settings->writeEntry (tempStr, massRangeEnabled());
      
   delete [] tempStr;
}

void UserSettings::writeTempPressRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
      
   // write max
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeMax);
   settings->writeEntry (tempStr, m_tempPressRange->getMax());
      
   // write min
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeMin);
   settings->writeEntry (tempStr, m_tempPressRange->getMin());
   
   // write enabled
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeEnabled);
   settings->writeEntry (tempStr, tempPressRangeEnabled());
      
   delete [] tempStr;
}

void UserSettings::writeAgeRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
      
   // write max
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeMax);
   settings->writeEntry (tempStr, m_ageRange->getMax());
      
   // write min
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeMin);
   settings->writeEntry (tempStr, m_ageRange->getMin());
   
   // write enabled
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeEnabled);
   settings->writeEntry (tempStr, snapshotRangeEnabled());
      
   delete [] tempStr;
}

void UserSettings::readSettingsFromFile()
{
   readOilGasVolRange (m_settings);
   readMassRange (m_settings);
   readTempPressRange (m_settings);
   readAgeRange (m_settings);
}

void UserSettings::readOilGasVolRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
   
   // read max
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeMax);
   m_volRange->setMax (settings->readDoubleEntry (tempStr));
   
   // read min
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeMin);
   m_volRange->setMin (settings->readDoubleEntry (tempStr));
   
   // read enable
   sprintf(tempStr, "%s%s", m_projectName, OilGasVolumeRangeEnabled);
   enableVolRange->setChecked (settings->readBoolEntry (tempStr));
   
   delete [] tempStr;
}

void UserSettings::readMassRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
   
   // read max
   sprintf(tempStr, "%s%s", m_projectName, MassRangeMax);
   m_massRange->setMax (settings->readDoubleEntry (tempStr));
   
   // read min
   sprintf(tempStr, "%s%s", m_projectName, MassRangeMin);
   m_massRange->setMin (settings->readDoubleEntry (tempStr));
   
   // read enable
   sprintf(tempStr, "%s%s", m_projectName, MassRangeEnabled);
   enableMassRange->setChecked (settings->readBoolEntry (tempStr));
   
   delete [] tempStr;
}

void UserSettings::readTempPressRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
   
   // read max
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeMax);
   m_tempPressRange->setMax (settings->readDoubleEntry (tempStr));
   
   // read min
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeMin);
   m_tempPressRange->setMin (settings->readDoubleEntry (tempStr));
   
   // read enable
   sprintf(tempStr, "%s%s", m_projectName, TempPressRangeEnabled);
   enableTempPressRange->setChecked (settings->readBoolEntry (tempStr));
   
   delete [] tempStr;
}

void UserSettings::readAgeRange (QSettings *settings)
{
   char *tempStr = new char[strlen (m_projectName) + 30];
   
   // read max
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeMax);
   m_ageRange->setMax (settings->readDoubleEntry (tempStr));
   
   // read min
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeMin);
   m_ageRange->setMin (settings->readDoubleEntry (tempStr));
   
   // read enable
   sprintf(tempStr, "%s%s", m_projectName, SnapshotRangeEnabled);
   enableAgeRange->setChecked (settings->readBoolEntry (tempStr));
   
   delete [] tempStr;
}

void UserSettings::deleteProjectName()
{
   if ( m_projectName ) delete [] m_projectName;
}

char* UserSettings::extractProjectName( const char * projectName )
{
   string tempStr = projectName;
   
   // get index of last '/' before actual project name
   int lastSlash = tempStr.find_last_of ('/');
   if ( lastSlash < 0 || lastSlash > tempStr.size() )
   {
      lastSlash = 0;
   }
   
   // remove string up to last '/'
   tempStr = tempStr.substr (lastSlash, tempStr.size() - lastSlash);
   
   // only save string before .project3d
   tempStr = tempStr.substr (0, tempStr.find_last_of ('.'));
    
   // convert string to char*
   char *retString = new char[tempStr.size()];
   strcpy (retString, tempStr.c_str());
   
   return retString;
}








