import test

system_desc = """4096 #block size in bytes
0.3  #block transfer time in milliseconds
5.6  #average rotational latency in milliseconds
10   #average seek time in milliseconds
512  #number of blocks in the buffer"""

database_desc = """1         #database file number
relation  #database file type
1024      #record size in bytes
2048      #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

2         #database file number
relation  #database file type
1024      #record size in bytes
2048      #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value
"""

access_desc = """1
1 block 1 2 0.5 0.5"""

def cb(time):
  global reads
  reads += int(granularity)
  print reads, time

granularity = 10000;
reads = 0

time = test.run([system_desc, database_desc % {'recs': 1000}, access_desc], 4, granularity, cb)

print "Takes", time, "milliseconds to run"
