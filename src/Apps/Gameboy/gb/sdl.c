// #include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

#include "llapi.h"

#include "keys_39gii.h"

#include <string.h>

static int button_start, button_select;
static int button_a, button_b;
static int button_down, button_up, button_left, button_right;
static int button_debug, button_quit;

unsigned int frames;

int sdl_init(void)
{
	return 0;
}

int sdl_update(void)
{
	//uint32_t keys = ll_vm_check_key();
	//uint32_t key = keys & 0xFFFF;
	//uint32_t press = keys >> 16;

/*
	button_up =    (key == KEY_UP)    & (press);
	button_down =  (key == KEY_DOWN)  & (press);
	button_left =  (key == KEY_LEFT)  & (press);
	button_right = (key == KEY_RIGHT) & (press);
*/
	button_up = llapi_is_key_down(KEY_UP) || llapi_is_key_down(KEY_8);
	button_down = llapi_is_key_down(KEY_DOWN) || llapi_is_key_down(KEY_2);
	button_left = llapi_is_key_down(KEY_LEFT) || llapi_is_key_down(KEY_4);
	button_right = llapi_is_key_down(KEY_RIGHT) || llapi_is_key_down(KEY_6);

	button_a = llapi_is_key_down(KEY_SYMB) || llapi_is_key_down(KEY_5);
	button_b = llapi_is_key_down(KEY_HOME);

	button_start = llapi_is_key_down(KEY_NUM);
	button_select = llapi_is_key_down(KEY_VIEWS);

	if(llapi_is_key_down(KEY_9))
	{
	   button_a = 1;
	   button_right = 1;
	}
	if(llapi_is_key_down(KEY_7))
	{
	   button_a = 1;
	   button_left = 1;
	}
	return 0;
}

unsigned int sdl_get_buttons(void)
{
	return (button_start*8) | (button_select*4) | (button_b*2) | button_a;
}

unsigned int sdl_get_directions(void)
{
	return (button_down*8) | (button_up*4) | (button_left*2) | button_right;
}

void sdl_frame(void)
{
	frames++;
	screen_flush();
	//if(frames % 5 == 0)
	//{
	//	for(int y = 0; y < 127; y++)
	//	{
	//		llapi_disp_put_hline_len(y, &gb_frame_buffer[y * 256], 160);
	//	}
	//}
		//ll_disp_put_area(gb_frame_buffer, 0, 0, 255, 126);
	//SDL_UpdateWindowSurface(window);
}

