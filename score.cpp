#include "score.h"
#include <math.h>

extern float car_x;
extern float car_y;

void InitCoin(Coin *coin, int worldWidth, int worldHeight) {
    coin->position = (Vector2){
        car_x + 200,
        car_y + 200
    };
    coin->radius = 20;
    coin->value = 10;
    coin->active = true;
}
void UpdateCoin(Coin *coin, Vector2 carPos, float carRadius, int *score) {
    if (!coin->active) return;

    float dx = coin->position.x - carPos.x;
    float dy = coin->position.y - carPos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < carRadius + coin->radius) {
        *score += coin->value;
        coin->active = false;

        // Respawn next coin
        InitCoin(coin, 5000, 5000);
    }
}

void DrawCoin(Coin coin) {
    if (coin.active)
        DrawCircleV(coin.position, coin.radius, GOLD);
}
