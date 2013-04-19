#!/usr/bin/python
#
"""
Creates property file from selected job log files

This program is part of "PerfTester", a parallel application
performance tester.

Note: This script is used for Cauldron projects only
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



def tst( words, field, value ):
    '''tests if a specific field has a string value'''
    if ( len(words) <= field ):
        return False
    if ( words[field] == value ):
        return True
    return False

    
def set_log_properties( prop, logfile ):
    """
    Parses Cauldron log file to determine execution properties

    The fastcauldron application shows information that is not displayed
    (but needed) by fastgenex5 and fastmig.

    Format:
rank              0 
mx , my, mz      21  21   1
xs , ys, zs       0   0   0
xm , ym, zm      21  21   1
gxs , gys, gzs    0   0   0
gxm , gym, gzm   21  21   1

------------ Load Balancing Information ------------
 CPU  Nb Nodes   Percent. Total      Percent. Activity
   0      441         100.0               100.0
rank              1 
mx , my, mz      27  27   1
xs , ys, zs       4   0   0
xm , ym, zm       3   4   1
gxs , gys, gzs    3   0   0
gxm , gym, gzm    5   5   1
   1       12           1.6               100.0

---------------------------------------------------------
 Number of Geological Events : 7

------------------ Number of Segments -------------------
        LayerName   (Depo)Age    Max.Thickness    Nb.Seg

          Formation1        0          1500        15
          Formation2        8           950        10
          Formation3       14           950        10
          Formation4       23           200         2
          Formation5       27           760         8
          Formation6       33           760         8
               Crust       45         30000         8
              Mantle     4500         90000         5

                                      -------------------
                                      Total        66

    """
    print 'Logfile ' + logfile
    gotxy = False
    state = 0
    count = 0
    wnodes = dict()
    maxnodes = -1
    f = open( logfile, 'r' )
    length = 0
    mx = 0
    my = 0
    for line in f.readlines():
        #print 'Line (' + str(state) + ') = [' + line + ']'
        if ( state == 0 ):
            if ( line.find('rank           ') >= 0 ):
                state = 1
        elif ( state == 1 ):
            w = line.split()
            if ( tst(w, 0, 'mx') == True and len(w) > 5 ):
                prop.set( 'X_GRIDPOINTS', w[4] )
                prop.set( 'Y_GRIDPOINTS', w[5] )
                mx = int(w[4])
                my = int(w[5])
                gotxy = True
                state = 2
                count = 0
        elif ( state == 2 ):
            # dummy reads
            count += 1
            if ( count > 6 ):
                state = 3
        elif ( state == 3 ):
            if ( len(line) == 0 ):
                state = 5
            elif ( line.find(' Number of Geological Events ') >= 0 ):
                state = 5
                w = line.split()
                prop.set( 'NUMBER_OF_EVENTS', w[5] )
            else:
                #   0     3074          25.0                36.9
                w = line.split()
                nodes = float(w[1]) * float(w[3]) / 100.0
                wnodes[w[0]] = nodes
                #print 'wnodes[' + w[0] + '] = ' + str(nodes)
                if ( nodes > maxnodes ):
                    maxnodes = nodes
                state = 4
                count = 0
        elif ( state == 4 ):
            if ( len(line) == 0 ):
                state = 5
            else:
                w = line.split()
                if ( tst(w, 0, 'xs') == True and len(w) > 5 ):
                    xs = int(w[4])
                    ys = int(w[5])
                if ( tst(w, 0, 'xm') == True and len(w) > 5 ):
                    xm = int(w[4])
                    ym = int(w[5])
                    if ( xs == 0 ):
                        ym = 0
                    if ( ys == 0 ):
                        xm = 0
                    length += xm + ym
                count += 1
                if ( count > 5 ):
                    state = 3
        if ( line.find(' Number of Geological Events ') >= 0 ):
            state = 5
            w = line.split()
            prop.set( 'NUMBER_OF_EVENTS', w[5] )
        if ( line.find(' Crust    ') >= 0 ):
            w = line.split()
            prop.set( 'AGE', w[1] )
            prop.set( 'CRUST_SEGMENTS', w[3] )
        if ( line.find(' Mantle   ') >= 0 ):
            prop.set( 'MANTLE_SEGMENTS', line.split()[3] )
        if ( line.find(' Total     ') >= 0 and line.find('CPU') < 0 ):
            val = int( line.split()[1] ) 
            prop.set( 'Z_SEGMENTS', str(val) )
            prop.set( 'Z_GRIDPOINTS', str(val+1) )
    f.close()
    total = 0.0
    for k, v in wnodes.iteritems():
        total += v
    prop.set( 'EFFECTIVE_2D_GRIDPOINTS', str(total) )
    prop.set( 'MAX_2D_GRIDPOINTS_PER_PROCESS', str(maxnodes) )
    #print 'z sege ' + prop.get('Z_SEGMENTS')
    zgp = int( prop.get('Z_SEGMENTS') ) + 1
    sgp = zgp - int( prop.get('CRUST_SEGMENTS') ) - int( prop.get('MANTLE_SEGMENTS') ) + 1
    prop.set( 'SEDIMENT_GRIDPOINTS', str(sgp) )
    size2d = int( prop.get('X_GRIDPOINTS') ) * int( prop.get('Y_GRIDPOINTS') )
    size3d = sgp * size2d
    prop.set('TOTAL_2D_GRIDPOINTS', str(size2d) ) 
    prop.set('TOTAL_3D_GRIDPOINTS', str(size3d) ) 
    prop.set('COMMUNICATION_BOUNDARY', str(length) ) 
    prop.set('COMMUNICATION_SURFACE', str(length * zgp) ) 


def parse_project3d( prop, jt ):
    """
    Parses Cauldron project3d file for data
    Format:
    ;
    ;
    ; Formation Data
    ;
    [AllochthonLithoIoTbl]
    ;
    LayerName Lithotype 
    () () 
    [End]
    ;
    """
    t = jt.get_str('TEMPLATE')
    print t + ': ' ,
    path = jt.get_str('PERFTESTER_HOME') + '/Templates/' + t + '/' + t + '.project3d'
    if ( os.path.exists(path) == False ):
        print 'Project ' + path + ' does not exist.'
        return
    f = open( path, 'r' )
    # set to -1 to offset table header that is counted
    counting = False 
    counting_rows = False
    count = -1
    counting_reservoirs = False
    source_rocks = -1
    reservoirs = -1
    for line in f.readlines():
        if ( line.find('[End]') >= 0 ):
            if ( counting == True ):
                if ( counting_reservoirs == True ):
                    reservoirs = count
                else:
                    source_rocks = count
            counting = False
        elif ( counting == True ):
            if ( line.find(';') >= 0 or line.find('()') >= 0 ):
                # skip comments
                x = 0
            elif ( counting_rows == True ):
                if ( line.find('<Row>') >= 0 ):
                    print '++ ' ,
                    count += 1
            else:
                if ( line.find('<Row>') >= 0 ):
                    print '<Rows> ' ,
                    counting_rows = True
                    count = 1
                else:
                    print '+ ',
                    count += 1
        if ( line.find('[SourceRockLithoIoTbl]') >= 0 ):
            print 'S ' ,
            counting_reservoirs = False
            counting = True
            counting_rows = False
            count = -1
        if ( line.find('[ReservoirIoTbl]') >= 0 ):
            print 'R ' ,
            counting_reservoirs = True
            counting = True
            counting_rows = False
            count = -1
    f.close()
    nongeometric_loop = False
    if ( source_rocks > 0 ):
        print ' SourceRocks = ' + str(source_rocks) ,
        prop.set( 'SOURCE_ROCKS', str(source_rocks) )
    if ( reservoirs > 0 ):
        print ' Reservoirs = ' + str(reservoirs) ,
        prop.set( 'RESERVOIRS', str(reservoirs) )
    print '' 


def scan_tag( jc, tagfile ):
    path = os.path.abspath('.') + '/tag/' + tagfile
    jt = utils.jobtag( path )
    jt.read_file()
    result = jt.get_str( 'RESULT' )
    if ( result != 'SUCCESS' ):
        return
    # property data can only come from fastcauldron 
    application = jt.get_str( 'APPLICATION_NAME' )
    if ( application == 'generation' or application == 'migration' ):
        return
    # property file exist
    template_name = jt.get_str( 'TEMPLATE' )
    select_id = jt.get_str( 'SELECTED_ID' )
    select_value = jt.get_str( 'SELECTED_VALUE' )
    processors = jt.get_int( 'PROCESSORS' )
    jc.property.set_name( template_name, select_id, select_value, processors )
    if ( jc.property.exists() == True ):
        return
    #
    runProp = utils.runProperty( jc.property.get_path() )
    lf = os.path.abspath('.') + '/log/' + tagfile.split('.')[0] + '.log'
    logfile = lf
    count = 2
    while ( os.path.exists(lf + '.' + str(count)) == True ):
        logfile = lf + '.' + str(count)
        count += 1
    runProp.set( 'TEMPLATE', template_name )
    runProp.set( 'INSTANCE', jt.get_str('INSTANCE') )
    runProp.set( 'SELECTED_ID', select_id )
    runProp.set( 'SELECTED_VALUE', select_value )
    runProp.set( 'PROCESSORS', str(processors) )
    set_log_properties( runProp, logfile )
    parse_project3d( runProp, jt )
    runProp.write_file()


def scan_project3d( jc, tagfile ):
    path = os.path.abspath('.') + '/tag/' + tagfile
    jt = utils.jobtag( path )
    jt.read_file()
    result = jt.get_str( 'RESULT' )
    if ( result != 'SUCCESS' ):
        print tagfile + ' Failed'
        return
    # property data can only come from fastcauldron 
    #application = jt.get_str( 'APPLICATION_NAME' )
    #if ( application == 'generation' or application == 'migration' ):
    #    print tagfile + ' not useful'
    #    return
    # property file exist
    template_name = jt.get_str( 'TEMPLATE' )
    select_id = jt.get_str( 'SELECTED_ID' )
    select_value = jt.get_str( 'SELECTED_VALUE' )
    processors = jt.get_int( 'PROCESSORS' )
    jc.property.set_name( template_name, select_id, select_value, processors )
    #if ( jc.property.exists() == True ):
    #    print tagfile + ' already exists'
    #    return
    #
    runProp = utils.runProperty( jc.property.get_path() )
    parse_project3d( runProp, jt )
    runProp.write_file()


def correct_prop( jc, tagfile ):
    print tagfile + ':' ,
    path = os.path.abspath('.') + '/tag/' + tagfile
    jt = utils.jobtag( path )
    jt.read_file()
    result = jt.get_str( 'RESULT' )
    if ( result != 'SUCCESS' ):
        print ' Failed'
        return
    # property data can only come from fastcauldron 
    application = jt.get_str( 'APPLICATION_NAME' )
    if ( application == 'generation' or application == 'migration' ):
        print ' not useful'
        return
    # property file exist
    template_name = jt.get_str( 'TEMPLATE' )
    select_id = jt.get_str( 'SELECTED_ID' )
    select_value = jt.get_str( 'SELECTED_VALUE' )
    processors = jt.get_int( 'PROCESSORS' )
    jc.property.set_name( template_name, select_id, select_value, processors )
    #if ( jc.property.exists() == True ):
    #    print tagfile + ' already exists'
    #    return
    #
    runProp = utils.runProperty( jc.property.get_path() )
    z_segments = int( runProp.get( 'Z_SEGMENTS' ) )
    crust_segments = int( runProp.get( 'CRUST_SEGMENTS' ) )
    mantle_segments = int( runProp.get( 'MANTLE_SEGMENTS' ) )
    zp = z_segments - crust_segments - mantle_segments
    zt = z_segments
    print ': zp = ' + str(zp) + ', zt = ' + str(zt)
    runProp.set( 'ZP_SEGMENTS', str(zp) )
    runProp.set( 'ZT_SEGMENTS', str(zt) )
    # cleanup old stuff
    runProp.write_file()


def scan_directory( jc ):
    print 'Looking for tag files in ' + os.path.abspath(',.')
    for tag in os.listdir( 'tag' ):
        #scan_tag( jc, tag )
        #scan_project3d( jc, tag )
        #correct_prop( jc, tag )
        x = 0


        
def main():
    """
    Spawn a gempis process that will write resource status to a file.
    """
    jc = jcd( file ) 
    # load application specific module
    global application_specific
    application_specific = utils.do_import('application_specific', jc.cfg.application_specific )
    #
    prev_dir = os.path.abspath('.')
    for test in os.listdir( jc.cfg.results_dir ):
        os.chdir( jc.cfg.results_dir )
        try:
            os.chdir( test )
            print 'Searching for date directories in ' + test
            for date_dir in os.listdir( '.'  ):
                os.chdir( date_dir )
                scan_directory( jc )
                os.chdir( '..' )
        except OSError:
            print 'Skipping ' + test
    os.chdir( prev_dir )

                    
if __name__ == '__main__':
    main()

# End Script
