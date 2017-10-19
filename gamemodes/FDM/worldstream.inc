#include <a_samp>

#if ENABLE_INTERIOR_WORLD_SYNC
	#define MIN_WORLD_ID 1002
#else
	#define MIN_WORLD_ID 1001
#endif
#define MAX_VIRTUAL_WORLDS MAX_PLAYERS

// 0 is reserved for main world
// 1 to 500 is reserved for jailed players or individual worlds
// 501 to 1000 is reserved for any other possible scripts
// 1001 to 2000 is reserved for minimodes in progress

enum worldDataInfo {
	wUsed
}
new wldData[MAX_VIRTUAL_WORLDS][worldDataInfo];

stock GetFreeVirtualWorldID() {
	for (new i; i < MAX_VIRTUAL_WORLDS; i++) {
		if (!wldData[i][wUsed]) {
			return i + MIN_WORLD_ID;
		}
	}
	return 0;
}

stock ToggleVirtualWorldUsed(worldid, toggle) {
	if (worldid < MIN_WORLD_ID || worldid > MIN_WORLD_ID + MAX_VIRTUAL_WORLDS) return 0;
	wldData[worldid - MIN_WORLD_ID][wUsed] = toggle;
	return 1;
}
