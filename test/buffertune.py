import test

system_desc = """4096 #block size in bytes
0.3  #block transfer time in milliseconds
5.6  #average rotational latency in milliseconds
10   #average seek time in milliseconds
%(buffersize)s #number of blocks in the buffer"""

database_desc = """1         #database file number
relation  #database file type
1024      #record size in bytes
64       #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

2         #database file number
relation  #database file type
1024      #record size in bytes
128       #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

3         #database file number
index     #database file type
secondary #conceptual index type
8         #attribute/key size in bytes
4         #pointer size in bytes
hash      #physical index type
0.39      #average fraction of each bucket in hash table occupied
-1        #sentinel value

4         #database file number
relation  #database file type
128       #record size in bytes
4096      #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

5         #database file number
index     #database file type
primary   #conceptual index type
8         #attribute/key size in bytes
4         #pointer size in bytes
b+tree    #physical index type
0.39      #average fraction of each bucket in hash table occupied
-1        #sentinel value
"""

access_desc1 = """1000
.1 scan 4 0.5
.8 lookup 5
.05 block 1 2 0.5 0.5
.05 indexloop 1 3 2
"""

access_desc2 = """1000
.1 scan 1 0.5
.8 lookup 3
.05 block 1 2 0.5 0.5
.05 indexloop 1 3 2"""

access_desc3 = """1000
.8 scan 1 0.5
.1 lookup 3
.05 block 1 2 0.5 0.5
.05 indexloop 1 3 2"""

granularity = 0;

f = open("buffertune.csv", "wt")

f.write("Buffer Size,Pattern 1,Pattern 2,Pattern 3\n")

for buffersize in range(64,385,16):
  sd = system_desc % {'buffersize': buffersize}
  time1 = test.run([sd, database_desc, access_desc1], 1, granularity, None)
  time2 = test.run([sd, database_desc, access_desc2], 1, granularity, None)
  time3 = test.run([sd, database_desc, access_desc3], 1, granularity, None)
  f.write("%i,%f,%f,%f\n" % (buffersize,time1,time2,time3))

f.close()