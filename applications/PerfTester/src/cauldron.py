#!/usr/bin/python
#

"""
This program is part of "PerfTester", a parallel application performance tester.

Cauldron specific rules

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
    data_dir = path + project_name + '_CauldronOutputDir'
    if ( os.path.exists(data_dir) == False ):
        return False
    # this should also check for an HDF file
    return True
    

# gives the fastreq options based on application name
fastreq_dict = dict( decompaction='-fastcauldron',
                temperature='-fastcauldron',
                itcoupled='-fastcauldron',
                hrdecompaction='-fastcauldron',
                generation='-fastgenex',
                migration='-fastmig' )


def set_memory_required( job ):
    '''
    sets the job.memory_required to the amount of memory required to run the job

    job - the jobd structure defined in make_tags.py

    job.memory_required values:
    -1     - value could not be determined
    >= 0   - value determined

    Returns: nothing
    '''
    name = job.instance.get_name()
    if ( job.application == 'migration' and job.instance.get_value() != 'none' ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Cannot subsample project for '
                     + job.application + ' .' )
        job.memory_required = -1
        return
    filename = job.cfg.instance_dir + job.instance.get_name() + '/' + job.template_name + '.project3d'
    if ( os.path.exists(filename) == False ):
        job.verbose( 1, 'Jobtag ' + job.tagname + ' skipped. Project file ' + filename
                     + ' does not exist.' )
        job.memory_required = -1
        return
    # run command
    command = 'fastreq -project ' + filename + ' ' + fastreq_dict[ job.application ]
    job.verbose( 2, 'Executing cauldron command ' + command )
    i, o = popen2.popen2( command )
    try:
        job.memory_required = int(i.readline().strip().split()[3])
    except IndexError:
        job.verbose( 0, 'Jobtag ' + job.tagname + ' skipped. fastreq failed.' )
        job.memory_required = -1
    i.close()
    o.close()


# defines the precondition applications
precondition_dict = dict( decompaction='',
                temperature='',
                itcoupled='',
                hrdecompaction='temperature',
                generation='temperature',
                migration='hrdecompaction' )

def write_jobtag( job ):
    '''
    writes application specific information to the jobtag.

    job - the jobd structure defined in make_tags.py

    Returns:
    True  - jobtag data successfully added
    False - and error occured, cannot continue
    '''
    pre = precondition_dict[ job.application ]
    if ( pre == 'hrdecompaction' and job.selected_value == 'none' ):
        pre = 'temperature'
    if ( len(pre) > 0 ):
        job.jt.set( 'PRECONDITION', pre )
    job.jt.set( 'INPUT_DIRECTORY', job.instance.get_precondition_path( pre ) )
    #
    save_output = False
    for k, v in precondition_dict.iteritems():
        if ( v == job.application ):
            save_output = True
            break
    if ( save_output == True ):
        job.jt.set( 'OUTPUT_DIRECTORY', job.instance.get_precondition_path( job.application ) )
    return True


def get_precondition_jobs( job ):
    """
    give a list of jobs that must be completed before this job can be run

    job - the jobd structure defined in make_tags.py

    Returns:
    string of the application names that must have completed before this one cane be run
    (empty) - there are no precondition jobs
    """
    if ( job.application != 'migration' and job.application != 'generation' ):
        return ''
    if ( job.application == 'generation'  ):
        return 'temperature'
    # then its migration
    for app in job.cfg.application_list:
        if ( app == 'generation' ):
            if ( job.cfg.selected_parameter_id == 'none'
                 or job.selected_value == 'none' ):
                return ''
            else:
                return 'hrdecompaction'
    # no scaling -> no hrdecompaction needed
    if ( job.cfg.selected_parameter_id == 'none' or job.selected_value == 'none' ):
        return 'temperature generation'
    return 'temperature generation hrdecompaction'


def create_project_instance( instance ):
    '''
    This routine creates project instances from template project data

    instance - the Instance class defined in utils.py and created
               when jobtags are being created in make_tags.py.

    Return:
    True - instance successfuloy created.
    False - could not create.
    '''
    #if ( instance.job.application == 'migration' or instance.job.application == 'hrdecompaction' ):
    #    instance.job.verbose( 1, 'Instance ' + instance.job.template_name + ' subsampling does not work for '
    #                          + instance.job.application + '.' )
    #    return False
    prev_dir = os.path.abspath('.')
    os.chdir( instance.path )
    appfile = instance.template_name + '.project3d'
    tmpfile = 'xxx.project3d'
    shutil.move( appfile, tmpfile )
    #
    command = 'subscale -input ' + tmpfile
    command += ' -output ' + appfile
    command += ' -x ' + instance.value +' -y ' + instance.value
    i, o, e = popen2.popen3( command )
    err = e.readline()
    print err
    i.close()
    o.close()
    e.close()
    os.remove( tmpfile )
    os.chdir( prev_dir )
    if ( len(err) > 0 ):
        result = False
    else:
        result = True
    return result


def find_fastcauldron_success( line ):
    ''' locally defined routine'''
    if ( line.find('End of simulation:') >= 0 ):
        #print 'Fastcauldron line [' + line + '] found.'
        return True
    return False

def find_fastgenex5_success( line ):
    ''' locally defined routine'''
    if ( line.find('Simulation Time: ') >= 0 ):
        #print 'Fastgenex5 line [' + line + '] found.'
        return True
    return False

def find_fastmig_success( line ):
    ''' locally defined routine'''
    if ( line.find('o Migration:: Finished Simulation Time Steps') >= 0 ):
        #print 'Fastmig line [' + line + '] found.'
        return True
    return False


def parse_logfile( jt ):
    """
    Parses Cauldron log file to determine success or failure of the job

    jt - the jobtag control structure define in utils.py. The main use of this is
         in submit.py to check if the command has completed successfully.

    Returns:
    0   = job was successful
    otherwise job failed
    
    Also determines:
     - the LSF JobId
     - The nuimber of compute nodes (computers) the job ran on
    """
    path = jt.get_logfile_path()
    if ( os.path.exists(path) == False ):
        print 'Logfile ' + jt.get_str('RESULTS_DIRECTORY') + '/log/' + path + ' does not exist.'
        return False
    nodes = {}
    result = 1
    loops = 0
    iterations = 0
    f = open( path, 'r' )
    for line in f.readlines():
        if ( find_fastcauldron_success( line ) == True
             or find_fastgenex5_success( line ) == True
             or find_fastmig_success( line ) == True ):
            result = 0
            break
        word = line.split()
        if ( len(word) == 0 ):
            continue
        if ( word[0] == 'Job' ):
            try:
                base = line.split('<')[1]
                id = base.split('>')[0]
                jt.set('JOBID', id )
            except IndexError:
                id = ''
        if ( word[0] == 'VERBOSE' ):
            # determine the number of compute nodes the job was run on
            #VERBOSE    AppWrap.amsdc1-n-c00568(23069): Starting [fastcauldron -v2008.04 ...
            if ( len(word) > 2 and word[2] == 'Starting' ):
                try:
                    name = word[1].split('.')[1]
                    host = name.split('(')[0]
                    nodes[host] += 1
                except KeyError:
                    nodes[host] = 1
        if ( line.find('o Decompacting... Age:') >= 0 ):
            iterations += 1
        if ( line.find('o Solving Temperature... Age:') >= 0 ):
            iterations += 1
        if ( line.find('o Solving Coupled [') >= 0 ):
            #o Solving Coupled [ 1 /  1]... Age:       32 (Ma) - TimeStep:        4 (Ma)
            iterations += 1
            word = line.split()
            if ( word[3] == '[' ):
                loops = int(word[4])
            else:
                loops = word[3].split('[')[1]
        if ( line.find('Computing SnapShot t:') >= 0 ):
            iterations += 1
    f.close()
    print 'Setting compute nodes to ' + str(len(nodes))
    jt.set( 'COMPUTE_NODES', str(len(nodes)) )
    # update tag file
    jt.set( 'LOGFILE_VERIFICATION_RETURN', str(result) )
    if ( iterations > 0 ):
        print ' Iterations = ' + str(iterations) ,
        jt.set( 'ITERATIONS', str(iterations) )
    if ( loops > 0 ):
        print ' Loops = ' + str(loops) ,
        jt.set( 'LOOPS', str(loops) )
    if ( result != 0 ):
        print 'ERROR:   Log file ' + path + ' did not find a successful completion.'
    duration = extract_duration( path )
    if ( duration > -1 ):
        jt.set( 'APPLICATION_DURATION', str(duration) )
    return result


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


# End Script
