#!/usr/bin/python
#

"""
A simple test script that loops and application using a
selected number of computer cores.

This program is part of "PerfTester", a parallel application
performance tester.
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"

import sys

for arg in sys.argv:
    print arg

version    = '-v2007.09'
#version    = ''

resource   = 'DarwinPerf'

#calculation = 'fastcauldron ' + version + ' -decompaction'
#calculation = 'fastcauldron ' + version + ' -temperature'
calculation = 'fastcauldron ' + version + ' -itcoupled'

#output     = "-nohdfoutput"
output      = ''

project    = 'project3_01M24'
suffix     = '.project3d'

core_list = [ 1, 2, 4, 8, 16, 32, 64 ]

import os

for cores in core_list:
    message = '----- Running ' + str(cores)
    print message
    logfile = project + '-' + resource + '-' + str(cores) + '.log'
    projinfo = '-project ' + project + suffix + ' -save _' + project + suffix
    command = 'gempis ' + version + ' ' + resource + ' ' + str(cores)
    command += ' ' + calculation + ' ' + projinfo + ' ' + output
    command += ' | tee -a ' + logfile
    print '  Executing command [' + command + ']'
    os.system( command )


