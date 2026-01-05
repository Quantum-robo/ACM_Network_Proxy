CXX = g++
CXXFLAGS = -Wall -pthread

SRC = src/main.cpp \
      src/server.cpp \
      src/client_handler.cpp \
      src/http_parser.cpp \
      src/proxy_forwarder.cpp \
      src/logger.cpp \
      src/filter.cpp


INC = -Iinclude

all:
	$(CXX) $(CXXFLAGS) $(SRC) $(INC) -o proxy

clean:
	rm -f proxy
