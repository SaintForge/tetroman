/* linux_game.h --- 
 * 
 * Filename: linux_game.h
 * Author: Sierra
 * Created: Вт окт 10 10:26:51 2017 (+0300)
 * Last-Updated: Ср окт 18 20:58:39 2017 (+0400)
 *           By: Sierra
 */

#if !defined(LINUX_GAME_H)

struct sdl_offscreen_buffer
{
		 SDL_Texture *Texture;
		 u32 *Memory;
		 int Width;
		 int Height;
		 int Pitch;
};

struct window_dimension
{
		 int Width;
		 int Height;
};

struct sdl_memory_bitmap
{
		 u32 Width;
		 u32 Height;
		 u32 Pitch;
		 u32 Rmask;
		 u32 Gmask;
		 u32 Bmask;
		 u32 Amask;

		 u8 BytesPerPixel;
		 u8 BitsPerPixel;
};

#define LINUX_GAME_H
#endif


