#include "raylib.h"
#include "raymath.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <array>
#include <iostream>
#include <string>
#include <memory>

Vector2 windowSize{800, 400};

constexpr struct {
  Color color1{255, 255, 255, 255};
  Color color2{239, 235, 234, 255};
  Color color3{228, 219, 214, 255};
  Color color4{244, 180, 134, 255};
  Color color5{212, 113, 93, 255};
  Color color6{77, 35, 74, 255};
} COLOR_PALLETE;

enum CatState {
  IDLE,
  WALKING_UP,
  WALKING_DOWN,
  ATTACKING,
};

struct Animation {
  Animation(Texture2D texture, int frames, int speed): texture(texture), frames(frames), speed(speed) {}
  Texture2D texture;
  int frames;
  int speed = 60;
  ~Animation() {
    UnloadTexture(texture);
  }
};


struct Cat {
  Vector2 position;
  float scale = 5;
  float flip = 1;
  float speed = 5;
  CatState state = IDLE;
  int direction = 0;
  bool attacking = false;
  float attackingTime = 0;
  float attackCoolDown = 30;
  Animation* currentAnimation;
  int currentFrame = 0;
  int animationStep = 0;
};

struct Ball {
  Vector2 position;
  float scale;
  Vector2 speed;
  Texture2D texture;
  int waitingTime = 0;
  Cat* catColliding = nullptr;
  float rotation = 0;
};

std::array<int, 2> points {0,0};
int markedPoints = 0;

Ball ball{{0,0}, 2};

int ballReactionTime = 60; //1 second

std::array<Cat, 2> cats;

std::unique_ptr<Animation> runAnimation;
std::unique_ptr<Animation> idleAnimation;
std::unique_ptr<Animation> attackAnimation;

bool gameOver = true;

void initAssets() {
  idleAnimation = std::make_unique<Animation>(LoadTexture("assets/cat_0_idle_1.png"), 2, 60);
  runAnimation = std::make_unique<Animation>(LoadTexture("assets/cat_0_run.png"), 8, 7);
  attackAnimation = std::make_unique<Animation>(LoadTexture("assets/cat_0_attack.png"), 6, 5);

  ball.texture = LoadTexture("assets/yarn.png");
}

void placeBall() {
  ball.position = {windowSize.x / 2.0f, windowSize.y / 2.0f};
  ball.waitingTime = 0;
}

void startGame() {
  cats[1].currentAnimation = idleAnimation.get();
  cats[0].currentAnimation = idleAnimation.get();

  cats[0].position = {(cats[0].currentAnimation->texture.width * cats[0].scale) / 2.0f +
                          10.0f,
                      windowSize.y / 2};
  cats[1].position = {windowSize.x -
                          (cats[0].currentAnimation->texture.width * cats[0].scale) / 2.0f -
                          10.0f,
                      windowSize.y / 2};
  cats[1].flip = -1;


  points.fill(0);

  placeBall();
}

Rectangle getCatRec(const Cat &cat) {
  const auto frameWidth =
      static_cast<float>(cat.currentAnimation->texture.width) * cat.scale /
      cat.currentAnimation->frames;
  const auto frameHeight =
      static_cast<float>(cat.currentAnimation->texture.height) * cat.scale;
  return {cat.position.x - (frameWidth / 2.0f),
          cat.position.y - (frameHeight / 2.0f), frameWidth, frameHeight};
}

void drawCatPosition(const Cat &cat) {
  const auto catRec = getCatRec(cat);
  DrawRectangleLines(catRec.x, catRec.y, catRec.width, catRec.height, RED);
}

void drawCats() {
  for (const auto &cat : cats) {
    auto frameWidth = static_cast<float>(cat.currentAnimation->texture.width) /
                      cat.currentAnimation->frames;
    auto frameHeight = static_cast<float>(cat.currentAnimation->texture.height);
    DrawTexturePro(cat.currentAnimation->texture,
                   {cat.currentFrame * frameWidth, 0.0f,
                    frameWidth * (cat.flip), frameHeight},
                   {cat.position.x - (frameWidth / 2.0f * cat.scale),
                    cat.position.y - (frameHeight / 2.0f * cat.scale),
                    frameWidth * cat.scale, frameHeight * cat.scale},
                   {frameWidth, frameHeight}, 0.0f, WHITE);


    // drawCatPosition(cat);
  }
}

