#!/usr/bin/python
#

"""Submits a job to the resource. This script should be spawned 

This program is part of "PerfTester", a parallel application
performance tester.
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"

import sys
import os
import getopt
import shutil
import time
import popen2



# my modules
import utils


# data
class jcd(object):
    """
    Job Parameters
    """
    def __init__(self):
        self.home = ''
        self.prev_cwd = ''
        self.run_result = 0
        self.log_result = 0
        self.par = utils.params()
        # creates lock file used by preapp.py
        utils.jobtag( self.par.file, './start.lock' )
        self.jt = utils.jobtag( self.par.file )
        self.jt.grab()
        self.res = utils.resourceLimits( self.jt.get_str('RESOURCE') )
        work_directory = self.jt.get_str( 'WORK_DIRECTORY' )
        self.home = self.jt.get_str( 'PERFTESTER_HOME' )
        self.input_directory = self.jt.get_str( 'INPUT_DIRECTORY' )
        self.output_directory = self.jt.get_str( 'OUTPUT_DIRECTORY' )
        results_directory = self.jt.get_str( 'RESULTS_DIRECTORY' )
        self.logfile_name = self.jt.get_str( 'LOGFILE_NAME' )
        self.command = self.jt.get_str( 'COMMAND' )
        self.logfile_path = self.jt.get_logfile_path()
        self.test = self.jt.get_str( 'TEST' )
        self.application_specific = self.jt.get_str( 'APPLICATION_SPECIFIC' )
        self.jt.set( 'HOSTNAME', utils.get_hostname() )
        self.jt.release()
        self.log = open( 'submit.log', 'w' )
        self.verbose = utils.Verbose( 2, self.log )
        self.work = utils.Work( '', work_directory )
        self.results = utils.Results( '', results_directory )


def define_gempis_environment_variables( jc ):
    '''define gempis environment vars'''
    jc.verbose( 2, 'define_gempis_environment_variables:' )
    os.putenv( 'GEMPIS_PREAPP', jc.home + '/bin/preapp.py -f ' + jc.par.file )
    os.putenv( 'GEMPIS_PROJECTNAME', jc.test )
    os.putenv( 'GEMPIS_VERBOSE', 'yes' )


def set_duration( jt ):
    te = time.time()
    jt.set( 'STOP_DATE', str(te) )
    tb = jt.get_float( 'START_DATE' )
    if ( tb <= 0 ):
        sval = '-1'
    else:
        sval = str(te - tb) 
    jt.set( 'DURATION', sval )

def set_attempts( jt ):
    ''' increment the number of attempts'''
    count = jt.get_int('ATTEMPTS')
    if ( count < 0 ):
        count = 0
    count += 1
    jt.set( 'ATTEMPTS', str(count) )

def exceeded_attempts( jt ):
    count = jt.get_int( 'ATTEMPTS' )
    maximum_attempts = jt.get_int( 'MAXIMUM_ATTEMPTS' )
    if ( count >= maximum_attempts ):
        return True
    return False


def set_directory_size( jc ):
    jc.verbose( 2, 'set_directory_size:' )
    d = utils.get_directory_size( jc.work.get_path() )
    jc.verbose( 1, 'Directory size was ' + d )
    dir_size = d.replace('K','')
    jc.jt.set( 'DIRECTORY_SIZE', dir_size )


def archive_logfile( jc ):
    '''archive logfile, if already there append with a .1'''
    jc.verbose( 2, 'archive_logfile:' )
    jc.verbose( 1, 'Archiving log file ' + jc.logfile_name )
    jc.results.archive_log( jc )


def create_work_dir( jc ):
    jc.verbose( 2, 'create_work_dir :' )
    jc.verbose( 1, 'Creating work data ' + jc.work.get_path() )
    if ( jc.work.create( jc.input_directory ) == False ):
        print 'Error: input directory ' + jc.input_directory + ' does not exist.'
        sys.exit(2)
    os.chdir( jc.work.get_path() )


def cleanup_work_dir( jc ):
    jc.verbose( 2, 'cleanup_work_dir:' )
    os.chdir( jc.prev_cwd )
    if ( jc.run_result == 0 and jc.log_result == 0 and len(jc.output_directory) > 0
         and os.path.exists(jc.output_directory) == False ):
        shutil.move( jc.work.get_path(), jc.output_directory )
    jc.work.cleanup()


def run_job( jc ):
    """
    run job given by tag file
    """
    jc.verbose( 2, 'run_job :' )
    command = 'nohup ' + jc.command + ' 2>&1 >' + jc.logfile_name
    jc.verbose( 1, 'Executing command [' + command + ']' )
    jc.run_result = os.system( command );
    #args = jc.command + ' 2>&1 >' + jc.logfile_name
    #p = Popen("nohup" + args, shell=True)
    #sts = os.waitpid(p.pid, 0)
    #
    jc.verbose( 1, 'Command complete result was ' + str(jc.run_result) )


def update_jobtag( jc ):
    '''update the jobtag with the results for the run'''
    jc.verbose( 2, 'update_jobtag:' )
    jc.jt.grab()
    jc.jt.set( 'EXECUTION_RETURN', str(jc.run_result) )
    set_directory_size( jc )
    set_duration( jc.jt )
    set_attempts( jc.jt )
    jc.log_result = application_specific.parse_logfile( jc.jt )
    result = jc.run_result + jc.log_result
    if ( result ):
        jc.jt.set( 'RESULT', 'FAILURE' )
        jc.jt.set( 'START_DATE', '' )
    else:
        jc.jt.set( 'RESULT', 'SUCCESS' )
    jc.jt.release()

def reset_jobtag( jc ):
    '''removes previous results from the jobtag'''
    jc.verbose( 2, 'reset_jobtag:' )
    jc.jt.grab()
    jc.jt.set( 'RESULT', '' )
    jc.jt.set( 'EXECUTION_RETURN', '' )
    jc.jt.set( 'LOGFILE_VERIFICATION_RETURN', '' )
    jc.jt.set( 'APPLICATION_DURATION', '' )
    jc.jt.set( 'DURATION', '' )
    jc.jt.set( 'START_DATE', '' )
    jc.jt.set( 'STOP_DATE', '' )
    jc.jt.release()

def move_jobtag( jc ):
    '''move the jobtag to the Results directory'''
    jc.verbose( 2, 'move_jobtag:' )
    path =  jc.results.get_path() + '/tag'
    jc.verbose( 1, 'Moving jobtag to ' + path )
    jc.jt.move( path )


def main():
    jc = jcd()
    jc.verbose( 2, 'main:' )
    # load application specific module
    global application_specific
    application_specific = utils.do_import('application_specific', jc.application_specific )
   #
    define_gempis_environment_variables( jc )
    jc.prev_cwd = os.path.abspath('.')
    while 1:
        reset_jobtag( jc )
        create_work_dir( jc )
        #
        run_job( jc )
        #
        update_jobtag( jc )
        #
        archive_logfile( jc )
        cleanup_work_dir( jc )
        if ( jc.run_result == 0 and jc.log_result == 0 ):
            jc.verbose( 1, 'Run succesfully completed.' )
            break
        # run failed
        if ( exceeded_attempts( jc.jt ) == True ):
            jc.verbose( 1, 'Number of attempts exceeded.' )
            break
        jc.verbose( 1, 'Retrying...' )
    move_jobtag( jc )


if __name__ == '__main__':
    main()

# End Script
