/* asset_game.cpp --- 
 * 
 * Filename: asset_game.h
 * Author: Sierra
 * Created: Пн окт 16 10:08:17 2017 (+0300)
 * Last-Updated: Пт окт 27 10:15:58 2017 (+0300)
 *           By: Sierra
 */

static u64
SDLSizeOfSDL_RWops(SDL_RWops *&BinaryFile)
{
    if(!BinaryFile) return 0;
    
    u32 DefaultOffset = SDL_RWtell(BinaryFile);
    SDL_RWseek(BinaryFile, 0, RW_SEEK_END);
    u32 ByteSize = SDL_RWtell(BinaryFile);
    SDL_RWseek(BinaryFile, DefaultOffset, RW_SEEK_SET);
    return(ByteSize);
}

static u64
SDLSizeOfBinaryFile(const char *FileName)
{
    SDL_RWops *File = SDL_RWFromFile(FileName, "rb");
    u64 ByteSize = SDLSizeOfSDL_RWops(File);
    return(ByteSize);
}

#if 0
static void
SDLWriteGameSoundToFile(SDL_RWops *&BinaryTarget, const char* SourceFile)
{
    u64 ByteSize  = 0;
    void *Samples = 0;
    SDL_RWops *BinarySource = 0;
    
    BinarySource = SDL_RWFromFile(SourceFile, "rb");
    Assert(BinarySource);
    
    ByteSize = SDLSizeOfSDL_RWops(BinarySource);
    
    Samples = malloc(ByteSize);
    Assert(Samples);
    SDL_RWread(BinarySource, Samples, ByteSize, 1);
    SDL_RWclose(BinarySource);
    
    SDL_RWwrite(BinaryTarget, &ByteSize, sizeof(u64), 1);
    SDL_RWwrite(BinaryTarget, Samples, ByteSize, 1);
    free(Samples);
}

static void
SDLWriteGameBitmapToFile(SDL_RWops *&BinaryFile, const char * FileName)
{
    SDL_Surface *Surface = IMG_Load(FileName);
    Assert(Surface);
    
    asset_bitmap BitmapInfo = {};
    BitmapInfo.Width         = Surface->w;
    BitmapInfo.Height        = Surface->h;
    BitmapInfo.Pitch         = Surface->pitch;
    BitmapInfo.Rmask         = Surface->format->Rmask;
    BitmapInfo.Gmask         = Surface->format->Gmask;
    BitmapInfo.Bmask         = Surface->format->Bmask;
    BitmapInfo.Amask         = Surface->format->Amask;
    BitmapInfo.BytesPerPixel = Surface->format->BytesPerPixel;
    BitmapInfo.BitsPerPixel  = Surface->format->BitsPerPixel;
    
    SDL_RWwrite(BinaryFile, &BitmapInfo,	sizeof(asset_memory_bitmap), 1);
    
    SDL_RWwrite(BinaryFile, Surface->pixels,
                BitmapInfo.BytesPerPixel * BitmapInfo.Width * BitmapInfo.Height, 1);
    
    SDL_FreeSurface(Surface);
}

static void
SDLLoadGameSound(SDL_RWops *&BinaryFile, game_sound *&Sound, s64 *ByteCount)
{
    // NOTE(Max): For some reason if you load big amount of bytes,
    // Mix_LoadWAV_RW function sets your SDL_RWops cursor to zero
    
    u64 Offset = SDL_RWtell(BinaryFile);
    u64 ByteSize = 0;
    SDL_RWread(BinaryFile, &ByteSize, sizeof(u64), 1);
    
    Sound = Mix_LoadWAV_RW(BinaryFile, 0);
    Assert(Sound);
    
    Offset = Offset + ByteSize + sizeof(u64);
    SDL_RWseek(BinaryFile, Offset, RW_SEEK_SET);
    
    *ByteCount = Offset;
}


