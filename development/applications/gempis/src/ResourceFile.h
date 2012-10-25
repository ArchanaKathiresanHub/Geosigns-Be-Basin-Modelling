#ifndef ResourceFile_H
#define ResourceFile_H _ResourceFile_H

#include <qstring.h>
#include <qstringlist.h>

#include <vector>
using namespace std;


struct t_resourceEntry
{
  t_resourceEntry( )
  { 
    res_name    = "";
    res_jobtype = "";
    res_mpiref  = "";
    res_remote  = "";
    res_request = "";
    res_submit  = "";
    res_prepath = "";
    res_cluster = "";
  }

  t_resourceEntry( const t_resourceEntry & input )
  { 
    res_name    = input.res_name;
    res_jobtype = input.res_jobtype;
    res_mpiref  = input.res_mpiref;
    res_remote  = input.res_remote;
    res_request = input.res_request;
    res_submit  = input.res_submit;
    res_prepath = input.res_prepath;
    res_cluster = input.res_cluster;
  }

  QString res_name;    // (required) Resource name as used in the gempis 
                       // command.
  QString res_jobtype; // (required) Job Type: STANDALONE, LSF, or LSFHPC, 
                       // or CUSTOM.
  QString res_mpiref;  // (required) MPI Reference name: refers to an entry 
                       // in the gempis_MPIRef file.
  QString res_remote;  // Name of remote computer to submit the gempis 
                       // command.
  QString res_request; // Identifier to use when requesting resource 
                       // information. 
  QString res_submit;  // Identifier to use when submitting a job.
  QString res_prepath; // Prepend directory list to PATH (separate entries
                       // using the 
                       // colon character ':' as used in the PATH)
  QString res_cluster; // Cluster list: a list of computer names and number 
                       // of processors when Job Type is CUSTOM.
};

#define NUM_RESOURCE_OFFSET 21  // 1st valid index for user references

class ResourceFile
{
    public: 
        ResourceFile();
        ~ResourceFile();

	void LoadResources( bool );
	void SaveResources();

	QStringList GetResourceList();
	QStringList GetJobTypes();
	QStringList GetMPIReferences();

	void GetResourceEntry( QString &, t_resourceEntry * );
	void ModifyResourceEntry( QString &, t_resourceEntry & );
	void AddResourceEntry( t_resourceEntry & );
	void DeleteResourceEntry( QString & );

    private:
	int getIndex( QString );
	void readJobTypesFile( QString );
	void readJobTypes();
	void readMpiRefsFile( QString );
	void readMpiRefs();
	void resetRETemp();
        void readDefinitionsFile( QString );
      	bool readResourceFile( QString );
      	void readResources( bool );
	void writeResourceFile( QString );
	void writeResources();
	void parseResourceLine( QString & );

	// data variables
	QString m_RegionName;
	QStringList m_ResourceNames;
	QStringList m_JobTypes;
	QStringList m_MpiRefs;
	t_resourceEntry m_RETemp;
	vector<t_resourceEntry> m_resourceEntry;
};

#endif // ifndef ResourceFile_H
