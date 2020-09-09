TAGET=testRTSPClient
BOOST_ROOT=../../boost_1_74_0/
CXXFLAGS= -Wall -I../media/include/ -I${BOOST_ROOT}
LDFLAGS=-lpthread

CXX_SRCS = ../media/RTSPClient.cpp \
	   testRTSPClient.cpp

CXX_OBJS = $(CXX_SRCS:%.cpp=%.ox)

all:$(CXX_OBJS)
	g++ -g  ${CFLAGS} $^ $(LDFLAGS) -o ${TAGET}

$(CXX_OBJS):%.ox:%.cpp
	g++ -g $(CXXFLAGS) -o $@ -c $^


clean:
	rm $(CXX_OBJS) ${TAGET}
