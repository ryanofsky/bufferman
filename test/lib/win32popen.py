# all-singing, all-dancing python popen class for windows
#
# Russ Yanofsky -- rey4@columbia.edu

# Set to 1 to show the commands being executed
DEBUG = 0

import win32process
import win32security
import win32pipe
import win32con
import win32event
import win32file
import os
import msvcrt
import win32api
import string
import sys

class popen:

  """win32popen.popen is Windows implementation of the features in
  Python's popen2.Popen3 and popen2.Popen4 classes. These do not work on
  windows because they use os.fork procedure which is only implemented on unix
  systems. This class spawns new processes, attaches their standard inputs and
  outputs to pipes, and makes those pipes accessible via their win32 handles.
  Two helper functions File2FileObject and DuplicateHandle can be used
  to wrap the pipes with standard Python File objects. The class has these
  members:
  
  stdIn (pywintypes.HANDLE object)
  
    win32 writeable file handle for the new process's standard input
  
  stdOut (pywintypes.HANDLE object)
  
    win32 readable file handle for the new process's standard output. If the
    combineouterr parameter of the contructor is true, this standard error
    output is also read through this handle
  
  stdErr (pywintypes.HANDLE object OR 0)
  
    win32 readable file handle for the new process's standard error stream.
    Or, if the combineouterr parameter of the contructor is true, then this
    variable is just set to 0.
    
  phandle (pywintypes.HANDLE object)
  
    win32 handle that references the new process. 
    
  pid (integer)
  
    process id. This is an integer that uniquely identifies the process over its lifetime. 

  thandle (pywintypes.HANDLE object)
    
    win32 handle that references the primary thread of the new process. If the
    process is created in a suspended state you should call
    win32process.ResumeThread() with it as an argument to start up the program.
    
  tid (integer)
   
    thread id of the primary thread of the new process.  
  """

  def __init__(self, file, args, combineouterr = 0, suspended = 0, hStdInput = 0, hStdOutput = 0, hStdError = 0):

    """Constructor. Arguments:
    
       file (string)
         
         path to executeable
         
       args (tuple or list of string)
       
         arguments that will be passed to the executable
         
       combineouterr (boolean)
       
         If true, the stdErr attribute is not available and standard error output
         is mixed in with the standard output. Default is false.
      
       suspended (boolean)
       
         If true, the process is created in a suspended state.
         win32process.ResumeThread must be called on the primary thread handle
         to start up the process. Default is false.
         
       hStdInput, hStdOutput, hStdError
       
         The last three arguments are win32 read, write, and write handles
         respectively. They can be used to connect files or 
    """  
    
    cmd = "\"" + string.replace(file, "\"", "\"\"") + "\"";
    for arg in args:
      cmd += " \"" + string.replace(arg, "\"", "\"\"") + "\""

    debugout("win32popen executing this command:\n  " + cmd)
    
    # initialize new process's security attributes
    sa = win32security.SECURITY_ATTRIBUTES()
    sa.Initialize()
    sa.bInheritHandle = 1

    # initialize new process's startup info
    si = win32process.STARTUPINFO() 
    si.dwFlags = win32process.STARTF_USESTDHANDLES 

    # create pipes if neccessary
    
    if hStdInput:
      si.hStdInput, self.stdIn = hStdInput, 0
    else:
      si.hStdInput, self.stdIn = win32pipe.CreatePipe(sa, 0) 
      self.stdIn = MakePrivateHandle(self.stdIn)
   
    if hStdOutput:
      self.stdOut, si.hStdOutput = 0, hStdOutput
    else:
      self.stdOut, si.hStdOutput = win32pipe.CreatePipe(sa, 0) 
      self.stdOut = MakePrivateHandle(self.stdOut)

    if combineouterr:
      self.stdErr, si.hStdError = 0, si.hStdOutput
    elif hStdError:
      self.stdErr, si.hStdError = 0, hStdError
    else:
      self.stdErr, si.hStdError = win32pipe.CreatePipe(sa, 0)
      self.stdErr = MakePrivateHandle(self.stdErr)
    
    # initialize creation flags
    cf = win32con.NORMAL_PRIORITY_CLASS
    if suspended: cf = cf | win32con.CREATE_SUSPENDED
    
    # create the process
    self.phandle, self.pid, self.thandle, self.tid = win32process.CreateProcess \
    (
      None,                            # appName
      cmd,                             # commandLine
      sa,                              # processAttributes
      sa,                              # threadAttributes
      1,                               # bInheritHandles
      win32con.NORMAL_PRIORITY_CLASS,  # dwCreationFlags
      None,                            # newEnvironment 
      None,                            # currentDirectory
      si                               # startupinfo 
    )
    
  def poll(self):

    """poll() functions exactly like Popen3::poll(). If the process has terminated
    it returns the process exit code, otherwise it returns -1.
    """

    code = win32process.GetExitCodeProcess(self.phandle)
    if code == win32con.STILL_ACTIVE: code = -1
    return code
    
  def wait(self, timeout = win32event.INFINITE):
    """poll() functions almost exactly like Popen3::wait(). It waits for the process
    to terminate and then returns its exit code. The difference is that this function
    takes an optional parameter called timeout that will cause the function to return
    -1 if the specified number of milliseconds elapses and the process still has not
    terminated.
    """

    win32event.WaitForSingleObject(self.phandle, timeout)
    return self.poll()
  

