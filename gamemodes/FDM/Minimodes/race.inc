#include <a_samp>

#define FILE_RACE "FDM/Minimodes/RaceList.ini"
#define FILE_RACE_MAPS "FDM/Minimodes/Race/"
#define FILE_RACE_STATS "FDM/Minimodes/Race/Stats/"

#if defined CRIPPLE_PLAYERS
	#define MAX_RACE_MAPS 12
#else
	#define MAX_RACE_MAPS 1024
#endif
#if defined CRIPPLE_PLAYERS
	#define MAX_RACE_RECORDS 6
#else
	#define MAX_RACE_RECORDS 32
#endif
#define MAX_RACE_CHECKPOINTS 512
#define MAX_RACE_SPAWNS 128
#define MAX_RACE_WARNINGS 2

#define RACE_DEFAULT_CP_SIZE 10.0

#define RACE_SOUND_CHECKPOINT 1058

#define RACE_ICON_ID 31
#define RACE_ICON_TYPE 0
#define RACE_ICON_COLOUR 0x99000060

#define INVALID_PICKUP_ID -1
#define MAX_RACE_PICKUPS 256
#define RACE_PICKUP_TYPE 1
#define RACE_PICKUP_SOUND 1133

#define RACE_PICKUP_TYPE_REPAIR 1
#define RACE_PICKUP_TYPE_NOS 2
#define RACE_PICKUP_TYPE_VEHICLE 3

#define RACE_PICKUP_MODEL_REPAIR 3096
#define RACE_PICKUP_MODEL_NOS 1010
#define RACE_PICKUP_MODEL_VEHICLE 1001 //3534

#define RACE_VEHICLE_DAMAGED_STATUS 0x33333333, 0x04040404, 0b1111, 0b1111

enum raceCheckpointInfo {
	Float:rcX,
	Float:rcY,
	Float:rcZ,
	Float:rcSize,
	rcType,
	rccVehicleModel,
	Float:rcVehicleHP,
	rcInterior,
	#if defined _YSI_included
		Float:rcGravity,
	#endif
}
enum racePickupInfo {
	rcpType,
	Float:rcpX,
	Float:rcpY,
	Float:rcpZ,
	rcpRespawnTime,
	rcpVehicleModel,
	rcpPickupID,
	Text3D:rcpTextID
}
enum racesInfo {
	rcIsLoaded,
	rcModeID,
	rcStartTime,
	rcRunTime,
	rcWinBaseXP,
	rcWinBaseCash,
	rcCheckpointCount,
	rcWinPlayerCount,
	rcCountDown,
	rcVehicleModel,
	Float:rcVehicleHealth,
	rcVehicleComponents[MAX_CAR_COMPONENTS],
	rcRecordFile[MAX_FILENAME],
	rcPlayerCount,
	rcTimeModified,
	rcTimeHour,
	rcTimeMinute,
	rcWeather,
	rcVehicleSpawnsCount,
	rcSpawnsUsed[MAX_RACE_SPAWNS],
	Float:rcDamageMultiplier,
	rcWinCount,
	rcFailCount,
	rcLeftCount,
	rcWinTime,
	rcFailTime,
	rcLeftTime
}
enum raceRecordInfo {
	rcPlayer[MAX_NAME],
	rcTime // In milliseconds
}
enum raceVehicleSpawnsInfo {
	Float:rcvX,
	Float:rcvY,
	Float:rcvZ,
	Float:rcvRotZ,
	rcvVehicleModel
}
enum racePlayerStats {
	rcMapID,
	rcCheckpointID,
	rcpVehicleID,
	rcpIsRaceVehicle,
	rcpIsTextDrawOn,
	Text:rcpTextCheckpoint,
	Text:rcpTextTimer,
	Text:rcpTextPosition,
	rcpTextPositionStr[MAX_INPUT],
	Float:rcpSpawnX,
	Float:rcpSpawnY,
	Float:rcpSpawnZ,
	Float:rcpSpawnRotZ,
	rcpJustSpawned,
	Float:rcpDamage,
	Float:rcpLastVHP,
	Float:rcpLastPHP,
	rcpWarnings,
	rcpPlayerUpdated,
	rcpIsInPickup[MAX_RACE_PICKUPS],
	rcpPickupAbused,
	Float:rcpLastX,
	Float:rcpLastY,
	Float:rcpLastZ,
	Float:rcpLastRotZ,
	Float:rcpLastSX,
	Float:rcpLastSY,
	Float:rcpLastSZ,
	Float:rcpLastSwitchTick,
	rcpPlayTimePaid
}
enum racePickupLink {
	rcplMapID,
	rcplPickupID
}
new racePStats[MAX_PLAYERS][racePlayerStats];
new racePickupsLink[MAX_PICKUPS][racePickupLink];
new raceMaps[MAX_RACE_MAPS][racesInfo];
new raceMapCPs[MAX_RACE_MAPS][MAX_RACE_CHECKPOINTS][raceCheckpointInfo];
new raceMapPickups[MAX_RACE_MAPS][MAX_RACE_PICKUPS][racePickupInfo];
new raceMapRecords[MAX_RACE_MAPS][MAX_RACE_RECORDS][raceRecordInfo];
new raceSpawns[MAX_RACE_MAPS][MAX_RACE_SPAWNS][raceVehicleSpawnsInfo];
// new raceTimer;
// new raceTimerFast;
// new racePickupCheck;
new raceEnableDamagePenalty;
new raceEnableDynamicWinnings;
new raceDynamicXPMultiplier;
new raceDynamicCashMultiplier;

new racesInProgress[MAX_RACE_MAPS + 1];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

RaceSettingsInit() {
	raceEnableDynamicWinnings = INI_ReadInt("RaceDynamicWinnings");
	raceDynamicXPMultiplier = INI_ReadInt("RaceDynamicXPMult");
	raceDynamicCashMultiplier = INI_ReadInt("RaceDynamicCashMult");
	raceEnableDamagePenalty = INI_ReadInt("RaceEnableDamagePenalty");
	return 1;
}

RacesInit() {
	for (new i; i < MAX_RACE_MAPS; i++) {
		raceMaps[i][rcModeID] = INVALID_MODE_ID;
		for (new j; j < MAX_RACE_PICKUPS; j++) {
			raceMapPickups[i][j][rcpPickupID] = INVALID_PICKUP_ID;
			raceMapPickups[i][j][rcpTextID] = Text3D:INVALID_3DTEXT_ID;
		}
	}
	for (new i; i < MAX_PICKUPS; i++) {
		racePickupsLink[i][rcplMapID] = INVALID_MAP_ID;
		racePickupsLink[i][rcplPickupID] = INVALID_PICKUP_ID;
	}
	for (new i; i < MAX_PLAYERS; i++) {
		racePStats[i][rcMapID] = INVALID_MAP_ID;
		racePStats[i][rcpVehicleID] = INVALID_VEHICLE_ID;
	}
	racesInProgress[0] = INVALID_MAP_ID;
	
	LoadRaceMaps();
	
	kEnableTimer(11); // raceTimer = SetTimer("fdm_race_checkRaces", 925, true);
	kEnableTimer(12); // raceTimerFast = SetTimer("fdm_race_raceTimes", 200, true);
	kEnableTimer(13); // racePickupCheck = SetTimer("fdm_race_PickupCheck", 16, true);
	return 1;
}

RacesUnload() {
	new tmp1[racePlayerStats];
	new tmp2[racesInfo];
	new tmp3[raceCheckpointInfo];
	new tmp4[raceRecordInfo];
	new tmp5[raceVehicleSpawnsInfo];
	new tmp6[racePickupInfo];
	new tmp7[racePickupLink];
	
	RaceUpdatePlayerInfoUnload();
	
	for (new a; a < MAX_PLAYERS; a++) {
		racePStats[a] = tmp1;
	}
	
	for (new a; a < MAX_PICKUPS; a++) {
		racePickupsLink[a] = tmp7;
	}
	
	for (new b; b < MAX_RACE_MAPS; b++) {
		raceMaps[b] = tmp2;
		for (new c; c < MAX_RACE_CHECKPOINTS; c++) {
			raceMapCPs[b][c] = tmp3;
		}
		for (new c; c < MAX_RACE_RECORDS; c++) {
			raceMapRecords[b][c] = tmp4;
		}
		for (new c; c < MAX_RACE_SPAWNS; c++) {
			raceSpawns[b][c] = tmp5;
		}
		for (new c; c < MAX_RACE_PICKUPS; c++) {
			raceMapPickups[b][c] = tmp6;
		}
	}
	
	kDisableTimer(11); // KillTimer(raceTimer);
	kDisableTimer(12); // KillTimer(raceTimerFast);
	kDisableTimer(13); // KillTimer(racePickupCheck);
	return 1;
}

