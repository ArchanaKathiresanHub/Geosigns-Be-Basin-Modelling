/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#define _CXXTEST_HAVE_EH
#define CXXTEST_TRAP_SIGNALS
#define _CXXTEST_ABORT_TEST_ON_FAIL
#define CXXTEST_TRACE_STACK
#define CXXTEST_STACK_TRACE_EXE "Debug/multi_array.a"
#define CXXTEST_XML_OUTPUT

#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/XmlStdioPrinter.h>

#ifdef CXXTEST_TRACE_STACK
#include <symreader.h>
#ifdef CXXTEST_INCLUDE_SYMREADER_DIRECTLY
#include <symreader.c>
#endif
#endif
typedef const CxxTest::SuiteDescription *SuiteDescriptionPtr;
typedef const CxxTest::TestDescription *TestDescriptionPtr;

#include "unit_tests/TestMultiArray.h"
static TestMultiArray *suite_TestMultiArray;

static CxxTest::List Tests_TestMultiArray;
CxxTest::StaticSuiteDescription suiteDescription_TestMultiArray;

static class TestDescription_TestMultiArray_test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray : public CxxTest::RealTestDescription {
public:
 void runTest() { if (suite_TestMultiArray) suite_TestMultiArray->test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray(); }
} testDescription_TestMultiArray_test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray;

namespace CxxTest {
 void initialize()
 {
  Tests_TestMultiArray.initialize();
_TS_TRY_WITH_SIGNAL_PROTECTION {
  _TS_TRY {
    suite_TestMultiArray = new TestMultiArray;
  } _TS_CATCH_ABORT( {} )
_TS_LAST_CATCH( { addSuiteToFailures("TestMultiArray", "Exception thrown when initializing TestMultiArray"); })
} _TS_CATCH_SIGNAL({
  addSuiteToFailures("TestMultiArray", CxxTest::__cxxtest_sigmsg);
});

  suiteDescription_TestMultiArray.initialize( "unit_tests/TestMultiArray.h", 20, "TestMultiArray", *suite_TestMultiArray, Tests_TestMultiArray );
  testDescription_TestMultiArray_test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray.initialize( Tests_TestMultiArray, suiteDescription_TestMultiArray, 24, "test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray" );
 }

 void cleanup()
 {
    delete suite_TestMultiArray;
 }
}
#define CHKPTR_BASIC_HEAP_CHECK

#include <cxxtest/Root.cpp>
#define MW_STACK_TRACE_INITIAL_PREFIX CXXTEST_STACK_TRACE_INITIAL_PREFIX
#define MW_STACK_TRACE_OTHER_PREFIX CXXTEST_STACK_TRACE_INITIAL_PREFIX

namespace CxxTest
{

void __cxxtest_chkptr_error_handler(bool fatal, const char* msg) _CXXTEST_NO_INSTR;

void __cxxtest_chkptr_error_handler(bool fatal, const char* msg)
{
	char text[256];

	if(fatal)
		sprintf(text, "Pointer error: %s", msg);
	else
		strncpy(text, msg, 256);

	if(fatal)
	{
#ifdef CXXTEST_TRAP_SIGNALS
		__cxxtest_assertmsg = text;
#else
		printf("%s\n", text);
#endif
		abort();
	}
	else
	{
		std::string finalMsg = text;

#ifdef CXXTEST_TRACE_STACK
    {
        std::string trace = CxxTest::getStackTrace(__cxxtest_runCompleted);
        if ( trace.length() )
        {
            finalMsg += "\n";
            finalMsg += trace;
        }
    }
#endif

		if(!__cxxtest_runCompleted)
		{
			CxxTest::doWarn("", 0, finalMsg.c_str());
		}
		else
		{
			printf("Warning: %s\n", finalMsg.c_str());
		}
	}
}

} // namespace CxxTest

