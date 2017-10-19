#include <a_samp>

#define FILE_DEATHMATCH_FFA "FDM/Minimodes/DeathMatchFFAList.ini"
#define FILE_DEATHMATCH_FFA_MAPS "FDM/Minimodes/Deathmatch_FFA/"

#if defined CRIPPLE_PLAYERS
	#define MAX_DEATHMATCH_FFA_MAPS 6
#else
	#define MAX_DEATHMATCH_FFA_MAPS 16 // Maximum deathmatches which can be loaded
#endif
#define MAX_DEATHMATCH_SPAWNS 64
#define MAX_DM_BOUNDS_TIME 5
#define MAX_DM_BOUND_POLYS 32

#if !defined MAX_WEAPON_ID
	#define MAX_WEAPON_ID 47
#endif

#define DM_GODE_TIME 2000

#define DM_BLIP_STEALTH 0
#define DM_BLIP_HIDDEN 1
#define DM_BLIP_SHOWN 2

#define DM_STEALTH_VISIBILITY_TIME 3000

enum dmffaSpawnInfo {
	Float:dmfX,
	Float:dmfY,
	Float:dmfZ,
	Float:dmfRot
}
enum dmffaBoundsRadiusInfo {
	dmfrUsed,
	Float:dmfCentreX,
	Float:dmfCentreY,
	Float:dmfCentreZ,
	Float:dmfRadius,
}
enum dmffaBoundsRectangleInfo {
	dmfeUsed,
	Float:dmfeMinX,
	Float:dmfeMinY,
	Float:dmfeMinZ,
	Float:dmfeMaxX,
	Float:dmfeMaxY,
	Float:dmfeMaxZ
}
enum dmffaInfo {
	dmfIsLoaded,
	dmfModeID,
	dmfStartTime,
	dmfRunTime,
	dmfWinBaseXP,
	dmfWinBaseCash,
	dmfKillXP,
	dmfBoundsRadius[dmffaBoundsRadiusInfo],
	dmfBoundsRectangle[dmffaBoundsRectangleInfo],
	dmfBoundPoly[E_POLYGON],
	dmfWeapons[MAX_WEAPON_ID],
	//dmfPlayers[MAX_PLAYERS],
	dmfCountDown,
	Float:dmfPlayerHealth,
	Float:dmfPlayerArmour,
	dmfEnemyBlips,
	dmfBlockJoinMidGame
}
enum dmffaPlayerStatsInfo {
	dmfpMapID,
	dmfpKills,
	dmfpDeaths,
	dmfpBoundsTime,
	dmfpIsTextDrawOn,
	Text:dmfpTime,
	Text:dmfpText,
	dmfpTextString[MAX_INPUT]
}
new dmfPStats[MAX_PLAYERS][dmffaPlayerStatsInfo];
new dmffaMaps[MAX_DEATHMATCH_FFA_MAPS][dmffaInfo];
new dmffaSpawns[MAX_DEATHMATCH_FFA_MAPS][MAX_DEATHMATCH_SPAWNS][dmffaSpawnInfo];
// new dmffaTimer;

new dmffaInProgress[MAX_DEATHMATCH_FFA_MAPS + 1];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

DeathmatchFFAInit() {
	for (new i; i < MAX_DEATHMATCH_FFA_MAPS; i++) {
		dmffaMaps[i][dmfModeID] = INVALID_MODE_ID;
	}
	for (new i; i < MAX_PLAYERS; i++) {
		dmfPStats[i][dmfpMapID] = INVALID_MAP_ID;
	}
	dmffaInProgress[0] = INVALID_MAP_ID;
	
	LoadDeathMatchFFAMaps();
	
	kEnableTimer(7); // dmffaTimer = SetTimer("fdm_dmffa_checkDM", 925, true);
	kEnableTimer(22); // fdm_dmffa_checkDMFast
	return 1;
}

DeathmatchFFAUnload() {
	new tmp1[dmffaInfo];
	new tmp2[dmffaSpawnInfo];
	new tmp3[dmffaPlayerStatsInfo];
	
	dmffaUpdatePlayerInfoUnload();
	
	for (new a; a < MAX_DEATHMATCH_FFA_MAPS; a++) {
		dmffaMaps[a] = tmp1;
		for (new b; b < MAX_DEATHMATCH_SPAWNS; b++) {
			dmffaSpawns[a][b] = tmp2;
		}
	}
	
	for (new c; c < MAX_PLAYERS; c++) {
		dmfPStats[c] = tmp3;
	}
	
	kDisableTimer(7); // KillTimer(dmffaTimer);
	kDisableTimer(22); // fdm_dmffa_checkDMFast
}