LoadRaceMaps() {
	if (!INI_Exist(FILE_RACE)) {
		printf("[FDM] WARNING: Race Map List Fail: '%s' - File not found.", FILE_RACE);
		return 0;
	}
	
	if (!INI_Open(FILE_RACE)) {
		printf("[FDM] WARNING: Race Map List Fail: '%s' - File cannot be opened.", FILE_RACE);
		return 0;
	}
	
	new raceFiles[MAX_RACE_MAPS][MAX_FILENAME];
	new racecount;
	new racesLoaded;
	//new key[INI_MAX_KEY];
	
	new i3;
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "RACE", false)) {
			format(raceMaps[i3][rcRecordFile], MAX_FILENAME, "%s%s", FILE_RACE_STATS, gCache[l][E_VALUE]);
			format(raceFiles[i3], MAX_FILENAME, "%s%s", FILE_RACE_MAPS, gCache[l][E_VALUE]);
			i3++;
			racecount = i3;
		}
		if (i3 >= MAX_RACE_MAPS) {
			printf("[FDM] WARNING: Maximum race maps reached (%i races)", MAX_RACE_MAPS);
			break; // Maximum races reached
		}
	}
	/*for (new i; i < MAX_RACE_MAPS; i++) { // Read race list
		format(key, sizeof(key), "RACE%i", i);
		
		if (INI_ReadString(raceFiles[i], key)) {
			// Add the file path to the beginning
			format(raceMaps[i][rcRecordFile], MAX_FILENAME, "%s%s", FILE_RACE_STATS, raceFiles[i]);
			format(raceFiles[i], MAX_FILENAME, "%s%s", FILE_RACE_MAPS, raceFiles[i]);
		} else {
			racecount = i;
			break; // End of file
		}
	}*/
	
	INI_Close();
	
	if (!racecount) return 0; // There are no races to load
	
	for (new i; i < racecount; i++) {
		if (!INI_Exist(raceFiles[i])) {
			printf("[FDM] WARNING: Race Load Fail: '%s' - File not found.", raceFiles[i]);
			continue;
		}
		if (!INI_Open(raceFiles[i])) {
			printf("[FDM] WARNING: Race Load Fail: '%s' - File cannot be opened.", raceFiles[i]);
			continue;
		}
		
		new racename[MAX_MODE_NAME];
		new tmpstr[MAX_STRING];
		//new minlevel;
		//new cost;
		new idx;
		
		INI_ReadString(racename, "Name");
		if (!racename[0]) { // Invalid race name
			INI_Close();
			printf("[FDM] WARNING: Race Load Fail: '%s' - File is corrupt/invalid.", raceFiles[i]);
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
		
		// Race data
		raceMaps[i][rcRunTime] = INI_ReadInt("RunTime");
		raceMaps[i][rcWinBaseXP] = INI_ReadInt("WinBaseXP");
		raceMaps[i][rcWinBaseCash] = INI_ReadInt("WinBaseCash");
		raceMaps[i][rcVehicleModel] = INI_ReadInt("Vehicle");
		
		if (INI_ReadString(tmpstr, "DamageWinMult")) {
			raceMaps[i][rcDamageMultiplier] = floatstr(tmpstr);
		} else {
			raceMaps[i][rcDamageMultiplier] = 1.0;
		}
		
		// Vehicle components
		raceMaps[i][rcVehicleHealth] = INI_ReadFloat("VehicleHealth");
		if (raceMaps[i][rcVehicleHealth] == 0.0) raceMaps[i][rcVehicleHealth] = 1000.0;
		if (INI_ReadString(tmpstr, "VehicleComponents")) {
			idx = 0;
			for (new d; d < MAX_CAR_COMPONENTS; d++) {
				raceMaps[i][rcVehicleComponents][d] = strval(split(tmpstr, idx, ','));
				if (!raceMaps[i][rcVehicleComponents][d]) break;
			}
		}
		
		new i2;
		raceMaps[i][rcCheckpointCount] = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "CP", false)) {
				idx = 0;
				
				// Checkpoint data
				raceMapCPs[i][i2][rcX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapCPs[i][i2][rcY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapCPs[i][i2][rcZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapCPs[i][i2][rcType] = strval(split(gCache[l][E_VALUE], idx, ','));
				raceMapCPs[i][i2][rcSize] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				if (raceMapCPs[i][i2][rcSize] == 0.0) {
					raceMapCPs[i][i2][rcSize] = RACE_DEFAULT_CP_SIZE;
				}
				
				// Vehicle switching
				strcpy(tmpstr, split(gCache[l][E_VALUE], idx, ','));
				if (!tmpstr[0]) {
					raceMapCPs[i][i2][rccVehicleModel] = ANY_VEHICLE_ID;
				} else {
					raceMapCPs[i][i2][rccVehicleModel] = strval(tmpstr);
				}
				
				// Vehicle repairing
				strcpy(tmpstr, split(gCache[l][E_VALUE], idx, ','));
				if (!tmpstr[0]) {
					raceMapCPs[i][i2][rcVehicleHP] = 0.0;
				} else {
					raceMapCPs[i][i2][rcVehicleHP] = floatstr(tmpstr);
				}
				
				// Interior changing
				strcpy(tmpstr, split(gCache[l][E_VALUE], idx, ','));
				if (!tmpstr[0]) {
					raceMapCPs[i][i2][rcInterior] = -1;
				} else {
					raceMapCPs[i][i2][rcInterior] = strval(tmpstr);
				}
				
				// Gravity changing
				#if defined _YSI_included
					splitcpy(tmpstr, gCache[l][E_VALUE], idx, ',');
					if (!tmpstr[0]) {
						raceMapCPs[i][i2][rcGravity] = INVALID_GRAVITY;
					} else {
						raceMapCPs[i][i2][rcGravity] = floatstr(tmpstr);
						if (raceMapCPs[i][i2][rcGravity] < -1.0 ||
							raceMapCPs[i][i2][rcGravity] > 1.0) {
							
							raceMapCPs[i][i2][rcGravity] = INVALID_GRAVITY;
						}
					}
				#endif
				
				i2++;
				raceMaps[i][rcCheckpointCount] = i2;
			}
			if (i2 >= MAX_RACE_CHECKPOINTS) break; // Maximum checkpoints reached
		}
		/*for (new c; c < MAX_RACE_CHECKPOINTS; c++) {
			format(key, sizeof(key), "CP%i", c);
			if (!INI_ReadString(tmpstr, key)) {
				break; // End of checkpoints
			}
			
			idx = 0;
			raceMapCPs[i][c][rcX] = floatstr(split(tmpstr, idx, ','));
			raceMapCPs[i][c][rcY] = floatstr(split(tmpstr, idx, ','));
			raceMapCPs[i][c][rcZ] = floatstr(split(tmpstr, idx, ','));
			raceMapCPs[i][c][rcType] = strval(split(tmpstr, idx, ','));
			raceMapCPs[i][c][rcSize] = floatstr(split(tmpstr, idx, ','));
			
			if (raceMapCPs[i][c][rcSize] == 0.0) {
				raceMapCPs[i][c][rcSize] = 8.0;
			}
			
			raceMaps[i][rcCheckpointCount] = c + 1;
		}*/
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "PICKUP", false, 6)) {
				idx = 0;
				
				//if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "NITRO", false)) {
				//	raceMapPickups[i][i2][rcpType] = RACE_PICKUP_TYPE_NOS;
					
				if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "REPAIR", false)) {
					raceMapPickups[i][i2][rcpType] = RACE_PICKUP_TYPE_REPAIR;
					
				} else if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY][6], "VEHICLE", false)) {
					raceMapPickups[i][i2][rcpType] = RACE_PICKUP_TYPE_VEHICLE;
					raceMapPickups[i][i2][rcpVehicleModel] = strval(split(gCache[l][E_VALUE], idx, ','));
					
				}
				
				if (raceMapPickups[i][i2][rcpType]) {
					raceMapPickups[i][i2][rcpX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					raceMapPickups[i][i2][rcpY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					raceMapPickups[i][i2][rcpZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
					raceMapPickups[i][i2][rcpRespawnTime] = strval(split(gCache[l][E_VALUE], idx, ','));
					i2++;
				}
			}
			if (i2 >= MAX_RACE_PICKUPS) break; // Maximum pickups reached
		}
		
		// Bugfix: Load nitro pickups after the vehicle pickups
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "PICKUPNITRO", false)) {
				idx = 0;
				raceMapPickups[i][i2][rcpType] = RACE_PICKUP_TYPE_NOS;
				raceMapPickups[i][i2][rcpX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapPickups[i][i2][rcpY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapPickups[i][i2][rcpZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceMapPickups[i][i2][rcpRespawnTime] = strval(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_RACE_PICKUPS) break; // Maximum pickups reached
		}
		
		i2 = 0;
		raceMaps[i][rcVehicleSpawnsCount] = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "SPAWN", false)) {
				idx = 0;
				raceSpawns[i][i2][rcvX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceSpawns[i][i2][rcvY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceSpawns[i][i2][rcvZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				raceSpawns[i][i2][rcvRotZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				
				splitcpy(tmpstr, gCache[l][E_VALUE], idx, ',');
				if (!tmpstr[0]) {
					raceSpawns[i][i2][rcvVehicleModel] = -2;
				} else {
					raceSpawns[i][i2][rcvVehicleModel] = strval(tmpstr);
				}
				
				i2++;
				raceMaps[i][rcVehicleSpawnsCount] = i2;
			}
			if (i2 >= MAX_RACE_SPAWNS) break; // Maximum spawns reached
		}
		
		new notes[MAX_MODE_NOTES];
		if (raceMaps[i][rcVehicleModel] == -1) {
			notes = "Vehicle: Your Own";
		} else {
			format(notes, sizeof(notes), "Vehicle: %s", GetVehicleName(raceMaps[i][rcVehicleModel]));
		}
		
		//format(racename, sizeof(racename), "Race: %s", racename);
		//raceMaps[i][rcModeID] = CreateMinimode(MODE_TYPE_RACE, racename, i, minlevel, cost, x, y, z, notes);
		raceMaps[i][rcModeID] = LoadMinimode(MODE_TYPE_RACE, racename, i, notes, 1, raceMaps[i][rcVehicleSpawnsCount] ? raceMaps[i][rcVehicleSpawnsCount] : MAX_PLAYERS, raceMaps[i][rcVehicleSpawnsCount]);
		INI_Close();
		
		LoadRaceMapStats(i);
		
		if (raceMaps[i][rcModeID] != INVALID_MODE_ID) {
			racesLoaded++;
		}
	}
	
	printf("[FDM] Races Loaded: %i", racesLoaded);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

UpdateRacesInProgress() {
	new indexLoaded;
	
	for (new mapid; mapid < MAX_RACE_MAPS; mapid++) {
		if (raceMaps[mapid][rcIsLoaded]) {
			racesInProgress[indexLoaded] = mapid;
			indexLoaded++;
		}
	}
	
	racesInProgress[indexLoaded] = INVALID_MAP_ID;
	return 1;
}

#define LoopRacesLoaded(%1) for (new mmc, %1; racesInProgress[mmc] != INVALID_MAP_ID; mmc++) if ((%1 = racesInProgress[mmc]) || true)


//-----------------------------------------------------------------------------------------------------
// RACE RECORD HANDLING
//-----------------------------------------------------------------------------------------------------

LoadRaceMapStats(mapid) {
	//if (!INI_Exist(raceMaps[mapid][rcRecordFile])) return 0;
	if (!INI_Open(raceMaps[mapid][rcRecordFile])) return 0;
	
	// Load data
	new key[INI_MAX_KEY];
	new tmpstr[MAX_STRING];
	
	for (new i; i < MAX_RACE_RECORDS; i++) {
		format(key, sizeof(key), "RECORD%i", i);
		if (!INI_ReadString(tmpstr, key)) continue;
		
		new idx;
		splitcpy(raceMapRecords[mapid][i][rcPlayer], tmpstr, idx, ' ', MAX_NAME);
		raceMapRecords[mapid][i][rcTime] = strval(split(tmpstr, idx, ' '));
	}
	
	raceMaps[mapid][rcWinCount] = INI_ReadInt("WinCount");
	raceMaps[mapid][rcFailCount] = INI_ReadInt("FailCount");
	raceMaps[mapid][rcLeftCount] = INI_ReadInt("LeftCount");
	raceMaps[mapid][rcWinTime] = INI_ReadInt("WinTime");
	raceMaps[mapid][rcFailTime] = INI_ReadInt("FailTime");
	raceMaps[mapid][rcLeftTime] = INI_ReadInt("LeftTime");
	
	if (SortRecords(mapid)) SaveRaceMapStats(mapid);
	
	INI_Close();
	return 1;
}

SaveRaceMapStats(mapid) {
	if (!INI_Open(raceMaps[mapid][rcRecordFile])) return 0;
	
	new key[INI_MAX_KEY];
	new tmpstr[MAX_STRING];
	
	for (new i; i < MAX_RACE_RECORDS; i++) {
		format(tmpstr, sizeof(tmpstr), "%s %i", raceMapRecords[mapid][i][rcPlayer], raceMapRecords[mapid][i][rcTime]);
		format(key, sizeof(key), "RECORD%i", i);
		if (!INI_WriteString(key, tmpstr)) continue;
	}
	
	INI_WriteInt("WinCount", raceMaps[mapid][rcWinCount]);
	INI_WriteInt("FailCount", raceMaps[mapid][rcFailCount]);
	INI_WriteInt("LeftCount", raceMaps[mapid][rcLeftCount]);
	INI_WriteInt("WinTime", raceMaps[mapid][rcWinTime]);
	INI_WriteInt("FailTime", raceMaps[mapid][rcFailTime]);
	INI_WriteInt("LeftTime", raceMaps[mapid][rcLeftTime]);
	
	INI_Save();
	INI_Close();
	return 1;
}

SortRecords(mapid, &keeptrackindex = -1) {
	//gnomeSort method
	new i = 1;
	new j = 2;
	new changed = 0;
	
	while (i < MAX_RACE_RECORDS) {
		
		new time1 = raceMapRecords[mapid][i - 1][rcTime];
		new time2 = raceMapRecords[mapid][i][rcTime];
		
		if (!time1) time1 = 0x7FFFFFFF;
		if (!time2) time2 = 0x7FFFFFFF;
		
		if (time1 <= time2) {
			i = j;
			j = j + 1;
		} else {
			// swap
			new tmp[raceRecordInfo];
			tmp = raceMapRecords[mapid][i - 1];
			raceMapRecords[mapid][i - 1] = raceMapRecords[mapid][i];
			raceMapRecords[mapid][i] = tmp;
			
			if (keeptrackindex != -1) {
				if (keeptrackindex == i) {
					keeptrackindex = i - 1;
				} else if (keeptrackindex == i - 1) {
					keeptrackindex = i;
				}
			}
			changed = 1;
			//end of swap
			i = i - 1;
			if (i == 0)
				i = 1;
		}
	}
	return changed;
}

AddRecord(mapid, const playername[], time, &newrecordid = -1, &oldrecordid = -1, external = 0) {
	if (time <= 0) return 0;
	if (!playername[0]) return 0;
	// Find if any times are beaten
	new recordid = -1;
	new recordcount;
	new samenamefound;
	new minimum;
	
	oldrecordid = -1;
	
	for (new i; i < MAX_RACE_RECORDS; i++) { // Find equal names first
		if (raceMapRecords[mapid][i][rcTime]) {
			if (raceMapRecords[mapid][i][rcPlayer][0] && !strcmp(raceMapRecords[mapid][i][rcPlayer], playername)) {
				// Equal names
				if (raceMapRecords[mapid][i][rcTime] > time) {
					recordid = i;
					oldrecordid = recordcount;
				}
				samenamefound = 1;
				break;
			}
			recordcount++;
		}
	}
	if (recordid == -1 && !samenamefound) {
		for (new i; i < MAX_RACE_RECORDS; i++) {
			if (!raceMapRecords[mapid][i][rcTime]) { // Free slot
				recordid = i;
				break;
			}
			if (raceMapRecords[mapid][i][rcTime] > time) { // Time beaten
				if (!minimum || raceMapRecords[mapid][i][rcTime] < minimum) {
					recordid = i;
				}
			}
		}
	}
	
	if (recordid == -1) {
		// No record
		newrecordid = oldrecordid;
		return 0;
	} else {
		// New record
		strcpy(raceMapRecords[mapid][recordid][rcPlayer], playername, MAX_NAME);
		raceMapRecords[mapid][recordid][rcTime] = time;
		
		SortRecords(mapid, recordid);
		newrecordid = recordid;
		
		if (external) {
			SaveRaceMapStats(mapid);
		}
		
		#if ENABLE_CALLBACKS
			if (!external) {
				CallRemoteFunction("fdmOnRaceRecordAdded", "isiii", raceMaps[mapid][rcModeID], playername, time, newrecordid, oldrecordid);
			}
		#endif
		
		return 1;
	}
}

RemoveRecord(mapid, const playername[], external = 0) {
	for (new i; i < MAX_RACE_RECORDS; i++) { // Find equal names first
		if (raceMapRecords[mapid][i][rcPlayer][0] && !strcmp(raceMapRecords[mapid][i][rcPlayer], playername, true)) {
			// Equal names - clear record
			strclr(raceMapRecords[mapid][i][rcPlayer]);
			raceMapRecords[mapid][i][rcTime] = 0;
			
			SortRecords(mapid);
			SaveRaceMapStats(mapid);
			
			#if ENABLE_CALLBACKS
				if (!external) {
					CallRemoteFunction("fdmOnRaceRemoveRecord", "is", raceMaps[mapid][rcModeID], playername);
				}
			#endif
			
			return 1;
		}
	}
	
	return 0;
}

RemovePlayerRecords(const playername[], external = 0) {
	new recordsRemoved;
	
	for (new i; i < MAX_RACE_MAPS; i++) {
		if (raceMaps[i][rcModeID] == INVALID_MODE_ID) continue;
		recordsRemoved += RemoveRecord(i, playername, 1);
	}
	
	#if ENABLE_CALLBACKS
		if (recordsRemoved && !external) {
			CallRemoteFunction("fdmOnRaceRemovePlayerRecords", "s", playername);
		}
	#endif
	
	return recordsRemoved;
}

RemoveRaceRecords(mapid, external = 0) {
	new changed;
	for (new i; i < MAX_RACE_RECORDS; i++) {
		if (raceMapRecords[mapid][i][rcTime]) changed = 1;
		strclr(raceMapRecords[mapid][i][rcPlayer]);
		raceMapRecords[mapid][i][rcTime] = 0;
	}
	if (changed) {
		SaveRaceMapStats(mapid);
		
		#if ENABLE_CALLBACKS
			if (!external) {
				CallRemoteFunction("fdmOnRaceRemoveRaceRecords", "i", raceMaps[mapid][rcModeID]);
			}
		#endif
		
		return 1;
	} else {
		return 0;
	}
}

ShowPlayerRaceRecords(mapid, playerid, count = 6) {
	new message[MAX_INPUT];
	new recordcount;
	new samecount;
	
	format(message, sizeof(message), "Records for %s:", modes[raceMaps[mapid][rcModeID]][mName]);
	SendClientMessage(playerid, COLOUR_TITLE, message);
	
	for (new i; i < MAX_RACE_RECORDS && recordcount < count; i++) {
		if (raceMapRecords[mapid][i][rcPlayer][0] && raceMapRecords[mapid][i][rcTime]) {
			recordcount++;
			
			if (i) {
				if (raceMapRecords[mapid][i][rcTime] == raceMapRecords[mapid][i - 1][rcTime]) {
					samecount++;
				} else {
					samecount = 0;
				}
			}
			
			format(message, sizeof(message), "%i. %s - %s",
				recordcount - samecount, raceMapRecords[mapid][i][rcPlayer], GetTimeString(raceMapRecords[mapid][i][rcTime]));
			SendClientMessage(playerid, COLOUR_HELP, message);
		}
	}
	if (!recordcount) {
		SendClientMessage(playerid, COLOUR_HELP, "No records for this race.");
	} else if (recordcount > 8) {
		SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// ACTUAL RACES
//-----------------------------------------------------------------------------------------------------

race_IsJoinable(mapid) {
	if (!raceMaps[mapid][rcIsLoaded]) return 0;
	if (raceMaps[mapid][rcCountDown]) return 1;
	return 0;
}

race_ShowModeInfo(mapid, playerid) {
	new msg[MAX_INPUT];
	
	format(msg, sizeof(msg), "Starting Vehicle: %s | Vehicle Health: %0.1f | Checkpoints: %i",
		GetVehicleName(raceMaps[mapid][rcVehicleModel]), raceMaps[mapid][rcVehicleHealth], raceMaps[mapid][rcCheckpointCount]);
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	format(msg, sizeof(msg), "Wins: %s / Fails: %s / Leaves: %s", kvalstr(raceMaps[mapid][rcWinCount]), kvalstr(raceMaps[mapid][rcFailCount]), kvalstr(raceMaps[mapid][rcLeftCount]));
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) return 1;
	
	if (raceEnableDynamicWinnings > 0) {
		new Float:totalTime = float(raceMaps[mapid][rcFailTime]) + float(raceMaps[mapid][rcLeftTime]) + float(raceMaps[mapid][rcWinTime]);
		if (totalTime > float(raceEnableDynamicWinnings * 3600) && raceMaps[mapid][rcWinTime] > 0) {
			SendClientMessage(playerid, COLOUR_HELP, "Dynamic Winnings: On");
		} else {
			SendClientMessage(playerid, COLOUR_HELP, "Dynamic Winnings: Off");
		}
	}
	
	format(msg, sizeof(msg), "Time Spent For: Wins: %s / Fails: %s / Leaves: %s", GetTimeString4(raceMaps[mapid][rcWinTime]), GetTimeString4(raceMaps[mapid][rcFailTime]), GetTimeString4(raceMaps[mapid][rcLeftTime]));
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	if (raceMaps[mapid][rcWinTime]) {
		format(msg, sizeof(msg), "Base XP: %s / Base Cash: %s / Suggested XP: %s / Suggested Cash: %s", kvalstr(raceMaps[mapid][rcWinBaseXP]), kvalstr(raceMaps[mapid][rcWinBaseCash]), kvalstr(RaceGetSuggestedWinnings(mapid, 0)), kvalstr(RaceGetSuggestedWinnings(mapid, 1)));
	} else {
		format(msg, sizeof(msg), "Base XP: %s / Base Cash: %s / Suggested XP: - / Suggested Cash: -", kvalstr(raceMaps[mapid][rcWinBaseXP]), kvalstr(raceMaps[mapid][rcWinBaseCash]));
	}
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	return 1;
}

race_OnStart(mapid, players[], playercount) { // Race started
	if (raceMaps[mapid][rcIsLoaded]) return 0; // Race already loaded
	if (raceMaps[mapid][rcCheckpointCount] < 1) return 0; // Invalid number of checkpoints
	
	raceMaps[mapid][rcIsLoaded] = 1;
	raceMaps[mapid][rcCountDown] = COUNTDOWN_TIME; // Start countdown for anyone in the race
	raceMaps[mapid][rcStartTime] = GetTickCount();
	raceMaps[mapid][rcWinPlayerCount] = 0;
	raceMaps[mapid][rcPlayerCount] = 0;
	
	UpdateRacesInProgress();
	
	racePickupLoad(mapid);
	
	for (new i; i < MAX_RACE_SPAWNS; i++) {
		raceMaps[mapid][rcSpawnsUsed][i] = 0;
	}
	
	LoopPlayers(i) {
		if (!players[i]) continue;
		if (!race_OnPlayerEnter(mapid, i, playercount)) {
			players[i] = 0;
		}
	}
	RaceUpdatePositions(mapid);
	
	return 1;
}

race_OnStop(mapid) { // Race stopped
	if (!raceMaps[mapid][rcIsLoaded]) return 0; // Race already stopped
	raceMaps[mapid][rcIsLoaded] = 0;
	
	UpdateRacesInProgress();
	
	racePickupUnload(mapid);
	
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] != mapid) continue;
		racePStats[i][rcMapID] = INVALID_MAP_ID;
		racePStats[i][rcCheckpointID] = 0;
		
		// Fail race for anyone still inside
		SendClientMessage(i, COLOUR_ERROR, "You have ran out of time for this race.");
		RaceFailForPlayer(mapid, i);
	}
	
	return 1;
}

race_OnPlayerEnter(mapid, playerid, playercount = 0) { // Player enter race
	if (!raceMaps[mapid][rcCountDown]) return 0; // Race must be on countdown for player to enter
	
	new vehicleid = GetPlayerVehicleID(playerid);
	new vehiclemodel, Float:x, Float:y, Float:z, Float:rotz;
	
	// Get spawn location
	new spawnid;
	if (raceMaps[mapid][rcVehicleSpawnsCount]) {
		new spawnfound;
		
		if (!playercount) {
			for (new i; i < raceMaps[mapid][rcVehicleSpawnsCount]; i++) {
				if (raceMaps[mapid][rcSpawnsUsed][i]) continue;
				spawnid = i;
				spawnfound = 1;
				break;
			}
		} else {
			new avaliablespawns[MAX_RACE_SPAWNS];
			new avaliablespawnscount;
			
			for (new i; i < playercount && i < raceMaps[mapid][rcVehicleSpawnsCount]; i++) {
				if (raceMaps[mapid][rcSpawnsUsed][i]) continue;
				avaliablespawns[avaliablespawnscount] = i;
				avaliablespawnscount++;
			}
			
			if (avaliablespawnscount) {
				spawnid = avaliablespawns[random(avaliablespawnscount)];
				spawnfound = 1;
			}
		}
		
		if (!spawnfound) {
			SendClientMessage(playerid, COLOUR_ERROR, "RACE: Error - No more spawn points avaliable.");
			return 0;
		}
		
		raceMaps[mapid][rcSpawnsUsed][spawnid] = 1;
		
		x = raceSpawns[mapid][spawnid][rcvX];
		y = raceSpawns[mapid][spawnid][rcvY];
		z = raceSpawns[mapid][spawnid][rcvZ];
		rotz = raceSpawns[mapid][spawnid][rcvRotZ];
		
		raceMaps[mapid][rcSpawnsUsed][spawnid] = 1;
		if (raceSpawns[mapid][spawnid][rcvVehicleModel] == -2) {
			vehiclemodel = raceMaps[mapid][rcVehicleModel];
		} else {
			vehiclemodel = raceSpawns[mapid][spawnid][rcvVehicleModel];
		}
	} else {
		vehiclemodel = raceMaps[mapid][rcVehicleModel];
		if (!playercount) {
			// Use random position in circle
			x = modes[raceMaps[mapid][rcModeID]][mLocation][mX] + (random(7) - 4);
			y = modes[raceMaps[mapid][rcModeID]][mLocation][mY] + (random(7) - 4);
			z = modes[raceMaps[mapid][rcModeID]][mLocation][mZ];
			rotz = ReturnAngle(modes[raceMaps[mapid][rcModeID]][mLocation][mX], modes[raceMaps[mapid][rcModeID]][mLocation][mY], raceMapCPs[mapid][0][rcX], raceMapCPs[mapid][0][rcY]) - 90.0;
		} else {
			if (vehicleid) {
				GetVehiclePos(vehicleid, x, y, z);
			} else {
				GetPlayerPos(playerid, x, y, z);
			}
			rotz = ReturnAngle(x, y, raceMapCPs[mapid][0][rcX], raceMapCPs[mapid][0][rcY]) - 90.0;
		}
	}
	
	racePStats[playerid][rcpSpawnX] = x;
	racePStats[playerid][rcpSpawnY] = y;
	racePStats[playerid][rcpSpawnZ] = z;
	racePStats[playerid][rcpSpawnRotZ] = rotz;
	racePStats[playerid][rcpJustSpawned] = 1;
	racePStats[playerid][rcpWarnings] = 0;
	racePStats[playerid][rcpPlayTimePaid] = 0;
	racePStats[playerid][rcpPickupAbused] = 0;
	
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		racePStats[playerid][rcpIsInPickup][i] = 0;
	}
	
	if (vehiclemodel == ANY_VEHICLE_ID) {
		// Let player use their existing vehicle
		racePStats[playerid][rcpVehicleID] = vehicleid;
		racePStats[playerid][rcpIsRaceVehicle] = 0;
		if (vehicleid && GetPlayerState(playerid) == PLAYER_STATE_DRIVER) {
			SetVehicleVirtualWorld(vehicleid, modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
			LinkVehicleToInterior(vehicleid, modes[raceMaps[mapid][rcModeID]][mInteriorID]);
			SetVehiclePos(vehicleid, x, y, z);
			SetVehicleZAngle(vehicleid, rotz);
		} else {
			raceMaps[mapid][rcSpawnsUsed][spawnid] = 0;
			/*SetPlayerPos(playerid, x, y, z);
			SetPlayerFacingAngle(playerid, rotz);*/
			SendClientMessage(playerid, COLOUR_ERROR, "RACE: Error - You need a vehicle to race.");
			return 0;
		}
	} else {
		// Give player race vehicle
		if (vehiclemodel) {
			racePStats[playerid][rcpVehicleID] = CreateVehicle(vehiclemodel, x, y, z, rotz, -1, -1, 100000);
			racePStats[playerid][rcpIsRaceVehicle] = 1;
			
			if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
				//if (vehicleid) RemovePlayerFromVehicle(playerid);
				PutPlayerInVehicle(playerid, racePStats[playerid][rcpVehicleID], 0);
				SetVehiclePos(racePStats[playerid][rcpVehicleID], x, y, z);
				SetVehicleZAngle(racePStats[playerid][rcpVehicleID], rotz);
				if (modes[raceMaps[mapid][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) UpdateVehicleDamageStatus(racePStats[playerid][rcpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
			} else {
				race_OnPlayerExit(mapid, playerid);
				SendClientMessage(playerid, COLOUR_ERROR, "RACE: Error - Cannot create your race vehicle.");
				return 0;
			}
		} else { // On foot race
			SetPlayerPos(playerid, x, y, z);
			SetPlayerFacingAngle(playerid, rotz);
			racePStats[playerid][rcpVehicleID] = INVALID_VEHICLE_ID;
		}
	}
	
	// Mod the car
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
		for (new d; d < MAX_CAR_COMPONENTS; d++) {
			if (raceMaps[mapid][rcVehicleComponents][d]) {
				AddVehicleComponent(racePStats[playerid][rcpVehicleID], raceMaps[mapid][rcVehicleComponents][d]);
			}
		}
	}
	
	// Set checkpoint
	if (raceMaps[mapid][rcCheckpointCount] == 1) {
		new type;
		if (!raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcType]) {
			type = 1;
		} else {
			type = 4;
		}
		SetPlayerRaceCheckpoint(playerid, type,
			raceMapCPs[mapid][0][rcX],
			raceMapCPs[mapid][0][rcY],
			raceMapCPs[mapid][0][rcZ],
			x,
			y,
			z,
			raceMapCPs[mapid][0][rcSize]);
		
		RemovePlayerMapIcon(playerid, RACE_ICON_ID);
	} else {
		new type;
		if (!raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcType]) {
			type = 0;
		} else {
			type = 3;
		}
		SetPlayerRaceCheckpoint(playerid, type,
			raceMapCPs[mapid][0][rcX],
			raceMapCPs[mapid][0][rcY],
			raceMapCPs[mapid][0][rcZ],
			raceMapCPs[mapid][1][rcX],
			raceMapCPs[mapid][1][rcY],
			raceMapCPs[mapid][1][rcZ],
			raceMapCPs[mapid][0][rcSize]);
		
		SetPlayerMapIcon(playerid, RACE_ICON_ID,
			raceMapCPs[mapid][1][rcX], raceMapCPs[mapid][1][rcY], raceMapCPs[mapid][1][rcZ],
			RACE_ICON_TYPE, RACE_ICON_COLOUR);
	}
	
	racePStats[playerid][rcCheckpointID] = 1;
	
	RaceUpdatePlayerInfoShow(playerid);
	RaceUpdatePlayerCheckpoint(mapid, playerid);
	if (!playercount) RaceUpdatePositions(mapid);
	
	EnableGodmode(playerid);
	SetPlayerVirtualWorld(playerid, modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
		SetVehicleVirtualWorld(racePStats[playerid][rcpVehicleID], modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
		LinkVehicleToInterior(racePStats[playerid][rcpVehicleID], modes[raceMaps[mapid][rcModeID]][mInteriorID]);
		SetVehicleHealth(racePStats[playerid][rcpVehicleID], raceMaps[mapid][rcVehicleHealth]);
	}
	
	racePStats[playerid][rcpDamage] = 0.0;
	racePStats[playerid][rcpLastVHP] = raceMaps[mapid][rcVehicleHealth];
	racePStats[playerid][rcpLastPHP] = 100.0;
	
	racePStats[playerid][rcMapID] = mapid;
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
	raceMaps[mapid][rcPlayerCount]++;
	
	ShowPlayerRaceRecords(mapid, playerid);
	
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		race_OnPlayerSpectate(i, playerid);
	}
	
	return 1;
}

race_OnPlayerExit(mapid, playerid, action[] = "left") { // Player exit race
	
	racePStats[playerid][rcMapID] = INVALID_MAP_ID;
	racePStats[playerid][rcCheckpointID] = 0;
	RemovePlayerMapIcon(playerid, RACE_ICON_ID);
	RaceUpdatePlayerInfoHide(playerid);
	SetLastSpawnTick(playerid);
	
	if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) {
		DisablePlayerRaceCheckpoint(playerid);
		SetPlayerVirtualWorld(playerid, 0);
	}
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		race_OnPlayerUnspectate(i, playerid);
	}
	
	// Increment statistics
	new timeTaken = (GetTickCount() - raceMaps[mapid][rcStartTime]) / 1000;
	new overflow;
	if (!strcmp(action, "finished")) {
		raceMaps[mapid][rcWinCount]++;
		raceMaps[mapid][rcWinTime] += timeTaken;
		if (raceMaps[mapid][rcWinTime] < 0) {
			raceMaps[mapid][rcWinTime] -= timeTaken;
			overflow = 1;
		}
	} else if (!strcmp(action, "failed")) {
		raceMaps[mapid][rcFailCount]++;
		raceMaps[mapid][rcFailTime] += timeTaken;
		if (raceMaps[mapid][rcFailTime] < 0) {
			raceMaps[mapid][rcFailTime] -= timeTaken;
			overflow = 1;
		}
	} else {
		raceMaps[mapid][rcLeftCount]++;
		raceMaps[mapid][rcLeftTime] += timeTaken;
		if (raceMaps[mapid][rcLeftTime] < 0) {
			raceMaps[mapid][rcLeftTime] -= timeTaken;
			overflow = 1;
		}
	}
	
	if (overflow) {
		raceMaps[mapid][rcWinCount] = raceMaps[mapid][rcWinCount] / 2;
		raceMaps[mapid][rcFailCount] = raceMaps[mapid][rcFailCount] / 2;
		raceMaps[mapid][rcLeftCount] = raceMaps[mapid][rcLeftCount] / 2;
		raceMaps[mapid][rcWinTime] = raceMaps[mapid][rcWinTime] / 2;
		raceMaps[mapid][rcFailTime] = raceMaps[mapid][rcFailTime] / 2;
		raceMaps[mapid][rcLeftTime] = raceMaps[mapid][rcLeftTime] / 2;
	}
	SaveRaceMapStats(mapid);
	
	
	// Race vehicle
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) { // Destroy their vehicle
		if (racePStats[playerid][rcpIsRaceVehicle]) {
			DestroyVehicle(racePStats[playerid][rcpVehicleID]);
		} else {
			SetVehicleVirtualWorld(racePStats[playerid][rcpVehicleID], 0);
			LinkVehicleToInterior(racePStats[playerid][rcpVehicleID], modes[raceMaps[mapid][rcModeID]][mLocation][mwInteriorID]);
		}
		racePStats[playerid][rcpVehicleID] = INVALID_VEHICLE_ID;
	}
	
	// Teleport back to start line
	if (IsPlayerSpawned(playerid)) {
		if (racePStats[playerid][rcpIsRaceVehicle]) {
			SetPlayerPosRandom(playerid,
				modes[raceMaps[mapid][rcModeID]][mLocation][mX],
				modes[raceMaps[mapid][rcModeID]][mLocation][mY],
				modes[raceMaps[mapid][rcModeID]][mLocation][mZ], 5);
		} else {
			teleportPlayerRandom(playerid,
				modes[raceMaps[mapid][rcModeID]][mLocation][mX],
				modes[raceMaps[mapid][rcModeID]][mLocation][mY],
				modes[raceMaps[mapid][rcModeID]][mLocation][mZ], 5);
			
			new vid = GetPlayerVehicleID(playerid);
			if (vid) {
				new Float:angle;
				GetVehicleZAngle(vid, angle);
				SetVehicleZAngle(vid, angle);
			}
		}
	}
	
	if (raceMaps[mapid][rcCountDown]) {
		TogglePlayerControllable(playerid, 1); // Player may still be frozen
		raceMaps[mapid][rcPlayerCount]--; // Removes exploit of joining/leaving at countdown to multiply XP
	}
	
	if (IsPlayerSpawned(playerid)) {
		//SetPlayerHealth(playerid, 100.0);
		SetCameraBehindPlayer(playerid);
	}
	
	kGameTextHide(playerid);
	
	// Check if there are any remaining players
	/*new playercount;
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] != mapid) continue;
		playercount++;
		break;
	}
	if (!playercount) StopMinimode(raceMaps[mapid][rcModeID]);*/
	
	return 1;
}