namespace CxxTest
{
	
class xml_chkptr_reporter : public ChkPtr::chkptr_reporter
{
private:
	FILE* xmlFile;
	int totalBytesAllocated;
	int maxBytesInUse;
	int numCallsToNew;
	int numCallsToArrayNew;
	int numCallsToDelete;
	int numCallsToArrayDelete;
	int numCallsToDeleteNull;

public:
	xml_chkptr_reporter(const char* path)
	{
		xmlFile = fopen(path, "w");
	}
	
	virtual void beginReport(int* tagList)
	{
		int numLeaks = 0;

		while(*tagList != CHKPTR_REPORT_END)
		{
			int tag = *tagList++;
			int value = *tagList++;
			
			switch(tag)
			{
				case CHKPTR_REPORT_NUM_LEAKS:
					numLeaks = value;
					break;
				
				case CHKPTR_REPORT_TOTAL_BYTES_ALLOCATED:
					totalBytesAllocated = value;
					break;
					
				case CHKPTR_REPORT_MAX_BYTES_IN_USE:
					maxBytesInUse = value;
					break;
					
				case CHKPTR_REPORT_NUM_CALLS_NEW:
					numCallsToNew = value;
					break;
	
				case CHKPTR_REPORT_NUM_CALLS_ARRAY_NEW:
					numCallsToArrayNew = value;
					break;
	
				case CHKPTR_REPORT_NUM_CALLS_DELETE:
					numCallsToDelete = value;
					break;
	
				case CHKPTR_REPORT_NUM_CALLS_ARRAY_DELETE:
					numCallsToArrayDelete = value;
					break;
	
				case CHKPTR_REPORT_NUM_CALLS_DELETE_NULL:
					numCallsToDeleteNull = value;
					break;
			}
		}

		fprintf(xmlFile, "<?xml version='1.0'?>\n");
		fprintf(xmlFile, "<memwatch actual-leak-count=\"%d\">\n", numLeaks);
	}
		
	virtual void report(const void* address, size_t size,
		const char* filename, int line)
	{
		fprintf(xmlFile, "    <leak address=\"%p\" size=\"%lu\">\n",
			address, (unsigned long)size);

#ifdef CXXTEST_TRACE_STACK
		fprintf(xmlFile,
			getStackTrace(false, CHKPTR_STACK_WINDOW_SIZE,
				(CxxTest::StackElem*)(((char*)address) + size)).c_str() );
#endif

		fprintf(xmlFile, "    </leak>\n");
	}
	
	virtual void reportsTruncated(int numReports, int actualCount)
	{
	}

	virtual void endReport()
	{
		fprintf(xmlFile, "    <summary "
			"total-bytes-allocated=\"%d\" max-bytes-in-use=\"%d\" "
			"calls-to-new=\"%d\" calls-to-array-new=\"%d\" "
			"calls-to-delete=\"%d\" calls-to-array-delete=\"%d\" "
			"calls-to-delete-null=\"%d\"" 
			"/>\n",
			totalBytesAllocated, maxBytesInUse, numCallsToNew,
			numCallsToArrayNew, numCallsToDelete, numCallsToArrayDelete,
			numCallsToDeleteNull);

		fprintf(xmlFile, "</memwatch>\n");
	}
};

}
#include <setjmp.h>	   // for siglongjmp()
#include <stdlib.h>	   // for exit()

void __cxxtest_sig_handler( int, siginfo_t*, void* ) _CXXTEST_NO_INSTR;

