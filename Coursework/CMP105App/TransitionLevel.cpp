#include "TransitionLevel.h"

TransitionLevel::TransitionLevel(sf::RenderWindow* hwnd, Input* in, GameState* gs, AudioManager* aud, TextureManager* tm)
{
	// store parameters in local references.
	window = hwnd;
	input = in;
	textMan = tm;
	audio = aud;
	gameState = gs;

	// set the text object
	font.loadFromFile("font/montS.ttf");
	explain.setFont(font);
	explain.setPosition(sf::Vector2f(10, 10));
	explain.setCharacterSize(40);

	// initialise background. base size: 5760, 3240
	bg.setTexture(&textMan->getTexture("redSkyBG"));
	float bgScalar = std::max(hwnd->getSize().x / 5760.f, hwnd->getSize().y / 3240.f);
	bg.setSize(sf::Vector2f(5760 * bgScalar, 3240 * bgScalar));

}

TransitionLevel::~TransitionLevel()
{
}

void TransitionLevel::handleInput(float dt)
{
	if (input->isPressed(sf::Keyboard::Enter))
	{
		switch (gameState->getCurrentState())
		{
		case State::PRE_ONE:
			gameState->setCurrentState(State::LEVEL);
			break;
		case State::PRE_TWO:
			gameState->setCurrentState(State::RUNNER);
			break;
		case State::PRE_THREE:
			gameState->setCurrentState(State::WIZARD);
			break;
		case State::ENDGAME:
			gameState->setCurrentState(State::RESET);
			break;
		}
	}
}

void TransitionLevel::update(float dt)
{
	// set text object by state.
	switch (gameState->getCurrentState())
	{
	case State::PRE_ONE:
		explain.setString("Move with WASD.\nMove in time with the claps.\nAvoid tanks and pits.\nThey are metaphors for adversary.\nThere is a checkpoint halfway round.\nIt will be yellow when enabled.\nYou cannot pause.\nYou will have assistance.\nYou will be motivated.\nIt will not be helpful.\nHit Enter.");
		break;
	case State::PRE_TWO:
		explain.setString("Kick or Stomp (enter) wooden boxes.\nJump over (space) metal ones.\nYou got this.\nGood Luck.\nHit Enter");
		break;
	case State::PRE_THREE:
		explain.setString("Move with WASD.\nMove in time with the claps.\nAvoid Tanks and Pits.\nThere is a checkpoint.\nYou will be assisted.\nKind of.\nHit Enter.\nGood Luck.");
		break;
	case State::ENDGAME:
		runResults* results = gameState->getResults();
		std::string resultsString;
		if (results->L1Time > 0)
		{
			resultsString += "Level 1 deaths:\t\t\t" + std::to_string(results->L1Deaths) + "\n";
			resultsString += "Level 1 time:\t\t\t" + std::to_string(results->L1Time) + " seconds\n\n";
		}
		if (results->L2Time > 0)
		{
			resultsString += "Level 2 mistakes:\t\t\t" + std::to_string(results->L2Deaths) + "\n";
			resultsString += "Level 2 time:\t\t\t" + std::to_string(results->L2Time) + " seconds\n\n";
		}
		if (results->L3Time > 0)
		{
			resultsString += "Level 3 deaths:\t\t\t" + std::to_string(results->L3Deaths) + "\n";
			resultsString += "Level 3 time:\t\t\t" + std::to_string(results->L3Time) + " seconds\n";
		}
		explain.setString(resultsString + "\nPress Enter to continue.");
		break;
	}
}

void TransitionLevel::render()
{
	beginDraw();
	window->draw(bg);
	window->draw(explain);
	endDraw();
}
