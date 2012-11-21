CXXFLAGS+=-Wall -Wextra -ansi -pedantic -I$(HOME)/Sys/include
LDFLAGS+=-L$(HOME)/Sys/lib -L$(HOME)/Sys/lib64 -lyaml-cpp
CC=$(CXX)

all: yaml2csv

yaml2csv: yaml2csv.o

clean:
	rm -f yaml2csv yaml2csv.o
