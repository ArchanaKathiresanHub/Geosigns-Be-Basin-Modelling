#!/usr/bin/python
#

"""
display incormation about the results of the tests.

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
import popen2



# my modules
import utils

def get_hours( t ):
    return str(t / 3600)

def get_mins( t ):
    return "%02d" % (t % 3600 / 60)

def get_secs( t ):
    return "%02d" % (t % 60)


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
        self.property = utils.Property( self.cfg.home )
        self.parse_args()
        self.run_command()
        self.sort_list()
        self.filter_bad()
        self.set_scale()
        self.set_times()

    def parse_args(self):
        if ( len(self.par.get_args()) < 2 ):
            print 'Usage: '
            print '    show_results.py -f <config_file> [-v] time|speedup <application_name> [<display_options>]'
            print ''
            if ( len(self.par.file) > 0 ):
                print 'the application names defined in the configuration file are '
                print self.cfg.application_list
            print ''
            jp = utils.jobProp()
            rp = utils.runProp()
            print 'The display_options are'
            print jp.get_keys() + rp.get_keys()
            print ''
            sys.exit(0)
        self.result_type = self.par.get_args()[0]
        self.application = self.par.get_args()[1]
        self.display = ''
        for x in self.par.get_args()[2:]:
            self.display +=  x + ' '

    def run_command(self):
        '''
            Do not change the first 2 elements, used for sorting
                   and duration is always used
            added new entries to the end of the list
        '''
        command = './report_tags.py -f ' + self.par.file
        command += ' APPLICATION_NAME=' + self.application
        #if ( self.application == 'migration' ):
        #    command += ' TEMPLATE'
        #else:
        command += ' INSTANCE'
        command += ' PROCESSORS'
        # genex time is compute only, does not show storage time
        if ( self.application == 'generation' ):
            command += ' DURATION '
        else:
            command += ' APPLICATION_DURATION '
        command += self.display
        self.verbose( 1, 'Running command [' + command + ']' )
        i, o = popen2.popen2( command )
        req = i.readline()
        header = i.readline()
        j = 0
        self.duration_list = []
        for line in i.readlines():
            element = line.strip().split()
            if ( element[1] == '---' or element[2] == '---' ):
                continue
            # set processors
            element[1] = int(element[1])
            # set duration
            element[2] = float(element[2])
            # ignore 0 duration values
            if ( element[2] <= 0 ):
                continue
            # save value
            self.duration_list.insert( -1, element )
            j += 1
        i.close()
        o.close()

    def sort_list(self):
        self.duration_list.sort()

    def filter_bad(self):
        '''remove larger duration values'''
        prev = []
        i = 0
        self.verbose( 1, 'duration_list before = ' )
        self.verbose( 1, self.duration_list )
        length = len(self.duration_list)
        while ( i < length ):
            x = self.duration_list[i]
            self.verbose( 2, x )
            # instance and processor number the same
            if ( len(prev) != 0 and x[0] == prev[0] and x[1] == prev[1] ):
                self.verbose( 2,  '' )
                self.verbose( 2, 'found at ' + str(i) )
                self.verbose( 2, prev )
                self.verbose( 2, x )
                # don't save 0 durations
                if ( x[2] < prev[2] ):
                    self.verbose( 2, 'deleting previous ' + str(i - 1) )
                    self.verbose( 2, self.duration_list[i-1] )
                    del self.duration_list[i-1]
                    prev = x
                else:
                    self.verbose( 2, 'deleting current ' + str(i) )
                    self.verbose( 2, self.duration_list[i] )
                    del self.duration_list[i]
                self.verbose( 2, '' )
            else:
                i += 1
                prev = x
            length = len(self.duration_list)
            self.verbose( 2, 'length = ' + str(length) )
        self.duration_list.sort()
        self.verbose( 1, 'duration_list after = ' )
        self.verbose( 1, self.duration_list )

    def set_scale(self):
        self.scale = []
        for x in self.duration_list:
            processors = x[1]
            if processors not in self.scale:
                self.verbose( 1, 'adding ' + str(processors) )
                self.scale.insert( 0, processors )
        self.scale.sort()
        self.verbose( 1, 'The scale detected was ' )
        self.verbose( 1, self.scale )

    def get_instance_index(self, name):
        i = 0
        while i < len(self.instances):
            if ( self.instances[i] == name ):
                return i
            i += 1
        self.verbose( 1, 'instance error for name ' + name )
        return -1
    
    def get_scale_index(self, value):
        i = 0
        while i < len(self.scale):
            if ( self.scale[i] == value ):
                return i
            i += 1
        self.verbose( 1, 'scale error for value ' + value )
        return -1
    
    def set_times(self):
        self.durations = []
        self.instances = []
        self.display_opts = []
        # fill-in with blanks
        for x in self.duration_list:
            inst_name = x[0]
            if inst_name not in self.instances:
                self.verbose( 1, 'adding instance ' + inst_name )
                self.instances.insert( 0, inst_name )
                self.display_opts.insert( 0, '' )
                self.blank_arr = []
                for y in self.scale:
                    self.blank_arr.insert( 0, '---' )
                self.durations.insert( 0, self.blank_arr )
        # set order
        self.instances.sort()
        # fill with values
        for x in self.duration_list:
            inst_name = x[0]
            i = self.get_instance_index( inst_name )
            if ( x[3] != '---' ):
                self.display_opts[ i ] = x[3:]
            j = self.get_scale_index( x[1] )
            self.verbose( 2, 'adding duration ' + str(x[2]) + ' at [' + str(i) + '][' + str(j) + ']' )
            self.durations[i][j] = x[2]
        # make sure that the first index has a time value
        for x in self.durations:
            if ( self.result_type == 'speedup' ):
                # speedup calculations
                if ( x[0] == '---' ):
                    i = 0
                    while i < len(self.scale):
                        if ( x[i] != '---' ):
                            break
                        i += 1
                    if ( i < len(self.scale) ):
                        x[0] = self.scale[i] * x[i]
                    else:
                        x[0] = 1.0
            else:
                # time display
                i = 0
                while i < len(self.scale):
                    if ( x[i] != '---' ):
                        t = int(x[i])
                        x[i] = get_hours(t) + ':' + get_mins(t) + ':' + get_secs(t)
                    i += 1
        self.verbose( 1, 'duration values are ' )
        self.verbose( 1, self.durations )

    def set_base_speedup(self):
        for x in self.durations:
            base = x[0]
            i = 0
            while i < len(self.scale):
                if ( x[i] == 0.0 ):
                    x[i] = 1.0
                if ( x[i] != '---' ):
                    x[i] = base / x[i]
                i += 1
        self.verbose( 1, 'Results values are ' )
        self.verbose( 1, self.durations )

    def print_results(self):
        print self.display ,
        for s in self.scale:
            print 'cores-' + str(s) ,
        print ''
        i = 0
        while i < len(self.durations):
            for x in self.display_opts[i]:
                print x ,
            j = 0
            while j < len(self.scale):
                print self.durations[i][j] ,
                j += 1
            print ''
            i += 1


def main():
    """
    Spawn a gempis process that will write resource status to a file.
    """
    jc = jcd()
    if ( jc.result_type == 'speedup' ):
        jc.set_base_speedup()
    jc.print_results()


     
if __name__ == '__main__':
    main()

# End Script
