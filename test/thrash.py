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

nop1 = 100
nop2 = 100
op1 = "block 1 2 0.7 0.3";
op2 = "lookup 3";

granularity = 0;

f = open("buffertune.csv", "wt")
f.write("Buffer Size,Pattern 1,Pattern 2,Pattern 3\n")
time1 = test.run([system_desc, database_desc, "%i\n1 %s" % (nop1,op1), 1, granularity, None)
time2 = test.run([system_desc, database_desc, "%i\n1 %s" % (nop2,op2)], 1, granularity, None)
mixed_access = "%i\n%f %s\n%f %s" % (nop1+nop2,float(nop1)/(nop1+nop2),op1,float(nop2)/(nop1+nop2),op2)
mixed = test.run([system_desc, database_desc, mixed_access], 1, granularity, None)
print "Sequential time", time1+time2
print "Mixed time", time1+time2

f.close()