#!/usr/bin/python
#
"""
Application that creates job tag files from the test configuration file

This program is part of "PerfTester", a parallel application
performance tester.
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"


import os
import time
import copy
import getopt
import sys
import shutil
import datetime

# my modules
import utils

global application_specific


# data
class jobd(object):
    """
    make_tag job control parameters
    """
    def __init__(self):
        self.application = ''
        self.template_name = ''
        self.selected_value = ''
        self.cores = 0
        self.memory_required = 0
        self.precondition = ''
        self.log_filename = ''
        self.tag_filename = ''
        self.jt = 0
        self.is_a_precondition = False
        self.par = utils.params()
        self.cfg = utils.config( self.par.file  )
        self.res = utils.resourceLimits( self.cfg.resource_name )
        self.verbose = utils.Verbose( self.par.verbose, sys.stdout )
        if ( os.path.exists(self.cfg.home) == False ):
            os.mkdir( self.cfg.home )
        self.results = utils.Results( self.cfg.home, self.cfg.testname )
        self.results.create_base()
        self.date_time = self.results.get_name()


def create_project_instance( job ):
    """
    if instance doesn't already exist copy project

    template and perform instantiation
    """
    job.verbose( 2, 'In routine create_project_instance.' )
    job.instance = utils.Instance( job, application_specific )
    if ( job.instance.has_data() == False ):
        return False
    return True


import ConfigParser

def set_command( job ):
    """
    ONLY REFERRED TO  LOCALLY
    
    returns a string to submit the job tio the cluster.
    
    this will be a Gempis command
    """
    command = 'gempis '
    if ( len(job.cfg.application_version) > 0 ):
        command += job.cfg.application_version
    command += ' ' + job.cfg.resource_name + ' ' + str(job.cores) + ' ' 
    command += job.cfg.configdict.get( job.application, "command" ) + ' '
    command += job.template_name
    command += job.cfg.configdict.get( job.application, "end_command" )
    option = job.cfg.application_optional_parameter
    if ( len(option) > 0 ):
        command += ' ' + option
    job.jt.set( 'COMMAND', command )


def write_jobtag( job, repeat=1 ):
    """
    Creates a job tag file 
    """
    # this name is unique for the test run
    job.verbose( 2, 'In routine write_jobtag.' )
    # This check is removed because other aopplications will not work
    #if ( job.memory_required <= 0 ):
    #    print 'Cannot create jobtag ' + job.tagname + ', MEMORY_REQUIRED not defined.'
    #    return False
    #
    jobs = utils.Jobs( job.cfg )
    job.tagname = jobs.set_tagname( job.instance.get_name(), job.application, job.cores, repeat )
    if ( job.is_a_precondition == False and jobs.tag_exists() == True ):
        job.verbose( 1, "Jobtag " + job.tagname + ' skipped. Jobtag already exists' )
        return False
    work = utils.Work( job.cfg.home, job.tagname )
    #
    job.log_filename = job.tagname + '.log'
    job.tag_filename = job.tagname + '.tag'
    job.jt = utils.jobtag( job.cfg.todo_dir + job.tag_filename )
    job.jt.set( 'PERFTESTER_HOME', job.cfg.home )
    job.jt.set( 'TEST', job.cfg.testname )
    job.jt.set( 'CREATE_DATE', job.date_time )
    job.jt.set( 'APPLICATION_SUITE', job.cfg.application_suite )
    job.jt.set( 'APPLICATION_NAME', job.application )
    job.jt.set( 'APPLICATION_VERSION', job.cfg.application_version )
    job.jt.set( 'APPLICATION_OPTIONAL_PARAMETER', job.cfg.application_optional_parameter )
    job.jt.set( 'APPLICATION_SPECIFIC', job.cfg.application_specific )
    job.jt.set( 'RESOURCE', job.cfg.resource_name )
    job.jt.set( 'WORK_DIRECTORY', work.get_path() )
    job.jt.set( 'PROCESSORS', str(job.cores) )
    job.jt.set( 'HOSTNAME', utils.get_hostname() )
    job.jt.set( 'DISK', utils.get_disk_device_name() )
    job.jt.set( 'LOGFILE_NAME', job.log_filename )
    job.jt.set( 'TEMPLATE', job.template_name )
    job.jt.set( 'INSTANCE', job.instance.get_name() )
    job.jt.set( 'MEMORY_REQUIRED', str(job.memory_required) )
    job.jt.set( 'MAXIMUM_ATTEMPTS', str(job.cfg.maximum_attempts) )
    job.jt.set( 'INPUT_DIRECTORY', job.instance.get_precondition_path( '' ) )
    if ( job.selected_value == 'none' ):
        val = 'none'
    else:
        val = job.cfg.selected_parameter_id 
    job.jt.set( 'SELECTED_ID', val )
    job.jt.set( 'SELECTED_VALUE', job.selected_value )
    job.jt.set( 'RESULTS_DIRECTORY', job.results.get_path() )
    set_command( job )
    if ( application_specific.write_jobtag( job ) == False ):
        return False
    if ( job.is_a_precondition == True ):
        job.verbose( 1, "Jobtag " + job.tagname + ' created. Precondition.' )
    else:
        job.verbose( 1, "Jobtag " + job.tagname + ' created.' )
    # create the tag file 
    job.jt.create()
    return True
    

def check_result( job, count ):
    '''
    looks through all Results/ directories to try and find 1 SUCCESS result

    = True  - create tag
    = False - do not create tag
    '''
    job.verbose( 2, 'In routine check_result.' )
    if ( len(job.cfg.only_test_once) == 0 ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' create each time.' )
        return True
    result = job.results.find_success( job.tagname + '.tag', count )
    if ( result == False ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Already have successful results.' )
    return result

def check_size( job ):
    '''verifies job can run on memory and cores available'''
    # enough cores to run
    if ( job.cores > job.res.limit ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Not enough cores on resource.' )
        return False
    # will memory fit 
    if ( job.memory_required > job.cores * job.res.memoryper ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Cannot run in the allocated memory.' )
        return False
    return True


def write_precondition_jobtag( job, app_name ):
    """
    The number of processors used is based on minimum memory requireement
    """
    #
    # check if data is already there
    job.verbose( 2, 'In routine write_precondition_JOBTAG.' )
    # determine if tha data is already there or a job tag exists that will do the processing
    if ( job.instance.have_precondition_data(app_name) == True ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Precondition data already exists for '
                     + app_name )
        return
    #
    job2 = copy.deepcopy( job )
    job2.is_a_precondition = True
    job2.precondition = job.application
    job2.application = app_name
    # determine the memory required for the job -> # cores
    application_specific.set_memory_required( job2 )
    if ( job2.memory_required < 0 ):
        return
    job2.cores = int( job2.memory_required + job2.res.memoryper - 1 ) / job2.res.memoryper
    job2.cores *= 2
    if ( job2.cores > job.res.limit ):
        job2.cores = job.res.limit
        self.verbose = utils.Verbose( self.par.verbose, sys.stdout )
    # needed or verbose will not work 
    job2.verbose = utils.Verbose( job.par.verbose, sys.stdout )
    write_jobtag( job2 )


def add_jobtag( job, repeat=1 ):
    '''create job tag, checks if any other jobs are needed'''
    job.verbose( 2, 'In routine add_jobtag.' )
    jobs = utils.Jobs( job.cfg )
    job.tagname = jobs.set_tagname( job.instance.get_name(), job.application, job.cores, repeat )
    if ( check_size(job) == False ):
        return
    if ( check_result( job, repeat ) == False ):
        return
    # create any precondition teste needed
    if ( write_jobtag(job, repeat) == False ):
        return
    precondition_list = application_specific.get_precondition_jobs( job ).split()
    for app in precondition_list:
        write_precondition_jobtag( job, app )



def main():
    """
    Spawn a gempis process that will write resource status to a file.
    """
    #global application_specific
    job = jobd()
    # load application specific module
    global application_specific
    application_specific = utils.do_import('application_specific', job.cfg.application_specific ) 
    # archive test configuration
    job.verbose( 1, 'Copying ' + job.par.file + ' to ' + job.results.get_path() )
    shutil.copy( job.par.file, job.results.get_path() )
    #
    for job.template_name in job.cfg.project_template_list:
        for job.selected_value in job.cfg.selected_parameter_list:
            job.verbose( 1, 'Select value = ' + job.selected_value )
            if ( create_project_instance(job) == False ):
                continue
            for job.application in job.cfg.application_list:
                application_specific.set_memory_required( job )
                if ( job.memory_required < 0 ):
                     continue
                for c in job.cfg.core_list:
                    job.cores = int(c)
                    count = 1
                    while ( count <= job.cfg.number_repeats ):
                        add_jobtag( job, count )
                        count += 1

                    
if __name__ == '__main__':
    main()

# End Script
