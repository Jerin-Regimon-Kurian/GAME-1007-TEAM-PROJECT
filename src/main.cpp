#include <crtdbg.h>
#include <iostream>
#include <Windows.h>
#include <sdl.h>
#include <SDL_Image.h>
#include <vector>

constexpr float FPS = 60.0f;//target frames per second
constexpr float DELAY_TIME = 1000.0f / FPS;//target time between frames in ms
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
float deltaTime = 1.0f / FPS;

bool GameIsRunning = true;

//Anything with a star to its right is a pointer, which can also be a null pointer.

SDL_Window* pWindow = nullptr;
SDL_Renderer* pRenderer = NULL;

namespace Game
{
	struct sprite
	{
	//The following are public field variables which can be used outside the struct or class. 
	public:
		SDL_Texture* pTexture;
		SDL_Rect src;
		SDL_Rect Dst;

		sprite()
		{
			std::cout << "Sprite Default Constructor!" << std::endl;
			pTexture = nullptr;
			src = { 0,0,0,0 };
			Dst = { 0,0,0,0 };
		}

		sprite(SDL_Renderer* renderer, const char* imagefilepath)
		{
			std::cout << "Sprite Default Constructor!" << std::endl;
			pTexture = IMG_LoadTexture(renderer, imagefilepath);
			if (pTexture == NULL)
			{
				std::cout << "Image failed to load." << SDL_GetError() << std::endl;
			}
			else
			{
				std::cout << "Image loaded successfully\n";
			}
			//Setting our texture's width and height to a default source rect value.
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

		void Draw(SDL_Renderer* renderer) {

			int result = SDL_RenderCopy(renderer, pTexture, &src, &Dst);
			if (result != 0) {
				std::cout << "Failed to render" << SDL_GetError() << std::endl;
			}
		}

	};
}

using namespace Game;

sprite Bg;
sprite Player;
sprite Enemy;
sprite Obstacles;
sprite Projectiles;
std::vector<sprite>bulletcontainer;

bool Initialize()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	pWindow = SDL_CreateWindow("Jerin Regimon Kurian -101457327, Jewel Kakkanattu James -101435885", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (pWindow == NULL) 
	{
		std::cout << "Failed to create a window." << SDL_GetError() << std::endl;
		return false;
	}
	else {
		std::cout << "Window created successfully\n";
	}

	//Get pointer to SDL_Renderer object for drawing sprites.
	pRenderer = SDL_CreateRenderer(pWindow, -1, 0);

	if (pRenderer == NULL)
	{

		std::cout << "Renderer creation failed!" << SDL_GetError() << std::endl;
		return false;
	}
	else {
		std::cout << "Renderer creation successful!\n";

	}
	return true;
}

void Load() {
	Bg = sprite(pRenderer, "../Assets/textures/Desert.png");
	Bg.Dst.w = 1200;
	Bg.Dst.h = 600;

	Player = sprite(pRenderer, "../Assets/textures/Player.png");

	int shipwidth = Player.src.w;
	int shipheight = Player.src.h;
	Player.Dst.x = SCREEN_WIDTH / 8;
	Player.Dst.y = (SCREEN_HEIGHT / 2) - (shipheight / 2);
	Player.Dst.w = shipwidth;
	Player.Dst.h = shipheight;

	Enemy = sprite(pRenderer, "../Assets/textures/enemy.png");
	Enemy.Dst.x = 700;
	Enemy.Dst.y = 400;
	Enemy.Dst.w = shipwidth;
	Enemy.Dst.h = shipheight;


	Obstacles = sprite(pRenderer, "../Assets/textures/mine.png");
	Obstacles.Dst.x = 250;
	Obstacles.Dst.y = 100;
	Obstacles.Dst.w = shipwidth;
	Obstacles.Dst.h = shipheight;

}

//Players input variables
bool isUpPressed = false;
bool isDownPressed = false;
bool isRightPressed = false;
bool isLeftPressed = false;
bool isShootPressed = false;
float playerMoveSpeedPx = 120.0f; // pixels per second 
float playerShootCooldownDuration = 0.25f; // time between shots
float playerShootCooldownTimer = 0.0f; // time down to determine when we  can shoot again
float BulletSpeed = 500; // pixels per second
float playerFireRepeatDelay = 1.0;//Seconds

void Input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case(SDL_KEYDOWN):// Response to pressing a key. 
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