race_OnPlayerDeath(mapid, playerid) { // Fail race
	RaceFailForPlayer(mapid, playerid);
	SkinsSetNextSpawnPos(playerid,
		modes[raceMaps[mapid][rcModeID]][mLocation][mX], modes[raceMaps[mapid][rcModeID]][mLocation][mY], modes[raceMaps[mapid][rcModeID]][mLocation][mZ], float(random(360)));
	return 1;
}

race_OnPlayerStateChange(mapid, playerid, newstate, oldstate) {
	
	if (oldstate == PLAYER_STATE_DRIVER && racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
		GetVehiclePos(racePStats[playerid][rcpVehicleID], racePStats[playerid][rcpLastX], racePStats[playerid][rcpLastY], racePStats[playerid][rcpLastZ]);
		GetVehicleZAngle(racePStats[playerid][rcpVehicleID], racePStats[playerid][rcpLastRotZ]);
		return 0;
	}
	
	if (newstate == PLAYER_STATE_DRIVER) {
		if (racePStats[playerid][rcpVehicleID] == INVALID_VEHICLE_ID) {
			new Float:px, Float:py, Float:pz;
			GetPlayerPos(playerid, px, py, pz);
			SetPlayerPos(playerid, px, py, pz);
			
			SendClientMessage(playerid, COLOUR_ATTENTION, "RACE: You cannot use this vehicle.");
			return 0;
		}
		
		new vehicleid = GetPlayerVehicleID(playerid);
		if (racePStats[playerid][rcpVehicleID] == vehicleid) {
			if (!raceMaps[mapid][rcCountDown] && racePStats[playerid][rcpPickupAbused]) {
				racePStats[playerid][rcpPickupAbused] = 0;
				
				new Float:vx, Float:vy, Float:vz, Float:distance;
				GetVehiclePos(racePStats[playerid][rcpVehicleID], vx, vy, vz);
				distance = FindDistance(vx, vy, vz, racePStats[playerid][rcpLastX], racePStats[playerid][rcpLastY], racePStats[playerid][rcpLastZ]);
				if (distance > 0.5) {
					SetVehiclePos(vehicleid, racePStats[playerid][rcpLastX], racePStats[playerid][rcpLastY], racePStats[playerid][rcpLastZ]);
					SetVehicleZAngle(vehicleid, racePStats[playerid][rcpLastRotZ]);
					SetCameraBehindPlayer(playerid);
				}
			}
		} else {
			LoopPlayers(i) {
				if (playerid == i) continue;
				if (racePStats[i][rcMapID] != mapid) continue;
				if (racePStats[i][rcpVehicleID] == INVALID_VEHICLE_ID) continue;
				if (racePStats[i][rcpVehicleID] != vehicleid) continue;
				
				racePStats[playerid][rcpWarnings]++;
				SendClientMessage(playerid, COLOUR_ATTENTION, "RACE: Do not steal another player's vehicle!");
				
				if (racePStats[playerid][rcpWarnings] >= MAX_RACE_WARNINGS) {
					RaceFailForPlayer(mapid, playerid);
					return 0;
				}
				
				new Float:px, Float:py, Float:pz;
				GetPlayerPos(playerid, px, py, pz);
				SetPlayerPos(playerid, px, py, pz);
				
				return 0;
			}
			
			new Float:px, Float:py, Float:pz;
			GetPlayerPos(playerid, px, py, pz);
			SetPlayerPos(playerid, px, py, pz);
			
			SendClientMessage(playerid, COLOUR_ATTENTION, "RACE: You cannot use this vehicle.");
			return 0;
		}
	}
	
	// Re-check the player checkpoints
	if (racePStats[playerid][rcpVehicleID] == INVALID_VEHICLE_ID) {
		if (newstate == PLAYER_STATE_ONFOOT && kIsPlayerInRaceCheckpoint(playerid)) {
			RaceNextCheckpoint(mapid, playerid);
		}
	} else {
		if (newstate == PLAYER_STATE_DRIVER && GetPlayerVehicleID(playerid) == racePStats[playerid][rcpVehicleID] && kIsPlayerInRaceCheckpoint(playerid)) {
			RaceNextCheckpoint(mapid, playerid);
		}
	}
	
	// Re-check the player pickups
	if (newstate == PLAYER_STATE_DRIVER && modes[raceMaps[mapid][rcModeID]][mVirtualWorld] == GetPlayerVirtualWorld(playerid)) {
		for (new racepickupid; racepickupid < MAX_RACE_PICKUPS; racepickupid++) {
			if (raceMapPickups[mapid][racepickupid][rcpPickupID] == INVALID_PICKUP_ID) break;
			
			if (IsPlayerInRangeOfPoint(playerid, 4.0, raceMapPickups[mapid][racepickupid][rcpX], raceMapPickups[mapid][racepickupid][rcpY], raceMapPickups[mapid][racepickupid][rcpZ])) {
				racePStats[playerid][rcpIsInPickup][racepickupid] = 1;
				race_OnPlayerPickUpPickup(playerid, raceMapPickups[mapid][racepickupid][rcpPickupID]);
			} else {
				racePStats[playerid][rcpIsInPickup][racepickupid] = 0;
			}
		}
	}
	
	return 0;
}

