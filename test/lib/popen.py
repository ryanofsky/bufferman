#
# Copyright (C) 2000-2001 The ViewCVS Group. All Rights Reserved.
#
# By using this file, you agree to the terms and conditions set forth in
# the LICENSE.html file which can be found at the top level of the ViewCVS
# distribution or at http://viewcvs.sourceforge.net/license-1.html.
#
# Contact information:
#   Greg Stein, PO Box 760, Palo Alto, CA, 94302
#   gstein@lyra.org, http://viewcvs.sourceforge.net/
#
# -----------------------------------------------------------------------
#
# popen.py: a replacement for os.popen()
#
# This implementation of popen() provides a cmd + args calling sequence,
# rather than a system() type of convention. The shell facilities are not
# available, but that implies we can avoid worrying about shell hacks in
# the arguments.
#
# -----------------------------------------------------------------------
#

import os
import sys

if sys.platform == "win32":
  import win32popen

def pipe_cmds(cmds):
  """Executes a sequence of commands. The output of each command is directed to
  the input of the next command. A _pipe object is returned for writing to the
  first command's input. The output of the last command is directed to the
  standard out. On windows, if sys.stdout is not an inheritable file handle
  (i.e. it is not possible to direct the standard out of a child process to
  it), then a separate thread will be spawned to spool output to
  sys.stdout.write(). In all cases, the pipe_cmds() caller should refrain
  from writing to the standard out until the last process has terminated.
  """
  if sys.platform == "win32":
    import win32file
    
    errorOut = win32popen.MakeInheritedHandle(win32popen.FileObject2File(sys.stderr),0)
    
    first = None
    prev = 0

    for cmd in cmds[:-1]:    
      process = win32popen.popen(cmd[0], cmd[1:], 0, 0, prev, 0, errorOut)
      if prev == 0: first = process
      prev = win32popen.MakeInheritedHandle(process.stdOut)

    process = win32popen.popen(cmds[-1][0], cmds[-1][1:], 0, 0, prev, 0, errorOut);
    if prev == 0: first = process

    return _pipe(win32popen.File2FileObject(process.stdOut, 'rt'), process)

  # flush the stdio buffers since we are about to change the FD under them
  sys.stdout.flush()
  sys.stderr.flush()

  prev_r, parent_w = os.pipe()
  last_r, last_w = os.pipe()

  null = os.open('/dev/null', os.O_RDWR)

  for cmd in cmds[:-1]:
    r, w = os.pipe()
    pid = os.fork()
    if not pid:
      # in the child

      # hook up stdin to the "read" channel
      os.dup2(prev_r, 0)

      # hook up stdout to the output channel
      os.dup2(w, 1)

      # toss errors
      os.dup2(null, 2)

      # close these extra descriptors
      os.close(prev_r)
      os.close(parent_w)
      os.close(null)
      os.close(r)
      os.close(w)

      # time to run the command
      try:
        os.execvp(cmd[0], cmd)
      except:
        pass

      sys.exit(127)

    # in the parent

    # we don't need these any more
    os.close(prev_r)
    os.close(w)

    # the read channel of this pipe will feed into to the next command
    prev_r = r

  # no longer needed
  os.close(null)

  # done with most of the commands. set up the last command to write to stdout
  pid = os.fork()
  if not pid:
    # in the child (the last command)

    # hook up stdin to the "read" channel
    os.dup2(prev_r, 0)

    # hook up stdout to the output channel
    os.dup2(last_w, 1)

    # close these extra descriptors
    os.close(prev_r)
    os.close(parent_w)

    # run the last command
    try:
      os.execvp(cmds[-1][0], cmds[-1])
    except:
      pass

    sys.exit(127)

  # not needed any more
  os.close(prev_r)
  os.close(last_w)

  os.close(parent_w)
  
  # read from the last pipe, wait on the final process
  return _pipe(os.fdopen(last_r, 'rt'), pid)


class _pipe:
  "Wrapper for a file which can wait() on a child process at close time."

  def __init__(self, file, child_pid):
    self.file = file
    self.child_pid = child_pid

  def eof(self):
    if sys.platform == "win32":
      status = self.child_pid.poll()
      if status == -1: status = None
      return status

    pid, status = os.waitpid(self.child_pid, os.WNOHANG)
    if pid:
      self.file.close()
      self.file = None
      return status
    return None

  def close(self):
    if self.file:
      self.file.close()
      self.file = None
      if sys.platform == "win32":
        status = self.child_pid.wait()
        if hasattr(self.child_pid, "outLock"):
          self.child_pid.outLock.acquire()
          self.child_pid.outLock.release()
          del self.child_pid.outLock
        if status == -1: status = None
        return status
      else:
        return os.waitpid(self.child_pid, 0)[1]
    return None

  def __getattr__(self, name):
    return getattr(self.file, name)

  def __del__(self):
    if self.file:
      self.close()