LoadDeathMatchFFAMaps() {
	if (!INI_Exist(FILE_DEATHMATCH_FFA)) {
		printf("[FDM] WARNING: DMFFA Map List Fail: '%s' - File not found.", FILE_DEATHMATCH_FFA);
		return 0;
	}
	
	if (!INI_Open(FILE_DEATHMATCH_FFA)) {
		printf("[FDM] WARNING: DMFFA Map List Fail: '%s' - File cannot be opened.", FILE_DEATHMATCH_FFA);
		return 0;
	}
	
	new dmFiles[MAX_DEATHMATCH_FFA_MAPS][MAX_FILENAME];
	new dmcount;
	new dmsLoaded;
	//new key[INI_MAX_KEY];
	
	new i3;
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "DMFFA", false)) {
			format(dmFiles[i3], MAX_FILENAME, "%s%s", FILE_DEATHMATCH_FFA_MAPS, gCache[l][E_VALUE]);
			i3++;
			dmcount = i3;
		}
		if (i3 >= MAX_DEATHMATCH_FFA_MAPS) {
			printf("[FDM] WARNING: Maximum Deathmatch Team maps reached (%i derbies)", MAX_DEATHMATCH_FFA_MAPS);
			break; // Maximum dmteams reached
		}
	}
	/*for (new i; i < MAX_DEATHMATCH_FFA_MAPS; i++) { // Read DM list
		format(key, sizeof(key), "DMFFA%i", i);
		
		if (INI_ReadString(dmFiles[i], key)) {
			// Add the file path to the beginning
			format(dmFiles[i], MAX_FILENAME, "%s%s", FILE_DEATHMATCH_FFA_MAPS, dmFiles[i]);
		} else {
			dmcount = i;
			break; // End of file
		}
	}*/
	INI_Close();
	
	if (!dmcount) return 0; // There are no DM's to load
	
	// Load deathmatch map files
	for (new i; i < dmcount; i++) {
		// Load deathmatch files
		if (!INI_Exist(dmFiles[i])) {
			printf("[FDM] WARNING: DMFFA Load Fail: '%s' - File not found.", dmFiles[i]);
			continue;
		}
		if (!INI_Open(dmFiles[i])) {
			printf("[FDM] WARNING: DMFFA Load Fail: '%s' - File cannot be opened.", dmFiles[i]);
			continue;
		}
		
		new dmname[MAX_MODE_NAME];
		new tmpstr[MAX_STRING];
		//new minlevel;
		//new cost;
		new idx;
		
		INI_ReadString(dmname, "Name");
		if (!dmname[0]) { // Invalid DM name
			INI_Close();
			printf("[FDM] WARNING: DMFFA Load Fail: '%s' - File is corrupt/invalid.", dmFiles[i]);
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
		
		// DM data
		dmffaMaps[i][dmfRunTime] = INI_ReadInt("RunTime");
		dmffaMaps[i][dmfWinBaseXP] = INI_ReadInt("WinBaseXP");
		dmffaMaps[i][dmfWinBaseCash] = INI_ReadInt("WinBaseCash");
		dmffaMaps[i][dmfKillXP] = INI_ReadInt("KillXP");
		dmffaMaps[i][dmfEnemyBlips] = INI_ReadInt("EnemyBlips");
		dmffaMaps[i][dmfBlockJoinMidGame] = INI_ReadInt("BlockJoinMidGame");
		
		dmffaMaps[i][dmfPlayerHealth] = INI_ReadFloat("PlayerHealth");
		dmffaMaps[i][dmfPlayerArmour] = INI_ReadFloat("PlayerArmour");
		if (dmffaMaps[i][dmfPlayerHealth] == 0.0) dmffaMaps[i][dmfPlayerHealth] = 100.0;
		
		if (INI_ReadString(tmpstr, "Bounds")) { // DM Boundaries
			idx = 0;
			dmffaMaps[i][dmfBoundsRadius][dmfrUsed] = 1;
			dmffaMaps[i][dmfBoundsRadius][dmfCentreX] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRadius][dmfCentreY] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRadius][dmfCentreZ] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRadius][dmfRadius] = floatstr(split(tmpstr, idx, ','));
		} else {
			dmffaMaps[i][dmfBoundsRadius][dmfrUsed] = 0;
		}
		
		idx = 0;
		if (INI_ReadString(tmpstr, "BoundRectangle")) { // DM Boundaries
			dmffaMaps[i][dmfBoundsRectangle][dmfeUsed] = 1;
			dmffaMaps[i][dmfBoundsRectangle][dmfeMinX] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRectangle][dmfeMinY] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRectangle][dmfeMinZ] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRectangle][dmfeMaxX] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRectangle][dmfeMaxY] = floatstr(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfBoundsRectangle][dmfeMaxZ] = floatstr(split(tmpstr, idx, ','));
			
			if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinX] > dmffaMaps[i][dmfBoundsRectangle][dmfeMaxX])
				floatswap(dmffaMaps[i][dmfBoundsRectangle][dmfeMinX], dmffaMaps[i][dmfBoundsRectangle][dmfeMaxX]);
			
			if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinY] > dmffaMaps[i][dmfBoundsRectangle][dmfeMaxY])
				floatswap(dmffaMaps[i][dmfBoundsRectangle][dmfeMinY], dmffaMaps[i][dmfBoundsRectangle][dmfeMaxY]);
			
			if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinZ] > dmffaMaps[i][dmfBoundsRectangle][dmfeMaxZ])
				floatswap(dmffaMaps[i][dmfBoundsRectangle][dmfeMinZ], dmffaMaps[i][dmfBoundsRectangle][dmfeMaxZ]);
			
		} else {
			dmffaMaps[i][dmfBoundsRectangle][dmfeUsed] = 0;
		}
		
		new i2;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "BoundPolyPoint", false)) {
				idx = 0;
				dmffaMaps[i][dmfBoundPoly][E_VERTEX_X][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmffaMaps[i][dmfBoundPoly][E_VERTEX_Y][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				
				i2++;
			}
			if (i2 >= MAX_VERTICIES) break; // Maximum polys reached
		}
		dmffaMaps[i][dmfBoundPoly][E_VERTICIES] = i2;
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "WPN", false)) {
				idx = 0;
				new weaponidx = strval(split(gCache[l][E_VALUE], idx, ','));
				dmffaMaps[i][dmfWeapons][weaponidx] = strval(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_WEAPON_ID) break; // Maximum weapons reached
		}
		/*for (new p; p < MAX_WEAPON_ID; p++) { // Weapons in deathmatch
			format(key, sizeof(key), "W%i", p);
			if (!INI_ReadString(tmpstr, key)) break; // End of weapons
			
			idx = 0;
			new weaponidx = strval(split(tmpstr, idx, ','));
			dmffaMaps[i][dmfWeapons][weaponidx] = strval(split(tmpstr, idx, ','));
		}*/
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "SPAWN", false)) {
				idx = 0;
				dmffaSpawns[i][i2][dmfX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmffaSpawns[i][i2][dmfY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmffaSpawns[i][i2][dmfZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmffaSpawns[i][i2][dmfRot] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_DEATHMATCH_SPAWNS) break; // Maximum spawns reached
		}
		/*for (new s; s < MAX_DEATHMATCH_SPAWNS; s++) { // Deathmatch spawn loations
			format(key, sizeof(key), "Spawn%i", s);
			if (!INI_ReadString(tmpstr, key)) break; // End of spawns
			
			idx = 0;
			dmffaSpawns[i][s][dmfX] = floatstr(split(tmpstr, idx, ','));
			dmffaSpawns[i][s][dmfY] = floatstr(split(tmpstr, idx, ','));
			dmffaSpawns[i][s][dmfZ] = floatstr(split(tmpstr, idx, ','));
			dmffaSpawns[i][s][dmfRot] = floatstr(split(tmpstr, idx, ','));
		}*/
		
		// Deathmatch map reading complete
		//format(dmname, sizeof(dmname), "Deathmatch FFA: %s", dmname);
		//dmffaMaps[i][dmfModeID] = CreateMinimode(MODE_TYPE_DMFFA, dmname, i, minlevel, cost, x, y, z);
		dmffaMaps[i][dmfModeID] = LoadMinimode(MODE_TYPE_DMFFA, dmname, i, "", 2, MAX_PLAYERS, 1);
		INI_Close();
		
		if (dmffaMaps[i][dmfModeID] != INVALID_MODE_ID) {
			dmsLoaded++;
		}
	}
	
	printf("[FDM] Deathmatch FFA's Loaded: %i", dmsLoaded);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

UpdateDMFFAInProgress() {
	new indexLoaded;
	
	for (new mapid; mapid < MAX_DEATHMATCH_FFA_MAPS; mapid++) {
		if (dmffaMaps[mapid][dmfIsLoaded]) {
			dmffaInProgress[indexLoaded] = mapid;
			indexLoaded++;
		}
	}
	
	dmffaInProgress[indexLoaded] = INVALID_MAP_ID;
	return 1;
}

#define LoopDMFFALoaded(%1) for (new mmc, %1; dmffaInProgress[mmc] != INVALID_MAP_ID; mmc++) if ((%1 = dmffaInProgress[mmc]) || true)


//-----------------------------------------------------------------------------------------------------
// ACTUAL DEATHMATCHES
//-----------------------------------------------------------------------------------------------------

dmffa_IsJoinable(mapid) {
	if (!dmffaMaps[mapid][dmfIsLoaded]) return 0;
	if (GetTickCount() - dmffaMaps[mapid][dmfStartTime] > dmffaMaps[mapid][dmfRunTime] * 500) return 0;
	return 1;
}

dmffa_ShowModeInfo(mapid, playerid) {
	new msg[MAX_STRING];
	
	// Show blipmode/health/armour
	if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_STEALTH) {
		msg = "Blips: Stealth";
	} else if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_SHOWN) {
		msg = "Blips: Always Visible";
	} else {
		msg = "Blips: Hidden";
	}
	
	format(msg, sizeof(msg), "%s | Health: %0.1f | Armour: %0.1f | Time Limit: %s",
		msg, dmffaMaps[mapid][dmfPlayerHealth], dmffaMaps[mapid][dmfPlayerArmour], GetTimeString(dmffaMaps[mapid][dmfRunTime] * 1000, 0));
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	// Show weapons
	msg = "Weapons: ";
	new weaponcount;
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!dmffaMaps[mapid][dmfWeapons][i]) continue;
		strcat(msg, ReturnWeaponName(i));
		
		if (!IsWeaponMelee(i)) {
			new strammo[MAX_REAL + 2];
			format(strammo, sizeof(strammo), "(%i)", dmffaMaps[mapid][dmfWeapons][i]);
			strcat(msg, strammo);
		}
		
		strcat(msg, ", ");
		
		weaponcount++;
	}
	if (weaponcount) {
		msg[strlen(msg) - 2] = 0;
		SendWrappedMessageToPlayer(playerid, COLOUR_HELP, msg);
	} else {
		SendClientMessage(playerid, COLOUR_HELP, "Weapons: None");
	}
	
	return 1;
}

