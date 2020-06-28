/* date = June 19th 2020 1:15 pm */

#ifndef TETROMAN_PLATFORM_H
#define TETROMAN_PLATFORM_H


#include <stdint.h>
#include <math.h>

// TODO(msokolov): this 2 need to be resolved
#include <string.h>
#include <string>

// TODO(msokolov): we need to get rid of this
#include <vector>

#include <stdint.h>
#include <stddef.h>

using namespace std;

typedef SDL_Rect    game_rect;
typedef SDL_Point   game_point;
typedef SDL_Color   game_color;
typedef SDL_Texture game_texture;
typedef SDL_Surface game_surface;
typedef Mix_Chunk   game_sound;
typedef Mix_Music   game_music;
typedef TTF_Font    game_font;

typedef SDL_RendererFlip figure_flip;

typedef int8_t   s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define Assert(Expression) if(!(Expression)) { LogErrorLine( __FILE__, __LINE__); abort();  }
void LogErrorLine(const char* Message, int Line)
{
    fprintf(stderr, "Assert fail in %s: %d\n",Message, Line);
}

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

struct game_offscreen_buffer
{
    s32 Width;
    s32 Height;
    
    s32 ReferenceWidth;
    s32 ReferenceHeight;
    
    SDL_Renderer *Renderer;
};


struct game_button_state
{
    bool EndedDown;
    bool EndedUp;
};

struct game_keyboard_input
{
    union
    {
        game_button_state Buttons[23];
        struct
        {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftClick;
            game_button_state RightClick;
            game_button_state Escape;
            game_button_state BackQuote;
            game_button_state Q_Button;
            game_button_state E_Button;
            game_button_state Tab;
            
            game_button_state Zero;
            game_button_state One;
            game_button_state Two;
            game_button_state Three;
            game_button_state Four;
            game_button_state Five;
            game_button_state Six;
            game_button_state Seven;
            game_button_state Eight;
            game_button_state Nine;
            game_button_state BackSpace;
            game_button_state Enter;
            game_button_state LeftShift;
        };
    };
};

struct game_input
{
    r32 TimeElapsedMs;
    
    s32 MouseX, MouseY;
    s32 MouseRelX, MouseRelY;
    
    game_button_state MouseButtons[2];
    game_keyboard_input Keyboard;
};


struct game_memory
{
    /* TODO(msokolov): these values needs to be in the game_state */
    u32 CurrentLevelIndex;
    u32 LevelMemoryAmount;
    u32 LevelMemoryReserved;
    
    void *LocalMemoryStorage;
    void *GlobalMemoryStorage;
    void *EditorMemoryStorage;
    
    void *AssetSpace;
    u32 AssetsSpaceAmount;
    bool AssetsInitialized;
    
    game_font *LevelNumberFont;
    
    /* */
    
    void *TransientStorage;
    u64 TransientStorageSize;
    
    void *PermanentStorage;
    u64 PermanentStorageSize;
    
    void *AssetStorage;
    u64 AssetStorageSize;
    
    void *LevelStorage;
    u64 LevelStorageSize;
    
    bool IsInitialized;
};

#endif //TETROMAN_PLATFORM_H
