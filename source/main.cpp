/*

Color Machine-Gun

Game made by Vagozino with art by Ioanna for the OLC Code Jam 2020 (https://itch.io/jam/olc-codejam-2020)

This code is terrible. I am still learning the very basics of C++ and OOP outside of Python.
The game concept is pretty good, though.


*/


#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

enum Mode {Gameplay, Menu, Dead, Exit};
enum State {Win, Normal, Lose};


class Camera
{
public:
	olc::vf2d vPos;
	float fShakeDuration;

public:
	void startShake(float fShakeDuration)
	{
		this->fShakeDuration = fShakeDuration;

		return;
	}

	void shakeCamera(float fElapsedTime)
	{

		vPos = {std::rand() % 10, std::rand() % 10};
		fShakeDuration -= fElapsedTime;

		return;
	}

};

class Entity
{
public:
	olc::vf2d vPos;
	olc::vf2d vVel;
	float fSize;
	float fSpeed;

public:

	bool isColliding(Entity * e)
	{
		return (this->vPos - e->vPos).mag() < (this->fSize + e->fSize);
	}

};

class Bullet : public Entity
{
public:
	float fTime; //Use time for projectiles
	olc::Pixel pColor;

public:
	Bullet(olc::Pixel pColor, olc::vf2d vPos, olc::vf2d vVel, float fSize = 5.0f, float fTime = 3.0f, float fSpeed = 500.0f)
	{
		this->pColor = pColor;
		this->vPos = vPos;
		this->vVel = vVel;
		this->fSize = fSize;
		this->fTime = fTime;
		this->fSpeed = fSpeed;
	}

};

class FloatingText
{
public:
	float fTime;
	float fSpeed;
	std::string sText;
	olc::vf2d vPos;
	olc::vf2d vVel;

public:

	FloatingText(std::string text, olc::vf2d position)
	{
		this->sText = text;
		this->vPos = position;
		this->fTime = 10.0f;
		this->fSpeed = 100.0f;
		this->vVel = {0, -1};

	}


};



class Player : public Entity 
{
public:
	int iScore;
	int iHighScore;
	float fHealth;
	float fMaxHealth;
	std::string sName;
	olc::vf2d vFacing; //Where the player is facing.
	State playerState;
	//float fMaxSpeed = 100;
	//float fAcceletation = 10;

public:
	Player()
	{
		this->fMaxHealth = 10.0f;
		this->fHealth = this->fMaxHealth;
		this->fSize = 13.0f;
		this->fSpeed = 150.0f;
		this->vPos = {std::rand() % 100, std::rand() % 100};

		this->iScore = 0;
		this->iHighScore = 0;
		this->playerState = Normal;
	}

	void move(float fElapsedTime)
	{
	
		if (!(this->vVel.x || this->vVel.y))
		{
			return;
		}

		this->vVel = this->vVel.norm() * fSpeed;
		this->vPos += this->vVel * fElapsedTime;
		this->vVel = {0,0};
	}


};

class Enemy : public Entity
{
public:
	float fCollisionTimer = 1.0f; //Timer for collisions to happen every 1 second
	float fHealth;
	olc::Pixel pColor;

public:
	Enemy(olc::Pixel pColor, float fHealth, olc::vf2d vPos, float fSpeed = 70.0f, float fSize = 15.0f)
	{
		this->pColor = pColor;
		this->fHealth = fHealth;
		this->vPos = vPos;
		this->fSpeed = fSpeed;
		this->fSize = fSize;
	}

	void move(float fElapsedTime, Player * player)
	{	
		this->vVel = (player->vPos - this->vPos).norm();
		this->vPos += this->vVel * this->fSpeed * fElapsedTime;

		this->fCollisionTimer -= fElapsedTime;

		return;
	}

};

class EnemySpawner
{
public:
	int iMaxEnemies;
	float fSpawnTimer = 0.0f;
	float fTimeToSpawn = 1.0f;
	float fTimeTotal = 0.0f;

public:

	EnemySpawner()
	{
		iMaxEnemies = 25;
	}

	void spawnEnemy(std::list<Enemy>& listEnemies, float fElapsedTime , olc::vi2d vScreen, olc::Pixel * pColors, int nColors)
	{
	
		if (listEnemies.size() > iMaxEnemies || fSpawnTimer > 0.0f) 
		{
			fSpawnTimer -= fElapsedTime;
			return;
		}

		listEnemies.push_back(Enemy(pColors[std::rand() % nColors], (std::rand() % 2) + 1, 
			{ float(std::rand() % vScreen.x), float(vScreen.y)}));

		fSpawnTimer = fTimeToSpawn;

		return;
	
	}


};