def FileObject2File(fileObject):
  return win32file._get_osfhandle(fileObject.fileno())

def File2FileObject(pipe, mode):
  """
  This takes a Win32 file handle and returns a Python file object that can
  be used instead of the handle. Example:

    myproc = win32popen.popen("l:/test.exe")
    myproc.wait()
    readobj = win32popen.File2FileObject(myproc.stdOut,'r')
    print readobj.readlines()

  
  The HANDLE object that is passed in as an argument will be invalidated and made
  no longer usable. This is done to prevent having separate references to the 
  same handle in both the HANDLE and FILE objects. If two objects were allowed 
  to store the same handle then the handle would be closed as soon as either
  of the objects was destroyed and errors would be triggered when the remaining
  object tried to use (or close) its copy of the handle. If you need to be able
  to access the original HANDLE object and the new file object you should call
  this function with a duplicate handle returned from the Duplicate Handle method.
  
  The following example shows how to use the DuplicateHandle method:
  
    myproc = win32popen.popen("l:/test.exe")
    myproc.wait()
    readobj = win32popen.File2FileObject(
      win32popen.DuplicateHandle(myproc.stdOut),
      'r'
    )
    
  In the example the result of DuplicateHandle(myproc.stdOut) instead of
  myproc.stdOut was passed to File2FileObject. This way myproc.stdOut is
  not invalidated by File2FileObject. It can still be used by Win32 file
  handling functions or be passed to File2FileObject to create another
  Python file object.
  """ 
  
  if mode.find('r') >= 0:
    wmode = os.O_RDONLY
  elif mode.find('w') >= 0:
    wmode = os.O_WRONLY
  if mode.find('b') >= 0:
    wmode = wmode | os.O_BINARY
  if mode.find('t') >= 0:
    wmode = wmode | os.O_TEXT

  # obj = os.fdopen(win32file._open_osfhandle(pipe,wmode),mode)
  return os.fdopen(msvcrt.open_osfhandle(pipe.Detach(),wmode),mode)

def DuplicateHandle(handle):
  """Duplicates a win32 handle."""
  proc = win32api.GetCurrentProcess()
  return win32api.DuplicateHandle(proc,handle,proc,0,0,win32con.DUPLICATE_SAME_ACCESS)

def MakePrivateHandle(handle, replace = 1):
  """Turn an inherited handle into a non inherited one. This avoids the implicit handle
  duplication occurs on CreateProcess calls which, among other thing, can create
  uncloseable pipes."""
  flags = win32con.DUPLICATE_SAME_ACCESS;
  proc = win32api.GetCurrentProcess()
  if replace: flags |= win32con.DUPLICATE_CLOSE_SOURCE
  newhandle = win32api.DuplicateHandle(proc,handle,proc,0,0,flags)
  if replace: handle.Detach() # handle was already deleted by the last call
  return newhandle

def MakeInheritedHandle(handle, replace = 1):
  """Turn a private handle into an inherited one."""
  flags = win32con.DUPLICATE_SAME_ACCESS;
  proc = win32api.GetCurrentProcess()
  if replace: flags |= win32con.DUPLICATE_CLOSE_SOURCE
  newhandle = win32api.DuplicateHandle(proc,handle,proc,0,1,flags)
  if replace: handle.Detach() # handle was deleted by the last call
  return newhandle

if DEBUG:
  def debugout(text):
    print text
    sys.stdout.flush()
else:  
  def debugout(text):
    pass  
    
#   Um... Extraneous Documentation
#       
#   Win32 handles are essentially pointers to internal operating system 
#   resources. Windows tracks handles to determine when resources should be 
#   created and destroyed. All handles to should be closed when they are no 
#   longer in use and no handles should be used after they have been closed. 
#   Basically, normal resource management procedures apply. 
