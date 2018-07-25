#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <GL/gl.h>
#define USE_SDL
#ifdef USE_SDL
    #ifdef USE_SDL2
    #include <SDL2/SDL.h>
    #else
    #include <SDL/SDL.h>
    #endif
#endif

#define WIDTH 640
#define HEIGHT 480
#ifdef USE_SDL2
SDL_Window *window;
SDL_GLContext glcontext;
#endif

void setup();

namespace Slitherio {
    class Circle {
        public:
            double radius;
            double x;
            double y;
            bool isfood;
            Circle(double X, double Y, double r, bool food) {
                x = X;
                y = Y;
                radius = r;
                isfood = food;
            }
            
            bool collide(const Circle& other) const {
                return ((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y))
                    <= ((radius + other.radius) * (radius + other.radius));
            }
            
            double distance(const Circle& other) const {
                return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
            }
            
            void draw() const {
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(x, y);
                for (float i = 0.0f; i <= 2.0f * M_PI; i += 1.0f/64.0f) {
                    glVertex2f(x + sin(i) * radius, y + cos(i) * radius);
                }
                glEnd();
            }
    };
    
    class Snake {
        public:
            int size;
            double xdir;
            double ydir;
            std::vector<Circle> segments;
            Snake(double X, double Y) {
                size = 10;
                xdir = 0.0f;
                ydir = 0.0f;
                
                for (int i = 0; i < size; i++) {
                    segments.push_back(Circle(X, Y - (i << 2), 3, false));
                }
            }
            
            /* return true if we collided into the other snake, otherwise return false. */
            bool collide(const Snake& other) const {
                // exclude the first circle
                for (int i = 1; i < other.segments.size(); i++) {
                    if (other.segments[i].collide(segments[0])) {
                        return true;
                    }
                }
                
                return false;
            }
            
            bool selfcollide() const {
                // have some room, start at segment 10 (counting from zero)
                for (int i = 10; i < segments.size(); i++) {
                    if (segments[i].collide(segments[0])) {
                        return true;
                    }
                }
                
                return false;
            }
            
            bool append(const Circle& food) {
                if (!food.isfood) {
                    return false;
                }
                
                if (!food.collide(segments[0])) {
                    return false;
                }
                
                for (int i = 0; i < food.radius; i++) {
                    segments.push_back(Circle(segments[segments.size() - 1].x, segments[segments.size() - 1].y - 2, 3, false));
                }
                
                return true;
            }
            
            /* if still alive, return true; otherwise, return false. */
            bool update(std::vector<Circle>& food) {
                // wrap around
                if (segments[0].x > 1000.0) {
                    for (int i = 0; i < segments.size(); i++) {
                        segments[i].x -= 1000.0;
                    }
                } else if (segments[0].x < -1000.0) {
                    for (int i = 0; i < segments.size(); i++) {
                        segments[i].x += 1000.0;
                    }
                } else if (segments[0].y > 1000.0) {
                    for (int i = 0; i < segments.size(); i++) {
                        segments[i].y -= 1000.0;
                    }
                } else if (segments[0].y < -1000.0) {
                    for (int i = 0; i < segments.size(); i++) {
                        segments[i].y += 1000.0;
                    }
                }
                
                segments[0].x += xdir;
                segments[0].y += ydir;
                for (int i = 0; i < (segments.size() - 1); i++) {
                    if (segments[i].distance(segments[i+1]) > 4) {
                        segments[i+1].x += (segments[i].x - segments[i+1].x) / 2;
                        segments[i+1].y += (segments[i].y - segments[i+1].y) / 2;
                    }
                }
                
                if (selfcollide()) {
                    return false;
                }
                
                std::vector<size_t> deleted;
                
                for (int i = 0; i < food.size(); i++) {
                    if (append(food[i])) {
                        deleted.push_back(i);
                    }
                }
                
                int i = 0;
                while ((i = deleted.size()) != 0) {
                    food.erase(food.begin() + deleted[i-1]);
                    deleted.pop_back();
                }
                
                return true;
            }
            
            void draw() const {
                for (int i = 0; i < segments.size(); i++) {
                    segments[i].draw();
                }
            }
            
