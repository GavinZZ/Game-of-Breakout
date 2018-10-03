// CS 349 Fall 2018
// A1: Breakout code sample
// You may use any or all of this code in your assignment!
// See makefile for compiling instructions

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <list>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

// X11 structures
Display* display;
Window window;

// fixed frames per second animation
int FPS = 60;
int inputFPS = 60;
int currentScore = 0;
int inputSpeed = 4;
int speed = 4;
bool godMode = false;
bool isTestMode = false;
int radius = 5;
XColor red, orange, yellow, green, blue;
Colormap cmap;
XColor paddleColor;

/*
 * Information to draw on the window.
 */
struct XInfo {
    Display*  display;
    Window   window;
    GC       gc;
};

class Displayable {
public:
    virtual void paint(XInfo& xinfo) = 0;
};

/*
 * A text displayable
 */
class Text : public Displayable {
public:
    virtual void paint(XInfo& xinfo) {
        XDrawImageString( xinfo.display, xinfo.window, xinfo.gc,
                         this->x, this->y, this->s.c_str(), this->s.length() );
    }
    
    void setString(string newStr) {
        s = newStr;
    }
    
    // constructor
    Text(int x, int y, string s): x(x), y(y), s(s)  {}
    
private:
    int x;
    int y;
    string s; // string to show
};

class Rect {
public:
    int x;
    int y;
    int width;
    int height;
    Rect(int x, int y, int width, int height):
    x(x), y(y), width(width), height(height) {};
};

// get current time
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

bool cornerRange(int a, int b) {
    if (a <= b + 5&& a >= b - 5) {
        return true;
    }
    return false;
}

bool withInRange(int a, int b) {
    if (a <= b + speed/2 && a >= b - speed/2) {
        return true;
    }
    return false;
}

bool checkCorner(XPoint ballPos, XPoint& ballDir, XPoint brick) {
    if (cornerRange(ballPos.x + radius, brick.x) && cornerRange(ballPos.y + radius, brick.y)) {
        // upper left corner
        if (ballDir.x > 0 && ballDir.y > 0) {
            ballDir.x = -ballDir.x;
            ballDir.y = -ballDir.y;
            return true;
        }
        if (ballDir.y > 0) {
            ballDir.y = -ballDir.y;
        } else {
            ballDir.x = -ballDir.x;
        }
        return true;
    } else if (cornerRange(ballPos.x + radius, brick.x) && cornerRange(ballPos.y - radius, brick.y + 50)) {
        // bottom left corner
        if (ballDir.x > 0 && ballDir.y < 0) {
            ballDir.x = -ballDir.x;
            ballDir.y = -ballDir.y;
            return true;
        }
        if (ballDir.y > 0) {
            ballDir.x = -ballDir.x;
        } else {
            ballDir.y = -ballDir.y;
        }
        return true;
    } else if (cornerRange(ballPos.x - radius, brick.x + 110) && cornerRange(ballPos.y - radius, brick.y + 50)) {
        // bottom right corner
        if (ballDir.x < 0 && ballDir.y < 0) {
            ballDir.x = -ballDir.x;
            ballDir.y = -ballDir.y;
            return true;
        }
        if (ballDir.y > 0) {
            ballDir.x = -ballDir.x;
        } else {
            ballDir.y = -ballDir.y;
        }
        return true;
    } else if (cornerRange(ballPos.x - radius, brick.x + 110) && cornerRange(ballPos.y + radius, brick.y)) {
        // upper right corner
        if (ballDir.x < 0 && ballDir.y > 0) {
            ballDir.x = -ballDir.x;
            ballDir.y = -ballDir.y;
            return true;
        }
        if (ballDir.y > 0) {
            ballDir.y = -ballDir.y;
        } else {
            ballDir.x = -ballDir.x;
        }
        return true;
    }
    return false;
}

