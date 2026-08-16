member_definition MembersOf_entity_hash[] = 
{
   {MetaMemberFlag_IsPointer, MetaType_entity, "Ptr", PointerToU32(&((entity_hash *)0)->Ptr)},
};
member_definition MembersOf_brain_hash[] = 
{
   {MetaMemberFlag_IsPointer, MetaType_brain, "Ptr", PointerToU32(&((brain_hash *)0)->Ptr)},
};
member_definition MembersOf_sim_region[] = 
{
   {MetaMemberFlag_IsPointer, MetaType_world, "World", PointerToU32(&((sim_region *)0)->World)},
   {0, MetaType_world_position, "Origin", PointerToU32(&((sim_region *)0)->Origin)},
   {0, MetaType_rectangle3, "Bounds", PointerToU32(&((sim_region *)0)->Bounds)},
   {0, MetaType_rectangle3, "UpdatableBounds", PointerToU32(&((sim_region *)0)->UpdatableBounds)},
   {0, MetaType_u32, "MaxEntityCount", PointerToU32(&((sim_region *)0)->MaxEntityCount)},
   {0, MetaType_u32, "EntityCount", PointerToU32(&((sim_region *)0)->EntityCount)},
   {MetaMemberFlag_IsPointer, MetaType_entity, "Entities", PointerToU32(&((sim_region *)0)->Entities)},
   {0, MetaType_u32, "MaxBrainCount", PointerToU32(&((sim_region *)0)->MaxBrainCount)},
   {0, MetaType_u32, "BrainCount", PointerToU32(&((sim_region *)0)->BrainCount)},
   {MetaMemberFlag_IsPointer, MetaType_brain, "Brains", PointerToU32(&((sim_region *)0)->Brains)},
   {0, MetaType_entity_hash, "EntityHash", PointerToU32(&((sim_region *)0)->EntityHash)},
   {0, MetaType_brain_hash, "BrainHash", PointerToU32(&((sim_region *)0)->BrainHash)},
   {0, MetaType_u64, "EntityHashOccupancy", PointerToU32(&((sim_region *)0)->EntityHashOccupancy)},
   {0, MetaType_u64, "BrainHashOccupancy", PointerToU32(&((sim_region *)0)->BrainHashOccupancy)},
   {0, MetaType_entity, "NullEntity", PointerToU32(&((sim_region *)0)->NullEntity)},
};
#define META_HANDLE_TYPE_DUMP(MemberPtr, NextIndentLevel) \
    case MetaType_sim_region: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_sim_region), MembersOf_sim_region, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_brain_hash: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_brain_hash), MembersOf_brain_hash, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_entity_hash: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_entity_hash), MembersOf_entity_hash, MemberPtr, (NextIndentLevel));} break; 
