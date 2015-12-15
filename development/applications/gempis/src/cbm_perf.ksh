#! /bin/bash
#
# Analyzes performance between two instances of the same suite of tests 
# in a performance report
#
# Syntax:
# perf_analyze <ReferencePath> <ComparePath> <CompareType> [<option>]
#

if [ -z "$GEMPIS_PERF_HOME" ]; then
    echo "Error! You need to setup the performance environment first."
    echo "type: . ./setup in the performance root directory."
    exit 1
fi

export SUITEDIR=$GEMPIS_PERF_HOME/tests

#
# define gempis utilities
#
# This definition is used througout gempis to locate gempis system definitions
. $GEMPIS_EXEDIR/gempis_utils


# define the format to use for date
dateformat='+%Y.%m.%d-%H:%M:%S'
datestamp="`date $dateformat`"

############################################################
#
# displays how to use this script and exists
#
usage()
{
    echo "Usage: `basename $0` run <Resource> <TestCaseName> <TestSuite>" 
    echo "                evaluate <LogFileName>" 
    echo "                report <TestCaseName>"
    echo "                speedup <TestCaseName>"
    echo "                compare <ReferenceTestCase> <CompareTestCase>"
    echo ""
    exit 1
}

############################################################
#
# determines if the run was successful or not
#
run_testcase()
{
    # get parameters
    resource=$1
    testcase=$2
    suite=$3

    # check parameters
    [ -z "$resource" ] && usage
    if [ -z "$suite" ]; then
	echo "Defined test suites are:"
	cd $SUITEDIR/
	ls *.suite | awk -F '.' '{ print $1 }'
#	ls -l | grep "d" | awk '{ print $9 }'
	exit 1
    fi

    # run the test
    gempis perf $resource $testcase $suite 

    # generate a report of the test
    cbm_perf report $testcase > $PERF_RESULTS/$testcase.report

    # pack up the results
    cwd=`pwd`
    cd $PERF_RESULTS
    tar cfz $cwd/$testcase.gz ${testcase}
    cd $cwd

    # send everything off to adminstrator
    uuencode $testcase.gz testcase.tar.gz | $GEMPIS_MAIL -s "Results of run for testcase $testcase" $GEMPIS_EMAIL_ADMINISTRATOR
}

############################################################
#
# determines if the run was successful or not
#
evaluate_run()
{
    # get parameters
    LogFileName=$1

    echo "`date $dateformat` ====== Evaluating log file [$LogFileName]" | tee -a $JOURNAL_FILE

    EvalFileName=$OUTPUT_DIR/${TEST}.cbm

    #
    # !!! There is a problem here. This will not always be so
    #    scan for the -project options and use the next one
    donext=""
    ProjectName=`
for word in $COMMAND; do
    if [ -n "$donext" ]; then
        echo "$word"
        break
    fi
    if [ $word = "-project" ]; then
        donext="yes"
    fi   
done | awk -F'.' '{ print $1 }'`


echo $ProjectName

    # check for fastcauldron 
    line=`cat $LogFileName | grep "Calculation:"`
    if [ -n "$line" ]; then
	# output format is:
	# "Decompaction Calculation: 0 Hrs. 0 Mins. 1 Sec."
	# "Temperature Decompaction Calculation: 0 Hrs. 0 Mins. 1 Sec."

	field=`echo "$line" | awk '{ print $1 }'`

	# the format of itcoupled output is different
	if [ "$field" = "P/T" ]; then
	    # P/T xxx Calculation: 0 Hrs. 0 Mins. 1 Sec.
	    time=`echo "$line" | awk '{ printf( "%02d:%02d:%02d\n", $4, $6, $8) }'`
	else
	    time=`echo "$line" | awk '{ printf( "%02d:%02d:%02d\n", $3, $5, $7) }'`
	fi
    fi

    # check for genex5
    if [ -z "$line" ]; then
	line=`cat $LogFileName | grep "Simulation Time:"`
	time=`echo "$line" | awk '{ printf( "%02d:%02d:%02d\n", $3, $5, $7) }'`
    fi

    # check for fastmig
    if [ -z "$line" ]; then
	line=`cat $LogFileName | grep "o Migration:: Finished"`
	time=`echo "$line" | awk '{ printf( "00:%s\n", $6) }'`
    fi

    # output result
    if [ -z "$line" ]; then
	evaluate="Failed"
    else
	evaluate="Success"

        # save time
	size=`du --si *_CauldronOutputDir | awk '{ print $1 }'`
    fi

    echo "`date $dateformat` ====== Evaluation was $evaluate in $time creating $size of data"  | tee -a $JOURNAL_FILE

    # write a line into the Evaluate log
    echo "Resource:$RESOURCE Project:$ProjectName Test:$TEST Trial:$TRIAL Cores:$CORES Evaluate:$evaluate Time:$time DiskSize:$size" >> $EvalFileName

    if [ "$evaluate" = "Success" ]; then
	return 0
    fi
    return 1
}