void __cxxtest_sig_handler( int signum, siginfo_t* info, void* /*arg*/ )
{
    const char* msg = "run-time exception";
    switch ( signum )
    {
        case SIGFPE:
            msg = "SIGFPE: floating point exception (div by zero?)";
	        // Currently, can't get cygwin g++ to pass in info,
            // so we can't be more specific.
	        break;

        case SIGSEGV:
            msg = "SIGSEGV: segmentation fault (null pointer dereference?)";
            break;

        case SIGILL:
            msg = "SIGILL: illegal instruction "
                "(dereference uninitialized or deleted pointer?)";
            break;

        case SIGTRAP:
            msg = "SIGTRAP: trace trap";
            break;

#ifdef SIGEMT
        case SIGEMT:
            msg = "SIGEMT: EMT instruction";
            break;
#endif

        case SIGBUS:
            msg = "SIGBUS: bus error "
                "(dereference uninitialized or deleted pointer?)";
            break;

        case SIGSYS:
            msg = "SIGSYS: bad argument to system call";
            break;

        case SIGABRT:
            msg = "SIGABRT: execution aborted "
                "(failed assertion, corrupted heap, or other problem?)";
            break;

        case SIGALRM:
            msg = "SIGALRM: allotted time expired";
            break;
    }

#ifdef CXXTEST_CREATE_BINARY_LOG
	executionLog.setLastResult(signum, CxxTest::__cxxtest_assertmsg);
#endif

    if ( !CxxTest::__cxxtest_assertmsg.empty() )
    {
        CxxTest::__cxxtest_sigmsg = CxxTest::__cxxtest_assertmsg;
        CxxTest::__cxxtest_assertmsg = "";
    }
    else if ( CxxTest::__cxxtest_sigmsg.empty() )
    {
        CxxTest::__cxxtest_sigmsg = msg;
    }
    else
    {
        CxxTest::__cxxtest_sigmsg = std::string(msg)
	        + ", maybe related to " + CxxTest::__cxxtest_sigmsg;
    }

#ifdef CXXTEST_TRACE_STACK
    {
        std::string trace = CxxTest::getStackTrace(CxxTest::__cxxtest_jmppos < 0);
        if ( trace.length() )
        {
            CxxTest::__cxxtest_sigmsg += "\n";
            CxxTest::__cxxtest_sigmsg += trace;
        }
    }
#endif

    if ( CxxTest::__cxxtest_jmppos >= 0 )
    {
        siglongjmp( CxxTest::__cxxtest_jmpbuf[CxxTest::__cxxtest_jmppos], 1 );
    }
    else
    {
        std::cout << "\nError: untrapped signal:\n"
	        << CxxTest::__cxxtest_sigmsg
            << "\n"; // std::endl;
		exit(1);
    }
}
class SignalRegistrar
{
public:
	SignalRegistrar() {
#ifdef CXXTEST_TRACE_STACK
        symreader_initialize(CXXTEST_STACK_TRACE_EXE, SYMFLAGS_DEMANGLE);
#endif
        ChkPtr::__manager.setErrorHandler(&CxxTest::__cxxtest_chkptr_error_handler);

        struct sigaction act;
        act.sa_sigaction = __cxxtest_sig_handler;
        act.sa_flags = SA_SIGINFO;
        sigaction( SIGSEGV, &act, 0 );
        sigaction( SIGFPE,  &act, 0 );
        sigaction( SIGILL,  &act, 0 );
        sigaction( SIGBUS,  &act, 0 );
        sigaction( SIGABRT, &act, 0 );
        sigaction( SIGTRAP, &act, 0 );
#ifdef SIGEMT
        sigaction( SIGEMT,  &act, 0 );
#endif
        sigaction( SIGSYS,  &act, 0 );
        sigaction( SIGALRM, &act, 0 );
	}
};
SignalRegistrar __signal_registrar __attribute__((init_priority(101)));

int main() {
 ChkPtr::__manager.setReportAtEnd(true);
 ChkPtr::__manager.setErrorHandler(&CxxTest::__cxxtest_chkptr_error_handler);
 ChkPtr::__manager.setReporter(new CxxTest::xml_chkptr_reporter(".memwatch.log"), true);
 CxxTest::initialize();
 FILE* resultsFile = fopen(".cxxtest.log", "w");
 int exitCode = CxxTest::XmlStdioPrinter(resultsFile).run();
 fclose(resultsFile);
 CxxTest::__cxxtest_runCompleted = true;
 CxxTest::cleanup();
 return exitCode;
}
