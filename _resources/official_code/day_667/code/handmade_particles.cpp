/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define MMSetExpr(Expr) F32_4x(Expr, Expr, Expr, Expr)

internal void
SpawnFire(particle_cache *Cache, v3 AtPInit)
{
    if(Cache)
    {
        particle_system *System = &Cache->FireSystem;
        random_series *Entropy = &Cache->ParticleEntropy;
        
        v3_4x AtP = V3_4x(AtPInit);
        
        u32 ParticleIndex4 = System->NextParticle4++;
        if(System->NextParticle4 >= MAX_PARTICLE_COUNT_4)
        {
            System->NextParticle4 = 0;
        }
        
        particle_4x *A = System->Particles + ParticleIndex4;
        
        f32_4x XDir = MMSetExpr(RandomBetween(Entropy, -1.0f, 1.0f));
        f32_4x YDir = MMSetExpr(RandomBetween(Entropy, -1.0f, 1.0f));
        f32_4x PScale = MMSetExpr(0.3f);
        f32_4x dPScale = MMSetExpr(1.2f);
        
        A->P.x = PScale*XDir;
        A->P.y = PScale*YDir;
        A->P.z = MMSetExpr(0.0f);
        A->P += AtP;
        
        A->dP.x = dPScale*XDir;
        A->dP.y = dPScale*YDir;
        A->dP.z = MMSetExpr(RandomBetween(Entropy, 1.0f, 3.0f));
        
        A->ddP.x = MMSetExpr(0.0f);
        A->ddP.y = MMSetExpr(0.0f);
        A->ddP.z = MMSetExpr(-9.8f);
        
        A->C.r = MMSetExpr(RandomBetween(Entropy, 0.75f, 1.0f));
        A->C.g = MMSetExpr(RandomBetween(Entropy, 0.75f, 1.0f));
        A->C.b = MMSetExpr(RandomBetween(Entropy, 0.75f, 1.0f));
        A->C.a = MMSetExpr(1.0f);
        
        A->dC.r = MMSetExpr(0.0f);
        A->dC.g = MMSetExpr(0.0f);
        A->dC.b = MMSetExpr(0.0f);
        A->dC.a = MMSetExpr(-3.0f);
        
        A->Size = MMSetExpr(0.5f);
    }
}

