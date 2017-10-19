#include <a_samp>

#define FILE_DERBY "FDM/Minimodes/DerbyList.ini"
#define FILE_DERBY_MAPS "FDM/Minimodes/Derby/"

#if defined CRIPPLE_PLAYERS
	#define MAX_DERBY_MAPS 6
#else
	#define MAX_DERBY_MAPS 24
#endif
#define MAX_DERBY_SPAWNS 128
#define MAX_DERBY_BOUND_POLYS 32

#define DERBY_BASE_XP 1000
#define DERBY_BASE_CASH 2500

#define MAX_DERBY_PICKUPS 256

enum derbyBoundsRadiusInfo {
	dbrUsed,
	Float:dbCentreX,
	Float:dbCentreY,
	Float:dbCentreZ,
	Float:dbRadius
}
enum derbyBoundsRectangleInfo {
	dbeUsed,
	Float:dbeMinX,
	Float:dbeMinY,
	Float:dbeMinZ,
	Float:dbeMaxX,
	Float:dbeMaxY,
	Float:dbeMaxZ
}
enum derbyPickupInfo {
	dpType,
	Float:dpX,
	Float:dpY,
	Float:dpZ,
	dpRespawnTime,
	dpVehicleModel,
	dpPickupID,
	Text3D:dpTextID
}
enum derbyInfo {
	dIsLoaded,
	dModeID,
	dStartTime,
	dRunTime,
	dWinBaseXPMultiplier,
	dWinBaseCashMultiplier,
	dVehicleSpawnsCount,
	dWinCount,
	dCountDown,
	dPlayerCount,
	dBoundsRadius[derbyBoundsRadiusInfo],
	dBoundsRectangle[derbyBoundsRectangleInfo],
	dBoundPoly[E_POLYGON],
	dVehicleModel,
	Float:dVehicleHealth,
	dVehicleComponents[MAX_CAR_COMPONENTS],
	dSpawnsUsed[MAX_DERBY_SPAWNS]
}
enum derbyVehicleSpawnsInfo {
	Float:dvX,
	Float:dvY,
	Float:dvZ,
	Float:dvRotZ
}
enum derbyPlayerStats {
	dpMapID,
	dpVehicleID,
	dpIsRaceVehicle,
	dpIsTextDrawOn,
	Text:dpTextPlayers,
	Float:dpSpawnX,
	Float:dpSpawnY,
	Float:dpSpawnZ,
	Float:dpSpawnRotZ,
	dpJustSpawned,
	dpPlayerUpdated,
	dpIsInPickup[MAX_DERBY_PICKUPS],
	Float:dpLastSX,
	Float:dpLastSY,
	Float:dpLastSZ,
	Float:dpLastSwitchTick,
	Float:dpDamage,
	Float:dpLastVHP
}
enum derbyPickupLink {
	dblMapID,
	dblPickupID
}
new derbyPStats[MAX_PLAYERS][derbyPlayerStats];
new derbyPickupsLink[MAX_PICKUPS][derbyPickupLink];
new derbyPickups[MAX_DERBY_MAPS][MAX_DERBY_PICKUPS][derbyPickupInfo];
new derbyMaps[MAX_DERBY_MAPS][derbyInfo];
new derbySpawns[MAX_DERBY_MAPS][MAX_DERBY_SPAWNS][derbyVehicleSpawnsInfo];
// new derbyTimer;
// new derbyPickupTimer;
new derbyDefaultXPMultiplier;
new derbyDefaultCashMultiplier;

new derbiesInProgress[MAX_DERBY_MAPS + 1];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

DerbySettingsInit() {
	derbyDefaultXPMultiplier = INI_ReadInt("DerbyDefaultXPMult");
	derbyDefaultCashMultiplier = INI_ReadInt("DerbyDefaultCashMult");
	return 1;
}

DerbysInit() {
	for (new i; i < MAX_DERBY_MAPS; i++) {
		derbyMaps[i][dModeID] = INVALID_MODE_ID;
		for (new j; j < MAX_DERBY_PICKUPS; j++) {
			derbyPickups[i][j][dpPickupID] = INVALID_PICKUP_ID;
		}
	}
	for (new i; i < MAX_PICKUPS; i++) {
		derbyPickupsLink[i][dblMapID] = INVALID_MAP_ID;
		derbyPickupsLink[i][dblPickupID] = INVALID_PICKUP_ID;
	}
	for (new i; i < MAX_PLAYERS; i++) {
		derbyPStats[i][dpMapID] = INVALID_MAP_ID;
		derbyPStats[i][dpVehicleID] = INVALID_VEHICLE_ID;
	}
	derbiesInProgress[0] = INVALID_MAP_ID;
	
	LoadDerbyMaps();
	
	kEnableTimer(9); // derbyTimer = SetTimer("fdm_derby_checkDerbys", 925, true);
	kEnableTimer(10); // derbyPickupTimer = SetTimer("fdm_derby_PickupCheck", 16, true);
	return 1;
}

DerbysUnload() {
	new tmp1[derbyPlayerStats];
	new tmp2[derbyInfo];
	new tmp3[derbyVehicleSpawnsInfo];
	new tmp4[derbyPickupInfo];
	
	for (new a; a < MAX_PLAYERS; a++) {
		derbyPStats[a] = tmp1;
	}
	for (new b; b < MAX_DERBY_MAPS; b++) {
		derbyMaps[b] = tmp2;
		for (new c; c < MAX_DERBY_SPAWNS; c++) {
			derbySpawns[b][c] = tmp3;
		}
		for (new c; c < MAX_DERBY_PICKUPS; c++) {
			derbyPickups[b][c] = tmp4;
		}
	}
	kDisableTimer(9); // KillTimer(derbyTimer);
	kDisableTimer(10); // KillTimer(derbyPickupTimer);
	return 1;
}