            void updateCursor(int X, int Y) {
                double x = X * 1.0 / WIDTH - 0.5;
                double y = Y * 1.0 / HEIGHT - 0.5;
                
                if ((x == 0.0) || (y == 0.0)) {
                    return;
                }
                
                double s = 1.0 / sqrt(x * x + y * y);
                
                xdir = s * x;
                ydir = -s * y;
            }
    };
}

bool pause = false;
Slitherio::Snake *me = NULL;
std::vector<Slitherio::Circle> food;

void updateCursor(int X, int Y) {
    me->updateCursor(X, Y);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glOrtho(me->segments[0].x-100.0, me->segments[0].x+100.0, me->segments[0].y-100.0, me->segments[0].y+100.0, -1.0, 1.0);
    glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
    
    me->draw();
    
    for (int i = 0; i < food.size(); i++) {
        food[i].draw();
    }
    
    #ifdef USE_SDL
        #ifdef USE_SDL2
        SDL_GL_SwapWindow(window);
        #else
        SDL_GL_SwapBuffers();
        #endif
    #else
    glutSwapBuffers();
    glutPostRedisplay();
    #endif
}

#ifdef USE_SDL
Uint32 update(Uint32 interval, void *value) {
#else
void update(int value) {
#endif
    if (pause) {
        #ifdef USE_SDL
            return interval;
        #else
            glutTimerFunc(10, update, value + 1);
            return;
        #endif
    }
    
    if (!me->update(food)) {
        setup();
    }
    
    #ifdef USE_SDL
    return interval;
    #else
    glutTimerFunc(10, update, value + 1);
    #endif
}

#ifdef USE_SDL
Uint32 addFood(Uint32 interval, void *value) {
#else
void addFood(int value) {
#endif
    if (pause) {
        #ifdef USE_SDL
        return interval;
        #else
        glutTimerFunc(500, addFood, value + 1);
        return;
        #endif
    }
    
    double x = rand() * 200.0 / RAND_MAX - 100.0;
    double y = rand() * 200.0 / RAND_MAX - 100.0;
    food.push_back(Slitherio::Circle(x, y, 1, true));
    
    #ifdef USE_SDL
    return interval;
    #else
    glutTimerFunc(500, addFood, value + 1);
    #endif
}

void setup() {
    if (me != NULL) {
        delete me;
    }
    
    me = new Slitherio::Snake(0, 0);
    food = std::vector<Slitherio::Circle>();
}

#ifdef USE_SDL
int eventThread(void *ptr) {
    while (1) {
        SDL_Event event;
        
        while (SDL_WaitEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    updateCursor(event.motion.x, event.motion.y);
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym != SDLK_q) {
                        if (event.key.keysym.sym == SDLK_p) {
                            pause = !pause;
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
#endif

int main(int argc, char **argv) {
    setup();
    #ifdef USE_SDL
    #ifndef USE_SDL2
    const SDL_VideoInfo* info = NULL;
    #endif
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error loading SDL video subsystem!: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    if (SDL_Init(SDL_INIT_TIMER) < 0) {
        std::cerr << "Error loading SDL timer subsystem!: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    #ifdef USE_SDL2
    window = SDL_CreateWindow("Slitherio",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL);
    if (window == 0) {
        std::cerr << "Error setting SDL video mode!: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    glcontext = SDL_GL_CreateContext(window);
    #else
    info = SDL_GetVideoInfo();
    if (!info) {
        std::cerr << "Error getting SDL video info!: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    if (SDL_SetVideoMode(WIDTH, HEIGHT, info->vfmt->BitsPerPixel, SDL_OPENGL) == 0) {
        std::cerr << "Error setting SDL video mode!: " << SDL_GetError() << std::endl;
        return 1;
    }
    #endif
    
    SDL_AddTimer(10, update, NULL);
    SDL_AddTimer(500, addFood, NULL);
    
    SDL_Thread *thread = SDL_CreateThread(eventThread, "eventThread", NULL);
    
    while (1) {
        SDL_Delay(30);
        display();
    }
     
    #else
    glutInit(&argc, argv);
    int window = glutCreateWindow("slitherio");
    glutPassiveMotionFunc(updateCursor);
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    
    glutTimerFunc(10, update, 0);
    glutTimerFunc(500, addFood, 0);
    
    glutDisplayFunc(display);
    
    glutMainLoop();
    #endif
    
    return 0;
}