static void
SDLLoadGameBitmap(SDL_RWops *&BinaryFile, SDL_Renderer *&Renderer, game_texture *&GameBitmap,
                  s64 *ByteCount)
{
    SDL_Surface *TempSurface = 0;
    asset_memory_bitmap BitmapInfo = {};
    
    SDL_RWread(BinaryFile, &BitmapInfo, sizeof(asset_memory_bitmap), 1);
    
    void *Pixels = malloc(BitmapInfo.BytesPerPixel * BitmapInfo.Width * BitmapInfo.Height);
    Assert(Pixels);
    
    SDL_RWread(BinaryFile, Pixels,
               BitmapInfo.BytesPerPixel * BitmapInfo.Width * BitmapInfo.Height, 1);
    
    TempSurface =
        SDL_CreateRGBSurfaceFrom(Pixels, BitmapInfo.Width, BitmapInfo.Height, BitmapInfo.BitsPerPixel, BitmapInfo.Pitch,
                                 BitmapInfo.Rmask, BitmapInfo.Gmask, BitmapInfo.Bmask, BitmapInfo.Amask);
    Assert(TempSurface);
    
    GameBitmap = SDL_CreateTextureFromSurface(Renderer, TempSurface);
    Assert(GameBitmap);
    
    free(Pixels);
    SDL_FreeSurface(TempSurface);
    
    *ByteCount = SDL_RWtell(BinaryFile);
}

static void
SDLLoadGameMusicFromMemory(void *&Memory, game_music *&Music, s64* ByteCount)
{
    u64 *ByteSize = (u64*)Memory;
    Memory = ((u8*) Memory) + sizeof(u64);
    *ByteCount += sizeof(u64);
    
    SDL_RWops *rw = SDL_RWFromConstMem(Memory, *ByteSize);
    Music  = Mix_LoadMUS_RW(rw, 0);
    Assert(Music);
    SDL_RWclose(rw);
    
    Memory = ((u8*) Memory) + *ByteSize;
    *ByteCount += *ByteSize;
}

static void
SDLLoadGameSoundFromMemory(void *&Memory, game_sound *&Sound, s64* ByteCount)
{
    u64 *ByteSize = (u64*)Memory;
    Memory = ((u8*) Memory) + sizeof(u64);
    *ByteCount += sizeof(u64);
    
    u8* SoundBuffer = (u8*)Memory;
    Sound = Mix_QuickLoad_WAV(SoundBuffer);
    Assert(Sound);
    
    Memory = ((u8*) Memory) + *ByteSize;
    *ByteCount += *ByteSize;
}

static void
SDLLoadBitmapFromMemory(void *&Memory, game_texture *& Texture, s64 *ByteOffset,
                        SDL_Renderer *&Renderer)
{
    Assert(Memory);
    SDL_Surface *TempSurface = 0;
    asset_memory_bitmap *BitmapInfo = (asset_memory_bitmap*)Memory;
    
    u64 BytePerSurface =
        BitmapInfo->BytesPerPixel * BitmapInfo->Width * BitmapInfo->Height;
    
    Memory = ((u8*) Memory) + sizeof(asset_memory_bitmap);
    *ByteOffset += sizeof(asset_memory_bitmap);
    
    TempSurface =
        SDL_CreateRGBSurfaceFrom(Memory, BitmapInfo->Width, BitmapInfo->Height,
                                 BitmapInfo->BitsPerPixel, BitmapInfo->Pitch,
                                 BitmapInfo->Rmask, BitmapInfo->Gmask,
                                 BitmapInfo->Bmask, BitmapInfo->Amask);
    Assert(TempSurface);
    
    Memory = ((u8*) Memory) + BytePerSurface;
    *ByteOffset += BytePerSurface;
    
    Texture = SDL_CreateTextureFromSurface(Renderer, TempSurface);
    Assert(Texture);
    
    SDL_FreeSurface(TempSurface);
}
#endif


