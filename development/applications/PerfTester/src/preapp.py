#!/usr/bin/python
#
# 
"""
Script executed by Gempis before each application instance is run.

Syntax:
    preapp <tag_file_path>

This program is part of "PerfTester", a parallel application
performance tester.
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"

import os
import sys
import getopt
import time

# my modules
import utils


def main():
    """
    Allows the first process running to write a timestamp
    when it has started and then remove the lock file so no other process
    will write over it..
    """
    # Only the first process reaching this lock should write this information
    # add starting date/time
    par = utils.params()
    jt = utils.jobtag( par.file, './start.lock' )
    jt.grab()
    print 'Start date of ' + par.file + ' is ' + jt.get_str( 'START_DATE' )
    if ( jt.get_float('START_DATE') <= 0  ):
        tstg = str( time.time() )
        print 'Setting start date to ' + tstg
        jt.set( 'START_DATE', tstg )
        jt.set( 'OS_RELEASE', utils.get_os_release() )
    jt.release()
    

if __name__ == '__main__':
    main()

# End Script
