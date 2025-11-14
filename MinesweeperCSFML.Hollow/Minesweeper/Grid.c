#include <SFML/Graphics.h>

#include "Grid.h"

static sfFont* globalFont = NULL;
Cell* CellCreate(sfVector2f size, sfVector2f pos, sfColor color)
{
	// Initialize all cell properties
	// ...
	sfFont_createFromFile("Resources/Roboto-Regular.ttf")
	Cell* newCell = (Cell*)malloc(sizeof(Cell));
	if (newCell == NULL) {
		return NULL;
	}

	// Initialisation de la forme
	newCell->shape = sfRectangleShape_create();
	sfRectangleShape_setSize(newCell->shape, size);
	sfRectangleShape_setPosition(newCell->shape, pos);
	sfRectangleShape_setFillColor(newCell->shape, color);
	sfRectangleShape_setOutlineColor(newCell->shape, sfBlack);
	sfRectangleShape_setOutlineThickness(newCell->shape, 1.0f);

	// Initialisation du texte - PROBLÈME PRINCIPAL ICI
	newCell->text = sfText_create();

	// VOUS DEVEZ AVOIR UNE POLICE CHARGÉE QUELQUE PART !
	// Ajoutez cette variable globale en haut de votre fichier :
	// static sfFont* font = NULL;

	// Et dans GridCreate() ou main(), initialisez-la :
	// if (font == NULL) {
	//     font = sfFont_createFromFile("arial.ttf");
	// }
	// sfText_setFont(newCell->text, font);

	sfText_setCharacterSize(newCell->text, 20);
	sfText_setFillColor(newCell->text, sfBlack);
	sfText_setString(newCell->text, ""); // Initialiser avec texte vide

	// Positionnement du texte - CORRECTION ICI
	// Ne pas utiliser les bounds quand le texte est vide
	sfVector2f textPos = {
		pos.x + (size.x - 10) / 2, // Approximation pour centrage
		pos.y + (size.y - 20) / 2  // Approximation pour centrage
	};
	sfText_setPosition(newCell->text, textPos);

	// Initialisation des propriétés
	newCell->bDiscovered = false;
	newCell->bFlagged = false;
	newCell->bPlanted = false;
	newCell->explosiveNeighbor = 0;

	return newCell;
}

void CellDraw(Cell* cell, sfRenderWindow* window)
{
	// Draw the cell shape and text on the window
	// ...
	if (cell->bDiscovered) {
		if (cell->bPlanted) {
			// Bombe
			sfText_setString(cell->text, "X");
			sfText_setColor(cell->text, sfBlack);
			sfRenderWindow_drawText(window, cell->text, NULL);
		}
		else if (cell->explosiveNeighbor > 0) {
			// Numéro
			char numberStr[2];
			snprintf(numberStr, sizeof(numberStr), "%d", cell->explosiveNeighbor);
			sfText_setString(cell->text, numberStr);

			// Couleurs différentes selon le numéro
			switch (cell->explosiveNeighbor) {
			case 1: sfText_setColor(cell->text, sfBlue); break;
			case 2: sfText_setColor(cell->text, sfGreen); break;
			case 3: sfText_setColor(cell->text, sfRed); break;
			case 4: sfText_setColor(cell->text, sfColor_fromRGB(0, 0, 128)); break;
			case 5: sfText_setColor(cell->text, sfColor_fromRGB(128, 0, 0)); break;
			case 6: sfText_setColor(cell->text, sfColor_fromRGB(0, 128, 128)); break;
			case 7: sfText_setColor(cell->text, sfBlack); break;
			case 8: sfText_setColor(cell->text, sfColor_fromRGB(64, 64, 64)); break;
			}
			sfRenderWindow_drawText(window, cell->text, NULL);
		}
	}
	else if (cell->bFlagged) {
		// Drapeau
		sfText_setString(cell->text, "F");
		sfText_setColor(cell->text, sfRed);
		sfRenderWindow_drawText(window, cell->text, NULL);
	}

	// TOUJOURS dessiner le rectangle par-dessus le texte
	sfRenderWindow_drawRectangleShape(window, cell->shape, NULL);
}