static void
SDLReadEntireAssetFile(const char* FileName, game_memory *&Memory)
{
    SDL_RWops *BinaryFile = SDL_RWFromFile(FileName, "rb");
    Memory->AssetsSpaceAmount = SDLSizeOfSDL_RWops(BinaryFile);
    // printf("filesize = %llu\n", Memory->AssetsSpace);
    
    Memory->AssetStorage = malloc(Memory->AssetsSpaceAmount);
    Assert(Memory->AssetStorage);
    
    SDL_RWread(BinaryFile, Memory->AssetStorage, Memory->AssetsSpaceAmount, 1);
    SDL_RWclose(BinaryFile);
}

static void
SDLReadEntireLevelFile(char* FileName, game_memory *&Memory)
{
    
}


static void
SDLWriteBitmapToFile(SDL_RWops *&BinaryFile, const char* FileName)
{
    char FullName[128];
    strcpy(FullName, SpritePath);
    strcat(FullName, FileName);
    
    printf("FileName = %s\n", FileName);
    printf("FullName = %s\n", FullName);
    
    SDL_Surface *Surface = IMG_Load(FullName);
    printf("error:\n", IMG_GetError());
    Assert(Surface);
    
    asset_bitmap_header BitmapHeader;
    BitmapHeader.Width   = Surface->w;
    BitmapHeader.Height  = Surface->h;
    BitmapHeader.Pitch   = Surface->pitch;
    BitmapHeader.Rmask   = Surface->format->Rmask;
    BitmapHeader.Gmask   = Surface->format->Gmask;
    BitmapHeader.Bmask   = Surface->format->Bmask;
    BitmapHeader.Amask   = Surface->format->Amask;
    BitmapHeader.BytesPerPixel = Surface->format->BytesPerPixel;
    BitmapHeader.BitsPerPixel  = Surface->format->BitsPerPixel;
    
    u32 ByteAmount = BitmapHeader.Width * BitmapHeader.Height * BitmapHeader.BytesPerPixel;
    
    asset_header AssetHeader = {};
    AssetHeader.AssetSize = ByteAmount;
    AssetHeader.AssetType = AssetType_Bitmap;
    strcpy(AssetHeader.AssetName, FileName);
    
    AssetHeader.Bitmap.Data = 0;
    AssetHeader.Bitmap.Header = BitmapHeader;
    // printf("AssetSize = %u\n", AssetHeader.AssetSize);
    
    SDL_RWwrite(BinaryFile, &AssetHeader, sizeof(asset_header), 1);
    SDL_RWwrite(BinaryFile, Surface->pixels, AssetHeader.AssetSize, 1);
    
    SDL_FreeSurface(Surface);
}

static void
SDLWriteSoundToFile(SDL_RWops *&BinaryFile, const char *FileName)
{
    char FullName[128];
    strcpy(FullName, SoundPath);
    strcat(FullName, FileName);
    
    SDL_RWops *SoundFile = SDL_RWFromFile(FullName, "rb");
    Assert(SoundFile);
    
    asset_header AssetHeader;
    AssetHeader.AssetSize = SDLSizeOfSDL_RWops(SoundFile);
    AssetHeader.AssetType = AssetType_Sound;
    strcpy(AssetHeader.AssetName, FileName);
    AssetHeader.Audio.Header.IsMusic = false;
    
    void *Memory = malloc(AssetHeader.AssetSize);
    Assert(Memory);
    
    SDL_RWread(SoundFile, Memory, AssetHeader.AssetSize, 1);
    
    SDL_RWwrite(BinaryFile, &AssetHeader, sizeof(asset_header), 1);
    SDL_RWwrite(BinaryFile, Memory, AssetHeader.AssetSize, 1);
    
    free(Memory);
    SDL_RWclose(SoundFile);
}

