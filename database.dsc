1         #database file number
relation  #database file type
120       #record size in bytes
1000      #number of records
1.0       #average fraction of each block containing data records (excluding fragmentation)
-1        #sentinel value

2         #database file number
index     #database file type
primary   #conceptual index type
4         #attribute/key size in bytes
4         #pointer size in bytes
b+tree    #physical index type
0.69      #average slot occupancy in the B+tree
-1        #sentinel value

3         #database file number
index     #database file type
secondary #conceptual index type
8         #attribute/key size in bytes
4         #pointer size in bytes
hash      #physical index type
0.5       #average fraction of each bucket in hash table occupied
-1        #sentinel value

4         #database file number
relation  #database file type
260       #record size in bytes
456       #number of records
.72       #average fraction of each block containing data records (excluding fragmentation)
-1        #sentinel value

6         #database file number
index     #database file type
primary   #conceptual index type
8         #attribute/key size in bytes
4         #pointer size in bytes
hash      #physical index type
0.39      #average fraction of each bucket in hash table occupied
-1        #sentinel value

#test cases

10        #database file number
relation  #database file type
100       #record size in bytes
150       #number of records
0.5       #average fraction of each block containing data records (excluding fragmentation)
-1        #sentinel value

12        #database file number
index     #database file type
secondary #conceptual index type
100       #attribute/key size in bytes
100       #pointer size in bytes
hash      #physical index type
0.5       #average fraction of each bucket in hash table occupied
-1        #sentinel value

13        #database file number
index     #database file type
secondary #conceptual index type
100       #attribute/key size in bytes
100       #pointer size in bytes
b+tree    #physical index type
0.5       #average fraction of each bucket in hash table occupied
-1        #sentinel value

11        #database file number
relation  #database file type
200       #record size in bytes
130      #number of records
1        #average fraction of each block containing data records (excluding fragmentation)
-1        #sentinel value

14        #database file number
relation  #database file type
200       #record size in bytes
2         #number of records
1         #average fraction of each block containing data records (excluding fragmentation)
-1        #sentinel value