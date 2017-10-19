#include <a_samp>

#define ENABLE_MTA_STYLE_CHECKPOINTS true

#define ANY_WORLD_ID -1
#define INVALID_CHECKPOINT_ID -1
#define MAX_CHECKPOINTS 256
#define MAX_CPDISTANCE 400.0
#define DEFAULT_CP_SIZE 10.0

enum checkpointInfo {
	cLoaded,
	Float:cX,
	Float:cY,
	Float:cZ,
	Float:cSize,
	cVirtualWorld
}
new cpData[MAX_CHECKPOINTS][checkpointInfo];
new pActiveCP[MAX_PLAYERS];

CheckpointInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		pActiveCP[i] = INVALID_CHECKPOINT_ID;
	}
	kEnableTimer(3); // SetTimer("fdm_CheckpointCheck", 100, true); // Start the checkpoint syncing loop
}
CheckpointExit() {
	RemoveAllCheckpoints();
	return 1;
}

checkpoint_OnPlayerConnect(playerid) {
	pActiveCP[playerid] = INVALID_CHECKPOINT_ID;
}

// Functions converted to return the checkpoint id
checkpoint_OnPlayerEnterCP(playerid) {
	if (IsPlayerSpawned(playerid)) {
		kOnPlayerEnterCheckpoint(playerid, pActiveCP[playerid]);
		
		#if ENABLE_CALLBACKS
			CallRemoteFunction("fdmOnPlayerEnterCheckpoint", "ii", playerid, pActiveCP[playerid]);
		#endif
	}
}
checkpoint_OnPlayerLeaveCP(playerid) {
	if (IsPlayerSpawned(playerid)) {
		kOnPlayerLeaveCheckpoint(playerid, pActiveCP[playerid]);
		
		#if ENABLE_CALLBACKS
			CallRemoteFunction("fdmOnPlayerLeaveCheckpoint", "ii", playerid, pActiveCP[playerid]);
		#endif
	}
}

stock IsPlayerInCheckpointID(playerid, checkpointid) {
	if ((checkpointid < 0) || (checkpointid >= MAX_CHECKPOINTS) || !cpData[checkpointid][cLoaded]) return 0;
	if (cpData[checkpointid][cVirtualWorld] != ANY_WORLD_ID && cpData[checkpointid][cVirtualWorld] != GetPlayerVirtualWorld(playerid)) return 0;
	if (!IsPlayerSpawned(playerid)) return 0;
	
	if (IsPlayerInRangeOfPoint(playerid, cpData[checkpointid][cSize], cpData[checkpointid][cX], cpData[checkpointid][cY], cpData[checkpointid][cZ])) {
		return 1;
	} else {
		return 0;
	}
}

stock AddCheckpoint(Float:x, Float:y, Float:z, Float:size = DEFAULT_CP_SIZE, virtualworld = ANY_WORLD_ID) {
	new checkpointid = FindFreeCheckpointID();
	if (checkpointid == INVALID_CHECKPOINT_ID) return INVALID_CHECKPOINT_ID; // No spare checkpoint slot
	
	// Load checkpoint data
	cpData[checkpointid][cLoaded] = 1;
	cpData[checkpointid][cX] = x;
	cpData[checkpointid][cY] = y;
	cpData[checkpointid][cZ] = z;
	cpData[checkpointid][cSize] = size;
	cpData[checkpointid][cVirtualWorld] = virtualworld;
	
	return checkpointid;
}

stock RemoveCheckpoint(checkpointid) {
	// Reset checkpoint data, the timer will automitcally remove the cp
	if (checkpointid == INVALID_CHECKPOINT_ID) return 0;
	new tmpdata[checkpointInfo];
	cpData[checkpointid] = tmpdata;
	return 1;
}

stock GetPlayerCurrentCheckpoint(playerid) {
	if (pActiveCP[playerid] != INVALID_CHECKPOINT_ID && cpData[pActiveCP[playerid]][cLoaded] && IsPlayerInCheckpointID(playerid, pActiveCP[playerid])) {
		return pActiveCP[playerid];
	}
	return INVALID_CHECKPOINT_ID;
}

