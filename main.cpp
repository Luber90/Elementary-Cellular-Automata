#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#define DELAY 50
#define RULE 30
#define WIDTH 100
#define HEIGHT 90



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

constexpr void getRuleSet(int* ruleset){
    for(int i = 0; i < 8; i++) {
        ruleset[7-i] = (RULE>>i)&0b1;
    }
}

int getNewState(int left, int mid, int right) {
    int ruleset[8] = {0};
    getRuleSet(ruleset); 
    return ruleset[7-((left<<2)+(mid<<1)+right)];
}

std::vector<int> getNextCells(const std::vector<int> cells) {
    std::vector<int> next_cells(cells.size(), 0);
    for(int i = 0; i < cells.size(); i++) {
        int left_id = i == 0 ? cells.size()-1 : i-1;
        int right_id = (i+1) % cells.size();

        next_cells[i] = getNewState(cells[left_id], cells[i], cells[right_id]);
    }
    return next_cells;
} 

void iterGeneration(SDL_Renderer* renderer, std::vector<int>& image, int width, int height, std::vector<int>& cells) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if(image.size() == width*height) image.erase(image.begin(), image.begin()+cells.size());
    auto next_cells = getNextCells(cells);
    image.insert(image.end(), next_cells.begin(), next_cells.end());

    cells = next_cells;
    drawScene(renderer, image, width, height);
    SDL_RenderPresent(renderer);
}

int main() {
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
        }

        iterGeneration(renderer, image, WIDTH, HEIGHT, cells);
        SDL_Delay(DELAY);
    }
    
    return 0;
}