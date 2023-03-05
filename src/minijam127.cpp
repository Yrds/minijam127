#include "raylib.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <array>
#include <iostream>
#include <string>
#include <vector>

Vector2 windowSize{800, 400};

enum CatState {
  IDLE,
  WALKING_UP,
  WALKING_DOWN,
  ATTACKING,
};

struct Animation {
  Texture2D texture;
  int frames;
};


struct Cat {
  Vector2 position {};
  float scale = 5;
  float flip = 1;
  float speed = 5;
  CatState state = IDLE;
  int direction = 0;
  bool attacking = false;
  float attackingTime = 0;
  float attackCoolDown = 30;
  Animation currentAnimation;
  float currentFrame = 0;
};

std::array<Cat, 2> cats;

std::vector<Animation> catImages;

void initAssets() {
  catImages.push_back({LoadTexture("assets/cat_0_idle_1.png"), 6});
}

void startGame() {
  cats[0].position = {(catImages[0].texture.width * cats[0].scale) / 2.0f + 10.0f, windowSize.y/2};
  cats[1].position = {windowSize.x - (catImages[0].texture.width * cats[0].scale) /2.0f - 10.0f, windowSize.y/2};
  cats[1].flip = -1;
}

Rectangle getCatRec(const Cat &cat) {
  return {
    cat.position.x,
    cat.position.y,
    cat.position.x + (catImages[0].texture.width * cat.scale / 2.0f),
    cat.position.y + (catImages[0].texture.height * cat.scale / 2.0f)
  };
}

void drawCatPosition(const Cat &cat) {
  const auto frameWidth = static_cast<float>(cat.currentAnimation.texture.width * cat.scale / cat.currentAnimation.frames);
  const auto frameHeight = static_cast<float>(catImages[0].texture.height) * cat.scale;
  DrawRectangleLines(cat.position.x - (frameWidth / 2.0f), cat.position.y - (frameHeight / 2.0f), catImages[0].texture.width * cat.scale, catImages[0].texture.height * cat.scale, RED);
}

void drawCats() {
  for(const auto& cat: cats) {
    auto frameWidth = static_cast<float>(cat.currentAnimation.texture.width) / cat.currentAnimation.frames;
    auto frameHeight = static_cast<float>(catImages[0].texture.height);
    DrawTexturePro(
        catImages[0].texture,
        {0.0f, 0.0f, frameWidth * (cat.flip), frameHeight},
        {cat.position.x - (frameWidth / 2.0f * cat.scale), cat.position.y - (frameHeight / 2.0f * cat.scale), frameWidth * cat.scale, frameHeight * cat.scale},
        {0.0f, 0.0f},
        0.0f,
        WHITE
        );
    drawCatPosition(cat);
  }
}

void processAnimation() {
  for(auto& cat: cats) {
    cat.currentFrame++;
    if(cat.currentFrame > cat.currentAnimation.frames) {
      cat.currentFrame = 0;
    }
  }
}

void processPlayerControl() {
  auto &playerCat = cats[0];

  if(IsKeyDown(KEY_UP)) {
    playerCat.direction = -1 * playerCat.speed;
  } else if(IsKeyDown(KEY_DOWN)) {
    playerCat.direction = 1 * playerCat.speed;
  } else {
    playerCat.direction = 0;
  }

  playerCat.attacking = IsKeyDown(KEY_SPACE);
}

void changeCatsState() {
  for(auto &cat: cats) {
    switch(cat.state) {
      case WALKING_UP:
      case WALKING_DOWN:
      case IDLE:
        if(cat.direction < -1) {
          cat.state = WALKING_UP;
        } else if(cat.direction > 1) {
          cat.state = WALKING_DOWN;
        } else if(cat.direction == 0) {
          cat.state = IDLE;
        } 

        if(cat.attacking) {
          cat.state = ATTACKING;
        }
        break;
      case ATTACKING:

        if(cat.attackingTime >= cat.attackCoolDown) {
          cat.state = IDLE;
        }

        break;
    }
  }
}

void processCatsState() {
  for(auto &cat: cats) {
    switch(cat.state) {
      case IDLE:
        cat.direction = 0;
        cat.attackingTime = 0;
      case WALKING_DOWN:
      case WALKING_UP:
        cat.position.y += cat.direction;
      case ATTACKING:
        cat.attackingTime++;
      break;
    }
  }
}

void frame() {

  processPlayerControl();
  changeCatsState();
  processCatsState();
  
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