void processAnimation() {
  for (auto &cat : cats) {

    const auto animationFrames = cat.currentAnimation->frames;

    if(cat.currentFrame > animationFrames) {
      cat.currentFrame = 0;
    } 

    cat.animationStep = (cat.animationStep + 1) % cat.currentAnimation->speed;

    if (cat.animationStep == 0) {
      cat.currentFrame = (cat.currentFrame + 1) % animationFrames;
    }

  }
}

void processPlayerControl() {
  auto &playerCat = cats[0];

  if (IsKeyDown(KEY_UP)) {
    playerCat.direction = -1;
  } else if (IsKeyDown(KEY_DOWN)) {
    playerCat.direction = 1;
  } else {
    playerCat.direction = 0;
  }

  playerCat.attacking = IsKeyDown(KEY_SPACE);
}


const Rectangle getBallRec() {
  return {ball.position.x - (ball.texture.width * ball.scale) / 2.0f, ball.position.y - (ball.texture.height * ball.scale) / 2.0f, ball.texture.width * ball.scale,
                            ball.texture.height * ball.scale};
}

void processAIControl() {
  static auto AIEnergy = 100;
  constexpr auto AICoolDown = 10;
  static auto AIRefreshTime = 0;

  AIEnergy--;

  if(AIEnergy < 0) {
    AIRefreshTime++;
    if(AIRefreshTime == AICoolDown) {
      AIRefreshTime = 0;
      AIEnergy = GetRandomValue(90, 100);
    }
    return;
  }


  auto &aiCat = cats[1];
  const auto ballRec = getBallRec();
  const auto catRec = getCatRec(aiCat);

  if(ballRec.y + ballRec.height > catRec.y + catRec.height) {
    aiCat.direction = 1;
  } else if(ballRec.y < catRec.y) {
    aiCat.direction = -1;
  }

  aiCat.attacking = CheckCollisionRecs(ballRec, catRec);
  

}

void changeCatsState() {
  for (auto &cat : cats) {
    const auto catRec = getCatRec(cat);

    switch (cat.state) {
    case WALKING_UP:
    case WALKING_DOWN:
    case IDLE:
      if (cat.direction < 0 && catRec.y + catRec.height > catRec.height) {
        cat.state = WALKING_UP;
      } else if (cat.direction > 0 && catRec.y + catRec.height < GetScreenHeight()) {
        cat.state = WALKING_DOWN;
      } else {
        cat.state = IDLE;
      }

      if (cat.attacking) {
        cat.state = ATTACKING;
      }
      break;
    case ATTACKING:

      if (cat.attackingTime >= cat.attackCoolDown) {
        cat.state = IDLE;
      }

      break;
    }
  }
}

void processCatsState() {
  for (auto &cat : cats) {
    switch (cat.state) {
    case IDLE:
      cat.direction = 0;
      cat.attackingTime = 0;
      cat.currentAnimation = idleAnimation.get();
      break;
    case WALKING_DOWN:
    case WALKING_UP:
      cat.currentAnimation = runAnimation.get();
      cat.position.y += cat.direction * cat.speed;
      break;
    case ATTACKING:
      cat.currentAnimation = attackAnimation.get();
      cat.attackingTime++;
      break;
    }
  }
}

void shootBallRandomDirection() {
  auto negativeX = GetRandomValue(0, 1);
  auto negativeY = GetRandomValue(0, 1);
  ball.speed = {static_cast<float>(GetRandomValue(3, 5)) * (negativeX ? -1 : 1),
                static_cast<float>(GetRandomValue(3, 5)) * (negativeY ? -1 : 1)};
}

