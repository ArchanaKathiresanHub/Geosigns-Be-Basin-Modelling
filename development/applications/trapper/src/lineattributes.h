/// Class LineAttributesList holds a list of LineAtributes, from which
/// LineAttributes can be retrieved
/// Struct LineAttributes holds color and width descriptions for a graph line

#ifndef __lineattributes__
#define __lineattributes__

#include <qcolor.h>

#include <string>
#include <map>
#include <vector>
using namespace std;

typedef struct LineAttributes
{
   LineAttributes (QColor c, int w) 
      : colour (c), width (w) {}
   
   QColor colour;
   int width;
} LineAttributes;

typedef map<string, LineAttributes> LineAttributeMap;
typedef vector<QColor> ColourList;

class LineAttributesList
{
public:
   static LineAttributesList* getList ();
   const LineAttributes* getLineAttributes (const char *lineName);
   const QColor& getGeneralColour ();
   void resetGeneralColourIndex () { m_generalColourIndex = -1; }
   
private:
   LineAttributesList ();
   ~LineAttributesList () {}
   void createLegendList ();
   void createGeneralColourList ();
   
   LineAttributeMap m_map;
   ColourList m_generalColours;
   short m_generalColourIndex;
};

#endif
