#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include "slitherio.hpp"
#include "slitherio.pb.h"
#define WIDTH 640
#define HEIGHT 480

namespace slitherio {
    /* Did two circles collide? i.e., do they overlap or touch? */
    bool collide(const Circle& a, const Circle& b) {
        return ((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()))
        <= (a.r() + b.r()) * (a.r() + b.r());
    }
    
    bool Server::updateSnake(Snake& snake) {
        snake.mutable_segments()->Mutable(0)->set_x(snake.segments().Get(0).x() + snake.dir().x());
        snake.mutable_segments()->Mutable(0)->set_y(snake.segments().Get(0).y() + snake.dir().y());
        
        for (int i = 0; i < (snake.segments().size() - 1); i++) {
            double distx = snake.segments().Get(i).x() - snake.segments().Get(i+1).x();
            double disty = snake.segments().Get(i).y() - snake.segments().Get(i+1).y();
            
            if ((distx * distx + disty + disty) > 16) {
                snake.mutable_segments()->Mutable(i+1)->set_x(snake.segments().Get(i+1).x() + distx / 2);
                snake.mutable_segments()->Mutable(i+1)->set_y(snake.segments().Get(i+1).y() + disty / 2);
            }
        }
        
        for (int i = 10; i < snake.segments().size(); i++) {
            if (collide(snake.segments().Get(i), snake.segments().Get(0))) {
                return false;
            }
        }
        
        if (snake.segments().Get(0).x() > 1000.0) {
            for (int i = 0; i < snake.segments().size(); i++) {
                snake.mutable_segments()->Mutable(i)->set_x(snake.segments().Get(i).x() - 1000.0);
            }
        } else if (snake.segments().Get(0).x() < -1000.0) {
            for (int i = 0; i < snake.segments().size(); i++) {
                snake.mutable_segments()->Mutable(i)->set_x(snake.segments().Get(i).x() + 1000.0);
            }
        } else if (snake.segments().Get(0).y() > 1000.0) {
            for (int i = 0; i < snake.segments().size(); i++) {
                snake.mutable_segments()->Mutable(i)->set_y(snake.segments().Get(i).y() - 1000.0);
            }
        } else if (snake.segments().Get(0).y() < -1000.0) {
            for (int i = 0; i < snake.segments().size(); i++) {
                snake.mutable_segments()->Mutable(i)->set_y(snake.segments().Get(i).y() + 1000.0);
            }
        }
        
        for (int i = food.size() - 1; i >= 0; i--) {
            const Circle& s = snake.segments().Get(0);
            const Circle& f = food[i];
            if (
                ((s.x() - f.x()) * (s.x() - f.x()) + (s.y() - f.y()) * (s.y() - f.y())) < 3
            ) {
                for (double i = 0.0; i < f.r(); i += 1.0) {
                    Circle *c = snake.mutable_segments()->Add();
                    c->set_food(false);
                    c->set_x(snake.segments().Get(snake.segments().size()-1).x());
                    c->set_y(snake.segments().Get(snake.segments().size()-1).y());
                    c->set_r(5.0);
                }
                
                food.erase(food.begin() + i);
                
                // only eat one piece of food at a time
                break;
            }
        }
        
        return true;
    }
        
    void Server::update() {
        for (int i = snakes.size() - 1; i >=0; i--) {
            if (!updateSnake(snakes[i])) {
                snakes.erase(snakes.begin() + i);
            }
        }
    }
    
    Server::Server(int port) {
        
    }
    
    void Client::drawCircle(const Circle& circle) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(circle.x(), circle.y());
        for (float i = 0.0f; i <= 2.0f * M_PI; i += 1.0f/64.0f) {
            glVertex2f(circle.x() + sin(i) * circle.r(), circle.y() + cos(i) * circle.r());
        }
        glEnd();
    }
    
    void Client::drawSnake(const Snake& s) {
        for (int i = 0; i < snake.size(); i++) {
            drawCircle(s.segments().Get(i));
        }
    }
    
    void Client::draw() {
        if (pause) {
            return;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glOrtho(snake.segments().Get(0).x()-100.0, snake.segments().Get(0).x()+100.0, snake.segments().Get(0).y()-100.0, snake.segments().Get(0).y()+100.0, -1.0, 1.0);
        glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
        
        drawSnake(snake);
        
        for (int i = 0; i < food.size(); i++) {
            drawCircle(food[i]);
        }
        
        for (int i = 0; i < others.size(); i++) {
            drawSnake(others[i]);
        }
        
        SDL_GL_SwapWindow(window);
    }
    
    void Client::updateDir(int X, int Y) {
        int width;
        int height;
        SDL_GetWindowSize(window, &width, &height);
        double x = X * 1.0 / width - 0.5;
        double y = Y * 1.0 / height - 0.5;
        
        if ((x == 0.0) || (y == 0.0)) {
            return;
        }
        
        double s = 1.0 / sqrt(x * x + y * y);
        
        xdir = s * x;
        ydir = -s * y;
    }
    
    int eventThread(void *ptr);
    
    Client::Client(std::string ip, int port) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Error loading SDL video subsystem!: " << SDL_GetError() << std::endl;
            return;
        }
        
        if (SDL_Init(SDL_INIT_TIMER) < 0) {
            std::cerr << "Error loading SDL timer subsystem!: " << SDL_GetError() << std::endl;
            return;
        }
        
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        
        window = SDL_CreateWindow("Slitherio",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WIDTH, HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (window == 0) {
            std::cerr << "Error setting SDL video mode!: " << SDL_GetError() << std::endl;
            return;
        }
        
        glcontext = SDL_GL_CreateContext(window);
        
        SDL_CreateThread(eventThread, "eventThread", this);
        
        while (1) {
            if (SDL_IsScreenSaverEnabled() != pause) {
                if (pause) {
                    SDL_EnableScreenSaver();
                } else {
                    SDL_DisableScreenSaver();
                }
            }
            
            SDL_Delay(30);
            if (snake.segments().size() > 0) {
                draw();
            }
        }
    }
    
    int eventThread(void *ptr) {
        while (1) {
            SDL_Event event;
            
            while (SDL_WaitEvent(&event)) {
                switch (event.type) {
                    case SDL_MOUSEMOTION:
                        ((Client *) ptr)->updateDir(event.motion.x, event.motion.y);
                        break;
                    case SDL_KEYUP:
                        if (event.key.keysym.sym != SDLK_q) {
                            if (event.key.keysym.sym == SDLK_p) {
                                ((Client *) ptr)->pause = !((Client *) ptr)->pause;
                            }
                            break;
                        }
                    case SDL_QUIT:
                        SDL_Quit();
                        exit(0);
                        return 0;
                        break;
                }
            }
        }
        
        return 1;
    }
}

int main(int argc, char **argv) {
    if (argc > 2) {
        slitherio::Client c(std::string(argv[1]), atoi(argv[2]));
    } else if (argc > 1) {
        slitherio::Server s(atoi(argv[1]));
    }
    
    
    return 0;
}
