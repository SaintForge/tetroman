// math.cpp --- 
// 
// Filename: game.cpp
// Author: Sierra
// Created: Вт окт 10 10:32:14 2017 (+0300)
// Last-Updated: Пт окт 27 14:17:15 2017 (+0300)
//           By: Sierra
//

#if !defined(GAME_MATH_H)

union v2
{
    struct 
    {
        r32 x, y;
    };
    struct
    {
        r32 w, h;
    };
    r32 E[2];
    
};

struct v3
{
    r32 x;
    r32 y;
    r32 z;
};

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };
        
        r32 w;        
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };
        
        r32 a;        
    };
    r32 E[4];
};

inline v2
V2(r32 x, r32 y)
{
    v2 Result;
    
    Result.x = x;
    Result.y = y;
    
    return(Result);
}

inline v2
operator*(r32 A, v2 B)
{
    v2 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2
operator*(v2 B, r32 A)
{
    v2 Result = A*B;
    
    return(Result);
}

inline v2 &
operator*=(v2 &B, r32 A)
{
    B = A * B;
    
    return(B);
}

inline v2
operator/(v2 A, r32 B)
{
    v2 Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    
    return(Result);
}

inline v2 &
operator/=(v2 &A, r32 B)
{
    A = A / B;
    
    return(A);
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2
operator+(v2 A, r32 B)
{
    v2 Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    
    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return(A);
}


inline v2 &
operator+=(v2 &A, r32 B)
{
    A.x = A.x + B;
    A.y = A.y + B;
    
    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline r32
Square(v2 v)
{
    r32 Result = 0.0f;
    Result = sqrt(v.x * v.x + v.y * v.y);
    
    return(Result);
}

inline v2
Normalize(v2 A)
{
    v2 Result = {};
    Result = A / Square(A);
    
    return(Result);
}

struct rectangle2
{
    v2 Min;
    v2 Max;
};

static v2
GetDim(rectangle2 Rectangle)
{
    v2 Result = {};
    
    Result.w = Rectangle.Max.x - Rectangle.Min.x;
    Result.w = Rectangle.Max.y - Rectangle.Min.y;
    
    return (Result);
}

static void
SetDim(rectangle2 *Rectangle, v2 Dimension)
{
    Rectangle->Max.x = Rectangle->Min.x + Dimension.w;
    Rectangle->Max.y = Rectangle->Min.y + Dimension.h;
}

static void
SetDim(rectangle2 *Rectangle, r32 DimensionWidth, r32 DimensionHeight)
{
    Rectangle->Max.x = Rectangle->Min.x + DimensionWidth;
    Rectangle->Max.y = Rectangle->Min.y + DimensionHeight;
}


inline void
Swap(r32 *A, r32 *B)
{
    r32 C = *A;
    *A = *B;
    *B = C;
}

inline static s32
Max2(s32 a, s32 b)
{
    return (a > b) ? a : b;
}

inline static s32
Min2(s32 a, s32 b)
{
    return (a < b) ? a : b;
}

inline static s32 
Min3(s32 a, s32 b, s32 c)
{
    return fmin(fmin(a,b),c);
}

#define GAME_MATH_H
#endif 