dmffa_OnStart(mapid, players[], playercount) { // When the deathmatch is started by the minimodes handler
	if (dmffaMaps[mapid][dmfIsLoaded]) return 0;
	if (playercount < 2) return 0; // Not enough players
	
	dmffaMaps[mapid][dmfIsLoaded] = 1;
	dmffaMaps[mapid][dmfStartTime] = GetTickCount();
	dmffaMaps[mapid][dmfCountDown] = COUNTDOWN_TIME;
	
	UpdateDMFFAInProgress();
	
	LoopPlayers(i) {
		if (players[i]) {
			if (!dmffa_OnPlayerEnter(mapid, i)) players[i] = 0;
		}
	}
	return 1;
}

dmffa_OnStop(mapid) { // When the deathmatch is stopped by the minimode handler
	if (!dmffaMaps[mapid][dmfIsLoaded]) return 0;
	dmffaMaps[mapid][dmfIsLoaded] = 0;
	
	UpdateDMFFAInProgress();
	
	// Announce winner
	new playerPositions[MAX_PLAYERS], playerCount, sameCount, message[MAX_INPUT];
	dmffaGetWinnerPositions(mapid, playerPositions, playerCount);
	
	if (playerCount == 0) {
		format(message, sizeof(message), "%s: No-one won this deathmatch.",
			modes[dmffaMaps[mapid][dmfModeID]][mName]);
		SendClientMessageToAll(COLOUR_TITLE, message);
		
	} else if (playerCount == 1) {
		format(message, sizeof(message), "%s: %s has won with %i kills, %i deaths.",
			modes[dmffaMaps[mapid][dmfModeID]][mName], pData[playerPositions[0]][pLoginName], dmfPStats[playerPositions[0]][dmfpKills], dmfPStats[playerPositions[0]][dmfpDeaths]);
		SendClientMessageToAll(COLOUR_TITLE, message);
		
		new playerXP = MinimodeProcessFlagsXP(dmffaMaps[mapid][dmfModeID], dmffaMaps[mapid][dmfWinBaseXP] + ((dmffaMaps[mapid][dmfWinBaseXP] * (playerCount + 1)) / 3));
		new playerCash = MinimodeProcessFlagsMoney(dmffaMaps[mapid][dmfModeID], dmffaMaps[mapid][dmfWinBaseCash] + ((dmffaMaps[mapid][dmfWinBaseCash] * (playerCount + 1)) / 3));
		
		SendXPMessageToPlayer(playerPositions[0], GivePlayerXP(playerPositions[0], playerXP));
		GivePlayerMoney(playerPositions[0], playerCash);
		SendMoneyMessageToPlayer(playerPositions[0], playerCash);
		
	} else {
		new playerXP[MAX_PLAYERS], playerCash[MAX_PLAYERS], playerPos[MAX_PLAYERS];
		
		for (new i; i < playerCount; i++) {
			playerXP[i] = MinimodeProcessFlagsXP(dmffaMaps[mapid][dmfModeID], (dmffaMaps[mapid][dmfWinBaseXP] + ((dmffaMaps[mapid][dmfWinBaseXP] * (playerCount + 1)) / 3)) / (i + 1));
			playerCash[i] = MinimodeProcessFlagsMoney(dmffaMaps[mapid][dmfModeID], (dmffaMaps[mapid][dmfWinBaseCash] + ((dmffaMaps[mapid][dmfWinBaseCash] * (playerCount + 1)) / 3)) / (i + 1));
			
			if (i) {
				if (dmfPStats[playerPositions[i]][dmfpKills] - dmfPStats[playerPositions[i]][dmfpDeaths] == dmfPStats[playerPositions[i - 1]][dmfpKills] - dmfPStats[playerPositions[i - 1]][dmfpDeaths]) {
					sameCount++;
				} else {
					if (sameCount) {
						new newXP, newCash;
						
						for (new j = i - sameCount - 1; j < i; j++) {
							newXP += playerXP[j];
							newCash += playerCash[j];
						}
						
						newXP = newXP / (sameCount + 1);
						newCash = newCash / (sameCount + 1);
						
						for (new j = i - sameCount - 1; j < i; j++) {
							playerXP[j] = newXP;
							playerCash[j] = newCash;
						}
						
						sameCount = 0;
					}
				}
			}
			
			playerPos[i] = i - sameCount + 1;
		}
		
		if (sameCount) {
			new newXP, newCash;
			
			for (new j = playerCount - sameCount - 1; j < playerCount; j++) {
				newXP += playerXP[j];
				newCash += playerCash[j];
			}
			
			newXP = newXP / (sameCount + 1);
			newCash = newCash / (sameCount + 1);
			
			for (new j = playerCount - sameCount - 1; j < playerCount; j++) {
				playerXP[j] = newXP;
				playerCash[j] = newCash;
			}
		}
		
		if (dmfPStats[playerPositions[0]][dmfpKills] - dmfPStats[playerPositions[0]][dmfpDeaths] == dmfPStats[playerPositions[1]][dmfpKills] - dmfPStats[playerPositions[1]][dmfpDeaths]) {
			format(message, sizeof(message), "%s: Multiple players won this deathmatch.",
				modes[dmffaMaps[mapid][dmfModeID]][mName]);
			SendClientMessageToAll(COLOUR_TITLE, message);
		} else {
			format(message, sizeof(message), "%s: %s has won with %i kills, %i deaths.",
				modes[dmffaMaps[mapid][dmfModeID]][mName], pData[playerPositions[0]][pLoginName], dmfPStats[playerPositions[0]][dmfpKills], dmfPStats[playerPositions[0]][dmfpDeaths]);
			SendClientMessageToAll(COLOUR_TITLE, message);
		}
		
		for (new i; i < playerCount; i++) {
			new playerid = playerPositions[i];
			
			if ((i < MAX_PLAYERS - 1 && playerPositions[i + 1] != INVALID_PLAYER_ID && dmfPStats[playerPositions[i]][dmfpKills] - dmfPStats[playerPositions[i]][dmfpDeaths] == dmfPStats[playerPositions[i + 1]][dmfpKills] - dmfPStats[playerPositions[i + 1]][dmfpDeaths]) ||
				(i > 0 && playerPositions[i - 1] != INVALID_PLAYER_ID && dmfPStats[playerPositions[i]][dmfpKills] - dmfPStats[playerPositions[i]][dmfpDeaths] == dmfPStats[playerPositions[i - 1]][dmfpKills] - dmfPStats[playerPositions[i - 1]][dmfpDeaths])) {
				if (playerPos[i] == 1) {
					format(message, sizeof(message), "%s: You won with %i kills, %i deaths in %s equal.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				} else if (playerPos[i] == playerCount) {
					format(message, sizeof(message), "%s: You lost with %i kills, %i deaths in %s equal.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				} else {
					format(message, sizeof(message), "%s: You finished with %i kills, %i deaths in %s equal.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				}
			} else {
				if (playerPos[i] == 1) {
					format(message, sizeof(message), "%s: You won with %i kills, %i deaths in %s place.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				} else if (playerPos[i] == playerCount) {
					format(message, sizeof(message), "%s: You lost with %i kills, %i deaths in %s place.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				} else {
					format(message, sizeof(message), "%s: You finished with %i kills, %i deaths in %s place.",
						modes[dmffaMaps[mapid][dmfModeID]][mName], dmfPStats[playerPositions[i]][dmfpKills], dmfPStats[playerPositions[i]][dmfpDeaths], racestr(playerPos[i]));
				}
			}
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
			
			SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, playerXP[i]));
			GivePlayerMoney(playerid, playerCash[i]);
			SendMoneyMessageToPlayer(playerid, playerCash[i]);
		}
	}
	
	/*for (new i; i < MAX_RACE_RECORDS && recordcount < count; i++) {
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
	}*/
	
	/*if (mapid == INVALID_MAP_ID) return INVALID_PLAYER_ID;
	if (!dmffaMaps[mapid][dmfIsLoaded]) return INVALID_PLAYER_ID;
	
	// Find winner
	new winnerid = INVALID_PLAYER_ID;
	new winnerscore;
	
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths] > winnerscore) {
			winnerid = i;
			winnerscore = dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths];
		}
	}
	if (winnerid == INVALID_PLAYER_ID) return INVALID_PLAYER_ID;
	
	// Check for duplicating winners
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (winnerid == i) continue;
		
		if (winnerscore == dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths]) {
			return INVALID_PLAYER_ID;
		}
	}
	
	return winnerid;*/
	
	/*new message[MAX_INPUT];
	if (winnerid == INVALID_PLAYER_ID) {
		format(message, sizeof(message), "%s: No-one won this deathmatch.",
			modes[dmffaMaps[mapid][dmfModeID]][mName]);
		SendClientMessageToAll(COLOUR_TITLE, message);
	} else {
		format(message, sizeof(message), "%s: %s has won with %i kills, %i deaths.",
			modes[dmffaMaps[mapid][dmfModeID]][mName], pData[winnerid][pLoginName], winnerkills, winnerdeaths);
		SendClientMessageToAll(COLOUR_TITLE, message);
		
		new xp = MinimodeProcessFlagsXP(dmffaMaps[mapid][dmfModeID], (dmffaMaps[mapid][dmfWinBaseXP] + ((dmffaMaps[mapid][dmfWinBaseXP] * (playercount + 1)) / 3)));
		SendXPMessageToPlayer(winnerid, GivePlayerXP(winnerid, xp));
		
		new money = MinimodeProcessFlagsMoney(dmffaMaps[mapid][dmfModeID], dmffaMaps[mapid][dmfWinBaseCash] + ((dmffaMaps[mapid][dmfWinBaseCash] * (playercount + 1)) / 3));
		GivePlayerMoney(winnerid, money);
		SendMoneyMessageToPlayer(winnerid, money);
	}*/
	
	// Clean up players
	LoopPlayers(i) {
		//if (!dmffaMaps[mapid][dmfPlayers][i]) continue;
		if (mpData[i][mpModeID] != dmffaMaps[mapid][dmfModeID]) continue;
		
		dmffa_OnPlayerExit(mapid, i);
		
		new Float:x, Float:y, Float:z;
		GetPlayerPos(i, x, y, z);
		PlayerPlaySound(i, SOUND_FINISH, x, y, z);
	}
	
	return 1;
}

dmffa_OnPlayerEnter(mapid, playerid) { // When a player enters the deathmatch while it's in progress
	if (dmfPStats[playerid][dmfpMapID] != INVALID_MAP_ID) return 0;
	
	if (dmffaMaps[mapid][dmfBlockJoinMidGame] && !dmffaMaps[mapid][dmfCountDown]) return 0; // Block join mid game
	if (GetTickCount() - dmffaMaps[mapid][dmfStartTime] > dmffaMaps[mapid][dmfRunTime] * 500) return 0; // Block join mid game when half finished
	
	dmfPStats[playerid][dmfpMapID] = mapid;
	dmfPStats[playerid][dmfpBoundsTime] = 0;
	//dmffaMaps[mapid][dmfPlayers][playerid] = 1;
	dmffaSetRandomSpawn(mapid, playerid);
	
	if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_HIDDEN || dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_STEALTH) {
		SetPlayerColor(playerid, GetPlayerColor(playerid) & 0xFFFFFF00);
		LoopPlayers(i) {
			if (dmfPStats[i][dmfpMapID] == mapid) {
				ShowPlayerNameTagForPlayer(i, playerid, 0);
			} else {
				ShowPlayerNameTagForPlayer(i, playerid, 1);
			}
		}
	} else {
		LoopPlayers(i) {
			if (dmfPStats[i][dmfpMapID] == mapid) {
				ShowPlayerNameTagForPlayer(i, playerid, 1);
			}
		}
	}
	
	if (dmffaMaps[mapid][dmfCountDown]) {
		TogglePlayerControllable(playerid, 0); // Freeze player
		EnableGodmode(playerid);
	} else {
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	
	SetPlayerVirtualWorld(playerid, modes[dmffaMaps[mapid][dmfModeID]][mVirtualWorld]);
	SetPlayerHealth(playerid, dmffaMaps[mapid][dmfPlayerHealth]);
	SetPlayerArmour(playerid, dmffaMaps[mapid][dmfPlayerArmour]);
	
	new potentialspawns[MAX_DEATHMATCH_SPAWNS];
	new spawncount;
	for (new i; i < MAX_DEATHMATCH_SPAWNS; i++) {
		if (dmffaSpawns[mapid][i][dmfX] == 0.0 &&
			dmffaSpawns[mapid][i][dmfY] == 0.0 &&
			dmffaSpawns[mapid][i][dmfZ] == 0.0 &&
			dmffaSpawns[mapid][i][dmfRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		if (IsPlayerSpawned(playerid)) {
			SetPlayerPos(playerid,
				dmffaSpawns[mapid][potentialspawns[rnd]][dmfX], dmffaSpawns[mapid][potentialspawns[rnd]][dmfY], dmffaSpawns[mapid][potentialspawns[rnd]][dmfZ]);
			SetPlayerFacingAngle(playerid, dmffaSpawns[mapid][potentialspawns[rnd]][dmfRot]);
			
			// Give player their weapons
			dmffaGivePlayerWeapons(mapid, playerid);
		} else {
			SkinsSetNextSpawnPos(playerid,
				dmffaSpawns[mapid][potentialspawns[rnd]][dmfX], dmffaSpawns[mapid][potentialspawns[rnd]][dmfY], dmffaSpawns[mapid][potentialspawns[rnd]][dmfZ], dmffaSpawns[mapid][potentialspawns[rnd]][dmfRot]);
		}
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			dmffaMaps[mapid][dmfBoundsRadius][dmfCentreX], dmffaMaps[mapid][dmfBoundsRadius][dmfCentreY], dmffaMaps[mapid][dmfBoundsRadius][dmfCentreZ], float(random(360)));
	}
	
	SetCameraBehindPlayer(playerid);
	
	dmffaUpdatePlayerInfoShow(playerid);
	dmffaUpdatePlayersText(mapid);
	
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		dmffa_OnPlayerSpectate(i, playerid);
	}
	
	return 1;
}

dmffa_OnPlayerExit(mapid, playerid) { // When a player exits the deathmatch while it's in progress
	
	dmfPStats[playerid][dmfpMapID] = INVALID_MAP_ID;
	dmfPStats[playerid][dmfpKills] = 0;
	dmfPStats[playerid][dmfpDeaths] = 0;
	//dmffaMaps[mapid][dmfPlayers][playerid] = 0;
	GivePlayerGangColour(playerid); // Reset player colour
	
	LoopPlayers(i) {
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	
	SetLastSpawnTick(playerid);
	if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) SetPlayerVirtualWorld(playerid, 0);
	
	if (IsPlayerSpawned(playerid)) {
		SetPlayerPosRandom(playerid,
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mX],
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mY],
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mZ], 5);
		SetPlayerHealth(playerid, 100.0);
	} else {
		SkinsSetNextSpawnPos(playerid,
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mX],
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mY],
			modes[dmffaMaps[mapid][dmfModeID]][mLocation][mZ], float(random(360)));
	}
	
	if (dmffaMaps[mapid][dmfCountDown]) {
		// Player may still be frozen
		TogglePlayerControllable(playerid, 1);
	}
	
	dmffaUpdatePlayerInfoHide(playerid);
	dmffaUpdatePlayersText(mapid);
	
	return 1;
}

