#include <string>
#include <vector>
#include "slitherio.pb.h"
#include <SDL2/SDL.h>

namespace slitherio {
    /* The server class. A server runs collision detection,
     * places food, etc. and accepts connections with clients.
     */
    class Server {
            /* Every snake. */
            std::vector<Snake> snakes;
            /* Every food pellet that hasn't been eaten yet. */
            std::vector<Circle> food;
        public:
            /* Update a snake. */
            bool updateSnake(Snake& snake);
            /* Update everything. */
            void update();
            Server(int port);
    };
    
    /* The client class. A client recieves (spelled right?)
     * information about it's location from the server, and
     * notifies the server about it's direction.
     */
    class Client {
            /* The client window. */
            SDL_Window *window;
            /* The rendering context from libSDL2. (OpenGL) */
            SDL_GLContext glcontext;
            /* Our snake; cached from the server. */
            Snake snake;
            /* The other nearby snakes; cached from the server. */
            std::vector<Snake> others;
            /* Nearby food; cached from the server. */
            std::vector<Circle> food;
            /* Draw a circle, whether that be a food pellet
             * or a segment.
             */
            void drawCircle(const Circle& circle);
            /* Draw a snake, made up of segments.
             * In essence, iterate over the segments and
             * draw them individually.
             */
            void drawSnake(const Snake& s);
            /* The x direction. */
            double xdir;
            /* The y direction. */
            double ydir;
        public:
            /* Update the xdir and ydir based on mouse motion
             * relative to the center of the screen.
             */
            void updateDir(int x, int y);
            /* Are we paused? true if we are, false if we aren't. */
            bool pause;
            /* Draw a new frame. Doesn't necessarily update physics,
             * or even communicate with the server.
             */
            void draw();
            /* Create a new client, given the ip and port to connect to.
             * If we don't have networking, the ip and port are going
             * to be ignored.
             */
            Client(std::string ip, int port);
    };
}