static void
SDLWriteMusicToFile(SDL_RWops *&BinaryFile, const char *FileName)
{
    char FullName[128];
    strcpy(FullName, SoundPath);
    strcat(FullName, FileName);
    
    SDL_RWops *MusicFile = SDL_RWFromFile(FullName, "rb");
    Assert(MusicFile);
    
    asset_header AssetHeader = {};
    AssetHeader.AssetSize = SDLSizeOfSDL_RWops(MusicFile);
    AssetHeader.AssetType = AssetType_Music;
    strcpy(AssetHeader.AssetName, FileName);
    AssetHeader.Audio.Header.IsMusic = true;
    
    void *Memory = malloc(AssetHeader.AssetSize);
    Assert(Memory);
    
    SDL_RWread(MusicFile, Memory, AssetHeader.AssetSize, 1);
    
    SDL_RWwrite(BinaryFile, &AssetHeader, sizeof(asset_header), 1);
    SDL_RWwrite(BinaryFile, Memory, AssetHeader.AssetSize, 1);
    
    free(Memory);
    SDL_RWclose(MusicFile);
}

static bool
IsAsset(asset_header*& AssetHeader, asset_type AssetType, const char* AssetName)
{
    bool Result = false;
    
    if(AssetHeader->AssetType == AssetType)
    {
        if(strcmp(AssetHeader->AssetName, AssetName) == 0)
        {
            Result = true;
        }
    }
    
    return(Result);
}

static asset_header*
GetAssetHeader(game_memory *&Memory, asset_type AssetType, const char* AssetName, u32 Offset)
{
    u8 *mem = (u8*)Memory->AssetStorage + Offset;
    asset_header *AssetHeader = (asset_header*)mem;
    u32 TotalByteSize = 0;
    
    while(TotalByteSize < Memory->AssetsSpaceAmount)
    {
        if(IsAsset(AssetHeader, AssetType, AssetName))
        {
            return AssetHeader;
        }
        else
        {
            TotalByteSize += (sizeof(asset_header) + AssetHeader->AssetSize);
            AssetHeader = ((asset_header*)(((u8*)AssetHeader) + sizeof(asset_header) + AssetHeader->AssetSize));
        }
    }
    
    return(NULL);
}

static game_music*
GetMusic(game_memory *Memory, char* FileName)
{
    game_music *Music = NULL;
    
    binary_header *BinaryHeader = (binary_header*)Memory->AssetStorage;
    u32 ByteOffset = sizeof(binary_header) + BinaryHeader->AudioSizeInBytes;
    
    asset_header *AssetHeader = GetAssetHeader(Memory, AssetType_Music, FileName, ByteOffset);
    if(AssetHeader)
    {
        asset_audio *Audio = &AssetHeader->Audio;
        asset_audio_header *Header = &Audio->Header;
        
        Audio->Data = (u8*)AssetHeader;
        Audio->Data = Audio->Data + sizeof(asset_header);
        
        void *MusicData = ((void*)Audio->Data);
        SDL_RWops *RWMusic = SDL_RWFromConstMem(MusicData, AssetHeader->AssetSize);
        Music = Mix_LoadMUS_RW(RWMusic, 1);
        Assert(Music);
    }
    
    return(Music);
}

static game_sound*
GetSound(game_memory *Memory, char* FileName)
{
    game_sound *Sound = NULL;
    
    binary_header *BinaryHeader = (binary_header*)Memory->AssetStorage;
    u32 ByteOffset = sizeof(binary_header) + BinaryHeader->AudioSizeInBytes;
    
    asset_header *AssetHeader = GetAssetHeader(Memory, AssetType_Sound, FileName, ByteOffset);
    if(AssetHeader)
    {
        asset_audio *Audio = &AssetHeader->Audio;
        asset_audio_header *Header = &Audio->Header;
        
        Audio->Data = (u8*)AssetHeader;
        Audio->Data = Audio->Data + sizeof(asset_header);
        
        Sound = Mix_QuickLoad_WAV(Audio->Data);
        Assert(Sound);
    }
    
    return(Sound);
}

