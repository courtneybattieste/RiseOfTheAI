#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);;
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;

    modelMatrix = glm::mat4(1.0f);
    objectCollided = NULL;
    gameOver = false;
    successful = false;
    jumpTime = 0.0f;
}

bool Entity::CheckCollision(Entity* other) {

    if (other == this) return false; // you shouldn't be able to collide with yourself

    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) {
        return true;
    }

    return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            if (object->entityType == ENEMY) {
                if (velocity.y > 0) {
                    velocity.y = 0;
                    collidedTop = true;
                }
                else if (velocity.y < 0) {
                    velocity.y = 0;
                    collidedBottom = true;
                    object->isActive = false;
                }
            }
            else {
                float ydist = fabs(position.y - object->position.y);
                float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
                if (velocity.y > 0) {
                    position.y -= penetrationY;
                    velocity.y = 0;
                    collidedTop = true;
                }
                else if (velocity.y < 0) {
                    position.y += penetrationY;
                    velocity.y = 0;
                    collidedBottom = true;
                }
            }
            objectCollided = object;
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            if (object->entityType == ENEMY) {
                if (velocity.x >= 0) {
                    velocity.x = 0;
                    collidedRight = true;
                }
                else if (velocity.x <= 0) {
                    velocity.x = 0;
                    collidedLeft = true;
                }
            }
            else {
                float xdist = fabs(position.x - object->position.x);
                float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
                if (velocity.x > 0) {
                    position.x -= penetrationX;
                    velocity.x = 0;
                    collidedRight = true;
                }
                else if (velocity.x < 0) {
                    position.x += penetrationX;
                    velocity.x = 0;
                    collidedLeft = true;
                }
            }
            objectCollided = object;

        }
    }
}

void Entity::AIWalker() {

    if (position.x <= -4.75) { //Move Right
        movement = glm::vec3(.75, 0, 0);
        animIndices = animRight;
    }
    else if (position.x >= 1.75) { //Move Left
        movement = glm::vec3(-.75, 0, 0);
        animIndices = animLeft;
    }
}

void Entity::AIWaitAndGo(Entity* player) {
    switch (aiState) {
    case IDLE:
        if (glm::distance(position, player->position) < 3.0f) {
            aiState = WALKING;
        }
        break;

    case WALKING:
        if (player->position.x < position.x) { //move to the left
            movement = glm::vec3(-.75, 0, 0);
            animIndices = animLeft;
        }
        else {
            movement = glm::vec3(.75, 0, 0); //otherwise move to the right
            animIndices = animRight;
        }
        break;
    }
}

void Entity::AIJumper() {

    if (jumpTime >= 2.5f)
        jump = true;

    if (position.x <= -4.75) { //Move Right
        movement = glm::vec3(.75, 0, 0);
        animIndices = animRight;
    }

    else if (position.x >= 4.75) { //Move Left
        movement = glm::vec3(-.75, 0, 0);
        animIndices = animLeft;
    }

}

void Entity::AI(Entity* player) {
    switch (aiType) {
    case WALKER:
        AIWalker();
        break;

    case WAITANDGO:
        AIWaitAndGo(player);
        break;

    case JUMPER:
        AIJumper();
        break;
    }
}


void Entity::Update(float deltaTime, Entity* player, Entity* platforms, int platformCount, Entity* enemies, int enemyCount, Entity* finalFlag)
{
    if (isActive == false) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY) {
        jumpTime += deltaTime;
        AI(player);
    }

    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndex = 0;
        }
    }

    if (jump) {
        jump = false;
        jumpTime = 0;
        velocity.y += jumpPower;
    }

    velocity.x = movement.x * speed; //this is so the character has instant velocity
    velocity += acceleration * deltaTime; //if we are moving, we are going to keep adding to velocity

    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(platforms, platformCount);// Fix if needed

    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(platforms, platformCount);// Fix if needed


    if (entityType == PLAYER) {

        if (CheckCollision(finalFlag))
        {
            gameOver = true;
            successful = true;
            return;
        }

        CheckCollisionsY(enemies, enemyCount);
        CheckCollisionsX(enemies, enemyCount);

        if (objectCollided != NULL && objectCollided->entityType == ENEMY) {
            if (collidedTop || collidedRight || collidedLeft) {
                gameOver = true;
                successful = false;
                return;
            }
        }
    }

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {

    if (isActive == false) return;

    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}