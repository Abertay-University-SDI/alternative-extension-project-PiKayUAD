#pragma once

#include "Framework/GameObject.h"
#include "Framework/Animation.h"

class Player : public GameObject
{
public:
	Player();
	~Player();

	void handleInput(float dt);
	void setDamaged(float timer);
	bool isDamaged();
	void update(float dt);
	void setFlipped(bool f);
	void setJumping(float h, float t);
	bool isKicking();
	void setKicking(float t);
	void setStomping();
	bool isStomping();
	bool canJump() const;
	void updateGroundHeight();


	sf::Texture dino;
	Input in;
	Animation walk;
	Animation damaged;
	Animation kick;
	Animation stomp;
	Animation squish;
	Animation* currentAnimation;

	// jump component
	float jumpHeight;
	float jumpTimeElapsed = 0.f;
	float jumpTime = 0.f;

	float groundHeight;

	float kickTime = 0.f;
	float kickTimeElapsed = 0.f;

	float damagedTimer = 0.f;
	float damageLength = 100.0f;
};

