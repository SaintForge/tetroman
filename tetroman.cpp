/* ========================================= */
//     $File: tetroman.cpp
//     $Date: October 10th 2017 10:32 am 
//     $Creator: Maksim Sokolov
//     $Revision: $
//     $Description: $
/* ========================================= */

#include "tetroman.h"

#include "tetroman_render_group.cpp"
#include "tetroman_playground.cpp"
#include "tetroman_asset.cpp"
#include "tetroman_menu.cpp"
#include "tetroman_editor.cpp"

game_texture *CreateTexture(game_offscreen_buffer *Buffer, v4 Color) {
    game_texture *Result = {};
    
    Result = SDL_CreateTexture(Buffer->Renderer, SDL_PIXELFORMAT_RGBA32,
                               SDL_TEXTUREACCESS_TARGET, VIRTUAL_GAME_WIDTH, VIRTUAL_GAME_HEIGHT);
    Assert(Result);
    
    SDL_SetRenderTarget(Buffer->Renderer, Result);
    SDL_SetRenderDrawColor(Buffer->Renderer, Color.r, Color.g, Color.b, Color.a);
    
    game_rect Rectangle = {0, 0, VIRTUAL_GAME_WIDTH, VIRTUAL_GAME_HEIGHT};
    
    SDL_RenderFillRect(Buffer->Renderer, &Rectangle);
    SDL_SetRenderTarget(Buffer->Renderer, NULL);
    
    return (Result);
}

