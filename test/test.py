import string
import sys
import os

# the stuff in the lib folder comes from an open source project called
# viewcvs (http://viewcvs.sourceforge.net/). They define some functions
# and classes that are useful for spawning processes
sys.path.insert(0, "./lib")
import popen

def writefile(filename, contents):
  """Write a string to a file"""
  f = open(filename, "wt")
  f.write(contents)
  f.close();

def run(parameters, seed = 1, granularity = 1000, callback = None):
  """Run the access generator and buffer manager. parameters is a sequence of
  3 strings containing the system parameters, the database parameters, and
  access pattern parameters (in that order). callback is a functor or function
  that is called when a time value is read. the return value is the
  cummulative time"""
  
  files = ["system.dsc", "database.dsc", "access.dsc"]
  map(writefile, files, parameters)
  f = popen.pipe_cmds([["../accessgen.exe", str(seed)], ["../bufferman.exe", str(granularity)]])
  lasttime = 0.0
  while 1:
    val = f.readline()
    if not val and f.eof() != None: break
    lasttime = float(string.strip(val))
    if callback: callback(lasttime)
  map(os.remove, files)
  return lasttime

