#include <iostream>
#include <SDL.h>
#include <stdio.h>

#include "nes_crt.h"
#include "nes_cpu.h"

class pixel_rgb
{
public:
	uint8_t red;
	uint8_t grn;
	uint8_t blu;
};
std::array<pixel_rgb, 256 * 240> frame_rgb;

int main(int argc, char* args[])
{
	SDL_Window* gWindow = NULL;
	SDL_Surface* gScreenSurface = NULL;
	SDL_Surface* gCurrentSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL Error: %s" << SDL_GetError();
		return 1;
	}
	gWindow = SDL_CreateWindow("project zero", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		std::cout << "SDL Error: %s" << SDL_GetError();
		return 2;
	}
	gScreenSurface = SDL_GetWindowSurface(gWindow);
	nes_crt nes_crt;
	if (nes_crt.failure)
	{
		std::cout << "cartridge initialization error";
		return 3;
	}


	nes_cpu nes_cpu(&nes_crt);
	nes_cpu.reset();
	SDL_Event event;
	uint32_t time_stamp = 0;

	/*
	int pixel_ctr = 0;
	for (int coarseY = 0; coarseY < 16; coarseY++)
	{
		for (int fineY = 0; fineY < 8; fineY++)
		{
			for (int coarseX = 0; coarseX < 16; coarseX++)
			{
				int loaddr = 256 * coarseY + 16 * coarseX + fineY;
				int hiaddr = loaddr + 8;
				uint8_t loshft = nes_crt.chr[loaddr];
				uint8_t hishft = nes_crt.chr[hiaddr];
				for (int fineX = 0; fineX < 8; fineX++)
				{
					int pixel_val = 0;
					if (loshft & 0x80)
						pixel_val += 1;
					if (hishft & 0x80)
						pixel_val += 2;
					switch (pixel_val)
					{
					case 0:
						frame_rgb[pixel_ctr] = { 0x00, 0x00, 0x00 };
						break;
					case 1:
						frame_rgb[pixel_ctr] = { 0xFF, 0x00, 0x00 };
						break;
					case 2:
						frame_rgb[pixel_ctr] = { 0x00, 0xFF, 0x00 };
						break;
					case 3:
						frame_rgb[pixel_ctr] = { 0x00, 0x00, 0xFF };
						break;
					}
					pixel_ctr++;
					loshft <<= 1;
					hishft <<= 1;
				}
			}
		}
	}
	*/
	
	bool running = true;
	while (running)
	{
		nes_cpu.clock();
		if (nes_cpu.my_ppu.frame_complete)
		{
			nes_cpu.controllerA = 0x00;
			while (SDL_PollEvent(&event) != 0)
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_KP_6:			//FR
						nes_cpu.controllerA |= 0x01;
						break;
					case SDLK_KP_4:			//FL
						nes_cpu.controllerA |= 0x02;
						break;
					case SDLK_KP_5:			//FD
						nes_cpu.controllerA |= 0x04;
						break;
					case SDLK_KP_8:			//FU
						nes_cpu.controllerA |= 0x08;
						break;
					case SDLK_KP_7:			//M2
						nes_cpu.controllerA |= 0x10;
						break;
					case SDLK_KP_9:			//M1
						nes_cpu.controllerA |= 0x20;
						break;
					case SDLK_KP_ENTER:		//FB
						nes_cpu.controllerA |= 0x40;
						break;
					case SDLK_KP_PLUS:		//FA
						nes_cpu.controllerA |= 0x80;
						break;
					}
				}
			}

			int pixel_ctr = 0;
			for (int coarseY = 0; coarseY < 30; coarseY++)
			{
				for (int fineY = 0; fineY < 8; fineY++)
				{
					for (int coarseX = 0; coarseX < 32; coarseX++)
					{
						uint8_t ntbyte = nes_cpu.my_ppu.name_mem[coarseY * 32 + coarseX];
						uint8_t plbyte = nes_cpu.my_ppu.name_mem[0x03C0 + coarseY * 2 + coarseX / 4];
						uint8_t loshft = nes_crt.chr[16 * ntbyte + fineY + 0];
						uint8_t hishft = nes_crt.chr[16 * ntbyte + fineY + 8];
						
						for (int fineX = 0; fineX < 8; fineX++)
						{
							int pixel_val = 0;
							if (loshft & 0x80)
								pixel_val += 1;
							if (hishft & 0x80)
								pixel_val += 2;

							switch (pixel_val)
							{
							case 0:
								frame_rgb[pixel_ctr] = { 0x00, 0x00, 0x00 };
								break;
							case 1:
								frame_rgb[pixel_ctr] = { 0xFF, 0x00, 0x00 };
								break;
							case 2:
								frame_rgb[pixel_ctr] = { 0x00, 0xFF, 0x00 };
								break;
							case 3:
								frame_rgb[pixel_ctr] = { 0x00, 0x00, 0xFF };
								break;
							}

							pixel_ctr++;
							loshft <<= 1;
							hishft <<= 1;
						}
					}
				}
			}
			SDL_FreeSurface(gCurrentSurface);
			gCurrentSurface = SDL_CreateRGBSurfaceFrom(&frame_rgb, 256, 240, 24, 768, 0x0000FF, 0x00FF00, 0xFF0000, 0x000000);
			SDL_Rect scaled = { 0, 0, 512, 480 };
			SDL_BlitScaled(gCurrentSurface, NULL, gScreenSurface, NULL);
			SDL_UpdateWindowSurface(gWindow);
			nes_cpu.my_ppu.frame_complete = false;
		}

		
	}

	SDL_FreeSurface(gCurrentSurface);
	gCurrentSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
	return 0;
}