class Gun
{
public:

	float fBulletDamage;
	float fTimeToFire;
	float fFireTimer = 0.0f;
	olc::Pixel pBulletColor;
	const olc::vf2d vGunOffset = {0.0f, 16.0f};


public:

	Gun()
	{
		fBulletDamage = 1.0f;
		fTimeToFire = 0.1f;
		pBulletColor = olc::WHITE;
	}

	void fireBullet(std::list<Bullet>& listBullets, Player * player, float fElapsedTime)
	{
		if (fFireTimer > 0.0f) 
			{
				fFireTimer -= fElapsedTime;
				return;
			}

	 	listBullets.push_back(Bullet(pBulletColor, player->vPos + vGunOffset, player->vFacing));

	 	fFireTimer = fTimeToFire; //Reset timer. 


	 	return;
	}

};



class Game : public olc::PixelGameEngine
{
private:
	olc::vf2d vMouse;
	
	Player * player = nullptr;
	Camera * camera = nullptr;
	Gun * playerGun = nullptr;
	EnemySpawner * enemySpawner = nullptr;

	olc::Pixel pColors[6] ={olc::RED, olc::GREEN, olc::BLUE, olc::DARK_MAGENTA, olc::CYAN, olc::YELLOW};
	int nColors = 6;

	std::list<Bullet> listBullets;
	std::list<Enemy> listEnemies;
	std::list<FloatingText> listFloatingText;

	Mode CurrentMode;

	float fColorChangeTimer;

	olc::Sprite * sprPlayer = nullptr;
	olc::Sprite * sprEnemy = nullptr;
	olc::Sprite * sprGun = nullptr;

	olc::Decal * decEnemy;
	olc::Decal * decGun;

	const olc::vf2d vPlayerOffset = {16.0f, 16.0f};


public:
	
	Game()
	{
		sAppName = "Color Machine-Gun";
	}


	bool OnUserCreate() override
	{
		player = new Player();
		player->vPos = {ScreenHeight() / 2, ScreenWidth() / 2};
		playerGun = new Gun();
		enemySpawner = new EnemySpawner();
		camera = new Camera();

		sprPlayer = new olc::Sprite("assets/player_states.png");
		sprEnemy = new olc::Sprite("assets/enemy.png");
		sprGun = new olc::Sprite("assets/gun.png");

		decEnemy = new olc::Decal(sprEnemy);
		decGun = new olc::Decal(sprGun);



		CurrentMode = Menu;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::WHITE);
		SetPixelMode(olc::Pixel::NORMAL);
		SetPixelMode(olc::Pixel::MASK); 
		vMouse = { (float)GetMouseX(), (float)GetMouseY() };

		if (CurrentMode == Exit) return false;

		if (CurrentMode == Dead)
		{
			drawEndScreen();
			getMenuInput();
		}

		if (CurrentMode == Menu)
		{
			drawMenu(fElapsedTime);
			getMenuInput();

			return true;
		}

		if (CurrentMode == Gameplay)
		{
			player->vFacing = (vMouse - player->vPos).norm();
			
			getInput(fElapsedTime);

			enemySpawner->spawnEnemy(listEnemies, fElapsedTime, {ScreenWidth(), ScreenHeight()}, pColors, nColors);
			
			if (camera->fShakeDuration > 0) camera->shakeCamera(fElapsedTime);
			else camera->vPos = {0,0};

			updatePlayer();
			updateBullets(fElapsedTime);
			checkBulletCollisions();
			updateEnemies(fElapsedTime);
			updateFloatingText(fElapsedTime);
			player->move(fElapsedTime);

			drawFloatingText();
			drawBullets();
			drawEnemies();
			drawReticle();
			drawPlayer();
			drawPlayerInfo();
			
			return true;	
		}