void checkBoundary(XPoint ballPos, vector<XPoint>& brickList, XPoint& ballDir) {
    for (int i = 0; i < brickList.size(); i ++) {
        XPoint brick = brickList[i];
        if (ballPos.x >= brick.x && ballPos.x <= brick.x + 110) {
            if ((withInRange(ballPos.y + radius, brick.y) && ballDir.y > 0) ||
                (withInRange(ballPos.y - radius, brick.y + 50) && ballDir.y < 0)) {
                ballDir.y = -ballDir.y;
                brickList.erase(brickList.begin() + i);
                currentScore += 1;
                i --;
				continue;
            }
        }
		if (ballPos.y >= brick.y && ballPos.y <= brick.y + 50) {
            if ((withInRange(ballPos.x + radius, brick.x) && ballDir.x > 0) ||
                (withInRange(ballPos.x - radius, brick.x + 110) && ballDir.x < 0)) {
                ballDir.x = -ballDir.x;
                brickList.erase(brickList.begin() + i);
                currentScore += 1;
                i --;
				continue;
            }
        }
        bool position = checkCorner(ballPos, ballDir, brick);
        if (position) {
            brickList.erase(brickList.begin() + i);
            currentScore += 1;
            i --;
        }
    }
}

void randomColor() {
    int dice_roll = rand() % 5 + 1;
    if (dice_roll == 1) {
        paddleColor = red;
    } else if (dice_roll == 2) {
        paddleColor = orange;
    } else if (dice_roll == 3) {
        paddleColor = yellow;
    } else if (dice_roll == 4) {
        paddleColor = green;
    } else if (dice_roll == 5) {
        paddleColor = blue;
    } 
}

void checkRectBoundary(XPoint ballPos, Rect& rectPos, XPoint& ballDir, int rectWidth, int rectHeight) {
    if (withInRange(ballPos.y + 5, rectPos.y) &&
        ballPos.x + 5 >= rectPos.x &&
        ballPos.x - 5 <= rectPos.x + rectWidth) {
        if ((withInRange(ballPos.x + 3, rectPos.x) && ballDir.x > 0) ||
            (withInRange(ballPos.x - 3, rectPos.x + rectWidth) && ballDir.x < 0)) {
            ballDir.x = -ballDir.x;
        }
        ballDir.y = -ballDir.y;
        randomColor();
        return;
    } else if (ballPos.y >= rectPos.y && ballPos.y <= rectPos.y + rectPos.height &&
        ((withInRange(ballPos.x + 5, rectPos.x) && ballDir.x > 0) ||
         (withInRange(ballPos.x - 5, rectPos.x + rectWidth) && ballDir.x < 0))) {
        ballDir.x = -ballDir.x;
        randomColor();
        return;
    }
}

vector<XPoint> generateBricks(int brickWidth = 110, int brickHeight = 50) {
    vector<XPoint> brickList;
    XPoint brick;
    brick.x = 40;
    brick.y = 80;
    for (int i = 0; i < 5; i ++) {
        for (int j = 0; j < 10; j ++) {
            brickList.push_back(brick);
            brick.x += brickWidth + 10;
        }
        brick.x = 40;
        brick.y += brickHeight + 10;
    }
    return brickList;
}

