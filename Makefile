COMPILER_OPTIONS = -O3 -c -I. -DNDEBUG
#COMPILER_OPTIONS = -c -I.
LINK_OPTIONS =

all : accessgen.exe bufferman.exe

accessgen.exe : database.o general.o accessgen.o operations.o random.o
	g++ $(LINK_OPTIONS) -o $@ $^

bufferman.exe : database.o general.o bufferman.o operations.o random.o
	g++ $(LINK_OPTIONS) -o $@ $^

database.o: database.cpp general.hpp database.hpp operations.hpp tokenizer.hpp random.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

general.o: general.cpp general.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

accessgen.o: accessgen.cpp database.hpp operations.hpp general.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

bufferman.o: bufferman.cpp database.hpp operations.hpp general.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

operations.o: operations.cpp operations.hpp database.hpp general.hpp random.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

random.o: random.cpp random.hpp
	g++ $(COMPILER_OPTIONS) -o $@ $<

clean:
	rm -f accessgen.exe bufferman.exe database.o general.o accessgen.o operations.o random.o bufferman.o

dox : dox/index.html

dox/index.html: dox.txt accessgen.cpp bufferman.cpp database.cpp general.cpp operations.cpp random.cpp bufferman.hpp database.hpp general.hpp operations.hpp random.hpp tokenizer.hpp
	doxygen $<

doxclean:
	rm -f dox/*.png dox/*.html dox/*.css