race_OnVehicleDestroyed(vehicleid) { // Fail race
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] == INVALID_MAP_ID) continue;
		if (racePStats[i][rcpVehicleID] == INVALID_VEHICLE_ID) continue;
		if (racePStats[i][rcpVehicleID] != vehicleid) continue;
		
		RaceFailForPlayer(racePStats[i][rcMapID], i);
		return 0;
	}
	return 0;
}

race_OnVehicleDeath(vehicleid) { // Fail race
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] == INVALID_MAP_ID) continue;
		if (raceMaps[racePStats[i][rcMapID]][rcCountDown]) continue;
		if (racePStats[i][rcpVehicleID] == INVALID_VEHICLE_ID) continue;
		if (racePStats[i][rcpVehicleID] != vehicleid) continue;
		
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
					SetTimerEx("race_Fail", 750, false, "ii", racePStats[i][rcMapID], i);
					return 0;
				}
			}
		}
		
		RaceFailForPlayer(racePStats[i][rcMapID], i);
		//SetTimerEx("race_Fail", 750, false, "ii", racePStats[i][rcMapID], i);
		return 0;
	}
	return 0;
}

race_OnVehicleSpawn(vehicleid) { // Fail race
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] == INVALID_MAP_ID) continue;
		if (racePStats[i][rcpVehicleID] == INVALID_VEHICLE_ID) continue;
		if (racePStats[i][rcpVehicleID] != vehicleid) continue;
		
		RaceFailForPlayer(racePStats[i][rcMapID], i);
		//SetTimerEx("race_Fail", 750, false, "ii", racePStats[i][rcMapID], i);
		return 0;
	}
	return 0;
}

forward race_Fail(mapid, playerid);
public race_Fail(mapid, playerid) {
	if (racePStats[playerid][rcMapID] != mapid) return 1;
	RaceFailForPlayer(racePStats[playerid][rcMapID], playerid);
	return 1;
}

race_OnPlayerEnterRaceCP(mapid, playerid) { // Next CP
	RaceNextCheckpoint(mapid, playerid);
	return 0;
}

