#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<cstdlib>
#include <time.h>
#include "classes.cpp"

PLAYER* NewGame(OBJECT** objects, PLAYER* _player) {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] != nullptr) {
			delete objects[i];
			objects[i] = nullptr;
		}
	}
	//RESET GRACZA
	PLAYER* player = new PLAYER(PLAYER_X, PLAYER_Y);
	objects[0] = player;
	return player;
}

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void DrawBackground(SDL_Surface* screen, SDL_Surface* background, SDL_Rect* camera, PLAYER* player) {
	camera->y -= player->speed;
	if (camera->y <= 0) {
		camera->y = BG_HEIGHT - SCREEN_HEIGHT;
	}
	SDL_BlitSurface(background, camera, screen, NULL);
};

void DrawObjects(SDL_Surface* screen,OBJECT** objects) {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] != nullptr) {
			DrawSurface(screen, objects[i]->surface, objects[i]->x, objects[i]->y);
		}
	}
}

void UpdatePos(OBJECT** objects, PLAYER* player) {
	for (int i = 1; i < MAX_OBJECTS; i++)
	{
		if (objects[i] != nullptr) {
			if (objects[i]->isMovingUpwards)
				objects[i]->y -= (objects[i]->speed - player->speed);
			else
				objects[i]->y += ( player->speed + objects[i]->speed);
			//IF OBJECT GETS OUT OF SCREEN SCOPE
			if (objects[i]->y >= SCREEN_HEIGHT + objects[i]->surface->h / 2 + DECAY_HEIGHT || objects[i]->y <= -(DECAY_HEIGHT/2) || objects[i]->lives <= 0) {
				delete objects[i];
				objects[i] = nullptr;
			}
		}
	}
}

bool CheckForCollisionsOBJECTS(OBJECT** objects) {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		for (int j = i + 1; j < MAX_OBJECTS; j++) {
			if (objects[i] != nullptr && objects[j] != nullptr) {
				// Check for collision between objects[i] and objects[j]
				if ((objects[i]->x + objects[i]->surface->w / 2 >= objects[j]->x - objects[j]->surface->w / 2)		//LEFT
					&& (objects[i]->x - objects[i]->surface->w / 2 <= objects[j]->x + objects[j]->surface->w / 2)	//RIGHT
					&& (objects[i]->y + objects[i]->surface->h / 2 >= objects[j]->y - objects[j]->surface->h / 2)	//DOWN
					&& (objects[i]->y - objects[i]->surface->h / 2 <= objects[j]->y + objects[j]->surface->h / 2))	//UP
				{
					// Handle collision between objects[i] and objects[j]
 	 				objects[i]->action(objects[j]);
					objects[j]->action(objects[i]);
					return true;
				}
			}
		}
	}
}

void SpawnObjects(OBJECT** objects, PLAYER* player, unsigned int _SEED) {
	static int counter = 0;
	static const int SPAWN_INTERVAL = FPS; // GENEROWANIE OBIEKTU CO SEKUNDE
	unsigned int x;

	counter++;
	if (counter >= SPAWN_INTERVAL) {
		counter = 0;
		for (int i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == nullptr) {
				_SEED = (_SEED * 7621 + 1) % 32768; // Linear congruential generator
				x = ROAD_LEFT + 10 + _SEED % ((ROAD_RIGHT + 1) - ROAD_LEFT - 10); // The range is WIDTH of the road
				if ((int)x % 100 <= 40) {
					ENEMY* obj = new ENEMY(x, SPAWN_Y);
					objects[i] = obj;
				}
				else if ((int)x % 100 <= 70) {
					ALLY* obj = new ALLY(x, SPAWN_Y, player);
					objects[i] = obj;
				}
				else if ((int)x % 100 <= 90) {
					PUDDLE* obj = new PUDDLE(x, SPAWN_Y);
					objects[i] = obj;
				}
				else {
					MISSLETOTAKE* obj = new MISSLETOTAKE(x, SPAWN_Y);
					objects[i] = obj;
				}
				return;
			}
		}
	}
}

