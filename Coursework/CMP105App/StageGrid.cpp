#include "StageGrid.h"

// helper method, min and max are both inclusive (eg gRI(0,10) could return [0, ..., 10]
int getRandInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(gen);
}

sf::Vector2i getRandGridPos(sf::Vector2i dimensions)
{
	return sf::Vector2i(getRandInt(0, dimensions.x - 1), getRandInt(0, dimensions.y - 1));
}

StageGrid::StageGrid()
{
}



// create grid, stage current [1-3] for difficulty, i.e, number of hazards..
StageGrid::StageGrid(sf::Vector2i dimensions, float cellSizeIn, sf::Vector2f positionIn, sf::Vector2i start, sf::Vector2i end, sf::Vector2i cp, int stage, TextureManager* tm)
{

	textMan = tm;
	cellSize = cellSizeIn;
	position = positionIn;


	// initialise grid to all safe and size the gameObjects for drawing.
	for (int i = 0; i < dimensions.x; ++i)
	{
		// for each column. 
		std::vector<cellState> col;
		std::vector<GameObject> gridCol;
		float colLeft = position.x + (i * cellSize);
		for (int j = 0; j < dimensions.y; ++j)
		{
			// for each cell in a column.
			col.push_back(cellState::SAFE);
			GameObject cell = GameObject();
			cell.setSize(sf::Vector2f(cellSize, cellSize));
			cell.setPosition(colLeft, position.y + (j * cellSize));
			gridCol.push_back(cell);
		}
		grid.push_back(col);
		board.push_back(gridCol);
	}
	grid[end.x][end.y] = cellState::END;
	grid[start.x][start.y] = cellState::START;
	grid[cp.x][cp.y] = cellState::CHECKPOINT;

	// Check if you are doing the motivation or confusion stage (1 or 2)
	if (stage == 1)
	{
		// MOTIVATION stage.
		// . add pits for central wall
		for (int i = 0; i < dimensions.x * 0.8; ++i)
		{
			grid[i][dimensions.y / 2] = cellState::PIT;
		}
		// . add tanks along the top
		for (int i = 0; 2 + i * 3 < dimensions.x; ++i)
		{
			grid[2 + i * 3][0] = cellState::HAZARD_DOWN;
		}
		// . add tanks along the middle
		for (int i = 0; 1 + i * 3 < dimensions.x * 0.8; ++i)
		{
			grid[1 + i * 3][dimensions.y / 2 + 1] = cellState::HAZARD_DOWN;

		}
	}
	if (stage == 2)
	{
		std::vector<std::pair<int, int>> maze;
		// draw top wall
		for (int i = 0; i < 7; ++i) maze.push_back({ i, 1 });
		// draw mid divider
		for (int i = 0; i < 4; ++i)
		{
			maze.push_back({ 9,i });
			maze.push_back({ 9, 9 - i });
			maze.push_back({ 8 - i, 6 + i });
		}
		// bottom of second corridor
		for (int i = 2; i < 9; ++i) maze.push_back({ i, 3 });

		for (std::pair<int, int> p : maze)
		{
			grid[p.first][p.second] = cellState::PIT;
		}

		// tanks for the first 'arena'
		grid[2][8] = HAZARD_UP;
		// grid[3][7] = HAZARD_UP; removed to make it easier.
		grid[4][6] = HAZARD_UP;

		// tanks for the second 'arena'
		grid[12][8] = HAZARD_UP;
		grid[13][5] = HAZARD_UP;
		//grid[18][2] = HAZARD_UP;		// removed to make it easier also.
		grid[12][2] = HAZARD_RIGHT;
		grid[13][4] = HAZARD_RIGHT;
		//grid[14][6] = HAZARD_RIGHT;	// and here, removed for balancing.
		grid[1][8] = HAZARD_RIGHT;

	}

	for (int i = 0; i < 4; ++i)
	{
		tile_turn.addFrame(sf::IntRect(i * 135.f, 0.f, 135.f, 135.f));
	}
	tile_turn.setFrameSpeed(1.f / 12.f);
}


// returns true if the player is colliding with a pit or hazard.
bool StageGrid::playerHit(std::pair<int, int> pos)
{
	if (grid[pos.first][pos.second] == cellState::PIT ||
		grid[pos.first][pos.second] == cellState::HAZARD_UP ||
		grid[pos.first][pos.second] == cellState::HAZARD_RIGHT ||
		grid[pos.first][pos.second] == cellState::HAZARD_DOWN ||
		grid[pos.first][pos.second] == cellState::HAZARD_LEFT)
		return true;
	return false;

}

