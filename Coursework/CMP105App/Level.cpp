#include "Level.h"

Level::Level(sf::RenderWindow* hwnd, Input* in, GameState* gs, AudioManager* aud, TextureManager* tm)
{
	window = hwnd;
	input = in;
	gameState = gs;
	audio = aud;
	textMan = tm;

	// create the lecturer object.
	lecturer = Lecturer(window, textMan);

	// initialise game objects
	selectedAction = NONE;
	beatsPlayed = 0;

	// initialise background. base size: 5760, 3240
	levelBG.setTexture(&textMan->getTexture("redSkyBG"));
	float bgScalar = std::max(hwnd->getSize().x / 5760.f, hwnd->getSize().y / 3240.f);
	levelBG.setSize(sf::Vector2f(5760*bgScalar, 3240*bgScalar));

	// initialise grid board
	// .. first calculate size. 
	float windowWidth = window->getSize().x;
	float windowHeight = window->getSize().y;
	boardTop = windowHeight * 0.05;	// the board is as wide as can be with the given constants.
	boardLeft = boardTop;	// uniform top and side padding.
	boardRight = windowWidth - boardLeft;
	int numRows = 10;
	int numCols = 20;
	cellDim = (boardRight - boardLeft) / numCols;
	boardBottom = numRows * cellDim + boardTop;
	// ensure grid does not take up too much of the sapce
	float maxHeight = boardTop + windowHeight * GRID_Y_MAX_PROPORTION;
	while (boardBottom > maxHeight)
	{
		// reduce cell size by 2px, reducing cells by 1px from each edge anchored at top. gridTop remains constant.
		cellDim -= 2;
		boardLeft += numCols;
		boardRight -= numCols;
		boardBottom -= numRows * 2;
	}

	// .. Then setup board
	gridBoard.setFillColor(sf::Color::White);
	gridBoard.setSize(sf::Vector2f(boardRight - boardLeft, boardBottom - boardTop));
	gridBoard.setPosition(boardLeft, boardTop);

	// setup player component.
	playerPosition = { start.x, start.y };
	player.setPosition(sf::Vector2f(
		gridBoard.getPosition().x + cellDim * start.x,
		gridBoard.getPosition().y + cellDim * start.y)
	);
	player.setSize(sf::Vector2f(cellDim, cellDim));

	// Setup progress bar component.
	progressInStep.setPosition(sf::Vector2f(900, 800));
	progressInStep.setSize(sf::Vector2f(0, 200));
	progressInStepBG.setPosition(sf::Vector2f(900, 790));
	progressInStepBG.setSize(sf::Vector2f(500, 220));
	progressInStepBG.setFillColor(sf::Color::Black);
	targetZone.setPosition(sf::Vector2f(
		progressInStepBG.getPosition().x + progressInStepBG.getSize().x * (TIME_PER_STEP - TIME_FOR_ACTION - TIME_BUFFER) / TIME_PER_STEP,
		790
	));
	targetZone.setSize(sf::Vector2f(
		progressInStepBG.getSize().x * (TIME_FOR_ACTION / TIME_PER_STEP),
		220
	));
	targetZone.setFillColor(sf::Color::Blue);

	// setup controls component.
	font.loadFromFile("font/montS.ttf");
	controls.push_back(sf::Text());
	controls[0].setPosition(boardLeft, boardBottom);
	controls[0].setCharacterSize(30);
	controls[0].setFont(font);
	controlBG.setPosition(boardLeft, boardBottom);
	controlBG.setSize(sf::Vector2f(380, 280));
	controlBG.setFillColor(sf::Color::Red);

	// setup grid component.
	grid = StageGrid(
		sf::Vector2i(numCols, numRows), 
		cellDim, 
		gridBoard.getPosition(), 
		start, 
		end, 
		checkPoint,
		1,	
		textMan
	);

	// setup indicators
	std::vector<std::string> indicatorNames = { "upIcon", "rightIcon", "downIcon", "leftIcon" };
	std::vector<char> indicatorLabels = { 'W', 'D', 'S', 'A'};
	for (int i = 0; i < indicatorNames.size(); ++i)
	{
		GameObject icon;
		icon.setTexture(&textMan->getTexture(indicatorNames[i]));
		icon.setSize(sf::Vector2f(80, 80));
		icon.setPosition(boardLeft + i*100, boardBottom + 100);
		indicators.push_back(icon);
		sf::Text iconText;
		iconText.setFont(font);
		iconText.setString(indicatorLabels[i]);
		iconText.setPosition(boardLeft + i * 100 + 28, boardBottom + 200);
		controls.push_back(iconText);
	}

	// set state.
	checkPointEnabled = false;

	// set alert
	alert.setFont(font);
	alert.setPosition(window->getSize().x * 0.4, boardBottom);
	alert.setCharacterSize(50);
	alert.setFillColor(sf::Color::Yellow);

}

Level::~Level()
{

}

