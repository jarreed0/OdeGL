#include "lib.h"

#include <iostream>

GBWindow gb;
int dx;
GBShape rect1;
GBShape rect2;
GBShape circ1;
GBShape circ2;

void render() {
 redraw(gb);
 drawBackground(gb);
 setDrawColor(gb, RED);
 if(touching(rect1,rect2) || touching(rect1,circ1) || touching(rect1,circ2)) setDrawColor(gb, GREEN);
 drawFilledRect(gb, rect1);
 setDrawColor(gb, RED);
 drawOval(gb, circ1);
 setDrawColor(gb, GREEN);
 drawFilledRect(gb, rect2);
 drawFilledCircle(gb, circ2);
}

void input() {
 //keys.update(gb);
 //int k = keys.update2();
 if (keys.pressed()) {
  if (keys.down()=='q') {
   close(gb);
  }
  if (keys.down('w')) {
   redraw(gb);
  }
  printf("You pressed the %c key!\n",keys.down());
 }
}

void update() {
 rect1.x=rect1.y=dx;
 dx++;
 if(dx>gb.Width) dx=0;
 gb.update();
}

void init() {
 initGB(gb, "Howdy", 300, 300, RESIZABLE);
 setBackground(gb, RGB(82, 192, 177));
 dx=0;
 rect1.width=rect1.height=50;
 circ1.x=circ1.y=150;
 circ1.width=120; circ1.height=50;
 rect2.x=100;rect2.y=200;
 rect2.width=40;rect2.height=80;
 circ2.x=80;circ2.y=40;
 circ2.width=22;
}

void loop() {

 while(1) {

  render();
  input();
  update();

 }
}

int main () {

 init();

 loop();

 return 0;
}
