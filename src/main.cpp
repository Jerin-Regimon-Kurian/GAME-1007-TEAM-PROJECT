#include <crtdbg.h>
#include <iostream>
#include <Windows.h>
#include <sdl.h>
#include <SDL_Image.h>
#include<vector>  

constexpr float FPS = 60.0f;//target frames per second
constexpr float DELAY_TIME = 1000.0f / FPS;//target time between frames in ms
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
float deltaTime = 1.0f / FPS; // time passing between frames in seconds

bool isGameRunning = true;


SDL_Window* pWindow = nullptr;// asssigning a pointer to nullptr means the address 0
SDL_Renderer* pRenderer = NULL;//NULL is the address 0

float enemySpawnDelay = 1.0f;//is used to store the time delay between enemy spawns, while 
float enemySpawnTimer = 0.0f;// is used to keep track of the elapsed time since the last enemy spawn.

namespace Fund
{
	struct vec2
	{
		float x = 0;// x and y which represent the coordinates of the point in the x and y directions, respectively.
		float y = 0;
	};


	struct sprite
	{
		//the public field variable means that things outside this struct or class .
	private:
		SDL_Texture* pTexture;//represents the image to be rendered.

		SDL_Rect src;// specifies the portion of a texture that should be displayed on the screen.

		SDL_Rect Dst;// position and size of the texture on the screen.

	public:
		double rotation = 0;//used to rotate an image or sprite before rendering.0 means no rotation

		SDL_RendererFlip flipState = SDL_FLIP_NONE;

		vec2 position;

		void SetSize(vec2 sizeWidthHeight)
			//The method updates the width and height of the destination rectangle
		{
			Dst.w = sizeWidthHeight.x;// represents hight and width
			Dst.h = sizeWidthHeight.y;
		}

		SDL_Rect GetRect() const
			//returns a copy of the SDL_Rect used for rendering the sprite, with its x and y position updated to match the sprite's current position.
		{
			SDL_Rect returnValue = Dst;
			returnValue.x = position.x;
			returnValue.y = position.y;
			return Dst;
		}

		void SetSize(int w, int h)
		{
			Dst.w = w;
			Dst.h = h;
		}

		vec2 GetSize() const
		{
			vec2 returnvec = { Dst.w,Dst.h };
			return returnvec;
		}

		sprite()
			// default constructor
		{
			std::cout << "sprite default constructor!" << std::endl;
			pTexture = nullptr;
			src = { 0,0,0,0 };
			Dst = { 0,0,0,0 };
		}

		// sprite constructor

		sprite(SDL_Renderer* renderer, const char* imagefilepath) // Only a  non-default constructor can take arguments
		{
			std::cout << "sprite default constructor!" << std::endl;
			pTexture = IMG_LoadTexture(renderer, imagefilepath);
			if (pTexture == NULL)
			{
				std::cout << "Image load failed!" << SDL_GetError() << std::endl;
			}
			else
			{
				std::cout << "Image load successful!\n";
			}


			if (SDL_QueryTexture(pTexture, NULL, NULL, &src.w, &src.h) != 0) {
				std::cout << "Query Texture failed!" << SDL_GetError() << std::endl;
			}

			src.x = 0;
			src.y = 0;

			Dst.x = 0;
			Dst.y = 0;
			Dst.w = src.w;
			Dst.h = src.h;
		}

		// renders the sprites

		void Draw(SDL_Renderer* renderer)
		{

			Dst.x = position.x;
			Dst.y = position.y;
			SDL_RenderCopyEx(renderer, pTexture, &src, &Dst, rotation, NULL, flipState);
		}
	};

	class projectiles
	{
	public:
		sprite sprite;
		vec2 velocity;

		void Update()// updates the position of the sprite based on the current velocity of the sprite

		{
			sprite.position.x += velocity.x * deltaTime;
			sprite.position.y += velocity.y * deltaTime;
			std::cout << "Sprite position: " << sprite.position.x << ", " << sprite.position.y << std::endl;
			std::cout << "Velocity: " << velocity.x << ", " << velocity.y << std::endl;
		}
	};

	class ship
	{
	public:
		sprite sprite;
		float moveSpeedPx = 100;
		float fireRepeatDelay = 0.1f;

	private:
		float fireRepeatTimer = 0.0f;

