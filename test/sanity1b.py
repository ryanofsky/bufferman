import test

system_desc = """4096 #block size in bytes
0.3  #block transfer time in milliseconds
5.6  #average rotational latency in milliseconds
10   #average seek time in milliseconds
512    #number of blocks in the buffer"""

database_desc = """1         #database file number
relation  #database file type
256       #record size in bytes
10000     #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

2         #database file number
index     #database file type
secondary #conceptual index type
4         #attribute/key size in bytes
4         #pointer size in bytes
b+tree    #physical index type
0.69      #average slot occupancy in the B+tree
-1        #sentinel value"""

access_desc = """10000
1 lookup 2"""

granularity = 1000;
reads = 0

fp = open("sanity1b.csv", "wt");
fp.write("Operations,Time\n")

str = None

def cb(time):
  global reads, str
  reads += int(granularity)
  if (str): fp.write(str)
  str = "%i,%f\n" % (reads,time)

print test.run([system_desc, database_desc, access_desc], 4, granularity, cb)

fp.close()
