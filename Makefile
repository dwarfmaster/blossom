OBJS=main.o
PROG=blossom
CXXFLAGS=-Wall -Wextra -pedantic -std=c++17 -O2
LDFLAGS=
CC=g++

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CXXFLAGS)    -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $^

clean:
	@touch $(OBJS) $(PROG)
	rm $(OBJS) $(PROG)

rec: clean all

.PHONY: all clean rec


