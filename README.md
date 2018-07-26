# slitherio
[![Build Status](https://travis-ci.org/dunnousername/slitherio.svg?branch=master)](https://travis-ci.org/dunnousername/slitherio)

Totally not related to that other game...
Just a quick project I put together. It only needs SDL2, protobuf and OpenGL headers (and Boost.Asio and Boost.Thread if you want networking) to be built.

Build with:
```
make
```

If you get strange build errors about the ```-E``` flag, try compiling with:

```
CPP=g++ make
```

or

```
CPP=clang++ make
```
.

If you'd like to build without networking, append ```nonet``` to the command, like

```
CPP=g++ make nonet
```

The client takes two arguments; an IP address and a port number.
The server takes one argument; the port number.
```nonet``` requires no arguments.

If you give one argument, it runs the server, and if you give two arguments, it runs the client (or in nonent, anything works).

Use the P key to pause, and the Q key to quit, in the client.
