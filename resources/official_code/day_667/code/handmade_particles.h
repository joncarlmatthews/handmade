/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define MAX_PARTICLE_COUNT 1024
#define MAX_PARTICLE_COUNT_4 (MAX_PARTICLE_COUNT/4)

struct particle_4x
{
    v3_4x P;
    v3_4x dP;
    v3_4x ddP;
    v4_4x C;
    v4_4x dC;
    f32_4x Size;
};

struct particle_system
{
    particle_4x Particles[MAX_PARTICLE_COUNT_4];
    u32 NextParticle4;
    bitmap_id BitmapID;
};

struct particle_cache
{
    random_series ParticleEntropy; // NOTE(casey): NOT for gameplay EVER
    particle_system FireSystem;
};