		return true;
	}

	void getInput(float fElapsedTime)
	{
		if (GetKey(olc::Key::W).bHeld)
		{
			player->vVel += {0.0f,-1.0f};
		}
		if (GetKey(olc::Key::A).bHeld)
		{
			player->vVel += {-1.0f,0.0f};
		}
		if (GetKey(olc::Key::S).bHeld)
		{
			player->vVel += {0.0f,1.0f};
		}
		
		if (GetKey(olc::Key::D).bHeld)
		{
			player->vVel += {1.0f,0.0f};
		}

		if (GetKey(olc::Key::ESCAPE).bPressed) CurrentMode = Menu;

		if (GetKey(olc::Key::G).bPressed) playerGun->pBulletColor = olc::GREEN;
		if (GetKey(olc::Key::R).bPressed) playerGun->pBulletColor = olc::RED;
		if (GetKey(olc::Key::B).bPressed) playerGun->pBulletColor = olc::BLUE;
		if (GetKey(olc::Key::R).bHeld && GetKey(olc::Key::B).bHeld) playerGun->pBulletColor = olc::DARK_MAGENTA;
		if (GetKey(olc::Key::B).bHeld && GetKey(olc::Key::G).bHeld) playerGun->pBulletColor = olc::CYAN;
		if (GetKey(olc::Key::R).bHeld && GetKey(olc::Key::G).bHeld) playerGun->pBulletColor = olc::YELLOW;
		if (GetKey(olc::Key::R).bHeld && GetKey(olc::Key::G).bHeld && GetKey(olc::Key::B).bHeld) 
			playerGun->pBulletColor = olc::WHITE;

		if (GetMouse(0).bHeld) //Firing
		{
		 	playerGun->fireBullet(listBullets, player, fElapsedTime);
		}
		

		return;

	}


	void updatePlayer()
	{
		
		if (player->vPos.x > ScreenWidth()) player->vPos.x = player->vPos.x - ScreenWidth();
		if (player->vPos.y > ScreenHeight()) player->vPos.y = player->vPos.y - ScreenHeight();
		if (player->vPos.x < 0) player->vPos.x = ScreenWidth();
		if (player->vPos.y < 0) player->vPos.y = ScreenHeight();



		if (player->iScore > player->iHighScore)  
		{
			player->iHighScore = player->iScore;
			player->playerState = Win;
		}
		
		if (player->fHealth <= 0.0f)
		{

			//Reset the game
			CurrentMode = Menu; //CurrentMode = End;
			listBullets.clear();
			listEnemies.clear();
			listFloatingText.clear();

			player->fHealth = 10.0f;
			player->iScore = 0;
			player->vPos = {ScreenHeight() / 2, ScreenWidth() / 2};
			CurrentMode = Dead;		
		}

		return;
	}

	void drawReticle()
	{
		
		DrawCircle(vMouse, 3, olc::RED);
	}

	void drawPlayer()
	{
		
		float fAngle = atan2(player->vFacing.y, player->vFacing.x);

		DrawPartialSprite(player->vPos - vPlayerOffset, sprPlayer, {(int)player->playerState * 32, 0}, {32, 32});
		//DrawDecal(player->vPos - vPlayerOffset, decPlayer);
		DrawRotatedDecal(player->vPos + playerGun->vGunOffset, decGun, fAngle, {0.0f, 16.0f}, {2.0f, 2.0f}, playerGun->pBulletColor);
		//DrawCircle(player->vPos + camera->vPos, player->fSize + 1 , olc::BLACK);
		//FillCircle(player->vPos + camera->vPos, player->fSize , playerGun->pBulletColor);
		return;
	}

	void drawPlayerInfo()
	{
		olc::vi2d vBarOffset = {0, -30};
		drawBar(player->vPos + vBarOffset, olc::GREEN, player->fHealth / player->fMaxHealth);
		DrawString(4,24, "SCORE: " + std::to_string(player->iScore), olc::BLACK);		

		return;
	}

	void drawBar(olc::vi2d vPos, olc::Pixel color, float percentage, olc::vi2d vSize = {50, 5}) //Center as anchor
	{
		olc::vi2d vLRCorner = {vSize.x * percentage, vSize.y};
		olc::vi2d vBarOffset = {vSize.x / -2, 0};
		DrawRect(vPos + vBarOffset, vSize, color);
		FillRect(vPos + vBarOffset, vLRCorner, color);

		return;
	}

	void updateBullets(float fElapsedTime)
	{
		for (auto& b : listBullets)
		{	
			b.vPos += b.vVel * fElapsedTime * b.fSpeed;
			b.fTime -= fElapsedTime; 
		}

		listBullets.erase(
			std::remove_if(listBullets.begin(), listBullets.end(), [&](const Bullet& b) {return b.fTime < 0.0f;}),
			listBullets.end());

		return;
	}

	void checkBulletCollisions()
	{
		for (auto& b : listBullets)
		{
			for (auto& e: listEnemies)
			{
				
				if(b.isColliding(&e)) 
				{
					b.fTime = 0.0f;
					if(b.pColor == e.pColor) e.fHealth -= playerGun->fBulletDamage;

				}
					
			}
		}
	}

	void updateEnemies(float fElapsedTime)
	{

		for (auto& e : listEnemies)
		{

			if (e.isColliding(player) && e.fCollisionTimer < 0)
			{
				player->fHealth -= 1.0f;	
				player->playerState = Lose;
				listFloatingText.push_back(FloatingText("-1 HP", player->vPos));
				e.fCollisionTimer = 1.0f; //Reset collision timer.
			}

			else
			{
				e.move(fElapsedTime, player);
			}


			if (e.fHealth <= 0.0f)
			{
				killEnemy(e);
			}
		}

		listEnemies.erase(
			std::remove_if(listEnemies.begin(), listEnemies.end(), [&](const Enemy& e) {return e.fHealth <= 0.0f;}),
			listEnemies.end());


		return;
	}

	void killEnemy(Enemy & e)
	{
		olc::vf2d vRandVel;
		player->iScore++;
		listFloatingText.push_back(FloatingText("+1", e.vPos));
		
		//Do effects
		if (player->playerState == Lose) player->playerState = Normal;
		camera->startShake(0.1f);

		for(int i = 0; i < 4; i++)
		{
			vRandVel = {(static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/2))) - 1, 
				(static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/2))) - 1};

			listBullets.push_back(Bullet(e.pColor, e.vPos, vRandVel.norm()));
		}


	}

	void updateFloatingText(float fElapsedTime)
	{
		for (auto& ft: listFloatingText)
		{
			ft.vPos += ft.vVel * fElapsedTime * ft.fSpeed;
			ft.fTime -= fElapsedTime;
		}

		listFloatingText.erase(
			std::remove_if(listFloatingText.begin(), listFloatingText.end(), [&](const FloatingText& ft) {return ft.fTime < 0.0f;}),
			listFloatingText.end());

		return;
	}

	void drawFloatingText()
	{
		for (auto& ft: listFloatingText)
		{
			DrawString(ft.vPos, ft.sText, olc::BLACK);
		}

		return;
	}

	void drawBullets()
	{
		for (auto& b : listBullets)
		{
			FillCircle(b.vPos, b.fSize, b.pColor);
		}

		return;
	}

	void drawEnemies()
	{
		const olc::vf2d vEnemyOffset = {16.0f, 16.0f};
		
		for (auto& e : listEnemies)
		{
			DrawDecal(e.vPos + camera->vPos - vEnemyOffset, decEnemy, {1.0f, 1.0f}, e.pColor);
			DrawString(e.vPos + camera->vPos, std::to_string((int)e.fHealth), e.pColor);
		}
		return;
	}

	void drawEndScreen()
	{
		DrawStringDecal({10,30},"You died :(", olc::RED, {2.0f, 2.0f});
		DrawStringDecal({10,50},"PRESS SPACE TO CONTINUE", olc::BLACK, {3.0f, 3.0f});
		DrawPartialSprite(player->vPos - vPlayerOffset, sprPlayer, {(int)player->playerState * 32, 0}, {32, 32});

		
	}

	void drawMenu(float fElapsedTime)
	{
		static int iColorFrame = 0;
		olc::vf2d vScale = {4, 4};

		DrawRect(0, 0, ScreenWidth() - 1 , ScreenHeight() - 1, olc::BLACK);
		DrawStringDecal({10,30},"Color Machine-Gun", pColors[iColorFrame % nColors], vScale);

		if (fColorChangeTimer > 0 )
		{
			fColorChangeTimer -= fElapsedTime;
		}
		else 
		{
			iColorFrame ++;
			fColorChangeTimer = 0.5f;
		}
		DrawString(10,80,"Code by Vagozino and art by Ioanna", olc::BLACK);
		DrawString(10,100,"Built using the olc::PixelGameEngine", olc::BLACK);

		DrawString(10,180,"How To Play:", olc::BLACK);
		DrawString(10,200,"WASD to move and Mouse to aim.", olc::BLACK);
		DrawString(10,210,"Press R,G,B to change colors. Colors can be combined!", olc::BLACK);
		
		DrawString(10,240, "Hi-Score: " + std::to_string(player->iHighScore), olc::BLACK);

		DrawStringDecal({10,300},"PRESS SPACE TO START", olc::BLACK, {3.0f, 3.0f});
		DrawString(10,330, "or ESC to exit.", olc::BLACK);
		


	}

	void getMenuInput()
	{

		if (GetKey(olc::Key::SPACE).bPressed) CurrentMode = Gameplay;
		if (GetKey(olc::Key::ESCAPE).bPressed) CurrentMode = Exit;
		


		return;

	}
};


int main()
{
	Game app;
	if (app.Construct(700, 700, 1, 1, false, true)) app.Start();

	return 0;
}

