#include "case.h"
#include "DatadrillerProperty.h"
#include <string>
#include <iostream>

Case::Case(const std::vector<Parameter*> & values)// const std::string & templateProjectFilename,
: m_values(values)
, m_workingProjectFile("not defined")
, m_outputDataFile("not defined")
, results(0)
{
}


Case::Case()
: m_values()
, m_workingProjectFile()
, m_outputDataFile()
, results()
{
}

/*
Case::Case(const Case & C)
: m_values()
, m_workingProjectFile()
, m_outputDataFile()
, results()
{
}
*/


void Case::addParameter(Parameter * pt_parameter)
{
  m_values.push_back( pt_parameter);
}


void Case::addVariableToDrill(const DatadrillerProperty & OneVariableDef )
{
  m_VariablesToDrill.push_back( OneVariableDef.getName() );
}


void Case::set_ProjectFile(const std::string & filename1)
{
  m_workingProjectFile = filename1;
}

void Case::set_ResultsFile(const std::string & filename2)
{
  m_outputDataFile = filename2;
}

void Case::create_project_file(const std::string & input, const std::string & output)
{
  Project project( input, output);

  std::cout << output << std::endl;

  for (unsigned i = 0; i < m_values.size(); ++i)
  {
    m_values[i]->Change_parameter(project);
  }

  project.close();
//  project.~Project();
//  setBasementProperty(const std::string & parameter, double newValue);
//  project.setBasementProperty(values[i], x2);
}



void Case::Define_location_to_drill(const DatadrillerProperty & PropertyDrilled)
{
  
double Start = PropertyDrilled.getPositionBegZ();
double End = PropertyDrilled.getPositionEndZ();
double Step = PropertyDrilled.getStepZ();

double current_depth = Start; 

zs.clear();

while(current_depth <= End)
{
  zs.push_back(current_depth);
  current_depth += Step;
}

xs=PropertyDrilled.getPositionX();
ys=PropertyDrilled.getPositionY();
ts=PropertyDrilled.getTime();

}


/// Probes a Cauldron project3d file. Given:
///  - a physical quantity, as 'propertyName',
///  - a snapshot time,
///  - horizontal coordinates, x and y, and
///  - a series of depths, in zs,
///  this function return the results from the simulation in the output vector 'results'.
void Case::readOnePropertyProjectFile(const std::string & propertyName, int Property_Iterator, double snapshotTime, double x, double y)
{


  // clear the result vector

//  results.clear();

  const std::string project = m_workingProjectFile;

  // Open project file
  DataAccess::Mining::DomainPropertyFactory factory;
  DataAccess::Interface::ProjectHandle::UseFactory( &factory );

  DataAccess::Mining::ProjectHandle* projectHandle 
    = dynamic_cast< DataAccess::Mining::ProjectHandle * >(
	DataAccess::Interface::OpenCauldronProject(project, "r")
      );

  if (!projectHandle)
    throw ProbeException() << "Could not load project file '" << project << "'";

  // Load property
  const DataAccess::Interface::Property* property = projectHandle->findProperty (propertyName);
  if (!property) 
    throw ProbeException() << "Unknown PropertyName value: " << propertyName;

  // Load snapshot
  if (snapshotTime < 0) 
    throw ProbeException() << "Illegal snapshot time: " << snapshotTime;
  const DataAccess::Interface::Snapshot * snapshot = projectHandle->findSnapshot (snapshotTime);
  DataAccess::Mining::CauldronDomain domain ( projectHandle );
  domain.setSnapshot (snapshot);
  DataAccess::Mining::DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();
  domainProperties->setSnapshot (snapshot);
	
  // Check whether the x and y are in the grid
  unsigned int a, b;
  const DataAccess::Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid ();
  if (!grid->getGridPoint (x, y, a, b)) 
    throw ProbeException() << "Illegal (XCoord, YCoord) pair: (" << x << ", " << y << ")";
       
  // Retrieve the results
  for (size_t i = 0; i < zs.size(); ++i)
  {
    DataAccess::Mining::ElementPosition element;
    if (!domain.findLocation (x, y, zs[i], element))
      throw ProbeException() << "Illegal point coordinates: " << x << ", " << y << ", " << zs[i];

    results[Property_Iterator].push_back( domainProperties->getDomainProperty (property)->compute (element) );
  }

}



void Case::readProjectFile()
{

/*  for (int i = 0; i < 5; ++i )  // To be changed 
  {
    zs.push_back(double(i) * 100.0);
  }*/

  results.resize( m_VariablesToDrill.size() );

/*  for (int i = 0; i < m_VariablesToDrill.size(); ++i )
  {
    results[i].resize(zs.size());
  }
*/



  for(size_t i = 0; i < m_VariablesToDrill.size(); ++i)
  {
    readOnePropertyProjectFile( m_VariablesToDrill[i], i, ts, xs, ys );
  }
}



void Case::display_results() const
{
 std::cout << "Output data are being saved..." << std::endl;;
 std::ofstream ofs( (m_outputDataFile + ".dat").c_str(), ios_base::out | ios_base::trunc );

  ofs << "Datamining from project " << m_workingProjectFile << " :"<< endl;
  for (size_t i = 0; i < results.size(); ++i)
  {
    ofs << m_VariablesToDrill[i] << " ";
    for (size_t j = 0; j < results[i].size(); ++j)
    {
      ofs << results[i][j] << " ";
    }
    ofs  << endl;
  }
}



void Case::display_Parameters() const
{
  std::cout << "Displaying parameters: " << std::endl;
  for (int i = 0; i < m_values.size(); ++i)
  {
    m_values[i]->print();
  }
}


