#include <SDL2/SDL.h>

#include <math.h>
#include <vector>
#include <stdlib.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

SDL_Renderer *renderer = nullptr;

struct vec3f {
    float x, y, z;
};

namespace Projection {
    float cameraX, cameraY, cameraZ = 0;
    void world_to_screen(int &outX, int &outY, float z)
    {
        // SCREEN_WIDTH/HEIGHT are the relative positions
        outX = (int)(outX / (z + 1) - cameraX / (z + 1) + SCREEN_WIDTH / 2);
        outY = (int)(outY / (z + 1) - cameraY / (z + 1) + SCREEN_HEIGHT / 2);
    }
    
    void adjust_camera(float relativeX, float relativeY) {
        cameraX = relativeX;
        cameraY = relativeY;
    }
};

struct mesh {
    vec3f *vertexBuffer = nullptr;
    int size = 0;
    void translate(vec3f translator) {
        vec3f *v = vertexBuffer;
        for (int i = 0; i < size; i++)
        {
            v[i].x += translator.x;
            v[i].y += translator.y;
            v[i].z += translator.z;
        }
    }
};

namespace Utils
{
       float clamp(float &value, float min, float max)
       {
           if (value < min) value = min;
           else if (value > max) value = max;
        
           return value;
       } 
       void draw_rect_fill(int x, int y, int w, int h)
       {
           SDL_Rect dest = { x - w / 2, y - h / 2, w, h };
           if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) 
               SDL_RenderFillRect(renderer, &dest);
       }
       void draw_color(float r, float g, float b)
       {
           float ar = r * 255;
           float ag = g * 255; 
           float ab = b * 255;
        
           clamp(ar, 0, 255);
           clamp(ag, 0, 255);
           clamp(ab, 0, 255); 
           SDL_SetRenderDrawColor(renderer, (int) ar, (int) ag, (int) ab, 255);
       }
       void draw_color(float r, float g, float b, float a, float r2, float g2, float b2, float a2, float progress)
       {
           // Linearly interpolates the color channels
           float rb = progress * (r2 - r);
           float gb = progress * (g2 - g);
           float bb = progress * (b2 - b);
           float ba = progress * (a2 - a);
           
           float ar = rb * 255;
           float ag = gb * 255; 
           float ab = bb * 255;
           float aa = aa * 255;
           
           clamp(ar, 0, 255);
           clamp(ag, 0, 255);
           clamp(ab, 0, 255); 
           clamp(aa, 0, 255);     
           SDL_SetRenderDrawColor(renderer, (int) ar, (int) ag, (int) ab, (int) aa);
       }
};

class Game
{
  public:
    const char *displayName = "";
    virtual ~Game(){};
    virtual void init(){};
    virtual void load(){};
    
    virtual void handle_event(SDL_Event ev){};

    virtual void update(float timeTook) = 0;
};
class Windmill : public Game {
    std::vector<mesh> meshes;
    
    int dx = 0, dy = 0;
    float cx = 0, cy = 0;
    float angle = 0;
    SDL_Color tintColor, shadeColor;
    public:
       void init() override {
           displayName = "Windmill";
           tintColor = { 255, 255, 255, 255 };
           shadeColor = { 25, 25, 25, 255 };
       } 
       void load() override {
           srand(98);
           for (int i = 0; i < 200; i++)
           {
               mesh modelCube;
               modelCube.size = 8;
               modelCube.vertexBuffer = new vec3f[8]{
                   { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 },
                   { 0.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 0.0 }, { 0.0, 1.0, 0.0 }
               };
               float x = rand() % 10;
               float y = rand() % 30;
               float z = rand() % 5;
               modelCube.translate({x, y, z});
               meshes.push_back(modelCube);
           }
       }
       void handle_event(SDL_Event ev) override {
           int cursorX = 0, cursorY = 0;
           SDL_GetMouseState(&cursorX, &cursorY);
           if (ev.type == SDL_MOUSEMOTION)
           {
               dx = cursorX > SCREEN_WIDTH / 2 ? 5 * 400 : -5 * 400;
               dy = cursorY > SCREEN_HEIGHT / 2 ? 5 * 400 : -5 * 400;   
           }
       }
       void update(float timeTook) override {
           cx += dx * timeTook;
           cy += dy * timeTook;
           angle += 5 * timeTook;
           if (angle >= 360) angle = 0;
           dx = 0;
           dy = 0;
           Projection::adjust_camera(cx, cy);
           
           Utils::draw_color(0.03, 0.03, 0.05);
           draw_background();
           
           float rad = angle / 180 * M_PI;
           for (auto m : meshes)
           {
               for (int i = 0; i < m.size; i++)
               {
                   vec3f v = m.vertexBuffer[i];
                   int x, y, z;
                   x = v.x * cos(rad) - v.y * sin(rad);
                   y = v.y * cos(rad) + v.x * sin(rad);
                   z = v.z;
                   scale_to_screen(x, y);
                   Projection::world_to_screen(x, y, z);
                   
                   float tint = 1 - v.z / 4;
                   Utils::clamp(tint, 0.1, 1); // Makes the dots clear enough
                   Utils::draw_color(
                      (float) shadeColor.r / 255, (float) shadeColor.g / 255, (float) shadeColor.b / 255, (float) shadeColor.a / 255,
                      (float) tintColor.r / 255, (float) tintColor.g / 255, (float) tintColor.b / 255, (float) tintColor.a / 255, 
                      tint
                   );
                   Utils::draw_rect_fill(x, y, 3, 3);
               }
           }
       }
       void scale_to_screen(int &outX, int &outY) {
           outX += 1.0; 
           outY += 1.0;
           outX *= 0.5 * float(SCREEN_WIDTH); 
           outY *= 0.5 * float(SCREEN_HEIGHT);
       }
       void draw_background()
       {
           SDL_Rect dest = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
           SDL_RenderFillRect(renderer, &dest);
       } 
};

int main()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    Windmill game;
    game.init();
    
    SDL_Window *window = SDL_CreateWindow(game.displayName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    game.load();
    
    float then = 0.0f, delta = 0.0f;
    bool disabled = false;
    SDL_Event e;
    while (!disabled)
    {
        // Code cited from lazyfoo.net
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    disabled = true;
                    break;
            }
            game.handle_event(e);
        }
        float now = SDL_GetTicks();
        delta = (now - then) * 1000 / SDL_GetPerformanceFrequency();
        then = now;
        
        Utils::draw_color(0, 0, 0);
        SDL_RenderClear(renderer);

        Utils::draw_color(1, 1, 1);
        game.update(delta);

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}