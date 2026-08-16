#define internal static
#define global static
#define Assert assert

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;
typedef s32 b32x;

typedef float f32;
typedef double f64;

#define U32Max ((u32)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX
#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#include "ray_lane.h"
#include "ray_math.h"

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct image_u32
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

struct brdf_table
{
    u32 Count[3];
    v3 *Values;
};

struct material
{
    f32 Specular; // NOTE(case): 0 is pure diffuse ("chalk"), 1 is pure specular ("mirror")
    v3 RefColor;
    v3 EmitColor;
    
    brdf_table BRDF;
};

// TODO(casey): These will probably be reorganized for SIMD :)
struct plane
{
    v3 Normal, Tangent, Binormal;
    f32 d;
    u32 MatIndex;
};

struct sphere
{
    v3 P;
    f32 r;
    u32 MatIndex;
};

struct world
{
    u32 MaterialCount;
    material *Materials;
    
    u32 PlaneCount;
    plane *Planes;
    
    u32 SphereCount;
    sphere *Spheres;
};

struct random_series
{
    lane_u32 State;
};

struct work_order
{
    world *World;
    image_u32 Image;
    u32 XMin;
    u32 YMin;
    u32 OnePastXMax;
    u32 OnePastYMax;
    
    random_series Entropy;
};

struct work_queue
{
    u32 RaysPerPixel;
    u32 MaxBounceCount;
    
    u32 WorkOrderCount;
    work_order *WorkOrders;
    
    volatile u64 NextWorkOrderIndex;
    volatile u64 BouncesComputed;
    volatile u64 LoopsComputed;
    volatile u64 TileRetiredCount;
};

struct cast_state
{
    // NOTE(casey): In
    world *World;
    u32 RaysPerPixel;
    u32 MaxBounceCount;
    random_series Series;
    
    v3 CameraP;
    v3 CameraX;
    v3 CameraY;
    v3 CameraZ;
    
    f32 FilmW;
    f32 FilmH;
    f32 HalfFilmW;
    f32 HalfFilmH;
    v3 FilmCenter;
    
    f32 HalfPixW;
    f32 HalfPixH;
    
    f32 FilmX;
    f32 FilmY;
    
    // NOTE(casey): Out
    v3 FinalColor;
    u64 BouncesComputed;
    u64 LoopsComputed;
};

// NOTE(casey): Platform-specific stuff:
internal u32 GetCPUCoreCount(void);
internal void CreateWorkThread(void *Parameter);

// NOTE(casey): LockedAddAndReturnPreviousValue _must_ include a barrier
// if the CPU would not otherwise enforce one!
internal u64 LockedAddAndReturnPreviousValue(u64 volatile *Value, u64 Addend);

