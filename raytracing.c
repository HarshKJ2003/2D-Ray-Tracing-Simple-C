#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 1200
#define HEIGHT 600
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define COLOR_RAY 0xFFD43BFF
#define RAYS_NUMBER 500
#define M_PI 3.14159265358979323846

struct Circle {
    double x;
    double y;
    double r;
};

struct Ray {
    double x_start, y_start;
    double angle;
};

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
    double radius_squared = pow(circle.r, 2);
    for (int x = (int)(circle.x - circle.r); x <= circle.x + circle.r; x++) {
        for (int y = (int)(circle.y - circle.r); y <= circle.y + circle.r; y++) {
            double dx = x - circle.x;
            double dy = y - circle.y;
            if (dx*dx + dy*dy <= radius_squared) {
                SDL_Rect pixel = {x, y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        double angle = ((double)i / RAYS_NUMBER) * 2 * M_PI;
        rays[i] = (struct Ray){circle.x, circle.y, angle};
    }
}

void FillRays(SDL_Surface* surface, struct Ray rays[RAYS_NUMBER], Uint32 color, struct Circle obstacle) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        struct Ray ray = rays[i];
        double x_draw = ray.x_start;
        double y_draw = ray.y_start;
        int end_of_screen = 0;
        int object_hit = 0;
        
        while (!end_of_screen && !object_hit) {
            x_draw += cos(ray.angle);
            y_draw += sin(ray.angle);
            
            double dx = x_draw - obstacle.x;
            double dy = y_draw - obstacle.y;
            if (dx*dx + dy*dy <= obstacle.r*obstacle.r) {
                object_hit = 1;
            }
            
            if (x_draw < 0 || x_draw >= WIDTH || y_draw < 0 || y_draw >= HEIGHT) {
                end_of_screen = 1;
            }
            else {
                SDL_Rect pixel = {(int)x_draw, (int)y_draw, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Raytracing",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );
    
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        fprintf(stderr, "Surface retrieval failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    struct Circle circle = {200, 200, 20};
    struct Circle shadow_circle = {550, 300, 40};
    SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
    
    struct Ray rays[RAYS_NUMBER];
    generate_rays(circle, rays);
    
    double obstacle_speed_y = 1;
    int simulation_running = 1;
    SDL_Event event;
    
    while (simulation_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) simulation_running = 0;
            
            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0) {
                circle.x = event.motion.x;
                circle.y = event.motion.y;
                generate_rays(circle, rays);
            }
        }

        SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
        
        shadow_circle.y += obstacle_speed_y;
        if (shadow_circle.y - shadow_circle.r < 0 || 
            shadow_circle.y + shadow_circle.r > HEIGHT) {
            obstacle_speed_y *= -1;
        }
        
        FillCircle(surface, circle, COLOR_WHITE);
        FillCircle(surface, shadow_circle, COLOR_WHITE);
        FillRays(surface, rays, COLOR_RAY, shadow_circle);
        
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
