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

nop1 = 100
nop2 = 10
op1 = "lookup 5";
op2 = "block 1 2 0.5 0.5";

granularity = 0;

f = open("thrash.csv", "wt")
f.write("Buffer Size,Sequential Time,Mixed Time\n")

for buffersize in range(16,257,16):
  sd = system_desc % {'buffersize': buffersize}
  time1 = test.run([sd, database_desc, "%i\n1 %s" % (nop1,op1)], 1, granularity, None)
  time2 = test.run([sd, database_desc, "%i\n1 %s" % (nop2,op2)], 1, granularity, None)
  mixed_access = "%i\n%f %s\n%f %s" % (nop1+nop2,float(nop1)/(nop1+nop2),op1,float(nop2)/(nop1+nop2),op2)
  mixed = test.run([sd, database_desc, mixed_access], 1, granularity, None)
  f.write("%f,%f,%f\n" % (buffersize,time1+time2,mixed))
  
f.close()