#include "lib.h"

#include <iostream>

GBWindow gb;
coord dot;
GBColor pink;


int main () {

	dot.x = 100; dot.y=100;
    pink.set(255, 92, 220);     
    initGB(gb, "Howdy", 300, 300, RESIZABLE);
    //int f = RESIZABLE | FULLSCREEN;
    //std::cout << f << std::endl;
    setDrawColor(gb, pink);
    setBackground(gb, RGB(82, 192, 177));

    //XWindowAttributes wnd;
    

    /*GBShape block;
    block.x=block.y=0;
    block.width=block.height=50;*/

    int dx=0;

	while(1) {	
    redraw(gb);
    gb.update();
    //XGetWindowAttributes(gb.dis, gb.win, &wnd);
    //std::cout << wnd.width << std::endl;
    drawBackground(gb);
    //drawBackground(gb);
    setDrawColor(gb, pink);
    int mx, my;
    keys.getMouse(mx,my);
    dot.x=dot.y=150;
    drawRect(gb, dx,dx,50,50);
    dx+=20;
    if(dx>gb.Width) dx=0;
    drawOval(gb, 150, 150, 120, 50);
    setDrawColor(gb, GREEN);
    drawFilledRect(gb, 100, 200, 40, 80);
    drawFilledCircle(gb, 80, 40, 22);
    setDrawColor(gb, RED);
        drawLine(gb,mx,my,dot.x,dot.y);

        keys.update(gb);
		if (keys.pressed()) {
			if (keys.down()=='q') {
				close(gb);
			}
            if (keys.down('w')) {
				redraw(gb);
			}
			printf("You pressed the %c key!\n",keys.down());
		}
		if (keys.mouseDown()) {
		/* tell where the mouse Button was Pressed */
       // keys.update();
            int x, y;
            keys.getMouse(x, y);
            drawLine(gb,x,y,dot.x,dot.y);
            char* text;
			strcpy(text,"X is FUN!");
            keys.getMouse(dot.x, dot.y);
			setFontColor(gb,rand()%keys.mouseX%255);
			write(gb,x,y, text);
		}

	}


return 0;
}