int CellReveal(Grid* grid, sfVector2i cellGridPos)
{
	Cell* cell = grid->cells[cellGridPos.x][cellGridPos.y];
	if (cell->bDiscovered || cell->bFlagged) {
		return 0;
	}

	// If the cell is planted with a bomb, return FAILURE to indicate game over
	if (cell->bPlanted) {
		return FAILURE;
	}

	// Change the cell's appearance to revealed (lighter color) and mark it as discovered
	sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(200, 200, 200));
	cell->bDiscovered = true;

	// If the cell has explosive neighbors, display the number
	if (cell->explosiveNeighbor > 0) {
		// Convertir le nombre en texte et l'afficher
		char numberStr[2];
		snprintf(numberStr, sizeof(numberStr), "%d", cell->explosiveNeighbor);
		sfText_setString(cell->text, numberStr);

		// Changer la couleur du texte selon le nombre (optionnel mais visuellement utile)
		switch (cell->explosiveNeighbor) {
		case 1: sfText_setColor(cell->text, sfBlue); break;
		case 2: sfText_setColor(cell->text, sfGreen); break;
		case 3: sfText_setColor(cell->text, sfRed); break;
		case 4: sfText_setColor(cell->text, sfColor_fromRGB(0, 0, 128)); break; // Dark blue
		case 5: sfText_setColor(cell->text, sfColor_fromRGB(128, 0, 0)); break; // Dark red
		case 6: sfText_setColor(cell->text, sfColor_fromRGB(0, 128, 128)); break; // Teal
		case 7: sfText_setColor(cell->text, sfBlack); break;
		case 8: sfText_setColor(cell->text, sfColor_fromRGB(64, 64, 64)); break; // Gray
		default: sfText_setColor(cell->text, sfBlack); break;
		}
	}
	else {
		// Pour les cellules vides, s'assurer qu'il n'y a pas de texte
		sfText_setString(cell->text, "");
	}

	// If the cell is completely empty (explosiveNeighbor == 0), start the "flood fill" (reveal neighbors) algorithm
	if (cell->explosiveNeighbor == 0) {
		// Révéler récursivement les 8 cellules voisines
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int newX = cellGridPos.x + dx;
				int newY = cellGridPos.y + dy;

				// Vérifier les limites de la grille
				if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE) {
					// Ne pas rappeler la cellule actuelle
					if (!(dx == 0 && dy == 0)) {
						Cell* neighbor = grid->cells[newX][newY];
						// Ne révéler que les cellules non découvertes et non drapeautées
						if (!neighbor->bDiscovered && !neighbor->bFlagged) {
							CellReveal(grid, (sfVector2i) { newX, newY });
						}
					}
				}
			}
		}
	}

	// Increase grid discovered cell count
	grid->discoveredCellCount++;

	// If all none planted cells are discovered, terminate the game (return SUCCESS)
	// Pour cela, vous aurez besoin de connaître le nombre total de bombes
	// Vous devrez peut-être modifier GridCreate() ou GridPlantBomb() pour stocker bombCount dans la structure Grid
	int totalCells = GRID_SIZE * GRID_SIZE;
	int bombCount = 0;

	// Compter les bombes (à faire une fois au début et stocker dans la structure Grid)
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			if (grid->cells[i][j]->bPlanted) {
				bombCount++;
			}
		}
	}

	// Vérifier si toutes les cellules non-minées sont découvertes
	if (grid->discoveredCellCount >= (totalCells - bombCount)) {
		return SUCCESS;
	}

	// Return 0 as the cell was revealed and was not a bomb
	return 0;
}
// If the cell is already discovered or flagged, do nothing and return 0
// ...

// If the cell is planted with a bomb, return FAILURE to indicate game over
// ...


// Change the cell's appearance to revealed (lighter color) and mark it as discovered
// ...

// If the cell has explosive neighbors, display the number
// ...

// If the cell is completely empty (explosiveNeighbor == 0), start the "flood fill" (reveal neighbors) algorithm
// ...


// Increase grid discovered cell count and If all none planted cells are discovered, terminate the game (return SUCCESS)
// ...

// Return 0 as the cell was revealed and was not a bomb



void CellFlag(Grid* grid, sfVector2i cellGridPos)
{
	Cell* cell = grid->cells[cellGridPos.x][cellGridPos.y];

	// If the cell is already discovered, do nothing and return
	// ...

	// Toggle the flagged state of the cell and update its appearance accordingly
	// ...

	if (cell->bDiscovered) return;

	cell->bFlagged = !cell->bFlagged;

	if (cell->bFlagged) {
		// Couleur rouge clair bien visible
		sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(255, 100, 100));
	}
	else {
		// Retour à la couleur normale non découverte
		sfRectangleShape_setFillColor(cell->shape, sfColor_fromRGB(120, 120, 120));
	}
}


