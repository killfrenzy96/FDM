#include <a_samp>

#define SPAWN_GODMODE_TIME 10000

enum godspawnInfo {
	pgIsOnGodmode,
	pgGodSpawning,
	pgGodSpawnTick
}
new pGodData[MAX_PLAYERS][godspawnInfo];

GodSpawnInit() {
	kEnableTimer(6); // SetTimer("fdm_GodTimerCheck", 100, true);
	return 1;
}

/*GodSpawnExit() {
	return 1;
}*/

godspawn_OnPlayerDeath(playerid) {
	DisableGodmode(playerid);
	return 1;
}

godspawn_OnPlayerDisconnect(playerid) {
	DisableGodmode(playerid);
	return 1;
}

EnableGodmode(playerid, disableweapons = 1) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (!pGodData[playerid][pgIsOnGodmode]) {
		kTogglePlayerGodmode(playerid, 1);
		if (disableweapons) {
			pGodData[playerid][pgIsOnGodmode] = 1;
		} else {
			pGodData[playerid][pgIsOnGodmode] = 2;
		}
	}
	if (disableweapons) {
		kDisablePlayerWeapons(playerid);
	} else {
		if (!GetPlayerInterior(playerid) || IsPlayerInMinimode(playerid)) {
			kEnablePlayerWeapons(playerid);
		}
	}
	KillGodTimer(playerid);
	return 1;
}

DisableGodmode(playerid, disableweapons = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (pGodData[playerid][pgIsOnGodmode]) {
		kTogglePlayerGodmode(playerid, 0);
		pGodData[playerid][pgIsOnGodmode] = 0;
	}
	if (disableweapons) {
		kDisablePlayerWeapons(playerid);
	} else {
		if (!GetPlayerInterior(playerid) || IsPlayerInMinimode(playerid)) {
			kEnablePlayerWeapons(playerid);
		}
	}
	KillGodTimer(playerid);
	return 1;
}

StartGodSpawning(playerid, time = SPAWN_GODMODE_TIME, disableweapons = 1) {
	if (!IsPlayerConnected(playerid)) return 0;
	EnableGodmode(playerid, disableweapons);
	pGodData[playerid][pgGodSpawning] = 1;
	pGodData[playerid][pgGodSpawnTick] = GetTickCount() + time;
	return 1;
}

KillGodTimer(playerid) {
	pGodData[playerid][pgGodSpawning] = 0;
	return 1;
}

godspawn_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		!((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED)) {
		// Basically OnPlayerUnspawn
		DisableGodmode(playerid);
		
	}/* else if (((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED) &&
		!((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		oldstate != PLAYER_STATE_SPECTATING && !IsPlayerInMinimode(playerid) && !skinPlayerData[playerid][pSelecting]) {
		// Basically OnPlayerSpawn, but not called if the player was spectating or in a minimode
		StartGodSpawning(playerid);
	}*/
	return 1;
}

godspawn_OnPlayerSpawn(playerid) {
	StartGodSpawning(playerid);
	return 1;
}

forward fdm_GodTimerCheck();
public fdm_GodTimerCheck() {
	LoopPlayers(i) {
		if (!pGodData[i][pgGodSpawning]) continue;
		if (GetTickCount() > pGodData[i][pgGodSpawnTick]) {
			DisableGodmode(i);
		}
	}
}