void repaint(list<Text*>& dList, vector<XPoint>& brickList, XInfo& xinfo,
             Rect& rectPos, XPoint& ballDir, int brickWidth = 110, int brickHeight = 50) {
    
    if (brickList.size() > 0) {
        for (int i = 0; i < brickList.size(); i ++) {
            if (i < 10) {
                XSetForeground(xinfo.display, xinfo.gc, red.pixel);
            } else if (i < 20) {
                XSetForeground(xinfo.display, xinfo.gc, orange.pixel);
            } else if (i < 30) {
                XSetForeground(xinfo.display, xinfo.gc, yellow.pixel);
            } else if (i < 40) {
                XSetForeground(xinfo.display, xinfo.gc, green.pixel);
            } else if (i < 50) {
                XSetForeground(xinfo.display, xinfo.gc, blue.pixel);
            }
            XFillRectangle(display, window, xinfo.gc, brickList[i].x, brickList[i].y, brickWidth, brickHeight);
        }
    } else {
		if (!isTestMode) {
        cout << "Level Difficulty Increased!" << endl;
			if (speed < 8) {
				speed += 1;
				cout << "Current Speed is " + to_string(speed) << endl;
				ballDir.y > 0? ballDir.y = speed : ballDir.y = -speed;
				ballDir.x > 0? ballDir.x = speed : ballDir.x = -speed;
			} else if (rectPos.width > 10) {
				cout << "Shortening the rect" << endl;
				rectPos.width -= 10;
			} else {
				cout << "You have reached the max difficulty" << endl;;
			} 
		}
        brickList = generateBricks();
    }

    // draw rectangle
    XSetForeground(xinfo.display, xinfo.gc, paddleColor.pixel);
    XFillRectangle(xinfo.display, xinfo.window, xinfo.gc, rectPos.x, rectPos.y, rectPos.width, rectPos.height);

    XSetForeground(xinfo.display, xinfo.gc, BlackPixel(xinfo.display, DefaultScreen( xinfo.display )));
    
    list<Text*>::const_iterator begin = dList.begin();
    list<Text*>::const_iterator end = dList.end();
    
    while ( begin != end ) {
        Text* d = *begin;
        if (begin == dList.begin()) {
            d->setString("Score: " + to_string(currentScore));
        }
        d->paint(xinfo);
        begin++;
    }
}

void error( string str ) {
    cerr << str << endl;
    exit(0);
}

void initX(int argc, char* argv[], XInfo& xinfo) {
    
    /*
     * Display opening uses the DISPLAY  environment variable.
     * It can go wrong if DISPLAY isn't set, or you don't have permission.
     */
    xinfo.display = XOpenDisplay( "" );
    if ( !xinfo.display ) {
        error( "Can't open display." );
    }
    
    /*
     * Find out some things about the display you're using.
     */
    // DefaultScreen is as macro to get default screen index
    int screen = DefaultScreen( xinfo.display );
    
    unsigned long white, black;
    white = XWhitePixel( xinfo.display, screen );
    black = XBlackPixel( xinfo.display, screen );
    
    xinfo.window = XCreateSimpleWindow(xinfo.display, DefaultRootWindow(xinfo.display),
                            10, 10, 1280, 700, 2, black, white);

	XSizeHints* sh = XAllocSizeHints();
	sh->flags = PMinSize | PMaxSize;
	sh->min_width = sh->max_width = 1280;
	sh->min_height = sh->max_height = 700;
	XSetWMNormalHints(xinfo.display, xinfo.window, sh);

    // extra window properties like a window title
    XSetStandardProperties(
       xinfo.display,    // display containing the window
       xinfo.window,   // window whose properties are set
       "displaylist",  // window's title
       "DL",       // icon's title
       None,       // pixmap for the icon
       argv, argc,     // applications command line args
       None );         // size hints for the window

    /*
     * Put the window on the screen.
     */
    XSelectInput(xinfo.display, xinfo.window, ButtonPressMask | KeyPressMask);
    XMapRaised( xinfo.display, xinfo.window );
    
    XFlush(xinfo.display);
    
    // give server time to setup
    sleep(1);
}

