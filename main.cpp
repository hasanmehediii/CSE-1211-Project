// Ctrl + Shift + i for auto indentation
// Compile: g++ -o name name.cpp -lSDL2 -lSDL2_ttf -lSDL2_image
// Run: ./game
// Right direction +x axis, Down: +y axis
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>

using namespace std;

const int win_wid = 1080;
const int win_ht = 720;
const int sqr_size = 35;
int speed = 35;
const int top_margin = 40;

struct SnakeSegment
{
    int x, y;
    SnakeSegment(int _x, int _y) : x(_x), y(_y) {}
};

struct Food
{
    int x, y;
    Food(int _x, int _y) : x(_x), y(_y) {}
};

bool checkCollision(const vector<SnakeSegment> &snake)
{
    for (size_t i = 1; i < snake.size(); ++i)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            return true;
        }
    }
    return false;
}

bool checkFoodCollision(const vector<SnakeSegment> &snake, const Food &food)
{
    SDL_Rect headRect = {snake[0].x, snake[0].y, sqr_size, sqr_size};
    SDL_Rect foodRect = {food.x, food.y, sqr_size, sqr_size};
    return SDL_HasIntersection(&headRect, &foodRect);
}

void renderText(SDL_Renderer *renderer, const char *text, int x, int y, TTF_Font *font, SDL_Color color)
{
    if (strlen(text) == 0)
    {
        return;
    }
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface)
    {
        cout << "Failed to create surface: " << TTF_GetError() << endl;
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        cout << "Failed to create texture: " << SDL_GetError() << endl;
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dstrect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void saveScore(const string &playerName, int score)
{
    ofstream file("data.txt", ios::app);
    if (file.is_open())
    {
        file << playerName << ": " << score << endl;
        file.close();
    }
    else
    {
        cout << "Unable to open file!" << endl;
    }
}
void displayHistory(SDL_Renderer *renderer, TTF_Font *font0, TTF_Font *font)
{
    // Load the background image
    SDL_Surface *imageSurface = IMG_Load("start_menu.jpg");
    if (!imageSurface)
    {
        cout << "Failed to load image: " << IMG_GetError() << endl;
        return;
    }
    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    ifstream file("data.txt");
    if (file.is_open())
    {
        SDL_Event event;
        bool quit = false;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color black = {0, 0, 0, 255};
        int y = 50;
        int maxVisibleLines = (win_ht - 100) / 30;

        renderText(renderer, "History", 450, 100, font0, black);

        vector<string> lines;
        string line;
        while (getline(file, line))
        {
            lines.push_back(line);
        }

        int scrollOffset = 0;

        while (!quit)
        {
            SDL_SetRenderDrawColor(renderer, 25, 25, 20, 255); // Black background
            SDL_RenderClear(renderer);

            // Render the background image
            SDL_RenderCopy(renderer, imageTexture, NULL, NULL);

            renderText(renderer, "History", 500, 15, font0, black); // do change here

            int displayStartIndex = max(0, scrollOffset);
            int displayEndIndex = min((int)lines.size(), scrollOffset + maxVisibleLines);

            y = 50;

            for (int i = displayStartIndex; i < displayEndIndex; ++i)
            {
                renderText(renderer, lines[i].c_str(), 50, y, font, black);
                y += 30;
            }

            SDL_RenderPresent(renderer);

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        quit = true;
                    }
                }
                else if (event.type == SDL_MOUSEWHEEL)
                {
                    if (event.wheel.y > 0)
                    {
                        // Scroll up
                        scrollOffset = max(0, scrollOffset - 1);
                    }
                    else if (event.wheel.y < 0)
                    {
                        // Scroll down
                        scrollOffset = min((int)lines.size() - maxVisibleLines, scrollOffset + 1);
                    }
                }
            }
        }

        file.close();
    }
    else
    {
        cout << "Unable to open file!" << endl;
    }

    // Clean up the image texture
    SDL_DestroyTexture(imageTexture);
}

