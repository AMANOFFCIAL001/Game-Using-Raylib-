#ifndef SCORE_H
#define SCORE_H

#include <raylib.h>

typedef struct {
    Vector2 position;
    float radius;
    int value;
    bool active;
} Coin;

void InitCoin(Coin *coin, int worldWidth, int worldHeight);
void UpdateCoin(Coin *coin, Vector2 carPos, float carRadius, int *score);
void DrawCoin(Coin coin);

#endif
