objects = main.o helpers.o parsers.o sock_ops.o

# INCLUDE_PATH = include/

CC = gcc
CXXFLAGS = -std=c11
CFLAGS = -Wall -pedantic -g
# CPPFLAGS = -I $(INCLUDE_PATH)
LDLIBS = -L$(MYSQL_LINK_PATH)

.PHONY: clean

magicARP: $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS) $(CXXFLAGS)

$(objects): helpers.h parsers.h

clean:
	rm -f $(objects) magicARP
