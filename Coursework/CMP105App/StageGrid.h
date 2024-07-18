#pragma once

#include <SFML/Graphics.hpp>
#include "Random"
#include "TextureManager.h"
#include "Framework/GameObject.h"
#include "Framework/Animation.h"

class StageGrid
{
public:
	StageGrid();
	StageGrid(sf::Vector2i dimensions, float cellSize, sf::Vector2f position, sf::Vector2i start, sf::Vector2i end, sf::Vector2i cp, int stage, TextureManager* tm);

	enum cellState
	{
		SAFE,
		PIT,			// a hazard that does not move.
		HAZARD_UP,
		HAZARD_DOWN,
		HAZARD_LEFT,
		HAZARD_RIGHT,
		START,
		END,
		CHECKPOINT,
		METEOR,
		METEOR_TRAIL1,
		METEOR_TRAIL2,
		METEOR_TRAIL3
	};

	void update(int frames, float dt);
	void render(sf::RenderWindow* wnd, bool cp_on);
	void update_animation(float dt);
	bool playerHit(std::pair<int, int> pos);
	void rotate_tiles();
	void rotate();
	sf::Vector2f getTilePosition(int x, int y);
	int get_turns();

	void reset_meteor();
	void set_player_pos(std::pair<int, int>);
	
	

private:
	std::pair<int, int> meteor_pathfind(std::pair<int, int> position);
	TextureManager* textMan;
	std::vector<std::vector<cellState>> grid;
	std::vector<std::vector<GameObject>> board;
	float cellSize;
	sf::Vector2f position;

	bool stage1;

	float time_elapsed;
	std::pair<int, int> player_position;
	sf::Texture safe_tile;
	Animation tile_turn;
	int turns;
};