static game_texture*
GetTexture(game_memory *&Memory, const char* FileName, SDL_Renderer *&Renderer)
{
    game_texture *Texture = NULL;
    
    binary_header *BinaryHeader = (binary_header*)Memory->AssetStorage;
    u32 ByteOffset = sizeof(binary_header) + BinaryHeader->BitmapSizeInBytes;
    
    asset_header *AssetHeader = GetAssetHeader(Memory, AssetType_Bitmap, FileName, ByteOffset);
    if(AssetHeader)
    {
        asset_bitmap *Bitmap = &AssetHeader->Bitmap;
        asset_bitmap_header *Header = &Bitmap->Header;
        
        Bitmap->Data = (void*)AssetHeader;
        Bitmap->Data = ((u8*)Bitmap->Data) + sizeof(asset_header);
        game_surface *Surface =
            SDL_CreateRGBSurfaceFrom(Bitmap->Data,
                                     Header->Width, Header->Height,
                                     Header->BitsPerPixel, Header->Pitch,
                                     Header->Rmask, Header->Gmask,
                                     Header->Bmask, Header->Amask);
        Assert(Surface);
        
        Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
        Assert(Texture);
        
        SDL_FreeSurface(Surface);
    }
    
    return(Texture);
}

static void
ConvertLevelMemoryFromRaw(game_memory *&Memory, void *&RawMemory, u32 RawMemorySpace)
{
    
    u32 TotalBytesRead = 0;
    level_memory *Level = NULL;
    u8 *U8Mem = (u8*)RawMemory;
    Level = (level_memory*)U8Mem;
    
    level_memory *LevelMemory = (level_memory *)Memory->GlobalMemoryStorage;
    
    while(TotalBytesRead < RawMemorySpace)
    {
        u32 Index = Memory->LevelMemoryAmount;
        u32 BytesToSkip = 0;
        
        LevelMemory[Index].IsLocked           = Level->IsLocked;
        LevelMemory[Index].LevelNumber        = Level->LevelNumber;
        LevelMemory[Index].RowAmount          = Level->RowAmount;
        LevelMemory[Index].ColumnAmount       = Level->ColumnAmount;
        LevelMemory[Index].MovingBlocksAmount = Level->MovingBlocksAmount;
        LevelMemory[Index].FigureAmount       = Level->FigureAmount;
        
        BytesToSkip += sizeof(level_memory);
        
        if(Level->RowAmount > 0 && Level->ColumnAmount > 0)
        { 
            s32 *UnitField = ((s32*)((U8Mem) + BytesToSkip));
            
            LevelMemory[Index].UnitField = (s32 *)malloc(Level->ColumnAmount * Level->RowAmount * sizeof(s32));
            Assert(LevelMemory[Index].UnitField);
            
            for(u32 i = 0; i < Level->RowAmount * Level->ColumnAmount; ++i)
            {
                LevelMemory[Index].UnitField[i] = UnitField[i];
            }
            
            BytesToSkip += Level->RowAmount * Level->ColumnAmount * sizeof(s32);
        }
        
        if(Level->MovingBlocksAmount > 0)
        {
            moving_block_memory *MovingBlocks = ((moving_block_memory*)((U8Mem) + BytesToSkip));
            
            LevelMemory[Index].MovingBlocks = (moving_block_memory*) malloc(sizeof(moving_block_memory) * Level->MovingBlocksAmount);
            Assert(LevelMemory[Index].MovingBlocks);
            
            for(u32 i = 0; i < Level->MovingBlocksAmount; ++i)
            {
                LevelMemory[Index].MovingBlocks[i].RowNumber = MovingBlocks->RowNumber;
                
                LevelMemory[Index].MovingBlocks[i].ColNumber = MovingBlocks->ColNumber;
                
                LevelMemory[Index].MovingBlocks[i].IsVertical = MovingBlocks->IsVertical;
                
                LevelMemory[Index].MovingBlocks[i].MoveSwitch = MovingBlocks->MoveSwitch;
                
                BytesToSkip += sizeof(moving_block_memory);
                MovingBlocks = ((moving_block_memory*)((U8Mem) + BytesToSkip));
            }
            
        }
        
        if(Level->FigureAmount > 0)
        {
            figure_memory *FigureMemory = ((figure_memory*)((U8Mem) + BytesToSkip));
            
            LevelMemory[Index].Figures = (figure_memory*) malloc(sizeof(figure_memory) * Level->FigureAmount);
            Assert(LevelMemory[Index].Figures);
            
            for(u32 i = 0; i < Level->FigureAmount; ++i)
            {
                LevelMemory[Index].Figures[i].Angle = FigureMemory->Angle;
                LevelMemory[Index].Figures[i].Flip  = FigureMemory->Flip;
                LevelMemory[Index].Figures[i].Form  = FigureMemory->Form;
                LevelMemory[Index].Figures[i].Type  = FigureMemory->Type;
                
                BytesToSkip += sizeof(figure_memory);
                FigureMemory = ((figure_memory*)((U8Mem) + BytesToSkip));
            }
        }
        
        Memory->LevelMemoryAmount += 1;
        TotalBytesRead += BytesToSkip;
        Level = ((level_memory*)(((u8*)Level) + BytesToSkip));
        U8Mem = U8Mem + BytesToSkip;
    }
    
    printf("READ %d LEVELS FROM BINARY!!!!!!!!\n",Memory->LevelMemoryAmount);
    printf("LevelMemory[0].IsLocked = %d\n", LevelMemory[0].IsLocked);
}