############################################################
#
# compares two results files
#
get_average()
{
    testcase=$1
    trial=$2
    cores=$3

    # line is in format: 
    # "count min average max"
    cat $PERF_RESULTS/$testcase/*.cbm | grep "Trial:$trial Cores:$cores Evaluate:Success" | 
awk '{ print $7 }' | awk -F':' '
BEGIN {
   total = 0
   count = 0
}
{
    val = ($2 * 3600) + ($3 * 60) + $4
    total = total + val
    count = count + 1
}
END { 
    if ( count == 0 )
       printf( "0\n" )
    else
       printf( "%d\n", total / count )
}'
}


############################################################
#
# speedup of a single trial
#
#
speedup_trial()
{
    trial=$1

    first_time=""
    for cores in $core_list; do

	# line is in format: 
	# "count min average max"
	avg=`get_average $testcase $trial $cores`
	if [ $avg = "0" ]; then
	    speedup[$cores]="xxx"
	    continue
	fi
	if [ -z "$first_time" ]; then
	    let 'first_time = avg * cores'
#	    first_time=$avg
	fi
#	let 'avg = avg * cores'

        # compute speedup
        speedup[$cores]=`echo "$first_time $avg" | awk '{ print $1 / $2 }'`
    done


    # print data
    printf "%24s " $trial 
    for cores in $core_list; do
	if [ ${speedup[$cores]} = "xxx" ]; then
	    printf "%7s " ""
	else
	    printf "%7.3f " ${speedup[$cores]} 
	fi
    done
    echo ""
}


############################################################
#
speedup()
{
    testcase=$1

    # check parameters
    if [ -z "$testcase" ]; then
	echo "Defined test cases are:"
	cd $PERF_RESULTS
	ls -l | grep "^d" | awk '{ printf( "    %s\n", $9) }'
	exit 1
    fi

    echo "                  CBM Performance Speedup"
    echo "                  ======================="
    echo ""
    echo "Computing speedups of $testcase"
    echo ""

    trial_list=`cat $PERF_RESULTS/$testcase/*.cbm | awk '{ print $4 }'| awk -F':' '{ print $2 }' | sort -u`
    core_list=`cat $PERF_RESULTS/$testcase/*.cbm | awk '{ print $5 }'| awk -F':' '{ print $2 }' | sort -gu`

    # print header
    printf "%24s " "Speedup" 
    for cores in $core_list; do
	printf "%7s " $cores
    done
    echo ""

    # compute averages for all data in the reference file
    for trial in $trial_list; do
       
	speedup_trial $trial

    done 
}



############################################################
#
# compares two results files
#
#
compare_trial()
{
    trial=$1

    for cores in $core_list; do

	# line is in format: 
	# "count min average max"
	ref=`get_average $ReferencePath $trial $cores`
	if [ $ref = "0" ]; then
	    efficency[$cores]="xxx"
	    continue
	fi

	cmp=`get_average $ComparePath $trial $cores`
	if [ $cmp = "0" ]; then
	    efficency[$cores]="xxx"
	    continue
	fi

        # compute efficiency
        efficency[$cores]=`echo "$ref $cmp" | awk '{ print $1 / $2 }'`
    done


    # print data
    printf "%24s " $trial 
    for cores in $core_list; do
	if [ ${efficency[$cores]} = "xxx" ]; then
	    printf "%7s " ""
	else
	    printf "%7.2f " ${efficency[$cores]} 
	fi
    done
    echo ""
}


############################################################
#
# compares two tests
#
#
compare()
{
    export ReferencePath=$1
    export ComparePath=$2

    # check parameters
    if [ -z "$ReferencePath" -o -z "$ComparePath" ]; then
	echo "Defined test cases are:"
	cd $PERF_RESULTS
	ls -l | grep "^d" | awk '{ printf( "    %s\n", $9) }'
	exit 1
    fi

    echo "                CBM Performance Comparison"
    echo "                =========================="
    echo ""
    echo "Comparing testcases [$ComparePath] to [$ReferencePath]"
    echo ""

    trial_list=`cat $PERF_RESULTS/$ReferencePath/*.cbm | awk '{ print $4 }'| awk -F':' '{ print $2 }' | sort -u`
    core_list=`cat $PERF_RESULTS/$ReferencePath/*.cbm | awk '{ print $5 }'| awk -F':' '{ print $2 }' | sort -gu`

    # print header
    printf "%24s " "Efficency" 
    for cores in $core_list; do
	printf "%7s " $cores
    done
    echo ""

    # compute averages for all data in the reference file
    for trial in $trial_list; do
       
	compare_trial $trial

    done 

    echo ""
}


############################################################
#
# compares two results files
#
#
show_data()
{
    testcase=$1

    test_list="`cat $PERF_RESULTS/$testcase/TestSuite | grep -v '^#'`"
    for testname in $test_list; do

	# clean up previous definitions
	for i in 1 2 3 4 5 6 7 8 9 10; do
	    TrialName[$i]=""
	    TrialCoreSequence[$i]=""
	    RunSubmitType[$i]=""
	    RunCommand[$i]=""
	done

	TestFile=$TESTDIR/$testname.test
	. $TestFile

	echo ""
	echo "Test [$testname]: $TestDesc"
	echo "  Number of repeats for each run: $Repeat"
	echo "  Original project data: $TestData"
	echo "  Test subdirectory: [$TestDirectory]"

	trial_list=`cat $PERF_RESULTS/$testcase/$testname.cbm | awk '{ print $4 }'| awk -F':' '{ print $2 }' | sort -u`
	core_list=`cat $PERF_RESULTS/$testcase/$testname.cbm | awk '{ print $5 }'| awk -F':' '{ print $2 }' | sort -gu`

	for trial in $trial_list; do

	    i=1
	    for name in ${TrialName[*]}; do
		if [ $name = $trial ] ; then
		    break
		fi
		let 'i = i + 1'
	    done

	    echo ""
	    echo "  Trial $trial - ${TrialDesc[$i]}"
	    echo "  Core Sequence: [${TrialCoreSequence[$i]}]"
	    echo "  Submit type:   [${RunSubmitType[$i]}]"
	    echo "  Command:       [${RunCommand[$i]}]"
	    printf "  %5s %8s  %8s  %8s  %8s  %8s\n" Cores Success Min Average Max Difference

	    for cores in $core_list; do

	        # compute successful/total
		total=`cat $PERF_RESULTS/$testcase/$testname.cbm | grep "Trial:$trial Cores:$cores " | wc -l`
		if [ $total = "0" ]; then
		    continue
		fi
		succ=`cat $PERF_RESULTS/$testcase/$testname.cbm | grep "Trial:$trial Cores:$cores Evaluate:Success" | wc -l`

	        # line is in format: 
	        # "count min average max"
		times=`cat $PERF_RESULTS/$testcase/$testname.cbm | grep "Trial:$trial Cores:$cores Evaluate:Success" | awk '{ print $7 }' | awk -F':' '
BEGIN {
   min = 99999999
   max = -1
   total = 0
   count = 0
}
{
    val = ($2 * 3600) + ($3 * 60) + $4
    total = total + val
    if ( val < min )
        min = val
    if ( max < val )
        max = val
    count = count + 1
}
END { 
    if ( count == 0 )
        printf( "xxx xxx xxx xxx\n" )
    else
    {
        diff = max - min
        printf( "%d %d %d %d\n", min, total / count, max, diff )
    }
}'`
	        # convert values to time format
		min=`echo "$times" | awk '{ print $1 }'`
		avg=`echo "$times" | awk '{ print $2 }'`
		max=`echo "$times" | awk '{ print $3 }'`
		dif=`echo "$times" | awk '{ print $4 }'`

		if [ $min = "xxx" ]; then
		    mintime="XX:XX:XX"
		    avgtime="XX:XX:XX"
		    maxtime="XX:XX:XX"
		    diftime="XX:XX:XX"
		else
		    mintime=`date -d "1970-01-01 00:00 $min seconds" +"%H:%M:%S"`
		    avgtime=`date -d "1970-01-01 00:00 $avg seconds" +"%H:%M:%S"`
		    maxtime=`date -d "1970-01-01 00:00 $max seconds" +"%H:%M:%S"`
		    diftime=`date -d "1970-01-01 00:00 $dif seconds" +"%H:%M:%S"`
		fi

	        # output line
		printf "  %5d %3d/%-4d  %8s  %8s  %8s  %8s\n" $cores $succ $total $mintime $avgtime $maxtime $diftime
	    done
	done
	echo ""
    done
}

############################################################
#
show_testdata()
{
    testcase=$1

    # check parameters
    if [ -z "$testcase" ]; then
	echo "Defined test cases are:"
	cd $PERF_RESULTS
	ls -l | grep "^d" | awk '{ printf( "    %s\n", $9) }'
	exit 1
    fi

    echo "                CBM Test Report"
    echo "                ==============="
    echo ""
    echo "Testcase name [$testcase]"
    echo "  Resource             `cat $PERF_RESULTS/$testcase/ResourceName`"
    suite=`cat $PERF_RESULTS/$testcase/TestSuite | grep -v '^#' | while read line; do
    echo -e "$line \c"
done`
    echo "  Test suite           [$suite]"

    echo "  Test period started  `cat $PERF_RESULTS/$testcase/*.journal | grep -v "^---" | head -1 | awk '{ print $1 }'`" 
    echo "  Test period ended    `cat $PERF_RESULTS/$testcase/*.journal | grep -v "^---" | tail -1 | awk '{ print $1 }'`"
    echo "  Total runs           `cat $PERF_RESULTS/$testcase/*.cbm | wc -l | awk '{ print $1 }'`"
    echo "  Runs that failed     `grep 'Evaluate:Failed' $PERF_RESULTS/$testcase/*.cbm | wc -l | awk '{ print $1 }'`"
    echo ""

    show_data $testcase
}

############################################################
#
# main
#

# load resource definitions
load_definitions
load_resources

# get parameters
[ -z "$1" ] && usage
option=$1

case $option in 
# runs test
run)
	run_testcase $2 $3 $4
	;;

# evaluates single test data
evaluate)
	evaluate_run $2
	;;

# generates test report
report)
	show_testdata $2
	;;

# displays speedup graph of test data
speedup)
	speedup $2
	;;

# compares two testcases
compare)
	compare $2 $3
	;;

# unknown option
*)
	usage
	;;
esac


# End Script

