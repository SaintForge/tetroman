// menu_game.cpp --- 
// 
// Filename: game.cpp
// Author: Sierra
// Created: Вт окт 10 10:32:14 2017 (+0300)
// Last-Updated: Пт окт 27 14:17:15 2017 (+0300)
//           By: Sierra
//

#include "menu_game.h"

static void
MenuChangeButtonText(game_font *&Font, char *Text, 
                     menu_entity *MenuEntity, 
                     menu_button *Button, 
                     game_color Color, 
                     game_offscreen_buffer *Buffer)
{
    game_surface *Surface = TTF_RenderUTF8_Blended(Font, Text, Color);
    Assert(Surface);
    
    Button->LevelNumberTextureQuad.w = Surface->w;
    Button->LevelNumberTextureQuad.h = Surface->h;
    
    Button->LevelNumberTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    Assert(Button->LevelNumberTexture);
    
    SDL_FreeSurface(Surface);
    
    Button->ButtonQuad.w = MenuEntity->ButtonSizeWidth;
    Button->ButtonQuad.h = MenuEntity->ButtonSizeHeight;
}

static void 
MenuDeleteLevel(game_offscreen_buffer *Buffer, 
                game_memory *Memory, u32 Index)
{
    if(Index >= Memory->LevelMemoryAmount) return;
    
    //TODO gotta delete the one that needs to be deleted first!!!
    
    if(Memory->LevelMemory[Index].UnitField)
    {
        free(Memory->LevelMemory[Index].UnitField);
    }
    
    if(Memory->LevelMemory[Index].MovingBlocks)
    {
        free(Memory->LevelMemory[Index].MovingBlocks);
    }
    
    if(Memory->LevelMemory[Index].Figures)
    {
        free(Memory->LevelMemory[Index].Figures);
    }
    
    for(u32 i = Index; i < Memory->LevelMemoryAmount-1; ++i)
    {
        Memory->LevelMemory[i].LevelNumber  = Memory->LevelMemory[i+1].LevelNumber;
        Memory->LevelMemory[i].RowAmount    = Memory->LevelMemory[i+1].RowAmount;
        Memory->LevelMemory[i].ColumnAmount = Memory->LevelMemory[i+1].ColumnAmount;
        Memory->LevelMemory[i].MovingBlocksAmount = Memory->LevelMemory[i+1].MovingBlocksAmount;
        Memory->LevelMemory[i].FigureAmount = Memory->LevelMemory[i+1].FigureAmount;
        
        Memory->LevelMemory[i].UnitField    = Memory->LevelMemory[i+1].UnitField;
        Memory->LevelMemory[i].MovingBlocks = Memory->LevelMemory[i+1].MovingBlocks;
        Memory->LevelMemory[i].Figures      = Memory->LevelMemory[i+1].Figures;
    }
    
    Memory->LevelMemoryAmount  -= 1;
    
    Memory->MenuEntity->ButtonsAmount  -= 1;
    Memory->MenuEntity->NewButtonIndex -= 1;
    
    MenuChangeButtonText(Memory->LevelNumberFont, "+", 
                         Memory->MenuEntity, 
                         &Memory->MenuEntity->Buttons[Memory->MenuEntity->NewButtonIndex], 
                         {255, 255 ,255}, 
                         Buffer);
    
    Memory->LevelMemory[Memory->LevelMemoryAmount].RowAmount          = 0;
    Memory->LevelMemory[Memory->LevelMemoryAmount].ColumnAmount       = 0;
    Memory->LevelMemory[Memory->LevelMemoryAmount].MovingBlocksAmount = 0;
    Memory->LevelMemory[Memory->LevelMemoryAmount].FigureAmount       = 0;
    
    Memory->LevelMemory[Memory->LevelMemoryAmount].UnitField   = NULL;
    Memory->LevelMemory[Memory->LevelMemoryAmount].MovingBlocks = NULL;
    Memory->LevelMemory[Memory->LevelMemoryAmount].Figures     = NULL;
}

