import test

system_desc = """4096 #block size in bytes
0.3  #block transfer time in milliseconds
5.6  #average rotational latency in milliseconds
10   #average seek time in milliseconds
128  #number of blocks in the buffer"""

database_desc = """1         #database file number
relation  #database file type
1024      #record size in bytes
512       #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

2         #database file number
relation  #database file type
1024      #record size in bytes
512       #number of records
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
"""

access_desc1 = """1
1 block 1 2 0.5 0.5"""

access_desc2 = """1
1 indexloop 1 3 4.5"""

granularity = 0;

time1 = test.run([system_desc, database_desc, access_desc1], 4, granularity, None)
time2 = test.run([system_desc, database_desc, access_desc2], 4, granularity, None)

print "Nested block time is", time1
print "Index loop time is", time2