internal void
UpdateAndRenderFire(particle_system *System, random_series *Entropy, f32 dt,
                    v3 FrameDisplacementInit, render_group *RenderGroup)
{
    v3_4x FrameDisplacement = V3_4x(FrameDisplacementInit);
    
#if 0
    // NOTE(casey): Particle system test
    ZeroStruct(WorldMode->ParticleCels);
    
    r32 GridScale = 0.25f;
    r32 InvGridScale = 1.0f / GridScale;
    v3 GridOrigin = {-0.5f*GridScale*PARTICLE_CEL_DIM, 0.0f, 0.0f};
    for(u32 ParticleIndex = 0;
        ParticleIndex < ArrayCount(WorldMode->Particles);
        ++ParticleIndex)
    {
        particle *Particle = WorldMode->Particles + ParticleIndex;
        
        v3 P = InvGridScale*(Particle->P - GridOrigin);
        
        s32 X = TruncateReal32ToInt32(P.x);
        s32 Y = TruncateReal32ToInt32(P.y);
        
        if(X < 0) {X = 0;}
        if(X > (PARTICLE_CEL_DIM - 1)) {X = (PARTICLE_CEL_DIM - 1);}
        if(Y < 0) {Y = 0;}
        if(Y > (PARTICLE_CEL_DIM - 1)) {Y = (PARTICLE_CEL_DIM - 1);}
        
        particle_cel *Cel = &WorldMode->ParticleCels[Y][X];
        real32 Density = Particle->Color.a;
        Cel->Density += Density;
        Cel->VelocityTimesDensity += Density*Particle->dP;
    }
    
    if(Global_Particles_ShowGrid)
    {
        for(u32 Y = 0;
            Y < PARTICLE_CEL_DIM;
            ++Y)
        {
            for(u32 X = 0;
                X < PARTICLE_CEL_DIM;
                ++X)
            {
                particle_cel *Cel = &WorldMode->ParticleCels[Y][X];
                real32 Alpha = Clamp01(0.1f*Cel->Density);
                PushRect(RenderGroup, EntityTransform, GridScale*V3((r32)X, (r32)Y, 0) + GridOrigin, GridScale*V2(1.0f, 1.0f),
                         V4(Alpha, Alpha, Alpha, 1.0f));
            }
        }
    }
#endif
    
    for(u32 ParticleIndex4 = 0;
        ParticleIndex4 < MAX_PARTICLE_COUNT_4;
        ++ParticleIndex4)
    {
        particle_4x *A = System->Particles + ParticleIndex4;
        
#if 0
        v3 P = InvGridScale*(Particle->P - GridOrigin);
        
        s32 X = TruncateReal32ToInt32(P.x);
        s32 Y = TruncateReal32ToInt32(P.y);
        
        if(X < 1) {X = 1;}
        if(X > (PARTICLE_CEL_DIM - 2)) {X = (PARTICLE_CEL_DIM - 2);}
        if(Y < 1) {Y = 1;}
        if(Y > (PARTICLE_CEL_DIM - 2)) {Y = (PARTICLE_CEL_DIM - 2);}
        
        particle_cel *CelCenter = &WorldMode->ParticleCels[Y][X];
        particle_cel *CelLeft = &WorldMode->ParticleCels[Y][X - 1];
        particle_cel *CelRight = &WorldMode->ParticleCels[Y][X + 1];
        particle_cel *CelDown = &WorldMode->ParticleCels[Y - 1][X];
        particle_cel *CelUp = &WorldMode->ParticleCels[Y + 1][X];
        
        v3 Dispersion = {};
        real32 Dc = 1.0f;
        Dispersion += Dc*(CelCenter->Density - CelLeft->Density)*V3(-1.0f, 0.0f, 0.0f);
        Dispersion += Dc*(CelCenter->Density - CelRight->Density)*V3(1.0f, 0.0f, 0.0f);
        Dispersion += Dc*(CelCenter->Density - CelDown->Density)*V3(0.0f, -1.0f, 0.0f);
        Dispersion += Dc*(CelCenter->Density - CelUp->Density)*V3(0.0f, 1.0f, 0.0f);
        
        v3 ddP = Particle->ddP + Dispersion;
#endif
        
        // NOTE(casey): Simulate the particle forward in time
        A->P += 0.5f*Square(dt)*A->ddP + dt*A->dP;
        //A->P += FrameDisplacement;
        A->dP += dt*A->ddP;
        A->C += dt*A->dC;
        
#if 0
        if(Particle->P.y < 0.0f)
        {
            r32 CoefficientOfRestitution = 0.3f;
            r32 CoefficientOfFriction = 0.7f;
            Particle->P.y = -Particle->P.y;
            Particle->dP.y = -CoefficientOfRestitution*Particle->dP.y;
            Particle->dP.x = CoefficientOfFriction*Particle->dP.x;
        }
        
        // TODO(casey): Shouldn't we just clamp colors in the renderer??
        v4 Color;
        Color.r = Clamp01(Particle->Color.r);
        Color.g = Clamp01(Particle->Color.g);
        Color.b = Clamp01(Particle->Color.b);
        Color.a = Clamp01(Particle->Color.a);
        
        if(Color.a > 0.9f)
        {
            Color.a = 0.9f*Clamp01MapToRange(1.0f, Color.a, 0.9f);
        }
#endif
        
        // NOTE(casey): Render the particle
        for(u32 SubIndex = 0;
            SubIndex < 4;
            ++SubIndex)
        {
            v3 P =
            {
                M(A->P.x, SubIndex),
                M(A->P.y, SubIndex),
                M(A->P.z, SubIndex),
            };
            
            v4 C =
            {
                M(A->C.r, SubIndex),
                M(A->C.g, SubIndex),
                M(A->C.b, SubIndex),
                M(A->C.a, SubIndex),
            };
            
            if(C.a > 0)
            {
                C.a *= C.a;
                PushBitmap(RenderGroup, System->BitmapID, M(A->Size, SubIndex), P, V2(0.5f, 0.5f), C);
            }
        }
        
        // NOTE(casey): Since the frame displacement happens on the _next_ frame,
        // we apply displacement _after_ we have rendered the particles for this frame.
        A->P += FrameDisplacement;
    }
}

internal void
UpdateAndRenderParticleSystems(particle_cache *Cache, f32 dt, render_group *RenderGroup,
                               v3 FrameDisplacement)
{
    TIMED_FUNCTION();
    UpdateAndRenderFire(&Cache->FireSystem, &Cache->ParticleEntropy, dt, FrameDisplacement,
                        RenderGroup);
}

internal void
InitParticleCache(particle_cache *Cache, game_assets *Assets)
{
    ZeroStruct(*Cache);
    Cache->ParticleEntropy = RandomSeed(1234);
    
    asset_tag_hash Tags = GetTagHash(Asset_Particle, Tag_Particle, Tag_Smoke);
    
    Cache->FireSystem.BitmapID = GetBestMatchBitmapFrom(Assets, Tags);
}
