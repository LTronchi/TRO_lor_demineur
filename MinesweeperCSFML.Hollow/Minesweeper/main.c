#include <SFML/Audio.h>
#include <SFML/Graphics.h>

#include "basics.h"
#include "Grid.h"

#define DEBUG_CLEAN false

int main(void)
{
    // Define the video mode and create the window
    sfVideoMode mode = { WIDTH, HEIGHT, 32 };
    sfRenderWindow* window;
    sfEvent event; // Event variable to handle events

    /* Create the main window */
    window = sfRenderWindow_create(mode, "Minesweeper", sfClose, NULL);
    if (!window)
    {
        return NULL_WINDOW;
    }

    // Seed the random number generator (to use rand() later)
    srand((unsigned int)time(NULL));

    // Initialize the game grid and all you need to set up before starting the game loop (Creating Grid, Planting bombs, etc.)
    // ...
    Grid* grid = GridCreate();
    if (!grid) {
        sfRenderWindow_destroy(window);
        return EXIT_FAILURE;
    }

    printf("Start Game ! \n");
    bool bFirstTouch = true;
    int gameState = 0;
    /* Start the game loop */

    sfFont* testFont = sfFont_createFromFile("arial.ttf");
    if (!testFont) {
        printf("ERREUR: Impossible de charger la police arial.ttf\n");
    }
    else {
        printf("SUCCES: Police chargee\n");
        sfFont_destroy(testFont);
    }

    while (sfRenderWindow_isOpen(window))
    {
        /* Process events */
        while (sfRenderWindow_pollEvent(window, &event))
        {
            /* Close window : exit */
            if (event.type == sfEvtClosed)
            {
                sfRenderWindow_close(window);
            }

            // Handle all events here
            // ...
            if (event.type == sfEvtMouseButtonPressed && gameState == 0)
            {
                if (event.mouseButton.button == sfMouseLeft)
                {
                    // Get the cell position from mouse click
                    sfVector2i mousePos = { event.mouseButton.x, event.mouseButton.y };
                    sfVector2i cellPos = {
                        mousePos.x / 30,  // Assuming cell size is 30x30
                        mousePos.y / 30
                    };

                    // Check if position is within grid bounds
                    if (cellPos.x >= 0 && cellPos.x < GRID_SIZE &&
                        cellPos.y >= 0 && cellPos.y < GRID_SIZE)
                    {
                        // First click - plant bombs avoiding this cell
                        if (bFirstTouch) {
                            GridPlantBomb(grid, BOMB_COUNT, cellPos);
                            bFirstTouch = false;
                        }

                        // Reveal the cell
                        int result = CellReveal(grid, cellPos);
                        if (result == FAILURE) {
                            printf("Game Over! You hit a bomb!\n");
                            gameState = FAILURE;
                        }
                        else if (result == SUCCESS) {
                            printf("Congratulations! You won!\n");
                            gameState = SUCCESS;
                        }
                    }
                }
                else if (event.mouseButton.button == sfMouseRight)
                {
                    // Right click to flag/unflag
                    sfVector2i mousePos = { event.mouseButton.x, event.mouseButton.y };
                    sfVector2i cellPos = {
                        mousePos.x / 30,
                        mousePos.y / 30
                    };

                    if (cellPos.x >= 0 && cellPos.x < GRID_SIZE &&
                        cellPos.y >= 0 && cellPos.y < GRID_SIZE)
                    {
                        CellFlag(grid, cellPos);
                    }
                }
            }
        }


        /* Clear the screen */
        sfRenderWindow_clear(window, sfBlack);

        // Draw everything here
        // ...
        GridDraw(grid, window);




        /* Update the window */
        sfRenderWindow_display(window);
    }

    /* Cleanup resources */
    // ...
    GridDestroy(grid);
    sfRenderWindow_destroy(window);

    return SUCCESS;
}
