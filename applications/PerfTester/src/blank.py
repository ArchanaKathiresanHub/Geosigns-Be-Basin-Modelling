#!/usr/bin/python
#

"""
This program is part of "PerfTester", a parallel application performance tester.

Empty (blaink) application specific rules

When running for another application this module must be replaced by a comparable
module with the same subroutines.

Routines required by other modules:
- have_output_data()
- set_memory_required()
- write_jobtag()
- get_precondition_jobs()
- create_project_instance()
- parse_logfile()
- extract_duration()
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"


import sys
import os
import getopt
import string
import popen2
import shutil

import utils


def have_output_data( path, project_name ):
    '''
    determines if output data has been created for the project
    
    path         - path to project directory
    project_name - the name of the project

    Returns:
    True - output data is available
    False - it is not
    '''
    return True
    

def set_memory_required( job ):
    '''
    sets the job.memory_required to the amount of memory required to run the job

    job - the jobd structure defined in make_tags.py

    job.memory_required values:
    -1     - value could not be determined
    >= 0   - value determined

    Returns: nothing
    '''
    job.memory_required = 0
    return


def write_jobtag( job ):
    '''
    writes application specific information to the jobtag.

    job - the jobd structure defined in make_tags.py

    Returns:
    True  - jobtag data successfully added
    False - and error occured, cannot continue
    '''
    return True


def get_precondition_jobs( job ):
    """
    give a list of jobs that must be completed before this job can be run

    job - the jobd structure defined in make_tags.py

    Returns:
    string of the application names that must have completed before this one cane be run
    (empty) - there are no precondition jobs
    """
    return ''


def create_project_instance( instance ):
    '''
    This routine creates project instances from template project data

    instance - the Instance class defined in utils.py and created
               when jobtags are being created in make_tags.py.

    Return:
    True - instance successfuloy created.
    False - could not create.
    '''
    return False


def parse_logfile( jt ):
    """
    Parses Cauldron log file to determine success or failure of the job

    jt - the jobtag control structure define in utils.py. The main use of this is
         in submit.py to check if the command has completed successfully.
    
    Returns:
    0   = job was successful
    otherwise job failed
    """
    return 0


# End Script
