import os, sys, binascii
from cffi import FFI
from cffi.verifier import Verifier
from dateutil import parser
from datetime import datetime, timedelta, time, date
from collections import defaultdict
import time as timemod
import threading, glob
########################################################################
## Python DB API Globals
########################################################################
apilevel = " 2.0 "
threadsafety = 0  # no idea, so better safe
paramstyle = "qmark"  # unfortunately


########################################################################
## FFI Initilization
########################################################################
#----------------------------------------------------------------------
def _create_modulename(cdef_sources, source, sys_version):
    """
    This is the same as CFFI's create modulename except we don't include the
    CFFI version.

    Thanks to https://caremad.io/2014/11/distributing-a-cffi-project/ for this
    code.
    """
    key = '\x00'.join([sys_version[:3], source, cdef_sources])
    key = key.encode('utf-8')
    k1 = hex(binascii.crc32(key[0::2]) & 0xffffffff)
    k1 = k1.lstrip('0x').rstrip('L')
    k2 = hex(binascii.crc32(key[1::2]) & 0xffffffff)
    k2 = k2.lstrip('0').rstrip('L')
    return '_Py4d_cffi_{0}{1}'.format(k1, k2)

def _compile_module(*args, **kwargs):
    raise RuntimeError(
        "Attempted implicit compile of a cffi module. All cffi modules should "
        "be pre-compiled at installation time."
    )

class LazyLoadLib(object):
    def __init__(self, ffi):
        self._ffi = ffi
        self._lib = None
        self._lock = threading.Lock()

    def __getattr__(self, name):
        if self._lib is None:
            with self._lock:
                if self._lib is None:
                    #change working directory for CFFI compilation
                    _CWD = os.getcwd()
                    _FILE_PATH = os.path.dirname(os.path.realpath(__file__))
                    os.chdir(_FILE_PATH)
                    os.chdir(os.pardir)
                    self._lib = self._ffi.verifier.load_library()
                    os.chdir(_CWD)

        return getattr(self._lib, name)

ffi = FFI()

#change working directory for CFFI compilation
_CWD = os.getcwd()
_FILE_PATH = os.path.dirname(os.path.realpath(__file__))
os.chdir(_FILE_PATH)
os.chdir(os.pardir)

#use the absolute path to load the file here so we don't have to worry about working directory issues
_CDEF = open("{}/py_fourd.h".format(_FILE_PATH)).read()

ffi.cdef(_CDEF)

_SOURCE = """
#include "fourd.h"
"""

source_files = glob.glob('lib4d_sql/*.c')

ffi.verifier = Verifier(ffi, _SOURCE,
                       modulename=_create_modulename(_CDEF, _SOURCE, sys.version),
                       sources=source_files,
                       include_dirs=['lib4d_sql', 'py4d/lib4d_sql'])

#ffi.verifier.compile_module = _compile_module
#ffi.verifier._compile_module = _compile_module

lib4d_sql = LazyLoadLib(ffi)
os.chdir(_CWD)

########################################################################


########################################################################
## Error Classes
########################################################################
class Warning(StandardError):
    pass

class Error(StandardError):
    pass

class InterfaceError(Error):
    pass

class DatabaseError(Error):
    pass

class DataError(DatabaseError):
    pass

class OperationalError(DatabaseError):
    pass

class IntegrityError(DatabaseError):
    pass

class InternalError(DatabaseError):
    pass

class ProgrammingError(DatabaseError):
    pass

class NotSupportedError(DatabaseError):
    pass
########################################################################

########################################################################
## Data type classes
########################################################################
def DateFromTicks(ticks):
    return Date(*timemod.localtime(ticks)[:3])

def TimeFromTicks(ticks):
    return Time(*timemod.localtime(ticks)[3:6])

def TimestampFromTicks(ticks):
    return Timestamp(*timemod.localtime(ticks)[:6])

########################################################################
class Binary(str):
    """"""
    pass