static void
LoadLevelMemoryFromFile(const char* FileName, game_memory *Memory)
{
    if(Memory->GlobalMemoryStorage)
    {
        free(Memory->GlobalMemoryStorage);
    }
    
    Memory->LevelMemoryAmount   = 0;
    Memory->LevelMemoryReserved = 100;
    
    level_memory *LevelMemory = (level_memory *)calloc(sizeof(level_memory), Memory->LevelMemoryReserved);
    Memory->GlobalMemoryStorage = LevelMemory;
    Assert(LevelMemory);
    
    SDL_RWops *BinaryFile = SDL_RWFromFile(FileName, "rb");
    u32 ByteAmount = SDLSizeOfSDL_RWops(BinaryFile);
    if(!(BinaryFile) || ByteAmount == 0)
    {
        printf("Failed to find package2.bin or it is empty!\n");
    }
    else
    {
        // TODO(Max): why do we do that 2 times???
        u32 RawMemorySpace = SDLSizeOfSDL_RWops(BinaryFile);
        void *RawMemory = malloc(RawMemorySpace);
        Assert(RawMemory);
        
        SDL_RWread(BinaryFile, RawMemory, RawMemorySpace, 1);
        
        ConvertLevelMemoryFromRaw(Memory, RawMemory, RawMemorySpace);
        
        SDL_RWclose(BinaryFile);
    }
}

static void
SaveLevelMemoryToFile(game_memory *Memory)
{
    SDL_RWops *BinaryFile = SDL_RWFromFile("package2.bin", "wb");
    
    level_memory *LevelMemory = (level_memory *)Memory->GlobalMemoryStorage;
    
    for(u32 i = 0; i < Memory->LevelMemoryAmount; ++i)
    {
        SDL_RWwrite(BinaryFile, &LevelMemory[i], sizeof(level_memory), 1);
        SDL_RWwrite(BinaryFile, LevelMemory[i].UnitField, sizeof(s32) *  LevelMemory[i].RowAmount * LevelMemory[i].ColumnAmount, 1);
        
        SDL_RWwrite(BinaryFile, LevelMemory[i].MovingBlocks, sizeof(moving_block_memory), LevelMemory[i].MovingBlocksAmount);
        
        SDL_RWwrite(BinaryFile, LevelMemory[i].Figures, sizeof(figure_memory), LevelMemory[i].FigureAmount);
    }
    
    SDL_RWclose(BinaryFile);
}

