#!/usr/bin/python
#

"""The job manager

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
import copy
import string


# my modules
import utils


# data
class jcd(object):
    """
    Job Control
    """
    def __init__(self):
        self.par = utils.params()
        self.cfg = utils.config( self.par.file  )
        self.res = utils.resourceLimits( self.cfg.resource_name )
        self.verbose = utils.Verbose( self.par.verbose, sys.stdout )
        self.child = dict()
        self.jobs = utils.Jobs( self.cfg )
        self.max_cores = 0
        self.procs_avail = 0
        self.disk_avail = 0


def set_maximum_cores( jc ):
    jc.verbose( 3, 'set_maximum_cores' )
    #path = jc.cfg.home + '/etc/maximum_cores'
    #if ( os.path.exists(path) == False ):
    #    jc.max_cores = int( jc.cfg.maximum_cores )
    #else:
    #    f = open( path, 'r' )
    #    jc.max_cores = int( f.readline().strip() )
    #    f.close()
    jc.max_cores = int( jc.cfg.maximum_cores )
    jc.verbose( 1, 'Maximum cores that can be used ' + str(jc.max_cores) )


def set_procs_available( jc ):
    '''scans the InProcess directory and counts the number
    of processors in use'''
    jc.verbose( 3, 'set_procs_available' )
    jc.procs_avail = jc.max_cores - jc.jobs.get_procs_used()
    jc.verbose( 2, 'Processors available ' + str(jc.procs_avail) + ' of ' + str(jc.max_cores) )
    if ( jc.procs_avail <= 0 ):
        return False
    return True
  

def set_work_disk_available( jc ):
    '''get the amount for free space on the working disrectory disk'''
    jc.verbose( 3, 'set_work_disk_available' )
    jc.disk_avail = utils.get_disk_size_available( jc.cfg.work_directory )
    jc.verbose( 2, 'Currently ' + str(jc.disk_avail) + ' MB available of work disk.' )
    # 100000 MB = 100GB
    if ( jc.disk_avail < 100000 ):
        jc.verbose( 0, ' Not enough disk available to start new jobs.' )
        return False
    return True


def same_resource( jc, tag ):
    '''only submit on this Gempis resource'''
    jc.verbose( 3, 'same_resource' )
    if ( jc.jobs.get_jt_procs( jc.cfg.todo_dir, tag ) <= 0 ):
        jc.verbose( 0, 'Tag ' + tag + ': not our resource.' )
        return False
    return True


def can_submit( jc, tag ):
    jc.verbose( 3, 'can_submit' )
    jt = utils.jobtag( jc.cfg.todo_dir + tag )
    jt.grab()
    procs = jt.get_int( 'PROCESSORS' )
    precondition = jt.get_str( 'PRECONDITION' )
    input_dir = jt.get_str( 'INPUT_DIRECTORY' )
    jt.release()
    if ( procs < 0 ):
        jc.verbose( 1, 'Cannot submit ' + tag + ': invalid number of processors.' )
        return False
    if ( procs > jc.procs_avail ):
        jc.verbose( 2, 'Cannot submit ' + tag + ': not enough processors.' )
        return False
    if ( len(precondition) == 0 ):
        return True
    # does precondition data exist
    if ( os.path.exists( input_dir ) == False ):
        jc.verbose( 1, 'Cannot submit ' + tag + ': precondition data ' + precondition + ' missing' )
        return False
    return True


def spawn_child( jc, tag ):
    ''' redirect here does not work

    need to modify prints to work in verbose mode and redirect to a log file
    '''
    jc.verbose( 3, 'spawn_child' )
    jc.verbose( 0, 'Spawning ' + tag )
    jt = utils.jobtag( jc.cfg.todo_dir + tag )
    # move tag to InProcess directory
    jt.move( jc.cfg.inproc_dir )
    mycmd = copy.deepcopy( 'submit.py -f ' + jc.cfg.inproc_dir + tag )
    cmd = mycmd.split()
    pid = os.spawnlp( os.P_NOWAIT, 'submit.py', cmd[0], cmd[1], cmd[2] )
    # save child process id (for poping later)
    jc.child[pid] = tag


def submit_jobtag( jc, tag ):
    '''submits job if possible'''
    jc.verbose( 3, 'submit_jobtag' )
    jc.verbose( 2, 'Testing tag ' + tag )
    # cannot submit job
    if ( same_resource(jc, tag) == False or can_submit(jc, tag) == False ):
        return
    spawn_child( jc, tag )
    # need to wait at least 1 second before continuing
    # gempis can only produce unique jobs 1 per second
    #time.sleep( 1 )


def acknowledge_children( jc ):
    '''
    check if any children have completed
    not doing this will create a lot of zombie children
    '''
    jc.verbose( 3, 'acknowledge_children' )
    poplist = []
    for pid, tag in jc.child.iteritems():
        try:
            # this is needed to terminate the zombie children
            p, status = os.waitpid( pid, os.WNOHANG )
            pid = p
        except OSError:
            pid = pid
        if ( pid > 0 ):
            jc.verbose( 1, '  tag ' + tag + ' completed.' )
            poplist.append( pid )
    # remove completed children from poplist
    for pid in poplist:
        jc.child.pop( pid )

    

def main():
    """
    Spawn a gempis process that will write resource status to a file.
    """
    try:
        jc = jcd()
        jc.verbose( 3, 'main' )
        # stay here until all jobs have completed
        while 1:
            todo_list = os.listdir( jc.cfg.todo_dir )
            # no more files to parse
            if ( len(todo_list) + len(os.listdir(jc.cfg.inproc_dir)) == 0 ):
                print 'All jobs have been completed.'
                break
            set_maximum_cores( jc )
            for tag in todo_list:
                # determine processors available
                if ( set_procs_available(jc) == False or set_work_disk_available(jc) == False ):
                    jc.verbose( 1, 'Breaking loop.' )
                    break
                submit_jobtag( jc, tag )
            acknowledge_children( jc )
            time.sleep( 60 )
    except KeyboardInterrupt:
        print 'Job manager terminated by user.'
        sys.exit(0)



if __name__ == '__main__':
    main()

# End Script
