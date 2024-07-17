#include "RunnerLevel.h"

int getRandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(gen);
}

RunnerLevel::RunnerLevel(sf::RenderWindow* hwnd, Input* in, GameState* gs, AudioManager* aud, TextureManager* tm)
{
	window = hwnd;
	input = in;
	gameState = gs;
	audio = aud;
	textMan = tm;
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	audio->addSound("sfx/bump.wav", "bump"); //bump 7 on pixabay

	// setup BGs as ten images next to each other. base dimensions 1024x1024
	float bgScalar = hwnd->getSize().y / 1024.0f;
	for (int i = 0; i < 21; ++i)
	{
		GameObject bg;
		bg.setTexture(&textMan->getTexture("bg_Scroll"));
		bg.setSize(sf::Vector2f(1024.0f*bgScalar , 1024.0f*bgScalar));
		bg.setPosition(i * 1024 * bgScalar, 0);
		BGs.push_back(bg);
	}

	distance = 19 * BGs.back().getSize().x - 30;
	finishLine.setSize(sf::Vector2f(10, window->getSize().y * 0.2));
	finishLine.setPosition(distance, window->getSize().y * 0.5);
	finishLine.setFillColor(sf::Color::Yellow);

	float placementIndex = 0.f;
	while (placementIndex < distance)
	{
		// go forward a random distance:
		placementIndex += getRandomInt(650,800);
		// harder in the back half.
		if (placementIndex > distance / 2) placementIndex -= getRandomInt(100, 250);
		if (placementIndex > distance) break;
		objects += 1;
		GameObject newObj;
		newObj.setPosition(placementIndex, window->getSize().y * 0.6);
		newObj.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.1));
		// randomly put a jumpable, kickable or two-jumpables next to each other
		int seed = getRandomInt(0, 3);
		switch (seed)
		{
		case 0:
			// kickable
			newObj.setTexture(&textMan->getTexture("kickable"));
			newObj.setPosition(placementIndex, window->getSize().y * 0.4);
			newObj.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.35));
			kickables.push_back(newObj);
			break;
		case 1:
			// jumpable
			newObj.setTexture(&textMan->getTexture("jumpable"));
			jumpables.push_back(newObj);
			break;
		case 3:
			// stompable
			newObj.setTexture(&textMan->getTexture("kickable"));
			newObj.setPosition(placementIndex, window->getSize().y * 0.6);
			newObj.setSize(sf::Vector2f(window->getSize().y * 0.5, window->getSize().y * 0.1));
			stompables.push_back(newObj);
			break;
		case 2:
			newObj.setTexture(&textMan->getTexture("jumpable"));
			jumpables.push_back(newObj);
			GameObject secondObj = newObj;
			secondObj.setPosition(secondObj.getPosition().x + secondObj.getSize().x, secondObj.getPosition().y);
			jumpables.push_back(secondObj);
			break;
		
		}
	}
	
	// setup Player
	p.setPosition(300, window->getSize().y * 0.6);
	p.updateGroundHeight();
	speed = 0.f;

	progressP.setSize(sf::Vector2f(window->getSize().y * 0.04, window->getSize().y * 0.04));
	progressP.setPosition(0.25 * window->getSize().x, 0.03 * window->getSize().y);
	progressLine.setSize(sf::Vector2f(window->getSize().x * 0.5, window->getSize().y * 0.01));
	progressLine.setFillColor(sf::Color::White);
	progressLine.setPosition(0.25 * window->getSize().x, 0.05 * window->getSize().y);
	destinationPoint.setRadius(10);
	destinationPoint.setPosition(0.75 * window->getSize().x + 5, 0.05 * window->getSize().y - 5);
	destinationPoint.setFillColor(sf::Color::Green);

	moon.setPosition(0.9 * window->getSize().x, 0);
	moon.setSize(sf::Vector2f(0.1 * window->getSize().x, 0.1 * window->getSize().x));
	moon.setTexture(&textMan->getTexture("lecturer"));
	moon.setTextureRect(sf::IntRect(0, 0, 24, 24));

	p.setDamaged(0.5);

}

RunnerLevel::~RunnerLevel()
{
}

bool Player::isStomping() {
	return (currentAnimation == &stomp);
}

