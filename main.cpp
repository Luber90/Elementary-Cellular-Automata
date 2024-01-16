#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>
#include <thread>

#define DELAY 25
#define WIDTH 200
#define HEIGHT 180
#define SCALE 5
#define START_RULE 129
#define MONO 0


typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    bool isOne() const {
        return (r < 128 || g < 128 || b < 128);
    }

    friend std::ostream& operator<<(std::ostream& os, const RGB& rgb){
        os << +rgb.r << " " << +rgb.g << " " << +rgb.b;
        return os;
    }

} RGB;


void drawScene(SDL_Renderer* renderer, const std::vector<RGB> image, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if(y*width+x < image.size()){
                auto current_pixel = image[y*width+x];
                SDL_SetRenderDrawColor(renderer, current_pixel.r, current_pixel.g, current_pixel.b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void getRuleSet(int rule, bool* ruleset){
    for(int i = 0; i < 8; i++) {
        ruleset[i] = (rule>>i)&0b1;
    }
}

RGB getNewState(RGB left, RGB mid, RGB right, bool *ruleset) {
    auto is_one = ruleset[(left.isOne()<<2)+(mid.isOne()<<1)+right.isOne()];
    if(is_one){
        #if MONO
        RGB result = {0, 0, 0};
        #else
        RGB result = {static_cast<uint8_t>(left.r/2), static_cast<uint8_t>(mid.g/2), static_cast<uint8_t>(right.b/2)};
        #endif
        return result;
    } else {
        #if MONO
        RGB result = {255, 255, 255};
        #else
        const uint8_t added = 128;
        RGB result;
        result.r = std::clamp(1*(left.r/2)+added, 0, 255);
        result.g = std::clamp(1*(mid.g/2)+added, 0, 255);
        result.b = std::clamp(1*(right.b/2)+added, 0, 255);
        #endif
        return result;
    }
}

std::vector<RGB> getNextCells(const std::vector<RGB> cells, bool *ruleset) {
    std::vector<RGB> next_cells(cells.size(), {0, 0, 0});
    for(int i = 0; i < cells.size(); i++) {
        int left_id = i == 0 ? cells.size()-1 : i-1;
        int right_id = (i+1) % cells.size();

        next_cells[i] = getNewState(cells[left_id], cells[i], cells[right_id], ruleset);
    }
    return next_cells;
}

void scroll(std::vector<RGB>& image, int width, int height, std::vector<RGB> next_cells) {
    if(image.size() == width*height) image.erase(image.begin(), image.begin()+next_cells.size());
    image.insert(image.end(), next_cells.begin(), next_cells.end());
}

void iterGeneration(SDL_Renderer* renderer, std::vector<RGB>& image, int width, int height, std::vector<RGB>& cells, bool *ruleset) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    auto next_cells = getNextCells(cells, ruleset);
    scroll(image, width, height, next_cells);

    cells = next_cells;
    drawScene(renderer, image, width, height);
    SDL_RenderPresent(renderer);
}

int main() {
    int rule = START_RULE;
    bool ruleset[8] = {0};
    getRuleSet(rule, ruleset);

    std::vector<RGB> image;
    image.reserve(WIDTH*HEIGHT);
    std::vector<RGB> cells(WIDTH, {255, 255, 255});
    cells[WIDTH/2] = {0, 0, 0};

    bool quit = false, first_frame = true;
    SDL_Event event;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH*SCALE, HEIGHT*SCALE, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, SCALE, SCALE);
    
    SDL_Delay(DELAY);


    while(!quit) {
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }

            if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        rule += 1;
                        if(rule > 255) rule = 255;
                        getRuleSet(rule, ruleset);
                        std::cout << rule << std::endl;
                        break;
                    case SDLK_DOWN:
                        rule -= 1;
                        if(rule < 0) rule = 0;
                        getRuleSet(rule, ruleset);
                        std::cout << rule << std::endl;
                        break;
                    case SDLK_r:
                        cells = std::vector<RGB>(WIDTH, {255, 255, 255});
                        cells[WIDTH/2] = {0, 0, 0};
                        first_frame = true;
                        std::cout << "reseted" << std::endl;
                        break;
                }
                
            }
        }
        if(first_frame){
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            scroll(image, WIDTH, HEIGHT, cells);
            drawScene(renderer, image, WIDTH, HEIGHT);
            SDL_RenderPresent(renderer);

            first_frame = false;
        } else {
            iterGeneration(renderer, image, WIDTH, HEIGHT, cells, ruleset);
        }
        
        SDL_Delay(DELAY);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}