dmffa_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED) &&
		!((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		oldstate != PLAYER_STATE_SPECTATING) {
		// Basically OnPlayerSpawn
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	return 1;
}

dmffa_OnPlayerSpawn(mapid, playerid) { // When the player spawns in the DM
	if (dmfPStats[playerid][dmfpMapID] == INVALID_MAP_ID) return 0;
	if (dmfPStats[playerid][dmfpMapID] != mapid) return 0;
	
	SetPlayerHealth(playerid, dmffaMaps[mapid][dmfPlayerHealth]);
	SetPlayerArmour(playerid, dmffaMaps[mapid][dmfPlayerArmour]);
	dmffaGivePlayerWeapons(mapid, playerid);
	
	if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_HIDDEN) {
		SetPlayerColor(playerid, (GetPlayerColor(playerid) & 0xFFFFFF00));
		LoopPlayers(i) {
			if (dmfPStats[i][dmfpMapID] == mapid) {
				ShowPlayerNameTagForPlayer(i, playerid, 0);
			}
		}
	} else if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_STEALTH) {
		dmffaUpdatePlayerAlpha(mapid, playerid);
	}
	return 1;
}

dmffa_OnPlayerDeath(mapid, playerid, killerid, reason) { // When the player dies in the DM
	if (dmfPStats[playerid][dmfpMapID] == INVALID_MAP_ID) return 0;
	if (dmfPStats[playerid][dmfpMapID] != mapid) return 0;
	
	//new message[MAX_INPUT];
	dmffaSetRandomSpawn(mapid, playerid);
	dmfPStats[playerid][dmfpDeaths]++;
	
	if (killerid != INVALID_PLAYER_ID && dmfPStats[killerid][dmfpMapID] == mapid) {
		dmfPStats[killerid][dmfpKills]++;
		
		new drunkxp = GetPlayerDrunkLevel(playerid);
		if (drunkxp < 0) drunkxp = 0;
		if (drunkxp > 5000) drunkxp = 5000;
		drunkxp = drunkxp * dmffaMaps[mapid][dmfKillXP] / 5000;
		
		new xp = MinimodeProcessFlagsXP(dmffaMaps[mapid][dmfModeID], dmffaMaps[mapid][dmfKillXP] + drunkxp);
		SendXPMessageToPlayer(killerid, GivePlayerXP(killerid, xp));
		
	} else if (killerid == INVALID_PLAYER_ID && reason == 54) { // Fall death
		dmfPStats[playerid][dmfpDeaths]++;
		GameTextForPlayer(killerid, "Don't jump off the cliff..", 3000, 5);
	}
	
	dmffaUpdatePlayersText(mapid);
	
	return 1;
}