LoadDerbyMaps() {
	if (!INI_Exist(FILE_DERBY)) {
		printf("[FDM] WARNING: Derby Map List Fail: '%s' - File not found.", FILE_DERBY);
		return 0;
	}
	
	if (!INI_Open(FILE_DERBY)) {
		printf("[FDM] WARNING: Derby Map List Fail: '%s' - File cannot be opened.", FILE_DERBY);
		return 0;
	}
	
	new derbyFiles[MAX_DERBY_MAPS][MAX_FILENAME];
	new derbycount;
	new derbysLoaded;
	//new key[INI_MAX_KEY];
	
	new i3;
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "DERBY", false)) {
			format(derbyFiles[i3], MAX_FILENAME, "%s%s", FILE_DERBY_MAPS, gCache[l][E_VALUE]);
			i3++;
			derbycount = i3;
		}
		if (i3 >= MAX_DERBY_MAPS) {
			printf("[FDM] WARNING: Maximum derby maps reached (%i derbies)", MAX_DERBY_MAPS);
			break; // Maximum derbies reached
		}
	}
	/*for (new i; i < MAX_DERBY_MAPS; i++) { // Read derby list
		format(key, sizeof(key), "DERBY%i", i);
		
		if (INI_ReadString(derbyFiles[i], key)) {
			// Add the file path to the beginning
			format(derbyFiles[i], MAX_FILENAME, "%s%s", FILE_DERBY_MAPS, derbyFiles[i]);
		} else {
			derbycount = i;
			break; // End of file
		}
	}*/
	INI_Close();
	
	if (!derbycount) return 0; // There are no derbys to load
	
	for (new i; i < derbycount; i++) {
		if (!INI_Exist(derbyFiles[i])) {
			printf("[FDM] WARNING: Derby Load Fail: '%s' - File not found.", derbyFiles[i]);
			continue;
		}
		if (!INI_Open(derbyFiles[i])) {
			printf("[FDM] WARNING: Derby Load Fail: '%s' - File cannot be opened.", derbyFiles[i]);
			continue;
		}
		
		new derbyname[MAX_MODE_NAME];
		new tmpstr[MAX_STRING];
		//new minlevel;
		//new cost;
		new idx;
		
		INI_ReadString(derbyname, "Name");
		if (!derbyname[0]) { // Invalid derby name
			INI_Close();
			printf("[FDM] WARNING: DMFFA Load Fail: '%s' - File is corrupt/invalid.", derbyFiles[i]);
			continue;
		}
		
		/* Minimode data
		minlevel = INI_ReadInt("MinLevel");
		cost = INI_ReadInt("JoinCost");
		
		INI_ReadString(tmpstr, "WaitLocation"); // Waiting location
		idx = 0;
		new Float:x = floatstr(split(tmpstr, idx, ','));
		new Float:y = floatstr(split(tmpstr, idx, ','));
		new Float:z = floatstr(split(tmpstr, idx, ','));*/
		
		// Derby data
		derbyMaps[i][dRunTime] = INI_ReadInt("RunTime");
		derbyMaps[i][dWinBaseXPMultiplier] = INI_ReadInt("WinMultiplierXP");
		derbyMaps[i][dWinBaseCashMultiplier] = INI_ReadInt("WinMultiplierCash");
		derbyMaps[i][dVehicleModel] = INI_ReadInt("Vehicle");
		
		// Vehicle components
		derbyMaps[i][dVehicleHealth] = INI_ReadFloat("VehicleHealth");
		if (derbyMaps[i][dVehicleHealth] == 0.0) derbyMaps[i][dVehicleHealth] = 1000.0;
		if (INI_ReadString(tmpstr, "VehicleComponents")) {
			idx = 0;
			for (new d; d < MAX_CAR_COMPONENTS; d++) {
				derbyMaps[i][dVehicleComponents][d] = strval(split(tmpstr, idx, ','));
				if (!derbyMaps[i][dVehicleComponents][d]) break;
			}
		}
		
		// Boundaries
		idx = 0;
		if (INI_ReadString(tmpstr, "Bounds")) { // DM Boundaries
			derbyMaps[i][dBoundsRadius][dbrUsed] = 1;
			derbyMaps[i][dBoundsRadius][dbCentreX] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRadius][dbCentreY] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRadius][dbCentreZ] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRadius][dbRadius] = floatstr(split(tmpstr, idx, ','));
		} else {
			derbyMaps[i][dBoundsRadius][dbrUsed] = 0;
		}
		
		idx = 0;
		if (INI_ReadString(tmpstr, "BoundRectangle")) { // DM Boundaries
			derbyMaps[i][dBoundsRectangle][dbeUsed] = 1;
			derbyMaps[i][dBoundsRectangle][dbeMinX] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRectangle][dbeMinY] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRectangle][dbeMinZ] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRectangle][dbeMaxX] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRectangle][dbeMaxY] = floatstr(split(tmpstr, idx, ','));
			derbyMaps[i][dBoundsRectangle][dbeMaxZ] = floatstr(split(tmpstr, idx, ','));
			
			if (derbyMaps[i][dBoundsRectangle][dbeMinX] > derbyMaps[i][dBoundsRectangle][dbeMaxX])
				floatswap(derbyMaps[i][dBoundsRectangle][dbeMinX], derbyMaps[i][dBoundsRectangle][dbeMaxX]);
			
			if (derbyMaps[i][dBoundsRectangle][dbeMinY] > derbyMaps[i][dBoundsRectangle][dbeMaxY])
				floatswap(derbyMaps[i][dBoundsRectangle][dbeMinY], derbyMaps[i][dBoundsRectangle][dbeMaxY]);
			
			if (derbyMaps[i][dBoundsRectangle][dbeMinZ] > derbyMaps[i][dBoundsRectangle][dbeMaxZ])
				floatswap(derbyMaps[i][dBoundsRectangle][dbeMinZ], derbyMaps[i][dBoundsRectangle][dbeMaxZ]);
		} else {
			derbyMaps[i][dBoundsRectangle][dbeUsed] = 0;
		}
		
		new i2;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "BoundPolyPoint", false)) {
				idx = 0;
				derbyMaps[i][dBoundPoly][E_VERTEX_X][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbyMaps[i][dBoundPoly][E_VERTEX_Y][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_VERTICIES) break; // Maximum polys reached
		}
		derbyMaps[i][dBoundPoly][E_VERTICIES] = i2;
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "PICKUP", false, 6)) {
				idx = 0;
				
				//if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "NITRO", false)) {
				//	derbyPickups[i][i2][dpType] = RACE_PICKUP_TYPE_NOS;
					
				if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "REPAIR", false)) {
					derbyPickups[i][i2][dpType] = RACE_PICKUP_TYPE_REPAIR;
					
				} else if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "VEHICLE", false)) {
					derbyPickups[i][i2][dpType] = RACE_PICKUP_TYPE_VEHICLE;
					derbyPickups[i][i2][dpVehicleModel] = strval(split(gCache[l][E_VALUE], idx, ','));
					
				}
				
				if (derbyPickups[i][i2][dpType]) {
					derbyPickups[i][i2][dpX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					derbyPickups[i][i2][dpY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					derbyPickups[i][i2][dpZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					derbyPickups[i][i2][dpRespawnTime] = strval(split(gCache[l][E_VALUE], idx, ','));
					i2++;
				}
			}
			if (i2 >= MAX_RACE_PICKUPS) break; // Maximum pickups reached
		}
		
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "PICKUPNITRO", false)) {
				idx = 0;
				derbyPickups[i][i2][dpType] = RACE_PICKUP_TYPE_NOS;
				derbyPickups[i][i2][dpX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbyPickups[i][i2][dpY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbyPickups[i][i2][dpZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbyPickups[i][i2][dpRespawnTime] = strval(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_RACE_PICKUPS) break; // Maximum pickups reached
		}
		
		i2 = 0;
		derbyMaps[i][dVehicleSpawnsCount] = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "SPAWN", false)) {
				idx = 0;
				derbySpawns[i][i2][dvX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbySpawns[i][i2][dvY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbySpawns[i][i2][dvZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				derbySpawns[i][i2][dvRotZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				
				i2++;
				derbyMaps[i][dVehicleSpawnsCount] = i2;
			}
			if (i2 >= MAX_DERBY_SPAWNS) break; // Maximum spawns reached
		}
		/*for (new c; c < MAX_DERBY_SPAWNS; c++) {
			format(key, sizeof(key), "Spawn%i", c);
			if (!INI_ReadString(tmpstr, key)) {
				break; // End of spawns
			}
			
			idx = 0;
			derbySpawns[i][c][dvX] = floatstr(split(tmpstr, idx, ','));
			derbySpawns[i][c][dvY] = floatstr(split(tmpstr, idx, ','));
			derbySpawns[i][c][dvZ] = floatstr(split(tmpstr, idx, ','));
			derbySpawns[i][c][dvRotZ] = floatstr(split(tmpstr, idx, ','));
			
			derbyMaps[i][dVehicleSpawnsCount] = c + 1;
		}*/
		
		new notes[MAX_MODE_NOTES];
		if (derbyMaps[i][dVehicleModel] == -1) {
			notes = "Vehicle: Any";
		} else {
			format(notes, sizeof(notes), "Vehicle: %s", GetVehicleName(derbyMaps[i][dVehicleModel]));
		}
		
		//format(derbyname, sizeof(derbyname), "Derby: %s", derbyname);
		//derbyMaps[i][dModeID] = CreateMinimode(MODE_TYPE_DERBY, derbyname, i, minlevel, cost, x, y, z, notes);
		derbyMaps[i][dModeID] = LoadMinimode(MODE_TYPE_DERBY, derbyname, i, notes, 2, derbyMaps[i][dVehicleSpawnsCount] ? derbyMaps[i][dVehicleSpawnsCount] : MAX_PLAYERS, derbyMaps[i][dVehicleSpawnsCount]);
		INI_Close();
		
		if (derbyMaps[i][dModeID] != INVALID_MODE_ID) {
			derbysLoaded++;
		}
	}
	
	printf("[FDM] Derbies Loaded: %i", derbysLoaded);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

UpdateDerbiesInProgress() {
	new indexLoaded;
	
	for (new mapid; mapid < MAX_DERBY_MAPS; mapid++) {
		if (derbyMaps[mapid][dIsLoaded]) {
			derbiesInProgress[indexLoaded] = mapid;
			indexLoaded++;
		}
	}
	
	derbiesInProgress[indexLoaded] = INVALID_MAP_ID;
	return 1;
}

#define LoopDerbiesLoaded(%1) for (new mmc, %1; derbiesInProgress[mmc] != INVALID_MAP_ID; mmc++) if ((%1 = derbiesInProgress[mmc]) || true)


//-----------------------------------------------------------------------------------------------------
// ACTUAL DERBYS
//-----------------------------------------------------------------------------------------------------

derby_IsJoinable(mapid) {
	if (!derbyMaps[mapid][dIsLoaded]) return 0;
	if (derbyMaps[mapid][dCountDown]) return 1;
	return 0;
}

derby_ShowModeInfo(mapid, playerid) {
	new msg[MAX_INPUT];
	format(msg, sizeof(msg), "Starting Vehicle: %s | Vehicle Health: %0.1f", GetVehicleName(derbyMaps[mapid][dVehicleModel]), derbyMaps[mapid][dVehicleHealth]);
	SendClientMessage(playerid, COLOUR_HELP, msg);
	return 1;
}

derby_OnStart(mapid, players[], playercount) { // Derby started
	if (derbyMaps[mapid][dIsLoaded]) return 0;
	if (playercount < 2) return 0; // Not enough players
	
	derbyMaps[mapid][dIsLoaded] = 1;
	derbyMaps[mapid][dCountDown] = COUNTDOWN_TIME; // Start countdown for anyone in the derby
	derbyMaps[mapid][dStartTime] = GetTickCount();
	derbyMaps[mapid][dWinCount] = 0;
	derbyMaps[mapid][dPlayerCount] = 0;
	
	UpdateDerbiesInProgress();
	
	derbyPickupLoad(mapid);
	
	for (new i; i < MAX_DERBY_SPAWNS; i++) {
		derbyMaps[mapid][dSpawnsUsed][i] = 0;
	}
	
	LoopPlayers(i) {
		if (!players[i]) continue;
		if (!derby_OnPlayerEnter(mapid, i, playercount)) {
			players[i] = 0;
		}
	}
	
	return 1;
}

derby_OnStop(mapid) { // Derby stopped
	if (!derbyMaps[mapid][dIsLoaded]) return 0; // Derby already stopped
	derbyMaps[mapid][dIsLoaded] = 0;
	
	UpdateDerbiesInProgress();
	
	derbyPickupUnload(mapid);
	
	LoopPlayers(i) {
		if (derbyPStats[i][dpMapID] != mapid) continue;
		derbyPStats[i][dpMapID] = INVALID_MAP_ID;
		
		// Fail derby for anyone still inside
		SendClientMessage(i, COLOUR_ERROR, "DERBY: You have ran out of time for this derby.");
		derby_OnPlayerExit(mapid, i, 0);
	}
	
	return 1;
}

derby_OnPlayerEnter(mapid, playerid, playercount = 0) { // Player enter derby
	if (!derbyMaps[mapid][dCountDown]) return 0;
	
	/*if (derbyMaps[mapid][dPlayerCount] >= derbyMaps[mapid][dVehicleSpawnsCount]) {
		SendClientMessage(playerid, COLOUR_ERROR, "DERBY: Error - No more spawn points avaliable.");
		return 0;
	}*/
	
	new spawnid;
	new spawnfound;
	
	if (!playercount) {
		for (new i; i < derbyMaps[mapid][dVehicleSpawnsCount]; i++) {
			if (derbyMaps[mapid][dSpawnsUsed][i]) continue;
			spawnid = i;
			spawnfound = 1;
			break;
		}
	} else {
		new avaliablespawns[MAX_DERBY_SPAWNS];
		new avaliablespawnscount;
		
		for (new i; i < playercount && i < derbyMaps[mapid][dVehicleSpawnsCount]; i++) {
			if (derbyMaps[mapid][dSpawnsUsed][i]) continue;
			avaliablespawns[avaliablespawnscount] = i;
			avaliablespawnscount++;
		}
		
		if (avaliablespawnscount) {
			spawnid = avaliablespawns[random(avaliablespawnscount)];
			spawnfound = 1;
		}
	}
	
	if (!spawnfound) {
		SendClientMessage(playerid, COLOUR_ERROR, "DERBY: Error - No more spawn points avaliable.");
		return 0;
	}
	derbyMaps[mapid][dSpawnsUsed][spawnid] = 1;
	
	new vid = GetPlayerVehicleID(playerid);
	if (derbyPStats[playerid][dpVehicleID] == ANY_VEHICLE_ID) {
		// Teleport current vehicle to derby
		if (!vid) {
			SendClientMessage(playerid, COLOUR_ERROR, "DERBY: You do not have a vehicle.");
			return 0;
		}
		derbyPStats[playerid][dpVehicleID] = vid;
		SetVehiclePos(vid, derbySpawns[mapid][spawnid][dvX], derbySpawns[mapid][spawnid][dvY], derbySpawns[mapid][spawnid][dvZ]);
		SetVehicleZAngle(vid, derbySpawns[mapid][spawnid][dvRotZ]);
	} else {
		// Give the player a vehicle
		derbyPStats[playerid][dpVehicleID] = CreateVehicle(derbyMaps[mapid][dVehicleModel],
			derbySpawns[mapid][spawnid][dvX], derbySpawns[mapid][spawnid][dvY], derbySpawns[mapid][spawnid][dvZ], derbySpawns[mapid][spawnid][dvRotZ], -1, -1, 100000);
		
		if (derbyPStats[playerid][dpVehicleID] == INVALID_VEHICLE_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "DERBY: Error - Cannot create your derby vehicle.");
			return 0;
		}
		
		//if (vid) RemovePlayerFromVehicle(playerid);
		PutPlayerInVehicle(playerid, derbyPStats[playerid][dpVehicleID], 0);
		SetVehiclePos(derbyPStats[playerid][dpVehicleID],
			derbySpawns[mapid][spawnid][dvX], derbySpawns[mapid][spawnid][dvY], derbySpawns[mapid][spawnid][dvZ]);
		SetVehicleZAngle(derbyPStats[playerid][dpVehicleID], derbySpawns[mapid][spawnid][dvRotZ]);
		if (modes[derbyMaps[mapid][dModeID]][mFlags] & MODE_FLAG_DAMAGED) UpdateVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
	}
	
	derbyPStats[playerid][dpSpawnX] = derbySpawns[mapid][spawnid][dvX];
	derbyPStats[playerid][dpSpawnY] = derbySpawns[mapid][spawnid][dvY];
	derbyPStats[playerid][dpSpawnZ] = derbySpawns[mapid][spawnid][dvZ];
	derbyPStats[playerid][dpSpawnRotZ] = derbySpawns[mapid][spawnid][dvRotZ];
	derbyPStats[playerid][dpJustSpawned] = 1;
	derbyPStats[playerid][dpDamage] = 0;
	derbyPStats[playerid][dpLastVHP] = derbyMaps[mapid][dVehicleHealth];
	
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		derbyPStats[playerid][dpIsInPickup][i] = 0;
	}
	
	// Mod the car
	for (new d; d < MAX_CAR_COMPONENTS; d++) {
		if (derbyMaps[mapid][dVehicleComponents][d]) {
			AddVehicleComponent(derbyPStats[playerid][dpVehicleID], derbyMaps[mapid][dVehicleComponents][d]);
		}
	}
	
	EnableGodmode(playerid);
	SetPlayerVirtualWorld(playerid, modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
	SetVehicleVirtualWorld(derbyPStats[playerid][dpVehicleID], modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
	LinkVehicleToInterior(derbyPStats[playerid][dpVehicleID], modes[derbyMaps[mapid][dModeID]][mInteriorID]);
	SetVehicleHealth(derbyPStats[playerid][dpVehicleID], derbyMaps[mapid][dVehicleHealth]);
	
	derbyPStats[playerid][dpMapID] = mapid;
	SetCameraBehindPlayer(playerid);
	TogglePlayerControllable(playerid, 0);
	ResetPlayerWeapons(playerid);
	SetPlayerHealth(playerid, 100.0);
	SetPlayerArmour(playerid, 0.0);
	GivePlayerGangColour(playerid);
	LoopPlayers(i) {
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	
	derbyMaps[mapid][dPlayerCount]++;
	return 1;
}

derby_OnPlayerExit(mapid, playerid, allowwinner = 1) { // TODO: Player exit derby
	
	derbyPStats[playerid][dpMapID] = INVALID_MAP_ID;
	SetLastSpawnTick(playerid);
	if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) SetPlayerVirtualWorld(playerid, 0);
	
	if (derbyMaps[mapid][dVehicleModel] == ANY_VEHICLE_ID) {
		// Map vehicle
		if (derbyPStats[playerid][dpVehicleID] != INVALID_VEHICLE_ID) {
			SetVehicleVirtualWorld(derbyPStats[playerid][dpVehicleID], 0);
			LinkVehicleToInterior(derbyPStats[playerid][dpVehicleID], modes[derbyMaps[mapid][dModeID]][mInteriorID]);
			derbyPStats[playerid][dpVehicleID] = INVALID_VEHICLE_ID;
			
			// Teleport back to start line
			if (IsPlayerSpawned(playerid)) {
				teleportPlayerRandom(playerid,
					modes[derbyMaps[mapid][dModeID]][mLocation][mX],
					modes[derbyMaps[mapid][dModeID]][mLocation][mY],
					modes[derbyMaps[mapid][dModeID]][mLocation][mZ], 5);
			}
		} else {
			if (IsPlayerSpawned(playerid)) {
				SetPlayerPosRandom(playerid,
					modes[derbyMaps[mapid][dModeID]][mLocation][mX],
					modes[derbyMaps[mapid][dModeID]][mLocation][mY],
					modes[derbyMaps[mapid][dModeID]][mLocation][mZ], 5);
			}
		}
	} else {
		// Derby vehicle
		if (derbyPStats[playerid][dpVehicleID] != INVALID_VEHICLE_ID) { // Destroy their vehicle
			DestroyVehicle(derbyPStats[playerid][dpVehicleID]);
			derbyPStats[playerid][dpVehicleID] = INVALID_VEHICLE_ID;
		}
		
		// Teleport back to start line
		if (IsPlayerSpawned(playerid)) {
			SetPlayerPosRandom(playerid,
				modes[derbyMaps[mapid][dModeID]][mLocation][mX],
				modes[derbyMaps[mapid][dModeID]][mLocation][mY],
				modes[derbyMaps[mapid][dModeID]][mLocation][mZ], 5);
		}
	}
	
	if (derbyMaps[mapid][dCountDown]) {
		TogglePlayerControllable(playerid, 1); // Player may still be frozen
		derbyMaps[mapid][dPlayerCount]--; // Removes exploit of joining/leaving at countdown to multiply XP
	}
	
	if (IsPlayerSpawned(playerid)) {
		SetPlayerHealth(playerid, 100.0);
		SetCameraBehindPlayer(playerid);
	}
	
	if (!allowwinner) return 1;
	
	// Check for winner
	new playercount;
	new lastpid;
	LoopPlayers(i) {
		if (derbyPStats[i][dpMapID] != mapid) continue;
		playercount++;
		lastpid = i;
	}
	
	new winXPMultiplier;
	new winCashMultiplier;
	
	if (!derbyMaps[mapid][dWinBaseXPMultiplier]) {
		winXPMultiplier = derbyDefaultXPMultiplier;
	} else {
		winXPMultiplier = derbyMaps[mapid][dWinBaseXPMultiplier];
	}
	if (!derbyMaps[mapid][dWinBaseCashMultiplier]) {
		winCashMultiplier = derbyDefaultCashMultiplier;
	} else {
		winCashMultiplier = derbyMaps[mapid][dWinBaseCashMultiplier];
	}
	
	switch (playercount) {
		case 0: {}
		case 1: {
			
			new message[MAX_INPUT];
			new xp, money;
			new Float:px, Float:py, Float:pz;
			
			// This is actually position 2 because of the way the winner is handled
			// Send lose gametext
			format(message, sizeof(message), "%s has lost %s in 2nd place.", pData[playerid][pLoginName], modes[derbyMaps[mapid][dModeID]][mName]);
			SendClientMessageToAll(COLOUR_SUCCESSFUL, message);
			GameTextForPlayer(playerid, "Derby Lost!", 5000, 6);
			
			// Vehicle damage
			new Float: vehicleHealth;
			GetVehicleHealth(derbyPStats[playerid][dpVehicleID], vehicleHealth);
			if (derbyPStats[playerid][dpLastVHP] - vehicleHealth > 0.0) {
				derbyPStats[playerid][dpDamage] += derbyPStats[playerid][dpLastVHP] - vehicleHealth;
			}
			
			new damagexp = floatround((derbyPStats[playerid][dpDamage] / derbyMaps[mapid][dVehicleHealth]) * winXPMultiplier, floatround_round);
			if (damagexp > winXPMultiplier) damagexp = winXPMultiplier;
			
			// Sound
			GetPlayerPos(playerid, px, py, pz);
			PlayerPlaySound(playerid, SOUND_FAIL, px, py, pz);
			
			new drunkxp = GetPlayerDrunkLevel(playerid);
			if (drunkxp < 0) drunkxp = 0;
			if (drunkxp > 5000) drunkxp = 5000;
			drunkxp = floatround(float(drunkxp) * (float(winXPMultiplier) / 5000.0), floatround_floor);
			
			// Give xp and cash
			xp = MinimodeProcessFlagsXP(derbyMaps[mapid][dModeID], DERBY_BASE_XP + (winXPMultiplier + drunkxp + damagexp) * ((GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3) / 2);
			SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, xp));
			
			money = MinimodeProcessFlagsMoney(derbyMaps[mapid][dModeID], DERBY_BASE_CASH + winCashMultiplier * ((GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3) / 2);
			GivePlayerMoney(playerid, money);
			SendMoneyMessageToPlayer(playerid, money);
			
			// Player win
			// Send win gamext
			format(message, sizeof(message), "%s has won %s.", pData[lastpid][pLoginName], modes[derbyMaps[mapid][dModeID]][mName]);
			SendClientMessageToAll(COLOUR_SUCCESSFUL, message);
			GameTextForPlayer(lastpid, "Derby Won!", 5000, 6);
			
			// Vehicle damage
			GetVehicleHealth(derbyPStats[lastpid][dpVehicleID], vehicleHealth);
			if (derbyPStats[lastpid][dpLastVHP] - vehicleHealth > 0.0) {
				derbyPStats[lastpid][dpDamage] += derbyPStats[lastpid][dpLastVHP] - vehicleHealth;
			}
			
			damagexp = floatround((derbyPStats[lastpid][dpDamage] / derbyMaps[mapid][dVehicleHealth]) * winXPMultiplier, floatround_round);
			if (damagexp > winXPMultiplier) damagexp = winXPMultiplier;
			
			// Sound
			GetPlayerPos(lastpid, px, py, pz);
			PlayerPlaySound(lastpid, SOUND_FINISH, px, py, pz);
			
			drunkxp = GetPlayerDrunkLevel(lastpid);
			if (drunkxp < 0) drunkxp = 0;
			if (drunkxp > 5000) drunkxp = 5000;
			drunkxp = floatround(float(drunkxp) * (float(winXPMultiplier) / 5000.0), floatround_floor);
			
			// Give xp and cash
			xp = MinimodeProcessFlagsXP(derbyMaps[mapid][dModeID], DERBY_BASE_XP + (winXPMultiplier + drunkxp + damagexp) * (GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3);
			SendXPMessageToPlayer(lastpid, GivePlayerXP(lastpid, xp));
			
			money = MinimodeProcessFlagsMoney(derbyMaps[mapid][dModeID], DERBY_BASE_CASH + winCashMultiplier * ((GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3));
			GivePlayerMoney(lastpid, money);
			SendMoneyMessageToPlayer(lastpid, money);
			
			// Exit derby
			RemovePlayerFromMode(lastpid, "won");
		}
		default: {
			// Send fail gamext
			new message[MAX_INPUT];
			format(message, sizeof(message), "%s has failed %s in %s place.", pData[playerid][pLoginName], modes[derbyMaps[mapid][dModeID]][mName], racestr(playercount + 1));
			SendClientMessageToAll(COLOUR_SUCCESSFUL, message);
			GameTextForPlayer(playerid, "Derby Failed!", 5000, 6);
			
			// Vehicle damage
			new Float: vehicleHealth;
			GetVehicleHealth(derbyPStats[playerid][dpVehicleID], vehicleHealth);
			if (derbyPStats[playerid][dpLastVHP] - vehicleHealth > 0.0) {
				derbyPStats[playerid][dpDamage] += derbyPStats[playerid][dpLastVHP] - vehicleHealth;
			}
			
			new damagexp = floatround((derbyPStats[playerid][dpDamage] / derbyMaps[mapid][dVehicleHealth]) * winXPMultiplier, floatround_round);
			if (damagexp > winXPMultiplier) damagexp = winXPMultiplier;
			
			// Sound
			new Float:px, Float:py, Float:pz;
			GetPlayerPos(playerid, px, py, pz);
			PlayerPlaySound(playerid, SOUND_FAIL, px, py, pz);
			
			new drunkxp = GetPlayerDrunkLevel(playerid);
			if (drunkxp < 0) drunkxp = 0;
			if (drunkxp > 5000) drunkxp = 5000;
			drunkxp = floatround(float(drunkxp) * (float(winXPMultiplier) / 5000.0), floatround_floor);
			
			// Give xp and cash
			new xp = MinimodeProcessFlagsXP(derbyMaps[mapid][dModeID], DERBY_BASE_XP + (winXPMultiplier + drunkxp + damagexp) * ((GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3) / (playercount + 1));
			SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, xp));
			
			new money = MinimodeProcessFlagsMoney(derbyMaps[mapid][dModeID], DERBY_BASE_CASH + winCashMultiplier * ((GetTickCount() - derbyMaps[mapid][dStartTime]) / 1000 * derbyMaps[mapid][dPlayerCount] / 3) / (playercount + 1));
			GivePlayerMoney(playerid, money);
			SendMoneyMessageToPlayer(playerid, money);
		}
	}
	return 1;
}

derby_OnPlayerDeath(mapid, playerid) {
	FailDerbyForPlayer(playerid);
	SkinsSetNextSpawnPos(playerid,
		modes[derbyMaps[mapid][dModeID]][mLocation][mX], modes[derbyMaps[mapid][dModeID]][mLocation][mY], modes[derbyMaps[mapid][dModeID]][mLocation][mZ], float(random(360)));
	return 1;
}

derby_OnPlayerStateChange(mapid, playerid, newstate, oldstate) {
	if (!derbyMaps[mapid][dCountDown] && oldstate == PLAYER_STATE_DRIVER && newstate != PLAYER_STATE_DRIVER) {
		SetTimerEx("derby_Fail", 750, false, "ii", mapid, playerid);
	}
	return 0;
}

derby_OnVehicleDeath(vehicleid) {
	LoopPlayers(i) {
		if (derbyPStats[i][dpMapID] == INVALID_MAP_ID) continue;
		if (derbyMaps[derbyPStats[i][dpMapID]][dCountDown]) continue;
		if (derbyPStats[i][dpVehicleID] != vehicleid) continue;
		
		if (GetPlayerVehicleID(i) == vehicleid) {
			switch (GetVehicleModel(vehicleid)) {
				case 472,473,493,595,484,430,453,452,446,454,539: { // boats
					new Float:vhp;
					GetVehicleHealth(vehicleid, vhp);
					
					if (vhp >= 250.0) {
						return 0;
					}
				}
				default: {
					SetTimerEx("derby_Fail", 750, false, "ii", derbyPStats[i][dpMapID], i);
					return 0;
				}
			}
		}
		FailDerbyForPlayer(i);
		//SetTimerEx("derby_Fail", 750, false, "ii", derbyPStats[i][dpMapID], i);
		return 0;
	}
	return 0;
}

derby_OnVehicleSpawn(vehicleid) {
	LoopPlayers(i) {
		if (derbyPStats[i][dpMapID] == INVALID_MAP_ID) continue;
		if (derbyPStats[i][dpVehicleID] != vehicleid) continue;
		
		FailDerbyForPlayer(i);
		//SetTimerEx("derby_Fail", 750, false, "ii", derbyPStats[i][dpMapID], i);
		return 0;
	}
	return 0;
}

derby_OnVehicleRepair(vehicleid) {
	LoopPlayers(i) {
		if (derbyPStats[i][dpMapID] == INVALID_MAP_ID) continue;
		if (derbyPStats[i][dpVehicleID] != vehicleid) continue;
		if (modes[derbyMaps[derbyPStats[i][dpMapID]][dModeID]][mFlags] & MODE_FLAG_DAMAGED) {
			UpdateVehicleDamageStatus(derbyPStats[i][dpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
		}
		return 1;
	}
	return 1;
}

forward derby_Fail(mapid, playerid);
public derby_Fail(mapid, playerid) {
	if (derbyPStats[playerid][dpMapID] != mapid) return 1;
	FailDerbyForPlayer(playerid);
	return 1;
}

FailDerbyForPlayer(playerid) { // Fail derby
	// Sound
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	PlayerPlaySound(playerid, SOUND_FAIL, px, py, pz);
	
	// Exit derby
	RemovePlayerFromMode(playerid, "failed");
	
	return 0;
}

forward fdm_derby_checkDerbys();
public fdm_derby_checkDerbys() {
	LoopDerbiesLoaded(i) {
		// Check countdown
		if (derbyMaps[i][dCountDown]) {
			derbyMaps[i][dCountDown]--;
			if (!derbyMaps[i][dCountDown]) {
				LoopPlayers(p) {
					if (derbyPStats[p][dpMapID] != i) continue;
					GameTextForPlayer(p, "GO!", 2000, 6);
					TogglePlayerControllable(p, 1);
					if (derbyPStats[p][dpVehicleID] != INVALID_VEHICLE_ID) {
						if (modes[derbyMaps[i][dModeID]][mFlags] & MODE_FLAG_DAMAGED) {
							UpdateVehicleDamageStatus(derbyPStats[p][dpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
						} else {
							UpdateVehicleDamageStatus(derbyPStats[p][dpVehicleID], 0, 0, 0, 0);
						}
						SetVehicleHealth(derbyPStats[p][dpVehicleID], derbyMaps[i][dVehicleHealth]);
					}
					DisableGodmode(p);
					
					new Float:x, Float:y, Float:z;
					GetPlayerPos(p, x, y, z);
					PlayerPlaySound(p, SOUND_START, x, y, z);
				}
				derbyMaps[i][dStartTime] = GetTickCount();
			} else {
				LoopPlayers(p) {
					if (derbyPStats[p][dpMapID] != i) continue;
					new gametext[MAX_INPUT];
					format(gametext, sizeof(gametext), "%i", derbyMaps[i][dCountDown]);
					GameTextForPlayer(p, gametext, 1000, 6);
					
					new Float:x, Float:y, Float:z;
					GetPlayerPos(p, x, y, z);
					
					PlayerPlaySound(p, SOUND_COUNTDOWN, x, y, z);
					
					if (derbyPStats[p][dpVehicleID] != INVALID_VEHICLE_ID) {
						if (GetPlayerVehicleID(p) != derbyPStats[p][dpVehicleID]) { // If their vehicle is not equal to their given vehicle
							PutPlayerInVehicle(p, derbyPStats[p][dpVehicleID], 0);
							SetCameraBehindPlayer(p);
						} else {
						/*	new Float:dist;
							GetVehiclePos(derbyPStats[p][dpVehicleID], x, y, z);
							dist = FindDistance(x, y, z,
								derbyPStats[p][dpSpawnX], derbyPStats[p][dpSpawnY], derbyPStats[p][dpSpawnZ]);
							
							if (dist > 1.0) {
								SetVehiclePos(derbyPStats[p][dpVehicleID],
									derbyPStats[p][dpSpawnX],
									derbyPStats[p][dpSpawnY],
									derbyPStats[p][dpSpawnZ]);
								SetVehicleZAngle(derbyPStats[p][dpVehicleID],
									derbyPStats[p][dpSpawnRotZ]);
								SetCameraBehindPlayer(p);
							}
						}*/
							new setpos;
							if (derbyPStats[p][dpJustSpawned]) {
								setpos = 1;
								derbyPStats[p][dpJustSpawned] = 0;
							} else {
								if (modeObjects[derbyMaps[i][dModeID]][0][oModelID]) {
									if (derbyMaps[i][dCountDown] == 1) {
										setpos = 1;
									} else {
										new Float:dist;
										GetVehiclePos(derbyPStats[p][dpVehicleID], x, y, z);
										dist = FindDistance(x, y, z,
											derbyPStats[p][dpSpawnX], derbyPStats[p][dpSpawnY], derbyPStats[p][dpSpawnZ]);
										if (dist > 1.0) setpos = 1;
									}
								}
							}
							if (setpos) {
								SetVehiclePos(derbyPStats[p][dpVehicleID],
									derbyPStats[p][dpSpawnX],
									derbyPStats[p][dpSpawnY],
									derbyPStats[p][dpSpawnZ]);
								SetVehicleZAngle(derbyPStats[p][dpVehicleID],
									derbyPStats[p][dpSpawnRotZ]);
								SetCameraBehindPlayer(p);
							}
						}
					}
				}
			}
		} else {
			// Check player distances for derby's
			LoopPlayers(p) {
				if (derbyPStats[p][dpMapID] != i) continue;
				
				if (GetPlayerVehicleID(p) != derbyPStats[p][dpVehicleID]) {
					FailDerbyForPlayer(p);
					continue;
				}
				
				if (derbyMaps[i][dBoundsRadius][dbrUsed] || derbyMaps[i][dBoundsRectangle][dbeUsed] || derbyMaps[i][dBoundPoly][E_VERTICIES] >= 2) {
					new Float:x, Float:y, Float:z, outofbounds;
					GetVehiclePos(derbyPStats[p][dpVehicleID], x, y, z);
					
					// Rectangular check
					if (derbyMaps[i][dBoundsRectangle][dbeUsed]) {
						if (derbyMaps[i][dBoundsRectangle][dbeMinX] > x) {
							outofbounds = 1;
						} else if (derbyMaps[i][dBoundsRectangle][dbeMaxX] < x) {
							outofbounds = 1;
						} else if (derbyMaps[i][dBoundsRectangle][dbeMinY] > y) {
							outofbounds = 1;
						} else if (derbyMaps[i][dBoundsRectangle][dbeMaxY] < y) {
							outofbounds = 1;
						} else if (derbyMaps[i][dBoundsRectangle][dbeMinZ] > z) {
							outofbounds = 1;
						} else if (derbyMaps[i][dBoundsRectangle][dbeMaxZ] < z) {
							outofbounds = 1;
						}
					}
					
					// Ciricular check
					if (!outofbounds && derbyMaps[i][dBoundsRadius][dbrUsed]) {
						new Float:distance = FindDistance(x, y, z,
							derbyMaps[i][dBoundsRadius][dbCentreX], derbyMaps[i][dBoundsRadius][dbCentreY], derbyMaps[i][dBoundsRadius][dbCentreZ]);
						
						if (distance > derbyMaps[i][dBoundsRadius][dbRadius]) {
							outofbounds = 1;
						}
					}
					
					// Polygonal check
					if (!outofbounds && derbyMaps[i][dBoundPoly][E_VERTICIES] >= 2) {
						new tmp[E_POLYGON];
						tmp[E_VERTICIES] = derbyMaps[i][dBoundPoly][E_VERTICIES];
						for (new j; j < tmp[E_VERTICIES]; j++) {
							tmp[E_VERTEX_X][j] = derbyMaps[i][dBoundPoly][E_VERTEX_X][j];
							tmp[E_VERTEX_Y][j] = derbyMaps[i][dBoundPoly][E_VERTEX_Y][j];
						}
						if (!IsPointInPoly(x, y, tmp)) {
							outofbounds = 1;
						}
					}
					
					if (outofbounds) {
						SendClientMessage(p, COLOUR_ATTENTION, "DERBIES: You are outside of the derby zone!");
						FailDerbyForPlayer(p);
					}
				}
			}
			
			// Check time limit on each derby
			if (GetTickCount() - derbyMaps[i][dStartTime] > derbyMaps[i][dRunTime] * 1000) {
				StopMinimode(derbyMaps[i][dModeID]); // Time for derby to end
			}
			
		}
		
	}
}

DerbySwitchPlayerVehicle(mapid, playerid, vehiclemodel) {
	new Float:x, Float:y, Float:z, Float:vx, Float:vy, Float:vz, Float:rotz, Float:vehiclehp, dmg[4];
	
	if (derbyPStats[playerid][dpVehicleID] != INVALID_VEHICLE_ID) {
		if (GetVehicleModel(derbyPStats[playerid][dpVehicleID]) == vehiclemodel) {
			return 1;
		}
		GetVehicleVelocity(derbyPStats[playerid][dpVehicleID], vx, vy, vz);
		GetVehiclePos(derbyPStats[playerid][dpVehicleID], x, y, z);
		GetVehicleZAngle(derbyPStats[playerid][dpVehicleID], rotz);
		GetVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], dmg[0], dmg[1], dmg[2], dmg[3]);
	} else {
		GetPlayerVelocity(playerid, vx, vy, vz);
		GetPlayerPos(playerid, x, y, z);
		GetPlayerFacingAngle(playerid, rotz);
	}
	
	// Lag fixer
	if (vx == 0 && vy == 0 && vz == 0 && derbyPStats[playerid][dpLastSwitchTick] + 1000 > GetTickCount()) {
		vx = derbyPStats[playerid][dpLastSX];
		vy = derbyPStats[playerid][dpLastSY];
		vz = derbyPStats[playerid][dpLastSZ];
	} else {
		derbyPStats[playerid][dpLastSX] = vx;
		derbyPStats[playerid][dpLastSY] = vy;
		derbyPStats[playerid][dpLastSZ] = vz;
	}
	derbyPStats[playerid][dpLastSwitchTick] = GetTickCount();
	
	if (vehiclemodel) {
		new oldvid = derbyPStats[playerid][dpVehicleID];
		derbyPStats[playerid][dpVehicleID] = CreateVehicle(vehiclemodel, x, y, z, rotz, -1, -1, 100000);
		
		if (!derbyPStats[playerid][dpVehicleID]) {
			SendClientMessage(playerid, COLOUR_ERROR, "RACE: Error - cannot create vehicle for switching.");
			RemovePlayerFromMode(playerid, "left", "(Error)");
			return 1;
		}
		
		SetVehicleVirtualWorld(derbyPStats[playerid][dpVehicleID], modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
		LinkVehicleToInterior(derbyPStats[playerid][dpVehicleID], GetPlayerInterior(playerid));
		
		if (modes[derbyMaps[mapid][dModeID]][mFlags] & MODE_FLAG_DAMAGED) {
			UpdateVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
		} else {
			UpdateVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], dmg[0], dmg[1], dmg[2], dmg[3]);
		}
		
		if (oldvid) {
			GetVehicleHealth(oldvid, vehiclehp);
			DestroyVehicle(oldvid);
		} else {
			vehiclehp = derbyMaps[mapid][dVehicleHealth];
		}
		
		PutPlayerInVehicle(playerid, derbyPStats[playerid][dpVehicleID], 0);
		
		//SetVehiclePos(derbyPStats[playerid][dpVehicleID], x, y, z);
		//SetVehicleZAngle(derbyPStats[playerid][dpVehicleID], rotz);
		SetVehicleVelocity(derbyPStats[playerid][dpVehicleID], vx, vy, vz);
		
		SetVehicleHealth(derbyPStats[playerid][dpVehicleID], vehiclehp);
	} else {
		DestroyVehicle(derbyPStats[playerid][dpVehicleID]);
		derbyPStats[playerid][dpIsRaceVehicle] = 0;
		
		derbyPStats[playerid][dpVehicleID] = INVALID_VEHICLE_ID;
		
		SetPlayerPos(playerid, x, y, z);
		SetPlayerFacingAngle(playerid, rotz);
		SetPlayerVelocity(playerid, vx, vy, vz);
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// PICKUPS
//-----------------------------------------------------------------------------------------------------

forward fdm_derby_PickupCheck();
public fdm_derby_PickupCheck() {
	LoopPlayers(playerid) {
		if (derbyPStats[playerid][dpPlayerUpdated]) {
			new mapid = derbyPStats[playerid][dpMapID];
			if (mapid == INVALID_MAP_ID) continue;
			
			// Syncing player armour to vehicle hp
			new Float:syncArmour;
			if (derbyPStats[playerid][dpVehicleID]) {
				GetVehicleHealth(derbyPStats[playerid][dpVehicleID], syncArmour);
				syncArmour = ((syncArmour - 245) / (derbyMaps[mapid][dVehicleHealth] -  245)) * 100.0;
				if (syncArmour < 0.0) syncArmour = 0.0;
				if (syncArmour > 100.0) syncArmour = 100.0;
				syncArmour = float(floatround(syncArmour, floatround_round));
			} else {
				syncArmour = 0.0;
			}
			
			new Float:playerArmour;
			GetPlayerArmour(playerid, playerArmour);
			if (playerArmour != syncArmour) {
				SetPlayerArmour(playerid, syncArmour);
			}
			
			// Updating pickups
			if (modes[derbyMaps[mapid][dModeID]][mVirtualWorld] != GetPlayerVirtualWorld(playerid)) continue;
			for (new derbypickupid; derbypickupid < MAX_RACE_PICKUPS; derbypickupid++) {
				if (derbyPickups[mapid][derbypickupid][dpPickupID] == INVALID_PICKUP_ID) break;
				
				if (IsPlayerInRangeOfPoint(playerid, 4.0, derbyPickups[mapid][derbypickupid][dpX], derbyPickups[mapid][derbypickupid][dpY], derbyPickups[mapid][derbypickupid][dpZ])) {
					if (!derbyPStats[playerid][dpIsInPickup][derbypickupid]) {
						derbyPStats[playerid][dpIsInPickup][derbypickupid] = 1;
						derby_OnPlayerPickUpPickup(playerid, derbyPickups[mapid][derbypickupid][dpPickupID]);
					}
				} else {
					derbyPStats[playerid][dpIsInPickup][derbypickupid] = 0;
				}
			}
			
			derbyPStats[playerid][dpPlayerUpdated] = 0;
		}
	}
	return 1;
}

derby_OnPlayerUpdate(playerid) {
	derbyPStats[playerid][dpPlayerUpdated] = 1;
	return 1;
}

derby_OnPlayerPickUpPickup(playerid, pickupid) {
	new mapid = derbyPickupsLink[pickupid][dblMapID];
	new derbypickupid = derbyPickupsLink[pickupid][dblPickupID];
	
	if (mapid == INVALID_MAP_ID ||
		derbypickupid == INVALID_PICKUP_ID ||
		mapid != modes[derbyMaps[mapid][dModeID]][mMapID]) return 1;
	
	if (derbyPStats[playerid][dpVehicleID] == INVALID_VEHICLE_ID) {
		if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return 1;
	} else {
		if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER || GetPlayerVehicleID(playerid) != derbyPStats[playerid][dpVehicleID]) return 1;
	}
	
	new pickupvalid = 0;
	
	switch (derbyPickups[mapid][derbypickupid][dpType]) {
		case RACE_PICKUP_TYPE_REPAIR: {
			if (derbyPStats[playerid][dpVehicleID] != INVALID_VEHICLE_ID) {
				new Float: vehicleHealth;
				GetVehicleHealth(derbyPStats[playerid][dpVehicleID], vehicleHealth);
				if (derbyPStats[playerid][dpLastVHP] - vehicleHealth > 0.0) {
					derbyPStats[playerid][dpDamage] += derbyPStats[playerid][dpLastVHP] - vehicleHealth;
				}
				derbyPStats[playerid][dpLastVHP] = derbyMaps[mapid][dVehicleHealth];
				
				if (modes[derbyMaps[mapid][dModeID]][mFlags] & MODE_FLAG_DAMAGED) {
					UpdateVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
				} else {
					UpdateVehicleDamageStatus(derbyPStats[playerid][dpVehicleID], 0, 0, 0, 0);
				}
				SetVehicleHealth(derbyPStats[playerid][dpVehicleID], derbyMaps[mapid][dVehicleHealth]);
			}
			SetPlayerHealth(playerid, 100.0);
			pickupvalid = 1;
		}
		case RACE_PICKUP_TYPE_NOS: {
			if (derbyPStats[playerid][dpVehicleID] != INVALID_VEHICLE_ID) {
				AddVehicleComponent(derbyPStats[playerid][dpVehicleID], 1010);
			}
			pickupvalid = 1;
		}
		case RACE_PICKUP_TYPE_VEHICLE: {
			DerbySwitchPlayerVehicle(mapid, playerid, derbyPickups[mapid][derbypickupid][dpVehicleModel]);
			pickupvalid = 1;
		}
	}
	
	if (pickupvalid) {
		new Float:pX, Float:pY, Float:pZ;
		GetPlayerPos(playerid, pX, pY, pZ);
		PlayerPlaySound(playerid, RACE_PICKUP_SOUND, pX, pY, pZ);
		
		// Spectator support
		LoopPlayers(i) {
			if (GetPlayerSpectatingPlayer(i) != playerid) continue;
			PlayerPlaySound(i, RACE_PICKUP_SOUND, pX, pY, pZ);
		}
	}
	
	return 1;
}

derbyPickupLoad(mapid) {
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		switch (derbyPickups[mapid][i][dpType]) {
			case RACE_PICKUP_TYPE_REPAIR: {
				derbyPickups[mapid][i][dpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_REPAIR, RACE_PICKUP_TYPE, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ], modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
				derbyPickups[mapid][i][dpTextID] = CreateDynamic3DTextLabel("Repair", 0xAAFFAAFF, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[derbyMaps[mapid][dModeID]][mVirtualWorld], -1, -1, 120.0);
			}
			case RACE_PICKUP_TYPE_NOS: {
				derbyPickups[mapid][i][dpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_NOS, RACE_PICKUP_TYPE, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ], modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
				derbyPickups[mapid][i][dpTextID] = CreateDynamic3DTextLabel("Nitrous", 0xAAAAFFFF, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[derbyMaps[mapid][dModeID]][mVirtualWorld], -1, -1, 120.0);
			}
			case RACE_PICKUP_TYPE_VEHICLE: {
				derbyPickups[mapid][i][dpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_VEHICLE, RACE_PICKUP_TYPE, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ], modes[derbyMaps[mapid][dModeID]][mVirtualWorld]);
				derbyPickups[mapid][i][dpTextID] = CreateDynamic3DTextLabel(GetVehicleName(derbyPickups[mapid][i][dpVehicleModel]), 0xFFFFFFFF, derbyPickups[mapid][i][dpX], derbyPickups[mapid][i][dpY], derbyPickups[mapid][i][dpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[derbyMaps[mapid][dModeID]][mVirtualWorld], -1, -1, 120.0);
			}
		}
		
		if (derbyPickups[mapid][i][dpPickupID] != INVALID_PICKUP_ID) {
			derbyPickupsLink[derbyPickups[mapid][i][dpPickupID]][dblMapID] = mapid;
			derbyPickupsLink[derbyPickups[mapid][i][dpPickupID]][dblPickupID] = i;
		}
	}
	return 1;
}

derbyPickupUnload(mapid) {
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		if (derbyPickups[mapid][i][dpPickupID] != INVALID_PICKUP_ID) {
			DestroyDynamicPickup(derbyPickups[mapid][i][dpPickupID]);
			
			derbyPickupsLink[derbyPickups[mapid][i][dpPickupID]][dblMapID] = INVALID_MAP_ID;
			derbyPickupsLink[derbyPickups[mapid][i][dpPickupID]][dblPickupID] = INVALID_PICKUP_ID;
			derbyPickups[mapid][i][dpPickupID] = INVALID_PICKUP_ID;
		}
		if (derbyPickups[mapid][i][dpTextID] != Text3D:INVALID_3DTEXT_ID) {
			DestroyDynamic3DTextLabel(derbyPickups[mapid][i][dpTextID]);
			derbyPickups[mapid][i][dpTextID] = Text3D:INVALID_3DTEXT_ID;
		}
	}
	return 1;
}

