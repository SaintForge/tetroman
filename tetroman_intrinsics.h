/* date = July 3rd 2020 11:50 am */

#ifndef TETROMAN_INTRINSICS_H
#define TETROMAN_INTRINSICS_H

enum quality_scale_hint {
    NEAREST_SCALE,
    LINEAR_SCALE,
    BEST_SCALE
};

#define Assert(Expression) if(!(Expression)) { LogErrorLine( __FILE__, __LINE__); *(int *)0 = 0; }
void LogErrorLine(const char* Message, int Line)
{
    fprintf(stderr, "Assert fail in %s: %d\n",Message, Line);
}

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

inline static v2
ScaleByLogicalResolution(game_offscreen_buffer *Buffer, v2 Point) {
    v2 Result = {};
    
    v2 SizeRatio = V2(Buffer->WidthRatio, Buffer->HeightRatio);
    v2 NDC = V2(SizeRatio * Point);
    
    Result.x = NDC.x * Buffer->ViewportWidth;
    Result.y = NDC.y * Buffer->ViewportHeight;
    
    return (Result);
}

inline static b32
IsInRectangle(v2 Position, rectangle2 Rectangle)
{
    b32 Result = false;
    
    Result = ((Position.x >= Rectangle.Min.x) &&
              (Position.y >= Rectangle.Min.y) &&
              (Position.x < Rectangle.Max.x) &&
              (Position.y < Rectangle.Max.y));
    
    return(Result);
}

inline static b32
IsInRectangle(v2 *Position, u32 Amount, rectangle2 Rectangle)
{
    b32 Result = false;
    
    for (u32 Index = 0;
         Index < Amount;
         Index++)
    {
        Result = IsInRectangle(Position[Index], Rectangle);
        if (Result) break;
    }
    
    return (Result);
}

inline static b32
IsInRectangle(v2 Position, rectangle2 *Target, u32 TargetAmount)
{
    b32 Result = false;
    
    return (Result);
}

inline static game_texture*
MakeTextureFromString(game_offscreen_buffer *Buffer, game_font *Font, 
                      const char *Text, v4 Color, quality_scale_hint Hint = NEAREST_SCALE)
{
    game_texture *Result = {};
    
    SDL_Color SDLColor = {};
    SDLColor.r = Color.r;
    SDLColor.g = Color.g;
    SDLColor.b = Color.b;
    
    game_surface *Surface = TTF_RenderText_Blended(Font, Text, SDLColor);
    Assert(Surface);
    
    if (Hint == NEAREST_SCALE) 
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    else if (Hint == LINEAR_SCALE)
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    else if (Hint == BEST_SCALE)
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    
    Result = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    Assert(Result);
    
    SDL_FreeSurface(Surface);
    
    return (Result);
}

inline static v2
QueryTextureDim(game_texture *Texture)
{
    v2 Result = {};
    
    s32 w, h;
    SDL_QueryTexture(Texture, 0, 0, &w, &h);
    
    Result.w = w;
    Result.h = h;
    
    return(Result);
}

inline static void
GetTimeString(char TimeString[64], r32 TimeElapsed)
{
    u32 Minutes = (u32)TimeElapsed / 60;
    u32 Seconds = (u32)TimeElapsed % 60;
    
    sprintf(TimeString, "%02d:%02d", Minutes, Seconds);
}

#endif //TETROMAN_INTRINSICS_H
