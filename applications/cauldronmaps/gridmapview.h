#ifndef GRIDMAPVIEW_H
#define GRIDMAPVIEW_H
#include "gridmapviewbase.h"

// #define NOSCROLLEDZOOM

class ScrollZoomer;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtDoublePoint;

namespace DataAccess
{
   namespace Interface
   {
      class GridMap;
   }
}

class GridMapView : public GridMapViewBase
{
    Q_OBJECT

public:
    GridMapView( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~GridMapView();

    void setGridMap( const DataAccess::Interface::GridMap * gm );
    void setTitle (const QString & title);
    void showTitle ();
    void hideTitle ();

protected:
    virtual void setupZooming (void);
    virtual void setupMouseTracking (void);

protected slots:
    virtual void displayPoint (const QwtDoublePoint &);

private:
   const DataAccess::Interface::GridMap * m_gridMap;

#ifdef NOSCROLLEDZOOM
    QwtPlotZoomer * m_zoomBL;
#else
    ScrollZoomer * m_zoomBL;
#endif
    QwtPlotPicker * m_plotPicker;


};

#endif // GRIDMAPVIEW_H
