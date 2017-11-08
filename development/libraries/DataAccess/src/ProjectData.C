#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"

#include "Interface/ProjectData.h"

using namespace DataAccess;
using namespace Interface;

ProjectData::ProjectData ( ProjectHandle * projectHandle, database::Record* record ) :  DAObject (projectHandle, record) {
}

ProjectData::~ProjectData (void)
{
}

const std::string& ProjectData::getProjectName () const {
   return database::getProjectName ( m_record );
}

const std::string& ProjectData::getDescription () const {
   return database::getDescription ( m_record );
}

const std::string& ProjectData::getProgramVersion () const {
   return database::getProgramVersion ( m_record );
}

double ProjectData::getXOrigin () const {
   return database::getXCoord ( m_record );
}

double ProjectData::getYOrigin () const {
   return database::getYCoord ( m_record );
}

double ProjectData::getDeltaX () const {
   return database::getDeltaX ( m_record );
}

double ProjectData::getDeltaY () const {
   return database::getDeltaY ( m_record );
}

int ProjectData::getNumberOfXNodes () const {
   return database::getNumberX ( m_record );
}

int ProjectData::getNumberOfYNodes () const {
   return database::getNumberY ( m_record );
}

int ProjectData::getXNodeStep () const {
   return database::getScaleX ( m_record );
}

int ProjectData::getYNodeStep () const {
   return database::getScaleY ( m_record );
}

int ProjectData::getXNodeOffset () const {
   return database::getOffsetX ( m_record );
}

int ProjectData::getYNodeOffset () const {
   return database::getOffsetY ( m_record );
}

int ProjectData::getWindowXMin () const {
   return database::getWindowXMin ( m_record );
}

int ProjectData::getWindowXMax () const {
   return database::getWindowXMax ( m_record );
}

int ProjectData::getWindowYMin () const {
   return database::getWindowYMin ( m_record );
}

int ProjectData::getWindowYMax () const {
   return database::getWindowYMax ( m_record );
}