forward fdm_CheckpointCheck();
public fdm_CheckpointCheck() {
	LoopPlayers(i) {
		checkpointCheckPlayer(i);
	}
	return 1;
}

checkpointCheckPlayer(playerid) {
	new Float:px;
	new Float:py;
	new Float:pz;
	
	// Check if players can show checkpoints
	if (IsPlayerInMinimode(playerid)) { // No point of running streamer if the player is in a minimode
		if (pActiveCP[playerid] == INVALID_CHECKPOINT_ID) return 1;
		DisablePlayerCheckpoint(playerid);
		pActiveCP[playerid] = INVALID_CHECKPOINT_ID;
		return 1;
	}
	if (GetPlayerState(playerid) != PLAYER_STATE_SPECTATING && IsPlayerPaused(playerid, 5000)) return 1; // Is player paused and not spectating
	
	// Find closest checkpoint
	new checkpointid = INVALID_CHECKPOINT_ID;
	new Float:cpdistance;
	new Float:mindistance = -1.0;
	
	GetPlayerPos(playerid, px, py, pz);
	for (new c; c < MAX_CHECKPOINTS; c++) {
		if (!cpData[c][cLoaded]) continue;
		if (cpData[c][cVirtualWorld] != ANY_WORLD_ID && cpData[c][cVirtualWorld] != GetPlayerVirtualWorld(playerid)) continue;
		if (!IsPlayerInRangeOfPoint(playerid, MAX_CPDISTANCE, cpData[c][cX], cpData[c][cY], cpData[c][cZ])) continue;
		
		cpdistance = FindDistance(px, py, pz, // Player position
			cpData[c][cX], cpData[c][cY], cpData[c][cZ]); // Checkpoint position
		
		if ((cpdistance < mindistance) || (mindistance == -1.0)) { // Checkpoint is closest
			checkpointid = c;
			mindistance = cpdistance;
		}
	}
	
	// Check if it's a valid checkpoint
	if (checkpointid == INVALID_CHECKPOINT_ID) {
		// No valid checkpoints, disable checkpoints for player
		pActiveCP[playerid] = INVALID_CHECKPOINT_ID;
		DisablePlayerCheckpoint(playerid);
		return 1;
	}
	
	// Update if it's different from the player's last checkpoint
	if (checkpointid != pActiveCP[playerid]) {
		pActiveCP[playerid] = checkpointid;
		DisablePlayerCheckpoint(playerid);
		SetPlayerCheckpoint(playerid,
			cpData[checkpointid][cX], cpData[checkpointid][cY], cpData[checkpointid][cZ], cpData[checkpointid][cSize]);
		return 1;
	}
	return 1;
}

stock FindFreeCheckpointID() {
	for (new i; i < MAX_CHECKPOINTS; i++) {
		if (!cpData[i][cLoaded]) return i;
	}
	return INVALID_CHECKPOINT_ID;
}

