extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	1080
#define SCREEN_HEIGHT	958
#define FPS				60
#define BG_HEIGHT		1916
#define BG_WIDTH		1080
#define ROAD_LEFT		100
#define ROAD_RIGHT		980
#define DECAY_HEIGHT	400
#define SPEED			2
#define ENEMY_SPEED		8
#define ALLY_SPEED		7
#define MAX_SPEED		16
#define BULLET_SPEED	16
#define MISSLE_SPEED	20
#define BRAKING			0.08
#define ACCELERATION	0.04
#define TIME			100
#define PLAYER_Y		SCREEN_HEIGHT*3/4
#define PLAYER_X		SCREEN_WIDTH/2
#define LIVES			3
#define MAX_OBJECTS		15
#define BULLETS			999
#define PLAYER_INDEX	0
#define KILL_POINTS		500
#define MISSLE_POWER	3
#define SPAWN_Y			-50
#define IMMUNE_TIME		5
#define EX_LIVE_POINTS	10000

class OBJECT {
public:
	char symbol;
	double x, y;
	SDL_Surface* surface;
	double speed = 0;
	bool killable = true;
	bool isActionDone = false;//PRZECIWNICY WYKONUJA TYLKO JEDNA AKCJE SPECJALNA
	bool isMovingUpwards = false;
	double points;
	int missles = 0;
	int bullets = 0;
	int lives = 1;
	double freezePointsTime = 0;
	virtual void action(OBJECT* object) {};
};

class PLAYER
	:public OBJECT
{
public:
	double timeLeft = TIME;
	double bonusLivePoints = 0;
	bool moving_up = false,
		moving_right = false,
		moving_down = false,
		moving_left = false;
	int bullets = BULLETS;
	double ghostTimeLeft = IMMUNE_TIME;

	PLAYER(double _x, double _y) {
		symbol = 'P';
		x = _x;
		y = _y;
		this->speed = 0;
		this->points = 0;
		this->killable = true;
		this->lives = LIVES;
		this->surface = SDL_LoadBMP("./BlueCar.bmp");
	}
	void action(OBJECT* object) {
		if ((object->symbol == 'E' || object->symbol == 'A') && this->ghostTimeLeft <= 0) {
			this->lives--;
			this->ghostTimeLeft = 3.0;
		}
	}
	void checkGhosting() {
		if (this->ghostTimeLeft <= 0) {
			this->surface = SDL_LoadBMP("./BlueCar.bmp");
			this->ghostTimeLeft = 0;
		}
		else
			this->surface = SDL_LoadBMP("./BlueCarTransparent.bmp");
	}
};

class PUDDLE
	:public OBJECT
{
public:
	PUDDLE(double _x, double _y) {
		symbol = 'p';
		x = _x;
		y = _y;
		this->speed = 0;
		this->killable = false;
		this->lives = 1;
		this->surface = SDL_LoadBMP("./puddle.bmp");
	}
	void action(OBJECT* object) {
		if (isActionDone == false && (object->symbol != 'M' && object->symbol != 'B'))
		{
			object->speed *= 0.5;
			this->isActionDone = true;
		}
	}
};

class BULLET
	:public OBJECT
{
public:
	PLAYER* owner;
	BULLET(double _x, double _y, PLAYER* _owner) {
		owner = _owner;
		symbol = 'B';
		x = _x;
		y = _y;
		this->speed = BULLET_SPEED + owner->speed;
		this->killable = true;
		this->lives = 1;
		this->surface = SDL_LoadBMP("./Bullet.bmp");
		this->isMovingUpwards = true;// Because bullet is moving upwards
	}
	void action(OBJECT* object) {
		if (isActionDone == false) {
			if (object->killable && object->symbol != 'M') {
				object->lives--;
				if (object->lives == 0 && object->symbol == 'E' && owner->freezePointsTime <= 0) {
					owner->points += KILL_POINTS;
					owner->bonusLivePoints += KILL_POINTS;
				}
				this->lives = 0;
				this->isActionDone = true;
			}

		}
	}
};

class MISSLE
	:public OBJECT
{
public:
	PLAYER* owner;
	MISSLE(double _x, double _y, PLAYER* _owner) {
		owner = _owner;
		symbol = 'M';
		x = _x;
		y = _y;
		this->speed = MISSLE_SPEED + owner->speed;
		this->killable = true;
		this->lives = 1;
		this->surface = SDL_LoadBMP("./Missle.bmp");
		this->isMovingUpwards = true;// Because bullet is moving upwards
	}

	void action(OBJECT* object) {
		if (isActionDone == false) {
			if (object->killable && object->symbol != 'B') {
				object->lives -= MISSLE_POWER;
				if (object->lives == 0 && object->symbol == 'E' && owner->freezePointsTime <= 0) {
					owner->points += KILL_POINTS;
					owner->bonusLivePoints += KILL_POINTS;
				}
				this->lives = 0;
				this->isActionDone = true;
			}

		}
	}
};

class MISSLETOTAKE
	:public OBJECT
{
public:
	MISSLETOTAKE(double _x, double _y) {
		symbol = 'm';
		x = _x;
		y = _y;
		this->killable = false;
		this->surface = SDL_LoadBMP("./Missle2Take.bmp");
	}
	void action(OBJECT* object) {
		if (object->symbol == 'P') {
			object->missles += 3;
			this->lives = 0;
		}
	}
};

class ENEMY
	:public OBJECT
{
public:
	ENEMY(double _x, double _y) {
		symbol = 'E';
		x = _x;
		y = _y;
		this->speed = ENEMY_SPEED;
		this->killable = true;
		this->lives = 3;
		this->surface = SDL_LoadBMP("./RedCar.bmp");
		this->isMovingUpwards = true;
	}
	void action(OBJECT* object) {
		if (object->killable) {
			object->speed *= 0.3;
			if (object->symbol == 'E') // ENEMIES CAN DESTROY OTHER ENEMIES
				object->lives = 0;
		}
		this->isActionDone = true;
	}
};

class ALLY
	:public OBJECT
{
public:
	PLAYER* player;
	ALLY(double _x, double _y, PLAYER* _player) {
		player = _player;
		symbol = 'A';
		x = _x;
		y = _y;
		this->speed = ALLY_SPEED;
		this->killable = true;
		this->lives = 3;
		this->surface = SDL_LoadBMP("./GreenCar.bmp");
		this->isMovingUpwards = true;
	}
	void action(OBJECT* object) {
		if (object->symbol == 'B' || object->symbol == 'M' || object->symbol == 'P') {
			player->freezePointsTime = 5.0;
			object->speed *= 0.3;
		}
		else if (object->symbol == 'E' || object->symbol == 'A')
			this->lives = 0;
	}
};