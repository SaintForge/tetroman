/* entity.h --- 
 * 
 * Filename: entity.h
 * Author: 
 * Created: Ср окт 18 20:58:16 2017 (+0400)
 * Last-Updated: Пт окт 20 16:50:56 2017 (+0300)
 *           By: Sierra
 */

#if !defined(ENTITY_H)

enum figure_form
{
    O_figure, I_figure, L_figure, J_figure,
    Z_figure, S_figure, T_figure
};

enum figure_type
{
    classic, stone, mirror
};

#define TilePerRow 5
#define TilePerColumn 5

struct figure_unit
{
    bool IsStick;
    bool IsEnlarged; // not sure if we need it
    bool IsIdle;
    
    u32 Index;     
    r32 Angle;
    r32 DefaultAngle;
    
    game_point Center;
    game_point DefaultCenter;
    game_point Shell[4];
    game_point DefaultShell[4];
    game_rect AreaQuad;
    
    figure_flip Flip;
    figure_form Form;
    figure_type Type;
    
    game_texture *Texture;
};


struct figure_entity
{
    u32 FigureAmountReserved;
    u32 ReturnIndex;
    s32 FigureActive;
    
    u32 *FigureOrder;
    
    u32 FigureAmount;
    figure_unit *FigureUnit;
    
    game_rect FigureArea;
    
    bool IsGrabbed;
    bool IsRotating;
    bool IsFlipping;
    bool IsReturning;
    bool IsRestarting;
    
    r32 AreaAlpha;
    r32 FigureAlpha;
    
    r32 FadeInSum;
    r32 FadeOutSum;
    r32 RotationSum;
    
    s32 FigureVelocity;
};

struct sticked_unit
{
    s32 Index;
    u32 Row[4];
    u32 Col[4];
    
    bool IsSticked;
    game_point Center;
};

struct moving_block
{
    game_rect AreaQuad;
    
    u32 RowNumber;
    u32 ColNumber;
    
    bool IsMoving;
    bool IsVertical;
    bool MoveSwitch;
};

struct grid_entity
{
    u32 RowAmount;
    u32 ColumnAmount;
    
    u32 MovingBlocksAmount;
    u32 MovingBlocksAmountReserved;
    
    u32 StickUnitsAmount;
    game_rect GridArea;
    
    r32 *UnitSize;
    s32 *UnitField;
    
    sticked_unit *StickUnits;
    moving_block *MovingBlocks;
    
    game_texture *NormalSquareTexture;
    game_texture *VerticalSquareTexture;
    game_texture *HorizontlaSquareTexture;
    
    game_texture *TopLeftCornerFrame;
    game_texture *TopRightCornerFrame;
    game_texture *DownLeftCornerFrame;
    game_texture *DownRightCornerFrame;
};

struct level_config
{
    //u32 DefaultBlocksInRow;
    //u32 DefaultBlocksInCol;
    s32 InActiveBlockSize;
    s32 GridBlockSize;
    
    r32 StartUpTimeToFinish;
    r32 StartUpTimeElapsed;
    
    r32 RotationVel;
    r32 StartAlphaPerSec;
    r32 FlippingAlphaPerSec;
    r32 GridScalePerSec;
    r32 PixelsToDraw;
    r32 PixelsDrawn;
};

struct p_texture
{
    game_texture *Texture;
};

struct level_animation
{
    game_rect FinishQuad;
    game_texture *FinishTexture;
    
    s32 AlphaChannel;
    
    r32 MaxTileDelaySec;
    r32 TilePixelPerSec;
    r32 TileAlphaPerSec;
    r32 TileAnglePerSec;
    r32 TileCountPerSec;
    
    s32 OldRowAmount;
    s32 OldColAmount;
    
    s32 TileAlphaChannel;
    
    r32 *TileAngle;
    r32 *TileOffset;
    s32 *TileAlpha;
    v3  *TilePos;
    p_texture *TileTexture;
    game_rect *TileQuad;
    
    game_rect TileRect;
    
    r32 TimeElapsed;
};

struct level_entity
{
    grid_entity   *GridEntity;
    figure_entity *FigureEntity;
    
    game_texture *LevelNumberTexture;
    game_texture *LevelNumberShadowTexture;
    
    game_rect LevelNumberQuad;
    game_rect LevelNumberShadowQuad;
    
    level_config Configuration;
    level_animation AnimationData;
    
    u32 LevelNumber;
    b32 LevelStarted;
    b32 LevelPaused;
    b32 LevelFinished;
};

struct figure_memory
{
    r32 Angle;
    figure_flip Flip;
    figure_form Form;
    figure_type Type;
};

struct moving_block_memory
{
    u32 RowNumber;
    u32 ColNumber;
    bool IsVertical;
    bool MoveSwitch;
};

struct level_memory
{
    // TODO(Sierra): Add lock/unlock toggle variable
    u32 IsLocked;
    u32 LevelNumber;
    u32 RowAmount;
    u32 ColumnAmount;
    u32 MovingBlocksAmount;
    u32 FigureAmount;
    
    s32 *UnitField;
    moving_block_memory *MovingBlocks;
    figure_memory *Figures;
};

struct menu_button
{
    b32 IsLocked;
    game_rect ButtonQuad;
    game_rect LevelNumberTextureQuad;
    game_texture *LevelNumberTexture;
};

struct menu_entity
{
    bool IsPaused;
    bool IsMoving;
    bool IsAnimating;
    
    s32 SpaceBetweenButtons;
    u32 ButtonSizeWidth;
    u32 ButtonSizeHeight;
    u32 ButtonsAmount;
    u32 ButtonsAmountReserved;
    
    r32 MaxVelocity;
    s32 ScrollingTicks;
    
    u32 TargetIndex;
    s32 ButtonIndex;
    
    s32 MouseOffsetX;
    s32 MouseOffsetY;
    vector2 Velocity;
    
    game_texture *BackTexture;
    game_texture *FrontTexture;
    
    game_rect ButtonsArea[5];
    menu_button  *Buttons;
    
    game_font *MainFont;
    game_font *LevelNumberFont;
};



#define ENTITY_H
#endif