static void 
SaveLevelToMemory(game_memory *Memory, level_entity* LevelEntity, u32 Index)
{
    if(Index < 0 || Index >= Memory->LevelMemoryAmount) return;
    
    level_memory *LevelMemory = (level_memory *)Memory->GlobalMemoryStorage;
    
    LevelMemory[Index].LevelNumber = LevelEntity->LevelNumber;
    
    if(LevelMemory[Index].RowAmount > 0)
    {
        free(LevelMemory[Index].UnitField);
    }
    
    LevelMemory[Index].RowAmount        = LevelEntity->GridEntity->RowAmount;
    LevelMemory[Index].ColumnAmount     = LevelEntity->GridEntity->ColumnAmount;
    
    if(LevelMemory[Index].RowAmount > 0)
    {
        LevelMemory[Index].UnitField = (s32*)malloc(sizeof(s32) * LevelMemory[Index].RowAmount * LevelMemory[Index].ColumnAmount);
        Assert(LevelMemory[Index].UnitField);
        
        for(u32 Row = 0; Row < LevelMemory[Index].RowAmount; ++Row)
        {
            for(u32 Col = 0; Col < LevelMemory[Index].ColumnAmount; ++Col)
            {
                s32 UnitIndex = (Row * LevelMemory[Index].ColumnAmount) + Col;
                LevelMemory[Index].UnitField[UnitIndex] = LevelEntity->GridEntity->UnitField[UnitIndex];
            }
        }
    }
    
    LevelMemory[Index].MovingBlocksAmount = LevelEntity->GridEntity->MovingBlocksAmount;
    
    if(LevelEntity->GridEntity->MovingBlocksAmount > 0)
    {
        if(LevelMemory[Index].MovingBlocksAmount > 0)
        {
            free(LevelMemory[Index].MovingBlocks);
        }
        
        if(LevelMemory[Index].MovingBlocksAmount > 0)
        {
            LevelMemory[Index].MovingBlocks = (moving_block_memory*)malloc(sizeof(moving_block_memory) * LevelMemory[Index].MovingBlocksAmount);
            
            
            for(u32 i = 0; i < LevelMemory[Index].MovingBlocksAmount; ++i)
            {
                LevelMemory[Index].MovingBlocks[i].RowNumber  = LevelEntity->GridEntity->MovingBlocks[i].RowNumber;
                LevelMemory[Index].MovingBlocks[i].ColNumber  = LevelEntity->GridEntity->MovingBlocks[i].ColNumber;
                LevelMemory[Index].MovingBlocks[i].IsVertical = LevelEntity->GridEntity->MovingBlocks[i].IsVertical;
                LevelMemory[Index].MovingBlocks[i].MoveSwitch = LevelEntity->GridEntity->MovingBlocks[i].MoveSwitch;
            }
        }
        
    }
    
    LevelMemory[Index].FigureAmount = LevelEntity->FigureEntity->FigureAmount;
    
    if(LevelEntity->FigureEntity->FigureAmount > 0)
    {
        if(LevelMemory[Index].FigureAmount > 0)
        {
            free(LevelMemory[Index].Figures);
        }
        
        if(LevelMemory[Index].FigureAmount > 0)
        {
            LevelMemory[Index].Figures = (figure_memory*)malloc(sizeof(figure_memory) * LevelMemory[Index].FigureAmount);
            
            for(u32 i = 0; i < LevelMemory[Index].FigureAmount; ++i)
            {
                LevelMemory[Index].Figures[i].Angle = LevelEntity->FigureEntity->FigureUnit[i].Angle;
                LevelMemory[Index].Figures[i].Flip = LevelEntity->FigureEntity->FigureUnit[i].Flip;
                
                LevelMemory[Index].Figures[i].Form = LevelEntity->FigureEntity->FigureUnit[i].Form;
                LevelMemory[Index].Figures[i].Type = LevelEntity->FigureEntity->FigureUnit[i].Type;
            }
        }
        
    }
    
    SaveLevelMemoryToFile(Memory);
}