RaceNextCheckpoint(mapid, playerid) {
	// Check if they're in the right world
	if (GetPlayerVirtualWorld(playerid) != modes[raceMaps[mapid][rcModeID]][mVirtualWorld]) {
		SendClientMessage(playerid, COLOUR_ERROR, "You must be in the race world to race!");
		return 1;
	}
	
	// Check if they're in the right vehicle, as driver
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
		if (GetPlayerVehicleID(playerid) != racePStats[playerid][rcpVehicleID]) { // If their vehicle is not equal to their given vehicle
			SendClientMessage(playerid, COLOUR_ERROR, "You need to be in your own vehicle to race!");
			return 1;
		}
		if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) {
			SendClientMessage(playerid, COLOUR_ERROR, "You must be the driver of your vehicle to race!");
			return 1;
		}
	} else {
		if (raceMaps[mapid][rcVehicleModel] != ANY_VEHICLE_ID) {
			if (IsPlayerInAnyVehicle(playerid)) {
				SendClientMessage(playerid, COLOUR_ERROR, "You must not be in a vehicle for this race!");
				return 1;
			}
		}
	}
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerHitRaceCheckpoint", "iii", playerid, raceMaps[mapid][rcModeID], racePStats[playerid][rcCheckpointID]);
	#endif
	
	// Calculate any vehicle damage on the checkpoint
	if (racePStats[playerid][rcpVehicleID] == INVALID_VEHICLE_ID) {
		new Float:php;
		racePStats[playerid][rcpLastVHP] = 1000.0;
		GetPlayerHealth(playerid, php);
		
		if (php < racePStats[playerid][rcpLastPHP]) {
			racePStats[playerid][rcpDamage] = racePStats[playerid][rcpDamage] + racePStats[playerid][rcpLastPHP] - php;
		}
		
		racePStats[playerid][rcpLastPHP] = php;
	} else {
		new Float:vhp;
		GetVehicleHealth(racePStats[playerid][rcpVehicleID], vhp);
		GetPlayerHealth(playerid, racePStats[playerid][rcpLastPHP]);
		
		if (vhp < racePStats[playerid][rcpLastVHP]) {
			racePStats[playerid][rcpDamage] = racePStats[playerid][rcpDamage] + racePStats[playerid][rcpLastVHP] - vhp;
		}
		
		racePStats[playerid][rcpLastVHP] = vhp;
	}
	
	new type;
	
	DisablePlayerRaceCheckpoint(playerid);
	if (racePStats[playerid][rcCheckpointID] >= raceMaps[mapid][rcCheckpointCount]) { // Player has finished race
		new message[MAX_INPUT];
		
		// Check records
		new recordxp;
		new newrecordpos;
		new oldrecordpos;
		new basexp = raceMaps[mapid][rcWinBaseXP];
		new basecash = raceMaps[mapid][rcWinBaseCash];
		new timeTaken = GetTickCount() - raceMaps[mapid][rcStartTime];
		new newrecord = AddRecord(mapid, pData[playerid][pLoginName], timeTaken, newrecordpos, oldrecordpos);
		
		if (raceEnableDynamicWinnings > 0 && raceMaps[mapid][rcWinTime] > 0) {
			new Float:totalTime = float(raceMaps[mapid][rcFailTime]) + float(raceMaps[mapid][rcLeftTime]) + float(raceMaps[mapid][rcWinTime]);
			if (totalTime > float(raceEnableDynamicWinnings * 3600)) { // totalTime > hours of race played
				basexp = RaceGetSuggestedWinnings(mapid, 0);
				basecash = RaceGetSuggestedWinnings(mapid, 1);
			}
		}
		
		raceMaps[mapid][rcWinPlayerCount]++;
		
		if (newrecord) {
			if (oldrecordpos == -1) {
				format(message, sizeof(message), "(New Record: %s)", racestr(newrecordpos + 1));
				//recordxp = (basexp * (MAX_RACE_RECORDS - newrecordpos)) / MAX_RACE_RECORDS;
			
			} else if (newrecordpos == oldrecordpos) {
				format(message, sizeof(message), "(Improved Record: %s)", racestr(newrecordpos + 1));
				//recordxp = basexp / MAX_RACE_RECORDS;
				
			} else if (newrecordpos < oldrecordpos) {
				format(message, sizeof(message), "(Improved Record: %s > %s)", racestr(oldrecordpos + 1), racestr(newrecordpos + 1));
				//recordxp = (basexp * (oldrecordpos - newrecordpos)) / MAX_RACE_RECORDS;
				
			} else {
				format(message, sizeof(message), "(New Record: %s)", racestr(newrecordpos + 1));
				//recordxp = basexp / MAX_RACE_RECORDS;
				
			}
			
			recordxp = (basexp * (MAX_RACE_RECORDS - newrecordpos)) / MAX_RACE_RECORDS;
			
			format(message, sizeof(message), "%s finished %s in %s place within %s. %s",
				pData[playerid][pLoginName], modes[raceMaps[mapid][rcModeID]][mName], racestr(raceMaps[mapid][rcWinPlayerCount]), GetTimeString(GetTickCount() - raceMaps[mapid][rcStartTime]), message);
			
		} else {
			new raceTime = GetTickCount() - raceMaps[mapid][rcStartTime];
			new recordpos = -1;
			
			format(message, sizeof(message), "%s finished %s in %s place within %s.",
				pData[playerid][pLoginName], modes[raceMaps[mapid][rcModeID]][mName], racestr(raceMaps[mapid][rcWinPlayerCount]), GetTimeString(raceTime));
			
			for (new i; i < MAX_RACE_RECORDS; i++) {
				if (!raceMapRecords[mapid][i][rcTime] || raceMapRecords[mapid][i][rcTime] > raceTime) {
					recordpos = i;
					break;
				}
			}
			
			if (recordpos != -1) {
				// Other records beaten, but not the player's current record
				recordxp = (basexp * (MAX_RACE_RECORDS - recordpos)) / MAX_RACE_RECORDS;
				
				format(message, sizeof(message), "%s (Record Beaten: %s)", message, racestr(recordpos + 1));
			}
		}
		
		SendClientMessageToAll(COLOUR_SUCCESSFUL, message);
		
		// Check drunk level
		new drunkxp = GetPlayerDrunkLevel(playerid);
		if (drunkxp < 0) drunkxp = 0;
		if (drunkxp > 5000) drunkxp = 5000;
		drunkxp = floatround(float(drunkxp) * (float(basexp) / 5000.0), floatround_floor);
		
		// Check damage penalty
		new damagexp;
		if (raceEnableDamagePenalty) {
			damagexp = floatround(((basexp * racePStats[playerid][rcpDamage]) / raceMaps[mapid][rcVehicleHealth]) * raceMaps[mapid][rcDamageMultiplier], floatround_floor);
			if (damagexp > basexp / 2) damagexp = basexp / 2; // Limit how much can be penalised
		}
		
		// Play finish sound
		new Float:x, Float:y, Float:z;
		GetPlayerPos(playerid, x, y, z);
		PlayerPlaySound(playerid, SOUND_FINISH, x, y, z);
		
		// Give player xp/cash
		new xp = MinimodeProcessFlagsXP(raceMaps[mapid][rcModeID], (basexp - damagexp + recordxp + drunkxp + ((modes[raceMaps[mapid][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) ? basexp : 0) + ((basexp * (raceMaps[mapid][rcPlayerCount] - 1)) / 3)) / raceMaps[mapid][rcWinPlayerCount]);
		SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, xp));
		
		new money = MinimodeProcessFlagsMoney(raceMaps[mapid][rcModeID], (basecash + ((basecash * (raceMaps[mapid][rcPlayerCount] - 1)) / 3)) / raceMaps[mapid][rcWinPlayerCount]);
		GivePlayerMoney(playerid, money);
		SendMoneyMessageToPlayer(playerid, money);
		
		// Spectator support
		LoopPlayers(i) {
			if (GetPlayerSpectatingPlayer(i) != playerid) continue;
			race_UpdateSpectatePlayer(i, playerid, 1);
		}
		RemovePlayerFromMode(playerid, "finished");
		return 1;
	}
	
	if (racePStats[playerid][rcCheckpointID] == raceMaps[mapid][rcCheckpointCount] - 1) { // Finish line CP
		if (!raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcType]) {
			type = 1;
		} else {
			type = 4;
		}
		SetPlayerRaceCheckpoint(playerid, type,
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcX],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcY],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcZ],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcX],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcY],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcZ],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcSize]);
		
		RemovePlayerMapIcon(playerid, RACE_ICON_ID);
	} else { // Normal CP
		if (!raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcType]) {
			type = 0;
		} else {
			type = 3;
		}
		SetPlayerRaceCheckpoint(playerid, type,
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcX],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcY],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcZ],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcX],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcY],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcZ],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID]][rcSize]);
		
		SetPlayerMapIcon(playerid, RACE_ICON_ID,
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcX],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcY],
			raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] + 1][rcZ],
			RACE_ICON_TYPE, RACE_ICON_COLOUR);
		
		// Checkpoint time
		new timemsg[128];
		format(timemsg, sizeof(timemsg), "%s", GetTimeString3(GetTickCount() - raceMaps[mapid][rcStartTime]));
		kGameTextForPlayer(playerid, timemsg, 5000, 320.0, 320.0);
	}
	
	// Vehicle switching
	if (raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rccVehicleModel] != ANY_VEHICLE_ID) {
		RaceSwitchPlayerVehicle(mapid, playerid, raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rccVehicleModel]);
	}
	
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID && raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcVehicleHP] != 0.0) {
		SetVehicleHealth(racePStats[playerid][rcpVehicleID], raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcVehicleHP]);
		racePStats[playerid][rcpLastVHP] = raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcVehicleHP];
	}
	
	if (raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcInterior] != -1) {
		SetPlayerInterior(playerid, raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcInterior]);
		LinkVehicleToInterior(racePStats[playerid][rcpVehicleID], raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcInterior]);
	}
	
	#if defined _YSI_included
		if (raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcGravity] != INVALID_GRAVITY) {
			SetPlayerGravity(playerid, raceMapCPs[mapid][racePStats[playerid][rcCheckpointID] - 1][rcGravity]);
		}
	#endif
	
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	PlayerPlaySound(playerid, RACE_SOUND_CHECKPOINT, x, y, z);
	
	racePStats[playerid][rcCheckpointID]++;
	RaceUpdatePlayerCheckpoint(mapid, playerid);
	
	// Spectator support
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		race_UpdateSpectatePlayer(i, playerid);
	}
	
	return 1;
}