string getPlayerName(SDL_Renderer *renderer, TTF_Font *font, TTF_Font *font0, int win_wid, int win_ht)
{
    SDL_Surface *imageSurface = IMG_Load("start_menu.jpg");                           // eita add korsi
    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface); // eita add korsi
    SDL_FreeSurface(imageSurface);                                                    // eita add korsi
    bool nameEntered = false;
    string playerName = "";

    SDL_Event event;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    bool shiftPressed = false;
    Uint32 cursorTimer = 0;
    bool cursorVisible = true;

    while (!nameEntered)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                exit(1);
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RETURN && playerName != "")
                {
                    nameEntered = true;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && playerName.length() > 0)
                {
                    playerName.pop_back();
                }
                else if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
                {
                    shiftPressed = true;
                }
                else if (event.key.keysym.sym == SDLK_SPACE)
                {
                    playerName += ' ';
                }
                else if ((event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) ||
                         (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9))
                {
                    char key = (char)event.key.keysym.sym;
                    if (shiftPressed)
                    {
                        key = toupper(key);
                    }
                    playerName += key;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
                {
                    shiftPressed = false;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - cursorTimer >= 500)
        {
            cursorVisible = !cursorVisible;
            cursorTimer = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        int boxWidth = 280;
        int boxHeight = 40;
        int boxX = ((win_wid - boxWidth) / 2) - 50;
        int boxY = (win_ht - boxHeight) / 2;

        SDL_RenderCopy(renderer, imageTexture, NULL, NULL);
        // input box
        SDL_Rect inputBox = {boxX, boxY, boxWidth, boxHeight};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black border
        SDL_RenderDrawRect(renderer, &inputBox);

        renderText(renderer, "Enter Your Name:", boxX - 50, boxY - 50, font0, black);
        renderText(renderer, playerName.c_str(), boxX + 10, boxY + 10, font, black);

        // blinking cursor
        if (cursorVisible)
        {
            int textWidth = 0;
            int textHeight = 0;
            TTF_SizeText(font, playerName.c_str(), &textWidth, &textHeight);
            SDL_Rect cursorRect = {boxX + 10 + textWidth, boxY + 10, 10, textHeight};
            // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White cursor
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black cursor
            SDL_RenderFillRect(renderer, &cursorRect);
        }

        SDL_RenderPresent(renderer); // eita add korsi
    }

    return playerName;
    SDL_DestroyTexture(imageTexture); // eita add korsi
}

