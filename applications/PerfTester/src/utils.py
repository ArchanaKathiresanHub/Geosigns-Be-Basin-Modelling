#!/usr/bin/python
#

"""Performance Tester utilitiees class definitions

This program is part of "PerfTester", a parallel application
performance tester.
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

def do_import( name, filename ):
    '''loads python module with this name'''
    module = imp.new_module( name )
    sys.modules[name] = module
    # get path for module
    path = ''
    for i in os.getenv('PATH').split(':'):
        path = i + '/' + filename
        if ( os.path.exists(path) == True ):
            break
    source = open( path, 'r' )
    exec source in vars(module)
    return module


def get_hostname():
    """
    returns the name of the computer
    """
    name = os.uname()[1]
    return name

def get_os_release():
    """
    returns the name of the computer
    """
    name = os.uname()[2]
    return name


def disk_read():
    """
    returns the disk name and size available in MB for the current disk
    Format df -k .:
Filesystem           1K-blocks      Used Available Use% Mounted on
amsdc2-na-p0250.europe.shell.com:/vol/v_s10/SWEast
                     1101004800 604366208 496638592  55% /nfs/rvl/groups/ept-sg/SWEast    
    """
    i, o = popen2.popen2( 'df -k .' )
    i.readline()
    w = i.readline().split()
    name = w[0]
    size = 0
    if ( len(w) < 4 ):
        w = i.readline().split()
        size = int(w[2])
    else:
        size = int(w[3])
    size = size / 1024
    for line in i.readlines():
        x = 0
    i.close()
    o.close()
    return name, size

def get_disk_device_name():
    """
    returns the name of the disk device in the current working directory
    """
    name, size = disk_read()
    return name


def get_disk_size_available( path ):
    """
    returns the size available in MB for the current disk
    """
    prev_dir = os.path.abspath('.')
    os.chdir( path )
    name, size = disk_read()
    os.chdir( prev_dir )
    return size


def get_directory_size( dir ):
    i, o = popen2.popen2( 'du -sh ' + dir )
    dir_size = i.readline().strip().split()[0]
    for line in i.readlines():
        x = 0
    i.close()
    o.close()
    return dir_size

def get_directory_kb( dir ):
    if ( os.path.exists(dir) == False ):
        return 0
    i, o = popen2.popen2( 'du -s ' + dir )
    dir_size = i.readline().strip().split()[0]
    for line in i.readlines():
        x = 0
    i.close()
    o.close()
    return int(dir_size)

def add_dir( path ):
    if ( os.path.exists(path) == False ):
        os.mkdir( path )


def del_dir( path ):
    if ( os.path.exists(path) == False ):
        return
    try:
        shutil.rmtree( path )
    except OSError:
        x = 0


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


from ConfigParser import *

class config(object):
    """
    data related to the test configuration file
    """
    def __init__(self, file):
        #print 'config.init( ' + file + ' )'
        self.file = file
        self.configdict = ConfigParser()
        self.configdict.read(file)
       # get data
        self.home = self.get_str('perftester_home')
        self.resource_name = self.get_str('resource_name')
        self.testname = self.get_str('testname')
        self.application_suite = self.get_str('application_suite')
        self.application_specific = self.get_str('application_specific')
        # use default value
        if ( len(self.application_specific) == 0 ):
            self.application_specific = 'blank.py'
        self.application_version = self.get_str('application_version')
        self.application_optional_parameter = self.get_str('application_optional_parameter')
        self.work_directory = self.get_str('work_directory')
        self.selected_parameter_id = self.get_str('selected_parameter_id')
        self.maximum_cores = self.get_str('maximum_cores')
        self.dedicated_resource = self.get_str('dedicated_resource')
        self.only_test_once = self.get_str('only_test_once')
        # integers
        self.maximum_attempts = self.get_int('maximum_attempts')
        self.number_repeats = self.get_int('number_repeats')
        self.cores_per_node = self.get_int('cores_per_node')
        # test lists
        self.project_template_list = self.get_list('project_template_list')
        self.selected_parameter_list = self.get_list('selected_parameter_list')
        self.application_list = self.get_list('application_list')
        self.core_list = self.get_list('core_list')
        # adjust values
        if ( len(self.selected_parameter_id) == 0 or len(self.selected_parameter_list) == 0 ):
            self.selected_parameter_id = 'none'
            self.selected_parameter_list = 'none'.split()
        # these are derived values
        self.instance_dir = self.home + '/Instances/'
        self.template_dir = self.home + '/Templates/'
        self.results_dir = self.home + '/Results/'
        self.todo_dir = self.home + '/Jobs/ToDo/'
        self.inproc_dir = self.home + '/Jobs/InProcess/'
        self.done_dir = self.home + '/Jobs/Completed/'
        
    def get_str(self, id):
        try:
            val = self.configdict.defaults()[id]
        except KeyError:
            val = ''
        return val
    
    def get_int(self, id):
        try:
            val = int( self.configdict.defaults()[id] )
        except KeyError:
            # default values here are always 1
            val = 1
        return val
    
    def get_list(self, id):
        try:
            val = self.configdict.defaults()[id]
        except KeyError:
            val = ''
        return val.split()



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


class prop:
    ''' This class manages property data'''
    def clean(self):
        for k, v in self.dct.iteritems():
            self.assign( k, v )
            
    def assign(self, property, value):
        # only assign if property is in list
        if ( self.dct.has_key(property) ):
            self.dct[property] = value
            #print property + ' = ' + value
        else:
            print 'Invalid assignment to ' + property
        
    def get_str(self, property):
        return self.dct[property]
    
    def get_int(self, property):
        try:
            v = int( self.dct[property] )
        except ValueError:
            v = -1
        return v
    
    def get_float(self, property):
        try:
            v = float( self.dct[property] )
        except ValueError:
            v = -1.0
        return v

    def get_keys(self):
        keys = ''
        for k in self.dct.iterkeys():
            keys += k + ' '
        return keys


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
        


class jobProp(prop):
    """
    class for managing job properties 
    """
    def __init__(self):
        self.dct = dict(
            APPLICATION_DURATION='', 
            APPLICATION_NAME='',
            APPLICATION_OPTIONAL_PARAMETER='',
            APPLICATION_SUITE='',
            APPLICATION_SPECIFIC='',
            APPLICATION_VERSION='',
            ATTEMPTS='',
            COMMAND='', 
            COMPUTE_NODES='', 
            CREATE_DATE='',
            DESCRIPTION='', 
            DIRECTORY_SIZE='', 
            DIRECTORY_SIZE_DIFFERENCE='', 
            DISK='', 
            DURATION='', 
            EXECUTION_RETURN='', 
            HOSTNAME='', 
            INPUT_DIRECTORY='',
            INSTANCE='',
            ITERATIONS='',
            JOBID='', 
            LOGFILE_NAME='', 
            LOGFILE_VERIFICATION_RETURN='',
            LOOPS='',
            MAXIMUM_ATTEMPTS='',
            MEMORY_REQUIRED='', 
            NO_HDF_OUTPUT='',
            OS_RELEASE='', 
            OUTPUT_DIRECTORY='',
            PERFTESTER_HOME='',
            PRECONDITION='',
            PROCESSORS='', 
            RESOURCE='', 
            RESULT='',
            RESULTS_DIRECTORY='',
            SELECTED_ID='', 
            SELECTED_VALUE='', 
            START_DATE='',
            STOP_DATE='',
            TEST='', 
            TEMPLATE='',
            WORK_DIRECTORY='')
        
    

class jobtag(pfile):
    """
    manages job tag files
    """
    def __init__(self, path, key = '/tmp/gempis/jobtag.lock'):
        self.path = path
        self.changed = False
        self.pr = jobProp()
        self.key = resourceLock( key )
        
    def clean(self):
        self.pr.clean()

    def get_str(self, property):
        return self.pr.get_str(property)
    
    def get_int(self, property):
        return self.pr.get_int(property)
    
    def get_float(self, property):
        return self.pr.get_float(property)

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
            
    def move(self, dest):
        self.key.lock()
        if ( os.path.exists(self.path) == True and os.path.exists(dest) == True ):
            shutil.move( self.path, dest )
        self.key.unlock()
        
    def get_input_dir(self):
        return self.get_str( 'PERFTESTER_HOME' ) + '/' + self.get_str( 'INPUT_DIRECTORY' )
    
    def get_logfile_path(self):
        return self.get_str( 'LOGFILE_NAME' )
    
    def get_jobtag_path(self):
        return self.get_str( 'WORK_DIRECTORY' ) + '/' + self.get_str( 'LOGFILE_NAME' )



class runProp(prop):
    """
    manages properties derived from run log files
    """
    def __init__(self):
        self.dct = dict(
            COMMUNICATION_BOUNDARY='',
            COMMUNICATION_SURFACE='',
            CRUST_SEGMENTS='',
            EFFECTIVE_2D_GRIDPOINTS='',
            INSTANCE='',
            MANTLE_SEGMENTS='',
            MAX_2D_GRIDPOINTS_PER_PROCESS='',
            NONGEOMETRIC_LOOP='',
            NUMBER_OF_EVENTS='',
            PROCESSORS='', 
            RESERVOIRS='', 
            SELECTED_ID='', 
            SELECTED_VALUE='', 
            SOURCE_ROCKS='', 
            TEMPLATE='',
            TOTAL_2D_GRIDPOINTS='',
            TOTAL_3D_GRIDPOINTS='',
            X_GRIDPOINTS='',
            Y_GRIDPOINTS='',
            Z_SEGMENTS='',
            ZP_SEGMENTS='',
            ZT_SEGMENTS='')


        
class runProperty(pfile):
    """
    manages properties derived from run log files
    """
    def __init__(self, path ):
        self.path = path
        self.changed = False
        self.pr = runProp()
        self.read_file()
        
    def clean(self):
        self.pr.clean()
            
    def get(self, property):
        return self.pr.get_str( property )
    


class Template(object):
    def __init__(self, home, name):
        self.home = home
        self.name = name
        self.base = 'Templates/' + self.name
        self.path = self.home + '/' + self.base
        # check that template is there
        if ( os.path.exists(self.path) == False ):
            print 'Template ' + self.path + ' does not exist.'
            sys.exit(1)

    def get_name(self):
        return self.name

    def get_path(self):
        return self.path



class Property(object):
    def __init__(self, home):
        self.home = home
        self.base = 'Properties/'
        self.path = ''
        add_dir( self.home + '/' + self.base )

    def set_name(self, template_name, selected_id, selected_value, processors ):
        self.name = template_name + '_' + selected_id + '_' + selected_value + '_%d' % processors
        self.path =  self.home + '/' + self.base + '/' + self.name + '.prop'

    def exists(self):
        if ( len(self.path) == 0 ):
            return False
        return os.path.exists( self.path )

    def get_path(self):
        return self.path



class Instance( object ):
    '''
    this class uses uses the global application_spe+cific
    '''
    def __init__(self, job, application_specific):
        """
        if instance doesn't already exist copy project template and perform instantiation
        """
        # if directory does not already exist
        self.job = job
        self.home = job.cfg.home
        self.template = Template( self.home, job.template_name )
        self.set_name( job.template_name, job.cfg.selected_parameter_id, job.selected_value )
        self.base = 'Instances/' + self.name
        self.path = self.home + '/'  + self.base
        self.datapath = self.home + '/Data' + self.name
        self.create( application_specific )

    def create(self, application_specific):
        if ( os.path.exists(self.path) == True ):
            return
        shutil.copytree( self.template.get_path(), self.path )
        if ( self.value != 'none' ):
            result = application_specific.create_project_instance( self )
            if ( result == False ):
                del_dir( self.path )
                self.job.verbose( 1, "Instance " + self.name + ' failed. Application specific changes failed' )
            else:
                self.job.verbose( 1, "Instance " + self.name + ' created.' )


    def set_name(self, template_name, id, value):
        self.template_name = template_name
        self.id = id
        self.value = value
        if ( self.value == 'none' or self.id == 'none' ):
            self.id = 'none'
            self.value = 'none'
        self.name = self.template_name + '_' + self.id + '_' + self.value

    def get_name(self):
        return self.name

    def get_value(self):
        return self.value

    def get_path(self, application):
        return self.path 

    def has_data(self):
        if ( os.path.exists(self.path) == True ):
            return True
        return False

    def get_precondition_path(self, application):
        s = self.home
        if ( len(application) > 0 ):
            s += '/Data/' + self.name + '-' + application
        else:
            s += '/Instances/' + self.name
        return s

    def have_precondition_data(self, application):
        if ( os.path.exists(self.get_precondition_path( application )) == True ):
            return True
        return False

    def need_precondition_data(self, path):
        if ( len(dest) <= 3 or os.path.exists(dest) == True ):
            return False
        return True

    def add_precondition_data(self, work_path, application, application_specific ):
        '''
        archive data only if:
           data is needed
           and it exists
        '''
        dest = self.get_precondition_path( application )
        if ( self.need_precondition_data(dest) == False ):
            return
        if ( application_specific.have_output_data(work_path, self.template_name) == False ):
            return
        shutil.move( work_path, dest )
        


class Jobs(object):
    def __init__(self, cfg):
        self.cfg = cfg
        self.base = cfg.home + '/Jobs'
        self.inproc_dir = self.base + '/InProcess/'
        self.todo_dir = self.base + '/ToDo/'
        add_dir( self.base )
        add_dir( self.inproc_dir )
        add_dir( self.todo_dir )

    def get_jt_procs(self, path, file):
        '''get number of processors for resource'''
        jt = jobtag( path + '/' + file )
        jt.grab()
        procs = 0
        if ( jt.get_str( 'RESOURCE' ) == self.cfg.resource_name ):
            procs = jt.get_int( 'PROCESSORS' )
        jt.release()
        return procs

    def get_procs_used(self):
        '''scans the InProcess directory and counts the number
        of processors in use'''
        inuse = 0
        for file in os.listdir( self.inproc_dir ):
            procs = self.get_jt_procs( self.inproc_dir, file )
            if ( procs > 0 ):
                if ( self.cfg.dedicated_resource == 'yes' ):
                    # adjust to compensate for dedicated allocation
                    procs = ( procs + self.cfg.cores_per_node - 1 )
                    procs = procs / self.cfg.cores_per_node * self.cfg.cores_per_node          
                inuse += procs
        return inuse

    def set_tagname(self, instance_name, application_name, num_cores, count ):
        self.tagname = instance_name + '_' + application_name + '_%d' % num_cores + '_%d' % count
        return self.tagname

    def tag_exists(self):
        if ( os.path.exists(self.todo_dir + self.tagname + '.tag') == True ):
            return True
        if ( os.path.exists(self.inproc_dir + self.tagname + '.tag') == True ):
            return True
        return False



class Work(object):
    def __init__(self, home, path):
        if ( len(home) > 0 ):
            self.home = home + '/'
            self.base = self.home + 'Work/'
            add_dir( self.base )
        else:
            self.base = ''
        self.path = self.base + path
        
    def create(self, input):
        # copy input data into work directory
        del_dir( self.path )
        if ( os.path.exists(input) == False ):
            return False
        #add_dir( self.path )
        shutil.copytree( input, self.path )
        return True
    
    def cleanup(self):
        del_dir( self.path )

    def get_path(self):
        return self.path

            

class Results(object):
    def __init__(self, home, testname):
        self.path = ''
        if ( len(home) > 0 ):
            self.home = home + '/'
            self.base = self.home + 'Results/'
            add_dir( self.base  )
            self.base += testname
            add_dir( self.base )
        else:
            self.path = testname
            
    def create_base(self):
        # create time based name
        now = datetime.datetime.now()
        d = now.date()
        t = now.time()
        self.name = str(d) + '_' + t.strftime('%H:%M:%S')
        self.path = self.base + '/' + self.name
        add_dir( self.path )
        add_dir( self.path + '/log' )
        add_dir( self.path + '/tag' )

    def get_name(self):
        return self.name

    def get_path(self):
        return self.path

    def archive_log(self, jc):
        '''archive logfile, if already there append with a .1'''
        if ( os.path.exists(jc.logfile_name) == False ):
            return
        if ( len(self.path) == 0 ):
            return
        dest_path = self.path + '/log/' + jc.logfile_name
        if ( len(dest_path) < 4 ):
            return
        print 'Archiving log ' + jc.logfile_name + ' to ' + self.path + '.'
        dest_file = dest_path
        count = 1
        while ( os.path.exists(dest_file) == True ):
            count += 1
            dest_file = dest_path + '.' + str(count)
        shutil.move( jc.logfile_name, dest_file )
        # update job tag with current location
        jc.jt.grab()
        jc.jt.set( 'LOGFILE_NAME', dest_file )
        jc.jt.release()

    def find_success(self, tagfile, count ):
        '''looks through all Results/ directories to try and find 1 SUCCESS result
        = True already have a successful run'''
        prev_dir = os.path.abspath('.')
        for test in os.listdir( self.home + 'Results/' ):
            os.chdir( self.home + 'Results/' )
            try:
                os.chdir( test )
                # print 'Searching for date directories in ' + test
                for date_dir in os.listdir( '.'  ):
                    os.chdir( date_dir + '/tag' )
                    if ( os.path.exists(tagfile) == True ):
                        jt = jobtag( os.path.abspath('.') + '/' + tagfile )
                        jt.grab()
                        result = jt.get_str( 'RESULT' )
                        jt.release()
                        if ( result == 'SUCCESS' ):
                            return True
                    os.chdir( '../..' )
            except OSError:
                x = 0
        os.chdir(prev_dir)
        return False



# End Script
