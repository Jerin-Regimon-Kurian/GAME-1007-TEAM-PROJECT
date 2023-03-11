#include <crtdbg.h>
#include <iostream>
#include <Windows.h>
#include <sdl.h>
#include <SDL_Image.h>

constexpr float FPS = 60.0f;//target frames per second
constexpr float DELAY_TIME = 1000.0f / FPS;//target time between frames in ms
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;

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

bool Initialize()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	pWindow = SDL_CreateWindow("Jerin Regimon Kurian -101457327, Jewel Kakkanattu James -", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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
	Bg = sprite(pRenderer, "../Assets/textures/grassyCliff.png");
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
	Obstacles.Dst.x = 90;
	Obstacles.Dst.y = 100;
	Obstacles.Dst.w = shipwidth;
	Obstacles.Dst.h = shipheight;


	Projectiles = sprite(pRenderer, "../Assets/textures/rocket01.png");
	Projectiles.Dst.x = 600;
	Projectiles.Dst.y = 300;
	Projectiles.Dst.w = 64;
	Projectiles.Dst.h = 64;



}
void Input()
{


}

void update()
{
	Player.Dst.y = Player.Dst.y - 2;
	Enemy.Dst.y = Enemy.Dst.y - 1;
	Obstacles.Dst.y = Obstacles.Dst.y - 1;
	Projectiles.Dst.y = Projectiles.Dst.y - 1;
}

void Draw()
{
	SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(pRenderer);

	Bg.Draw(pRenderer);
	Player.Draw(pRenderer);
	Enemy.Draw(pRenderer);
	Obstacles.Draw(pRenderer);
	Projectiles.Draw(pRenderer);

	SDL_RenderPresent(pRenderer);
}

/**
 * \brief Program Entry Point
 */
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