static int
SDLAssetLoadBinaryFile(void *Data)
{
    game_memory *Memory = ((game_memory*)Data);
    
    /* GlobalMemoryStorage allocation */
    
    SDLReadEntireAssetFile("package1.bin", Memory);
    LoadLevelMemoryFromFile("package2.bin", Memory);
    
    Memory->AssetsInitialized = true;
    
    return(1);
}

static void
SDLAssetBuildBinaryFile()
{
    printf("start");
    SDL_RWops *BinaryFile = SDL_RWFromFile("package1.bin", "wb");
    
    binary_header BinaryHeader = {};
    SDL_RWwrite(BinaryFile, &BinaryHeader, sizeof(binary_header), 1);
    
    /* Bitmap loading */
    BinaryHeader.BitmapSizeInBytes = 0;
    
    //SDLWriteBitmapToFile(BinaryFile, "circle_touch.png");
    
    SDLWriteBitmapToFile(BinaryFile, "grid_cell.png");
    SDLWriteBitmapToFile(BinaryFile, "grid_cell_1.png");
    SDLWriteBitmapToFile(BinaryFile, "grid_cell_2.png");
    
    SDLWriteBitmapToFile(BinaryFile, "frame1.png");
    SDLWriteBitmapToFile(BinaryFile, "frame2.png");
    SDLWriteBitmapToFile(BinaryFile, "frame3.png");
    SDLWriteBitmapToFile(BinaryFile, "frame4.png");
    
    SDLWriteBitmapToFile(BinaryFile, "i_d.png");
    SDLWriteBitmapToFile(BinaryFile, "i_m.png");
    SDLWriteBitmapToFile(BinaryFile, "i_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "o_d.png");
    SDLWriteBitmapToFile(BinaryFile, "o_m.png");
    SDLWriteBitmapToFile(BinaryFile, "o_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "l_d.png");
    SDLWriteBitmapToFile(BinaryFile, "l_m.png");
    SDLWriteBitmapToFile(BinaryFile, "l_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "j_d.png");
    SDLWriteBitmapToFile(BinaryFile, "j_m.png");
    SDLWriteBitmapToFile(BinaryFile, "j_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "s_d.png");
    SDLWriteBitmapToFile(BinaryFile, "s_m.png");
    SDLWriteBitmapToFile(BinaryFile, "s_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "z_d.png");
    SDLWriteBitmapToFile(BinaryFile, "z_m.png");
    SDLWriteBitmapToFile(BinaryFile, "z_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "t_d.png");
    SDLWriteBitmapToFile(BinaryFile, "t_m.png");
    SDLWriteBitmapToFile(BinaryFile, "t_s.png");
    
    SDLWriteBitmapToFile(BinaryFile, "left_arrow.png");
    SDLWriteBitmapToFile(BinaryFile, "right_arrow.png");
    
    /* Audio loading */
    BinaryHeader.AudioSizeInBytes = SDL_RWtell(BinaryFile) - sizeof(binary_header);
    
    SDLWriteSoundToFile(BinaryFile, "focus.wav");
    SDLWriteSoundToFile(BinaryFile, "chunk.wav");
    SDLWriteMusicToFile(BinaryFile, "amb_ending_water.ogg");
    
    SDL_RWseek(BinaryFile, 0, RW_SEEK_SET);
    
    SDL_RWwrite(BinaryFile, &BinaryHeader, sizeof(binary_header), 1);
    
    SDL_RWclose(BinaryFile);
    printf("finish");
}
