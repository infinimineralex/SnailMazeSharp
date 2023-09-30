#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

int main(void)
{
    // Initialization
    const int screenWidth = 2650;
    const int screenHeight = 1600;

    InitWindow(screenWidth, screenHeight, "TheBetterSnailMaze");
    InitAudioDevice(); // Initialize audio device

    Music music = LoadMusicStream("resources/ps.mp3");

    PlayMusicStream(music);

    Camera camera = {{0.2f, 0.4f, 0.2f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
    camera.fovy = 110.0f;
    Image imMap = LoadImage("resources/sus1.png");                               // Load cubicmap image (RAM)
    Texture2D cubicmap = LoadTextureFromImage(imMap);                            // Convert image to texture to display (VRAM)
    Mesh mesh = GenMeshCubicmap(imMap, (Vector3){1.0f, 3.0f, 1.0f});              //xyz
    Model model = LoadModelFromMesh(mesh);

    // How does it map the texture idk
    Texture2D texture = LoadTexture("resources/troll1.png");                     // Load map texture
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;             // Set map diffuse texture

    // Get map image data to be used for collision detection
    Color *mapPixels = LoadImageColors(imMap);
    UnloadImage(imMap);                                                           // Unload image from RAM
    Texture2D bill = LoadTexture("resources/billboardA.png");                     // Our Hmonster texture
    Vector3 enemyPosition = {-16.0f, -2.0f, -7.0f};                               // Position of Hmonster
    Vector3 mapPosition = {-16.0f, 0.0f, -7.0f};                                   // Set model position
    Rectangle source = {-16.0f, -7.0f, (float)bill.width, (float)bill.height};

    Texture2D adam = LoadTexture("resources/billboard.png");                      // zadam texture
    Vector3 enemy2Position = {-16.0f, -2.0f, -7.0f};
    float enemy2Speed = .01f;
    bool enemy2Frozen = false;                                                     // Flag to indicate whether the semite is frozen or not

    // NOTE: Billboard locked on axis-Y and it doesnt even work right
    Vector3 billUp = {-16.0f, -5.0f, -7.0f};
    Vector2 rotateOrigin = {0.0f};
    SetCameraMode(camera, CAMERA_FIRST_PERSON);                                   // Set camera mode

    float rotation = 0.0f;
    SetTargetFPS(165);
    bool CheckRayBoxCollision(Ray ray, BoundingBox box)
    {
    float t[8];

    t[0] = (box.min.x - ray.position.x) / ray.direction.x;
    t[1] = (box.max.x - ray.position.x) / ray.direction.x;
    t[2] = (box.min.y - ray.position.y) / ray.direction.y;
    t[3] = (box.max.y - ray.position.y) / ray.direction.y;
    t[4] = (box.min.z - ray.position.z) / ray.direction.z;
    t[5] = (box.max.z - ray.position.z) / ray.direction.z;

    float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
    float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

    return tmax >= tmin && tmax >= 0;
}


    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        Vector3 oldCamPos = camera.position; // Store old camera position

        UpdateCamera(&camera);
        UpdateMusicStream(music);

        // Check player collision (we simplify to 2D collision detection)
        Vector2 playerPos = {camera.position.x, camera.position.z};
        float playerRadius = 0.1f; // Collision radius (player is modeled as a cylinder for collision)

        int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
        int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f + 0.5f);

        // Out-of-limits security check
        if (playerCellX < 0)
            playerCellX = 0;
        else if (playerCellX >= cubicmap.width)
            playerCellX = cubicmap.width - 1;

        if (playerCellY < 0)
            playerCellY = 0;
        else if (playerCellY >= cubicmap.height)
            playerCellY = cubicmap.height - 1;

        // Check map collisions using image data and player position
        for (int y = 0; y < cubicmap.height; y++)
        {
            for (int x = 0; x < cubicmap.width; x++)
            {
                if ((mapPixels[y * cubicmap.width + x].r == 255) && // Collision: white pixel, only check R channel for now
                    (CheckCollisionCircleRec(playerPos, playerRadius,
                                             (Rectangle){mapPosition.x - 0.5f + x * 1.0f, mapPosition.z - 0.5f + y * 1.0f, 1.0f, 1.0f})))
                {
                    // Collision detected, reset camera position
                    camera.position = oldCamPos;
                }
            }
        }

        // Calculate direction from enemy to player
        Vector3 playerPosition = {camera.position.x, camera.position.y, camera.position.z};
        Vector3 direction = Vector3Normalize(Vector3Subtract(playerPosition, enemyPosition));

        // Update enemy position based on direction and speed
        float enemySpeed = 0.005f; // Adjust speed as needed
        enemyPosition = Vector3Add(enemyPosition, Vector3Scale(direction, enemySpeed));

        // Calculate direction from new enemy to player
        Vector3 direction2 = Vector3Normalize(Vector3Subtract(playerPosition, enemy2Position));

        Ray ray = {camera.position, Vector3Normalize(Vector3Subtract(enemy2Position, camera.position))};

    float distanceToAdam = Vector3Distance(camera.position, enemy2Position);
    float closestDistance = distanceToAdam;
    bool rayCollided = false;

    // Iterate through map pixels along the ray
    for (float t = 0.0f; t < distanceToAdam; t += 0.1f)
    {
        Vector3 pointAlongRay = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        int cellX = (int)(pointAlongRay.x - mapPosition.x + 0.5f);
        int cellY = (int)(pointAlongRay.z - mapPosition.z + 0.5f + 0.5f);

        // Check if the point along the ray collides with any map pixel (should probably change to collision with Adam)
        if (cellX >= 0 && cellX < cubicmap.width && cellY >= 0 && cellY < cubicmap.height)
        {
            if (mapPixels[cellY * cubicmap.width + cellX].r == 255)
            {
                rayCollided = true;
                break;
            }
        }
    }

    if (!rayCollided)
    {
        enemy2Frozen = true;
    }
    else
    {
        enemy2Frozen = false;
    }

    // Update Adam's position if he's not frozen
    if (!enemy2Frozen)
    {
        enemy2Position = Vector3Add(enemy2Position, Vector3Scale(direction2, enemy2Speed));
    }

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawModel(model, mapPosition, 1.0f, WHITE);                                  // Draw maze map
        DrawBillboardRec(camera, bill, source, enemyPosition, (Vector2){2.0f, 2.0f}, WHITE); // Draw enemy (Hmonster)
        DrawBillboardRec(camera, adam, source, enemy2Position, (Vector2){2.0f, 2.0f}, WHITE);  // Draw enemy (Adam)
        EndMode3D();

        DrawTextureEx(cubicmap, (Vector2){GetScreenWidth() - cubicmap.width * 4.0f - 20, 20.0f}, 0.0f, 4.0f, WHITE);
        DrawRectangleLines(GetScreenWidth() - cubicmap.width * 4 - 20, 20, cubicmap.width * 4, cubicmap.height * 4, GREEN);

        // Draw player position radar
        DrawRectangle(GetScreenWidth() - cubicmap.width * 4 - 20 + playerCellX * 4, 20 + playerCellY * 4, 4, 4, RED);

        DrawFPS(10, 10);

        EndDrawing();
    }

    // De-Initialization
    UnloadImageColors(mapPixels);   // Unload color array
    UnloadTexture(bill);            // Unload texture
    UnloadTexture(cubicmap);        // Unload cubicmap texture
    UnloadTexture(texture);         // Unload map texture
    UnloadModel(model);             // Unload map model
    UnloadMusicStream(music);       // Unload music stream buffers from RAM
    CloseAudioDevice();             // Close audio device (music streaming is automatically stopped)

    CloseWindow();                  // Close window and OpenGL context

    return 0;
}

