int main(){
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Error();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Something",
            0, 0, 1, 1, // NOTE: The window will resize itself to fullscreen with SDL_WINDOW_FULLSCREEN_DESKTOP
            SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if(!window){
        SDL_Error();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!renderer){
        Destroy(window);
        SDL_Error();
        SDL_Quit();
        return 1;
    }

    std::string bitmap_path = RessourcePath() + "debug_smallUV.bmp";
    SDL_Texture* texture = TextureFromBitmap(bitmap_path.c_str(), renderer);
    if(!texture){
        Destroy(renderer, window);
        SDL_Error();
        SDL_Quit();
        return 1;
    }

    std::string spritesheet_path = RessourcePath() + "debug_spritesheet_50x50_2_4.bmp";
    SDL_Texture* spritesheet = TextureFromBitmap(spritesheet_path.c_str(), renderer);
    if(!spritesheet){
        Destroy(texture, renderer, window);
        SDL_Error();
        SDL_Quit();
        return 1;
    }
    Animation* animation = AnimationFromSpriteSheet(spritesheet, 100, 100, 2, 0, 4);

    std::string adventurer_path = RessourcePath() + "adventurer_50x37_7_72.bmp";
    SDL_Texture* adventurer_sheet = TextureFromBitmap(adventurer_path.c_str(), renderer);
    if(!adventurer_sheet){
        Destroy(spritesheet, texture, renderer, window);
        SDL_Error();
        SDL_Quit();
        return 1;
    }
    Animation* adventurer = AnimationFromSpriteSheet(adventurer_sheet, 50, 37, 7, 0, 72);

    std::string blob_path = RessourcePath() + "blob.bmp";
    SDL_Texture* blob_sheet = TextureFromBitmap(blob_path.c_str(), renderer);
    if(!blob_sheet){
        Destroy(spritesheet, texture, renderer, window);
        SDL_Error();
        SDL_Quit();
        return 1;
    }
    Animation* blob = AnimationFromSpriteSheet(blob_sheet, 32, 32, 5, 0, 23);

    bool appRunning = true;
    unsigned int animation_step = 0;
    while(appRunning){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                appRunning = false;
            }
            if(event.type == SDL_KEYDOWN){
                appRunning = false;
            }
            if(event.type == SDL_MOUSEBUTTONDOWN){
                if(event.button.button == SDL_BUTTON_LEFT){
                    ++animation_step;
                }else if(event.button.button == SDL_BUTTON_RIGHT){
                    --animation_step;
                }
                PRINT(animation_step);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        TextureBlit(texture, renderer, 0, 0);

        SDL_Rect texture_destination = {1500, 500, 100, 100};
        SDL_RenderCopy(renderer, texture, NULL, &texture_destination);

        SDL_Rect spritesheet_destination = {500, 500, 150, 150};
        SDL_RenderCopy(renderer, spritesheet, NULL, &spritesheet_destination);

        AnimationBlit(animation, renderer, animation_step, 0, 0);
        SDL_RenderSetScale(renderer, 5.f, 5.f);
        AnimationBlit(adventurer, renderer, animation_step, 100, 100);
        SDL_RenderSetScale(renderer, 10.f, 10.f);
        AnimationBlit(blob, renderer, animation_step, 10, 30);
        SDL_RenderSetScale(renderer, 1.f, 1.f);


        SDL_Rect square = {150, 150, 50, 50};
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &square);

        square.x = 200 + 50 * animation_step;
        SDL_RenderDrawRect(renderer, &square);

        square.x = 125;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x80);
        SDL_RenderFillRect(renderer, &square);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_SetRenderDrawColor(renderer, 0x80, 0xFF, 0x00, 0x80);
        SDL_RenderDrawLine(renderer, 100, 175, 350, 175);

        SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0xFF, 0xFF);
        for(int ipoint = 0; ipoint < 400; ipoint += 2){
            SDL_RenderDrawPoint(renderer, 100 + ipoint, 176);
        }

        SDL_RenderPresent(renderer);
    }

    Destroy(adventurer, animation, spritesheet, texture, renderer, window);
    SDL_Quit();

    return  0;

}
