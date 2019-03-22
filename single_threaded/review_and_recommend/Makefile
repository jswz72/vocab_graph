cc = "$(shell which g++)" 
flags = -I. -fopenmp -march=athlon64 -O3
flags += -std=c++11

ifeq ($(debug), 1)
	flags+= -DDEBUG 
endif

objs = $(patsubst %.cpp,%.o,$(wildcard ../../lib/*.cpp)) \
			$(patsubst %.cpp,%.o,$(wildcard *.cpp))

dep=review_and_recommend.cpp utils.cpp

default: review recommend

review:
	$(cc) review.cpp ${dep} $(flags) -o review.bin

recommend:
	$(cc) recommend.cpp ${dep} $(flags) -o recommend.bin

clean:
	rm -rf $(exe) $(objs) 
