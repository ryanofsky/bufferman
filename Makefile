#COMPILER_OPTIONS = -c -I. -O3 -dNDEBUG
COMPILER_OPTIONS = -c -I.
LINK_OPTIONS =

accessgen.exe : database.o general.o main.o operations.o random.o
	g++ $(LINK_OPTIONS) -o $@ $^

database.o: database.cpp general.hpp database.hpp operations.hpp tokenizer.hpp random.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

general.o: general.cpp general.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<


main.o: main.cpp database.hpp operations.hpp general.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<
	
operations.o: operations.cpp operations.hpp database.hpp general.hpp random.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<
	
random.o: random.cpp random.hpp                                         
	g++ $(COMPILER_OPTIONS) -o $@ $<

clean:
	rm -f accessgen.exe database.o general.o main.o operations.o random.o
