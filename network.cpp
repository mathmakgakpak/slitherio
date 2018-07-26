#include "network.hpp"
#include <string>
#ifndef NO_NET
#include <sstream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
using boost::asio::ip::tcp;
#endif
#include <SDL2/SDL.h>

namespace slitherio {
    #ifndef NO_NET
    
    template<class T> class AsioDriver: public NetworkDriver<T> {
            int port;
            std::string addr;
        public:
            bool isclient;
            
            AsioDriver(bool isc) {
                isclient = isc;
            }
            
            void setPort(int p) {
                port = p;
            }
            
            void setAddress(std::string a) {
                addr = a;
            }
            
            void dataThread(Socket *socket, T *t, bool(T::*data)(Socket *, std::string)) {
                while (1) {
                    boost::system::error_code error;
                    std::stringstream ss = std::stringstream();
                    boost::asio::read(*((tcp::socket *) socket), ss, error);
                    if (error == boost::asio::error::eof) {
                        break;
                    } else if (error) {
                        throw boost::system::system_error(error);
                    }
                    
                    if (!data(socket, ss.str())) {
                        break;
                    }
                }
            }
            
            void accept(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) {
                boost::asio::io_service io_service;
                
                tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
                
                while (1) {
                    tcp::socket socket(io_service);
                    acceptor.accept(socket);
                    
                    if (t->*conn((Socket *) &socket)) {
                        boost::thread(dataThread, (Socket *) &socket, t, data);
                    }
                }
            }
            
            void connect(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) {
                boost::asio::io_service io_service;
                
                tcp::resolver resolver(io_service);
                
                while (1) {
                    tcp::socket socket(io_service);
                    boost::asio::connect(socket, resolver.resolve(addr, "slitherio"));
                    
                    if (t->*conn((Socket *) &socket)) {
                        boost::thread(dataThread, (Socket *) &socket, t, data);
                    }
                }
            }
            
            void write(Socket *socket, const std::string& data) {
                boost::asio::write(*((tcp::socket *) socket), std::stringstream(data));
            }
    };
    
    #endif
    
    template<class T> class NoNetDriver: public NetworkDriver<T> {
            int port;
            std::string addr;
            static NoNetDriver *server;
            static NoNetDriver *client;
        public:
            bool hasdata;
            std::string data;
            bool isclient;
            NoNetDriver(bool isclient) {
                if (isclient) {
                    client = this;
                } else {
                    server = this;
                }
            }
            
            void setPort(int p) {
                port = p;
            }
            
            void setAddress(std::string a) {
                addr = a;
            }
            
            void accept(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) {
                t->*conn((Socket *) NULL);
                while (1) {
                    while (!hasdata) {
                        SDL_Delay(1);
                    }
                    hasdata = false;
                    t->*data((Socket *) NULL, data);
                }
            }
            
            void connect(T *t, bool(T::*conn)(Socket *), bool(T::*data)(Socket *, std::string)) {
                t->*conn((Socket *) NULL);
                while (1) {
                    while (!hasdata) {
                        SDL_Delay(1);
                    }
                    hasdata = false;
                    t->*data((Socket *) NULL, data);
                }
            }
            
            void close(Socket *socket) {
                return;
            }
            
            void write(Socket *socket, const std::string& data) {
                if (isclient) {
                    while (server->hasdata) {
                        SDL_Delay(1);
                    }
                    server->data = data;
                    server->hasdata = true;
                } else {
                    while (client->hasdata) {
                        SDL_Delay(1);
                    }
                    client->data = data;
                    client->hasdata = false;
                }
            }
    };
    
    /* Find an available client driver; if nonet
     * is true, attempt to use a nonet driver.
     * if nonet is false, try to use the Asio
     * driver. nonet has no effect if compiled without
     * networking; the NoNetDriver will always be
     * chosen.
     */
    /*NetworkDriver *getClientNetwork(bool nonet) {
        #ifdef NO_NET
        if (true) {
        #else
        if (nonet) {
        #endif
            return new NoNetDriver(true);
        } else {
            #ifndef NO_NET
            return new AsioDriver(true);
            #endif
        }
    }*/
    
    /* See getClientNetwork. The only difference is that
     * we act as a server.
     */
    
    /*NetworkDriver *getServerNetwork(bool nonet) {
        #ifdef NO_NET
        if (true) {
        #else
        if (nonet) {
        #endif
            return new NoNetDriver(false);
        } else {
            #ifndef NO_NET
            return new AsioDriver(false);
            #endif
        }
    }*/
}