static game_return_values
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    game_return_values Result = {};
	
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    
    
    if(!Memory->IsInitialized)
    {
        // NOTE(msokolov): figure colors 
        // 212, 151, 0
        // 212, 10,  128
        // 116, 0,   40
        // 23,  156, 234
        // 108, 174, 0
        
        /* NOTE(msokolov): game_memory initialization starts here */
        /* NOTE(msokolov): game_state initialization starts here */
        InitializeMemoryGroup(&GameState->MemoryGroup, Memory->PermanentStorageSize - sizeof(game_state), (u8*)Memory->PermanentStorage + sizeof(game_state));
        
        r32 WidthRatio = 0.00052083333f;
        r32 HeightRatio = 0.00092592592;
		{
            r32 StandardSize = 50.0f;
            r32 PixelHeight = roundf(StandardSize * HeightRatio * Buffer->ViewportHeight);
            GameState->Font = TTF_OpenFont(FontPath, PixelHeight);
            Assert(GameState->Font);
        }
        
        {
            r32 StandardSize = 80.0f;
            r32 PixelHeight = roundf(StandardSize * HeightRatio * Buffer->ViewportHeight);
            GameState->TimerFont = TTF_OpenFont(FontPath, PixelHeight);
            Assert(GameState->TimerFont);
        }
        
		{
            r32 StandardSize = 40.0f;
            r32 PixelHeight = roundf(StandardSize * HeightRatio * Buffer->ViewportHeight);
            GameState->MenuTimerFont = TTF_OpenFont(FontPath, PixelHeight);
            Assert(GameState->MenuTimerFont);
        }
        
		
        GameState->PlaygroundIndex = 0;
        GameState->CurrentMode     = game_mode::MENU;
		
        playground_config *Configuration = &GameState->Configuration;
        Configuration->StartUpTimeToFinish = 0.0f;
        Configuration->RotationVel         = 1000.0f;
        Configuration->StartAlphaPerSec    = 500.0f;
        Configuration->FlippingAlphaPerSec = 1000.0f;
        Configuration->FigureVelocity      = 600.0f;
        Configuration->MovingBlockVelocity = 555.0f;
        
        /* NOTE(msokolov): playground initialization starts here */
        playground* Playground = &GameState->Playground;
        Playground->LevelNumber           = 0;
        Playground->LevelStarted          = true;
        Playground->LevelFinished         = false;
        Playground->LevelPaused           = false;
        
        Playground->BackgroundTexture         = GetTexture(Memory, "background_.png", Buffer->Renderer);
        Playground->BackgroundDimTexture      = GetTexture(Memory, "background_dimmed.png", Buffer->Renderer);
        Playground->CornerLeftTopTexture      = GetTexture(Memory, "corner_left_top.png", Buffer->Renderer);
        Playground->CornerLeftBottomTexture   = GetTexture(Memory, "corner_left_bottom.png", Buffer->Renderer);
        Playground->CornerRightTopTexture     = GetTexture(Memory, "corner_right_top.png", Buffer->Renderer);
        Playground->CornerRightBottomTexture  = GetTexture(Memory, "corner_right_bottom.png", Buffer->Renderer);
        Playground->VerticalBorderTexture     = GetTexture(Memory, "vertical_border.png", Buffer->Renderer);
        Playground->IndicatorEmptyTexture     = GetTexture(Memory, "level_indicator_empty.png", Buffer->Renderer);
        Playground->IndicatorFilledTexture    = GetTexture(Memory, "level_indicator_filled.png", Buffer->Renderer);
        
        SDL_SetTextureBlendMode(Playground->CornerLeftTopTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->CornerLeftBottomTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->CornerRightTopTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->CornerRightBottomTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->VerticalBorderTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->BackgroundDimTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(Playground->BackgroundTexture, SDL_BLENDMODE_BLEND);
        
        Playground->PickSound   = GetSound(Memory, "figure_pick.wav");
        Playground->StickSound  = GetSound(Memory, "figure_stick.wav");
        Playground->RotateSound = GetSound(Memory, "figure_drop.wav");
        
        /* Playground Option Menu */ 
        playground_options *PlaygroundOptions = &Playground->Options;
        
        PlaygroundOptions->ToggleMenu  = false;
        PlaygroundOptions->Choice      = options_choice::RESTART_OPTION;
        PlaygroundOptions->MenuPosition    = { 1500.0f, 650.0f };
        PlaygroundOptions->ButtonDimension = { 180.0f, 80.0f };
        PlaygroundOptions->GearTexture = GetTexture(Memory, "gear_new.png", Buffer->Renderer);
        PlaygroundOptions->GearShadowTexture = GetTexture(Memory, "gear_new_shadow.png", Buffer->Renderer);
        PlaygroundOptions->HorizontalLineTexture = GetTexture(Memory, "horizontal_border_2.png", Buffer->Renderer);
        PlaygroundOptions->MenuTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Restart", {255, 255, 255, 255});
        PlaygroundOptions->MenuTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "Settings", {255, 255, 255, 255});
        PlaygroundOptions->MenuTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "Menu", {255, 255, 255, 255});
        PlaygroundOptions->MenuTexture[3] = MakeTextureFromString(Buffer, GameState->Font, "Quit", {255, 255, 255, 255});
        
        PlaygroundOptions->MenuShadowTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Restart", {0, 0, 0, 127});
        PlaygroundOptions->MenuShadowTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "Settings", {0, 0, 0, 127});
        PlaygroundOptions->MenuShadowTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "Menu", {0, 0, 0, 127});
        PlaygroundOptions->MenuShadowTexture[3] = MakeTextureFromString(Buffer, GameState->Font, "Quit", {0, 0, 0, 127});
        
        // Animation control
        Playground->Animation.Finished = false;
        Playground->Animation.InterpPoint = 0.0f;
        Playground->Animation.TimeMax = 1.0f;
        
        Playground->GearIsRotating = false;
        Playground->GearRotationSum = 0.0f;
        Playground->GearAngle = 0.0f;
        
        /* NOTE(msokolov): figure_entity initialization starts here */
        figure_entity* FigureEntity  = &Playground->FigureEntity;
        FigureEntity->FigureAmount   = 0;
        FigureEntity->ReturnIndex    = -1;
        FigureEntity->FigureActive   = -1;
        FigureEntity->FigureOutline  = -1;
        FigureEntity->IsGrabbed      = false;
        FigureEntity->IsRotating     = false;
        FigureEntity->IsReturning    = false;
        FigureEntity->IsRestarting   = false;
        FigureEntity->IsFlipping     = false;
        FigureEntity->RotationSum    = 0;
        FigureEntity->AreaAlpha      = 0;
        FigureEntity->FigureAlpha    = 0;
        FigureEntity->FadeInSum      = 0;
        FigureEntity->FadeOutSum     = 0;
        FigureEntity->FigureVelocity = 400.0f;
        FigureEntity->RotationVelocity = Configuration->RotationVel;
        
        FigureEntity->FigureArea.Min.x = 1500;
        FigureEntity->FigureArea.Min.y = 81;
        FigureEntity->FigureArea.Max.x = FigureEntity->FigureArea.Min.x + 552;
        FigureEntity->FigureArea.Max.y = FigureEntity->FigureArea.Min.y + 972;
        
        FigureEntityAlignFigures(&Playground->FigureEntity);
        
        for(u32 i = 0; i < FIGURE_AMOUNT_MAXIMUM; ++i) 
        {
            FigureEntity->FigureOrder[i] = i;
        }
        
        FigureEntity->O_GreenTexture   = GetTexture(Memory, "o_green.png", Buffer->Renderer);
        FigureEntity->O_BlueTexture    = GetTexture(Memory, "o_blue.png", Buffer->Renderer);
        FigureEntity->O_OrangeTexture  = GetTexture(Memory, "o_orange.png", Buffer->Renderer);
        FigureEntity->O_ClassicTexture = GetTexture(Memory, "o_red.png", Buffer->Renderer);
        FigureEntity->O_ShadowTexture  = GetTexture(Memory, "o_shadow.png", Buffer->Renderer);
        FigureEntity->O_OutlineTexture  = GetTexture(Memory, "o_outline.png", Buffer->Renderer);
        
        FigureEntity->I_GreenTexture   = GetTexture(Memory, "i_green.png", Buffer->Renderer);
        FigureEntity->I_BlueTexture    = GetTexture(Memory, "i_blue.png", Buffer->Renderer);
        FigureEntity->I_OrangeTexture  = GetTexture(Memory, "i_orange.png", Buffer->Renderer);
        FigureEntity->I_ClassicTexture = GetTexture(Memory, "i_red.png", Buffer->Renderer);
        FigureEntity->I_ShadowTexture  = GetTexture(Memory, "i_shadow.png", Buffer->Renderer);
        FigureEntity->I_OutlineTexture  = GetTexture(Memory, "i_outline.png", Buffer->Renderer);
        
        FigureEntity->L_GreenTexture   = GetTexture(Memory, "l_green.png", Buffer->Renderer);
        FigureEntity->L_BlueTexture    = GetTexture(Memory, "l_blue.png", Buffer->Renderer);
        FigureEntity->L_OrangeTexture  = GetTexture(Memory, "l_orange.png", Buffer->Renderer);
        FigureEntity->L_ClassicTexture = GetTexture(Memory, "l_red.png", Buffer->Renderer);
        FigureEntity->L_ShadowTexture  = GetTexture(Memory, "l_shadow.png", Buffer->Renderer);
        FigureEntity->L_OutlineTexture  = GetTexture(Memory, "l_outline.png", Buffer->Renderer);
        
        FigureEntity->J_GreenTexture   = GetTexture(Memory, "j_green.png", Buffer->Renderer);
        FigureEntity->J_BlueTexture    = GetTexture(Memory, "j_blue.png", Buffer->Renderer);
        FigureEntity->J_OrangeTexture  = GetTexture(Memory, "j_orange.png", Buffer->Renderer);
        FigureEntity->J_ClassicTexture = GetTexture(Memory, "j_red.png", Buffer->Renderer);
        FigureEntity->J_OutlineTexture = GetTexture(Memory, "j_outline.png", Buffer->Renderer);
        FigureEntity->J_ShadowTexture  = GetTexture(Memory, "j_shadow.png", Buffer->Renderer);
        
        FigureEntity->Z_GreenTexture   = GetTexture(Memory, "z_green.png", Buffer->Renderer);
        FigureEntity->Z_BlueTexture    = GetTexture(Memory, "z_blue.png", Buffer->Renderer);
        FigureEntity->Z_OrangeTexture  = GetTexture(Memory, "z_orange.png", Buffer->Renderer);
        FigureEntity->Z_ClassicTexture = GetTexture(Memory, "z_red.png", Buffer->Renderer);
        FigureEntity->Z_ShadowTexture = GetTexture(Memory, "z_shadow.png", Buffer->Renderer);
        FigureEntity->Z_OutlineTexture = GetTexture(Memory, "z_outline.png", Buffer->Renderer);
        
        FigureEntity->S_GreenTexture   = GetTexture(Memory, "s_green.png", Buffer->Renderer);
        FigureEntity->S_BlueTexture    = GetTexture(Memory, "s_blue.png", Buffer->Renderer);
        FigureEntity->S_OrangeTexture  = GetTexture(Memory, "s_orange.png", Buffer->Renderer);
        FigureEntity->S_ClassicTexture = GetTexture(Memory, "s_red.png", Buffer->Renderer);
        FigureEntity->S_ShadowTexture  = GetTexture(Memory, "s_shadow.png", Buffer->Renderer);
        FigureEntity->S_OutlineTexture  = GetTexture(Memory, "s_outline.png", Buffer->Renderer);
        
        FigureEntity->T_GreenTexture   = GetTexture(Memory, "t_green.png", Buffer->Renderer);
        FigureEntity->T_BlueTexture    = GetTexture(Memory, "t_blue.png", Buffer->Renderer);
        FigureEntity->T_OrangeTexture  = GetTexture(Memory, "t_orange.png", Buffer->Renderer);
        FigureEntity->T_ClassicTexture = GetTexture(Memory, "t_red.png", Buffer->Renderer);
        FigureEntity->T_ShadowTexture  = GetTexture(Memory, "t_shadow.png", Buffer->Renderer);
        FigureEntity->T_OutlineTexture  = GetTexture(Memory, "t_outline.png", Buffer->Renderer);
        
        /* NOTE(msokolov): grid_entity initialization starts here */
        grid_entity *GridEntity = &Playground->GridEntity;
        GridEntity->RowAmount           = 8;
        GridEntity->ColumnAmount        = 6;
        GridEntity->StickUnitsAmount    = FigureEntity->FigureAmount;
        GridEntity->MovingBlocksAmount  = 1;
        GridEntity->MovingBlockVelocity = Configuration->MovingBlockVelocity;
        
        GridEntity->GridArea.Min.x = 100;
        GridEntity->GridArea.Min.y = 81;
        GridEntity->GridArea.Max.x = GridEntity->GridArea.Min.x + 1128;
        GridEntity->GridArea.Max.y = GridEntity->GridArea.Min.y + 972;
        
        for (u32 Row = 0; Row < ROW_AMOUNT_MAXIMUM; ++Row)
        {
            for (u32 Col = 0; Col < COLUMN_AMOUNT_MAXIMUM; ++Col)
            {
                GridEntity->UnitField[(Row * COLUMN_AMOUNT_MAXIMUM) + Col] = 0;
            }
        }
        
        for (u32 i = 0; i < FIGURE_AMOUNT_MAXIMUM; ++i)
        {
            GridEntity->StickUnits[i].Index     = -1;
            GridEntity->StickUnits[i].IsSticked = false;
        }
        
        GridEntity->NormalSquareTexture     = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
        GridEntity->GridCell1Texture        = GetTexture(Memory, "grid_cell_new.png", Buffer->Renderer);
        GridEntity->GridCell2Texture        = GetTexture(Memory, "grid_cell_new2.png", Buffer->Renderer);
        
        GridEntity->VerticalSquareTexture   = GetTexture(Memory, "o_s.png", Buffer->Renderer);
        GridEntity->HorizontlaSquareTexture = GetTexture(Memory, "o_m.png", Buffer->Renderer);
        GridEntity->TopLeftCornerFrame      = GetTexture(Memory, "frame3.png", Buffer->Renderer);
        GridEntity->TopRightCornerFrame     = GetTexture(Memory, "frame4.png", Buffer->Renderer);
        GridEntity->DownLeftCornerFrame     = GetTexture(Memory, "frame2.png", Buffer->Renderer);
        GridEntity->DownRightCornerFrame    = GetTexture(Memory, "frame1.png", Buffer->Renderer);
        
        for (u32 BlockIndex = 0; 
             BlockIndex < GridEntity->MovingBlocksAmount;
             ++BlockIndex)
        {
            moving_block *Block = &GridEntity->MovingBlocks[BlockIndex];
            
            Block->Area.Min.x = GridEntity->GridArea.Min.x + (Block->ColNumber * GRID_BLOCK_SIZE);
            Block->Area.Min.y = GridEntity->GridArea.Min.y + (Block->RowNumber * GRID_BLOCK_SIZE);
            Block->Area.Max.x = Block->Area.Min.x + GRID_BLOCK_SIZE;
            Block->Area.Max.y = Block->Area.Min.y + GRID_BLOCK_SIZE;
            
            GridEntity->UnitField[(Block->RowNumber * COLUMN_AMOUNT_MAXIMUM) + Block->ColNumber] = 2;
        }
        
        /* player_data from binary initialization */
        player_data *PlayerData = (player_data *) Memory->SettingsStorage;
        Assert(PlayerData);
        
        // The first level should always be unlocked
        PlayerData->PlaygroundUnlocked[0] = true;
        GameState->PlayerData = PlayerData;
        
        game_settings *Settings = &PlayerData->Settings;
        
        /* playground data from binary initialization */
        playground_data *PlaygroundData = (playground_data *) Memory->LevelStorage;
        Assert(PlaygroundData);
        
        GameState->PlaygroundData = PlaygroundData;
        
        /* NOTE(msokolov): playground_menu initialization */
        playground_menu *PlaygroundMenu = &GameState->PlaygroundMenu;
        PlaygroundMenu->MenuPage = menu_page::MAIN_PAGE;
        PlaygroundMenu->DiffMode = difficulty::EASY;
        PlaygroundMenu->PlaygroundSwitch = false;
        PlaygroundMenu->ButtonIndex = 1;
        PlaygroundMenu->AnimationFinished = false;
        PlaygroundMenu->AnimationFinishedHalf = true;
        PlaygroundMenu->InterpPoint = 0.0f;
        PlaygroundMenu->InterpPointDiff = 1.0f;
        PlaygroundMenu->TimeMax = 0.2f;
        
        PlaygroundMenu->ForwardAnimation = false;
        PlaygroundMenu->InterpPointNext  = 1.0f;
        
        //game_settings *Settings = &PlayerData->Settings;
        PlaygroundMenu->SoundOn = Settings->SoundIsOn;
        PlaygroundMenu->MusicOn = Settings->MusicIsOn;
        
        PlaygroundMenu->BackgroundArea = {V2((r32)VIRTUAL_GAME_WIDTH * 0.5f, 0.0f), V2((r32) VIRTUAL_GAME_WIDTH * 0.5f, VIRTUAL_GAME_HEIGHT)};
        
        PlaygroundMenu->BackgroundTexture = GetTexture(Memory, "background_.png", Buffer->Renderer);
        PlaygroundMenu->LevelButtonTexture = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
        Assert(PlaygroundMenu->LevelButtonTexture);
        
        v2 Canvas = V2((8 * 150.0f) + 30.0f + 10.0f, 4 * 100.0f);
        v2 SizeRatio = V2(WidthRatio, HeightRatio);
        v2 NDC = V2(SizeRatio * Canvas);
        v2 RealSizeCanvas = V2(NDC.x * Buffer->ViewportWidth, NDC.y * Buffer->ViewportHeight);
        
        PlaygroundMenu->LevelsCanvasTexture = SDL_CreateTexture(Buffer->Renderer, SDL_PIXELFORMAT_RGBA8888,
                                                                SDL_TEXTUREACCESS_TARGET, RealSizeCanvas.w, RealSizeCanvas.h);
        PlaygroundMenu->DifficultyTexture[0] = MakeTextureFromString(Buffer, GameState->TimerFont, "I", V4(255.0f, 255.0f, 255.0f, 255.0f));
        PlaygroundMenu->DifficultyShadowTexture[0] = MakeTextureFromString(Buffer, GameState->TimerFont, "I", V4(0, 0, 0, 128));
        
        PlaygroundMenu->DifficultyTexture[1] = MakeTextureFromString(Buffer, GameState->TimerFont, "II", V4(255, 255, 255, 255));
        PlaygroundMenu->DifficultyShadowTexture[1] = MakeTextureFromString(Buffer, GameState->TimerFont, "II", V4(0, 0, 0, 128));
        
        PlaygroundMenu->DifficultyTexture[2] = MakeTextureFromString(Buffer, GameState->TimerFont, "III", V4(255, 255, 255, 255));
        PlaygroundMenu->DifficultyShadowTexture[2] = MakeTextureFromString(Buffer, GameState->TimerFont, "III", V4(0, 0, 0, 128));
        
        for(u32 Index = 0;
            Index < PLAYGROUND_MAXIMUM;
            ++Index) {
            
            char TimeString[64] = {};
            GetTimeString(TimeString, PlayerData->PlaygroundTime[Index]);
            
            GameState->PlaygroundMenu.LevelTimeTexture[Index] = MakeTextureFromString(Buffer, GameState->MenuTimerFont, TimeString, {255, 255, 255, 255});
            
            GameState->PlaygroundMenu.LevelTimeShadowTexture[Index] = MakeTextureFromString(Buffer, GameState->MenuTimerFont, TimeString, {0, 0, 0, 255});
        }
        
        PlaygroundMenu->NextLevelTexture = GetTexture(Memory, "next_level_indicator.png", Buffer->Renderer);
        PlaygroundMenu->NextLevelBackgroundTexture = GetTexture(Memory, "square_frame_unlocked_next.png", Buffer->Renderer);
        
        PlaygroundMenu->MainMenuTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Play", V4(255, 255, 255, 255));
        PlaygroundMenu->MainMenuTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "Settings", V4(255, 255, 255, 255));
        PlaygroundMenu->MainMenuTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "Quit", V4(255, 255, 255, 255));
        
        PlaygroundMenu->MainMenuShadowTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Play", V4(0, 0, 0, 128));
        PlaygroundMenu->MainMenuShadowTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "Settings", V4(0, 0, 0, 128));
        PlaygroundMenu->MainMenuShadowTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "Quit", V4(0, 0, 0, 128));
        
        PlaygroundMenu->CornerTexture[0] = GetTexture(Memory, "corner_left_top.png", Buffer->Renderer);
        PlaygroundMenu->CornerTexture[1] = GetTexture(Memory, "corner_left_bottom.png", Buffer->Renderer);
        PlaygroundMenu->CornerTexture[2] = GetTexture(Memory, "corner_right_top.png", Buffer->Renderer);
        PlaygroundMenu->CornerTexture[3] = GetTexture(Memory, "corner_right_bottom.png", Buffer->Renderer);
        PlaygroundMenu->HorizontalLineTexture = GetTexture(Memory, "horizontal_border_2.png", Buffer->Renderer);
        
        PlaygroundMenu->LevelCornerTexture[0] = GetTexture(Memory, "corner_menu_left_top.png", Buffer->Renderer);
        PlaygroundMenu->LevelCornerTexture[1] = GetTexture(Memory, "corner_menu_left_bottom.png", Buffer->Renderer);
        PlaygroundMenu->LevelCornerTexture[2] = GetTexture(Memory, "corner_menu_right_top.png", Buffer->Renderer);
        PlaygroundMenu->LevelCornerTexture[3] = GetTexture(Memory, "corner_menu_right_bottom.png", Buffer->Renderer);
        
        PlaygroundMenu->SquareFrameLocked   = GetTexture(Memory, "square_frame_locked.png", Buffer->Renderer);
        PlaygroundMenu->SquareFrameUnlocked = GetTexture(Memory, "square_frame_unlocked.png", Buffer->Renderer);
        
        PlaygroundMenu->ColorBarTexture[0] = GetTexture(Memory, "blue_bar.png", Buffer->Renderer);
        PlaygroundMenu->ColorBarTexture[1] = GetTexture(Memory, "green_bar.png", Buffer->Renderer);
        PlaygroundMenu->ColorBarTexture[2] = GetTexture(Memory, "orange_bar.png", Buffer->Renderer);
        PlaygroundMenu->ColorBarTexture[3] = GetTexture(Memory, "red_bar.png", Buffer->Renderer);
        
        //
        // This stuff needs to go away
        //
        
        PlaygroundMenu->ResolutionNameTexture = MakeTextureFromString(Buffer, GameState->Font, "Resolution: ", V4(255, 255, 255, 128));
        PlaygroundMenu->ResolutionNameShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Resolution: ", V4(0, 0, 0, 255));
        
        PlaygroundMenu->ResolutionTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "720p", V4(255, 255, 255, 255));
        PlaygroundMenu->ResolutionShadowTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "720p", V4(0, 0, 0, 128));
        
        PlaygroundMenu->ResolutionTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "1080p", V4(255, 255, 255, 255));
        PlaygroundMenu->ResolutionShadowTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "1080p", V4(0, 0, 0, 128));
        
        PlaygroundMenu->ResolutionTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "1440p", V4(255, 255, 255, 255));
        PlaygroundMenu->ResolutionShadowTexture[2] = MakeTextureFromString(Buffer, GameState->Font, "1440p", V4(0, 0, 0, 128));
        
        PlaygroundMenu->FullScreenNameTexture = MakeTextureFromString(Buffer, GameState->Font, "Fullscreen: ", V4(255, 255, 255, 255));
        PlaygroundMenu->FullScreenNameShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Fullscreen: ", V4(0, 0, 0, 128));
        
        PlaygroundMenu->FullScreenTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Off", V4(255, 255, 255, 255));
        PlaygroundMenu->FullScreenShadowTexture[0] = MakeTextureFromString(Buffer, GameState->Font, "Off", V4(0, 0, 0, 128));
        
        PlaygroundMenu->FullScreenTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "On", V4(255, 255, 255, 255));
        PlaygroundMenu->FullScreenShadowTexture[1] = MakeTextureFromString(Buffer, GameState->Font, "On", V4(0, 0, 0, 128));
        
        // 
        //
        //
        
        PlaygroundMenu->SoundNameTexture = MakeTextureFromString(Buffer, GameState->Font, "Sound: ", V4(255, 255, 255, 255));
        PlaygroundMenu->SoundNameShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Sound: ", V4(0, 0, 0, 255));
        
        PlaygroundMenu->MusicNameTexture = MakeTextureFromString(Buffer, GameState->Font, "Music: ", V4(255, 255, 255, 255));
        PlaygroundMenu->MusicNameShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Music: ", V4(0, 0, 0, 255));
        
        // For sound and music settings
        PlaygroundMenu->SoundOnTexture = MakeTextureFromString(Buffer, GameState->Font, "On", V4(255, 255, 255, 255));
        PlaygroundMenu->SoundOnShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "On", V4(0, 0, 0, 255));
        PlaygroundMenu->SoundOffTexture = MakeTextureFromString(Buffer, GameState->Font, "Off", V4(255, 255, 255, 255));
        PlaygroundMenu->SoundOffShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Off", V4(0, 0, 0, 255));
        
        PlaygroundMenu->BackTexture = MakeTextureFromString(Buffer, GameState->Font, "Back", V4(255, 255, 255, 255));
        PlaygroundMenu->BackShadowTexture = MakeTextureFromString(Buffer, GameState->Font, "Back", V4(0, 0, 0, 128));
        
        Memory->IsInitialized = true;
        printf("Memory has been initialized!\n");
        