void CellDestroy(Cell* cell)
{
	// Free all resources associated with the cell
	// ...

	if (cell->shape != NULL) {
		sfRectangleShape_destroy(cell->shape);
		cell->shape = NULL;
	}

	// Destroy the SFML text
	if (cell->text != NULL) {
		sfText_destroy(cell->text);
		cell->text = NULL;
	}

	// Free the cell structure itself
	free(cell);
}


Grid* GridCreate()
{
	// Initialize grid properties, create all cell and register them in grid array
	// ...



	Grid* newGrid = (Grid*)malloc(sizeof(Grid));
	if (!newGrid) return NULL;

	newGrid->discoveredCellCount = 0;

	for (int x = 0; x < GRID_SIZE; x++) {
		for (int y = 0; y < GRID_SIZE; y++) {
			sfVector2f cellSize = { 30.0f, 30.0f };
			sfVector2f cellPos = { x * 30.0f, y * 30.0f };
			sfColor cellColor = sfColor_fromRGB(120, 120, 120); // Plus foncé

			newGrid->cells[x][y] = CellCreate(cellSize, cellPos, cellColor);

			// ASSOCIER LA POLICE AU TEXTE
			if (globalFont != NULL) {
				sfText_setFont(newGrid->cells[x][y]->text, globalFont);
			}
		}
	}

	return newGrid;
}

void GridPlantBomb(Grid* grid, int bombCount, sfVector2i cellToAvoid)
{
	// Plant all bomb and avoid avoided spot
	// Update explosiveNeighbor count for all cells around each bomb planted
	// ...
	int bombsPlanted = 0;
	int attempts = 0;
	int maxAttempts = GRID_SIZE * GRID_SIZE * 2; // Safety limit

	while (bombsPlanted < bombCount && attempts < maxAttempts) {
		int randomX = rand() % GRID_SIZE;
		int randomY = rand() % GRID_SIZE;

		if ((randomX != cellToAvoid.x || randomY != cellToAvoid.y) &&
			!grid->cells[randomX][randomY]->bPlanted) {

			grid->cells[randomX][randomY]->bPlanted = true;
			bombsPlanted++;

			// Update neighbors
			for (int dx = -1; dx <= 1; dx++) {
				for (int dy = -1; dy <= 1; dy++) {
					int neighborX = randomX + dx;
					int neighborY = randomY + dy;

					if (neighborX >= 0 && neighborX < GRID_SIZE &&
						neighborY >= 0 && neighborY < GRID_SIZE) {
						grid->cells[neighborX][neighborY]->explosiveNeighbor++;
					}
				}
			}
		}
		attempts++;
	}

	// If we couldn't plant all bombs (shouldn't happen with reasonable bombCount)
	if (bombsPlanted < bombCount) {
		printf("Warning: Only planted %d out of %d bombs\n", bombsPlanted, bombCount);
	}
}


sfVector2i GridUpdateLoop(Grid* grid, sfRenderWindow* window)
{
	// Get mouse position relative to the window
	sfVector2i mousePos = sfMouse_getPositionRenderWindow(window);

	// Initialize hovered cell coordinates to (-1, -1) (no cell hovered)
	sfVector2i cellCoord = { -1, -1 };

	// Search for hovered cell (if any)
	// Return cell coordinates or (-1, -1) if no cell is hovered
	// Use global bounds and contains function from SFML to detect if mouse is over a cell
	// ...

	return cellCoord;
}

void GridDraw(Grid* grid, sfRenderWindow* window)
{
	// Draw all cells in the grid
	// ...
	{
		// Loop through all cells of the grid and call CellDraw to display them
		for (int x = 0; x < GRID_SIZE; x++) {
			for (int y = 0; y < GRID_SIZE; y++) {
				CellDraw(grid->cells[x][y], window);
			}
		}
	}
}

void GridDestroy(Grid* grid)
{
	// Free all resources associated with the grid and its cells
	// ...
	for (int x = 0; x < GRID_SIZE; x++) {
		for (int y = 0; y < GRID_SIZE; y++) {
			if (grid->cells[x][y] != NULL) {
				CellDestroy(grid->cells[x][y]);
				grid->cells[x][y] = NULL;
			}
		}
	}
	free(grid);

	// Libérer la police quand la dernière grille est détruite
	if (globalFont != NULL) {
		sfFont_destroy(globalFont);
		globalFont = NULL;
	}
}