dmffaUpdatePlayerAlpha(mapid, playerid) {
	
	LoopPlayers(i) {
		if (dmfPStats[playerid][dmfpMapID] != mapid) continue;
		if (playerid == i) continue;
		
		new volume = GetPlayerVolumeToPlayerByte(playerid, i);
		
		SetPlayerMarkerForPlayer(i, playerid, (GetPlayerColor(playerid) & 0xFFFFFF00) | volume);
		if (volume) {
			ShowPlayerNameTagForPlayer(i, playerid, 1);
		} else {
			ShowPlayerNameTagForPlayer(i, playerid, 0);
		}
	}
	return 1;
}

dmffa_OnPlayerVolumeChange(mapid, playerid) {
	if (dmffaMaps[mapid][dmfEnemyBlips] == DM_BLIP_STEALTH) {
		dmffaUpdatePlayerAlpha(mapid, playerid);
	}
	return 1;
}

forward fdm_dmffa_checkDM();
public fdm_dmffa_checkDM() {
	// End the deathmatches when the time runs out
	LoopDMFFALoaded(i) {
		if (GetTickCount() - dmffaMaps[i][dmfStartTime] > dmffaMaps[i][dmfRunTime] * 1000) {
			// Time for deathmatch to end
			StopMinimode(dmffaMaps[i][dmfModeID]);
		} else {
			// Check countdown for DM
			if (dmffaMaps[i][dmfCountDown]) {
				dmffaMaps[i][dmfCountDown]--;
				
				if (!dmffaMaps[i][dmfCountDown]) {
					LoopPlayers(c) {
						if (dmfPStats[c][dmfpMapID] != i) continue;
						GameTextForPlayer(c, "Open Fire!", 1000, 6);
						TogglePlayerControllable(c, 1);
						StartGodSpawning(c, DM_GODE_TIME, 0);
						dmffaMaps[i][dmfStartTime] = GetTickCount();
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_START, x, y, z);
					}
				} else {
					LoopPlayers(c) {
						if (dmfPStats[c][dmfpMapID] != i) continue;
						new gametext[MAX_INPUT];
						format(gametext, sizeof(gametext), "%i", dmffaMaps[i][dmfCountDown]);
						GameTextForPlayer(c, gametext, 1000, 6);
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_COUNTDOWN, x, y, z);
					}
				}
			}
			
			// Check player distances for DM's
			if (!dmffaMaps[i][dmfCountDown]) { // Only after countdown is finished
				LoopPlayers(p) {
					if (dmfPStats[p][dmfpMapID] != i) continue;
					if (!IsPlayerSpawned(p)) continue;
					
					if (dmffaMaps[i][dmfBoundsRadius][dmfrUsed] || dmffaMaps[i][dmfBoundsRectangle][dmfeUsed] || dmffaMaps[i][dmfBoundPoly][E_VERTICIES] >= 2) {
						new Float:x, Float:y, Float:z, outofbounds;
						GetPlayerPos(p, x, y, z);
						
						// Rectangular check
						if (dmffaMaps[i][dmfBoundsRectangle][dmfeUsed]) {
							if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinX] > x) {
								outofbounds = 1;
							} else if (dmffaMaps[i][dmfBoundsRectangle][dmfeMaxX] < x) {
								outofbounds = 1;
							} else if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinY] > y) {
								outofbounds = 1;
							} else if (dmffaMaps[i][dmfBoundsRectangle][dmfeMaxY] < y) {
								outofbounds = 1;
							} else if (dmffaMaps[i][dmfBoundsRectangle][dmfeMinZ] > z) {
								outofbounds = 1;
							} else if (dmffaMaps[i][dmfBoundsRectangle][dmfeMaxZ] < z) {
								outofbounds = 1;
							}
						}
						
						// Ciricular check
						if (!outofbounds && dmffaMaps[i][dmfBoundsRadius][dmfrUsed]) {
							new Float:distance = FindDistance(x, y, z,
								dmffaMaps[i][dmfBoundsRadius][dmfCentreX], dmffaMaps[i][dmfBoundsRadius][dmfCentreY], dmffaMaps[i][dmfBoundsRadius][dmfCentreZ]);
							
							// Check player distances for DM's
							if (distance > dmffaMaps[i][dmfBoundsRadius][dmfRadius]) {
								outofbounds = 1;
							}
						}
						
						// Polygonal check
						if (!outofbounds && dmffaMaps[i][dmfBoundPoly][E_VERTICIES] >= 2) {
							new tmp[E_POLYGON];
							tmp[E_VERTICIES] = dmffaMaps[i][dmfBoundPoly][E_VERTICIES];
							for (new j; j < tmp[E_VERTICIES]; j++) {
								tmp[E_VERTEX_X][j] = dmffaMaps[i][dmfBoundPoly][E_VERTEX_X][j];
								tmp[E_VERTEX_Y][j] = dmffaMaps[i][dmfBoundPoly][E_VERTEX_Y][j];
							}
							if (!IsPointInPoly(x, y, tmp)) {
								outofbounds = 1;
							}
						}
						
						if (outofbounds) {
							if (!dmfPStats[p][dmfpBoundsTime]) {
								dmfPStats[p][dmfpBoundsTime] = MAX_DM_BOUNDS_TIME;
							} else {
								dmfPStats[p][dmfpBoundsTime]--;
								if (!dmfPStats[p][dmfpBoundsTime]) {
									SetPlayerHealth(p, 0.0); // Kill player if they're outside the DM
									SendClientMessage(p, COLOUR_ATTENTION, "DEATHMATCHES: You were killed for being outside the deathmatch zone.");
								} else {
									new message[MAX_INPUT];
									format(message, sizeof(message), "DEATHMATCHES: Stay within the deathmatch zone or be killed!! (%is to go)", dmfPStats[p][dmfpBoundsTime]);
									SendClientMessage(p, COLOUR_ATTENTION, message);
									format(message, sizeof(message), "~r~STAY IN THE DEATHMATCH ZONE [%is]", dmfPStats[p][dmfpBoundsTime]);
									kGameTextForPlayer(p, message, 3000, 320.0, 280.0);
								}
							}
						} else {
							dmfPStats[p][dmfpBoundsTime] = 0;
						}
					}
					
					/*if (dmffaMaps[i][dmfEnemyBlips] == DM_BLIP_STEALTH) {
						dmffaUpdatePlayerAlpha(i, p);
					}*/
					//	GivePlayerGangColour(p);
				}
				
				dmffaUpdatePlayersTime(i);
			}
		}
	}
}