static void
MenuUpdateAndRender(game_offscreen_buffer *Buffer, game_memory *Memory, game_input *Input)
{
    s32 OffsetX = 0;
    
    if(Input->WasPressed)
    {
        if(Input->BackQuote.IsDown)
        {
            if(Memory->MenuEntity->DevMode)
            {
                printf("Dev mode is off\n");
                Memory->MenuEntity->DevMode = false;
            }
            else
            {
                printf("Dev mode is on\n");
                Memory->MenuEntity->DevMode = true;
            }
        }
        
        if(Input->LeftClick.IsDown)
        {
            Memory->MenuEntity->IsMoving    = true;
            Memory->MenuEntity->IsAnimating = false;
            
            Memory->MenuEntity->OldMouseX      = Input->MouseX;
            Memory->MenuEntity->ScrollingTicks = SDL_GetTicks();
            
            Memory->MenuEntity->TargetPosition  = 0;
            Memory->MenuEntity->Velocity.x      = 0;
            
            u32 BeginIndex = Memory->MenuEntity->TargetIndex * 20;
            u32 EndIndex   = BeginIndex + 20;
            
            for(u32 i = BeginIndex; i < EndIndex; ++i)
            {
                if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Memory->MenuEntity->Buttons[i].ButtonQuad))
                {
                    Memory->MenuEntity->ButtonIndex = i;
                    break;
                }
            }
            
            if(Memory->MenuEntity->DevMode)
            {
                if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Memory->MenuEntity->ConfirmButtons[0].ButtonQuad))
                {
                    Memory->MenuEntity->IsToBeDeleted = true;
                }
                Memory->MenuEntity->IsShowingDelete = false;
            }
            
        }
        else if(Input->LeftClick.WasDown)
        {
            Memory->MenuEntity->IsMoving    = false;
            Memory->MenuEntity->IsAnimating = true;
            Memory->MenuEntity->ScrollingTicks = SDL_GetTicks() - Memory->MenuEntity->ScrollingTicks;
            Memory->MenuEntity->MaxVelocity = 20.0f;
            //Memory->MenuEntity->ButtonIndex = -1;
            
            u32 ButtonsAreaAmount = (Memory->MenuEntity->ButtonsAmount / 20) + 1;
            
            if(abs(Memory->MenuEntity->TargetPosition) >= Memory->MenuEntity->ButtonSizeWidth * 0.5f)
            {
                s32 Center_x = 0;
                s32 Center_y = 0;
                s32 CenterOffset = 0;
                
                if(Memory->MenuEntity->ScrollingTicks < 500)
                {
                    bool ShouldJump = false;
                    
                    s32 LeftBorder  = Memory->MenuEntity->ButtonsArea[0].x;
                    s32 RightBorder = Memory->MenuEntity->ButtonsArea[ButtonsAreaAmount-1].x + Memory->MenuEntity->ButtonsArea[ButtonsAreaAmount-1].w;
                    
                    ShouldJump = (LeftBorder >= Buffer->Width / 2) || (RightBorder <= Buffer->Width / 2);
                    
                    if(!ShouldJump)
                    {
                        s32 TargetOffset = Memory->MenuEntity->TargetPosition;
                        if(abs(TargetOffset) < Memory->MenuEntity->ButtonsArea[0].w)
                        {
                            TargetOffset = TargetOffset > 0
                                ? Buffer->Width
                                : -Buffer->Width;
                            
                            Memory->MenuEntity->MaxVelocity   *= 2;
                            Memory->MenuEntity->TargetPosition = TargetOffset;
                            Memory->MenuEntity->ButtonIndex    = -1;
                        }
                    }
                }
                
                if(Memory->MenuEntity->TargetPosition > 0)
                {
                    Center_x = Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].x + (Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].w * 0.2f);
                }
                else
                {
                    Center_x = Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].x + (Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].w * 0.8f);
                }
                
                CenterOffset = Center_x - Memory->MenuEntity->TargetPosition;
                
                for(u32 i = 0; i < ButtonsAreaAmount; ++i)
                {
                    game_rect TargetArea;
                    TargetArea.x = Memory->MenuEntity->ButtonsArea[i].x + (Memory->MenuEntity->ButtonsArea[i].w / 2) - (Buffer->Width / 2);
                    TargetArea.y = 0;
                    TargetArea.w = Buffer->Width;
                    TargetArea.h = Buffer->Height;
                    
                    if(IsPointInsideRect(CenterOffset, Center_y, &TargetArea))
                    {
                        Memory->MenuEntity->TargetIndex = i;
                        break;
                    }
                }
                
                Memory->MenuEntity->TargetPosition = Buffer->Width / 2;
            }
            else
            {
                Memory->MenuEntity->TargetPosition = Buffer->Width / 2;
                
                s32 Index = Memory->MenuEntity->ButtonIndex;
                if(Index == Memory->MenuEntity->NewButtonIndex)
                {
                    if(Memory->MenuEntity->DevMode)
                    {
                        u32 RowAmount = Memory->LevelEntity.GridEntity->RowAmount;
                        u32 ColAmount = Memory->LevelEntity.GridEntity->ColumnAmount;
                        
                        LevelEntityUpdateLevelEntityFromMemory(&Memory->LevelEntity,
                                                               Index,
                                                               Memory, Buffer);
                        
                        LevelEditorChangeGridCounters(Memory->LevelEditor, 
                                                      Memory->LevelEntity.GridEntity->RowAmount, Memory->LevelEntity.GridEntity->ColumnAmount, 
                                                      RowAmount, ColAmount,
                                                      Buffer);
                        SDL_DestroyTexture(Memory->MenuEntity->Buttons[Index].LevelNumberTexture);
                        
                        Memory->MenuEntity->ButtonsAmount += 1;
                        
                        char LevelNumber[3] = {0};
                        sprintf(LevelNumber, "%d", Index + 1);
                        
                        MenuChangeButtonText(Memory->LevelNumberFont, LevelNumber, 
                                             Memory->MenuEntity, 
                                             &Memory->MenuEntity->Buttons[Index], 
                                             {255, 255 ,255}, 
                                             Buffer);
                        
                        MenuChangeButtonText(Memory->LevelNumberFont, "+", 
                                             Memory->MenuEntity, 
                                             &Memory->MenuEntity->Buttons[Index+1], 
                                             {255, 255 ,255}, 
                                             Buffer);
                        Memory->MenuEntity->NewButtonIndex = Index+1;
                        
                        MenuEntityAlignButtons(Memory->MenuEntity, Buffer->Width, Buffer->Height);
                        
                        Memory->LevelMemoryAmount += 1;
                    }
                    
                }
                else if(Index >= 0)
                {
                    if(Memory->MenuEntity->DevMode)
                    {
                        if(Memory->MenuEntity->IsToBeDeleted)
                        {
                            MenuDeleteLevel(Buffer, Memory, Memory->MenuEntity->ButtonIndex);
                            
                        }
                    }
                    else
                    {
                        u32 RowAmount = Memory->LevelEntity.GridEntity->RowAmount;
                        u32 ColAmount = Memory->LevelEntity.GridEntity->ColumnAmount;
                        
                        LevelEntityUpdateLevelEntityFromMemory(&Memory->LevelEntity, 
                                                               Index,
                                                               Memory, Buffer);
                        LevelEditorChangeGridCounters(Memory->LevelEditor, 
                                                      Memory->LevelEntity.GridEntity->RowAmount, Memory->LevelEntity.GridEntity->ColumnAmount, 
                                                      RowAmount, ColAmount,
                                                      Buffer);
                        Memory->ToggleMenu = false;
                        Memory->LevelEntity.LevelNumber = Index;
                    }
                }
                
                Memory->MenuEntity->ButtonIndex = -1;
            }
            
        }
        else if(Input->RightClick.IsDown)
        {
            printf("Right mouse click\n");
            
            if(Memory->MenuEntity->DevMode)
            {
                s32 Index = -1;
                
                u32 BeginIndex = Memory->MenuEntity->TargetIndex * 20;
                u32 EndIndex   = BeginIndex + 20;
                
                for(u32 i = BeginIndex; i < EndIndex; ++i)
                {
                    if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Memory->MenuEntity->Buttons[i].ButtonQuad))
                    {
                        Index = i;
                        break;
                    }
                }
                
                if(Index >= 0 && Index != Memory->MenuEntity->NewButtonIndex)
                {
                    Memory->MenuEntity->IsShowingDelete = true;
                    printf("Are u sure u want to delete %d level?\n", Index + 1);
                    
                    Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.x = Memory->MenuEntity->Buttons[Index].ButtonQuad.x + Memory->MenuEntity->Buttons[Index].ButtonQuad.w - (Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.w * 2);
                    Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.y = Memory->MenuEntity->Buttons[Index].ButtonQuad.y;
                    
                    Memory->MenuEntity->ConfirmButtons[0].LevelNumberTextureQuad.x = Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.x + (Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.w / 2) - (Memory->MenuEntity->ConfirmButtons[0].LevelNumberTextureQuad.w / 2);
                    
                    Memory->MenuEntity->ConfirmButtons[0].LevelNumberTextureQuad.y = Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.y + (Memory->MenuEntity->ConfirmButtons[0].ButtonQuad.h / 2) - (Memory->MenuEntity->ConfirmButtons[0].LevelNumberTextureQuad.h / 2);
                    
                    Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.x = Memory->MenuEntity->Buttons[Index].ButtonQuad.x + Memory->MenuEntity->Buttons[Index].ButtonQuad.w - (Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.w);
                    Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.y = Memory->MenuEntity->Buttons[Index].ButtonQuad.y;
                    
                    Memory->MenuEntity->ConfirmButtons[1].LevelNumberTextureQuad.x = Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.x + (Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.w / 2) - (Memory->MenuEntity->ConfirmButtons[1].LevelNumberTextureQuad.w / 2);
                    
                    Memory->MenuEntity->ConfirmButtons[1].LevelNumberTextureQuad.y = Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.y + (Memory->MenuEntity->ConfirmButtons[1].ButtonQuad.h / 2) - (Memory->MenuEntity->ConfirmButtons[1].LevelNumberTextureQuad.h / 2);
                }
                
            }
        }
    }
    
    if(Input->MouseMotion && Memory->MenuEntity->IsMoving)
    {
        OffsetX += Input->MouseRelX;
        Memory->MenuEntity->TargetPosition += OffsetX;
        
        if(abs(Memory->MenuEntity->TargetPosition) >= Memory->MenuEntity->ButtonSizeWidth * 0.5f)
        {
            Memory->MenuEntity->ButtonIndex = -1;
        }
    }
    
    
    if(Memory->MenuEntity->IsAnimating)
    {
        s32 RelativeCenter = Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].x + (Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].w/2);
        
        vector2 Vector = { Memory->MenuEntity->TargetPosition - RelativeCenter, 0 };
        vector2 Acceleration = {0};
        
        r32 Ratio = 0;
        r32 Distance = Vector2Mag(&Vector);
        r32 ApproachRadius = 50.0f;
        Vector2Norm(&Vector);
        if(Distance < ApproachRadius)
        {
            Ratio = Distance / ApproachRadius;
            if(Ratio > 0.02f)
            {
                Vector.x *= Distance / ApproachRadius * Memory->MenuEntity->MaxVelocity;
                Vector.y *= Distance / ApproachRadius * Memory->MenuEntity->MaxVelocity;
            }
            else
            {
                s32 Width = (Buffer->Width / 2) - (Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].w / 2);
                s32 Offset = Width - Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].x;
                
                Vector.x = 0;
                Vector.y = 0;
                
                Memory->MenuEntity->Velocity.x     = 0;
                Memory->MenuEntity->TargetPosition = 0;
                Memory->MenuEntity->IsAnimating    = false;
                
                u32 ButtonsAreaAmount = (Memory->MenuEntity->ButtonsAmount / 20) + 1;
                for(u32 i = 0; i < ButtonsAreaAmount; ++i)
                {
                    Memory->MenuEntity->ButtonsArea[i].x += Offset;
                }
                
                for(u32 i = 0; i < Memory->MenuEntity->ButtonsAmount; ++i)
                {
                    Memory->MenuEntity->Buttons[i].ButtonQuad.x += Offset;
                    Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad.x += Offset;
                }
            }
        }
        else
        {
            Vector.x *= Memory->MenuEntity->MaxVelocity;
            Vector.y *= Memory->MenuEntity->MaxVelocity;
        }
        
        
        Acceleration.x = Vector.x - Memory->MenuEntity->Velocity.x;
        Vector2Add(&Memory->MenuEntity->Velocity, &Acceleration);
    }
    
    //
    // Menu Rendering
    //
    
    game_rect ScreenQuad = {0, 0, Buffer->Width, Buffer->Height };
    DEBUGRenderQuadFill(Buffer, &ScreenQuad, {0, 0, 0}, 0);
    
    u32 ButtonsAreaAmount = (Memory->MenuEntity->ButtonsAmount / 20) + 1;
    for(u32 i = 0; i < ButtonsAreaAmount; ++i)
    {
        if(Memory->MenuEntity->IsMoving)
        {
            Memory->MenuEntity->ButtonsArea[i].x += OffsetX;
            {
                game_rect TargetArea;
                TargetArea.x = Memory->MenuEntity->ButtonsArea[i].x + (Memory->MenuEntity->ButtonsArea[i].w / 2) - (Buffer->Width / 2);
                TargetArea.y = 0;
                TargetArea.w = Buffer->Width;
                TargetArea.h = Buffer->Height;
                
                if(Memory->MenuEntity->DevMode)
                {
                    DEBUGRenderQuad(Buffer, &TargetArea, {255, 0, 0}, 255);
                }
                
            }
        }
        
        if(Memory->MenuEntity->IsAnimating)
        {
            Memory->MenuEntity->ButtonsArea[i].x += roundf(Memory->MenuEntity->Velocity.x);
        }
        
        if(Memory->MenuEntity->DevMode)
        {
            DEBUGRenderQuad(Buffer, &Memory->MenuEntity->ButtonsArea[i], {255, 255, 255}, 255);
        }
        
    }
    
    for(u32 i = 0; i < Memory->MenuEntity->ButtonsAmount; ++i)
    {
        if(Memory->MenuEntity->IsMoving)
        {
            Memory->MenuEntity->Buttons[i].ButtonQuad.x += OffsetX;
            Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad.x += OffsetX;
        }
        
        if(Memory->MenuEntity->IsAnimating)
        {
            Memory->MenuEntity->Buttons[i].ButtonQuad.x += roundf(Memory->MenuEntity->Velocity.x);
            Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad.x += roundf(Memory->MenuEntity->Velocity.x);
        }
        if(Memory->MenuEntity->NewButtonIndex == i)
        {
            if(Memory->MenuEntity->DevMode)
            {
                GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->BackTexture, &Memory->MenuEntity->Buttons[i].ButtonQuad);
                GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->Buttons[i].LevelNumberTexture,
                                         &Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad);
            }
        }
        else if(Memory->MenuEntity->ButtonIndex == i)
        {
            game_point AreaCenter = {0};
            AreaCenter.x = Memory->MenuEntity->Buttons[i].ButtonQuad.x + (Memory->MenuEntity->Buttons[i].ButtonQuad.w / 2);
            AreaCenter.y = Memory->MenuEntity->Buttons[i].ButtonQuad.y + (Memory->MenuEntity->Buttons[i].ButtonQuad.h / 2);
            
            game_rect AreaQuad = {0};
            AreaQuad.w = Memory->MenuEntity->Buttons[i].ButtonQuad.w * 1.2f;
            AreaQuad.h = Memory->MenuEntity->Buttons[i].ButtonQuad.h * 1.2f;
            AreaQuad.x = AreaCenter.x - (AreaQuad.w / 2);
            AreaQuad.y = AreaCenter.y - (AreaQuad.h / 2);
            
            game_rect NumberQuad = {0};
            NumberQuad.w = Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad.w * 1.2f;
            NumberQuad.h = Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad.h * 1.2f;
            NumberQuad.x = AreaCenter.x - (NumberQuad.w / 2);
            NumberQuad.y = AreaCenter.y - (NumberQuad.h / 2);
            
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->BackTexture, &AreaQuad);
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->Buttons[i].LevelNumberTexture,
                                     &NumberQuad);
        }
        else
        {
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->BackTexture, &Memory->MenuEntity->Buttons[i].ButtonQuad);
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->Buttons[i].LevelNumberTexture,
                                     &Memory->MenuEntity->Buttons[i].LevelNumberTextureQuad);
        }
    }
    
    if(Memory->MenuEntity->DevMode)
    {
        if(Memory->MenuEntity->IsMoving)
        {
            s32 ButtonCenter = Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].x + (Memory->MenuEntity->ButtonsArea[Memory->MenuEntity->TargetIndex].w / 2);
            s32 Target = ButtonCenter + Memory->MenuEntity->TargetPosition;
            DEBUGRenderLine(Buffer, 
                            ButtonCenter, Buffer->Height / 2,
                            Target, Buffer->Height / 2,
                            {0, 255, 0}, 255);
        }
        
        if(Memory->MenuEntity->IsShowingDelete)
        {
            DEBUGRenderQuadFill(Buffer, &Memory->MenuEntity->ConfirmButtons[0].ButtonQuad, {0, 255, 0}, 255);
            DEBUGRenderQuad(Buffer, &Memory->MenuEntity->ConfirmButtons[0].ButtonQuad, {0, 0, 0}, 255);
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->ConfirmButtons[0].LevelNumberTexture, &Memory->MenuEntity->ConfirmButtons[0].LevelNumberTextureQuad);
            
            DEBUGRenderQuadFill(Buffer, &Memory->MenuEntity->ConfirmButtons[1].ButtonQuad, {0, 255, 0}, 255);
            DEBUGRenderQuad(Buffer, &Memory->MenuEntity->ConfirmButtons[1].ButtonQuad, {0, 0, 0}, 255);
            GameRenderBitmapToBuffer(Buffer, Memory->MenuEntity->ConfirmButtons[1].LevelNumberTexture, &Memory->MenuEntity->ConfirmButtons[1].LevelNumberTextureQuad);
        }
    }
    
    
}


