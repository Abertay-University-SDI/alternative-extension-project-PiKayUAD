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

	player_position = std::pair<int, int>{ 0,0 };

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
		turns = 0;
	}
	if (stage == 2)
	{
		std::vector<std::pair<int, int>> maze;
		// draw top wall
		for (int i = 0; i < 5; ++i) maze.push_back({ i, 1 });
		for (int i = 0; i < 6; ++i) maze.push_back({ 14 - i, 5 });
		for (int i = 0; i < 6; ++i) maze.push_back({ 14 - i, 10 });
		for (int i = 0; i < 3; ++i) maze.push_back({ 9, 10+i });
		

		for (int i = 0; i < 7; i++) {
			maze.push_back({7, 4+i});
			maze.push_back({ 4 + i,7 });
		}

		// draw mid divider
		
		/*
		for (int i = 0; i < 4; ++i)
		{
			maze.push_back({ 9,i });
			maze.push_back({ 9, 9 - i });
			maze.push_back({ 8 - i, 6 + i });
		}
		*/

		/*
		for (int i = 0; i < 15; ++i)
		{
			maze.push_back({ 1,i });
			maze.push_back({ 13, i });
		}
		*/

		// bottom of second corridor
		for (int i = 2; i < 7; ++i) maze.push_back({ i, 4 });

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
		//grid[13][5] = HAZARD_UP;
		//grid[18][2] = HAZARD_UP;		// removed to make it easier also.
		grid[12][2] = HAZARD_RIGHT;
		grid[12][3] = HAZARD_LEFT;
		//grid[13][4] = HAZARD_RIGHT;
		//grid[14][6] = HAZARD_RIGHT;	// and here, removed for balancing.
		grid[1][8] = HAZARD_RIGHT;
		turns = 3;
		grid[13][13] = METEOR;
	}
	
	tile_turn.setLooping(false);
	tile_turn.setFrameSpeed(1.f / 12.f);
	for (int i = 0; i < 5; ++i)
	{
		tile_turn.addFrame(sf::IntRect(i * 135.f, 0.f, 135.f, 135.f));
		tile_turn.animate(1.f/3.f); //makes the animation start on the last frame due to how this prevents an offset
	}
	tile_turn.setLooping(false);
	
}
void StageGrid::reset_meteor() {
	for (int x = 0; x < grid.size(); ++x)
	{
		for (int y = 0; y < grid[x].size(); y++)
		{
			if (grid[x][y] == METEOR) grid[x][y] = SAFE;
		}
	}
	grid[13][13] = METEOR;
}

void StageGrid::set_player_pos(std::pair<int, int> pos) {
	player_position = pos;
}

// returns true if the player is colliding with a pit or hazard.
bool StageGrid::playerHit(std::pair<int, int> pos)
{
	if (grid[pos.first][pos.second] == cellState::PIT ||
		grid[pos.first][pos.second] == cellState::HAZARD_UP ||
		grid[pos.first][pos.second] == cellState::HAZARD_RIGHT ||
		grid[pos.first][pos.second] == cellState::HAZARD_DOWN ||
		grid[pos.first][pos.second] == cellState::HAZARD_LEFT ||
		grid[pos.first][pos.second] == cellState::METEOR ||
		grid[pos.first][pos.second] == cellState::METEOR_TRAIL1 ||
		grid[pos.first][pos.second] == cellState::METEOR_TRAIL2 || 
		grid[pos.first][pos.second] == cellState::METEOR_TRAIL3)
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
			case METEOR_TRAIL1:
				updatedGrid[x][y] = METEOR_TRAIL2;
				break;
			case METEOR_TRAIL2:
				updatedGrid[x][y] = METEOR_TRAIL3;
				break;
			case METEOR_TRAIL3:
				updatedGrid[x][y] = SAFE;
				break;
			case METEOR:
				std::pair<int, int> new_pos = meteor_pathfind(std::pair<int, int>{x, y});
				updatedGrid[x][y] = METEOR_TRAIL1;
				updatedGrid[new_pos.first][new_pos.second] = METEOR;
				break;
			}
		}
	}

	// update the grid with the newly calculated positions
	grid = updatedGrid;
}