void Shoot(OBJECT** objects, PLAYER* player) {
	if (player->bullets > 0 || player->missles > 0) {
		for (int i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == nullptr) {
				if (player->missles > 0) {
					MISSLE* obj = new MISSLE(player->x, player->y - player->surface->h, player);
					objects[i] = obj;
					player->missles--;
				}
				else if (player->bullets > 0) {
					BULLET* obj = new BULLET(player->x, player->y - player->surface->h, player);
					objects[i] = obj;
					player->bullets--;
				}
				return;
			}
		}
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, objectsOnRoad, bonusLivePoints;
	unsigned int seed;
	double delta, worldTime, fpsTimer, fps, distance;
	bool paused, gameOver;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *road;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Rect camera;
	camera.x = 0;
	camera.y = 0;
	camera.w = SCREEN_WIDTH;
	camera.h = SCREEN_HEIGHT;
	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	 //tryb pe³noekranowy / fullscreen mode
	/*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	                                 &window, &renderer);*/
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);
	
	road = SDL_LoadBMP("./road_0.bmp");
	if (road == NULL) {
		printf("SDL_LoadBMP(road_0.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	objectsOnRoad = 0;
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	bonusLivePoints = 0;
	worldTime = 0;
	distance = 0;
	seed = 0;
	paused = false;
	gameOver = false;

	PLAYER* player = new PLAYER(PLAYER_X, PLAYER_Y);
	
	OBJECT** objects = new OBJECT*[MAX_OBJECTS];
	for (int i = 0; i < MAX_OBJECTS; i++) {
		objects[i] = nullptr;
	}
	objects[PLAYER_INDEX] = player;
	while(!quit) {
		if (!paused) {
			t2 = SDL_GetTicks();

			// w tym momencie t2-t1 to czas w milisekundach,
			// jaki uplyna³ od ostatniego narysowania ekranu
			// delta to ten sam czas w sekundach
			// here t2-t1 is the time in milliseconds since
			// the last screen was drawn
			// delta is the same time in seconds
			delta = (t2 - t1) * 0.001;
			t1 = t2;

			worldTime += delta;
			if (player->timeLeft > 0) {
				player->timeLeft -= delta;
			}
			distance += player->speed * delta;
			//Computing new seed value
			seed = (long long)worldTime * 11617 * (player->speed+5); // to randomize spawning objects
			//Computing score
			if (player->freezePointsTime <= 0 && player->timeLeft > 0) {
				player->points += (player->speed * delta) * 5;			//Total number of points
				player->bonusLivePoints += (player->speed * delta) * 5; //Number of points to receive extra live
				player->freezePointsTime = 0;
			}
			//Receiving extra lives
			if (player->bonusLivePoints >= EX_LIVE_POINTS) {
				player->lives++;
				player->bonusLivePoints -= EX_LIVE_POINTS;
			}
			player->freezePointsTime -= delta;
			player->checkGhosting();
			player->ghostTimeLeft -= delta;
			if (player->lives <= 0)
				gameOver = true;
			//
			SDL_FillRect(screen, NULL, czarny);
			DrawBackground(screen, road, &camera, player);
			DrawObjects(screen, objects);
			UpdatePos(objects, player);
			DrawSurface(screen, player->surface, player->x, player->y);

			//CheckForCollisions(objects);
			CheckForCollisionsOBJECTS(objects);
			SpawnObjects(objects, player, seed);


			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};
		}
		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "Krystian Przybysz 188918, pozostaly czas = %.1d s,  %d pkt. Rakiety = %d Zycia = %d", (int)player->timeLeft, (int)player->points, player->missles, player->lives);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - hamowanie, \032 - skret w lewo, \033 - skret w prawo, n - nowa gra");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		//Dolny tekst
		sprintf(text, "a-f,i,j,k,m,n");
		DrawString(screen, screen->w - 60 - strlen(text) * 8 / 2, SCREEN_HEIGHT - 10, text, charset);
		
		if (gameOver)
			paused = true;

		if (paused) {
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
			if(gameOver)
				sprintf(text, "KONIEC GRY, NACISNIJ n BY ROZPOCZAC NOWA GRE... PUNKTY = %d", (int)player->points);
				
			else
				sprintf(text, "PAUZA");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_p) {
						paused = !paused;
						t1 = SDL_GetTicks();
					}
					else if (event.key.keysym.sym == SDLK_n) {
						player = NewGame(objects, player);
						if (paused == true)
							paused = false;
						gameOver = false;
						t1 = SDL_GetTicks();
					}
					else if (event.key.keysym.sym == SDLK_UP) player->moving_up = true;
					else if (event.key.keysym.sym == SDLK_DOWN) player->moving_down = true;
					else if (event.key.keysym.sym == SDLK_RIGHT) player->moving_right = true;
					else if (event.key.keysym.sym == SDLK_LEFT) player->moving_left = true;
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_RIGHT) player->moving_right = false;
					else if (event.key.keysym.sym == SDLK_LEFT) player->moving_left = false;
					else if (event.key.keysym.sym == SDLK_UP) player->moving_up = false;
					else if (event.key.keysym.sym == SDLK_DOWN) player->moving_down = false;
					else if (event.key.keysym.sym == SDLK_SPACE) Shoot(objects, player);
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		if (!paused) {
			frames++;
			//obs³uga poruszania sie
			if (player->moving_left) {
				if (player->x - player->surface->w / 2 >= ROAD_LEFT)
					player->x -= SPEED * (1 + player->speed / 3);
				else if(player->ghostTimeLeft <= 0){
					player->lives--;
					player->speed *= 0.3;
					player->ghostTimeLeft = 3;
				}
			}
			if (player->moving_right) {
				if (player->x + player->surface->w / 2 <= ROAD_RIGHT)
					player->x += SPEED * (1 + player->speed / 3);
				else if (player->ghostTimeLeft <= 0) {
					player->lives--;
					player->speed *= 0.3;
					player->ghostTimeLeft = 3;
				}
			}
			if (player->moving_up) {
				if (player->speed <= MAX_SPEED)
					player->speed += ACCELERATION;
			}
			if (player->moving_down) {
				if (player->speed > 0)
					player->speed -= BRAKING;
				else
					player->speed = 0;
			}


			//obsluga FPS
			if (1000 / FPS > SDL_GetTicks() - t2)
				SDL_Delay(1000 / FPS - (SDL_GetTicks() - t2));
		}
		};
	// zwolnienie pamiêci	
		for (int i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] != nullptr) {
				delete objects[i];
				objects[i] = nullptr;
			}
		}
		delete[] objects;

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