forward fdm_dmffa_checkDMFast();
public fdm_dmffa_checkDMFast() {
	LoopDMFFALoaded(i) {
		if (!dmffaMaps[i][dmfCountDown]) {
			LoopPlayers(p) {
				if (dmfPStats[p][dmfpMapID] != i) continue;
				if (!IsPlayerSpawned(p)) continue;
				
				if (dmffaMaps[i][dmfEnemyBlips] == DM_BLIP_STEALTH) {
					dmffaUpdatePlayerAlpha(i, p);
				}
			}
		}
	}
}

dmffaSetRandomSpawn(mapid, playerid) {
	new potentialspawns[MAX_DEATHMATCH_SPAWNS];
	new spawncount;
	for (new i; i < MAX_DEATHMATCH_SPAWNS; i++) {
		if (dmffaSpawns[mapid][i][dmfX] == 0.0 &&
			dmffaSpawns[mapid][i][dmfY] == 0.0 &&
			dmffaSpawns[mapid][i][dmfZ] == 0.0 &&
			dmffaSpawns[mapid][i][dmfRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		SkinsSetNextSpawnPos(playerid,
			dmffaSpawns[mapid][potentialspawns[rnd]][dmfX], dmffaSpawns[mapid][potentialspawns[rnd]][dmfY], dmffaSpawns[mapid][potentialspawns[rnd]][dmfZ], dmffaSpawns[mapid][potentialspawns[rnd]][dmfRot]);
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			dmffaMaps[mapid][dmfBoundsRadius][dmfCentreX], dmffaMaps[mapid][dmfBoundsRadius][dmfCentreY], dmffaMaps[mapid][dmfBoundsRadius][dmfCentreZ], float(random(360)));
	}
	return 1;
}

dmffaGivePlayerWeapons(mapid, playerid) {
	ResetPlayerWeapons(playerid);
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!dmffaMaps[mapid][dmfWeapons][i]) continue;
		GivePlayerWeapon(playerid, i, dmffaMaps[mapid][dmfWeapons][i]);
	}
}

