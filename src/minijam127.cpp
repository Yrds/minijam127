#include "raylib.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <array>
#include <iostream>
#include <string>
#include <vector>

Vector2 windowSize{800, 400};

enum AnimationState {
  IDLE,
  WALKING,
  RUNNING,
  CHEERING,
};

struct Cat {
  Vector2 position {};
};

std::array<Cat, 2> cats;

std::vector<Texture2D> catImages;

void initAssets() {
  catImages.push_back(LoadTexture("assets/cat_0_idle_0.png"));
  catImages.push_back(LoadTexture("assets/cat_0_idle_1.png"));
  catImages.push_back(LoadTexture("assets/cat_0_idle_2.png"));
}

void startGame() {
  cats[0].position = {catImages[0].width - 10.0f, windowSize.y/2};
  cats[1].position = {windowSize.x - catImages[0].width - 10.0f, windowSize.y/2};
}

void drawCats() {
  for(const auto& cat: cats) {
    DrawTextureEx(
        catImages[0],
        { cat.position.x - (catImages[0].width / 2.0f ),
           cat.position.y - (catImages[0].height / 2.0f) },
        0,
        5.0,
        WHITE);

    DrawRectangleLines(cat.position.x, cat.position.y, catImages[0].width, catImages[0].height, RED);
  }

}

void frame() {
  // DRAW
  BeginDrawing();
  ClearBackground(RAYWHITE);

  drawCats();

  EndDrawing();
}

int main(int argc, const char **argv) {
  InitWindow(windowSize.x, windowSize.y, "cat pong");

  initAssets();

  startGame();

  SetTargetFPS(60);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(&frame, 0, 1);
#else
  while (!WindowShouldClose()) {
    frame();
  }
#endif
  CloseWindow();

  return 0;
}