// entry point
int main( int argc, char *argv[] ) {

    int desiredDistance = 60 * 4;
    
    if (argc == 2) {
        inputFPS = atoi(argv[1]);
        int inputSpeed = desiredDistance / inputFPS;
    } else if (argc == 3) {
        inputFPS = atoi(argv[1]);
        inputSpeed = atoi(argv[2]) * 60 / inputFPS;
        if (inputSpeed > 25) {
            inputSpeed = 25;
        }
    }
    FPS = inputFPS;
	speed = inputSpeed;
    
    XInfo xinfo;
    
    initX(argc, argv, xinfo);
    
    display = xinfo.display;
    window = xinfo.window;
    
    GC gc = XCreateGC(display, window, 0, 0);
    int screen = DefaultScreen( xinfo.display );
    XSetForeground(xinfo.display, gc, BlackPixel(xinfo.display, screen));
    XSetBackground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
  
    XWindowAttributes w;
    XGetWindowAttributes(display, window, &w);
    XSetFillStyle(display, gc, FillSolid);

    xinfo.gc = gc;
 
	// ball postition, size, and velocity
	XPoint ballPos;
	ballPos.x = 670;
	ballPos.y = 660;
	int ballSize = 10;
    
    XPoint ballDir;
    ballDir.x = 0;
    ballDir.y = 0;
    
    bool isMainPage = true;
    bool isGameStarted = false;

    // block position, size
    // block current size is 180 * 5
    Rect rectPos = Rect(580, 680, 180, 5);

    // bricks to be cleared
    vector<XPoint> brickList;
    int brickWidth = 110;
    int brickHeight = 50;

    brickList = generateBricks();
    
    // Generate Text
    list<Text*> dList;
    
    dList.push_back(new Text(20, 20, "Score: " + to_string(currentScore)));
    dList.push_back(new Text(500, 20, "Isn't it amazing that blocks can change their colors?"));
    dList.push_back(new Text(20, 500, "Press <space> to start the game"));
    dList.push_back(new Text(20, 520, "Press <a> to move paddle to the left"));
    dList.push_back(new Text(20, 540, "Press <d> to move paddle to the right"));
    dList.push_back(new Text(20, 560, "Press <r> to return to the main menu"));
    dList.push_back(new Text(20, 580, "Press <g> to open god mode"));
    dList.push_back(new Text(20, 600, "Press <q> to quit the game"));
    dList.push_back(new Text(20, 40, "FPS: " + to_string(FPS)));
    dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));

    
	// save time of last window paint
	unsigned long lastRepaint = 0;
    
    // DOUBLE BUFFER
    // create bimap (pximap) to us a other buffer
    int depth = DefaultDepth(display, DefaultScreen(display));
    Pixmap buffer = XCreatePixmap(display, window, w.width, w.height, depth);
    bool useBuffer = true;
	bool gameOver = false;
   
    list<Text*> mainPage;
    list<Text*> mainMenu;
    list<Text*> endGameMenu;
    
    mainMenu.push_back(new Text(350, 150, "Welcome to the CS349: the Game of Breakout"));
    mainMenu.push_back(new Text(350, 200, "Developed by Yuanhao Zhang(20658228)"));
    mainMenu.push_back(new Text(350, 250, "Press <n> to start the game with normal mode."));
    mainMenu.push_back(new Text(350, 300, "Press <h> to start the game with hard mode."));
    mainMenu.push_back(new Text(350, 350, "Press <i> to start the game with impossible mode. Note that this will override custom speed/FPS."));
	mainMenu.push_back(new Text(350, 400, "Press <t> to start the game with test mode (for TA use only). Note that if no custom speed/FPS specified, the speed is set to 20."));
    mainMenu.push_back(new Text(350, 450, "Press <q> to quit the game."));
    mainMenu.push_back(new Text(350, 500, "Note that every successful clear of all bricks will generate new levels with increased speed and shorter brick width."));

    endGameMenu.push_back(new Text(500, 200, "Game Over!"));
    endGameMenu.push_back(new Text(500, 250, "Press <r> to return to the main menu."));
    endGameMenu.push_back(new Text(500, 300, "Press <q> to quit the game."));
    
    mainPage = mainMenu;

    cmap = XDefaultColormap(xinfo.display, DefaultScreen(xinfo.display));
    XAllocNamedColor(display, cmap, "red", &red, &red);
    XAllocNamedColor(display, cmap, "orange", &orange, &orange);
    XAllocNamedColor(display, cmap, "yellow", &yellow, &yellow);
    XAllocNamedColor(display, cmap, "green", &green, &green);
    XAllocNamedColor(display, cmap, "blue", &blue, &blue);

	// event handle for current event
	XEvent event;

	// event loop
	while ( true ) {

		// process if we have any events
		if (XPending(display) > 0) { 
			XNextEvent( display, &event ); 

			switch ( event.type ) {

				// mouse button press
				case ButtonPress:
					cout << "CLICK" << endl;
					break;
                    
				case KeyPress: // any keypress
					KeySym key;
					char text[10];
					int i = XLookupString( (XKeyEvent*)&event, text, 10, &key, 0 );
                    
                    if (!isMainPage) {
                        
                        // clear all bricks
                        if ( i == 1 && text[0] == 'c' && isGameStarted) {
                            brickList.clear();
                            dList.pop_back();
                            dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));
                        }
                        
                        // shoot the ball
                        if ( i == 1 && text[0] == ' ') {
                            if (!isGameStarted) {
                                ballDir.x = speed;
                                ballDir.y = -speed;
                                isGameStarted = true;
                            }
                        }
                        
                        // bomb effects
                        if ( i == 1 && text[0] == 'g') {
                            cout << "God mode turned on" << endl;
                            godMode = true;
                        }

                        // move right
                        if ( i == 1 && text[0] == 'd' ) {
                            if (rectPos.x + 40 <= w.width - rectPos.width) {
                                rectPos.x += 40;
                                isGameStarted ? 0 : ballPos.x += 40;
                            } else if (!isTestMode) {
                                rectPos.x = w.width - rectPos.width;
                                if (!isGameStarted) {
                                    ballPos.x = w.width - rectPos.width/2;
                                }
                            }
                        }

                        // move left
                        if ( i == 1 && text[0] == 'a' ) {
                            if (rectPos.x - 40 >= 0) {
                                rectPos.x -= 40;
                                isGameStarted ? 0 : ballPos.x -= 40;
                            } else if (!isTestMode) {
                                rectPos.x = 0;
                                if (!isGameStarted) {
                                    ballPos.x = 90;
                                }
                            }
                        }
                        
                        if ( i == 1 && text[0] == 'r' ) {
                            useBuffer = true;
                            currentScore = 0;
                            ballPos.x = 670;
                            ballPos.y = 660;
                            rectPos.x = 580;
                            rectPos.y = 680;
                            rectPos.width = 180;
                            ballDir.x = 0;
                            ballDir.y = 0;
                            brickList = generateBricks();
                            mainPage = mainMenu;
                            godMode = false;
							isTestMode = false;
                            isMainPage = true;
                            isGameStarted = false;
                        }

                    } else {
    
                        if ( i == 1 && text[0] == 'n' && !gameOver) {
                            isMainPage = false;
                            useBuffer = false;
                            speed = 4;
                            FPS = 60;
                            rectPos.width = 180;
                            rectPos.x = w.width / 2 - rectPos.width / 2;
							ballPos.x = rectPos.x + rectPos.width / 2;
                            dList.pop_back();
                            dList.pop_back();
                            dList.push_back(new Text(20, 40, "FPS: " + to_string(FPS)));
                            dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));
                        }
                        
                        if ( i == 1 && text[0] == 'h' && !gameOver) {
                            isMainPage = false;
                            useBuffer = false;
                            rectPos.width = 100;
                            speed = 6;
                            FPS = 60;
                            rectPos.x = w.width / 2 - rectPos.width / 2;
							ballPos.x = rectPos.x + rectPos.width / 2;
                            dList.pop_back();
                            dList.pop_back();
                            dList.push_back(new Text(20, 40, "FPS: " + to_string(FPS)));
                            dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));
                        }

                        if ( i == 1 && text[0] == 'i' && !gameOver) {
                            isMainPage = false;
                            useBuffer = false;
                            rectPos.width = 20;
                            speed = 8;
                            FPS = 60;
                            rectPos.x = w.width / 2 - rectPos.width / 2;
							ballPos.x = rectPos.x + rectPos.width / 2;
                            dList.pop_back();
                            dList.pop_back();
                            dList.push_back(new Text(20, 40, "FPS: " + to_string(FPS)));
                            dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));
                        }

						if ( i == 1 && text[0] == 't' && !gameOver) {
							isMainPage = false;
							useBuffer = false;
							isTestMode = true;
                            if (argc >= 2) {
							    speed = inputSpeed;
                                FPS = inputFPS;
                            } else {
                                speed = 12;
                                FPS = 60;
                            }
							rectPos.width = 1300;
							rectPos.x = w.width / 2 - rectPos.width / 2;
							ballPos.x = rectPos.x + rectPos.width / 2;
                            dList.pop_back();
                            dList.pop_back();
                            dList.push_back(new Text(20, 40, "FPS: " + to_string(FPS)));
                            dList.push_back(new Text(20, 60, "Speed: " + to_string(speed)));
						}
                        
                        if ( i == 1 && text[0] == 'r' ) {
                            mainPage = mainMenu;
							gameOver = false;
                        }
                    }
                    
                    // quit game
                    if ( i == 1 && text[0] == 'q' ) {
                        XCloseDisplay(display);
                        exit(0);
                    }
                
                    break;
				}
		}

		unsigned long end = now();	// get current time in microsecond

		if (end - lastRepaint > 1000000 / FPS) {
            
            Pixmap pixmap;
            
            if (useBuffer) {
                pixmap = buffer;
                // draw into the buffer
                // note that a window and a pixmap are “drawables”
                XSetForeground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
                XFillRectangle(display, pixmap, gc,
                               0, 0, w.width, w.height);
                
            } else {
                pixmap = window;
                // clear background
                XClearWindow(display, pixmap);

                repaint(dList, brickList, xinfo, rectPos, ballDir);
                XSetForeground(xinfo.display, gc, BlackPixel(xinfo.display, screen));

                // draw ball from centre
                XFillArc(display, window, gc,
                    ballPos.x - ballSize/2,
                    ballPos.y - ballSize/2,
                    ballSize, ballSize,
                    0, 360*64);

                // update ball position
                ballPos.x += ballDir.x;
                ballPos.y += ballDir.y;

                // bounce ball
                if (ballPos.x + ballSize/2 > w.width ||
                    ballPos.x - ballSize/2 < 0)
                    ballDir.x = -ballDir.x;
                if (ballPos.y - ballSize/2 < 0 ||
                    (godMode && ballPos.y + ballSize/2 > w.height))
                    ballDir.y = -ballDir.y;
                
                checkBoundary(ballPos, brickList, ballDir);
                if (ballPos.y > 540) {
                    checkRectBoundary(ballPos, rectPos, ballDir, rectPos.width, rectPos.height);
                }
                
                if (ballPos.y - ballSize/2 > w.height && !godMode && !isMainPage) {
                    cout << "Game ended" << endl;
					gameOver = true;
                    useBuffer = true;
					isTestMode = false;
                    currentScore = 0;
                    ballPos.x = 670;
                    ballPos.y = 660;
                    rectPos.x = 580;
                    rectPos.y = 680;
                    rectPos.width = 180;
                    ballDir.x = 0;
                    ballDir.y = 0;
                    brickList = generateBricks();
                    mainPage = endGameMenu;
                    godMode = false;
                    isMainPage = true;
                    isGameStarted = false;
                }
            }
            
            if (useBuffer) {
                XCopyArea(display, pixmap, window, gc,
                          0, 0, w.width, w.height,  // region of pixmap to copy
                          0, 0); // position to put top left corner of pixmap in window

                XSetBackground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
                XSetForeground(xinfo.display, gc, BlackPixel(xinfo.display, screen));
                
                list<Text*>::const_iterator begin = mainPage.begin();
                list<Text*>::const_iterator end = mainPage.end();
                
                while ( begin != end ) {
                    Text* d = *begin;
                    d->paint(xinfo);
                    begin++;
                }
            }

            XFlush( display );
            
			lastRepaint = now(); // remember when the paint happened
		}

		// IMPORTANT: sleep for a bit to let other processes work
		if (XPending(display) == 0) {
			usleep(1000000 / FPS - (now() - lastRepaint));
		}
	}
	XCloseDisplay(display);
}
