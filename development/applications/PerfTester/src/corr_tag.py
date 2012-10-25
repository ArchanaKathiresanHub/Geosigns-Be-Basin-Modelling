#!/usr/bin/python
#

"""
The purpose of thei script is to correct any errors in the jobs
that that may have occurred. All Job tags in the Results directory
are examined. This gives the ability to modify specific tag values
fopr all tests run. It goes without saying that much care should be
used before executing this script.

This program is part of "PerfTester", a parallel application
performance tester.
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"


import getopt
import sys
import os
import getopt
import shutil
import time
import popen2


# my modules
import utils


# data
class jc(object):
    """
    Job Control
    """
    def __init__(self, file):
        self.par = utils.params()
        self.cfg = utils.config( self.par.file  )
        self.res = utils.resourceLimits( self.cfg.resource_name )
        self.verbose = utils.Verbose( self.par.verbose, sys.stdout )
        self.test = ''
        self.date_dir = ''



def test_tag( path ):
    '''tests whether the tag file has a duration value'''
    #print 'Scanning tag file ' + path
    jt = utils.jobtag( path )
    jt.grab()
    #print 'Duration = ' + jt.get_str('DURATION')
    duration = jt.get_float('DURATION')
    jt.release()
    if ( duration < 0  ):
        return True
    return False

def extract_duration( logfile ):
    """
    Parses the log file to determines the execution duration

    Returns: 
    < 0.0   - error
    otherwise a time in floating format
    """
    # set failure
    time = -1.0
    h = -1
    m = -1
    s = -1
    if ( os.path.exists( logfile ) == False ):
        return -1.0
    f = open( logfile, 'r' )
    for line in f.readlines():
        if ( find_fastcauldron_success( line ) == True ):
            #print logfile + ' FC Correcting with line ' + line
            w = line.split()
            h = int(w[3])
            m = int(w[5])
            s = int(w[7])
            break
        if ( find_fastgenex5_success( line ) == True ):
            #print logfile + ' FG Correcting with line ' + line
            w = line.split()
            h = int(w[2])
            m = int(w[4])
            s = int(w[6])
            break
        if ( find_fastmig_success( line ) == True ):
            #print logfile + ' FM Correcting with line ' + line
            w = line.split()
            t = w[7]
            c = t.split(':')
            if ( len(c) > 2 ):
                h = int(c[0])
                m = int(c[1])
                s = int(c[2])
            else:
                h = 0
                m = int(c[0])
                s = int(c[1])
            break
    f.close()
    if ( h > -1 and m > -1 and s > -1 ):
        time = h * 3600.0 + m * 60.0 + s
    return time


def set_duration_only( j, tag ):
    '''sets the APPLICATION_DURATION value in the tag file to that determined from the log file'''
    tagfile = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/tag/' + tag
    log = tag.split('.')[0] + '.log'
    logfile = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/log/' + log
    lf = logfile
    i = 1
    while ( os.path.exists(lf) == True ):
        duration = extract_duration( lf )
        if ( duration > -1 ):
            j.verbose( 1, 'Setting APPLICATION_DURATION in ' + tagfile + ' to ' + str(duration) )
            jt = utils.jobtag( tagfile )
            jt.grab()
            duration = jt.set( 'APPLICATION_DURATION', str(duration) )
            jt.release()
            break
        i += 1
        lf = logfile + '.' + str(i)
        print 'retrying with ' + lf


def set_duration( j, tag ):
    '''This soubroutine has several uses:
       - sets the APPLICATION_DURATION like the routine above,
       - verifies whether the result was successful from the log file
    '''
    tagfile = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/tag/' + tag
    log = tag.split('.')[0] + '.log'
    logbase = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/log/' + log
    logfile = logbase
    i = 1
    gotone = False
    jt = utils.jobtag( tagfile )
    jt.grab()
    jt.set( 'LOGFILE_NAME', logfile )
    while ( os.path.exists(logfile) == True ):
        duration = application_specific.extract_duration( logfile )
        application_specific.parse_logfile( jt )
        if ( duration > -1 ):
            gotone = True
            # adjust name of logfile
            jt.set( 'LOGFILE_NAME', logfile )
            j.verbose( 1, 'Setting APPLICATION_DURATION in ' + tagfile + ' to ' + str(duration) )
            duration = jt.set( 'APPLICATION_DURATION', str(duration) )
            break
        i += 1
        logfile = logbase + '.' + str(i)
        j.verbose( 1, 'retrying with ' + logfile )
    if ( gotone == False ):
        j.verbose( 1, 'Tag' + tagfile + ' failed.' )
        if ( jt.get_int('LOGFILE_VERIFICATION_RETURN') != 1 ):
            print 'Correcting FALSE POSITIVE LOGFILE_VERIFICATION_RETURN on tag ' + tagfile  
            jt.set( 'LOGFILE_VERIFICATION_RETURN', str(1) )
        if ( jt.get_str('RESULT') != 'FAILURE' ):
            print 'Correcting FALSE POSITIVE RESULT on tag ' + tagfile  
            jt.set( 'RESULT', 'FAILURE' )
    else:
        j.verbose( 1, 'Tag' + tagfile + ' succeeded.' )
        if ( jt.get_int('LOGFILE_VERIFICATION_RETURN') != 0 ):
            print 'Correcting FALSE NEGATIVE LOGFILE_VERIFICATION_RETURN on tag ' + tagfile  
            jt.set( 'LOGFILE_VERIFICATION_RETURN', str(0) )
        if ( jt.get_str('RESULT') != 'SUCCESS' ):
            print 'Correcting FALSE NEGATIVE RESULT on tag ' + tagfile  
            jt.set( 'RESULT', 'SUCCESS' )
    jt.release()


def set_iterations( j, tag ):
    print tag + ': ' ,
    tagfile = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/tag/' + tag
    jt = utils.jobtag( tagfile )
    jt.grab()
    application_specific.parse_logfile2( jt )
    jt.release()
    print ''



def set_disksize( j, tag ):
    '''
    translates the DIRECTORY_SIZE from human readable to a value in KB


    WARNING!!! only do this routine once !!!!!!!!!!


    '''
    print tag + ' ' ,
    tagfile = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/tag/' + tag
    log = tag.split('.')[0] + '.log'
    logbase = j.cfg.results_dir + '/' + j.test + '/' + j.date_dir +'/log/' + log
    logfile = logbase
    i = 1
    gotone = False
    jt = utils.jobtag( tagfile )
    jt.grab()
    s = jt.get_str('DIRECTORY_SIZE')
    if ( s.find('G') > 0 ):
        size = int( float( s.replace('G','') ) * 1048576 )
    elif ( s.find('M') > 0 ):
        size = int( float( s.replace('M','') ) * 1024 )
    elif ( s.find('K') > 0 ):
        size = int( float( s.replace('K','') ) )
    else:
        print tagfile + ' has no directory size'
        sys.exit(1)
    if ( len(s) > 0 ):
        print s + ' => ' + str(size) , 
    d = jt.get_str('PERFTESTER_HOME') + '/Instances/' + jt.get_str('INSTANCE')
    if ( os.path.exists(d) == True ):
        td = d + '/temperature'
        t = utils.get_directory_kb(td)
        if ( os.path.exists(td) ):
            print ' T(' + str(t) + ')' ,
            if ( size >= (t * 2) ):
                print '-' ,
                size -= t
        hd = d + '/hrdecompaction'
        h = utils.get_directory_kb(hd)
        if ( os.path.exists(hd) ):
            print ' H(' + str(h) + ')' ,
            if ( size >= (h * 2) ):
                print '-' ,
                size -= h
    print ' ====== ' + str(size) , 
    jt.set('DIRECTORY_SIZE', str(size) )
    print ''
    jt.release()


def main():
    """
    Spawn a gempis process that will write resource status to a file.

    the specific subroutine is activated as needed.
    """
    j = jc( file ) 
    # load application specific module
    global application_specific
    application_specific = utils.do_import('application_specific', j.cfg.application_specific )
    #
    prev_dir = os.path.abspath('.')
    for j.test in os.listdir( j.cfg.results_dir ):
        os.chdir( j.cfg.results_dir )
        try:
            os.chdir( j.test )
            print 'Searching for date directories in ' + j.test
            for j.date_dir in os.listdir( '.'  ):
                os.chdir( j.date_dir )
                print 'Looking for tag files in ' + j.date_dir
                for tag in os.listdir( 'tag' ):
                    #set_duration( j, tag )
                    #set_disksize( j, tag )
                    #set_iterations( j, tag )
                    continue
                os.chdir( '..' )
        except OSError:
            print 'Skipping ' + j.test


if __name__ == '__main__':
    main()

# End Script