	public:
		void Move(vec2 input)
		{
			sprite.position.y += input.y * (moveSpeedPx * deltaTime);
			//updates the y-position of the sprite by adding the product of the y-component of the input vector and the move speed and then 
			// multiplied by the time since the last frame 
			sprite.position.x += input.x * (moveSpeedPx * deltaTime);
		}

		void Shoot(bool towardRight, std::vector<Fund::projectiles>& container, Fund::vec2 velocity)
		{
			Fund::sprite projectilesSprite = Fund::sprite(pRenderer, "../Assets/textures/bullet.png");

			//sets the position of the projectile sprite for shooting 
			projectilesSprite.position.x = sprite.position.x;
			if (towardRight)
			{
				projectilesSprite.position.x += sprite.GetSize().x;
			}

			projectilesSprite.position.y = sprite.position.y + (sprite.GetSize().y / 2) - (projectilesSprite.GetSize().y);

			projectiles projectiles;
			projectiles.sprite = projectilesSprite;

			projectiles.velocity = velocity;// v of the projectile

			//add bullet to container ( to the end of the array)
			container.push_back(projectiles);

			//tick down the timefor our firing cooldown 
			fireRepeatTimer = fireRepeatDelay;
		}

		void Update()
			//control the rate at which shooting occurs
		{
			fireRepeatTimer -= deltaTime;
		}

		bool CanShoot()
			// check whether the sprite is able to shoot aprojectiles or not
		{
			return (fireRepeatTimer <= 0.0f);
		}
	};
	// to make the player stay in the screen area
	// collision detection 
	bool areBoundsOverlapping(int minA, int maxA, int minB, int maxB)
	{
		bool isoverlapping = false;
		if (maxA >= minB && maxA <= maxB)
			isoverlapping = true;
		if (minA <= maxB && minA >= minB)
			isoverlapping = true;
		return isoverlapping;
	}

	bool areSptritesOverlapping(const sprite& A, const sprite& B)
	{
		int minAx, maxAx, minBx, maxBx;
		int minAy, maxAy, minBy, maxBy;

		SDL_Rect boundsA = A.GetRect();
		SDL_Rect boundsB = B.GetRect();

		SDL_bool isColliding = SDL_HasIntersection(&boundsA, &boundsB);
		return(bool)isColliding;
	}
}

using namespace Fund;


sprite Background;
ship Player;
sprite obstacles;

std::vector<projectiles>PlayerBulletcontainer; //represents projectiles fired by the player
std::vector<ship>enemycontainer;//epresents enemies in the game
std::vector<projectiles>enemybulletcontainer;// represents projectiles fired by the enemies

// set up an SDL game window
bool Initialize()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	pWindow = SDL_CreateWindow("Jerin Regimon Kurian -101457327, Jewel Kakkanattu James -101435885", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (pWindow == NULL) // if window address as not been set
	{
		std::cout << "window creation failed!" << SDL_GetError() << std::endl;
		return false;
	}
	else {
		std::cout << "window creation successful!\n";
	}

	// pointers are setup to SDL_Renderer object to use for drawing sprites
	pRenderer = SDL_CreateRenderer(pWindow, -1, 0);

	if (pRenderer == NULL) //if create renderer failed....
	{

		std::cout << "Renderer creation failed!" << SDL_GetError() << std::endl;
		return false;
	}
	else {
		std::cout << "Renderer creation successful!\n";

	}
	return true;
}

// Define a variable to keep track of the background position
float backgroundSpeed = 1.0f;
float backgroundX = 0.0f; // declare backgroundX as a global variable

// Define two instances of the background sprite
sprite Background1 = sprite(pRenderer, "../Assets/textures/Desert.png");
sprite Background2 = sprite(pRenderer, "../Assets/textures/Desert.png");


void Load()
{
	// loading textures for background
	Background1 = sprite(pRenderer, "../Assets/textures/Desert.png");
	Background2 = sprite(pRenderer, "../Assets/textures/Desert.png");

	// Set the size of the background sprites
	Fund::vec2 backgroundSize = Background1.GetSize();

	// represents sprites for background
	Background1.SetSize(SCREEN_WIDTH, backgroundSize.y);// represents the width of the screen
	Background2.SetSize(SCREEN_WIDTH, backgroundSize.y);

	// Set the initial positions of the background sprites
	Background1.position.x = 0;
	Background1.position.y = 0;
	Background2.position.x = backgroundSize.x;
	Background2.position.y = 0;

	// Load the player texture
	Player.sprite = sprite(pRenderer, "../Assets/textures/Player.png");

	// Resize the player sprite
	Fund::vec2 shipSize = Player.sprite.GetSize();

	// setting the size of a sprite object for the player's ship
	int shipwidth = shipSize.x;// sets the size of the sprite image file
	int shipheight = shipSize.y;
	Player.sprite.SetSize(shipwidth, shipheight);

	// Set the initial position of the player sprite
	Player.sprite.position.x = 50;
	Player.sprite.position.y = (SCREEN_HEIGHT / 1) - (shipwidth / 1);



}