// move hazards about.
void StageGrid::update(int frames, float dt)
{




	// create a temporary grid to store updated positions
	std::vector<std::vector<cellState>> updatedGrid = grid;
	// for every column.
	for (int x = 0; x < grid.size(); ++x)
	{
		// for every cell.
		for (int y = 0; y < grid[x].size(); y++)
		{
			// check if there are hazards that need to move.
			switch (grid[x][y])
			{
			case HAZARD_DOWN:
				// if tank can proceed, move it down.
				if (y + 1 < grid[x].size() && updatedGrid[x][y + 1] == SAFE)
				{
					updatedGrid[x][y + 1] = HAZARD_DOWN;
					updatedGrid[x][y] = SAFE;
				}
				// if it can't, turn it around.
				else
				{
					updatedGrid[x][y] = HAZARD_UP;
				}
				break;
			case HAZARD_UP:
				// if tank can proceed, move it down.
				if (y - 1 >= 0 && updatedGrid[x][y - 1] == SAFE)
				{
					updatedGrid[x][y - 1] = HAZARD_UP;
					updatedGrid[x][y] = SAFE;
				}
				// if it can't, turn it around.
				else
				{
					updatedGrid[x][y] = HAZARD_DOWN;
				}
				break;
			case HAZARD_LEFT:
				// if tank can proceed, move it down.
				if (x - 1 >= 0 && updatedGrid[x - 1][y] == SAFE)
				{
					updatedGrid[x - 1][y] = HAZARD_LEFT;
					updatedGrid[x][y] = SAFE;
				}
				// if it can't, turn it around.
				else
				{
					updatedGrid[x][y] = HAZARD_RIGHT;
				}
				break;
			case HAZARD_RIGHT:
				// if tank can proceed, move it down.
				if (x + 1 < grid.size() && updatedGrid[x + 1][y] == SAFE)
				{
					updatedGrid[x + 1][y] = HAZARD_RIGHT;
					updatedGrid[x][y] = SAFE;
				}
				// if it can't, turn it around.
				else
				{
					updatedGrid[x][y] = HAZARD_LEFT;
				}
				break;
			}
		}
	}

	// update the grid with the newly calculated positions
	grid = updatedGrid;
}


// draw the current grid state. Takes window to draw to
void StageGrid::render(sf::RenderWindow* wnd, bool cp_on)
{
	// Draw all safe tile.
	for (int x = 0; x < grid.size(); ++x)
	{

		for (int y = 0; y < grid[x].size(); ++y)
		{
			board[x][y].setTexture(&textMan->getTexture("safe_tile"));
			board[x][y].setSize(sf::Vector2f(cellSize, cellSize) * 1.5f);
			board[x][y].setPosition(position.x + (x * cellSize), position.y + (y * cellSize));
			rotate_tiles();
			wnd->draw(board[x][y]);
		}
	}

	for (int x = 0; x < grid.size(); ++x)
	{
		for (int y = 0; y < grid[x].size(); ++y)
		{
			if (grid[x][y] == cellState::SAFE) continue;

			GameObject cellOverlay;
			cellOverlay.setSize(sf::Vector2f(cellSize, cellSize));
			cellOverlay.setPosition(board[x][y].getPosition());

			switch (grid[x][y])
			{
			case cellState::PIT:
				cellOverlay.setTexture(&textMan->getTexture("pit"));
				break;
			case cellState::START:
				cellOverlay.setTexture(&textMan->getTexture("start"));
				break;
			case cellState::END:
				cellOverlay.setTexture(&textMan->getTexture("end"));
				break;
			case cellState::CHECKPOINT:
				if (cp_on) cellOverlay.setTexture(&textMan->getTexture("cp_on"));
				else cellOverlay.setTexture(&textMan->getTexture("cp_off"));
				break;
			case cellState::HAZARD_DOWN:
				cellOverlay.setTexture(&textMan->getTexture("tankDown"));
				break;
			case cellState::HAZARD_UP:
				cellOverlay.setTexture(&textMan->getTexture("tankUp"));
				break;
			case cellState::HAZARD_LEFT:
				cellOverlay.setTexture(&textMan->getTexture("tankLeft"));
				break;
			case cellState::HAZARD_RIGHT:
				cellOverlay.setTexture(&textMan->getTexture("tankRight"));
				break;
			}

			wnd->draw(cellOverlay);
		}
	}
}

void StageGrid::update_animation(float dt) {
	tile_turn.animate(dt);
	for (int x = 0; x < grid.size(); ++x)
	{

		for (int y = 0; y < grid[x].size(); ++y)
		{
			board[x][y].setTextureRect(tile_turn.getCurrentFrame());
		}
	}
}

void StageGrid::rotate_tiles() {
	sf::Vector2f centre_position = board[7][7].getPosition();
	sf::Vector2f new_position;

	int frame_number = tile_turn.getCurrentFrame().left / 135;
	float angle = 0.f;

	switch (frame_number) {
	case(0):
		angle = 0.f;
		break;
	case(1):
		angle = 0.174533f; //10 degrees
		break;
	case(2):
		angle = 0.785398f; //45 degrees
		break;
	case(3):
		angle = 1.39626f; //80 degrees
		break;
	}

	for (int x = 0; x < grid.size(); ++x)
	{
		for (int y = 0; y < grid[x].size(); ++y)
		{
			new_position = board[x][y].getPosition() - centre_position;
			new_position = sf::Vector2f(new_position.x * cos(angle) - new_position.y * sin(angle), (new_position.x * sin(angle) + new_position.y * cos(angle)) * 0.72f);

			new_position += centre_position;
			board[x][y].setPosition(new_position);
		}
	}
}