stock RemoveAllCheckpoints() { //TODO
	// Reset all checkpoints for players
	for (new i1; i1 < MAX_PLAYERS; i1++) {
		pActiveCP[i1] = INVALID_CHECKPOINT_ID;
		
		if (!IsPlayerConnected(i1)) continue;
		DisablePlayerCheckpoint(i1);
	}
	
	// Reset all checkpoint data
	new tmpdata[checkpointInfo];
	for (new i2; i2 < MAX_CHECKPOINTS; i2++) {
		cpData[i2] = tmpdata;
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// NOT STREAMING, BUT MTA STYLE CHECKPOINTS
//-----------------------------------------------------------------------------------------------------



#if ENABLE_MTA_STYLE_CHECKPOINTS

#define CP_TYPE_NONE 0
#define CP_TYPE_GROUND 1
#define CP_TYPE_AIR 2

enum playerCheckpointInfo {
	Float:cEnabled,
	cType,
	Float:cX,
	Float:cY,
	Float:cZ,
	Float:cSize,
	cIsInCheckpoint
}
new pcData[MAX_PLAYERS][playerCheckpointInfo];

kSetPlayerRaceCheckpoint(playerid, type, Float:x, Float:y, Float:z, Float:nextx, Float:nexty, Float:nextz, Float:size) {
	
	pcData[playerid][cEnabled] = 1;
	pcData[playerid][cIsInCheckpoint] = 0;
	
	pcData[playerid][cX] = x;
	pcData[playerid][cY] = y;
	pcData[playerid][cZ] = z;
	pcData[playerid][cSize] = size;
	
	switch (type) {
		case 0,1: pcData[playerid][cType] = CP_TYPE_GROUND;
		case 3,4: pcData[playerid][cType] = CP_TYPE_AIR;
		default: pcData[playerid][cType] = CP_TYPE_NONE;
	}
	
	return SetPlayerRaceCheckpoint(playerid, type, x, y, z, nextx, nexty, nextz, size * 0.5);
}

kDisablePlayerRaceCheckpoint(playerid) {
	if (DisablePlayerRaceCheckpoint(playerid)) {
		pcData[playerid][cEnabled] = 0;
		return 1;
	}
	return 0;
}

kIsPlayerInRaceCheckpoint(playerid) {
	if (!IsPlayerSpawned(playerid)) return 0;
	return pcData[playerid][cIsInCheckpoint];
}

checkpoint_OnPlayerDisconnect(playerid) {
	pcData[playerid][cEnabled] = 0;
	return 1;
}

checkpoint_OnPlayerUpdate(playerid) {
	if (!pcData[playerid][cEnabled]) return 1;
	if (!IsPlayerSpawned(playerid)) return 1;
	
	new Float:x, Float:y, Float:z, Float:rotz, vid, incheckpoint;
	
	vid = GetPlayerVehicleID(playerid);
	if (vid) {
		// SAMP position bug when just entering vehicle
		GetVehicleZAngle(vid, rotz);
		if (rotz == 0.0) {
			GetPlayerPos(playerid, x, y, z);
		} else {
			GetVehiclePos(vid, x, y, z);
		}
	} else {
		GetPlayerPos(playerid, x, y, z);
	}
	
	new Float:radius = pcData[playerid][cSize];
	switch (pcData[playerid][cType]) {
		case CP_TYPE_GROUND: {
			/*if (x < pcData[playerid][cX] - radius ||
				x > pcData[playerid][cX] + radius ||
				y < pcData[playerid][cY] - radius ||
				y > pcData[playerid][cY] + radius) {
				vid = -1;
			} else {
				distance = FindDistanceXY(x, y, pcData[playerid][cX], pcData[playerid][cY]);
			}*/
			if (IsPlayerInRangeOfPoint(playerid, radius, pcData[playerid][cX], pcData[playerid][cY], z)) {
				incheckpoint = 1;
			}
		}
		case CP_TYPE_AIR: {
			/*if (x < pcData[playerid][cX] - radius ||
				x > pcData[playerid][cX] + radius ||
				y < pcData[playerid][cY] - radius ||
				y > pcData[playerid][cY] + radius ||
				z < pcData[playerid][cZ] - radius ||
				z > pcData[playerid][cZ] + radius) {
				vid = -1;
			} else {
				distance = FindDistance(x, y, z, pcData[playerid][cX], pcData[playerid][cY], pcData[playerid][cZ]);
			}*/
			if (IsPlayerInRangeOfPoint(playerid, radius, pcData[playerid][cX], pcData[playerid][cY], pcData[playerid][cZ])) {
				incheckpoint = 1;
			}
		}
		default: return 1;
	}
	
	if (incheckpoint) {
		if (!pcData[playerid][cIsInCheckpoint]) {
			pcData[playerid][cIsInCheckpoint] = 1;
			kOnPlayerEnterRaceCheckpoint(playerid);
		}
	} else {
		if (pcData[playerid][cIsInCheckpoint]) {
			pcData[playerid][cIsInCheckpoint] = 0;
			kOnPlayerLeaveRaceCheckpoint(playerid);
		}
	}
	
	return 1;
}

#define SetPlayerRaceCheckpoint kSetPlayerRaceCheckpoint
#define DisablePlayerRaceCheckpoint kDisablePlayerRaceCheckpoint

#endif
	