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
1 block 1 2 %(buffer1)f %(buffer2)f"""

granularity = 0;

f = open("algorithmtune.csv", "wt")

f.write("Outer Buffer Fraction,Time\n")

for i in range(0,9):
  bufferfrac = 0.1 + 0.1 * i
  ad = access_desc % {'buffer1': bufferfrac, 'buffer2': 1.0 - bufferfrac}
  print ad
  time = test.run([system_desc, database_desc, ad], 1, granularity, None)
  f.write("%f,%f\n" % (bufferfrac,time))

f.close()