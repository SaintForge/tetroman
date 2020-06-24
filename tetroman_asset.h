/* assert_game.h --- 
 * 
 * Filename: assert_game.h
 * Author: Sierra
 * Created: Пн окт 16 16:29:29 2017 (+0300)
 * Last-Updated: Ср окт 25 17:18:06 2017 (+0300)
 *           By: Sierra
 */

#if !defined(ASSERT_GAME_H)

enum asset_type
{
    AssetType_None,
    AssetType_Bitmap,
    AssetType_Sound,
    AssetType_Music
};

struct asset_audio_header
{
    bool IsMusic;
};

struct asset_bitmap_header
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

struct asset_audio
{
    asset_audio_header Header;
    u8 *Data;
};

struct asset_bitmap
{
    asset_bitmap_header Header;
    void *Data;
};

struct asset_header
{
    char AssetName[32];
    u32 AssetSize;
    asset_type AssetType;
    union
    {
        asset_audio  Audio;
        asset_bitmap Bitmap;
    };
};

struct binary_header
{
    u32 BitmapSizeInBytes;
    u32 AudioSizeInBytes;
};

static game_sound*   GetSound(game_memory *Memory, char* FileName);
static game_music*   GetMusic(game_memory *Memory, char* FileName);
static game_texture* GetTexture(game_memory *&Memory, const char* FileName, SDL_Renderer*& Renderer);

static void FreeSound(game_sound *&Sound)
{
    if(Sound)
    {
        Mix_FreeChunk(Sound);
    }
}

static void FreeMusic(game_music *&Music)
{
    if(Music)
    {
        Mix_FreeMusic(Music);
    }
}

static void FreeTexture(game_texture *&Texture)
{
    if(Texture)
    {
        SDL_DestroyTexture(Texture);
    }
}


#define ASSERT_GAME_H
#endif