//Players input variables
bool isUpPressed = false;
bool isDownPressed = false;
bool isRightPressed = false;
bool isLeftPressed = false;
bool isShootPressed = false;

float bulletSpeedPx = 1000;
void Input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case(SDL_KEYDOWN):
			switch (event.key.keysym.scancode)
			{
			case(SDL_SCANCODE_W):
				isUpPressed = true;
				break;
			case(SDL_SCANCODE_A):
				isLeftPressed = true;
				break;
			case(SDL_SCANCODE_S):
				isDownPressed = true;
				break;
			case(SDL_SCANCODE_D):
				isRightPressed = true;
				break;
			case(SDL_SCANCODE_SPACE):
				isShootPressed = true;
				break;
			}
			break;
		case(SDL_KEYUP):
			switch (event.key.keysym.scancode)
			{
			case(SDL_SCANCODE_W):
				isUpPressed = false;
			case(SDL_SCANCODE_A):
				isLeftPressed = false;
				break;
				break;
			case(SDL_SCANCODE_S):
				isDownPressed = false;

				break;
			case(SDL_SCANCODE_D):
				isRightPressed = false;
				break;
			case(SDL_SCANCODE_SPACE):
				isShootPressed = false;
				break;
			}
			break;
		}
	}
}

void spawnEnemy()
// spawn the enemy 
{

	Fund::sprite Enemy;
	Enemy = sprite(pRenderer, "../Assets/textures/enemy.png");
	Fund::vec2 enemySize = Enemy.GetSize();
	int enemywidth = enemySize.x;// calculates the width and height of the enemy
	int enemyheight = enemySize.y;

	Enemy.SetSize(enemywidth, enemyheight);

	Enemy.position =
	{
		SCREEN_WIDTH,
		(FLOAT)(rand() % (SCREEN_HEIGHT - (int)Enemy.GetSize().y))//enemy sprites appears on random positions on the screen
	};

	Enemy.flipState = SDL_FLIP_HORIZONTAL;
	Enemy.rotation = 180;
	// spawns the enemy 

	Fund::ship enemy1;
	enemy1.sprite = Enemy;

	enemy1.fireRepeatDelay = 2.0;
	enemy1.moveSpeedPx = 80;
	enemycontainer.push_back(enemy1);

	enemySpawnTimer = enemySpawnDelay;
}

void UpdatePlayer()
// updates the player's movement and shooting
{
	Fund::vec2 inputVector;
	if (isUpPressed)
	{
		inputVector.y = -1;
		if (Player.sprite.position.y < 0)
		{
			Player.sprite.position.y = 0;
		}


	}
	if (isDownPressed)
	{
		inputVector.y = 1;
		if (Player.sprite.position.x < 0)
		{
			Player.sprite.position.x = 0;
		}

	}
	if (isRightPressed)
	{

		inputVector.x = 1;
		if (Player.sprite.position.x > SCREEN_WIDTH - Player.sprite.GetSize().x)
		{
			Player.sprite.position.x = SCREEN_WIDTH - Player.sprite.GetSize().x;
		}

	}
	if (isLeftPressed)
	{

		inputVector.x = -1;

		if (Player.sprite.position.y > SCREEN_HEIGHT - Player.sprite.GetSize().y)
		{
			Player.sprite.position.y = SCREEN_HEIGHT - Player.sprite.GetSize().y;
		}
	}

	if (isShootPressed && Player.CanShoot())
	{
		bool toRight = true;
		Fund::vec2 velocity = { 1000,0 };
		Player.Shoot(toRight, PlayerBulletcontainer, velocity);
	}



	Player.Move(inputVector);
	Player.Update();


}

