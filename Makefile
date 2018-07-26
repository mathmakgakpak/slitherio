CPP ?= clang++

all:
	protoc slitherio.proto --cpp_out=.
	$(CPP) -Wall slitherio.cpp slitherio.pb.cc network.cpp -lSDL2 -lGL -lboost_system -lprotobuf -o slitherio.o
nonet:
	protoc slitherio.proto --cpp_out=.
	$(CPP) -DNO_NET -Wall slitherio.cpp slitherio.pb.cc network.cpp -lSDL2 -lGL -lprotobuf -o slitherio.o
clean:
	rm slitherio.pb.cc slitherio.pb.h slitherio.o