		case(SDL_KEYUP):// Response to stop pressing a key.
			switch (event.key.keysym.scancode)
			{
			case(SDL_SCANCODE_W):
				isUpPressed = false;  
				break;
			case(SDL_SCANCODE_A):
				isLeftPressed = false;  
				break;
			case(SDL_SCANCODE_S):
				isDownPressed = false;
				break;
			case(SDL_SCANCODE_D):
				isRightPressed = false;  
				break;
			case (SDL_SCANCODE_SPACE):
				isShootPressed = false;
				break;
			}
			break;
		}
	}

}

void update(){
	float playerDeltaX = 0.0f;
	float playerDeltaY = 0.0f;

	if (isUpPressed)
	{
		playerDeltaY = -playerMoveSpeedPx * deltaTime;
	}

	if (isDownPressed)
	{
		playerDeltaY = playerMoveSpeedPx * deltaTime;
	}

	if (isLeftPressed)
	{
		playerDeltaX = -playerMoveSpeedPx * deltaTime;
	}

	if (isRightPressed)
	{
		playerDeltaX = playerMoveSpeedPx * deltaTime;
	}

	// Update player position based on input
	Player.Dst.x += static_cast<int>(playerDeltaX);
	Player.Dst.y += static_cast<int>(playerDeltaY);

	// Check if the player is going beyond the screen's boundaries
	if (Player.Dst.x < 0)
	{
		Player.Dst.x = 0;
	}

	if (Player.Dst.y < 0)
	{
		Player.Dst.y = 0;
	}

	if (Player.Dst.x > SCREEN_WIDTH - Player.Dst.w)
	{
		Player.Dst.x = SCREEN_WIDTH - Player.Dst.w;
	}

	if (Player.Dst.y > SCREEN_HEIGHT - Player.Dst.h)
	{
		Player.Dst.y = SCREEN_HEIGHT - Player.Dst.h;
	}

	// Spawn projectiles when shoot is pressed and player shoot cooldown is over ie. less than or equal to 0.
	if (isShootPressed && playerShootCooldownTimer <= 0.0f) {
		std::cout << "Shoot\n";

		sprite Projectiles = sprite(pRenderer, "../Assets/textures/bullet.png");
		Projectiles.Dst.x = Player.Dst.x + Player.Dst.w;
		Projectiles.Dst.y = Player.Dst.y + (Player.Dst.h / 2) - Projectiles.Dst.h;
		playerShootCooldownTimer = playerFireRepeatDelay;
		bulletcontainer.push_back(Projectiles);
	}
	playerShootCooldownTimer -= deltaTime;

	for (int i = 0; i < bulletcontainer.size(); i++) {
		sprite* someprojectiles = &bulletcontainer[i];
		someprojectiles->Dst.x += BulletSpeed * deltaTime;
	}
}

void Draw()
{
	SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(pRenderer);

	Bg.Draw(pRenderer);
	Player.Draw(pRenderer);
	for (int i = 0; i < bulletcontainer.size(); i++) {
		sprite* someprojectiles = &bulletcontainer[i];
		someprojectiles->Draw(pRenderer);
	}
	Enemy.Draw(pRenderer);
	Obstacles.Draw(pRenderer);
	SDL_RenderPresent(pRenderer);
}

int main(int argc, char* args[])
{
	//Display the application console
	AllocConsole();
	auto console = freopen("CON", "w", stdout);
	const auto window_handle = GetConsoleWindow();
	MoveWindow(window_handle, 100, 700, 800, 200, TRUE);

	GameIsRunning = Initialize();
	Load();



 // Main Game Loop.
	while (GameIsRunning)
	{
		const auto frame_start = static_cast<float>(SDL_GetTicks());
		Input();
		update();
		Draw();

		const float frame_time = static_cast<float>(SDL_GetTicks()) - frame_start;
		if (frame_time < DELAY_TIME)// if time passed is less than the time we have for each frame do nothing for a while until its time for the next frame.
		{
			
			SDL_Delay(static_cast<int>(DELAY_TIME - frame_time));
		}

		const float delta_time = (static_cast<float>(SDL_GetTicks()) - frame_start) / 1000.0f;

	}



	return 0;
}