RaceSwitchPlayerVehicle(mapid, playerid, vehiclemodel) {
	new Float:x, Float:y, Float:z, Float:vx, Float:vy, Float:vz, Float:rotz, Float:vehiclehp, dmg[4];
	
	if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
		if (GetVehicleModel(racePStats[playerid][rcpVehicleID]) == vehiclemodel) {
			return 1;
		}
		GetVehicleVelocity(racePStats[playerid][rcpVehicleID], vx, vy, vz);
		GetVehiclePos(racePStats[playerid][rcpVehicleID], x, y, z);
		GetVehicleZAngle(racePStats[playerid][rcpVehicleID], rotz);
		GetVehicleDamageStatus(racePStats[playerid][rcpVehicleID], dmg[0], dmg[1], dmg[2], dmg[3]);
	} else {
		GetPlayerVelocity(playerid, vx, vy, vz);
		GetPlayerPos(playerid, x, y, z);
		GetPlayerFacingAngle(playerid, rotz);
	}
	
	// Lag fixer
	if (vx == 0 && vy == 0 && vz == 0 && racePStats[playerid][rcpLastSwitchTick] + 1000 > GetTickCount()) {
		vx = racePStats[playerid][rcpLastSX];
		vy = racePStats[playerid][rcpLastSY];
		vz = racePStats[playerid][rcpLastSZ];
	} else {
		racePStats[playerid][rcpLastSX] = vx;
		racePStats[playerid][rcpLastSY] = vy;
		racePStats[playerid][rcpLastSZ] = vz;
	}
	racePStats[playerid][rcpLastSwitchTick] = GetTickCount();
	
	if (vehiclemodel) {
		new oldvid = racePStats[playerid][rcpVehicleID];
		racePStats[playerid][rcpVehicleID] = CreateVehicle(vehiclemodel, x, y, z, rotz, -1, -1, 100000);
		
		if (!racePStats[playerid][rcpVehicleID]) {
			SendClientMessage(playerid, COLOUR_ERROR, "RACE: Error - cannot create vehicle for switching.");
			RemovePlayerFromMode(playerid, "left", "(Error)");
			return 1;
		}
		
		SetVehicleVirtualWorld(racePStats[playerid][rcpVehicleID], modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
		LinkVehicleToInterior(racePStats[playerid][rcpVehicleID], GetPlayerInterior(playerid));
		
		if (modes[raceMaps[mapid][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) {
			UpdateVehicleDamageStatus(racePStats[playerid][rcpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
		} else {
			UpdateVehicleDamageStatus(racePStats[playerid][rcpVehicleID], dmg[0], dmg[1], dmg[2], dmg[3]);
		}
		
		if (oldvid) {
			GetVehicleHealth(oldvid, vehiclehp);
			
			if (racePStats[playerid][rcpIsRaceVehicle]) {
				DestroyVehicle(oldvid);
			} else {
				SetVehicleVirtualWorld(oldvid, 0);
				LinkVehicleToInterior(oldvid, 0);
				SetVehicleToRespawn(oldvid);
			}
		} else {
			vehiclehp = raceMaps[mapid][rcVehicleHealth];
		}
		
		racePStats[playerid][rcpIsRaceVehicle] = 1;
		PutPlayerInVehicle(playerid, racePStats[playerid][rcpVehicleID], 0);
		
		//SetVehiclePos(racePStats[playerid][rcpVehicleID], x, y, z);
		//SetVehicleZAngle(racePStats[playerid][rcpVehicleID], rotz);
		SetVehicleVelocity(racePStats[playerid][rcpVehicleID], vx, vy, vz);
		
		SetVehicleHealth(racePStats[playerid][rcpVehicleID], vehiclehp);
	} else {
		if (racePStats[playerid][rcpIsRaceVehicle]) {
			DestroyVehicle(racePStats[playerid][rcpVehicleID]);
			racePStats[playerid][rcpIsRaceVehicle] = 0;
		} else {
			SetVehicleToRespawn(racePStats[playerid][rcpVehicleID]);
			SetVehicleVirtualWorld(racePStats[playerid][rcpVehicleID], 0);
			LinkVehicleToInterior(racePStats[playerid][rcpVehicleID], 0);
		}
		
		racePStats[playerid][rcpVehicleID] = INVALID_VEHICLE_ID;
		
		SetPlayerPos(playerid, x, y, z);
		SetPlayerFacingAngle(playerid, rotz);
		SetPlayerVelocity(playerid, vx, vy, vz);
	}
	return 1;
}

RaceFailForPlayer(mapid, playerid) {
	// Send fail gamext
	new message[MAX_INPUT];
	format(message, sizeof(message), "You have failed the %s.", modes[raceMaps[mapid][rcModeID]][mName]);
	SendClientMessage(playerid, COLOUR_ERROR, message);
	GameTextForPlayer(playerid, "Race Failed!", 5000, 6);
	
	/*format(message, sizeof(message), "%s failed %s",
		pData[playerid][pLoginName], modes[raceMaps[mapid][rcModeID]][mName]);
	SideChatAddLine(message);*/
	
	// Exit race
	RemovePlayerFromMode(playerid, "failed");
	return 1;
}

RaceGetSuggestedWinnings(mapid, type = 0) { // type 0 for xp, 1 for cash
	new Float:winTime = raceMaps[mapid][rcWinTime];
	new Float:totalTime = float(raceMaps[mapid][rcFailTime]) + float(raceMaps[mapid][rcLeftTime]) + float(raceMaps[mapid][rcWinTime]);
	new raceLength = raceMaps[mapid][rcWinTime] / raceMaps[mapid][rcWinCount];
	
	/*new raceRecordCount;
	for (new i; i < MAX_RACE_RECORDS; i++) {
		if (raceMapRecords[mapid][i][rcPlayer][0]) {
			raceLength += raceMapRecords[mapid][i][rcTime];
			raceRecordCount++;
		}
	}
	raceLength = (raceLength / raceRecordCount) / 1000;*/
	
	if (winTime < 1.0) winTime = totalTime;
	
	if (!type) {
		return floatround((totalTime / winTime) * float(raceDynamicXPMultiplier * raceLength), floatround_round);
	} else {
		return floatround((totalTime / winTime) * float(raceDynamicCashMultiplier * raceLength), floatround_round);
	}
}

forward fdm_race_checkRaces();
public fdm_race_checkRaces() {
	LoopRacesLoaded(i) {
		// Check countdown
		if (raceMaps[i][rcCountDown]) {
			raceMaps[i][rcCountDown]--;
			if (!raceMaps[i][rcCountDown]) {
				LoopPlayers(p) {
					if (racePStats[p][rcMapID] != i) continue;
					GameTextForPlayer(p, "GO!", 2000, 6);
					TogglePlayerControllable(p, 1);
					if (racePStats[p][rcpVehicleID] != INVALID_VEHICLE_ID) {
						if (modes[raceMaps[i][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) {
							UpdateVehicleDamageStatus(racePStats[p][rcpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
						} else {
							UpdateVehicleDamageStatus(racePStats[p][rcpVehicleID], 0, 0, 0, 0);
						}
						SetVehicleHealth(racePStats[p][rcpVehicleID], raceMaps[i][rcVehicleHealth]);
					}
					DisableGodmode(p);
					
					new Float:x, Float:y, Float:z;
					GetPlayerPos(p, x, y, z);
					PlayerPlaySound(p, SOUND_START, x, y, z);
				}
				
				RaceUpdatePositions(i);
				raceMaps[i][rcStartTime] = GetTickCount();
			} else {
				LoopPlayers(p) {
					if (racePStats[p][rcMapID] != i) continue;
					new gametext[MAX_INPUT];
					format(gametext, sizeof(gametext), "%i", raceMaps[i][rcCountDown]);
					GameTextForPlayer(p, gametext, 1000, 6);
					
					new Float:x, Float:y, Float:z;
					GetPlayerPos(p, x, y, z);
					
					PlayerPlaySound(p, SOUND_COUNTDOWN, x, y, z);
					
					if (racePStats[p][rcpVehicleID] != INVALID_VEHICLE_ID) {
						if (GetPlayerVehicleID(p) != racePStats[p][rcpVehicleID]) { // If their vehicle is not equal to their given vehicle
							PutPlayerInVehicle(p, racePStats[p][rcpVehicleID], 0);
							SetCameraBehindPlayer(p);
						} else {
							new setpos;
							if (racePStats[p][rcpJustSpawned]) {
								setpos = 1;
								racePStats[p][rcpJustSpawned] = 0;
							} else {
								if (modeObjects[raceMaps[i][rcModeID]][0][oModelID]) {
									if (raceMaps[i][rcCountDown] == 1) {
										setpos = 1;
									} else {
										new Float:dist;
										GetVehiclePos(racePStats[p][rcpVehicleID], x, y, z);
										dist = FindDistance(x, y, z,
											racePStats[p][rcpSpawnX], racePStats[p][rcpSpawnY], racePStats[p][rcpSpawnZ]);
										if (dist > 1.0) setpos = 1;
									}
								}
							}
							if (setpos) {
								SetVehiclePos(racePStats[p][rcpVehicleID],
									racePStats[p][rcpSpawnX],
									racePStats[p][rcpSpawnY],
									racePStats[p][rcpSpawnZ]);
								SetVehicleZAngle(racePStats[p][rcpVehicleID],
									racePStats[p][rcpSpawnRotZ]);
								SetCameraBehindPlayer(p);
							}
						}
					}
				}
			}
		} else {
			
			// Check time limit on each race
			if (GetTickCount() - raceMaps[i][rcStartTime] > raceMaps[i][rcRunTime] * 1000) {
				StopMinimode(raceMaps[i][rcModeID]); // Time for race to end
			}
			
			// Update time elapsed
			new tmpstr[MAX_INPUT];
			format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Time: ~w~%s", GetTimeString2(GetTickCount() - raceMaps[i][rcStartTime]));
			
			LoopPlayers(p) {
				if (racePStats[p][rcMapID] == i) {
					TextDrawSetString(racePStats[p][rcpTextTimer], tmpstr);
				}
			}
		}
	}
}

forward fdm_race_raceTimes();
public fdm_race_raceTimes() {
	LoopRacesLoaded(i) {
		RaceUpdatePositions(i);
	}
}

RaceUpdatePlayerCheckpoint(mapid, playerid) { // Player positions and text draws
	if (!racePStats[playerid][rcpIsTextDrawOn]) return 1;
	new message[MAX_INPUT];
	
	format(message, sizeof(message), "~g~~h~~h~Checkpoint: ~w~%i / %i", racePStats[playerid][rcCheckpointID] - 1, raceMaps[mapid][rcCheckpointCount]);
	TextDrawSetString(racePStats[playerid][rcpTextCheckpoint], message);
	return 1;
}

RaceUpdatePositions(mapid) {	
	new playerpositions[MAX_PLAYERS];
	new playercount;
	new string[MAX_INPUT];
	
	GetPlayerPositions(mapid, playerpositions, playercount);
	
	for (new position; position < MAX_PLAYERS; position++) {
		if (playerpositions[position] == INVALID_PLAYER_ID) break;
		
		format(string, sizeof(string), "~g~~h~~h~Position: ~w~%i / %i", 
			raceMaps[mapid][rcWinPlayerCount] + position + 1, raceMaps[mapid][rcPlayerCount]);
		
		if (!racePStats[playerpositions[position]][rcpTextPositionStr][0] || strcmp(string, racePStats[playerpositions[position]][rcpTextPositionStr])) {
			TextDrawSetString(racePStats[playerpositions[position]][rcpTextPosition], string);
			strcpy(racePStats[playerpositions[position]][rcpTextPositionStr], string, MAX_INPUT);
		}
	}
	
	return 1;
}

GetPlayerPositions(mapid, playerpositions[MAX_PLAYERS], &playercount) {
	if (!raceMaps[mapid][rcIsLoaded]) return 0;
	
	new pcheckpoint[MAX_PLAYERS],
		Float:distance[MAX_PLAYERS];
	
	// Create needed data
	playercount = 0;
	for (new i; i < MAX_PLAYERS; i++) {
		if (racePStats[i][rcMapID] != mapid) {
			pcheckpoint[i] = 0x80000000;
			playerpositions[i] = INVALID_PLAYER_ID;
		} else {
			pcheckpoint[i] = racePStats[i][rcCheckpointID];
			playerpositions[i] = i;
			playercount++;
		}
	}
	
	// Sort by checkpoints and distance
	
	new i = 1;
	new j = 2;
	while (i < MAX_PLAYERS) {
		// Sort by checkpoints
		if (pcheckpoint[i - 1] >= pcheckpoint[i]) {
			// Sort by distance
			if (pcheckpoint[i - 1] == pcheckpoint[i] && playerpositions[i] != INVALID_PLAYER_ID) {
				new Float:x, Float:y, Float:z;
				new checkpointid = pcheckpoint[i] - 1;
				
				if (distance[playerpositions[i - 1]] == 0.0) {
					if (racePStats[playerpositions[i - 1]][rcpVehicleID]) {
						GetVehiclePos(racePStats[playerpositions[i - 1]][rcpVehicleID], x, y, z);
					} else {
						GetPlayerPos(playerpositions[i - 1], x, y, z);
					}
					distance[playerpositions[i - 1]] = FindDistance(x, y, z,
						raceMapCPs[mapid][checkpointid][rcX],
						raceMapCPs[mapid][checkpointid][rcY],
						raceMapCPs[mapid][checkpointid][rcZ]);
				}
				
				if (distance[playerpositions[i]] == 0.0) {
					if (racePStats[playerpositions[i]][rcpVehicleID]) {
						GetVehiclePos(racePStats[playerpositions[i]][rcpVehicleID], x, y, z);
					} else {
						GetPlayerPos(playerpositions[i], x, y, z);
					}
					distance[playerpositions[i]] = FindDistance(x, y, z,
						raceMapCPs[mapid][checkpointid][rcX],
						raceMapCPs[mapid][checkpointid][rcY],
						raceMapCPs[mapid][checkpointid][rcZ]);
				}
				
				if (distance[playerpositions[i - 1]] <= distance[playerpositions[i]]) {
					i = j;
					j = j + 1;
				} else {
					//start of swap
					new tmp = playerpositions[i];
					playerpositions[i] = playerpositions[i - 1];
					playerpositions[i - 1] = tmp;
					tmp = pcheckpoint[i];
					pcheckpoint[i] = pcheckpoint[i - 1];
					pcheckpoint[i - 1] = tmp;
					//end of swap
					
					i = i - 1;
					if (!i) i = 1;
				}
			} else {
				i = j;
				j = j + 1;
			}
		} else {
			//start of swap
			new tmp;
			tmp = playerpositions[i];
			playerpositions[i] = playerpositions[i - 1];
			playerpositions[i - 1] = tmp;
			tmp = pcheckpoint[i];
			pcheckpoint[i] = pcheckpoint[i - 1];
			pcheckpoint[i - 1] = tmp;
			//end of swap
			
			i = i - 1;
			if (!i) i = 1;
		}
	}
	
	return 1;
}

RaceUpdatePlayerInfoShow(playerid) {
	if (racePStats[playerid][rcpIsTextDrawOn]) return 1;
	racePStats[playerid][rcpIsTextDrawOn] = 1;
	
	racePStats[playerid][rcpTextTimer] = TextDrawCreate(20.0, 280.0, "~g~~h~~h~Time: ~w~0");
	racePStats[playerid][rcpTextCheckpoint] = TextDrawCreate(20.0, 300.0, " ");
	racePStats[playerid][rcpTextPositionStr][0] = ' ';
	racePStats[playerid][rcpTextPosition] = TextDrawCreate(20.0, 320.0, racePStats[playerid][rcpTextPositionStr]);
	
	TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextTimer]);
	TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextCheckpoint]);
	TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextPosition]);
	return 1;
}

RaceUpdatePlayerInfoHide(playerid) {
	if (!racePStats[playerid][rcpIsTextDrawOn]) return 1;
	new Text:tmp;
	racePStats[playerid][rcpIsTextDrawOn] = 0;
	TextDrawDestroy(racePStats[playerid][rcpTextCheckpoint]);
	TextDrawDestroy(racePStats[playerid][rcpTextTimer]);
	TextDrawDestroy(racePStats[playerid][rcpTextPosition]);
	racePStats[playerid][rcpTextCheckpoint] = tmp;
	racePStats[playerid][rcpTextTimer] = tmp;
	racePStats[playerid][rcpTextPosition] = tmp;
	strclr(racePStats[playerid][rcpTextPositionStr]);
	return 1;
}

RaceUpdatePlayerInfoUnload() { // Destroy all textdraws
	LoopPlayers(i) {
		RaceUpdatePlayerInfoHide(i);
	}
}

//-----------------------------------------------------------------------------------------------------
// PICKUPS
//-----------------------------------------------------------------------------------------------------

forward fdm_race_PickupCheck();
public fdm_race_PickupCheck() {
	LoopPlayers(playerid) {
		if (racePStats[playerid][rcpPlayerUpdated]) {
			new mapid = racePStats[playerid][rcMapID];
			if (mapid == INVALID_MAP_ID) continue;
			
			// Syncing player armour to vehicle hp
			new Float:syncArmour;
			if (racePStats[playerid][rcpVehicleID]) {
				GetVehicleHealth(racePStats[playerid][rcpVehicleID], syncArmour);
				syncArmour = ((syncArmour - 245) / (raceMaps[mapid][rcVehicleHealth] - 245)) * 100.0;
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
			if (modes[raceMaps[mapid][rcModeID]][mVirtualWorld] != GetPlayerVirtualWorld(playerid)) continue;
			for (new racepickupid; racepickupid < MAX_RACE_PICKUPS; racepickupid++) {
				if (raceMapPickups[mapid][racepickupid][rcpPickupID] == INVALID_PICKUP_ID) break;
				
				if (IsPlayerInRangeOfPoint(playerid, 4.0, raceMapPickups[mapid][racepickupid][rcpX], raceMapPickups[mapid][racepickupid][rcpY], raceMapPickups[mapid][racepickupid][rcpZ])) {
					if (!racePStats[playerid][rcpIsInPickup][racepickupid]) {
						racePStats[playerid][rcpIsInPickup][racepickupid] = 1;
						race_OnPlayerPickUpPickup(playerid, raceMapPickups[mapid][racepickupid][rcpPickupID]);
					}
				} else {
					racePStats[playerid][rcpIsInPickup][racepickupid] = 0;
				}
			}
			
			racePStats[playerid][rcpPlayerUpdated] = 0;
		}
	}
	return 1;
}

race_OnPlayerUpdate(playerid) {
	racePStats[playerid][rcpPlayerUpdated] = 1;
	return 1;
}

race_OnPlayerPickUpPickup(playerid, pickupid) {
	new mapid = racePickupsLink[pickupid][rcplMapID];
	new racepickupid = racePickupsLink[pickupid][rcplPickupID];
	
	if (mapid == INVALID_MAP_ID ||
		racepickupid == INVALID_PICKUP_ID ||
		mapid != modes[raceMaps[mapid][rcModeID]][mMapID]) return 1;
	
	if (racePStats[playerid][rcpVehicleID] == INVALID_VEHICLE_ID) {
		if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return 1;
	} else {
		if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER || GetPlayerVehicleID(playerid) != racePStats[playerid][rcpVehicleID]) {
			racePStats[playerid][rcpPickupAbused] = 1;
			return 1;
		}
	}
	
	new pickupvalid = 0;
	
	switch (raceMapPickups[mapid][racepickupid][rcpType]) {
		case RACE_PICKUP_TYPE_REPAIR: {
			if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
				if (modes[raceMaps[mapid][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) {
					UpdateVehicleDamageStatus(racePStats[playerid][rcpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
				} else {
					UpdateVehicleDamageStatus(racePStats[playerid][rcpVehicleID], 0, 0, 0, 0);
				}
				SetVehicleHealth(racePStats[playerid][rcpVehicleID], raceMaps[mapid][rcVehicleHealth]);
			}
			SetPlayerHealth(playerid, 100.0);
			pickupvalid = 1;
		}
		case RACE_PICKUP_TYPE_NOS: {
			if (racePStats[playerid][rcpVehicleID] != INVALID_VEHICLE_ID) {
				AddVehicleComponent(racePStats[playerid][rcpVehicleID], 1010);
			}
			pickupvalid = 1;
		}
		case RACE_PICKUP_TYPE_VEHICLE: {
			RaceSwitchPlayerVehicle(mapid, playerid, raceMapPickups[mapid][racepickupid][rcpVehicleModel]);
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

race_OnVehicleRepair(vehicleid) {
	LoopPlayers(i) {
		if (racePStats[i][rcMapID] == INVALID_MAP_ID) continue;
		if (racePStats[i][rcpVehicleID] != vehicleid) continue;
		if (modes[raceMaps[racePStats[i][rcMapID]][rcModeID]][mFlags] & MODE_FLAG_DAMAGED) {
			UpdateVehicleDamageStatus(racePStats[i][rcpVehicleID], RACE_VEHICLE_DAMAGED_STATUS);
		}
		return 1;
	}
	return 1;
}

racePickupLoad(mapid) {
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		switch (raceMapPickups[mapid][i][rcpType]) {
			case RACE_PICKUP_TYPE_REPAIR: {
				raceMapPickups[mapid][i][rcpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_REPAIR, RACE_PICKUP_TYPE, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ], modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
				raceMapPickups[mapid][i][rcpTextID] = CreateDynamic3DTextLabel("Repair", 0xAAFFAAFF, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[raceMaps[mapid][rcModeID]][mVirtualWorld], -1, -1, 120.0);
			}
			case RACE_PICKUP_TYPE_NOS: {
				raceMapPickups[mapid][i][rcpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_NOS, RACE_PICKUP_TYPE, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ], modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
				raceMapPickups[mapid][i][rcpTextID] = CreateDynamic3DTextLabel("Nitrous", 0xAAAAFFFF, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[raceMaps[mapid][rcModeID]][mVirtualWorld], -1, -1, 120.0);
			}
			case RACE_PICKUP_TYPE_VEHICLE: {
				raceMapPickups[mapid][i][rcpPickupID] = CreateDynamicPickup(RACE_PICKUP_MODEL_VEHICLE, RACE_PICKUP_TYPE, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ], modes[raceMaps[mapid][rcModeID]][mVirtualWorld]);
				raceMapPickups[mapid][i][rcpTextID] = CreateDynamic3DTextLabel(GetVehicleName(raceMapPickups[mapid][i][rcpVehicleModel]), 0xFFFFFFFF, raceMapPickups[mapid][i][rcpX], raceMapPickups[mapid][i][rcpY], raceMapPickups[mapid][i][rcpZ] + 0.5, 100.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, modes[raceMaps[mapid][rcModeID]][mVirtualWorld], -1, -1, 120.0);
			}
		}
		
		if (raceMapPickups[mapid][i][rcpPickupID] != INVALID_PICKUP_ID) {
			racePickupsLink[raceMapPickups[mapid][i][rcpPickupID]][rcplMapID] = mapid;
			racePickupsLink[raceMapPickups[mapid][i][rcpPickupID]][rcplPickupID] = i;
		}
	}
	return 1;
}

racePickupUnload(mapid) {
	for (new i; i < MAX_RACE_PICKUPS; i++) {
		if (raceMapPickups[mapid][i][rcpPickupID] != INVALID_PICKUP_ID) {
			DestroyDynamicPickup(raceMapPickups[mapid][i][rcpPickupID]);
			
			racePickupsLink[raceMapPickups[mapid][i][rcpPickupID]][rcplMapID] = INVALID_MAP_ID;
			racePickupsLink[raceMapPickups[mapid][i][rcpPickupID]][rcplPickupID] = INVALID_PICKUP_ID;
			raceMapPickups[mapid][i][rcpPickupID] = INVALID_PICKUP_ID;
		}
		if (raceMapPickups[mapid][i][rcpTextID] != Text3D:INVALID_3DTEXT_ID) {
			DestroyDynamic3DTextLabel(raceMapPickups[mapid][i][rcpTextID]);
			raceMapPickups[mapid][i][rcpTextID] = Text3D:INVALID_3DTEXT_ID;
		}
	}
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// SPECTATE EVENTS
//-----------------------------------------------------------------------------------------------------

race_OnPlayerSpectate(playerid, specid) {
	if (racePStats[playerid][rcMapID] != INVALID_MAP_ID) {
		if (racePStats[playerid][rcpIsTextDrawOn]) {
			TextDrawHideForPlayer(playerid, racePStats[playerid][rcpTextCheckpoint]);
			TextDrawHideForPlayer(playerid, racePStats[playerid][rcpTextTimer]);
			TextDrawHideForPlayer(playerid, racePStats[playerid][rcpTextPosition]);
		}
	}
	
	if (racePStats[specid][rcMapID] != INVALID_MAP_ID) {
		race_UpdateSpectatePlayer(playerid, specid, 0, 0);
		
		if (racePStats[specid][rcpIsTextDrawOn]) {
			TextDrawShowForPlayer(playerid, racePStats[specid][rcpTextCheckpoint]);
			TextDrawShowForPlayer(playerid, racePStats[specid][rcpTextTimer]);
			TextDrawShowForPlayer(playerid, racePStats[specid][rcpTextPosition]);
		}
	} else {
		DisablePlayerRaceCheckpoint(playerid);
		if (racePStats[specid][rcpIsTextDrawOn]) {
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextCheckpoint]);
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextTimer]);
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextPosition]);
		}
	}
	return 1;
}

race_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (racePStats[newspecid][rcMapID] != INVALID_MAP_ID) {
		race_UpdateSpectatePlayer(playerid, newspecid, 0, 0);
		
		if (racePStats[oldspecid][rcpIsTextDrawOn]) {
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextCheckpoint]);
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextTimer]);
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextPosition]);
		}
		if (racePStats[newspecid][rcpIsTextDrawOn]) {
			TextDrawShowForPlayer(playerid, racePStats[newspecid][rcpTextCheckpoint]);
			TextDrawShowForPlayer(playerid, racePStats[newspecid][rcpTextTimer]);
			TextDrawShowForPlayer(playerid, racePStats[newspecid][rcpTextPosition]);
		}
	} else {
		DisablePlayerRaceCheckpoint(playerid);
		if (racePStats[oldspecid][rcpIsTextDrawOn]) {
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextCheckpoint]);
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextTimer]);
			TextDrawHideForPlayer(playerid, racePStats[oldspecid][rcpTextPosition]);
		}
	}
	return 1;
}

