#ifndef INVERSION_PROJECTRESULTSREADER_H
#define INVERSION_PROJECTRESULTSREADER_H

#include <string>
#include <vector>
#include <memory>

#include "formattingexception.h"

namespace DataAccess
{
   namespace Interface 
   {
      class Grid;
   }
   namespace Mining
   {
      class ProjectHandle;
      class DomainPropertyFactory;
      class CauldronDomain;
      class DomainPropertyCollection;
   }
}

class ProjectResultsReader
{
public:
   struct Exception : formattingexception::BaseException< Exception > { };

   ProjectResultsReader( const std::string & projectFile );

   /// Probes a Cauldron project3d file. 
   /// This function return the results from the simulation in the output vector 'results'.
   void read(const std::string & propertyName,  // a physical quantity
             double snapshotTime,               // at which age
             double x, double y,                // horizontal coordinates
             const std::vector<double> & zs,    // series of depths
             std::vector< double > & results ) const;

private:
   ProjectResultsReader( const ProjectResultsReader & ); // prohibit copying 
   ProjectResultsReader operator=( const ProjectResultsReader & ); // prohibit assignment

   class SwitchDAFactory;

   std::auto_ptr<DataAccess::Mining::DomainPropertyFactory> m_factory;
   std::auto_ptr<DataAccess::Mining::ProjectHandle> m_project;
   std::auto_ptr<DataAccess::Mining::CauldronDomain> m_domain;
   DataAccess::Mining::DomainPropertyCollection * m_domainProperties;
   const DataAccess::Interface::Grid * m_grid;
};

#endif