dmffaGetWinnerID(mapid) {
	if (mapid == INVALID_MAP_ID) return INVALID_PLAYER_ID;
	if (!dmffaMaps[mapid][dmfIsLoaded]) return INVALID_PLAYER_ID;
	
	// Find winner
	new winnerid = INVALID_PLAYER_ID;
	new winnerscore = 0x80000000;
	
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths] > winnerscore) {
			winnerid = i;
			winnerscore = dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths];
		}
	}
	if (winnerid == INVALID_PLAYER_ID) return INVALID_PLAYER_ID;
	
	// Check for duplicating winners
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (winnerid == i) continue;
		
		if (winnerscore == dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths]) {
			return INVALID_PLAYER_ID;
		}
	}
	
	return winnerid;
}

dmffaGetWinnerPositions(mapid, playerpositions[MAX_PLAYERS], &playercount) {
	new pScore[MAX_PLAYERS];
	
	// Create needed data
	playercount = 0;
	for (new i; i < MAX_PLAYERS; i++) {
		if (dmfPStats[i][dmfpMapID] != mapid) {
			pScore[i] = 0x80000000;
			playerpositions[i] = INVALID_PLAYER_ID;
		} else {
			pScore[i] = dmfPStats[i][dmfpKills] - dmfPStats[i][dmfpDeaths];
			playerpositions[i] = i;
			playercount++;
		}
	}
	
	// Sort by score
	
	new i = 1;
	new j = 2;
	while (i < MAX_PLAYERS) {
		if (pScore[i - 1] >= pScore[i] || playerpositions[i] == INVALID_PLAYER_ID) {
			i = j;
			j += 1;
		} else {
			// swap
			new tmp = pScore[i];
			pScore[i] = pScore[i - 1];
			pScore[i - 1] = tmp;
			
			tmp = playerpositions[i];
			playerpositions[i] = playerpositions[i - 1];
			playerpositions[i - 1] = tmp;
			// end of swap
			
			i = i - 1;
			if (i == 0)
				i = 1;
		}
	}
	
	return 1;
}

