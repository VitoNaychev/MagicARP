objects = main.o helpers.o parsers.o sock_ops.o packet_builders.o

# INCLUDE_PATH = include/

CC = gcc
CXXFLAGS = -std=c11
CFLAGS = -Wall -pedantic -Wextra -g -D_DEFAULT_SOURCE
# CPPFLAGS = -I $(INCLUDE_PATH)
LDLIBS = -L$(MYSQL_LINK_PATH)

.PHONY: clean

magicARP: $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS) $(CXXFLAGS)

$(objects): helpers.h parsers.h packet_builders.h

clean:
	rm -f $(objects) magicARP
