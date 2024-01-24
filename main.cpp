#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

#define DELAY 25
#define WIDTH 200
#define HEIGHT 180
#define SCALE 5
#define START_RULE 129
#define MONO 0
#define SHOW_FRAME_TIME


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


void drawScene(SDL_Surface *window_surface, SDL_Window *window, const std::vector<RGB> image, unsigned int width, unsigned int height) {
    const unsigned int threads_n = 4;
    unsigned int image_size = image.size();
    SDL_LockSurface(window_surface);
    auto job = [height, width, image_size, &image, &window_surface](unsigned int start, unsigned int end){
        for (unsigned int y = start; y < end; y++) {
            for (unsigned int x = 0; x < width; x++) {
                if(y*width+x < image_size){
                    auto current_pixel = image[y*width+x];
                    if(SCALE > 1) {
                        for(int k = 0; k < SCALE; k++) {
                            for(int l = 0; l < SCALE; l++){
                                static_cast<uint32_t*>(window_surface->pixels)[(y*SCALE+k)*SCALE*width+x*SCALE+l] = current_pixel.b + (current_pixel.g<<8) + (current_pixel.r<<16);
                            }    
                        }
                    } else {
                        static_cast<uint32_t*>(window_surface->pixels)[y*width+x] = current_pixel.b + (current_pixel.g<<8) + (current_pixel.r<<16);
                    }
                }
            }
        }
    };
    std::vector<std::thread> threads;
    for(unsigned int i = 0; i < threads_n; i++){
        threads.push_back(std::thread(job, i*(height/threads_n), (i+1)*(height/threads_n)));
    }
    for(auto & t : threads) {
        t.join();
    }
    SDL_UnlockSurface(window_surface);
    SDL_UpdateWindowSurface(window);
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
    for(unsigned int i = 0; i < cells.size(); i++) {
        int left_id = i == 0 ? cells.size()-1 : i-1;
        int right_id = (i+1) % cells.size();

        next_cells[i] = getNewState(cells[left_id], cells[i], cells[right_id], ruleset);
    }
    return next_cells;
}

void scroll(std::vector<RGB>& image, unsigned int width, unsigned int height, std::vector<RGB> next_cells) {
    if(image.size() == width*height) image.erase(image.begin(), image.begin()+next_cells.size());
    image.insert(image.end(), next_cells.begin(), next_cells.end());
}

void iterGeneration(SDL_Window* window, SDL_Surface *window_surface, std::vector<RGB>& image,
                    int width, int height, std::vector<RGB>& cells, bool *ruleset) {
    auto next_cells = getNextCells(cells, ruleset);
    scroll(image, width, height, next_cells);

    cells = next_cells;
    drawScene(window_surface, window, image, width, height);
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
    SDL_Surface *window_surface = nullptr;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH*SCALE, HEIGHT*SCALE, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, SCALE, SCALE);
    window_surface = SDL_GetWindowSurface(window);


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

        #ifdef SHOW_FRAME_TIME
        auto t_1 = std::chrono::system_clock::now();
        #endif

        if(first_frame){
            scroll(image, WIDTH, HEIGHT, cells);
            drawScene(window_surface, window, image, WIDTH, HEIGHT);
            
            first_frame = false;
        } else {
            iterGeneration(window, window_surface, image, WIDTH, HEIGHT, cells, ruleset);
        }

        #ifdef SHOW_FRAME_TIME
        std::cout << "render time: " <<
         std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t_1).count()/1000.0 <<
         "ms" << std::endl;
        #endif

        SDL_Delay(DELAY);
    }
    
    SDL_DestroyWindowSurface(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}