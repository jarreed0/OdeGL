#include "ode.h"

ODEWindow ode;
ODEScene scene1;
mesh plane, cube, monkey;
std::vector<mesh> clouds;
int tick;

void createCloud() {
  int count = rand() & 3 + 1;
  for (int i = 0; i < count; i++) {
    mesh cloud;
    double size = rand() % 2 + 0.3;
    double x = rand() % 2 - 1;
    double y = rand() % 2 - 1;
    cloud = createTriangle(size);
    shiftMeshOnX(cloud, x);
    shiftMeshOnX(cloud, y);
    cloud.setColors(WHITE, RGB(212, 212, 212));
    clouds.push_back(cloud);
  }
}

void updateClouds() {
  for (int i = 0; i < clouds.size(); i++) {
    shiftMeshOnX(clouds[i], 0.0003f);
    if (tick > 56130) {
      shiftMeshOnX(clouds[i], -20.0f);
    }
  }
}
void drawClouds() {
  for (int i = 0; i < clouds.size(); i++) {
    drawMesh(ode, clouds[i]);
  }
}

void render() {
  redraw(ode);
  drawBackground(ode);
  setWireColor(ode, WHITE);
  setFaceColor(ode, 13172580);
  //drawMesh(ode, monkey);
  renderScene(ode, scene1);
  drawClouds();
}

void input() {}

void update() {
  if (tick > 56130) {
    shiftMeshOnX(scene1.plane, -12.0f);
    for (int i = 0; i < scene1.objects.size(); i++) {
      shiftMeshOnX(scene1.objects[i], -12.0f);
    }
    tick = 0;
    createCloud();
  }
  for (int i = 0; i < scene1.objects.size(); i++) {
    if (scene1.objects[i].plane) {
      shiftMeshOnX(scene1.objects[i], 0.0002f);
    } else {
      shiftMeshOnX(scene1.objects[i], 0.0002f);
    }
  }
  shiftMeshOnX(scene1.plane, 0.0002f);
  updateClouds();
  ode.update();
  tick++;
}

void init() {
  initODE(ode, "Howdy", 400, 320, RESIZABLE);
  setBackground(ode, BLACK);
  createCloud();
  plane = createPlane(5.0f, 5.0f, -4.0f, -2.0f);
  plane.setColors(BLACK, GREEN);
  mesh road = createPlane(1.0f, 5.0f, -5.0f, -2.0f);
  shiftMeshOnZ(road, 2);
  shiftMeshOnZ(plane, 2);
  road.setColors(WHITE, RGB(63, 60, 60));
  addToScene(scene1, road);
  shiftMeshOnX(road, 3.5f);
  addToScene(scene1, road);
  road = createPlane(5, 0.5f, -4, -0.7f);
  shiftMeshOnZ(road, 2);
  road.setColors(WHITE, RGB(63, 60, 60));
  addToScene(scene1, road);
  cube = createRect(1, 2, 3);
  cube.setColors(BLUE, WHITE);
  addToScene(scene1, cube);
  shiftMeshOnX(cube, -3.4f);
  addToScene(scene1, cube);
  shiftMeshOnY(cube, -3);
  scene1.plane = plane;
  cube.setColors(BLUE, RED);
  addToScene(scene1, cube);
  monkey.loadObj("monkey.obj");
  //shiftMeshOnZ(monkey,-1.0f);
  //shiftMeshOnX(monkey,-1.0f);
  //shiftMeshOnY(monkey,-1.0f);
}

void loop() {
  while (1) {
    render();
    input();
    update();
    //std::cout << monkey.tris[0].c[0].x << ", " << monkey.tris[0].c[0].y << ", " <<  monkey.tris[0].c[0].z << std::endl;
  }
}

int main() {
  init();
  loop();
  return 0;
}
