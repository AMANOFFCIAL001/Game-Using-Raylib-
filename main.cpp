#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include "score.h"

#define PI 3.14159265358979323846f
#define BACKGROUND_COLOR (Color){186, 149, 127}
#define CAR_COLOR BLACK
#define CAMERA_FOLLOW_THRESH 400
#define MAX_SKIDMARKS 500
#define SKIDMARK_TIME 3

typedef struct {
    float left_tire_x;
    float left_tire_y;
    float right_tire_x;
    float right_tire_y;
    double time;
} Skidmark;

int main() {
    int width = 1300;
    int height = 700;
    InitWindow(width, height, "Racer");
    SetTargetFPS(60);

    int world_width = 5000;
    int world_height = 5000;

    Image soil_image = LoadImage("craftpix-889156-free-racing-game-kit/PNG/Background_Tiles/Soil_Tile.png");
    ImageRotateCW(&soil_image);
    Texture2D soil_texture = LoadTextureFromImage(soil_image);

    Image car_image = LoadImage("craftpix-889156-free-racing-game-kit/PNG/Car_1_Main_Positions/Car_1_01.png");
    Texture2D car_texture = LoadTextureFromImage(car_image);
    Rectangle car_texture_rec = {
        .x = 0,
        .y = 0,
        .width = (float)car_texture.width,
        .height = (float)car_texture.height,
    };

    // coin + score system
    Coin coin;
    InitCoin(&coin, world_width, world_height);

    int score = 0;
    float carRadius = 40;   // collision radius of car

    int car_width = 80;
    int car_length = 150;
    float car_x = width/2 - car_width/2;
    float car_y = height/2 - car_length/2;
    float car_speed = 0;
    float car_max_speed = 7;
    int car_direction = -1;
    float car_angle = 0;
    float car_speedup = 10;
    float car_slowdown = 0.97f;

    float drift_angle = car_angle;
    float drift_bias = 15;

    float steering = 0;
    float steering_speed = 2;
    float max_steering = 4;
    float steer_back_speed = 0.04f;

    Skidmark skidmarks[MAX_SKIDMARKS];
    int skidmark_count = 0;

    // Use a standard camera center offset so we can follow the car via camera.target
    Camera2D camera = {
        .offset = (Vector2){ width/2.0f, height/2.0f }, // screen center
        .target = (Vector2){ car_x, car_y },            // will update each frame
        .rotation = 0,
        .zoom = 1.0f,
    };

    while( !WindowShouldClose() ) {
        float dt = GetFrameTime();

        // input / physics
        if( IsKeyDown(KEY_UP) ) {
            car_direction = -1;
            car_speed += car_speedup * dt;
            if( car_speed > car_max_speed ) {car_speed = car_max_speed;}
        } else if( IsKeyDown(KEY_DOWN) ) {
            car_direction = 1;
            car_speed -= car_speedup * dt;
            if( car_speed < -car_max_speed ) car_speed = -car_max_speed;
        } else {
            car_speed = car_slowdown * car_speed;
        }

        if( IsKeyDown(KEY_LEFT) ) {
            steering -= steering_speed * dt * fabsf(car_speed);
            if( steering < -max_steering ) steering = -max_steering;
        } else if( IsKeyDown(KEY_RIGHT) ) {
            steering += steering_speed * dt * fabsf(car_speed);
            if( steering > max_steering ) steering = max_steering;
        }

        steering = steering * (1 - steer_back_speed);

        car_angle += steering;

        drift_angle = (car_angle + drift_angle * drift_bias) / (1 + drift_bias);

        float drift_diff = drift_angle - car_angle;
        bool drifting = fabsf(drift_diff) > 30.0f;

        // Move car forward (along car_angle)
        float radians = PI * (car_angle - 90.0f) / 180.0f;
        car_x += car_speed * cosf(radians);
        car_y += car_speed * sinf(radians);

        // Move car slightly toward drift angle (gives drifting effect)
        radians = PI * (drift_angle - 90.0f) / 180.0f;
        car_x += car_speed * cosf(radians);
        car_y += car_speed * sinf(radians);

        Vector2 carPos = { car_x, car_y };

        // Update coin and score (assuming UpdateCoin matches this signature)
        UpdateCoin(&coin, carPos, carRadius, &score);

        // Update camera target to follow the car
        camera.target = carPos;

        // Optional: clamp camera.target to world bounds so camera doesn't show outside world
        // Compute min/max target allowed so camera offset keeps view inside world
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;
        if (camera.target.x < halfW) camera.target.x = halfW;
        if (camera.target.x > (world_width - halfW)) camera.target.x = world_width - halfW;
        if (camera.target.y < halfH) camera.target.y = halfH;
        if (camera.target.y > (world_height - halfH)) camera.target.y = world_height - halfH;

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        BeginMode2D(camera);

        // Draw coin (only once; keep it inside camera/world space)
        DrawCoin(coin);     // MUST BE HERE (draw before car)

        // draw tiled soil background (use float division before ceil)
        int tile_count_col = (int)ceilf((float)world_width / (float)soil_texture.width);
        int tile_count_row = (int)ceilf((float)world_height / (float)soil_texture.height);

        for( int x = 0; x < tile_count_col; x++ ) {
            for( int y = 0; y < tile_count_row; y++ ) {
                DrawTexture(soil_texture, x * soil_texture.width, y * soil_texture.height, WHITE);
            }
        }

        // drifting skidmarks
        if( drifting ) {
            radians = PI * (car_angle - 240.0f) / 180.0f;
            float left_tire_x = car_x + (car_length / 2.6f) * cosf(radians);
            float left_tire_y = car_y + (car_length / 2.6f) * sinf(radians);

            radians = PI * (car_angle - 300.0f) / 180.0f;
            float right_tire_x = car_x + (car_length / 2.6f) * cosf(radians);
            float right_tire_y = car_y + (car_length / 2.6f) * sinf(radians);

            skidmarks[skidmark_count % MAX_SKIDMARKS] = (Skidmark){
                left_tire_x,
                left_tire_y,
                right_tire_x,
                right_tire_y,
                GetTime(),
            };
            skidmark_count++;
        }

        for( int i = 0; i < skidmark_count && i < MAX_SKIDMARKS; i++ ) {
            Skidmark skidmark = skidmarks[i];

            double current_time = GetTime();
            if( current_time - skidmark.time > SKIDMARK_TIME ) {
                continue;
            }

            DrawCircle((int)skidmark.left_tire_x, (int)skidmark.left_tire_y, 6, BLACK);
            DrawCircle((int)skidmark.right_tire_x, (int)skidmark.right_tire_y, 6, BLACK);
        }

        // draw car
        Rectangle car_rec = {
            .x = car_x,
            .y = car_y,
            .width = (float)car_width,
            .height = (float)car_length,
        };
        Vector2 car_origin = {
            .x = car_width/2.0f,
            .y = car_length/2.0f,
        };

        DrawTexturePro(car_texture, car_texture_rec, car_rec, car_origin, car_angle, WHITE);

        EndMode2D();

        DrawText(TextFormat("Score: %d", score), 20, 20, 40, BLACK);

        EndDrawing();
    }

    // unload resources
    UnloadTexture(car_texture);
    UnloadImage(car_image);

    UnloadTexture(soil_texture);
    UnloadImage(soil_image);

    CloseWindow();

    return 0;
}