#ifndef EXPORTWIZARD_H
#define EXPORTWIZARD_H
#include "exportwizardbase.h"
#include "infobase.h"

#include <vector>
#include <string>


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

namespace BPA
{
   class CauldronProject;
}

class ProjectView;

class ExportWizard : public ExportWizardBase, public InfoBase
{
    Q_OBJECT

public:
    ExportWizard( DataAccess::Interface::ProjectHandle * projectHandle, ProjectView * parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ExportWizard();

   OwnerOrgInfo * getOwnerOrgInfoSelected (void);
   const BPA::CoordinateSystem * getCoordinateSystemSelected (void);
   const BPA::GeoTransform * getGeoTransformSelected (void);
   std::vector<const BPA::Basin *> * getBasinsSelected (void);

   ProjectView * getProjectView (void);

   std::string getName ();
   std::string getDescription ();


signals:
    void finished();

public slots:
    void startExport();
    void startReexport (std::string ownerOrgName, std::string projectName);
    void finishExport();

    void accept();
    void reject();

protected slots:

   virtual ListSelectionStatus displayOwnerOrgs (void);
   virtual ListSelectionStatus displayRegions (void);
   virtual ListSelectionStatus displayCountries (void);
   virtual ListSelectionStatus displayBasins (void);
   virtual ListSelectionStatus displayCoordinateSystems (void);
   virtual ListSelectionStatus displayGeoTransforms (void);

   void highlightCoordinateSystem (const QString & partialName);
   void highlightGeoTransform (const QString & partialName);

   void evaluateNamePage ();
   void displayBasinSelection (void);
   void displayCoordinateSystemSelection (void);
   void displayGeoTransformSelection (void);

protected:
   DataAccess::Interface::ProjectHandle * m_projectHandle;

private:
   ProjectInfo * m_projectInfo;

   bool m_highlightCS;
   bool m_highlightGT;
};

#endif // EXPORTWIZARD_H