dmffaUpdatePlayersText(mapid) { // Player positions and text draws
	
	// Find winner
	new winnerid = dmffaGetWinnerID(mapid);
	new winnername[MAX_NAME];
	if (winnerid == INVALID_PLAYER_ID) {
		winnername = "Nil";
	} else {
		strcpy(winnername, pData[winnerid][pLoginName]);
	}
	
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (!dmfPStats[i][dmfpIsTextDrawOn]) continue;
		
		new message[MAX_INPUT];
		
		format(message, sizeof(message), "~g~~h~~h~Kills: ~w~%i  ~g~~h~~h~Deaths: ~w~%i  ~g~~h~~h~Winning Player: ~w~%s",
			dmfPStats[i][dmfpKills], dmfPStats[i][dmfpDeaths], winnername);
		
		if (!dmfPStats[i][dmfpTextString][0] || strcmp(dmfPStats[i][dmfpTextString], message)) { // Text is different
			TextDrawSetString(dmfPStats[i][dmfpText], message);
			strcpy(dmfPStats[i][dmfpTextString], message, MAX_INPUT);
		}
	}
	return 1;
}

dmffaUpdatePlayersTime(mapid) {
	new text[MAX_INPUT];
	format(text, sizeof(text), "~g~~h~~h~Time Remaining: ~w~%s", GetTimeString2(dmffaMaps[mapid][dmfRunTime] * 1000 - (GetTickCount() - dmffaMaps[mapid][dmfStartTime])));
	
	LoopPlayers(i) {
		if (dmfPStats[i][dmfpMapID] != mapid) continue;
		if (!dmfPStats[i][dmfpIsTextDrawOn]) continue;
		
		TextDrawSetString(dmfPStats[i][dmfpTime], text);
	}
	return 1;
}

dmffaUpdatePlayerInfoShow(playerid) {
	if (dmfPStats[playerid][dmfpIsTextDrawOn]) return 1;
	
	dmfPStats[playerid][dmfpIsTextDrawOn] = 1;
	
	dmfPStats[playerid][dmfpText] = TextDrawCreate(150.0, 430.0, " ");
	dmfPStats[playerid][dmfpTime] = TextDrawCreate(20.0, 300.0, " ");
	
	TextDrawShowForPlayer(playerid, dmfPStats[playerid][dmfpText]);
	TextDrawShowForPlayer(playerid, dmfPStats[playerid][dmfpTime]);
	dmfPStats[playerid][dmfpTextString][0] = ' ';
	return 1;
}

dmffaUpdatePlayerInfoHide(playerid) {
	if (!dmfPStats[playerid][dmfpIsTextDrawOn]) return 1;
	
	dmfPStats[playerid][dmfpIsTextDrawOn] = 0;
	
	new Text:tmp;
	TextDrawDestroy(dmfPStats[playerid][dmfpText]);
	TextDrawDestroy(dmfPStats[playerid][dmfpTime]);
	
	dmfPStats[playerid][dmfpText] = tmp;
	dmfPStats[playerid][dmfpTime] = tmp;
	strclr(dmfPStats[playerid][dmfpTextString]);
	return 1;
}

dmffaUpdatePlayerInfoUnload() { // Destroy all textdraws
	LoopPlayers(i) {
		dmffaUpdatePlayerInfoHide(i);
	}
}

//-----------------------------------------------------------------------------------------------------
// SPECTATE EVENTS
//-----------------------------------------------------------------------------------------------------

dmffa_OnPlayerSpectate(playerid, specid) {
	if (dmfPStats[playerid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[playerid][dmfpIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmfPStats[playerid][dmfpText]);
		TextDrawHideForPlayer(playerid, dmfPStats[playerid][dmfpTime]);
	}
	if (dmfPStats[specid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[specid][dmfpIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmfPStats[specid][dmfpText]);
		TextDrawShowForPlayer(playerid, dmfPStats[specid][dmfpTime]);
	}
	return 1;
}

dmffa_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (dmfPStats[oldspecid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[oldspecid][dmfpIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmfPStats[oldspecid][dmfpText]);
		TextDrawHideForPlayer(playerid, dmfPStats[oldspecid][dmfpTime]);
	}
	if (dmfPStats[newspecid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[newspecid][dmfpIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmfPStats[newspecid][dmfpText]);
		TextDrawShowForPlayer(playerid, dmfPStats[newspecid][dmfpTime]);
	}
	return 1;
}

dmffa_OnPlayerUnspectate(playerid, specid) {
	if (dmfPStats[playerid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[playerid][dmfpIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmfPStats[playerid][dmfpText]);
		TextDrawShowForPlayer(playerid, dmfPStats[playerid][dmfpTime]);
	}
	if (dmfPStats[specid][dmfpMapID] != INVALID_MAP_ID && dmfPStats[specid][dmfpIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmfPStats[specid][dmfpText]);
		TextDrawHideForPlayer(playerid, dmfPStats[specid][dmfpTime]);
	}
	return 1;
}

