#ifndef ResourceLimits_H
#define ResourceLimits_H _ResourceLimits_H

#include <qstring.h> 


class ResourceLimits
{
    public: 
        ResourceLimits( QString & );
        ~ResourceLimits();

        long int GetStatus() const;
        long int GetTotalCores() const;
        long int GetMemoryPerCore() const;

	void CreateLimits();

	enum Status 
	  {
	    HAVE_VALUES,    // startup was successful, have lim it values
	    CREATE_LIMITS,  // need to collect values
	    CREATE_FAILED,  // gempis was unsuccessful in collecting limit values
	  };

    public slots:

    private:
	void readFile( void );
	void readComputer( void );

	bool m_defined;         // true = have memory required value
	bool m_createFailed;    // true = have memory required value
	int m_error;
	long int m_totalCores;
 	long int m_memoryPerCore;
	QString m_resourceName;
	QString m_resourceFileName;
	QString m_jobtype;

    private slots:
    
    protected:
};

#endif