void update()
{
	// Update the background position
	backgroundX -= backgroundSpeed;
	Background1.position.x = backgroundX;
	Background2.position.x = backgroundX + Background1.GetSize().x;

	UpdatePlayer();

	//move all bullet across screen
	for (int i = 0; i < PlayerBulletcontainer.size(); i++)
	{

		PlayerBulletcontainer[i].Update();
		// get a reference to the bullet in the container
		//sprite* someprojectiles = &bulletcontainer[i];
		//someprojectiles->position.x += bulletSpeedPx * deltaTime;
	}
	for (int i = 0; i < enemybulletcontainer.size(); i++)
	{
		enemybulletcontainer[i].Update();
	}
	for (int i = 0; i < enemycontainer.size(); i++)
	{

		Fund::ship& enemy = enemycontainer[i];
		enemy.Move({ -1,0 });
		enemy.Update();
		if (enemy.CanShoot());
		{
			bool towardRight = false;

			Fund::vec2 velocity = { -50000,0 };
			enemy.Shoot(towardRight, enemybulletcontainer, velocity);
		}


	}
	if (enemySpawnTimer <= 0)
	{
		spawnEnemy();
	}
	else
	{
		enemySpawnTimer -= deltaTime;
	}
	for (std::vector<Fund::projectiles>::iterator it = enemybulletcontainer.begin(); it != enemybulletcontainer.end();)
	{
		Fund::sprite& enemyBullet = it->sprite;
		if (Fund::areSptritesOverlapping(Player.sprite, enemyBullet))
		{
			std::cout << " player was hit" << std::endl;
			Player.sprite.rotation += 90.0;

			it = enemybulletcontainer.erase(it);

		}
		if (it != enemybulletcontainer.end()) it++;

	}

	for (std::vector<Fund::projectiles>::iterator bulletIterator = PlayerBulletcontainer.begin(); bulletIterator != PlayerBulletcontainer.end();)
	{

		for (auto enemyIterator = enemycontainer.begin(); enemyIterator != enemycontainer.end();)
		{

			if (Fund::areSptritesOverlapping(bulletIterator->sprite, enemyIterator->sprite))
			{
				bulletIterator = PlayerBulletcontainer.erase(bulletIterator);
				enemyIterator = enemycontainer.erase(enemyIterator);

				if (bulletIterator == PlayerBulletcontainer.end())
					break;

			}

			if (enemyIterator != enemycontainer.end()) enemyIterator++;
		}
		if (bulletIterator != PlayerBulletcontainer.end()) bulletIterator++;
	}


}


void Draw()
// drawing all the sprites
{
	SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 250);
	SDL_RenderClear(pRenderer);

	// Update the background position
	backgroundX -= backgroundSpeed;
	if (backgroundX <= -Background1.GetSize().x)
	{
		backgroundX += Background1.GetSize().x;
	}

	// Draw the background
	Background1.position.x = backgroundX;
	Background2.position.x = backgroundX + Background1.GetSize().x;

	Background1.Draw(pRenderer);
	Background2.Draw(pRenderer);

	// If the background is fully off-screen, reset its position to start again
	if (Background1.position.x <= -Background.GetSize().x)
	{
		Background1.position.x = 0;
	}

	// Draw the player
	Player.sprite.Draw(pRenderer);
	for (int i = 0; i < PlayerBulletcontainer.size(); i++)
	{
		PlayerBulletcontainer[i].sprite.Draw(pRenderer);
	}

	for (int i = 0; i < enemybulletcontainer.size(); i++)
	{
		enemybulletcontainer[i].sprite.Draw(pRenderer);

	}
	for (int i = 0; i < enemycontainer.size(); i++)
	{
		enemycontainer[i].sprite.Draw(pRenderer);

	}
	obstacles.Draw(pRenderer);

	SDL_RenderPresent(pRenderer);
}


int main(int argc, char* args[])
{
	// show and position the application console
	AllocConsole();
	auto console = freopen("CON", "w", stdout);
	const auto window_handle = GetConsoleWindow();
	MoveWindow(window_handle, 100, 700, 800, 200, TRUE);

	isGameRunning = Initialize();
	Load();





	// Main Game Loop;
	while (isGameRunning)
	{
		//time at the start of the frame, in ms
		const auto frame_start = static_cast<float>(SDL_GetTicks());
		Input();//take player input
		update();//update game state
		Draw();//draw to score to show new game state to player

		// current time - time at start of frame = time elapsed during this frame
		const float frame_time = static_cast<float>(SDL_GetTicks()) - frame_start;
		if (frame_time < DELAY_TIME)// if time passed is less than the time we have for each frame...
		{
			//do nothing for a while until its time for the next frame
			SDL_Delay(static_cast<int>(DELAY_TIME - frame_time));
		}



	}



	return 0;
}