void RunnerLevel::handleInput(float dt)
{
	
	if (input->isKeyDown(sf::Keyboard::Space) && p.canJump())
	{
		p.setJumping(window->getSize().y*0.25, 0.85);
	}
	if (input->isKeyDown(sf::Keyboard::Enter) && !p.canJump() && !p.isStomping())
	{
		p.setStomping();
	}
	else if (input->isKeyDown(sf::Keyboard::Enter) && !p.isKicking() && !p.isStomping())
	{
		p.setKicking(0.5);
	}
	
}

bool RunnerLevel::colliding(GameObject obj)
{
	if (obj.getGlobalBounds().intersects(p.getGlobalBounds()))
	{
		return true;
	}

	return false;
}

void RunnerLevel::update(float dt)
{
	// race over;
	if (travelled >= distance)
	{
		gameState->addResult("l2deaths", hits);
		gameState->addResult("l2time", time);
		if (gameState->getSingleRun())
		{
			gameState->setCurrentState(State::ENDGAME);
		}
		else
		{
			gameState->setCurrentState(State::PRE_THREE);
		}
		return;
	}

	time += dt;

	//plays thump the frame the player lands from a stomp
	bool stomp = p.isStomping();
	p.update(dt);
	if (stomp != p.isStomping() && !p.isDamaged()) { audio->playSoundbyName("bump"); }

	progressP.update(dt);
	float progress = (travelled / distance) * window->getSize().x * 0.5;
	progressP.setPosition(0.25 * window->getSize().x + progress, 0.03 * window->getSize().y);
	travelled += dt * speed;

	// scroll stuff
	for (GameObject& bg : BGs)
	{
		bg.move(-dt * speed, 0);
	}
	for (GameObject& j : jumpables)
	{
		j.move(-dt * speed, 0);
	}
	for (GameObject& k : kickables)
	{
		k.move(-dt * speed, 0);
	}
	for (GameObject& s : stompables)
	{
		s.move(-dt * speed, 0);
	}
	for (GameObject& e : explosions)
	{
		e.move(-dt * speed, 0);
	}
	finishLine.move(- dt * speed, 0);

	// speed increases to max. Changes animation speed.
	if (speed < MAX_SPEED)
		speed += dt * ACCELERATION;
	if (!p.isKicking() && !p.isStomping())
		p.currentAnimation->setFrameSpeed(25 / speed);

	// explosions
	for (int i = 0; i < explosions.size(); ++i)
	{
		explosionTimer[i] += dt;
		if (explosionTimer[i] > 0.05 && explosionTimer[i] < 0.1)
		{
			explosions[i].setTexture(&textMan->getTexture("explosion2"));
		}
		if (explosionTimer[i] > 0.1 && explosionTimer[i] < 0.15)
		{
			explosions[i].setTexture(&textMan->getTexture("explosion3"));
		}
		if (explosionTimer[i] > 0.15 && explosionTimer[i] < 0.2)
		{
			explosions[i].setTexture(&textMan->getTexture("explosion4"));
		}
		if (explosionTimer[i] > 0.2 && explosionTimer[i] < .25)
		{
			explosions[i].setTexture(&textMan->getTexture("explosion5"));
		}
		explosions[i].setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.1));
		if (explosionTimer[i] > 0.25)
		{
			explosions.erase(explosions.begin() + i);
			explosionTimer.erase(explosionTimer.begin() + i);

		}
	}

	// check for collisions
	if (p.isDamaged())
	{
		// if you're damaged you can keep going.
		return;
	}
	for (int i = 0; i < kickables.size(); ++i)// k : kickables)
	{
		GameObject k = kickables[i];
		if (colliding(k))
		{
			if (p.isKicking())
			{
				// explode the box
				kickables.erase(kickables.begin() + i);
				audio->playSoundbyName("success");
				GameObject explosion;
				explosion.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.1));
				explosion.setPosition(p.getPosition());
				explosion.setTexture(&textMan->getTexture("explosion1"));
				explosions.push_back(explosion);
				explosionTimer.push_back(0.f);
			} 
			else
			{
				hits++;
				p.setDamaged(1);
				speed = 0.f;
				// delete object so you don't collide with it again.
				kickables.erase(kickables.begin()+i);
				audio->playSoundbyName("death");
			}
		}
	}
	// check for collisions
	for (int i = 0; i < jumpables.size(); ++i)
	{
		GameObject j = jumpables[i];
		if (colliding(j))
		{
			hits++;
			p.setDamaged(1);
			speed = 0.f;
			jumpables.erase(jumpables.begin() + i);
			audio->playSoundbyName("death");
		}
	}

	for (int i = 0; i < stompables.size(); ++i)
	{
		GameObject k = stompables[i];
		if (colliding(k))
		{
			if (p.isStomping())
			{
				// explode the box
				stompables.erase(stompables.begin() + i);
				audio->playSoundbyName("success");
				GameObject explosion;
				explosion.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.1));
				explosion.setPosition(p.getPosition());
				explosion.setTexture(&textMan->getTexture("explosion1"));
				explosions.push_back(explosion);
				explosionTimer.push_back(0.f);
			}
			else
			{
				hits++;
				p.setDamaged(1);
				speed = 0.f;
				// delete object so you don't collide with it again.
				stompables.erase(stompables.begin() + i);
				audio->playSoundbyName("death");
			}
		}
	}
}