std::pair<int, int> StageGrid::meteor_pathfind(std::pair<int,int> meteor_position) {
	
	enum cellSearchState
	{
		BLOCKED,
		UNCHECKED,
		UP,
		LEFT,
		DOWN,
		RIGHT,
	};

	std::vector<std::vector<cellSearchState>>new_grid;
	
	
	for (int x = 0; x < grid.size(); x++) {
		std::vector<cellSearchState> row;
		for (int y = 0; y < grid[0].size(); y++) {
			if (grid[x][y] == SAFE) { row.push_back(UNCHECKED); }
			else { row.push_back(BLOCKED); }
		}
		new_grid.push_back(row);
	}
	
	std::vector<std::pair<int, int>> unfinished_paths;
	//unfinished_paths.push_back(std::make_pair(1, 1));
	unfinished_paths.push_back(meteor_position);

	bool path_found = false;
	
	while (unfinished_paths.size() > 0 && !path_found) {
		if (unfinished_paths[0].first + 1 < new_grid.size()) {
			//std::cout << unfinished_paths[0].first + 1 << ", " << unfinished_paths[0].second << "\n";
			if (new_grid[unfinished_paths[0].first + 1][unfinished_paths[0].second] == UNCHECKED) {
				new_grid[unfinished_paths[0].first + 1][unfinished_paths[0].second] = RIGHT;
				unfinished_paths.push_back(std::make_pair(unfinished_paths[0].first + 1, unfinished_paths[0].second));
			}
		}
		
		if (unfinished_paths[0].first - 1 >= 0) {
			if (new_grid[unfinished_paths[0].first - 1][unfinished_paths[0].second] == UNCHECKED) {
				new_grid[unfinished_paths[0].first - 1][unfinished_paths[0].second] = LEFT;
				unfinished_paths.push_back(std::make_pair(unfinished_paths[0].first - 1, unfinished_paths[0].second));
			}
		}
		if (unfinished_paths[0].second + 1 < new_grid[0].size()) {
			if (new_grid[unfinished_paths[0].first][unfinished_paths[0].second + 1] == UNCHECKED) {
				new_grid[unfinished_paths[0].first][unfinished_paths[0].second +1] = UP;
				unfinished_paths.push_back(std::make_pair(unfinished_paths[0].first , unfinished_paths[0].second + 1));
			}
		}
		if (unfinished_paths[0].second - 1 >= 0) {
			if (new_grid[unfinished_paths[0].first][unfinished_paths[0].second - 1] == UNCHECKED) {
				new_grid[unfinished_paths[0].first][unfinished_paths[0].second - 1] = DOWN;
				unfinished_paths.push_back(std::make_pair(unfinished_paths[0].first , unfinished_paths[0].second - 1));
			}
		}
		
		if (unfinished_paths[0] == player_position) {
			path_found = true;
		}
		else {
			unfinished_paths.erase(unfinished_paths.begin());
		}
	}
	

	if (path_found) {
		std::pair<int, int> path_follower = player_position;
		std::pair<int, int> path_follower2 = path_follower;
		while(path_follower != meteor_position) {
			switch (new_grid[path_follower.first][path_follower.second])
			{
			case(RIGHT):
				path_follower.first--;
				break;
			case(LEFT):
				path_follower.first++;
				break;
			case(UP):
				path_follower.second--;
				break;
			case(DOWN):
				path_follower.second++;
				break;
			}

			if (path_follower != meteor_position) {
				path_follower2 = path_follower;
			}
		}
		meteor_position = path_follower2;
	}
	return meteor_position;
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
			board[x][y].setTextureRect(tile_turn.getCurrentFrame());
			board[x][y].setSize(sf::Vector2f(cellSize * 1.5f, cellSize * 1.5f)); // 
			board[x][y].setPosition(position.x + (x * cellSize), position.y + (y * cellSize));// );
			
			
		}
	}
	rotate_tiles();
	for (int x = 0; x < grid.size(); ++x)
	{

		for (int y = 0; y < grid[x].size(); ++y)
		{
			wnd->draw(board[x][y]);
		}
	}
	
	for (int x = 0; x < grid.size(); ++x)
	{
		//std:: cout << "overlay pos" << board[x][14].getPosition().y << std::endl;
		for (int y = 0; y < grid[x].size(); ++y)
		{
			if (grid[x][y] == cellState::SAFE) continue;

			GameObject cellOverlay;
			cellOverlay.setSize(sf::Vector2f(cellSize * 0.7f, cellSize * 0.7f)); // * 0.7f)
			cellOverlay.setPosition(board[x][y].getPosition() + sf::Vector2f(12.5f, 12.5f));

			int tank = 0;
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
			case METEOR:
				cellOverlay.setTexture(&textMan->getTexture("meteor"));
				break;
			case METEOR_TRAIL1:
				cellOverlay.setTexture(&textMan->getTexture("flame"));
				break;
			case METEOR_TRAIL2:
				cellOverlay.setTexture(&textMan->getTexture("flame"));
				cellOverlay.setSize(cellOverlay.getSize() * 0.8f);
				cellOverlay.setPosition(cellOverlay.getPosition() + sf::Vector2f(5.f, 5.f));
				break;
			case METEOR_TRAIL3:
				cellOverlay.setTexture(&textMan->getTexture("flame"));
				cellOverlay.setSize(cellOverlay.getSize() * 0.6f);
				cellOverlay.setPosition(cellOverlay.getPosition() + sf::Vector2f(10.f, 10.f));
				break;

			case cellState::HAZARD_DOWN:
				tank = 1;
				break;
			case cellState::HAZARD_LEFT:
				tank = 2;
				break;
			case cellState::HAZARD_UP:
				tank = 3;
				break;
			case cellState::HAZARD_RIGHT:
				tank = 4;
				break;

			}
			if (tank > 0) {
				//tank--;
				tank = (tank + turns) % 4;
				switch (tank)
				{
				case(0):
					cellOverlay.setTexture(&textMan->getTexture("tankDown"));
					break;
				case(1):
					cellOverlay.setTexture(&textMan->getTexture("tankLeft"));
					break;
				case(2):
					cellOverlay.setTexture(&textMan->getTexture("tankUp"));
					break;
				case(3):
					cellOverlay.setTexture(&textMan->getTexture("tankRight"));
					break;
				}
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
	case(4):
		angle = 1.5708; //90 degreees
		break;
	}
	angle += 1.5708 * turns;

	for (int i = 0; i < grid.size(); ++i)
	{
		
		for (int j = 0; j < grid[i].size(); ++j)
		{
			new_position = board[i][j].getPosition() - centre_position;
			new_position = sf::Vector2f(new_position.x * cos(angle) - new_position.y * sin(angle), (new_position.x * sin(angle) + new_position.y * cos(angle))); //rotation matrix
			new_position.y *= 0.72f;
			new_position += centre_position;

			
			
			board[i][j].setPosition(new_position);
		}
	}
}

sf::Vector2f StageGrid::getTilePosition(int x, int y) {
	return board[x][y].getPosition();
}

void StageGrid::rotate() {
	turns++;
	turns = turns % 4;
	tile_turn.reset();
	tile_turn.setPlaying(true);
}

int StageGrid::get_turns() { return turns; }