#if DEBUG_BUILD
        
        GameState->EditorMode = false;
        
        playground_editor *PlaygroundEditor = PushStruct(&GameState->MemoryGroup, playground_editor);
        GameState->PlaygroundEditor = PlaygroundEditor;
        
        PlaygroundEditor->Font = TTF_OpenFont(FontPath, 50);
        Assert(PlaygroundEditor->Font);
        
        PlaygroundEditor->SelectedArea = selected_area::GRID_PLAYGROUND;
        PlaygroundEditor->FigureIndex  = 0;
        
        PlaygroundEditor->FigureFormTexture   = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Shape", {255, 255, 255, 255});
        PlaygroundEditor->FigureRotateTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Rotate", {255, 255, 255, 255});
        PlaygroundEditor->FigureAddTexture    = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Add", {255, 255, 255, 255});
        PlaygroundEditor->FigureDeleteTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Delete", {255, 255, 255, 255});
        
        PlaygroundEditor->FigureButtonsArea.Min.x = FigureEntity->FigureArea.Min.x - 400.0f; 
        PlaygroundEditor->FigureButtonsArea.Min.y = FigureEntity->FigureArea.Min.y + (GetDim(FigureEntity->FigureArea).h / 2.0f) - (180.0f / 2.0f); 
        SetDim(&PlaygroundEditor->FigureButtonsArea, 180, 240);
        
        PlaygroundEditor->GridRowTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Row", {255, 255, 255, 255});;
        PlaygroundEditor->GridColumnTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Column", {255, 255, 255, 255});;
        PlaygroundEditor->GridPlusTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "+", {255, 255, 255, 255});;
        PlaygroundEditor->GridMinusTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "-", {255, 255, 255, 255});;
        
        PlaygroundEditor->GridButtonsArea.Min.x = FigureEntity->FigureArea.Min.x - 200.0f; 
        PlaygroundEditor->GridButtonsArea.Min.y = FigureEntity->FigureArea.Max.y - (GetDim(FigureEntity->FigureArea).h / 2.0f) - (60.0f);
        SetDim(&PlaygroundEditor->GridButtonsArea, 300.0f, 120.0f);
        
        for (u32 Index = 0; Index < PLAYGROUND_MAXIMUM; ++ Index) {
            //PlayerData->PlaygroundUnlocked[Index] = true;
        }
        
        // NOTE(msokolov): temporary
        PlaygroundData[0].FigureAmount = 4;
        PlaygroundData[0].Figures[0].Form = figure_form::I_figure;
        PlaygroundData[0].Figures[0].Type = figure_type::Red;
        
        PlaygroundData[0].Figures[1].Form = figure_form::O_figure;
        PlaygroundData[0].Figures[1].Type = figure_type::Red;
        
        PlaygroundData[0].Figures[2].Form = figure_form::L_figure;
        PlaygroundData[0].Figures[2].Type = figure_type::Red;
        
        PlaygroundData[0].Figures[3].Form = figure_form::J_figure;
        PlaygroundData[0].Figures[3].Type = figure_type::Red;
        
        PlaygroundData[0].RowAmount    = 4;
        PlaygroundData[0].ColumnAmount = 4;
        
        PrepareNextPlayground(Playground, Configuration, PlaygroundData, 0);
        
        char LevelString[64] = {};
        sprintf(LevelString, "%d", Playground->LevelNumber);
        
        Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
        Assert(Playground->LevelNumberTexture);
        
        game_music *Music = GetMusic(Memory, "Jami Saber - Maenam.ogg");
        Mix_PlayMusic(Music, -1);
        
        if (!Settings->MusicIsOn)
            Mix_PauseMusic();
        
        if(!Settings->SoundIsOn)
            Mix_Volume(1, 0);
        
        PlaygroundEditor->IsInitialized = true;