########################################################################
## Cursor Object
########################################################################
class py4d_cursor(object):
    """"""
    arraysize = 1
    pagesize = 100

    __resulttype = None
    __prepared = False

    @property
    def rownumber(self):
        return self.__rownumber

    @property
    def description(self):
        return self.__description

    @property
    def rowcount(self):
        """"""
        return self.__rowcount

    #----------------------------------------------------------------------
    def setinputsizes(self):
        """"""
        pass

    #----------------------------------------------------------------------
    def setoutputsize(self):
        """"""
        pass

    #----------------------------------------------------------------------
    def __init__(self, connection, fourdconn, lib4d):
        """Constructor"""
        self.__rowcount = -1
        self.__description = None
        self.__rownumber = None
        self.result = None
        self.fourd_query = None

        self.fourdconn = fourdconn
        self.connection = connection
        self.lib4d_sql = lib4d._lib  #so we can address it directly

    #----------------------------------------------------------------------
    def close(self):
        """Close the database connection"""
        if self.result is not None:
            self.lib4d_sql.fourd_free_result(self.result)
        self.connection.close()
        self.__description = None
        self.__rowcount = -1
        self.__resulttype = None

    #----------------------------------------------------------------------
    def replace_nth(self, source, search, replace, n):
        """Find the Nth occurance of a string, and replace it with another."""
        i = -1
        for _ in range(n):
            i = source.find(search, i+len(search))
            if i == -1:
                return source  #return an unmodified string if there are not n occurances of value

        isinstance(source, str)
        result = "{}{}{}".format(source[:i],replace,source[i+len(search):])
        return result




    #----------------------------------------------------------------------
    def execute(self, query, params=[], describe=True):
        """Prepare and execute a database operation"""
        if self.connection.connected == False:
            raise InternalError("Database not connected")

        # if any parameter is a tuple, we need to modify the query string and
        # make multiple passes through the parameters, breaking out one tuple/list
        # each time.
        while True:
            foundtuple = False
            for idx, param in enumerate(params):
                if type(param) == list or type(param) == tuple:
                    foundtuple = True
                    paramlen = len(param)
                    query = self.replace_nth(query, "?",
                                             "({})".format(",".join("?"*paramlen)),
                                             idx+1)  #need 1 based count

                    params = tuple(params[:idx]) + tuple(param) + tuple(params[idx+1:])
                    break  #only handle one tuple at a time, otherwise the idx parameter is off.

            if not foundtuple:
                break

        if self.__prepared == False:  #Should always be false, unless we are running an executemany
            #clean up anything from a previous query, if needed.
            if self.result is not None and self.result != ffi.NULL:
                self.lib4d_sql.fourd_close_statement(self.result)

            self.fourd_query = self.lib4d_sql.fourd_prepare_statement(self.fourdconn, query)

        if self.fourd_query == ffi.NULL:
            error = ffi.string(self.lib4d_sql.fourd_error(self.fourdconn))
            raise ProgrammingError(error)

        # Some data types need special handling, but most we can just convert to a string.
        # All strings need UTF-16LE encoding.
        fourdtypes = defaultdict(lambda:self.lib4d_sql.VK_STRING,
                                 {str: self.lib4d_sql.VK_STRING,
                                  unicode: self.lib4d_sql.VK_STRING,
                                  bool: self.lib4d_sql.VK_BOOLEAN,
                                  int: self.lib4d_sql.VK_LONG,
                                  long: self.lib4d_sql.VK_LONG,
                                  float: self.lib4d_sql.VK_REAL,
                                  })

        for idx, parameter in enumerate(params):
            param_type = type(parameter)
            fourd_type = fourdtypes[param_type]

            if param_type == str or param_type == unicode:
                # Very similar to the default, but we don't have to call string on the parameter
                param = ffi.new("FOURD_STRING *")
                param.length = len(parameter)
                param.data = ffi.new("char[]", parameter.encode('UTF-16LE'))
            elif param_type == bool:
                param = ffi.new("FOURD_BOOLEAN *", parameter)
            elif param_type == int or param_type == long:
                param = ffi.new("FOURD_LONG *", parameter)
            elif param_type == float:
                param = ffi.new("FOURD_REAL *", parameter)
            elif param_type == None:
                param = ffi.NULL
            elif param_type == time:
                #almost the same as calling str(), but without milliseconds
                itemstr = parameter.strftime('%H:%M:%S')
                param = ffi.new("FOURD_STRING *")
                param.length = len(itemstr)
                param.data = ffi.new("char[]", itemstr.encode('UTF-16LE'))
            elif param_type == tuple:
                numparams = len(parameter)

                itemstr =  str(parameter)
                param = ffi.new("FOURD_STRING *")
                param.length = len(itemstr)
                param.data = ffi.new("char[]", itemstr)
            else:
                itemstr =  str(parameter)
                param = ffi.new("FOURD_STRING *")
                param.length = len(itemstr)
                param.data = ffi.new("char[]", itemstr.encode('UTF-16LE'))


            bound = self.lib4d_sql.fourd_bind_param(self.fourd_query, idx, fourd_type, param)
            if bound != 0:
                raise ProgrammingError(ffi.string(self.lib4d_sql.fourd_error(self.fourdconn)))

        #properly clean up any old results
        if self.result is not None and self.result != ffi.NULL:
            self.lib4d_sql.fourd_free_result(self.result)

        # Run the query and return the results
        self.result = self.lib4d_sql.fourd_exec_statement(self.fourd_query, self.pagesize)

        if self.result == ffi.NULL:
            raise ProgrammingError(ffi.string(self.lib4d_sql.fourd_error(self.fourdconn)))

        self.__resulttype = self.result.resultType
        if self.__resulttype == self.lib4d_sql.RESULT_SET:
            self.__rowcount = self.lib4d_sql.fourd_num_rows(self.result)
        elif self.__resulttype == self.lib4d_sql.UPDATE_COUNT:
            self.__rowcount = self.lib4d_sql.fourd_affected_rows(self.fourdconn);
        else:
            self.__rowcount = -1  # __resulttype is an enum, so this shouldn't happen.

        self.__rownumber = -1  #not on a row yet

        if describe:
            # Populate the description object
            self.__describe()

    #----------------------------------------------------------------------
    def __describe(self):
        """Populate the description object"""
        if self.result == ffi.NULL:
            return

        columncount = self.lib4d_sql.fourd_num_columns(self.result)

        description = []
        pythonTypes = {self.lib4d_sql.VK_BOOLEAN: bool,
                       self.lib4d_sql.VK_BYTE: str,
                       self.lib4d_sql.VK_WORD: str,
                       self.lib4d_sql.VK_LONG: int,
                       self.lib4d_sql.VK_LONG8: int,
                       self.lib4d_sql.VK_REAL: float,
                       self.lib4d_sql.VK_FLOAT: float,
                       self.lib4d_sql.VK_TIME: time,
                       self.lib4d_sql.VK_TIMESTAMP: datetime,
                       self.lib4d_sql.VK_DURATION: timedelta,
                       self.lib4d_sql.VK_TEXT: str,
                       self.lib4d_sql.VK_STRING: str,
                       self.lib4d_sql.VK_BLOB: Binary,
                       self.lib4d_sql.VK_IMAGE: Binary,}

        for colidx in range(columncount):
            colName = ffi.string(self.lib4d_sql.fourd_get_column_name(self.result, colidx))
            colType = self.lib4d_sql.fourd_get_column_type(self.result, colidx)
            try:
                pytype = pythonTypes[colType]
            except KeyError:
                raise OperationalError("Unrecognized 4D type: {}".format(str(colType)))

            colDescript = (colName, pytype, None, None, None, None, None)
            description.append(colDescript)

        self.__description = description

    #----------------------------------------------------------------------
    def executemany(self, query, params):
        """"""
        for paramlist in params:
            #free any memory used in the last pass.
            if self.result is not None and self.result != ffi.NULL:
                self.lib4d_sql.fourd_free_result(self.result)
                self.result = None

            self.execute(query, paramlist, describe=False)
            self.lib4d_sql.fourd_close_statement(self.result)  #close the statement
            self.__prepared = True

        #we don't run describe on the individual queries in order to be more efficent.
        self.__describe()

        #finally free any remaining memory used.
        self.lib4d_sql.fourd_free_result(self.result)
        self.result = None
        self.__prepared = False

    #----------------------------------------------------------------------
    def fetchone(self):
        """"""
        if self.connection.connected == False:
            raise InternalError("Database not connected")

        if self.__resulttype is None:
            raise DataError("No rows to fetch")

        if self.rowcount == 0 or self.__resulttype == self.lib4d_sql.UPDATE_COUNT:
            return None

        # get the next row of the result set
        #if self.rownumber >= self.result.row_count_sent - 1:
        #    return None  #no more results have been returned

        goodrow = self.lib4d_sql.fourd_next_row(self.result)
        if goodrow == 0:
            return None

        self.__rownumber = self.result.numRow

        numcols = self.lib4d_sql.fourd_num_columns(self.result);
        strlen = ffi.new("size_t*")
        inbuff = ffi.new("char*[1]")

        row=[]
        for col in range(numcols):
            fieldtype=self.lib4d_sql.fourd_get_column_type(self.result,col)
            if self.lib4d_sql.fourd_field(self.result,col)==ffi.NULL:  #shouldn't happen, really. but handle just in case.
                        row.append(None)
                        continue

            self.lib4d_sql.fourd_field_to_string(self.result, col, inbuff, strlen)
            strdata = inbuff[0]
            output = str(ffi.buffer(strdata, strlen[0])[:])
            if strdata != ffi.NULL:
                self.lib4d_sql.free(strdata)  #must call free explicitly, otherwise we leak.
                strdata = ffi.NULL

            if fieldtype==self.lib4d_sql.VK_STRING or fieldtype==self.lib4d_sql.VK_TEXT:
                row.append(output.decode('UTF-16LE'))
            elif fieldtype == self.lib4d_sql.VK_BOOLEAN:
                boolval = self.lib4d_sql.fourd_field_long(self.result, col)
                row.append(bool(boolval[0]))
            elif fieldtype == self.lib4d_sql.VK_LONG or fieldtype == self.lib4d_sql.VK_LONG8:
                intval = self.lib4d_sql.fourd_field_long(self.result, col)
                row.append(intval[0])
            elif fieldtype == self.lib4d_sql.VK_REAL or fieldtype == self.lib4d_sql.VK_FLOAT:
                row.append(float(output))
            elif fieldtype == self.lib4d_sql.VK_TIMESTAMP:
                if output == '0000/00/00 00:00:00.000':
                    dateval = None
                else:
                    try:
                        dateval = datetime(int(output[:4]), int(output[5:7]),
                                           int(output[8:10]), int(output[11:13]),
                                           int(output[14:16]), int(output[17:19]),
                                           int(output[20:23])*1000)
                        #dateval = parser.parse(output)
                    except:
                        dateval = None
                row.append(dateval)
            elif fieldtype == self.lib4d_sql.VK_DURATION:
                #milliseconds from midnight
                longval = self.lib4d_sql.fourd_field_long(self.result, col)
                durationval = timedelta(milliseconds=longval[0])
                midnight = datetime(1, 1, 1)  #we are going to ignore the date anyway
                timeval = midnight + durationval
                row.append(timeval.time())
            elif fieldtype == self.lib4d_sql.VK_BLOB or fieldtype == self.lib4d_sql.VK_IMAGE:
                field = self.lib4d_sql.fourd_field(self.result, col)
                if field != ffi.NULL:
                    field = ffi.cast("FOURD_BLOB *", field)
                    fieldlen = field.length
                    fielddata = ffi.buffer(field.data, fieldlen)[:]
                    blobbuff = Binary(fielddata)
                    row.append(blobbuff)
                else:
                    row.append(None)
            else:
                row.append(output)

        return tuple(row)

    #----------------------------------------------------------------------
    def fetchmany(self, size=arraysize):
        """"""
        if self.connection.connected == False:
            raise InternalError("Database not connected")

        if self.__resulttype is None:
            raise DataError("No rows to fetch")

        resultset = []
        for i in range(size):
            row = self.fetchone()
            if row is none:
                break
            resultset.append(row)

        return resultset

    #----------------------------------------------------------------------
    def fetchall(self):
        """"""
        if self.connection.connected == False:
            raise InternalError("Database not connected")

        if self.__resulttype is None:
            raise DataError("No rows to fetch")

        resultset = []
        while True:
            row = self.fetchone()
            if row is None:
                break
            resultset.append(row)

        return resultset

    #----------------------------------------------------------------------
    def next(self):
        """Return the next result row"""
        result = self.fetchone()
        if result is None:
            raise StopIteration
        return result

    #----------------------------------------------------------------------
    def __iter__(self):
        """"""
        return self

    #----------------------------------------------------------------------
    def __del__(self):
        """Garbage collector"""
        if self.fourd_query is not None and self.fourd_query != ffi.NULL:
            self.lib4d_sql.fourd_free_statement(self.fourd_query)