race_OnPlayerUnspectate(playerid, specid) {
	DisablePlayerRaceCheckpoint(playerid);
	
	if (racePStats[specid][rcMapID] != INVALID_MAP_ID) {
		if (racePStats[specid][rcpIsTextDrawOn]) {
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextCheckpoint]);
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextTimer]);
			TextDrawHideForPlayer(playerid, racePStats[specid][rcpTextPosition]);
		}
	}
	
	if (racePStats[playerid][rcMapID] != INVALID_MAP_ID) {
		if (racePStats[playerid][rcpIsTextDrawOn]) {
			TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextCheckpoint]);
			TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextTimer]);
			TextDrawShowForPlayer(playerid, racePStats[playerid][rcpTextPosition]);
		}
		
		new type;
		#define checkpointid racePStats[playerid][rcCheckpointID]-1
		if (checkpointid == raceMaps[racePStats[playerid][rcMapID]][rcCheckpointCount] - 1) { // Finish line CP
			if (!raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcType]) {
				type = 1;
			} else {
				type = 4;
			}
			SetPlayerRaceCheckpoint(playerid, type,
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcX],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcY],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcZ],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid - 1][rcX],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid - 1][rcY],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid - 1][rcZ],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcSize]);
			
		} else { // Normal CP
			if (!raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcType]) {
				type = 0;
			} else {
				type = 3;
			}
			SetPlayerRaceCheckpoint(playerid, type,
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcX],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcY],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcZ],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid + 1][rcX],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid + 1][rcY],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid + 1][rcZ],
				raceMapCPs[racePStats[playerid][rcMapID]][checkpointid][rcSize]);
			
		}
		#undef checkpointid
	}
	return 1;
}

race_UpdateSpectatePlayer(playerid, specid, finish = 0, sound = 1) {
	DisablePlayerRaceCheckpoint(playerid);
	
	if (finish) {
		if (sound) {
			new Float:x, Float:y, Float:z;
			GetPlayerPos(playerid, x, y, z);
			PlayerPlaySound(playerid, SOUND_FINISH, x, y, z);
		}
	} else {
		new type;
		#define checkpointid racePStats[specid][rcCheckpointID]-1
		if (raceMaps[racePStats[specid][rcMapID]][rcCheckpointCount] == 1) {
			if (!raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcType]) {
				type = 1;
			} else {
				type = 4;
			}
			SetPlayerRaceCheckpoint(playerid, type,
				raceMapCPs[racePStats[specid][rcMapID]][0][rcX],
				raceMapCPs[racePStats[specid][rcMapID]][0][rcY],
				raceMapCPs[racePStats[specid][rcMapID]][0][rcZ],
				racePStats[specid][rcpSpawnX],
				racePStats[specid][rcpSpawnY],
				racePStats[specid][rcpSpawnZ],
				raceMapCPs[racePStats[specid][rcMapID]][0][rcSize]);
			
		} else {
			if (checkpointid == raceMaps[racePStats[specid][rcMapID]][rcCheckpointCount] - 1) { // Finish line CP
				if (!raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcType]) {
					type = 1;
				} else {
					type = 4;
				}
				SetPlayerRaceCheckpoint(playerid, type,
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcX],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcY],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcZ],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid - 1][rcX],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid - 1][rcY],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid - 1][rcZ],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcSize]);
				
			} else { // Normal CP
				if (!raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcType]) {
					type = 0;
				} else {
					type = 3;
				}
				SetPlayerRaceCheckpoint(playerid, type,
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcX],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcY],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcZ],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid + 1][rcX],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid + 1][rcY],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid + 1][rcZ],
					raceMapCPs[racePStats[specid][rcMapID]][checkpointid][rcSize]);
				
			}
		}
		
		if (sound) {
			new Float:x, Float:y, Float:z;
			GetPlayerPos(playerid, x, y, z);
			PlayerPlaySound(playerid, RACE_SOUND_CHECKPOINT, x, y, z);
		}
		#undef checkpointid
	}
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:aresetracestats(playerid, text[]) {
	new message[MAX_INPUT];
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) { // Admin needed
		format(message, sizeof(message), "RESET RACE DATA: You must be at least admin level %i to use /aresetracestats", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /aresetracestats [racemode id/name]");
		return 1;
	}
	
	new modeid = FindMinimode(text, MODE_TYPE_RACE);
	if (modeid == INVALID_MODE_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "RESET RACE DATA: Invalid mode ID/Name.");
		return 1;
	}
	
	new mapid = modes[modeid][mMapID];
	
	raceMaps[mapid][rcWinCount] = 0;
	raceMaps[mapid][rcFailCount] = 0;
	raceMaps[mapid][rcLeftCount] = 0;
	raceMaps[mapid][rcWinTime] = 0;
	raceMaps[mapid][rcFailTime] = 0;
	raceMaps[mapid][rcLeftTime] = 0;
	SaveRaceMapStats(mapid);
	
	format(message, sizeof(message), "RESET RACE DATA: Race statistics for %s has been reset.", modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	format(message, sizeof(message), "%s has reset race statistics for %s", ReturnPlayerName(playerid), modes[modeid][mName]);
	AddAdminLogLine(COLOUR_LOG, message);
	printf("[FDM] %s", message);
	
	return 1;
}

