# Day 035 - Virtualized Tile Maps

Now only reserving memory from our "tiles memory block" when we need to. As in, when we write a value to a tile. That said, the memory block allocated to hold all of the tile data is hardcoded to 1MB, it's just that we simply don't reserve a portion of it for a chunk's tiles if we don't need to. With this in mind, the sparse storage is somewhat basic at this point in the project.
