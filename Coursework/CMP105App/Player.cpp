#include "Player.h"
#include <iostream>

Player::Player()
{
	setPosition(sf::Vector2f(100, 100));
	setSize(sf::Vector2f(100, 100));

	dino.loadFromFile("gfx/dinoVita.png");

	setTexture(&dino); // set that texture to this object.

	// dino texture is 24x24px. Frames for walk are 4 - 9, inclusive.
	for (int i = 4; i < 10; ++i)
	{
		walk.addFrame(sf::IntRect(i * 24.f, 0.f, 24.f, 24.f));
	}
	for (int i = 0; i < 4; ++i)
	{
		kick.addFrame(sf::IntRect(i * 24.f + 240.f, 0.f, 24.f, 24.f));
	}
	for (int i = 0; i < 3; i++) {
		stomp.addFrame(sf::IntRect(i * 24.f + 576.f, 0.f, 24.f, 24.f));
	}
	squish.addFrame(sf::IntRect(648.f, 0.f, 24.f, 24.f));
	damaged.addFrame(sf::IntRect(0, 0, 24, 24));
	damaged.addFrame(sf::IntRect(15 * 24, 0, 24, 24));
	damaged.setFrameSpeed(1.f / 4.f);
	currentAnimation = &walk;
	stomp.setLooping(false);
	stomp.setFrameSpeed(1.f / 20.f);
	walk.setFrameSpeed(1.f / 10.f);
	//kick.setLooping(false);

	velocity = { 0,0 };
}

Player::~Player()
{
}

void Player::updateGroundHeight() {
	groundHeight = getPosition().y;
}

void Player::setJumping(float jumpH, float jumpLength)
{
	jumpTime = jumpLength;
	jumpTimeElapsed = 0;
	jumpHeight = jumpH;
}

bool Player::isKicking()
{
	return kickTimeElapsed > 0 && kickTimeElapsed < kickTime;
}

void Player::setKicking(float t)
{
	kick.setFrameSpeed(t / kick.getSize());
	kick.reset();
	currentAnimation = &kick;
	kickTime = t;
	kickTimeElapsed = 0.f;
}

bool Player::canJump() const
{
	return jumpTime == 0 && currentAnimation != &stomp;
}

void Player::setFlipped(bool f)
{
	currentAnimation->setFlipped(f);
}

void Player::handleInput(float dt)
{

}

void Player::setStomping() {
	currentAnimation = &stomp;
	jumpHeight = 0;
	jumpTimeElapsed = 0.f;
	jumpTime = 0.f;
	stomp.reset();
}

bool Player::isDamaged()
{
	return currentAnimation == &damaged;
}

void Player::setDamaged(float timer)
{
	currentAnimation = &damaged;
	damageLength = timer;
}

void Player::update(float dt)
{
	if (currentAnimation == &damaged)
		damagedTimer += dt;

	if (damagedTimer > damageLength)
	{
		currentAnimation = &walk;
		damagedTimer = 0.f;
	}

	if (currentAnimation == &stomp) {
		jumpTime = 0.f;
		if (getPosition().y < groundHeight) {
			move(0, 3.f);
		}
		else
		{
			currentAnimation = &squish;
			kickTime = 0.1f;
			kickTimeElapsed = 0;
		}
	}

	// check for jump
	if (jumpTimeElapsed < jumpTime)
	{

		jumpTimeElapsed += dt;
		// go up.
		if (jumpTimeElapsed < jumpTime * 0.2)
		{
			move(0, -jumpHeight * dt / (jumpTime * 0.2));
		}
		// hover for a bit
		// go down.
		if (jumpTimeElapsed > jumpTime * 0.7)
		{
			move(0, jumpHeight * dt / (jumpTime * 0.3));
		}

	}
	else
	{
		jumpHeight = 0;
		jumpTimeElapsed = 0.f;
		jumpTime = 0.f;

		if ((currentAnimation == &walk || currentAnimation == &kick) && getPosition().y < groundHeight) {
			move(0, 1.f);
		}
	}

	// check for kick status.
	if (kickTime != 0)
	{
		if (kickTimeElapsed < kickTime) kickTimeElapsed += dt;
		else
		{
			currentAnimation = &walk;
			kickTime = 0.f;
			kickTimeElapsed = 0.f;
		}
	}

	currentAnimation->animate(dt);
	setTextureRect(currentAnimation->getCurrentFrame());

}