#!/usr/bin/python
#
"""
Generates a list of the selected properties from all job tag files.

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


# data
class jcd(object):
    """
    Job Control
    """
    def __init__(self, file):
        self.par = utils.params()
        self.cfg = utils.config( self.par.file  )
        self.res = utils.resourceLimits( self.cfg.resource_name )
        self.verbose = utils.Verbose( self.par.verbose, sys.stdout )
        self.property = utils.Property( self.cfg.home )
        self.parse_args()
        self.header()

    def parse_args(self):
        self.test_keys = dict()
        self.disp_keys = []
        self.disp_index = 0
        if ( len(self.par.get_args()) == 0 ):
            jp = utils.jobProp()
            rp = utils.runProp()
            print 'The options are: ' + jp.get_keys() + rp.get_keys()
            sys.exit(0)
        for a in self.par.get_args():
            if ( a.find('=') > 0 ):
                o = a.split('=')
                self.test_keys[o[0]] = o[1]
            else:
                self.disp_keys.insert(self.disp_index, a)
                self.disp_index += 1

    def open_tag(self, file):
        path = os.path.abspath('.') + '/tag/' + file
        self.jt = utils.jobtag( path )
        self.jt.read_file()
        result = self.jt.get_str( 'RESULT' )
        if ( result != 'SUCCESS' ):
            return False
        return True

    def get_val(self, key):
        try:
            val = self.jt.get_str(key)
        except KeyError:
            val = self.get_prop(key)
        return val
        
    def test_tag(self):
        for k, v in self.test_keys.iteritems():
            if ( v != self.get_val(k) ):
                return False
        return True

    def header(self):
        '''print header '''
        print 'Requirements: ' ,
        for k, v in self.test_keys.iteritems():
            print k + '=' + v + ' ' ,
        print ''
        for k in self.disp_keys:
            print k + ' ' ,
        print ''
        
    def print_tag(self):
        for k in self.disp_keys:
            print self.get_val(k) + ' ' ,
        print ''

    def get_prop(self, key):
        # property file exist
        template_name = self.jt.get_str( 'TEMPLATE' )
        select_id = self.jt.get_str( 'SELECTED_ID' )
        select_value = self.jt.get_str( 'SELECTED_VALUE' )
        processors = self.jt.get_int( 'PROCESSORS' )
        self.property.set_name( template_name, select_id, select_value, processors )
        if ( self.property.exists() == False ):
            return '---'
        #
        rp = utils.runProperty( self.property.get_path() )
        return rp.get( key )



def check_tag( jc, tagfile ):
    if ( jc.open_tag( tagfile ) != True ):
        return
    if ( jc.test_tag() == True ):
        jc.print_tag()
    

def scan_directory( jc ):
    #print 'Looking for tag files in ' + os.path.abspath('.')
    for tag in os.listdir( 'tag' ):
        check_tag( jc, tag )


def main():
    jc = jcd( file ) 
    prev_dir = os.path.abspath('.')
    for test in os.listdir( jc.cfg.results_dir ):
        os.chdir( jc.cfg.results_dir )
        try:
            os.chdir( test )
            jc.verbose( 1, 'Searching for date directories in ' + test )
            for date_dir in os.listdir( '.'  ):
                os.chdir( date_dir )
                scan_directory( jc )
                os.chdir( '..' )
        except OSError:
            #print 'Skipping ' + test
            x = 0
    os.chdir( prev_dir )

                    
if __name__ == '__main__':
    main()

# End Script
