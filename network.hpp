#include <string>

namespace slitherio {
    /* Get a client network driver. */
    //NetworkDriver *getClientNetwork(bool nonet);
    /* Get a server network driver. */
    //NetworkDriver *getServerNetwork(bool nonet);
    
    /* A generic socket type. */
    typedef void Socket;
    
    template<class T> class NetworkDriver {
            int port;
            std::string addr;
        public:
            /* Are we a client driver? */
            bool isclient;
            /* Set the port for the driver. If we are a server, listen
             * on this port; if we are a client, connct to this port
             */
            virtual void setPort(int port) = 0;
            /* Set the address of this network driver. Applicable only if
             * we're a client.
             */
            virtual void setAddress(std::string addr) = 0;
            /* Start accepting connections. After we receive a connection,
             * callback bool t->*conn(Socket *).
             * After any data is received, callback bool t->*data(Socket *, [the data]).
             * The return value 'true' indicates we should continue the
             * connection. 'false' indicates the socket should be closed.
             * Probably won't return. Make sure everything's thread safe.
             */
            virtual void accept(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) = 0;
            /* Connect to a server. When connected, callback bool t->*conn(Socket *).
             * on data, callback bool t->*data(Socket *, [the data]).
             * If at any time the return value from a callback is 'false', the socket
             * connection is terminated. A return value of 'true' prevents the connection
             * from being destroyed.
             * Probably won't return. Make sure everything's thread safe.
             */
            virtual void connect(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) = 0;
            /* Close the socket. */
            virtual void close(Socket *socket) = 0;
            /* Write an std::string to the socket. */
            virtual void write(Socket *socket, const std::string& data) = 0;
    };
    
    #ifndef NO_NET
    /* The preferred driver based on Boost.Asio. If
     * compiled without networking, this driver is not
     * available, and the program can only be run locally.
     */
    //class AsioDriver: public NetworkDriver { };
    #endif
    /* A single-player driver which runs internally.
     * Nothing from the outside can connect to this driver;
     * It doesn't even implement any type of sockets.
     */
    //class NoNetDriver: public NetworkDriver { };
}
