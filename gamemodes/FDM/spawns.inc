#include <a_samp>

#define FILE_SPAWNS "FDM/Settings/Spawns.ini"
#define MAX_SPAWNS 256

enum spawnPointInfo {
	sExists,
	sMinLevelPercent,
	sMaxLevelPercent,
	Float:xX,
	Float:xY,
	Float:xZ,
	Float:xRot
}
new spawnPoints[MAX_SPAWNS][spawnPointInfo]; // Spawn positions

SpawnsInit() {
	new File:fhd;
	fhd = fopen(FILE_SPAWNS, filemode:io_read);
	if (!fhd) return 0;
	
	new linestr[MAX_STRING],
		sdata[spawnPointInfo];
	
	new i;
	while (i < sizeof(spawnPoints) && fread(fhd, linestr, sizeof(linestr)) > 0) {
		
		if (!linestr[0] || linestr[0] == ';') continue;
		new idx;
		
		sdata[sMinLevelPercent] = strval(split(linestr, idx, ','));
		sdata[sMaxLevelPercent] = strval(split(linestr, idx, ','));
		sdata[xX] = floatstr(split(linestr, idx, ','));
		sdata[xY] = floatstr(split(linestr, idx, ','));
		sdata[xZ] = floatstr(split(linestr, idx, ','));
		sdata[xRot] = floatstr(split(linestr, idx, ','));
		
		if (sdata[xX] == 0.0 &&
			sdata[xY] == 0.0 &&
			sdata[xZ] == 0.0 &&
			sdata[xRot] == 0.0) continue;
		
		sdata[sExists] = 1;
		
		spawnPoints[i] = sdata;
		i++;
	}
	
	fclose(fhd);
	return 1;
}

SpawnsUnload() {
	new tmpdata[spawnPointInfo];
	for (new i; i < sizeof(spawnPoints); i++) {
		spawnPoints[i] = tmpdata;
	}
	return 1;
}

spawns_OnPlayerDeath(playerid) {
	GetNewSpawnPoint(playerid);
	return 1;
}

spawns_OnPlayerSpawn(playerid) {
	GetNewSpawnPoint(playerid);
	return 1;
}

spawns_OnPlayerLevelChange(playerid) {
	GetNewSpawnPoint(playerid);
	return 1;
}

GetNewSpawnPoint(playerid) {
	// Get possible spawn points for player
	new i2;
	new spawns[sizeof(spawnPoints)];
	for (new i1; i1 < sizeof(spawnPoints); i1++) {
		if (!spawnPoints[i1][sExists]) continue;
		if (pData[playerid][pLevel] < (spawnPoints[i1][sMinLevelPercent] * lData[lMax]) / 100) continue;
		if (pData[playerid][pLevel] > (spawnPoints[i1][sMaxLevelPercent] * lData[lMax]) / 100) continue;
		
		spawns[i2] = i1;
		i2++;
	}
	if (!i2) {
		// No avaliable spawn points, default to this location
		SkinsSetNextSpawnPos(playerid, 0.0, 0.0, 5.0, 0.0);
		return 1;
	}
	
	// Randomize spawn points
	new rcount = random(i2);
	SkinsSetNextSpawnPos(playerid, spawnPoints[spawns[rcount]][xX], spawnPoints[spawns[rcount]][xY], spawnPoints[spawns[rcount]][xZ], spawnPoints[spawns[rcount]][xRot]);
	return 1;
}


// IsPlayerSpawned

static pAdminState[MAX_PLAYERS];
static pIsSpawned[MAX_PLAYERS];

stock IsSpawnedInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		pAdminState[i] = GetAdminState(i);
		pIsSpawned[i] = kIsPlayerSpawned(i);
	}
	return 1;
}

stock spawns_OnPlayerDisconnect(playerid) {
	pAdminState[playerid] = 0;
	pIsSpawned[playerid] = 0;
	return 1;
}

stock spawns_OnPlayerStateChange(playerid, newstate) {
	pIsSpawned[playerid] = kIsPlayerSpawned(playerid, newstate);
	return 1;
}

stock spawns_OnPlayerStartSelection(playerid) {
	pIsSpawned[playerid] = kIsPlayerSpawned(playerid);
	return 1;
}

stock spawns_OnPlayerStopSelection(playerid) {
	pIsSpawned[playerid] = kIsPlayerSpawned(playerid);
	return 1;
}

public OnAdminStateChange(playerid, newstate, oldstate) {
	pAdminState[playerid] = newstate;
	pIsSpawned[playerid] = kIsPlayerSpawned(playerid);
	return 1;
}

stock kIsPlayerSpawned(playerid, pstate = -1) {
	if (pstate == -1) pstate = GetPlayerState(playerid);
	if (((pstate >= PLAYER_STATE_ONFOOT && pstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || pstate == PLAYER_STATE_SPAWNED) &&
		!skinPlayerData[playerid][pSelecting] && // Player is in skin selection
		!(pAdminState[playerid] & ADMIN_STATE_JAILED)) { // Player is jailed
		return 1;
	}
	return 0;
}

stock IsPlayerSpawned(playerid) {
	return pIsSpawned[playerid];
}

stock kGetAdminState(playerid) {
	return pAdminState[playerid];
}

// ------------
