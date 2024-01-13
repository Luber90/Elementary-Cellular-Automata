#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#define DELAY 50
#define WIDTH 200
#define HEIGHT 180



void drawScene(SDL_Renderer* renderer, const std::vector<int> image, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if(y*width+x < image.size() && image[y*width+x] != 0){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void getRuleSet(int rule, int* ruleset){
    for(int i = 0; i < 8; i++) {
        ruleset[7-i] = (rule>>i)&0b1;
    }
}

int getNewState(int left, int mid, int right, int *ruleset) {
    return ruleset[7-((left<<2)+(mid<<1)+right)];
}

std::vector<int> getNextCells(const std::vector<int> cells, int *ruleset) {
    std::vector<int> next_cells(cells.size(), 0);
    for(int i = 0; i < cells.size(); i++) {
        int left_id = i == 0 ? cells.size()-1 : i-1;
        int right_id = (i+1) % cells.size();

        next_cells[i] = getNewState(cells[left_id], cells[i], cells[right_id], ruleset);
    }
    return next_cells;
} 

void iterGeneration(SDL_Renderer* renderer, std::vector<int>& image, int width, int height, std::vector<int>& cells, int *ruleset) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if(image.size() == width*height) image.erase(image.begin(), image.begin()+cells.size());
    auto next_cells = getNextCells(cells, ruleset);
    image.insert(image.end(), next_cells.begin(), next_cells.end());

    cells = next_cells;
    drawScene(renderer, image, width, height);
    SDL_RenderPresent(renderer);
}

int main() {
    int rule = 129;
    int ruleset[8] = {};
    getRuleSet(rule, ruleset);

    const int scale = 5;
    std::vector<int> image;
    image.reserve(WIDTH*HEIGHT);
    std::vector<int> cells(WIDTH, 0);
    cells[WIDTH/2] = 1;
    image.insert(image.begin(), cells.begin(), cells.end());

    bool quit = false;
    SDL_Event event;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH*scale, HEIGHT*scale, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);


    drawScene(renderer, image, WIDTH, HEIGHT);
    SDL_RenderPresent(renderer);
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
                        cells = std::vector(WIDTH, 0);
                        cells[WIDTH/2] = 1;
                        break;
                }
                
            }
        }

        iterGeneration(renderer, image, WIDTH, HEIGHT, cells, ruleset);
        SDL_Delay(DELAY);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}