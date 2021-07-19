#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"
#include <vector>

#define PLATFORM_COUNT 27
#define ENEMY_COUNT 3

struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* enemies;

    Entity* finishFlag;
    bool gameStart = true;
    bool zeroKill = true;
    bool gameOver = false;
    bool successful = false;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Rise of the AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Initialize Game Objects

    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(4.5, 2.5, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("george_0.png");

    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };

    state.player->animIndices = state.player->animLeft;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;

    state.player->height = 0.8f;
    state.player->width = 0.8f;

    state.player->jumpPower = 4.8f;


    //platforms
    state.platforms = new Entity[PLATFORM_COUNT];

    GLuint platformTextureID = LoadTexture("platformPack_tile001.png");

    for (int i = 0; i < 11; i++) {
        state.platforms[i].position = glm::vec3(-5 + i, -3.95, 0);
    }

    int index = 0;
    for (int i = 11; i < 19; i++) {
        state.platforms[i].position = glm::vec3(-5 + index, -1.22, 0);
        index++;
    }

    index = 0;
    for (int i = 19; i < PLATFORM_COUNT - 1; i++) {
        state.platforms[i].position = glm::vec3(-2 + index, 1.5, 0);
        index++;
    }

    //starter platform
    GLuint starterPlatformTextureID = LoadTexture("platformPack_tile025.png");
    state.platforms[26].position = glm::vec3(4.5, 2, 0);


    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        //state.platforms[i].height = .95;
        //state.platforms[i].width = .9;
        state.platforms[i].Update(0, NULL, NULL, 0, NULL, 0, NULL);
    }


    //finish flag
    GLuint finishFlagTextureID = LoadTexture("finish_flag.png");
    state.finishFlag = new Entity();
    state.finishFlag->entityType = FINISH;
    state.finishFlag->position = glm::vec3(-4.5, -3, 0);
    state.finishFlag->textureID = finishFlagTextureID;
    state.finishFlag->width = .7;
    state.finishFlag->Update(0, NULL, NULL, 0, NULL, 0, NULL);


    //enemies
    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemyTextureID = LoadTexture("Sprite_enemy_scorpion.png");


    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].textureID = enemyTextureID;
        state.enemies[i].speed = .7;
        state.enemies[i].width = .6;
        state.enemies[i].height = 0.4f;

        state.enemies[i].animRight = new int[6]{ 18, 19, 20, 21, 22, 23 };
        state.enemies[i].animLeft = new int[6]{ 12, 13, 14, 15, 16, 17 };

        state.enemies[i].animFrames = 6;
        state.enemies[i].animIndex = 0;
        state.enemies[i].animTime = 0;
        state.enemies[i].animCols = 6;
        state.enemies[i].animRows = 4;
        state.enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        state.enemies[i].animIndices = state.enemies->animRight;
    }

    state.enemies[0].position = glm::vec3(-.75, 2.75, 0);
    state.enemies[0].aiType = WAITANDGO;
    state.enemies[0].aiState = IDLE;

    state.enemies[1].animIndices = state.enemies->animLeft;
    state.enemies[1].position = glm::vec3(.75, 0, 0);
    state.enemies[1].movement = glm::vec3(-.75, 0, 0);
    state.enemies[1].aiType = WALKER;
    state.enemies[1].aiState = WALKING;

    state.enemies[2].position = glm::vec3(-2.75, -3, 0);
    state.enemies[2].movement = glm::vec3(.75, 0, 0);
    state.enemies[2].jumpPower = 3;
    state.enemies[2].aiType = JUMPER;
    state.enemies[2].aiState = JUMPING;


}

void ProcessInput() {

    state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            state.gameStart = false;
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                break;

            case SDLK_SPACE:
                if (state.player->collidedBottom) {
                    state.player->jump = true;
                }
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->animIndices = state.player->animLeft;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->animIndices = state.player->animRight;
    }


    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {

    if (state.gameOver) {
        return;
    }

    float ticks = (float)SDL_GetTicks() / 1000.0f; //first we get how much time has passed
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
       
        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.finishFlag);
        }
        
        state.player->Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.finishFlag);

        deltaTime -= FIXED_TIMESTEP;
    }


    accumulator = deltaTime;
    state.gameOver = state.player->gameOver;
    state.successful = state.player->successful;

    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (state.enemies[i].isActive == false)
            state.zeroKill = false;
    }
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
    float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
         offset + (-0.5f * size), 0.5f * size,
         offset + (-0.5f * size), -0.5f * size,
         offset + (0.5f * size), 0.5f * size,
         offset + (0.5f * size), -0.5f * size,
         offset + (0.5f * size), 0.5f * size,
         offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
        u, v,
        u, v + height,
        u + width, v,
        u + width, v + height,
        u + width, v,
        u, v + height,
            });

    } // end of for loop
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Render(&program);
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].Render(&program);
    }

    state.player->Render(&program);
    state.finishFlag->Render(&program);

    GLuint fontTexture = LoadTexture("font2.png");
    if (state.gameStart) {
        DrawText(&program, fontTexture, "Kill all enemies & reach the end to win!", .43, -0.20f, glm::vec3(-4.55, 0, 0));
    }
    else if (state.zeroKill && !state.gameOver) {
        DrawText(&program, fontTexture, "Kill enemies by jumping on them", .43, -0.20f, glm::vec3(-3.25, 0, 0));
    }

    if (state.gameOver)
    {
        if (state.successful) {
            bool allGone = true;
            for (int i = 0; i < ENEMY_COUNT; i++) {
                if (state.enemies[i].isActive)
                    allGone = false;
            }

            if (allGone)
                DrawText(&program, fontTexture, "You Win!", .75, -0.33f, glm::vec3(-1.0, 0, 0));
            else
                DrawText(&program, fontTexture, "You Lose, Didn't Kill All Enemies", .5, -0.23f, glm::vec3(-4.15, 0, 0));
        }

        else
            DrawText(&program, fontTexture, "You Lose :(", .75, -0.35f, glm::vec3(-2.0, 0, 0));

    }

    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}