void ballMovement() {
  if(ball.waitingTime < ballReactionTime) {
    ball.waitingTime++;
    if(ball.waitingTime >= ballReactionTime) {
      shootBallRandomDirection();
    }
  } else {
    ball.position = Vector2Add(ball.position, ball.speed);
  }
}

void drawBall() {
  DrawTexturePro(ball.texture, 
      {0.0f, 0.0f, static_cast<float>(ball.texture.width), static_cast<float>(ball.texture.height)},
      {ball.position.x, ball.position.y, ball.texture.width * ball.scale, ball.texture.height * ball.scale},
      {static_cast<float>(ball.texture.width), static_cast<float>(ball.texture.height)},
      ball.rotation,
      WHITE
      );

  //DrawRectangleLines(ballRec.x, ballRec.y, ballRec.width, ballRec.height, RED);
}

void ballCollision() {
  const auto ballRec = getBallRec();

  if (ballRec.x + ballRec.width >= GetScreenWidth()) {
    markedPoints++;
  }
  if (ballRec.x + ballRec.width <= ballRec.width) {
    markedPoints--;
  }

  if ((ballRec.y + ballRec.height >= GetScreenHeight()) ||
      (ballRec.y + ballRec.height <= ballRec.height))
    ball.speed.y *= -1.0f;

  for (auto &cat : cats) {
    if (ball.catColliding != &cat && cat.state == ATTACKING &&
        CheckCollisionRecs(getCatRec(cat), getBallRec())) {
      ball.catColliding = &cat;
      ball.speed.x *= -1 * 1.05;
    }
  }

  if (ball.catColliding && !CheckCollisionRecs(getCatRec(*ball.catColliding), getBallRec())) {
    ball.catColliding = nullptr;
  }
}

void drawPoints() {
  const std::string text { std::to_string(points[0]) + " | " + std::to_string(points[1])};
  constexpr auto fontSize = 24;
  const auto measuredText = MeasureText(text.c_str(), fontSize);
  constexpr auto offsetY = 10;

  DrawText(text.c_str(), GetScreenWidth() / 2.0f - (measuredText / 2.0f), fontSize + offsetY, fontSize, COLOR_PALLETE.color6);
}

void drawGameOver() {
  const std::string text {"Press Space to start!"};
  constexpr auto fontSize = 24;
  const auto measuredText = MeasureText(text.c_str(), fontSize);
  constexpr auto offsetY = 32;

  DrawText(text.c_str(), GetScreenWidth() / 2.0f - (measuredText / 2.0f), GetScreenHeight() - offsetY, fontSize, COLOR_PALLETE.color6);

}

constexpr auto pointsToWin = 5;

void checkGameOver() {
  if(points[0] == pointsToWin || points[1] == pointsToWin) {
    gameOver = true;
  }
}

void drawWinQuote() {
  std::string text;
  constexpr auto fontSize = 24;
  constexpr auto offsetY = 36;

  if(points[0] == pointsToWin) {
    text = "You Win";
  } else if(points[1] == pointsToWin) {
    text = "You Lose";
  }


  if(text.size()) {
    const auto measuredText = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), GetScreenWidth() / 2.0f - (measuredText / 2.0f), fontSize + offsetY, fontSize, COLOR_PALLETE.color6);
  }
}



void frame() {

  if (!gameOver) {
    processPlayerControl();
    processAIControl();
    changeCatsState();
    processCatsState();
    processAnimation();
    ballMovement();
    ball.rotation += ball.speed.x;
    ballCollision();

    if (markedPoints) {
      if (markedPoints > 0) {
        points[0]++;
      } else {
        points[1]++;
      }

      placeBall();

      markedPoints = 0;
    }

    checkGameOver();
  } else {
    if (IsKeyDown(KEY_SPACE)) {
      gameOver = false;
      startGame();
    }
  }

  // DRAW
  BeginDrawing();
  ClearBackground(COLOR_PALLETE.color3);

  if(gameOver) {
    drawGameOver();
    drawWinQuote();
  }
  drawPoints();
  drawBall();
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

  idleAnimation.reset();
  runAnimation.reset();
  attackAnimation.reset();

  CloseWindow();

  return 0;
}