void drawRoundedRect(SDL_Renderer *renderer, int x, int y, int w, int h, int radius)
{
    // Draw the four corners
    for (int i = 0; i < radius * 2; ++i)
    {
        for (int j = 0; j < radius * 2; ++j)
        {
            int dx = radius - i;
            int dy = radius - j;
            if (dx * dx + dy * dy <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, x + i, y + j);                 // Top-left
                SDL_RenderDrawPoint(renderer, x + w - i - 1, y + j);         // Top-right
                SDL_RenderDrawPoint(renderer, x + i, y + h - j - 1);         // Bottom-left
                SDL_RenderDrawPoint(renderer, x + w - i - 1, y + h - j - 1); // Bottom-right
            }
        }
    }

    // Draw the four sides
    SDL_Rect rects[4] = {
        {x + radius, y, w - radius * 2, radius},              // Top
        {x + radius, y + h - radius, w - radius * 2, radius}, // Bottom
        {x, y + radius, radius, h - radius * 2},              // Left
        {x + w - radius, y + radius, radius, h - radius * 2}  // Right
    };
    SDL_RenderFillRects(renderer, rects, 4);

    // Draw the center
    SDL_Rect centerRect = {x + radius, y + radius, w - radius * 2, h - radius * 2};
    SDL_RenderFillRect(renderer, &centerRect);
}
void SDL_RenderFillCircle(SDL_Renderer *renderer, int cx, int cy, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int dy = 1; dy <= radius; ++dy)
    {
        int dx = (int)floor(sqrt((2.0 * radius * dy) - (dy * dy)));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy - radius, cx + dx, cy + dy - radius);
        SDL_RenderDrawLine(renderer, cx - dx, cy - dy + radius, cx + dx, cy - dy + radius);
    }
}
void runGame(SDL_Renderer *renderer, const string &playerName, TTF_Font *font)
{
    SDL_Surface *bgSurface = IMG_Load("desert.jpg");
    if (!bgSurface)
    {
        cout << "Failed to load background image: " << IMG_GetError() << endl;
        return;
    }
    SDL_Texture *background = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    vector<SnakeSegment> snake; // initial snake positon
    snake.push_back(SnakeSegment(win_wid / 2, win_ht / 2));
    snake.push_back(SnakeSegment(win_wid / 2 - sqr_size, win_ht / 2));
    snake.push_back(SnakeSegment(win_wid / 2 - 2 * sqr_size, win_ht / 2));

    srand(time(0));
    Food food((rand() % (win_wid / sqr_size)) * sqr_size, (rand() % ((win_ht - top_margin) / sqr_size) * sqr_size + top_margin));
    int directionX = speed;
    int directionY = 0;
    int score = 0;
    int bestScore = 0;

    ifstream bestScoreFile("bestscore.txt");
    if (bestScoreFile.is_open())
    {
        bestScoreFile >> bestScore;
        bestScoreFile.close();
    }

    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    if (directionY != speed)
                    {
                        directionX = 0;
                        directionY = -speed;
                    }
                    break;
                case SDLK_DOWN:
                    if (directionY != -speed)
                    {
                        directionX = 0;
                        directionY = speed;
                    }
                    break;
                case SDLK_LEFT:
                    if (directionX != speed)
                    {
                        directionX = -speed;
                        directionY = 0;
                    }
                    break;
                case SDLK_RIGHT:
                    if (directionX != -speed)
                    {
                        directionX = speed;
                        directionY = 0;
                    }
                    break;
                }
            }
        }

        int newX = snake[0].x + directionX;
        int newY = snake[0].y + directionY;

        if (newX >= win_wid)
            newX = 0;
        else if (newX < 0)
            newX = win_wid - sqr_size;
        if (newY >= win_ht)
            newY = top_margin;
        else if (newY < top_margin)
            newY = win_ht - sqr_size;

        if (checkFoodCollision(snake, food))
        {
            score++;
            if (score % 2 == 0 && score != 0)
            {
                speed += 0.015;
            }
            if (score > bestScore)
            {
                bestScore = score;
                ofstream bestScoreFile("bestscore.txt");
                if (bestScoreFile.is_open())
                {
                    bestScoreFile << bestScore;
                    bestScoreFile.close();
                }
            }
            food.x = (rand() % (win_wid / sqr_size)) * sqr_size;
            food.y = (rand() % ((win_ht - top_margin) / sqr_size)) * sqr_size + top_margin;

            snake.push_back(SnakeSegment(snake.back().x, snake.back().y));
        }

        if (checkCollision(snake))
        {
            quit = true;
        }

        for (size_t i = snake.size() - 1; i > 0; --i)
        {
            snake[i].x = snake[i - 1].x;
            snake[i].y = snake[i - 1].y;
        }
        snake[0].x = newX;
        snake[0].y = newY;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, background, NULL, NULL);

        SDL_Rect topMarginRect = {0, 0, win_wid, top_margin};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &topMarginRect);

        for (size_t i = 0; i < snake.size(); ++i)
        {
            SDL_Rect sqrRect = {snake[i].x, snake[i].y, sqr_size, sqr_size};
            if (i == 0)
            {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green head
            }
            else
            {
                if ((i + (snake[i].x / sqr_size) + (snake[i].y / sqr_size)) % 2 == 0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Darker green
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // Lighter green
                }
            }
            SDL_RenderFillRect(renderer, &sqrRect);

            if (i == 0)
            {
                // head with eye
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green head
                SDL_RenderFillRect(renderer, &sqrRect);

                // eye
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black eyes
                SDL_Rect eye1 = {snake[i].x + 5, snake[i].y + 5, 5, 5};
                SDL_Rect eye2 = {snake[i].x + 20, snake[i].y + 5, 5, 5};
                SDL_RenderFillRect(renderer, &eye1);
                SDL_RenderFillRect(renderer, &eye2);
            }
            else
            {
                // sap er check pattern
                if ((i / 2) % 2 == 0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Darker green
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // Lighter green
                }
                SDL_RenderFillRect(renderer, &sqrRect);
            }
        }

        // Render red circular food
        SDL_Color redColor = {255, 0, 0, 255};
        SDL_RenderFillCircle(renderer, food.x + sqr_size / 2, food.y + sqr_size / 2, sqr_size / 2, redColor);

        // Render the score and best score
        SDL_Color white = {255, 255, 255, 255};
        string scoreText = "Score: " + to_string(score);
        string bestScoreText = "Best Score: " + to_string(bestScore);
        renderText(renderer, scoreText.c_str(), 10, 10, font, white);
        renderText(renderer, bestScoreText.c_str(), win_wid - 172, 10, font, white);

        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / 10);
    }

    SDL_DestroyTexture(background);

    cout << "Your score was: " << score << endl;
    saveScore(playerName, score);
}
bool gameOverScreen(SDL_Renderer *renderer, TTF_Font *font, TTF_Font *font0)
{
    SDL_Surface *imageSurface = IMG_Load("start_menu.jpg");
    if (!imageSurface)
    {
        cout << "Failed to load image: " << IMG_GetError() << endl;
        return false;
    }
    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    SDL_Event event;
    bool quit = false;
    bool restart = false;

    while (!quit && !restart)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = event.button.x;
                int y = event.button.y;
                if (x >= win_wid / 2 - 50 && x <= win_wid / 2 + 50 && y >= win_ht / 2 - 30 && y <= win_ht / 2 + 20)
                {
                    restart = true;
                }
                else if (x >= win_wid / 2 - 50 && x <= win_wid / 2 + 50 && y >= win_ht / 2 + 25 && y <= win_ht / 2 + 75)
                {
                    quit = true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the image
        SDL_RenderCopy(renderer, imageTexture, NULL, NULL);

        SDL_Color white = {255, 255, 255, 255};
        renderText(renderer, "Game Over", win_wid / 2 - 88, win_ht / 2 - 85, font0, white);

        // start button
        SDL_Rect startButton = {win_wid / 2 - 50, win_ht / 2 - 30, 100, 50};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &startButton);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &startButton);

        // exit button
        SDL_Rect exitButton = {win_wid / 2 - 50, win_ht / 2 + 25, 100, 50};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &exitButton);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &exitButton);

        renderText(renderer, "Restart", win_wid / 2 - 38, win_ht / 2 - 15, font, {0, 0, 0, 255});
        renderText(renderer, "Back", win_wid / 2 - 25, win_ht / 2 + 35, font, {0, 0, 0, 255});

        SDL_RenderPresent(renderer);
    }

    // pic add korle eita o dite hoy
    SDL_DestroyTexture(imageTexture);

    return restart;
}