// handle user input
void Level::handleInput(float dt)
{
	if (damagedTimer > 0)
	{
		damagedTimer -= dt;
		return;
	}
	damagedTimer = 0.f;

	// check input has come at suitable point in progress.
	float timeLeft = TIME_PER_STEP - timeInStep;
	if (timeLeft > TIME_BUFFER && timeLeft < TIME_BUFFER + TIME_FOR_ACTION && selectedAction != FAIL)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			selectedAction = LEFT;
			player.setFlipped(true);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			selectedAction = UP;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			selectedAction = RIGHT;
			player.setFlipped(false);

		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			selectedAction = DOWN;
		}
		alert.setString("");
	}
	else 
	{
		// Timing incorrect, any attempted input is a fail.
		if (selectedAction == NONE)	misses++;	// don't increment misses more than once / step
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			selectedAction = FAIL;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			selectedAction = FAIL;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			selectedAction = FAIL;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			selectedAction = FAIL;
		}
		
	}
}

// Update game objects
void Level::update(float dt)
{
	// display alert or reset alert.
	if (alert.getString() != "" && alertHasBeenActiveFor < TIME_TO_DISPLAY_ALERT)
	{
		alertHasBeenActiveFor += dt;
	}
	else
	{
		alert.setString("");
		alertHasBeenActiveFor = TIME_TO_DISPLAY_ALERT;
	}

	// check for win
	if (playerPosition.first == end.x && playerPosition.second == end.y)
	{
		gameState->addResult("l1deaths", deaths);
		gameState->addResult("l1time", timeTaken);
		if (gameState->getSingleRun()) gameState->setCurrentState(State::ENDGAME);
		else gameState->setCurrentState(State::PRE_TWO);
		audio->playSoundbyName("success");
	}
	else
	{
		timeTaken += dt;
	}

	// check for checkpoint
	if (!checkPointEnabled && playerPosition.second > checkPoint.y)
	{
		checkPointEnabled = true;
		audio->playSoundbyName("success");
		alert.setString("checkpoint");
		alertHasBeenActiveFor = 0.f;
	}

	// update references. NOTE: gridboard only updated sometimes (enemies move each step, not each frame).
	player.update(dt);
	lecturer.update(dt);

	// Play beat ONCE per step.
	if (!soundPlayed && timeInStep > TIME_PER_STEP - (TIME_FOR_ACTION / 2 + TIME_BUFFER))
	{
		audio->playSoundbyName("clap");
		soundPlayed = true;
	}
	
	// update UI
	for(int i = 0; i < 4; ++i) indicators[i].setFillColor(sf::Color::White);
	switch (selectedAction)
	{
	case UP:
		controls[0].setString("up");
		indicators[0].setFillColor(sf::Color::Yellow);
		break;
	case RIGHT:
		controls[0].setString("right");
		indicators[1].setFillColor(sf::Color::Yellow);
		break;
	case DOWN:
		controls[0].setString("down");
		indicators[2].setFillColor(sf::Color::Yellow);
		break;
	case LEFT:
		controls[0].setString("left");
		indicators[3].setFillColor(sf::Color::Yellow);
		break;
	case FAIL:
		controls[0].setString("FAIL");
		break;
	case NONE:
		controls[0].setString("none");
		break;
	}

	// update progress component
	timeInStep += dt;
	progressInStep.setSize(sf::Vector2f(
		(timeInStep / TIME_PER_STEP) * progressInStepBG.getSize().x,
		200
	));
	float timeLeft = TIME_PER_STEP - timeInStep;
	if (timeLeft > TIME_BUFFER && timeLeft < TIME_BUFFER + TIME_FOR_ACTION && selectedAction != FAIL)
	{
		progressInStep.setFillColor(sf::Color::Green);
	}
	else
	{
		progressInStep.setFillColor(sf::Color::Red);
	}

	// movement
	if (timeInStep >= TIME_PER_STEP)
	{
		// do a move.
		beatsPlayed++;
		grid.update(0, dt);
		timeInStep = 0.f;
		soundPlayed = false;
		switch (selectedAction)
		{
		case UP:
			if (playerPosition.second == 0)
			{
				resetPlayer();
				break;
			}
			playerPosition.second--;	// positive-y innit.
			break;
		case RIGHT:
			if (playerPosition.first == boardDimensions.x - 1)
			{
				resetPlayer();
				break;
			}
			playerPosition.first++;
			player.setFlipped(false);
			break;
		case DOWN:
			if (playerPosition.second == boardDimensions.y - 1)
			{
				resetPlayer();
				break;
			}
			playerPosition.second++;
			break;
		case LEFT:
			if (playerPosition.first == 0)
			{
				resetPlayer();
				break;
			}
			playerPosition.first--;
			break;
		}
		player.setPosition(sf::Vector2f(
			gridBoard.getPosition().x + cellDim * playerPosition.first,
			gridBoard.getPosition().y + cellDim * playerPosition.second)
		);
		if (grid.playerHit(playerPosition))
		{
			resetPlayer();
		}
		selectedAction = NONE;

	}
	// update alert for early/late if needed.
	if (alert.getString() == "" && selectedAction == FAIL)
	{
		alertHasBeenActiveFor = 0.f;
		if (timeLeft < TIME_BUFFER)
		{
			// hack: stop alert carrying over in early frames of new step.
			if (alert.getString() != "too fast")
				alert.setString("too slow");
		}
		else
		{
			alert.setString("too fast");
		}
	}
}