#endif
        
    } // if (!Memory->IsInitialized)
    
    Assert(sizeof(transient_state) < Memory->TransientStorageSize);
    transient_state *TransState = (transient_state *)Memory->TransientStorage;
    if (!TransState->IsInitialized)
    {
        InitializeMemoryGroup(&TransState->TransGroup, Memory->TransientStorageSize - sizeof(transient_state), (u8*)Memory->TransientStorage + sizeof(transient_state));
        
        TransState->IsInitialized = true;
    }
    
    playground *Playground   = &GameState->Playground;
    playground_menu *PlaygroundMenu = &GameState->PlaygroundMenu;
    playground_data *PlaygroundData = GameState->PlaygroundData;
    
    player_data *PlayerData = GameState->PlayerData;
    game_settings *Settings = &PlayerData->Settings;
    
#if DEBUG_BUILD
    if(Input->Keyboard.BackQuote.EndedDown)
    {
        RestartLevelEntity(Playground);
        
        if (!GameState->EditorMode)
        {
            GameState->EditorMode   = true;
            Playground->LevelPaused = true;
        }
        else
        {
            GameState->EditorMode   = false;
            Playground->LevelPaused = false;
        }
    }
    
    if (GameState->EditorMode)
    {
        if(Input->Keyboard.E_Button.EndedDown)
        {
            u32 PlaygroundIndex = GameState->PlaygroundIndex;
            if (PlaygroundIndex < PLAYGROUND_MAXIMUM)
            {
                RestartLevelEntity(Playground);
                PlaygroundIndex++;
                
                PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, PlaygroundIndex);
                
                char LevelString[64] = {};
                sprintf(LevelString, "%d", Playground->LevelNumber);
                if (Playground->LevelNumberTexture)
                {
                    SDL_DestroyTexture(Playground->LevelNumberTexture);
                }
                
                Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255.0f, 255.0f, 255.0f, 255.0f});
                Assert(Playground->LevelNumberTexture);
            }
            
            GameState->PlaygroundEditor->FigureIndex = 0;
            GameState->PlaygroundIndex = PlaygroundIndex;
        }
        
        if (Input->Keyboard.Q_Button.EndedDown)
        {
            u32 PlaygroundIndex = GameState->PlaygroundIndex;
            if (PlaygroundIndex > 0)
            {
                RestartLevelEntity(Playground);
                
                PlaygroundIndex--;
                
                PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, PlaygroundIndex);
                
                char LevelString[64] = {};
                sprintf(LevelString, "%d", Playground->LevelNumber);
                if (Playground->LevelNumberTexture)
                {
                    SDL_DestroyTexture(Playground->LevelNumberTexture);
                }
                
                Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255.0f, 255.0f, 255.0f, 255.0f});
                Assert(Playground->LevelNumberTexture);
            }
            
            GameState->PlaygroundEditor->FigureIndex = 0;
            GameState->PlaygroundIndex = PlaygroundIndex;
        }
        
        if(Input->Keyboard.S_Button.EndedDown)
        {
            WritePlaygroundData(PlaygroundData, Playground, GameState->PlaygroundIndex);
            WriteLevelsToFile(Memory->LevelStorage, Memory->LevelStorageSize);
            printf("saved.\n");
        }
    }
    