kcmd:adeleterecord(playerid, text[]) {
	new message[MAX_INPUT];
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MODERATOR) { // Admin needed
		format(message, sizeof(message), "DELETE RECORD: You must be at least admin level %i to use /adeleterecord", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /adeleterecord [playername] [racemode id/name]");
		return 1;
	}
	
	new modename[MAX_INPUT];
	new modeid;
	new playername[MAX_INPUT];
	new idx;
	
	splitcpy(playername, text, idx, ' ');
	splitcpy(modename, text, idx, 0);
	
	if (!modename[0] || !playername[0]) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /adeleterecord [playername] [racemode id/name]");
		return 1;
	}
	
	modeid = FindMinimode(modename, MODE_TYPE_RACE);
	if (modeid == INVALID_MODE_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "DELETE RECORD: Invalid mode ID/Name.");
		return 1;
	}
	/*if (modes[modeid][mType] != MODE_TYPE_RACE) {
		format(message, sizeof(message), "DELETE RECORD: %s is not a race.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}*/
	if (!RemoveRecord(modes[modeid][mMapID], playername)) {
		format(message, sizeof(message), "DELETE RECORD: The record for '%s' in '%s' cannot be found.", playername, modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	format(message, sizeof(message), "DELETE RECORD: The record for '%s' in '%s' was deleted.", playername, modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	new adminname[MAX_NAME];
	GetPlayerName(playerid, adminname, sizeof(adminname));
	format(message, sizeof(message), "%s deleted the race record for %s in %s", adminname, playername, modes[modeid][mName]);
	AddAdminLogLine(COLOUR_LOG, message);
	printf("[FDM] %s", message);
	return 1;
}

kcmd:adeleteracerecords(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) { // Admin needed
		format(message, sizeof(message), "DELETE RECORD: You must be at least admin level %i to use /adeleteracerecords", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /adeleteracerecords [racemode id/name]");
		return 1;
	}
	
	new modeid = FindMinimode(text, MODE_TYPE_RACE);
	if (modeid == INVALID_MODE_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "DELETE RECORD: Invalid mode id/name.");
		return 1;
	}
	if (!RemoveRaceRecords(modes[modeid][mMapID])) {
		format(message, sizeof(message), "DELETE RECORD: There are no records for '%s'.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	format(message, sizeof(message), "DELETE RECORD: Records for '%s' deleted.", modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	new adminname[MAX_NAME];
	GetPlayerName(playerid, adminname, sizeof(adminname));
	format(message, sizeof(message), "%s deleted all race records for %s", adminname, modes[modeid][mName]);
	AddAdminLogLine(COLOUR_LOG, message);
	printf("[FDM] %s", message);
	return 1;
}

kcmd:adeleteplayerrecords(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MODERATOR) { // Admin needed
		format(message, sizeof(message), "DELETE RECORD: You must be at least admin level %i to use /adeleteplayerrecords", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /adeleteplayerrecords [playername]");
		return 1;
	}
	
	new recordscount = RemovePlayerRecords(text);
	
	if (!recordscount) {
		format(message, sizeof(message), "DELETE RECORD: '%s' did not have any records.", text);
		SendClientMessage(playerid, COLOUR_ERROR, message);
	} else {
		format(message, sizeof(message), "DELETE RECORD: %i records were deleted from '%s'.", recordscount, text);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		new adminname[MAX_NAME];
		GetPlayerName(playerid, adminname, sizeof(adminname));
		format(message, sizeof(message), "%s deleted all race records for player %s (%i records)", adminname, text, recordscount);
		AddAdminLogLine(COLOUR_LOG, message);
		printf("[FDM] %s", message);
	}
	
	return 1;
}

kcmd:amergerecords(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) { // Admin needed
		format(message, sizeof(message), "MERGE RECORDS: You must be at least admin level %i to use /amergerecords", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /amergerecords [foldername]");
		return 1;
	}
	
	new lastChar = text[strlen(text) - 1];
	if (lastChar != '/' && lastChar != '\\') {
		format(message, sizeof(message), "MERGE RECORDS: '%s' is not a folder. Example: /amergerecords Records2/", text);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (!fexist(text)) {
		format(message, sizeof(message), "MERGE RECORDS: Folder '%s' not found.", text);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (!INI_Exist(FILE_RACE) || !INI_Open(FILE_RACE)) {
		format(message, sizeof(message), "MERGE RECORDS: Could not load '%s'.", FILE_RACE);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new raceFiles[MAX_RACE_MAPS][MAX_FILENAME];
	new raceCount;
	new totalRecordAddCount;
	new j;
	
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "RACE", false)) {
			strcpy(raceFiles[j], gCache[l][E_VALUE], MAX_FILENAME);
			j++;
			raceCount = j;
		}
		if (j >= MAX_RACE_MAPS) {
			break; // Maximum races reached
		}
	}
	
	for (new i; i < raceCount; i++) {
		// Load record file to merge
		new fileName[MAX_FILENAME];
		format(fileName, sizeof(fileName), "%s%s", text, raceFiles[i]);
		
		if (!INI_Exist(fileName) || !INI_Open(fileName)) {
			continue;
		}
		
		// Find matching current record
		new fdmRecordFile[MAX_FILENAME], mapid = INVALID_MAP_ID;
		format(fdmRecordFile, sizeof(fdmRecordFile), "%s%s", FILE_RACE_STATS, raceFiles[i]);
		
		for (j = 0; j < MAX_RACE_MAPS; j++) {
			if (raceMaps[j][rcModeID] == INVALID_MODE_ID) {
				continue;
			}
			
			if (!strcmp(raceMaps[j][rcRecordFile], fdmRecordFile)) {
				mapid = j;
				break;
			}
		}
		
		if (mapid == INVALID_MAP_ID) {
			INI_Close();
			continue;
		}
		
		// Load the records
		new key[INI_MAX_KEY], value[INI_MAX_VALUE], playerName[MAX_NAME], playerRecordTime, recordAddCount;
		for (j = 0; j < MAX_RACE_RECORDS; j++) {
			format(key, sizeof(key), "RECORD%i", j);
			if (!INI_ReadString(value, key)) continue;
			
			new idx;
			splitcpy(playerName, value, idx, ' ', MAX_NAME);
			playerRecordTime = strval(split(value, idx, ' '));
			
			// Add the record
			recordAddCount += AddRecord(mapid, playerName, playerRecordTime);
		}
		
		// Close
		INI_Close();
		
		// Save
		if (recordAddCount) {
			totalRecordAddCount += recordAddCount;
			SaveRaceMapStats(mapid);
		}
	}
	
	if (totalRecordAddCount) {
		format(message, sizeof(message), "MERGE RECORDS: %i records were merged.", totalRecordAddCount);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		format(message, sizeof(message), "%s merged %i race records from the folder '%s'", ReturnPlayerName(playerid), totalRecordAddCount, text);
		AddAdminLogLine(COLOUR_LOG, message);
		printf("[FDM] %s", message);
	} else {
		format(message, sizeof(message), "MERGE RECORDS: No records were merged.", text);
		SendClientMessage(playerid, COLOUR_ERROR, message);
	}
	return 1;
}

kcmd:record(playerid, text[]) return kcmd_records(playerid, text);
kcmd:records(playerid, text[]) {
	new modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /records [racemode id/name]", "RECORDS: Invalid race mode ID/Name.", text, MODE_TYPE_RACE);
	if (modeid == INVALID_MODE_ID) return 1;
	
	ShowPlayerRaceRecords(modes[modeid][mMapID], playerid, MAX_RACE_RECORDS);
	return 1;
}

kcmd:myrecords(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must registered to use /myrecords (/register [password])");
		return 1;
	}
	
	new message[2048];
	new checkpos;
	new count;
	new totalcount;
	new tmp;
	new lines;
	
	if (isnull(text)) {
		checkpos = 0;
	} else {
		checkpos = strval(text);
	}
	
	if (!checkpos) {
		SendClientMessage(playerid, COLOUR_TITLE, "Your race records (All):");
	} else {
		if (checkpos < 1 || checkpos > MAX_RACE_RECORDS) {
			format(message, sizeof(message), "USAGE: /myrecords [1-%i]", MAX_RACE_RECORDS);
			SendClientMessage(playerid, COLOUR_HELP, message);
			return 1;
		} else {
			format(message, sizeof(message), "Your race records (%s):", racestr(checkpos));
			SendClientMessage(playerid, COLOUR_TITLE, message);
		}
	}
	
	for (new position = checkpos - 1; position < MAX_RACE_RECORDS; position++) {
		if (!checkpos) {
			format(message, sizeof(message), "%s - ", racestr(position + 1));
		} else {
			message = WRAPPED_MESSAGE_PREFIX;
		}
		
		for (new a; a < MAX_RACE_MAPS; a++) {
			if (raceMaps[a][rcModeID] == INVALID_MODE_ID) continue; // No race there
			
			tmp = 0;
			for (new b; b < MAX_RACE_RECORDS; b++) {
				if (!raceMapRecords[a][b][rcTime]) continue; // Invalid record
				
				if (tmp == position) {
					if (!strcmp(raceMapRecords[a][b][rcPlayer], pData[playerid][pLoginName], true)) { // Record
						strcat(message, modes[raceMaps[a][rcModeID]][mName][6]);
						strcat(message, ",  ");
						count++;
					}
					break;
				}
				
				tmp++;
			}
		}
		
		if (count) {
			totalcount += count;
			count = 0;
			message[strlen(message) - 3] = 0;
			lines += SendWrappedMessageToPlayer(playerid, COLOUR_HELP, message);
		}
		
		if (checkpos) break;
	}
	
	if (!totalcount) {
		if (checkpos) {
			format(message, sizeof(message), "You have no records in position %i.", checkpos);
			SendClientMessage(playerid, COLOUR_ATTENTION, message);
		} else {
			SendClientMessage(playerid, COLOUR_ATTENTION, "You have no records.");
		}
	} else {
		if (lines > 8) {
			SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
		}
	}
	
	return 1;
}

kcmd:playerrecords(playerid, text[]) {
	new message[2048];
	new playerName[MAX_PLAYER_NAME];
	new checkid;
	new checkpos;
	new count;
	new totalcount;
	new tmp;
	new lines;
	
	if (isnull(text)) {
		checkid = playerid;
		
		if (pData[checkid][pIsLoggedIn]) {
			strcpy(playerName, pData[checkid][pLoginName]);
		} else {
			GetPlayerName(checkid, playerName, sizeof(playerName));
		}
	} else {
		new idx;
		splitcpy(message, text, idx, ' ');
		
		checkid = FindPlayer(message);
		if (checkid == INVALID_PLAYER_ID) {
			/*SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: Invalid player ID/Name.");
			return 1;*/
			strcpy(playerName, message);
			if (message[0] == '"' && message[strlen(message) - 1] == '"') {
				strcpy(playerName, message[1], strlen(message) - 1);
			} else {
				strcpy(playerName, message);
			}
		} else {
			if (pData[checkid][pIsLoggedIn]) {
				strcpy(playerName, pData[checkid][pLoginName]);
			} else {
				GetPlayerName(checkid, playerName, sizeof(playerName));
			}
		}
		
		splitcpy(message, text, idx, ' ');
		
		if (isnull(message)) {
			checkpos = 0;
		} else {
			checkpos = strval(message);
		}
	}
	
	/*if (!pData[checkid][pIsLoggedIn]) {
		format(message, sizeof(message), "MINIMODES: %s must be registered to check their records.", ReturnPlayerName(checkid));
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}*/
	
	if (!checkpos) {
		format(message, sizeof(message), "%s's race records (All):", playerName);
		SendClientMessage(playerid, COLOUR_TITLE, message);
	} else {
		if (checkpos < 1 || checkpos > MAX_RACE_RECORDS) {
			format(message, sizeof(message), "USAGE: /playerrecords [player] [1-%i]", MAX_RACE_RECORDS);
			SendClientMessage(playerid, COLOUR_HELP, message);
			return 1;
		} else {
			format(message, sizeof(message), "%s's race records (%s):", playerName, racestr(checkpos));
			SendClientMessage(playerid, COLOUR_TITLE, message);
		}
	}
	
	for (new position = checkpos - 1; position < MAX_RACE_RECORDS; position++) {
		if (!checkpos) {
			format(message, sizeof(message), "%s - ", racestr(position + 1));
		} else {
			message = WRAPPED_MESSAGE_PREFIX;
		}
		
		for (new a; a < MAX_RACE_MAPS; a++) {
			if (raceMaps[a][rcModeID] == INVALID_MODE_ID) continue; // No race there
			
			tmp = 0;
			for (new b; b < MAX_RACE_RECORDS; b++) {
				if (!raceMapRecords[a][b][rcTime]) continue; // Invalid record
				
				if (tmp == position) {
					if (!strcmp(raceMapRecords[a][b][rcPlayer], playerName, true)) { // Record
						strcat(message, modes[raceMaps[a][rcModeID]][mName][6]);
						strcat(message, ",  ");
						count++;
					}
					break;
				}
				
				tmp++;
			}
		}
		
		if (count) {
			totalcount += count;
			count = 0;
			message[strlen(message) - 3] = 0;
			lines += SendWrappedMessageToPlayer(playerid, COLOUR_HELP, message);
		}
		
		if (checkpos) break;
	}
	
	if (!totalcount) {
		if (checkpos) {
			format(message, sizeof(message), "%s has no records in position %i.", playerName, checkpos);
			SendClientMessage(playerid, COLOUR_ATTENTION, message);
		} else {
			format(message, sizeof(message), "%s has no records.", playerName, checkpos);
			SendClientMessage(playerid, COLOUR_ATTENTION, message);
		}
	} else {
		if (lines > 8) {
			SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
		}
	}
	
	return 1;
}