// Render level
void Level::render()
{
	beginDraw();
	window->draw(levelBG);
	grid.render(window, checkPointEnabled);
	window->draw(controls[0]);
	window->draw(player);
	window->draw(progressInStepBG);
	window->draw(targetZone);
	window->draw(progressInStep);
	window->draw(lecturer);
	if(!lecturer.getMessageToDisplay(boardTop, boardRight, boardBottom, boardLeft).getString().isEmpty())
		window->draw(lecturer.getMessageToDisplay(boardTop, boardRight, boardBottom, boardLeft));
	window->draw(controlBG);
	for (GameObject ind : indicators)
	{
		window->draw(ind);
	}
	for (sf::Text label : controls)
	{
		window->draw(label);
	}
	window->draw(alert);
	endDraw();
}

/*
Put player back to the starting space. 
*/
void Level::resetPlayer()
{
	if(checkPointEnabled) playerPosition = { checkPoint.x, checkPoint.y };
	else playerPosition = { start.x, start.y };
	audio->playSoundbyName("death");
	damagedTimer = RESET_TIME;
	player.setDamaged(damagedTimer);
	deaths++;
}

void Level::reset()
{
	// create the lecturer object.
	lecturer = Lecturer(window, textMan);

	// initialise game objects
	selectedAction = NONE;
	beatsPlayed = 0;
	timeTaken = 0.f;
	misses = 0;
	deaths = 0;

	// initialise background. base size: 5760, 3240
	levelBG.setTexture(&textMan->getTexture("redSkyBG"));
	float bgScalar = std::max(window->getSize().x / 5760.f, window->getSize().y / 3240.f);
	levelBG.setSize(sf::Vector2f(5760 * bgScalar, 3240 * bgScalar));

	// initialise grid board
	// .. first calculate size. 
	float windowWidth = window->getSize().x;
	float windowHeight = window->getSize().y;
	boardTop = windowHeight * 0.05;	// the board is as wide as can be with the given constants.
	boardLeft = boardTop;	// uniform top and side padding.
	boardRight = windowWidth - boardLeft;
	int numRows = 10;
	int numCols = 20;
	cellDim = (boardRight - boardLeft) / numCols;
	boardBottom = numRows * cellDim + boardTop;
	// ensure grid does not take up too much of the sapce
	float maxHeight = boardTop + windowHeight * GRID_Y_MAX_PROPORTION;
	while (boardBottom > maxHeight)
	{
		// reduce cell size by 2px, reducing cells by 1px from each edge anchored at top. gridTop remains constant.
		cellDim -= 2;
		boardLeft += numCols;
		boardRight -= numCols;
		boardBottom -= numRows * 2;
	}

	start = { 0,3 };
	end = { 0, 8 };
	checkPoint = { 19, 5 };

	playerPosition = { start.x, start.y };

	// .. Then setup board
	gridBoard.setFillColor(sf::Color::White);
	gridBoard.setSize(sf::Vector2f(boardRight - boardLeft, boardBottom - boardTop));
	gridBoard.setPosition(boardLeft, boardTop);

	// setup player component.
	player.setPosition(sf::Vector2f(
		gridBoard.getPosition().x + cellDim * start.x,
		gridBoard.getPosition().y + cellDim * start.y)
	);
	player.setSize(sf::Vector2f(cellDim, cellDim));

	// Setup progress bar component.
	progressInStep.setPosition(sf::Vector2f(900, 800));
	progressInStep.setSize(sf::Vector2f(0, 200));
	progressInStepBG.setPosition(sf::Vector2f(900, 790));
	progressInStepBG.setSize(sf::Vector2f(500, 220));
	progressInStepBG.setFillColor(sf::Color::Black);
	targetZone.setPosition(sf::Vector2f(
		progressInStepBG.getPosition().x + progressInStepBG.getSize().x * (TIME_PER_STEP - TIME_FOR_ACTION - TIME_BUFFER) / TIME_PER_STEP,
		790
	));
	targetZone.setSize(sf::Vector2f(
		progressInStepBG.getSize().x * (TIME_FOR_ACTION / TIME_PER_STEP),
		220
	));
	targetZone.setFillColor(sf::Color::Blue);

	// setup grid component.
	grid = StageGrid(
		sf::Vector2i(numCols, numRows),
		cellDim,
		gridBoard.getPosition(),
		start,
		end,
		checkPoint,
		1,
		textMan
	);

	// set state.
	checkPointEnabled = false;

	// set alert
	alert.setFont(font);
	alert.setPosition(window->getSize().x * 0.4, boardBottom);
	alert.setCharacterSize(50);
	alert.setFillColor(sf::Color::Yellow);
}
