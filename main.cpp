#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#define SDL_MAIN_HANDLED

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int SNAKE_SIZE = 20; // Taille du serpent et des pommes
const int INITIAL_LENGTH = 3;
const int NUM_APPLES = 3;  // Nombre de pommes à générer

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Apple {
    int x, y;
};

struct Snake {
    std::vector<SDL_Point> body;
    Direction dir;
};

bool checkCollision(const Snake& snake) {
    const SDL_Point& head = snake.body[0];
    // Collision avec le mur
    if (head.x < 0 || head.x >= WINDOW_WIDTH || head.y < 0 || head.y >= WINDOW_HEIGHT) {
        return true;
    }
    // Collision avec la queue
    for (size_t i = 1; i < snake.body.size(); ++i) {
        if (head.x == snake.body[i].x && head.y == snake.body[i].y) {
            return true;
        }
    }
    return false;
}

void generateApples(std::vector<Apple>& apples, const Snake& snake) {
    apples.clear();
    for (int i = 0; i < NUM_APPLES; ++i) {
        Apple apple;
        bool validPosition;
        do {
            validPosition = true;
            apple.x = (rand() % (WINDOW_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
            apple.y = (rand() % (WINDOW_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;

            // Vérifier que la pomme ne se trouve pas sur le serpent
            for (size_t j = 0; j < snake.body.size(); ++j) {
                if (apple.x == snake.body[j].x && apple.y == snake.body[j].y) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
        apples.push_back(apple);
    }
}

void moveSnake(Snake& snake) {
    SDL_Point newHead = snake.body[0];

    switch (snake.dir) {
        case UP: newHead.y -= SNAKE_SIZE; break;
        case DOWN: newHead.y += SNAKE_SIZE; break;
        case LEFT: newHead.x -= SNAKE_SIZE; break;
        case RIGHT: newHead.x += SNAKE_SIZE; break;
    }

    // Ajout de la nouvelle tête
    snake.body.insert(snake.body.begin(), newHead);
    // Supprime la dernière partie du serpent si aucune pomme n'est mangée
    snake.body.pop_back();
}

void growSnake(Snake& snake) {
    SDL_Point newTail = snake.body.back();
    snake.body.push_back(newTail);
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(0)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur d'initialisation SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Jeu du Serpent", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Erreur de création de la fenêtre: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Erreur de création du renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Snake snake;
    for (int i = 0; i < INITIAL_LENGTH; ++i) {
        SDL_Point newSegment;
        newSegment.x = INITIAL_LENGTH * SNAKE_SIZE - i * SNAKE_SIZE;
        newSegment.y = 0;
        snake.body.push_back(newSegment);
    }
    snake.dir = RIGHT;

    std::vector<Apple> apples;
    generateApples(apples, snake);

    int applesEaten = 0; // Compteur de pommes mangées

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: if (snake.dir != DOWN) snake.dir = UP; break;
                    case SDLK_DOWN: if (snake.dir != UP) snake.dir = DOWN; break;
                    case SDLK_LEFT: if (snake.dir != RIGHT) snake.dir = LEFT; break;
                    case SDLK_RIGHT: if (snake.dir != LEFT) snake.dir = RIGHT; break;
                }
            }
        }

        moveSnake(snake);
        // Vérification des collisions
        if (checkCollision(snake)) {
            running = false;
        }

        // Vérifier si le serpent mange une pomme
        for (size_t i = 0; i < apples.size(); ++i) {
            if (snake.body[0].x == apples[i].x && snake.body[0].y == apples[i].y) {
                growSnake(snake);
                apples.erase(apples.begin() + i); // Supprimer la pomme mangée
                applesEaten++; // Incrémenter le compteur de pommes mangées
                break;
            }
        }

        // Si le serpent a mangé les 3 pommes, régénérer 3 nouvelles pommes
        if (applesEaten == NUM_APPLES) {
            applesEaten = 0;
            generateApples(apples, snake);
        }

        // Rendu
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Fond blanc
        SDL_RenderClear(renderer);

        // Dessiner le serpent
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Serpent vert
        for (size_t i = 0; i < snake.body.size(); ++i) {
            SDL_Rect rect = { snake.body[i].x, snake.body[i].y, SNAKE_SIZE, SNAKE_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }

        // Dessiner les pommes avec un effet de bordure pour rendre un aspect physique
        for (size_t i = 0; i < apples.size(); ++i) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge pour le corps des pommes
            SDL_Rect appleRect = { apples[i].x, apples[i].y, SNAKE_SIZE, SNAKE_SIZE };
            SDL_RenderFillRect(renderer, &appleRect);

            // Dessiner une bordure noire autour des pommes
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &appleRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(100); // Vitesse du jeu
    }

    // Nettoyage
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