void RunnerLevel::render()
{
	beginDraw();
	for(GameObject bg : BGs) window->draw(bg);
	window->draw(moon);
	for (GameObject j : jumpables) window->draw(j);
	for (GameObject k : kickables) window->draw(k);
	for (GameObject s : stompables) window->draw(s);
	window->draw(finishLine);
	window->draw(p);
	window->draw(progressLine);
	window->draw(destinationPoint);
	window->draw(progressP);
	for (GameObject e : explosions) window->draw(e);
	window->draw(moon);
	endDraw();
}

void RunnerLevel::reset()
{
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	hits = 0;
	time = 0.f;
	objects = 0.f;
	travelled = 0.f;

	// setup Player
	p.setPosition(300, window->getSize().y * 0.6);
	speed = 0.f;

	BGs.clear();
	kickables.clear();
	jumpables.clear();
	stompables.clear();

	// setup BGs as ten images next to each other. base dimensions 1024x1024
	float bgScalar = window->getSize().y / 1024.0f;
	for (int i = 0; i < 21; ++i)
	{
		GameObject bg;
		bg.setTexture(&textMan->getTexture("bg_Scroll"));
		bg.setSize(sf::Vector2f(1024.0f * bgScalar, 1024.0f * bgScalar));
		bg.setPosition(i * 1024 * bgScalar, 0);
		BGs.push_back(bg);
	}

	float placementIndex = 0.f;
	while (placementIndex < distance)
	{
		// go forward a random distance:
		placementIndex += getRandomInt(650, 800);
		// harder in the back half.
		if (placementIndex > distance / 2) placementIndex -= getRandomInt(100, 250);
		if (placementIndex > distance) break;
		objects += 1;
		GameObject newObj;
		newObj.setPosition(placementIndex, window->getSize().y * 0.6);
		newObj.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.1));
		// randomly put a jumpable, kickable or two-jumpables next to each other
		int seed = getRandomInt(0, 3);
		switch (seed)
		{
		case 0:
			// kickable
			newObj.setTexture(&textMan->getTexture("kickable"));
			newObj.setPosition(placementIndex, window->getSize().y * 0.4);
			newObj.setSize(sf::Vector2f(window->getSize().y * 0.1, window->getSize().y * 0.35));
			kickables.push_back(newObj);
			break;
		case 1:
			// jumpable
			newObj.setTexture(&textMan->getTexture("jumpable"));
			jumpables.push_back(newObj);
			break;
		case 3:
			// stompable
			newObj.setTexture(&textMan->getTexture("kickable"));
			newObj.setPosition(placementIndex, window->getSize().y * 0.6);
			newObj.setSize(sf::Vector2f(window->getSize().y * 0.5, window->getSize().y * 0.1));
			stompables.push_back(newObj);
			break;
		case 2:
			newObj.setTexture(&textMan->getTexture("jumpable"));
			jumpables.push_back(newObj);
			GameObject secondObj = newObj;
			secondObj.setPosition(secondObj.getPosition().x + secondObj.getSize().x, secondObj.getPosition().y);
			jumpables.push_back(secondObj);
			break;

		}
	}
	finishLine.setPosition(distance, window->getSize().y * 0.5);
	p.setDamaged(0.5);
}