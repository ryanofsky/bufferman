import test

system_desc = """4096 #block size in bytes
0.3  #block transfer time in milliseconds
5.6  #average rotational latency in milliseconds
10   #average seek time in milliseconds
512  #number of blocks in the buffer"""

database_desc = """1         #database file number
relation  #database file type
1024      #record size in bytes
%(r1size)i #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value

2         #database file number
relation  #database file type
1024      #record size in bytes
%(r2size)i #number of records
0.5       #average fraction of each block containing data records
-1        #sentinel value
"""

access_desc = """1
1 block 1 2 %(buffer1)f %(buffer2)f"""

granularity = 0;

r1size = (1024, 2048, 2096);
r2size = (1024, 2048, 2096);

f = open("algorithmtune.csv", "wt")

f.write("Outer Buffer Fraction,Database 1,Database 2,Database 3\n")

r1size = (1024,2048,4096)
r2size = (4096,2048,1024)

for i in range(0,9):
  bufferfrac = 0.1 + 0.1 * i
  f.write("%f" % bufferfrac)
  ad = access_desc % {'buffer1': bufferfrac, 'buffer2': 1.0 - bufferfrac}
  print "ad =", ad,"\n------------------------------"
  for j in range(3):
    dd = database_desc % {'r1size': r1size[j], 'r2size': r2size[j]}
    print "ad =", ad,"\n------------------------------"
    print "dd =", dd,"\n------------------------------"
    time = test.run([system_desc, dd, ad], 1, granularity, None)
    f.write(",%f" % (time))
  
  f.write("\n")





  


f.close()