#endif
    
    /*
    NOTE(msokolov): Render 
    */
    
    memory_group TemporaryMemory = TransState->TransGroup;
    render_group *RenderGroup = AllocateRenderGroup(&TransState->TransGroup, Kilobytes(100), Buffer->Width, Buffer->Height);
    
    game_mode CurrentMode = GameState->CurrentMode;
    switch (CurrentMode)
    {
        case PLAYGROUND:
        {
            playground_status PlaygroundStatus = PlaygroundUpdateAndRender(Playground, RenderGroup, Input);
            
            if (PlaygroundStatus == playground_status::LEVEL_RUNNING)
            {
                /* Updating the timer UI */ 
                // Don't update the timer once a level is finished and we're waiting for the animation to finish
                if (!Playground->LevelFinished)
                    Playground->TimeElapsed += Input->dtForFrame;
                
                if (Playground->ShowTimer)
                {
                    if (!Playground->LevelFinished) {
                        char TimeString[64] = {};
                        GetTimeString(TimeString, Playground->TimeElapsed);
                        
                        if (Playground->TimerShadowTexture)
                            SDL_DestroyTexture(Playground->TimerShadowTexture);
                        
                        if (Playground->TimerTexture)
                            SDL_DestroyTexture(Playground->TimerTexture);
                        
                        Playground->TimerShadowTexture = MakeTextureFromString(Buffer, GameState->TimerFont, TimeString, V4(0.0f, 0.0f, 0.0f, 128.0f));
                        Playground->TimerTexture       = MakeTextureFromString(Buffer, GameState->TimerFont, TimeString, V4(255.0f, 255.0f, 255.0f, 255.0f));
                    }
                    else {
                        r32 AlphaChannel = Lerp1(255.0f, 0.0f, Playground->Animation.InterpPoint);
                        SDL_SetTextureAlphaMod(Playground->TimerTexture, AlphaChannel);
                        SDL_SetTextureAlphaMod(Playground->TimerShadowTexture, AlphaChannel);
                    }
                    
                    rectangle2 TimerRectangle = {};
                    TimerRectangle.Min.x = (VIRTUAL_GAME_WIDTH * 0.5f);
                    TimerRectangle.Min.y = 50.0f;
                    SetDim(&TimerRectangle, 0.0f, 0.0f);
                    
                    TimerRectangle = GetTextOnTheCenterOfRectangle(TimerRectangle, Playground->TimerTexture);
                    
                    TimerRectangle.Min += V2(5.0f, 5.0f);
                    TimerRectangle.Max += V2(5.0f, 5.0f);
                    PushFontBitmap(RenderGroup, Playground->TimerShadowTexture, TimerRectangle);
                    
                    TimerRectangle.Min -= V2(5.0f, 5.0f);
                    TimerRectangle.Max -= V2(5.0f, 5.0f);
                    PushFontBitmap(RenderGroup, Playground->TimerTexture, TimerRectangle);
                }
            }
            else if (PlaygroundStatus == playground_status::LEVEL_FINISHED)
            {
                if (GameState->PlaygroundIndex < PLAYGROUND_MAXIMUM)
                {
                    Result.SettingsChanged = true;
                    
                    /* Check if this is a record */
                    u32 PlaygroundIndex = GameState->PlaygroundIndex;
                    if (Playground->TimeElapsed < PlayerData->PlaygroundTime[PlaygroundIndex]
                        || PlayerData->PlaygroundTime[PlaygroundIndex] == 0.0f)
                    {
                        PlayerData->PlaygroundTime[PlaygroundIndex] = Playground->TimeElapsed;
                        RestartLevelEntity(Playground);
                        
                        char TimeString[64] = {};
                        GetTimeString(TimeString, PlayerData->PlaygroundTime[PlaygroundIndex]);
                        
                        PlaygroundMenu->LevelTimeTexture[PlaygroundIndex] = MakeTextureFromString(Buffer, GameState->MenuTimerFont, TimeString, {255.0f, 255.0f, 255.0f, 255.0f});
                        
                        PlaygroundMenu->LevelTimeShadowTexture[PlaygroundIndex] = MakeTextureFromString(Buffer, GameState->MenuTimerFont, TimeString, {0.0f, 0.0f, 0.0f, 255.0f});
                    }
                    
                    if (PlaygroundIndex < PLAYGROUND_MAXIMUM)
                    {
                        GameState->PlaygroundIndex = ++PlaygroundIndex;
                        PlayerData->PlaygroundUnlocked[PlaygroundIndex] = true;
                        
                        if (PlaygroundIndex % 8 == 0) {
                            u32 NextUnlockIndex = PlaygroundIndex - 8 + 32;
                            if (NextUnlockIndex < PLAYGROUND_MAXIMUM) {
                                PlayerData->PlaygroundUnlocked[NextUnlockIndex] = true;
                            }
                        }
                        
                        PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, GameState->PlaygroundIndex);
                        
                        RestartLevelEntity(Playground);
#if DEBUG_BUILD
                        char LevelString[64] = {};
                        sprintf(LevelString, "%d", Playground->LevelNumber);
                        if (Playground->LevelNumberTexture)
                        {
                            SDL_DestroyTexture(Playground->LevelNumberTexture);
                        }
                        
                        Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                        Assert(Playground->LevelNumberTexture);
#endif
                    }
                    
                }
            }
            else if (PlaygroundStatus == playground_status::LEVEL_MENU_QUIT)
            {
                GameState->CurrentMode = MENU;
                PlaygroundMenu->MenuPage = menu_page::DIFFICULTY_PAGE;
                PlaygroundMenu->AnimationFinished = false;
                PlaygroundMenu->InterpPoint       = 0.0f;
                PlaygroundMenu->ForwardAnimation  = false;
                PlaygroundMenu->InterpPointNext   = 1.0f;
                
                UpdateTextureForLevels(PlaygroundMenu, PlayerData, RenderGroup);
                SDL_ShowCursor(SDL_ENABLE);
                
#if DEBUG_BUILD
                GameState->EditorMode = false;
                GameState->PlaygroundEditor->FigureIndex = 0;
#endif
            }
            else if (PlaygroundStatus == playground_status::LEVEL_GAME_QUIT)
            {
                Result.ShouldQuit = true;
            }
            else if (PlaygroundStatus == playground_status::LEVEL_SETTINGS_QUIT)
            {
                GameState->CurrentMode = MENU;
                PlaygroundMenu->MenuPage = menu_page::SETTINGS_PAGE;
                PlaygroundMenu->PlaygroundSwitch = true;
            }
            else if (PlaygroundStatus == playground_status::LEVEL_RESTARTED)
            {
                RestartLevelEntity(Playground);
            }
            
#if DEBUG_BUILD
            if (GameState->EditorMode)
            {
                PlaygroundEditorUpdateAndRender(Playground, PlaygroundData, &GameState->Configuration, GameState->PlaygroundEditor, RenderGroup, Input);
            }
#endif
            
        } break;
        
        case MENU:
        {
            menu_result_option MenuResult = PlaygroundMenuUpdateAndRender(PlaygroundMenu, PlayerData, Settings, Input, RenderGroup);
            
            if (MenuResult.SwitchToPlayground)
            {
                if (PlaygroundMenu->PlaygroundSwitch)
                {
                    PlaygroundMenu->PlaygroundSwitch = false;
                }
                else
                {
                    u32 ResultLevelIndex = MenuResult.PlaygroundIndex;
                    RestartLevelEntity(Playground);
                    GameState->PlaygroundIndex = ResultLevelIndex;
                    
                    PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, ResultLevelIndex);
                    
#if DEBUG_BUILD
                    char LevelString[64] = {};
                    sprintf(LevelString, "%d", Playground->LevelNumber);
                    if (Playground->LevelNumberTexture)
                    {
                        SDL_DestroyTexture(Playground->LevelNumberTexture);
                    }
                    
                    Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                    Assert(Playground->LevelNumberTexture);
#endif
                }
                
                GameState->CurrentMode = PLAYGROUND;
            }
            if (MenuResult.QuitGame)
            {
                Result.ShouldQuit = true;
            }
            if (MenuResult.SettingsChanged)
            {
                Result.SettingsChanged = true;
            }
        } break;
    }
    
    RenderGroupToOutput(RenderGroup, Buffer);
    TransState->TransGroup = TemporaryMemory;
    
    return(Result);
}