########################################################################
## Connection object
########################################################################
class py4d_connection:
    """Connection object for a 4D database"""

    #----------------------------------------------------------------------
    def __init__(self, host, user, password, database):
        """Initalize a connection object and connect to a server"""
        self.connptr = lib4d_sql.fourd_init()
        self.cursors = []
        if self.connptr == ffi.NULL:
            raise InterfaceError("Unable to intialize connection object")

        connected = lib4d_sql.fourd_connect(self.connptr,
                                            host,
                                            user,
                                            password,
                                            database,
                                            19812)
        if connected != 0:
            self.connected = False
            raise OperationalError("Unable to connect to 4D Server")
        else:
            self.connected = True

    #----------------------------------------------------------------------
    def close(self):
        """Close the connection to the 4D database"""
        if self.cursors:
            for cursor in self.cursors:
                if cursor.result is not None and cursor.result != ffi.NULL:
                    lib4d_sql.fourd_free_result(cursor.result)

        if self.connected:
            disconnect = lib4d_sql.fourd_close(self.connptr)
            if disconnect != 0:
                self.connected = False
                raise OperationalError("Failed to close connection to 4D Server")
            lib4d_sql.fourd_free(self.connptr)

        self.connected = False

    #----------------------------------------------------------------------
    def commit(self):
        """This module is not implemented with transactional functionality built-in"""
        pass

    def cursor(self):
        cursor = py4d_cursor(self, self.connptr, lib4d_sql)
        self.cursors.append(cursor)
        return cursor

#----------------------------------------------------------------------
def connect(dsn=None, user=None, password=None, host=None, database=None):
    connect_args = {}

    # make an argument dict based off of the arguments passed.
    # if a dsn is given, we need to split it up.
    if dsn is not None:
        dsn_parts = dsn.split(';')
        for part in dsn_parts:
            part = part.strip()
            part_parts = part.split("=")
            if part_parts[0] not in ['host', 'user', 'password', 'database']:
                raise ValueError("Unrecognized parameter: {}".format(part_parts[0]))

            connect_args[part_parts[0]] = part_parts[1]

    if password is not None:
        connect_args['password'] = password

    if host is not None:
        connect_args['host'] = host

    if user is not None:
        connect_args['user'] = user

    if database is not None:
        connect_args['database'] = database

    if 'host' not in connect_args:
        # Need at least a host to connect to
        raise ValueError("Host name is required")

    for key in ['user', 'password', 'database']:
        if key not in connect_args:
            connect_args[key] = ""  # use an empty string if the argument is not provided. For example, if you don't need a user and password to log in.

    # Try to connect to the database
    fourd_connection = py4d_connection(**connect_args)
    return fourd_connection