int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return -1;
    }

    if (TTF_Init() == -1)
    {
        cout << "TTF_Init: " << TTF_GetError() << endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG))
    {
        cout << "IMG_Init: Failed to initialize image! IMG_Error: " << IMG_GetError() << endl;
        return -1;
    }

    SDL_Surface *iconSurface = IMG_Load("snake.png");
    if (!iconSurface)
    {
        cout << "Failed to load icon image: " << IMG_GetError() << endl;
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (!font)
    {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        return -1;
    }

    TTF_Font *font0 = TTF_OpenFont("arial.ttf", 40);
    if (!font0)
    {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        return -1;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);

    bool quit = false;

    while (!quit)
    {
        // eita main menu
        window = SDL_CreateWindow("Main Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_wid, win_ht, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_Surface *bgSurface = IMG_Load("start_menu.jpg");
        if (!bgSurface)
        {
            cout << "Failed to load background image: " << IMG_GetError() << endl;
            return -1;
        }
        SDL_Texture *background = SDL_CreateTextureFromSurface(renderer, bgSurface);
        SDL_FreeSurface(bgSurface);

        SDL_Event event;
        bool gameStarted = false;

        while (!quit && !gameStarted)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    int x = event.button.x;
                    int y = event.button.y;
                    if (x >= win_wid / 2 - 50 && x <= win_wid / 2 + 50 && y >= win_ht / 2 - 30 && y <= win_ht / 2 + 20)
                    {
                        gameStarted = true;
                    }
                    else if (x >= win_wid / 2 - 50 && x <= win_wid / 2 + 50 && y >= win_ht / 2 + 25 && y <= win_ht / 2 + 75)
                    {
                        quit = true;
                    }
                    // Eita history button er kaj korbe
                    else if (x >= win_wid / 2 - 50 && x <= win_wid / 2 + 50 && y >= win_ht / 2 + 80 && y <= win_ht / 2 + 130)
                    {
                        displayHistory(renderer, font0, font);
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Backgrund
            SDL_RenderCopy(renderer, background, NULL, NULL);

            // start button
            SDL_Rect startButton = {win_wid / 2 - 50, win_ht / 2 - 30, 100, 50}; // x.pos, y.pos, x.size, y.size
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &startButton);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &startButton);

            // exit button
            SDL_Rect exitButton = {win_wid / 2 - 50, win_ht / 2 + 25, 100, 50};
            SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
            SDL_RenderFillRect(renderer, &exitButton);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &exitButton);

            // history button
            SDL_Rect historyButton = {win_wid / 2 - 50, win_ht / 2 + 80, 100, 50};
            SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
            SDL_RenderFillRect(renderer, &historyButton);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &historyButton);

            SDL_Color white = {255, 255, 255, 255};
            SDL_Color black = {0, 0, 0, 255};
            SDL_Color blue = {0, 0, 255, 255};
            renderText(renderer, "Snake: The Desert King", 350, 45, font0, blue);
            renderText(renderer, "Press Green for Start, Red for Exit, Blue for History", 250, 100, font, black);

            // start text, button e add hoise
            renderText(renderer, "Start", win_wid / 2 - 23, win_ht / 2 - 15, font, black);

            // exit text, button e add hoise
            renderText(renderer, "Exit", win_wid / 2 - 20, win_ht / 2 + 35, font, black);

            // history text, button e add hoise
            renderText(renderer, "History", win_wid / 2 - 36, win_ht / 2 + 88, font, black);

            SDL_RenderPresent(renderer);
        }

        SDL_DestroyTexture(background);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        if (gameStarted)
        {
            window = SDL_CreateWindow("Player Name", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_wid, win_ht, SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            string playerName = getPlayerName(renderer, font, font0, win_wid, win_ht);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            bool gameRunning = true;
            while (gameRunning)
            {
                window = SDL_CreateWindow("Snake Game: Main Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_wid, win_ht, SDL_WINDOW_SHOWN); // SDL_WINDOW_RESIZEABLE
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
                runGame(renderer, playerName, font);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);

                window = SDL_CreateWindow("Game Over", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_wid, win_ht, SDL_WINDOW_SHOWN);
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
                gameRunning = gameOverScreen(renderer, font, font0);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
            }
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
