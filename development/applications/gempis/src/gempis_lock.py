#!/usr/bin/python
#

"""
File locking utility
"""

__author__ = "Greg Lackore"
__version__ = "$Revision: 0.1 $"
__date__ = "$Date:  $"
__copyright__ = "Copyright (c) 2008 Shell"
__license__ = "Python"

import getopt
import imp, sys
import os
import time
import string
import popen2
import shutil
import datetime
import struct, fcntl

class Verbose:
    '''
    example of use 
    if __name__ == \'__main__\': 
        import sys
        verbose = Verbose( 4, sys.stdout )
        verbose( 1, "Level 1 message" )
    '''
    import sys as __sys
    def __init__(self, verbosity = 0, log = __sys.stderr):
        self.__verbosity = verbosity
        self.__log = log
        
    def __call__(self, verbosity, msg):
        if verbosity <= self.__verbosity:
            # it can occur that this is attempted on a copy, just ignore
            try:
                print >> self.__log, '*' * verbosity, msg
            except ValueError:
                x = 0

class params(object):
    def __init__(self):
        self.verbose = False
        self.file = ''
        self.verbose = 0
        try:
            self.opts, self.args = getopt.getopt(sys.argv[1:], "hf:v", ["help", "file="])
        except getopt.GetoptError, err:
            # print help information and exit:
            print str(err) # will print something like "option -a not recognized"
            self.usage()
        for o, a in self.opts:
            if o == "-v":
                self.verbose += 1
            elif o in ("-h", "--help"):
                self.usage()
                sys.exit()
            elif o in ("-f", "--file"):
                self.file = a
            else:
                assert False, "unhandled option"
        if ( len(self.file) == 0 ):
            self.usage()
        if ( not os.path.exists(self.file) ):
            print 'File ' + self.file + ' does not exist.'
            sys.exit()
            
    def usage(self):
        print "Usage: " + sys.argv[0] + " -h -f <file> -v"
        sys.exit(2)

    def get_args(self):
        return self.args


class resourceLimits(object):
    """
    manages resource information
    """
    def __init__(self, name):
        """
        Spawn a gempis process that will write resource status to a file.
        """
        self.name = name
        file = os.getenv('HOME') + '/.gempis/resources/' + name
        if ( os.path.exists(file) == True ):
            self.get_file_limits(file)
        else:
            print 'Could not find resource file ' + file + ' using local computer.'
            self.limit, self.memoryper = self.get_computer_limits()
        self.inuse = 0
        
    def get_file_limits(self, file):
        f = open( file, 'r' )
        line = f.readline().strip()
        f.close()
        self.limit = int(line.split()[0])
        self.memoryper = int(line.split()[1])

    def get_computer_limits(self):
        # count occurances of 'processor' in file
        f = open( '/proc/cpuinfo', 'r' )
        count = 0
        for line in f.readlines():
            if ( line.find('processor') >= 0 ):
                count += 1
        f.close()
        limit = count
        # MemTotal:     16418320206 kB
        f = open( '/proc/meminfo', 'r' )
        memory = int( f.readline().strip().split()[1] )
        f.close()
        memoryper = ( memory / 1024 - 512 ) / count
        return limit, memoryper


class fileLock(object):
    """
    This class manages a file based locking

    only allows r and w modes
    """
    def __init__(self, filename, mode):
        self.filename = filename
        self.mode = mode
        if ( self.mode == 'w' ):
            self.flag = os.O_WRONLY
        else:
            self.mode = 'r'
            self.flag = os.O_RDONLY
            
    def flopen(self):
        '''open lock file'''
        #print 'fileLock.open' + self.filename + ',' + self.mode + ')'
        #self.f = os.open( self.filename, self.flag )
        self.f = open( self.filename, self.mode )
        try:
            # struct flock {
            #    short l_type;
            #    short l_whence;
            #    off_t l_start;
            #    off_t l_len;
            #    pid_t l_pid;
            # };
            if ( self.mode == 'w' ):
                flockdata = struct.pack('hhllhh', fcntl.F_WRLCK, 0, 0, 0, 0, 0)
            else:
                flockdata = struct.pack('hhllhh', fcntl.F_RDLCK, 0, 0, 0, 0, 0)
            fcntl.fcntl( self.f, fcntl.F_SETLKW, flockdata )
        except IOError, err:
            print 'ERROR!!!   fileLock.flopen(' + self.filename + ',' + self.mode + '): ' + str(err)
        return self.f
    
    def flclose(self):
        '''the lock is removed when the file is closed'''
        #print 'fileLock.flclose()'
        try:
            # flush buffers
            self.f.flush()
            # sync data to disk
            os.fsync( self.f.fileno() )
            # then close file
            self.f.close()
        except IOError, err:
            print 'ERROR!     fileLock.flclose(' + self.filename + ',' + self.mode + '): ' + str(err)


class resourceLock(object):
    """
    This class manages a locking mechanism for resources
    """
    def __init__(self, filename):
        self.filename = filename
        # creates the lock if it does not exist
        if ( os.path.exists(self.filename) == False ):
            self.create()
        self.fl = fileLock( self.filename, 'w' )
        
    def create(self):
        '''no locking here...'''
        f = open( self.filename, 'w' )
        print >> f, ""
        f.close()
        
    def remove(self):
        '''warning: do not use if you do not need it'''
        if ( os.path.exists(self.filename) ):
            os.remove( self.filename )
            
    def test(self):
        if ( os.path.exists(self.filename) ):
            return True
        else:
            return False
        
    def lock(self):
        self.fl.flopen()
        
    def unlock(self):
        self.fl.flclose()



class pfile:
    '''class that manages property files'''
    def set(self, property, value):
        self.pr.assign( property, value )
        self.changed = True
                
    def parse(self, line):
        '''parse line of jobtab file'''
        if ( len(line.strip()) > 0 ):
            o = line.split('=')
            if ( len(o) > 1 ):
                k = o[0].strip()
                v = o[1].strip()
                self.pr.assign( k, v )

    def format( self, f, k, v ):
        if ( len(v) > 0 ):
            print >> f, k + ' = ' + v
        
    def read_file(self):
        '''read contents, no lock'''
        if ( os.path.exists(self.path) == True ):
            f = open( self.path, 'r' )
            for line in f.readlines():
                self.parse( line )
            f.close()
        else:
            self.clean()
            
    def write_file(self):
        '''write contents, no lock'''
        f = open( self.path, 'w' )
        for k, v in self.pr.dct.iteritems():
            self.format( f, k, v )
        # flush buffers and sync to disk
        f.flush()
        os.fsync( f.fileno() )
        f.close()
        




class jobid(pfile):
    """
    manages the file containing the last jobid
    """
    def __init__(self, path, key = '/tmp/gempis/jobid.lock'):
        self.path = path
        self.changed = False
        self.key = resourceLock( key )
        
    def clean(self):
        self.pr.clean()

    def get_next(self):
        return self.pr.get_int(property)
    
    def create(self):
        '''open file, write contents, release'''
        self.key.lock()
        self.write_file()
        self.key.unlock()
        
    def grab(self):
        '''lock key, open file, read contents'''
        self.key.lock()
        self.read_file()
        # don't unlock
        
    def release(self):
        '''
        if lock file exists,
            data has changed, and file there:
                write contents,
            release key'''
        if ( self.key.test() == True ):
            # don't change if file has moved
            if ( self.changed == True and os.path.exists(self.path) == True ):
                self.write_file()
        self.key.unlock()
            


# End Script
