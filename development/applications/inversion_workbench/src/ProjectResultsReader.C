#include "ProjectResultsReader.h"

#include "formattingexception.h"
#include "Interface/ProjectHandle.h"
#include "DataMiningProjectHandle.h"
#include "CauldronDomain.h"

// A class that replaces the DataAccess ObjectFactory during the scope of a block
class ProjectResultsReader :: SwitchDAFactory
{
public:
   SwitchDAFactory( DataAccess::Mining::DomainPropertyFactory & newFactory)
      : m_oldFactory( DataAccess::Interface::ProjectHandle::GetFactoryToUse() )
      , m_newFactory( & newFactory )
   {
      DataAccess::Interface::ProjectHandle::UseFactory( m_newFactory );
   }

   ~SwitchDAFactory()
   {
      DataAccess::Interface::ProjectHandle::UseFactory( m_oldFactory );
   }

private:
   SwitchDAFactory( const SwitchDAFactory & ); // copying prohibited
   SwitchDAFactory & operator=( const SwitchDAFactory & ); // assignment prohibited

   DataAccess::Interface::ObjectFactory * m_oldFactory;
   DataAccess::Interface::ObjectFactory * m_newFactory;
};

ProjectResultsReader :: ProjectResultsReader( const std::string & projectFile )
   : m_factory( new DataAccess::Mining::DomainPropertyFactory )
   , m_project()
   , m_domain()
   , m_domainProperties(0  )                              
   , m_grid(0)
{
   SwitchDAFactory _(*m_factory);

   m_project.reset( dynamic_cast< DataAccess::Mining::ProjectHandle * >(
                  DataAccess::Interface::OpenCauldronProject(projectFile, "r")
              ) );

   if (!m_project.get())
      throw Exception() << "Could not load project file '" << projectFile << "'";

   m_domain.reset( new DataAccess::Mining::CauldronDomain( m_project.get() ));
   m_domainProperties = m_project->getDomainPropertyCollection();
   m_grid = m_project->getLowResolutionOutputGrid();
}
  
void ProjectResultsReader :: read( const std::string & propertyName, 
      double snapshotTime, double x, double y, const std::vector<double> & zs,
      std::vector< double > & results ) const
{
   SwitchDAFactory _(*m_factory);

   // clear the result vector
   results.clear();

   const DataAccess::Interface::Property* property = m_project->findProperty(propertyName);
   if (!property) 
      throw Exception() << "Unknown PropertyName value: " << propertyName;

   // Load snapshot
   if (snapshotTime < 0) 
      throw Exception() << "Illegal snapshot time: " << snapshotTime;
   const DataAccess::Interface::Snapshot * snapshot = m_project->findSnapshot(snapshotTime);
   m_domain->setSnapshot (snapshot);
   m_domainProperties->setSnapshot (snapshot);

   // Check whether the x and y are in the grid
   unsigned int a, b;
   if (!m_grid->getGridPoint (x, y, a, b)) 
      throw Exception() << "Illegal (XCoord, YCoord) pair: (" << x << ", " << y << ")";

   // Retrieve the results
   for (size_t i = 0; i < zs.size(); ++i)
   {
      DataAccess::Mining::ElementPosition element;
      if (!m_domain->findLocation(x, y, zs[i], element))
         throw Exception() << "Illegal point coordinates: " << x << ", " << y << ", " << zs[i];

      results.push_back( m_domainProperties->getDomainProperty (property)->compute (element) );
   }

}
