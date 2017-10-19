#include <a_samp>

#define FILE_MODE_WAIT_ZONES "FDM/Settings/WaitZones.ini"

#define MODE_LEVEL_REQUIREMENTS false

#define INVALID_MODE_ID -1
#define INVALID_MAP_ID -1
#define MODE_ALLOW_TELEPORT true

#define MODE_TYPE_INVALID 0
#define MODE_TYPE_DMFFA 1
#define MODE_TYPE_RACE 2
#define MODE_TYPE_DMTEAM 3
#define MODE_TYPE_DERBY 4
#define MODE_TYPE_CTF 5

#define COLOUR_MODE_ALLOWED 0xFFAADDAA
#define COLOUR_MODE_BLOCKED 0xAA7799AA

#define COLOUR_MODE_VOTE 0xFF7700AA
#define COLOUR_MODE_JOIN 0xEEAA00AA
#define COLOUR_MODE_LEAVE 0xEEAA00AA
#define COLOUR_MODE_START 0x00DD00AA
#define COLOUR_MODE_STOP 0xDD0000AA
#define COLOUR_MODE_DESCRIPTION 0xFFAA00AA

#define MODE_STATE_UNLOADED 0
#define MODE_STATE_WAITING 1
#define MODE_STATE_PROGRESS 2

#define MODE_START_TIME 60000
#define MODE_MAX_WAITING 8
#define MODE_MAX_WAITZONES 256

#define MODE_MAX_SEARCH_RESULTS 7

#define MIN_START_INTERVAL 75000
#define MIN_START_AFTER_FINISH 30000
#define MIN_QUICKSTART_INTERVAL 15000

#define MODE_TEXT_X 20.0
#define MODE_TEXT_Y 300.0

#if defined CRIPPLE_PLAYERS
	#define MAX_MINIMODES 24
#else
	#define MAX_MINIMODES 768
#endif
#define MODES_PER_PAGE 8

// Shared defines
#define ANY_VEHICLE_ID -1
#define MAX_CAR_COMPONENTS 16

#define COUNTDOWN_TIME 6

#define SOUND_COUNTDOWN 1056
#define SOUND_START 1057
#define SOUND_FINISH 1057
#define SOUND_FAIL 1057

#define INVALID_TEAM_ID -1
#define MAX_MODE_NAME 64
#define MAX_MODE_NOTES 64

#define MODE_OBJECT_MAX 1280
#define OBJECT_DEFAULT_VIEW_DISTANCE 300.0

#define MODE_VEHICLE_MAX 16
#define MODE_VEHICLE_DEFAULT_RESPAWN 60

#define MODE_FLAG_NONE 0
#define MODE_FLAG_DRUNK 1
#define MODE_FLAG_HALFXP 2
#define MODE_FLAG_XP_L1 4
#define MODE_FLAG_XP_L2 8
#define MODE_FLAG_XP_L3 16
#define MODE_FLAG_CASH_L1 32
#define MODE_FLAG_CASH_L2 64
#define MODE_FLAG_CASH_L3 128
#define MODE_FLAG_DAMAGED 256

#define MODE_CHANCE_DRUNK 15
#define MODE_CHANCE_XP_L1 15
#define MODE_CHANCE_XP_L2 30
#define MODE_CHANCE_XP_L3 60
#define MODE_CHANCE_CASH_L1 15
#define MODE_CHANCE_CASH_L2 30
#define MODE_CHANCE_CASH_L3 60
#define MODE_CHANCE_DAMAGED 15

#define MODE_DRUNK_LEVEL 2000000000
#define MODE_MULTIPLIER_XP_L1 4
#define MODE_MULTIPLIER_XP_L2 8
#define MODE_MULTIPLIER_XP_L3 16
#define MODE_CASH_MULTIPLIER_L1 4
#define MODE_CASH_MULTIPLIER_L2 8
#define MODE_CASH_MULTIPLIER_L3 16

#define INVALID_GRAVITY -2.0
#define DEFAULT_GRAVITY 0.008
// --------------------

enum objectInfo {
	oObjectID,
	oModelID,
	Float:oX,
	Float:oY,
	Float:oZ,
	Float:oRotX,
	Float:oRotY,
	Float:oRotZ,
	Float:oViewDistance
}
enum modesVehicleInfo {
	mvVehicleID,
	mvModel,
	Float:mvX,
	Float:mvY,
	Float:mvZ,
	Float:mvRotZ,
	mvColour1,
	mvColour2,
	mvRespawnDelay
}
enum modesWaitLocationInfo {
	Float:mX,
	Float:mY,
	Float:mZ,
	mwInteriorID
}
enum modesWaitZoneInfo {
	Float:mzX,
	Float:mzY,
	Float:mzZ,
	mzInteriorID,
	mzInUse
}
enum modesInfo {
	mType,
	mState,
	mName[MAX_MODE_NAME],
	mFilePath[MAX_STRING],
	mNotes[MAX_MODE_NOTES],
	mDescription[MAX_STRING],
	mAuthor[MAX_NAME],
	mCheckpointID,
	mMapID,
	mStartTime,
	#if MODE_LEVEL_REQUIREMENTS
		mMinLevel,
	#endif
	mCost,
	mLocation[modesWaitLocationInfo],
	mPlayerCount,
	mVirtualWorld,
	mInteriorID,
	mFlags,
	mBlockedFlags,
	#if defined _YSI_included
		Float:mGravity,
	#endif
	mMinPlayers,
	mMaxPlayers,
	
	mEnableRandomWaitZone,
	mWaitZoneUsed,
	
	mTimeBaseTick,
	mTimeChanged,
	mTimeHour,
	mTimeMinute,
	mWeatherChanged,
	mWeather,
	
	Text:mModeName
}
enum playerModesInfo {
	mpModeID,
	mpLastJoinModeID,
	mpVoteMinimode,
	mpTextExists,
	Text:mpText,
	mpStartDelay,
	mpLoadWeapons
}
new mpData[MAX_PLAYERS][playerModesInfo];
new modes[MAX_MINIMODES][modesInfo];
new modeObjects[MAX_MINIMODES][MODE_OBJECT_MAX][objectInfo];
new modeVehicles[MAX_MINIMODES][MODE_VEHICLE_MAX][modesVehicleInfo];
new modeWaitZones[MODE_MAX_WAITZONES][modesWaitZoneInfo];
new modeWaitZonesCount;
new modesLocked;
new modesWaiting;
new modesCount;
// new modesTimer;
new modesMaxReached;
new modeStartVotePercent;
new modeStartHalfXP;

new modesStateWaiting[MAX_MINIMODES + 1];
new modesStateProgress[MAX_MINIMODES + 1];

// Optimised loops
#define LoopMinimodesWaiting(%1) for (new mma, %1; modesStateWaiting[mma] != INVALID_MODE_ID; mma++) if ((%1 = modesStateWaiting[mma]) || true)
#define LoopMinimodesProgress(%1) for (new mmb, %1; modesStateProgress[mmb] != INVALID_MODE_ID; mmb++) if ((%1 = modesStateProgress[mmb]) || true)
// ---------------

#include "spec.inc" // Spectate - Handles spectating for minimodes

#include "Minimodes\deathmatchffa.inc"
#include "Minimodes\deathmatchteam.inc"
#include "Minimodes\race.inc"
#include "Minimodes\derby.inc"
#include "Minimodes\ctf.inc"

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

MinimodesSettingsInit() {
	modeStartVotePercent = INI_ReadInt("ModeStartVotePercent");
	modeStartHalfXP = INI_ReadInt("ModeStartHalfXP");
	modesLocked = INI_ReadInt("ModeLockedByDefault");
	RaceSettingsInit();
	DerbySettingsInit();
	CTFInit();
	return 1;
}

MinimodesInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		mpData[i][mpModeID] = INVALID_MODE_ID;
		mpData[i][mpLastJoinModeID] = INVALID_MODE_ID;
		mpData[i][mpVoteMinimode] = INVALID_MODE_ID;
	}
	for (new i; i < MAX_MINIMODES; i++) {
		modes[i][mType] = MODE_TYPE_INVALID;
		modes[i][mCheckpointID] = INVALID_CHECKPOINT_ID;
		
		for (new j; j < MODE_OBJECT_MAX; j++) {
			modeObjects[i][j][oObjectID] = INVALID_OBJECT_ID;
		}
	}
	
	modesStateWaiting[0] = INVALID_MODE_ID;
	modesStateProgress[0] = INVALID_MODE_ID;
	
	// Load all minimodes
	MinimodesWaitZonesInit();
	DeathmatchFFAInit();
	DeathmatchTeamInit();
	RacesInit();
	DerbysInit();
	CTFInit();
	
	printf("[FDM] Total Minimodes Loaded: %i", modesCount);
	
	kEnableTimer(14); // modesTimer = SetTimer("fdm_CheckMinimodes", 925, true); // Will start minimodes
	return 1;
}

MinimodesWaitZonesInit() {
	if (!INI_Exist(FILE_MODE_WAIT_ZONES)) {
		printf("[FDM] WARNING: Wait Zones List Fail: '%s' - File not found.", FILE_MODE_WAIT_ZONES);
		return 0;
	}
	
	if (!INI_Open(FILE_MODE_WAIT_ZONES)) {
		printf("[FDM] WARNING: Wait Zones List Fail: '%s' - File cannot be opened.", FILE_MODE_WAIT_ZONES);
		return 0;
	}
	
	new i2;
	new idx;
	
	for (new i; gCache[i][E_VALUE][0] && i < INI_MAX_LINES; i++) { // Read directly from the cache
		if (gCache[i][E_KEY][0] && !strcmp(gCache[i][E_KEY], "WAITZONE", false)) {
			// Is an object
			idx = 0;
			modeWaitZones[i2][mzX] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeWaitZones[i2][mzY] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeWaitZones[i2][mzZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeWaitZones[i2][mzInteriorID] = strval(split(gCache[i][E_VALUE], idx, ','));
			modeWaitZones[i2][mzInUse] = 0;
			
			i2++;
			modeWaitZonesCount = i2;
		}
		if (i2 >= MODE_OBJECT_MAX) {
			break; // Maximum objects reached
		} else {
			modeWaitZones[i2][mzInUse] = -1;
		}
	}
	
	printf("[FDM] Wait Zones Loaded: %i", modeWaitZonesCount);
	
	return 1;
}

MinimodesUnload() {
	new tmp[modesInfo];
	new tmp2[objectInfo];
	new tmp3[modesWaitZoneInfo];
	
	//SpecExit();
	
	for (new i; i < MAX_MINIMODES; i++) {
		StopMinimode(i);
		RemoveCheckpoint(modes[i][mCheckpointID]);
		modes[i] = tmp;
		for (new b; b < MODE_OBJECT_MAX; b++) {
			if (modeObjects[i][b][oObjectID] != INVALID_OBJECT_ID) {
				DestroyDynamicObject(modeObjects[i][b][oObjectID]);
				modeObjects[i][b] = tmp2;
			}
		}
	}
	
	for (new i; i < MODE_MAX_WAITZONES; i++) {
		modeWaitZones[i] = tmp3;
	}
	
	modesWaiting = 0;
	kDisableTimer(14); // KillTimer(modesTimer);
	
	DeathmatchFFAUnload();
	DeathmatchTeamUnload();
	RacesUnload();
	DerbysUnload();
	CTFUnload();
	
	modesCount = 0;
	modesLocked = 0;
	modesMaxReached = 0;
	modeWaitZonesCount = 0;
	
	modesStateWaiting[0] = INVALID_MODE_ID;
	modesStateProgress[0] = INVALID_MODE_ID;
	
	return 1;
}

/*MinimodeExit() {
	RaceUpdatePlayerInfoUnload();
	dmteamUpdatePlayerInfoUnload();
	dmffaUpdatePlayerInfoUnload();
	return 1;
}*/

/*CreateMinimode(type, name[], mapid, minlevel, cost, Float:lx, Float:ly, Float:lz, notes[] = "") { // Older version
	// Get free minimode ID
	if (!name[0]) return INVALID_MODE_ID;
	new modeid = GetFreeMinimodeID();
	if (modeid == INVALID_MODE_ID) return INVALID_MODE_ID;
	
	modes[modeid][mType] = type;
	strcpy(modes[modeid][mName], name, MAX_MODE_NAME);
	modes[modeid][mMapID] = mapid;
	modes[modeid][mMinLevel] = minlevel;
	modes[modeid][mCost] = cost;
	modes[modeid][mLocation][mX] = lx;
	modes[modeid][mLocation][mY] = ly;
	modes[modeid][mLocation][mZ] = lz;
	strcpy(modes[modeid][mNotes], notes, MAX_MODE_NOTES);
	modesCount++;
	
	printf("[FDM] Minimode Loaded: %s", name);
	return modeid;
}*/

LoadMinimode(type, name[], mapid, notes[] = "", minplayers = 1, maxplayers = MAX_PLAYERS, EnableRandomWaitZone = 0) { // Create and load shared minimode data from an existing open file
	new modeid;
	
	if (!name[0]) {
		printf("[FDM] WARNING: Minimode Load Fail: '%s' - Invalid minimode name.", gFile[E_FILENAME]);
		return INVALID_MODE_ID;
	}
	
	if (type == MODE_TYPE_INVALID) {
		printf("[FDM] WARNING: Minimode Load Fail: '%s' - Invalid minimode type.", gFile[E_FILENAME]);
		return INVALID_MODE_ID;
	}
	
	if (mapid == INVALID_MAP_ID) {
		printf("[FDM] WARNING: Minimode Load Fail: '%s' - Invalid map ID.", gFile[E_FILENAME]);
		return INVALID_MODE_ID;
	}
	
	if ((modeid = GetFreeMinimodeID()) == INVALID_MODE_ID) {
		if (!modesMaxReached) {
			printf("[FDM] WARNING: Maximum minimodes reached (%i minimodes)", MAX_MINIMODES);
			modesMaxReached = 1;
		}
		return INVALID_MODE_ID;
	}
	
	new typestr[16];
	switch (type) {
		case MODE_TYPE_DMFFA: typestr = "Deathmatch FFA";
		case MODE_TYPE_RACE: typestr = "Race";
		case MODE_TYPE_DMTEAM: typestr = "Deathmatch Team";
		case MODE_TYPE_DERBY: typestr = "Derby";
		case MODE_TYPE_CTF: typestr = "CTF";
		default: return INVALID_MODE_ID;
	}
	
	format(modes[modeid][mName], MAX_MODE_NAME, "%s: %s", typestr, name);
	modes[modeid][mType] = type;
	modes[modeid][mMapID] = mapid;
	strcpy(modes[modeid][mNotes], notes, MAX_MODE_NOTES);
	modes[modeid][mMinPlayers] = minplayers;
	modes[modeid][mMaxPlayers] = maxplayers;
	
	if (EnableRandomWaitZone) {
		modes[modeid][mEnableRandomWaitZone] = 1;
	} else {
		modes[modeid][mEnableRandomWaitZone] = 0;
	}
	modes[modeid][mWaitZoneUsed] = -1;
	
	#if MODE_LEVEL_REQUIREMENTS
		modes[modeid][mMinLevel] = INI_ReadInt("MinLevel");
	#endif
	modes[modeid][mCost] = INI_ReadInt("JoinCost");
	INI_ReadString(modes[modeid][mAuthor], "Author", MAX_NAME);
	INI_ReadString(modes[modeid][mDescription], "Description", MAX_STRING);
	
	new idx, tmpstr[MAX_STRING];
	INI_ReadString(tmpstr, "WaitLocation"); // Waiting location
	modes[modeid][mLocation][mX] = floatstr(split(tmpstr, idx, ','));
	modes[modeid][mLocation][mY] = floatstr(split(tmpstr, idx, ','));
	modes[modeid][mLocation][mZ] = floatstr(split(tmpstr, idx, ','));
	modes[modeid][mLocation][mwInteriorID] = strval(split(tmpstr, idx, ','));
	
	modes[modeid][mInteriorID] = INI_ReadInt("Interior");
	if (!modes[modeid][mInteriorID]) {
		modes[modeid][mInteriorID] = modes[modeid][mLocation][mwInteriorID];
	}
	
	#if defined _YSI_included
		if (INI_ReadString(tmpstr, "Gravity")) {
			modes[modeid][mGravity] = floatstr(tmpstr);
			if (modes[modeid][mGravity] < -1.0 ||
				modes[modeid][mGravity] > 1.0) {
				
				modes[modeid][mGravity] = INVALID_GRAVITY;
			}
		} else {
			modes[modeid][mGravity] = INVALID_GRAVITY;
		}
	#endif
	
	if (INI_ReadString(tmpstr, "Time")) {
		idx = 0;
		modes[modeid][mTimeChanged] = 1;
		modes[modeid][mTimeHour] = strval(split(tmpstr, idx, ':'));
		modes[modeid][mTimeMinute] = strval(split(tmpstr, idx, ':'));
	}
	if (INI_ReadString(tmpstr, "Weather")) {
		modes[modeid][mWeatherChanged] = 1;
		modes[modeid][mWeather] = strval(tmpstr);
	}
	
	new Float:defaultViewDistance = OBJECT_DEFAULT_VIEW_DISTANCE;
	new viewDistanceOverride;
	if (INI_ReadString(tmpstr, "DefaultViewDistance")) {
		defaultViewDistance = floatstr(tmpstr);
		viewDistanceOverride = 1;
	}
	
	new i2;
	for (new i; gCache[i][E_VALUE][0] && i < INI_MAX_LINES; i++) { // Read directly from the cache
		if (gCache[i][E_KEY][0] && !strcmp(gCache[i][E_KEY], "OBJECT", false)) {
			// Is an object
			idx = 0;
			modeObjects[modeid][i2][oModelID] = strval(split(gCache[i][E_VALUE], idx, ','));
			if (!modeObjects[modeid][i2][oModelID]) continue;
			
			modeObjects[modeid][i2][oX] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oY] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oRotX] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oRotY] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oRotZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeObjects[modeid][i2][oViewDistance] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			if (modeObjects[modeid][i2][oViewDistance] == 0.0) modeObjects[modeid][i2][oViewDistance] = defaultViewDistance;
			
			i2++;
		}
		if (i2 >= MODE_OBJECT_MAX) break; // Maximum objects reached
	}
	
	if (!viewDistanceOverride && i2 && i2 < 400) {
		for (new i; i < i2; i++) {
			modeObjects[modeid][i][oViewDistance] = 100000.0;
		}
	}
	
	i2 = 0;
	for (new i; gCache[i][E_VALUE][0] && i < INI_MAX_LINES; i++) { // Read directly from the cache
		if (gCache[i][E_KEY][0] && !strcmp(gCache[i][E_KEY], "VEHICLE", false)) {
			// Is a vehicle
			idx = 0;
			modeVehicles[modeid][i2][mvModel] = strval(split(gCache[i][E_VALUE], idx, ','));
			if (!modeVehicles[modeid][i2][mvModel]) continue;
			
			modeVehicles[modeid][i2][mvX] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvY] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvRotZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvColour1] = strval(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvColour2] = strval(split(gCache[i][E_VALUE], idx, ','));
			modeVehicles[modeid][i2][mvRespawnDelay] = strval(split(gCache[i][E_VALUE], idx, ','));
			if (!modeVehicles[modeid][i2][mvRespawnDelay]) modeVehicles[modeid][i2][mvRespawnDelay] = MODE_VEHICLE_DEFAULT_RESPAWN;
			
			i2++;
		}
		if (i2 >= MODE_VEHICLE_MAX) break; // Maximum vehicles reached
	}
	
	modesCount++;
	
	//printf("[FDM] Minimode Loaded: %s", modes[modeid][mName]);
	
	return modeid;
}

GetFreeMinimodeID() {
	for (new i; i < MAX_MINIMODES; i++) {
		if (modes[i][mType] == MODE_TYPE_INVALID) return i;
	}
	return INVALID_MODE_ID;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

UpdateModeArrays() {
	new indexWaiting, indexProgress;
	
	modesWaiting = 0;
	
	for (new modeid; modeid < MAX_MINIMODES; modeid++) {
		switch (modes[modeid][mState]) {
			case MODE_STATE_WAITING: {
				modesStateWaiting[indexWaiting] = modeid;
				indexWaiting++;
				modesWaiting++;
			}
			case MODE_STATE_PROGRESS: {
				modesStateProgress[indexProgress] = modeid;
				indexProgress++;
			}
		}
	}
	
	modesStateWaiting[indexWaiting] = INVALID_MODE_ID;
	modesStateProgress[indexProgress] = INVALID_MODE_ID;
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------------------------------

minimode_OnPlayerDisconnect(playerid) {
	new modeid = mpData[playerid][mpVoteMinimode];
	RemovePlayerFromMode(playerid);
	
	mpData[playerid][mpModeID] = INVALID_MODE_ID;
	mpData[playerid][mpLastJoinModeID] = INVALID_MODE_ID;
	mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
	mpData[playerid][mpStartDelay] = 0;
	mpData[playerid][mpLoadWeapons] = 0;
	
	mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
	
	if (mpData[playerid][mpTextExists]) {
		TextDrawDestroy(mpData[playerid][mpText]);
		mpData[playerid][mpTextExists] = 0;
	}
	
	CheckMinimodeVoting(modeid);
	return 1;
}

minimode_OnPlayerSpawn(playerid) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) {
		if (mpData[playerid][mpLoadWeapons]) {
			mpData[playerid][mpLoadWeapons] = 0;
			
			// Very messy, perhaps do something about it later
			EnableGodmode(playerid);
			LoadPlayerInfo(playerid);
			DisableNextSpawnWeapons(playerid);
		}
		return 0;
	}
	if (modes[mpData[playerid][mpModeID]][mState] != MODE_STATE_PROGRESS) return 0;
	
	SetPlayerInterior(playerid, modes[mpData[playerid][mpModeID]][mInteriorID]);
	
	if (modes[mpData[playerid][mpModeID]][mFlags] & MODE_FLAG_DRUNK) SetPlayerDrunkLevel(playerid, MODE_DRUNK_LEVEL);
	
	switch (modes[mpData[playerid][mpModeID]][mType]) {
		//case MODE_TYPE_INVALID: 
		case MODE_TYPE_DMFFA: return dmffa_OnPlayerSpawn(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		//case MODE_TYPE_RACE: 
		case MODE_TYPE_DMTEAM: return dmteam_OnPlayerSpawn(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		//case MODE_TYPE_DERBY: 
		case MODE_TYPE_CTF: return ctf_OnPlayerSpawn(modes[mpData[playerid][mpModeID]][mMapID], playerid);
	}
	
	return 0;
}

minimode_OnPlayerDeath(playerid, killerid, reason) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) {
		if (mpData[playerid][mpTextExists]) {
			TextDrawDestroy(mpData[playerid][mpText]);
			mpData[playerid][mpTextExists] = 0;
		}
		
		new modeid = mpData[playerid][mpVoteMinimode];
		mpData[playerid][mpLastJoinModeID] = INVALID_MODE_ID;
		mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
		
		CheckMinimodeVoting(modeid);
		return 0;
	}
	if (modes[mpData[playerid][mpModeID]][mState] != MODE_STATE_PROGRESS) return 0;
	
	switch (modes[mpData[playerid][mpModeID]][mType]) {
		//case MODE_TYPE_INVALID: 
		case MODE_TYPE_DMFFA: return dmffa_OnPlayerDeath(modes[mpData[playerid][mpModeID]][mMapID], playerid, killerid, reason);
		case MODE_TYPE_RACE: return race_OnPlayerDeath(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		case MODE_TYPE_DMTEAM: return dmteam_OnPlayerDeath(modes[mpData[playerid][mpModeID]][mMapID], playerid, killerid, reason);
		case MODE_TYPE_DERBY: return derby_OnPlayerDeath(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		case MODE_TYPE_CTF: return ctf_OnPlayerDeath(modes[mpData[playerid][mpModeID]][mMapID], playerid, killerid, reason);
	}
		
	return 0;
}

minimode_OnVehicleDeath(vehicleid) {
	if (race_OnVehicleDeath(vehicleid)) return 1;
	if (derby_OnVehicleDeath(vehicleid)) return 1;
	return 0;
}

minimode_OnVehicleSpawn(vehicleid) {
	if (race_OnVehicleSpawn(vehicleid)) return 1;
	if (derby_OnVehicleSpawn(vehicleid)) return 1;
	return 0;
}

minimode_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) return 0;
	if (modes[mpData[playerid][mpModeID]][mState] != MODE_STATE_PROGRESS) return 0;
	
	switch (modes[mpData[playerid][mpModeID]][mType]) {
		//case MODE_TYPE_INVALID: 
		case MODE_TYPE_DMFFA: return dmffa_OnPlayerStateChange(playerid, newstate, oldstate);
		case MODE_TYPE_RACE: return race_OnPlayerStateChange(modes[mpData[playerid][mpModeID]][mMapID], playerid, newstate, oldstate);
		case MODE_TYPE_DMTEAM: return dmteam_OnPlayerStateChange(playerid, newstate, oldstate);
		case MODE_TYPE_DERBY: return derby_OnPlayerStateChange(modes[mpData[playerid][mpModeID]][mMapID], playerid, newstate, oldstate);
		case MODE_TYPE_CTF: return ctf_OnPlayerStateChange(playerid, newstate, oldstate);
	}
	
	return 0;
}

minimode_OnPlayerEnterRaceCP(playerid) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) return 0;
	if (modes[mpData[playerid][mpModeID]][mState] != MODE_STATE_PROGRESS) return 0;
	
	switch (modes[mpData[playerid][mpModeID]][mType]) {
		//case MODE_TYPE_INVALID: 
		//case MODE_TYPE_DMFFA: 
		case MODE_TYPE_RACE: return race_OnPlayerEnterRaceCP(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		//case MODE_TYPE_DMTEAM: 
		//case MODE_TYPE_DERBY: 
	}
	
	return 0;
}

minimode_OnPlayerVolumeChange(playerid) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) return 0;
	if (modes[mpData[playerid][mpModeID]][mState] != MODE_STATE_PROGRESS) return 0;
	
	switch (modes[mpData[playerid][mpModeID]][mType]) {
		//case MODE_TYPE_INVALID: 
		case MODE_TYPE_DMFFA: return dmffa_OnPlayerVolumeChange(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		//case MODE_TYPE_RACE: 
		case MODE_TYPE_DMTEAM: return dmteam_OnPlayerVolumeChange(modes[mpData[playerid][mpModeID]][mMapID], playerid);
		//case MODE_TYPE_DERBY: 
		case MODE_TYPE_CTF: return ctf_OnPlayerVolumeChange(modes[mpData[playerid][mpModeID]][mMapID], playerid);
	}
	
	return 0;
}

mode_OnPlayerEnterCheckpoint(playerid, checkpointid) {
	new modeid = INVALID_MODE_ID;
	LoopMinimodesWaiting(i) {
		if (modes[i][mCheckpointID] == checkpointid) {
			modeid = i;
			break;
		}
	}
	LoopMinimodesProgress(i) {
		if (modes[i][mCheckpointID] == checkpointid) {
			modeid = i;
			break;
		}
	}
	
	if (modeid == INVALID_MODE_ID) return 1;
	if (IsPlayerInMinimode(playerid)) return 1;
	if (modes[modeid][mState] == MODE_STATE_WAITING) {
		
		// Send minimode information
		new message[MAX_INPUT];
		SendMinimodeInfoToPlayer(playerid, modeid);
		if (!pData[playerid][pIsLoggedIn]) {
			SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must be registered to join minimodes. (/register [password])");
			return 0;
		}
		#if MODE_LEVEL_REQUIREMENTS
			if (pData[playerid][pLevel] < modes[modeid][mMinLevel]) {
				format(message, sizeof(message), "MINIMODES: You need to be at least level %i to join %s.",
					modes[modeid][mMinLevel], modes[modeid][mName]);
				SendClientMessage(playerid, COLOUR_ERROR, message);
				return 0;
			}
		#endif
		if (GetPlayerMoney(playerid) < modes[modeid][mCost]) {
			format(message, sizeof(message), "MINIMODES: You need to have at least $%i to join %s.",
				modes[modeid][mCost], modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 0;
		}
		
		EnableGodmode(playerid);
		
		if (mpData[playerid][mpLastJoinModeID] != modeid) {
			
			// Show text draw
			if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID) {
				TextDrawHideForPlayer(playerid, modes[mpData[playerid][mpLastJoinModeID]][mModeName]);
			}
			TextDrawShowForPlayer(playerid, modes[modeid][mModeName]);
			
			new votemodeid = mpData[playerid][mpVoteMinimode];
			mpData[playerid][mpLastJoinModeID] = modeid;
			mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
			
			format(message, sizeof(message), "%s joined %s",
				pData[playerid][pLoginName], modes[modeid][mName]);
			SideChatAddLine(message, COLOUR_MODE_JOIN);
			
			new tmpstr[MAX_INPUT], playersVoted, playersInMode;
			CheckMinimodeVoting(modeid, playersVoted, playersInMode, 0);
			if (playersVoted > 0 && playersInMode > 0) {
				if (MODE_START_TIME - (modes[modeid][mStartTime] - GetTickCount()) < MIN_QUICKSTART_INTERVAL) {
					format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i ~r~(%i/%i)", (modes[modeid][mStartTime] - GetTickCount()) / 1000, playersVoted, playersInMode);
				} else {
					format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i ~y~(%i/%i)", (modes[modeid][mStartTime] - GetTickCount()) / 1000, playersVoted, playersInMode);
				}
			} else {
				format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i", (modes[modeid][mStartTime] - GetTickCount()) / 1000);
			}
			
			if (!mpData[playerid][mpTextExists]) {
				mpData[playerid][mpText] = TextDrawCreate(MODE_TEXT_X, MODE_TEXT_Y, tmpstr);
				TextDrawShowForPlayer(playerid, mpData[playerid][mpText]);
				mpData[playerid][mpTextExists] = 1;
			} else {
				TextDrawSetString(mpData[playerid][mpText], tmpstr);
			}
			
			// Support the spectators for text draws
			LoopPlayers(i) {
				if (GetPlayerSpectatingPlayer(i) == playerid) {
					if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID) {
						TextDrawHideForPlayer(i, modes[mpData[playerid][mpLastJoinModeID]][mModeName]);
					}
					TextDrawShowForPlayer(i, modes[modeid][mModeName]);
					TextDrawShowForPlayer(i, mpData[playerid][mpText]);
				}
			}
			
			#if defined ENABLE_CALLBACKS
				CallRemoteFunction("fdmOnPlayerMinimodeWaiting", "ii", playerid, modeid);
			#endif
			
			CheckMinimodeVoting(votemodeid);
		}
		return 0;
		
	} else if (modes[modeid][mState] == MODE_STATE_PROGRESS) {
		
		/*new message[MAX_INPUT];
		if (IsMinimodeJoinable(modeid)) {
			format(message, sizeof(message), "%s is in progress. Use /spec %i to spectate, or /join %i to join.", modes[modeid][mName], modeid, modeid);
		} else {
			format(message, sizeof(message), "%s is in progress. Use /spec %i to spectate.", modes[modeid][mName], modeid);
		}
		SendClientMessage(playerid, COLOUR_MODE_DESCRIPTION, message);*/
		SendMinimodeInfoToPlayer(playerid, modeid);
		
		// Show text draw
		//TextDrawShowForPlayer(playerid, modes[modeid][mModeName]);
		
		return 0;
	}
	return 1;
}

mode_OnPlayerLeaveCheckpoint(playerid, checkpointid) {
	new modeid = INVALID_MODE_ID;
	LoopMinimodesWaiting(i) {
		if (modes[i][mCheckpointID] == checkpointid) {
			modeid = i;
			break;
		}
	}
	
	if (modeid == INVALID_MODE_ID) return 1;
	if (IsPlayerInMinimode(playerid)) return 1;
	
	TextDrawHideForPlayer(playerid, modes[modeid][mModeName]);
	
	if (modes[modeid][mState] == MODE_STATE_WAITING) {
		DisableGodmode(playerid);
		SetLastSpawnTick(playerid);
		
		new message[MAX_INPUT];
		format(message, sizeof(message), "MINIMODES: Stay inside the circle if you want to join %s.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		
		if (mpData[playerid][mpTextExists]) {
			TextDrawDestroy(mpData[playerid][mpText]);
			mpData[playerid][mpTextExists] = 0;
		}
		
		new votemodeid = mpData[playerid][mpVoteMinimode];
		mpData[playerid][mpLastJoinModeID] = INVALID_MODE_ID;
		mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
		
		CheckMinimodeVoting(votemodeid);
	}
	
	return 1;
}

forward fdm_CheckMinimodes(); // Execute minimodes
public fdm_CheckMinimodes() {
	if (!modesLocked && modesWaiting < MODE_MAX_WAITING) {
		// Chance minimode will be started on this timer count depending on amount of minimodes loaded
		if (!random((modesWaiting + 1) * 15)) {
			/*
			// Find average player level
			#if MODE_LEVEL_REQUIREMENTS
				new playerCount;
				new levelsTotal;
				new levelsAverage;
				
				LoopPlayers(p) {
					if (pData[p][pIsLoggedIn]) {
						levelsTotal += pData[p][pLevel];
						playerCount++;
					}
				}
				if (playerCount) {
					levelsAverage = levelsTotal / playerCount;
				}
			#endif
			
			// Load a random minimode
			new potentialmodes[MAX_MINIMODES];
			new i2;
			for (new i1; i1 < MAX_MINIMODES; i1++) {
				#if MODE_LEVEL_REQUIREMENTS
				if (modes[i1][mType] != MODE_TYPE_INVALID &&
					modes[i1][mState] == MODE_STATE_UNLOADED &&
					modes[i1][mMinLevel] <= levelsAverage) {
				#else
				if (modes[i1][mType] != MODE_TYPE_INVALID &&
					modes[i1][mState] == MODE_STATE_UNLOADED) {
				#endif
				
					potentialmodes[i2] = i1;
					i2++;
				}
			}
			
			if (i2) {
				new rand = random(i2);
				StartMinimode(potentialmodes[rand]);
			}
			*/
			
			// Optimised
			new randomMinimode;
			new randomCount;
			
			// Find a random unloaded minimode
			randomMinimode = random(modesCount);
			while ((modes[randomMinimode][mType] == MODE_TYPE_INVALID || modes[randomMinimode][mState] != MODE_STATE_UNLOADED) && randomCount < 6) {
				randomMinimode = random(modesCount);
				randomCount++;
			}
			
			if (modes[randomMinimode][mType] != MODE_TYPE_INVALID && modes[randomMinimode][mState] == MODE_STATE_UNLOADED) {
				new minimodeFlags = ((!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_DRUNK) && !random(MODE_CHANCE_DRUNK)) ? MODE_FLAG_DRUNK : 0);
				
				if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_XP_L1) && !random(MODE_CHANCE_XP_L1)) minimodeFlags += MODE_FLAG_XP_L1;
					else if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_XP_L2) && !random(MODE_CHANCE_XP_L2)) minimodeFlags += MODE_FLAG_XP_L2;
					else if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_XP_L3) && !random(MODE_CHANCE_XP_L3)) minimodeFlags += MODE_FLAG_XP_L3;
				
				if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_CASH_L1) && !random(MODE_CHANCE_CASH_L1)) minimodeFlags += MODE_FLAG_CASH_L1;
					else if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_CASH_L2) && !random(MODE_CHANCE_CASH_L2)) minimodeFlags += MODE_FLAG_CASH_L2;
					else if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_CASH_L3) && !random(MODE_CHANCE_CASH_L3)) minimodeFlags += MODE_FLAG_CASH_L3;
				
				switch (modes[randomMinimode][mType]) {
					case MODE_TYPE_RACE, MODE_TYPE_DERBY: {
						if (!(modes[randomMinimode][mBlockedFlags] & MODE_FLAG_DAMAGED) && !random(MODE_CHANCE_DAMAGED)) minimodeFlags = minimodeFlags += MODE_FLAG_DAMAGED;
					}
				}
				
				StartMinimode(randomMinimode, minimodeFlags);
			}
		}
	}
	
	
	LoopPlayers(p) {
		// Update the timer for all player's start timer
		if (mpData[p][mpLastJoinModeID] != INVALID_MODE_ID && modes[mpData[p][mpLastJoinModeID]][mState] == MODE_STATE_WAITING) {
			new tmpstr[MAX_INPUT], playersVoted, playersInMode;
			CheckMinimodeVoting(mpData[p][mpLastJoinModeID], playersVoted, playersInMode, 0);
			if (playersVoted > 0 && playersInMode > 0) {
				if (MODE_START_TIME - (modes[mpData[p][mpLastJoinModeID]][mStartTime] - GetTickCount()) < MIN_QUICKSTART_INTERVAL) {
					format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i ~r~(%i/%i)", (modes[mpData[p][mpLastJoinModeID]][mStartTime] - GetTickCount()) / 1000, playersVoted, playersInMode);
				} else {
					format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i ~y~(%i/%i)", (modes[mpData[p][mpLastJoinModeID]][mStartTime] - GetTickCount()) / 1000, playersVoted, playersInMode);
				}
			} else {
				format(tmpstr, sizeof(tmpstr), "~g~~h~~h~Start Time: ~w~%i", (modes[mpData[p][mpLastJoinModeID]][mStartTime] - GetTickCount()) / 1000);
			}
			
			if (!mpData[p][mpTextExists]) {
				mpData[p][mpText] = TextDrawCreate(MODE_TEXT_X, MODE_TEXT_Y, tmpstr);
				TextDrawShowForPlayer(p, mpData[p][mpText]);
				mpData[p][mpTextExists] = 1;
			} else {
				TextDrawSetString(mpData[p][mpText], tmpstr);
			}
		} else {
			if (mpData[p][mpTextExists]) {
				TextDrawDestroy(mpData[p][mpText]);
				mpData[p][mpTextExists] = 0;
			}
		}
		
		// Check for pausers
		if (mpData[p][mpModeID] != INVALID_MODE_ID && modes[mpData[p][mpModeID]][mState] == MODE_STATE_PROGRESS) {
			new modeid = mpData[p][mpModeID];
			if (IsPlayerPaused(p, 10000) && IsPlayerSpawned(p) && RemovePlayerFromMode(p, "left", "(AutoKick: Paused)")) {
				new tmpstr[MAX_INPUT];
				format(tmpstr, sizeof(tmpstr), "You were auto-kicked from %s for pausing.", modes[modeid][mName]);
				SendClientMessage(p, COLOUR_ATTENTION, tmpstr);
			} else {
				// Check drunk levels
				if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
					SetPlayerDrunkLevel(p, MODE_DRUNK_LEVEL);
				}
			}
		}
		
		// Check for Jetpack
		if (GetPlayerSpecialAction(p) == SPECIAL_ACTION_USEJETPACK) {
			RemovePlayerFromMode(p, "left", "(AutoKick: Jetpack)");
		}
	}
	
	// Start any modes in which the timer has expired or voted
	LoopMinimodesWaiting(i) {
		if (((modes[i][mStartTime] - GetTickCount()) / 1000) > 0) {
			CheckMinimodeVoting(i);
			continue;
		}
		if (!StartMinimode(i)) StopMinimode(i);
	}
}

//-----------------------------------------------------------------------------------------------------
// MINIMODE HANDLING
//-----------------------------------------------------------------------------------------------------

StartMinimode(modeid, flags = 0) { // Start a minimode
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return 0;
	
	switch (modes[modeid][mState]) {
		
		case MODE_STATE_UNLOADED: { // Bring to waiting state
			if (modes[modeid][mEnableRandomWaitZone]) {
				// Randomize wait zone
				new avaliablespawns[MODE_MAX_WAITZONES];
				new avaliablespawnscount;
				new waitzoneid = -1;
				
				for (new i; i < modeWaitZonesCount; i++) {
					if (modeWaitZones[i][mzInUse]) continue;
					avaliablespawns[avaliablespawnscount] = i;
					avaliablespawnscount++;
				}
				
				if (avaliablespawnscount) {
					waitzoneid = avaliablespawns[random(avaliablespawnscount)];
				}
				
				if (waitzoneid == -1) {
					return 0;
				} else {
					modeWaitZones[waitzoneid][mzInUse] = 1;
					modes[modeid][mWaitZoneUsed] = waitzoneid;
					modes[modeid][mLocation][mX] = modeWaitZones[waitzoneid][mzX];
					modes[modeid][mLocation][mY] = modeWaitZones[waitzoneid][mzY];
					modes[modeid][mLocation][mZ] = modeWaitZones[waitzoneid][mzZ];
				}
			}
			
			modes[modeid][mCheckpointID] = AddCheckpoint(
				modes[modeid][mLocation][mX], modes[modeid][mLocation][mY], modes[modeid][mLocation][mZ], 10.0, 0);
			
			if (modes[modeid][mCheckpointID] == INVALID_CHECKPOINT_ID) return 0;
			
			modes[modeid][mStartTime] = GetTickCount() + MODE_START_TIME;
			modes[modeid][mState] = MODE_STATE_WAITING;
			UpdateModeArrays();
			
			if (flags != -1) {
				modes[modeid][mFlags] = flags;
			}
			
			// Send minimode information
			SendMinimodeInfoToAll(modeid);
			
			// Create the mode text draw
			modes[modeid][mModeName] = TextDrawCreate(10.0, 438.0, modes[modeid][mName]);
			TextDrawLetterSize(modes[modeid][mModeName], 0.265, 0.85);
			TextDrawColor(modes[modeid][mModeName], COLOUR_MODE_ALLOWED);
			TextDrawSetShadow(modes[modeid][mModeName], 1);
			return 1;
		}
		
		case MODE_STATE_WAITING: { // Bring to progress state
			// Put all players who are within the circle and with the right requirements into the minimode, then start it.
			// Find players in circle
			new players[MAX_PLAYERS];
			new playercount;
			
			LoopPlayers(i) {
				if (playercount > modes[modeid][mMaxPlayers]) break;
				if (IsPlayerNPC(i)) continue;
				if (!IsPlayerSpawned(i)) continue;
				if (mpData[i][mpModeID] != INVALID_MODE_ID) continue;
				if (GetPlayerVirtualWorld(i)) continue;
				if (IsPlayerPaused(i, 10000)) continue;
				if (!IsPlayerInCheckpointID(i, modes[modeid][mCheckpointID])) continue;
				
				if (!pData[i][pIsLoggedIn]) {
					SendClientMessage(i, COLOUR_ERROR, "MINIMODES: You must be registered to join minimodes. (/register [password])");
					continue;
				}
				#if MODE_LEVEL_REQUIREMENTS
					if (pData[i][pLevel] < modes[modeid][mMinLevel]) {
						new message[MAX_INPUT];
						format(message, sizeof(message), "MINIMODES: You must be at least level %i to join %s.", modes[modeid][mMinLevel], modes[modeid][mName]);
						SendClientMessage(i, COLOUR_ERROR, message);
						continue;
					}
				#endif
				if (modes[modeid][mCost] >= 0 && GetPlayerMoney(i) < modes[modeid][mCost]) {
					new message[MAX_INPUT];
					format(message, sizeof(message), "MINIMODES: You have $%i to join %s.", modes[modeid][mCost], modes[modeid][mName]);
					SendClientMessage(i, COLOUR_ERROR, message);
					continue;
				}
				
				players[i] = 1;
				playercount++;
			}
			
			if (playercount < modes[modeid][mMinPlayers]) {
				// Not enough players
				//StopMinimode(modeid);
				return 0;
			} else {
				// There are players - start minimode
				LoopPlayers(i) {
					if (mpData[i][mpLastJoinModeID] == modeid || players[i]) {
						if (mpData[i][mpTextExists]) {
							TextDrawDestroy(mpData[i][mpText]);
							mpData[i][mpTextExists] = 0;
						}
						
						SavePlayerInfo(i);
						DisableGodmode(i);
						TextDrawHideForPlayer(i, modes[modeid][mModeName]);
						
						/*new Float:px, Float:py, Float:pz, Float:distance;
						GetPlayerPos(i, px, py, pz);
						
						distance = FindDistance(px, py, pz, cpData[modes[modeid][mCheckpointID]][cX], cpData[modes[modeid][mCheckpointID]][cY], cpData[modes[modeid][mCheckpointID]][cZ]);
						
						if (distance < cpData[modes[modeid][mCheckpointID]][cSize] * 2.5) {
							SetLastSpawnTick(i);
						}*/
						
						if (IsPlayerInCheckpointID(i, modes[modeid][mCheckpointID])) {
							SetLastSpawnTick(i);
						}
						
						new votemodeid = mpData[i][mpVoteMinimode];
						mpData[i][mpLastJoinModeID] = INVALID_MODE_ID;
						mpData[i][mpVoteMinimode] = INVALID_MODE_ID;
						
						if (modeid != votemodeid && mpData[i][mpVoteMinimode] != INVALID_MODE_ID) {
							mpData[i][mpVoteMinimode] = INVALID_MODE_ID;
							CheckMinimodeVoting(votemodeid);
						}
					}
				}
				
				new rtn;
				modes[modeid][mVirtualWorld] = GetFreeVirtualWorldID();
				
				switch (modes[modeid][mType]) {
					case MODE_TYPE_DMFFA: rtn = dmffa_OnStart(modes[modeid][mMapID], players, playercount);
					case MODE_TYPE_RACE: rtn = race_OnStart(modes[modeid][mMapID], players, playercount);
					case MODE_TYPE_DMTEAM: rtn = dmteam_OnStart(modes[modeid][mMapID], players, playercount);
					case MODE_TYPE_DERBY: rtn = derby_OnStart(modes[modeid][mMapID], players, playercount);
					case MODE_TYPE_CTF: rtn = ctf_OnStart(modes[modeid][mMapID], players, playercount);
					default: rtn = 0;
				}
				
				if (rtn) {
					// Minimode started
					ToggleVirtualWorldUsed(modes[modeid][mVirtualWorld], 1);
					modes[modeid][mTimeBaseTick] = GetTickCount();
					
					// Mark it started
					modes[modeid][mState] = MODE_STATE_PROGRESS;
					//RemoveCheckpoint(modes[modeid][mCheckpointID]);
					//modes[modeid][mCheckpointID] = INVALID_CHECKPOINT_ID;
					UpdateModeArrays();
					
					// Mark players as being in this minimode
					new count;
					new logline[MAX_INPUT];
					new tmpstr[MAX_REAL + 1];
					format(logline, sizeof(logline), "[FDM] Minimode: %s started - Players(", modes[modeid][mName]);
					
					for (new o; o < MODE_OBJECT_MAX; o++) {
						if (modeObjects[modeid][o][oModelID] && modeObjects[modeid][o][oObjectID] == INVALID_OBJECT_ID) {
							modeObjects[modeid][o][oObjectID] = CreateDynamicObject(
								modeObjects[modeid][o][oModelID],
								modeObjects[modeid][o][oX],
								modeObjects[modeid][o][oY],
								modeObjects[modeid][o][oZ],
								modeObjects[modeid][o][oRotX],
								modeObjects[modeid][o][oRotY],
								modeObjects[modeid][o][oRotZ],
								modes[modeid][mVirtualWorld], -1, -1,
								modeObjects[modeid][o][oViewDistance]);
						} else {
							break;
						}
					}
					
					for (new o; o < MODE_VEHICLE_MAX; o++) {
						if (modeVehicles[modeid][o][mvModel] && !modeVehicles[modeid][o][mvVehicleID]) {
							modeVehicles[modeid][o][mvVehicleID] = CreateVehicle(
								modeVehicles[modeid][o][mvModel],
								modeVehicles[modeid][o][mvX],
								modeVehicles[modeid][o][mvY],
								modeVehicles[modeid][o][mvZ],
								modeVehicles[modeid][o][mvRotZ],
								modeVehicles[modeid][o][mvColour1],
								modeVehicles[modeid][o][mvColour2],
								modeVehicles[modeid][o][mvRespawnDelay]);
							
							if (modeVehicles[modeid][o][mvVehicleID]) {
								SetVehicleVirtualWorld(modeVehicles[modeid][o][mvVehicleID], modes[modeid][mVirtualWorld]);
								LinkVehicleToInterior(modeVehicles[modeid][o][mvVehicleID], modes[modeid][mInteriorID]);
							} else {
								break;
							}
						} else {
							break;
						}
					}
					
					LoopPlayers(i) {
						if (players[i]) {
							new Float:x,
								Float:y,
								Float:z,
								vehicleid = GetPlayerVehicleID(i);
							
							if (vehicleid) {
								GetVehiclePos(vehicleid, x, y ,z);
							} else {
								GetPlayerPos(i, x, y ,z);
							}
							
							#if defined _YSI_included
								if (modes[modeid][mGravity] != INVALID_GRAVITY) {
									SetPlayerGravity(i, modes[modeid][mGravity]);
								} else {
									SetPlayerGravity(i, DEFAULT_GRAVITY);
								}
							#endif
							
							if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
								SetPlayerDrunkLevel(i, MODE_DRUNK_LEVEL);
							}
							
							SetPlayerInterior(i, modes[modeid][mInteriorID]);
							//StreamObjects(i, x, y, z, modes[modeid][mVirtualWorld], modes[modeid][mInteriorID]);
							
							format(tmpstr, sizeof(tmpstr), "%i,", i);
							strcat(logline, tmpstr);
							
							mpData[i][mpModeID] = modeid;
							GivePlayerMoney(i, -modes[modeid][mCost]); // Charge the player money
							
							// Description
							if (modes[modeid][mDescription][0]) {
								new msg[MAX_INPUT];
								format(msg, sizeof(msg), "Description: %s", modes[modeid][mDescription]);
								SendWrappedMessageToPlayer(i, COLOUR_MODE_DESCRIPTION, msg, 128);
							}
							
							// Time / weather
							if (modes[modeid][mTimeChanged])
								SetPlayerTime(i, modes[modeid][mTimeHour], modes[modeid][mTimeMinute]);
							if (modes[modeid][mWeatherChanged])
								SetPlayerWeather(i, modes[modeid][mWeather]);
							
							// Spectators
							LoopPlayersEx(j, bb) {
								if (GetPlayerSpectatingPlayer(j) != i) continue;
								mode_OnPlayerSpectate(j, i);
							}
							
							// Show text draw
							TextDrawShowForPlayer(i, modes[modeid][mModeName]);
							
							count++;
						}
					}
					
					if (!count) { // No players, just stop it
						StopMinimode(modeid);
						return 0;
					}
					
					logline[strlen(logline) - 1] = ')';
					printf("%s [Flags: %i]", logline, modes[modeid][mFlags]);
					
					modes[modeid][mPlayerCount] = count;
					
					// Send started information
					new message[MAX_INPUT];
					format(message, sizeof(message), "%s has started.", modes[modeid][mName]);
					SendClientMessageToAll(COLOUR_SUCCESSFUL, message);
					SideChatAddLine(message, COLOUR_MODE_START);
					
					OnMinimodeStart(modeid, players);
					
					return 1;
				} else {
					// Minimode could not start
					StopMinimode(modeid);
					modes[modeid][mVirtualWorld] = 0;
					return 0;
				}
			}
		}
		
		case MODE_STATE_PROGRESS: return 0; // Minimode already loaded
		
		default: return 0;
	}
	
	return 0;
}

StopMinimode(modeid) { // Stop a minimode
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return 0;
	
	switch (modes[modeid][mState]) {
		
		case MODE_STATE_UNLOADED: return 0; // Minimode already unloaded
		
		case MODE_STATE_WAITING: { // Bring to unloaded state
			modes[modeid][mState] = MODE_STATE_UNLOADED;
			modes[modeid][mFlags] = 0;
			UpdateModeArrays();
			
			// Destroy text draw
			TextDrawDestroy(modes[modeid][mModeName]);
			
			LoopPlayers(i) {
				if (mpData[i][mpLastJoinModeID] == modeid) {
					if (mpData[i][mpTextExists]) {
						TextDrawDestroy(mpData[i][mpText]);
						mpData[i][mpTextExists] = 0;
					}
					
					DisableGodmode(i);
					
					/*new Float:px, Float:py, Float:pz, Float:distance;
					GetPlayerPos(i, px, py, pz);
					
					distance = FindDistance(px, py, pz, cpData[modes[modeid][mCheckpointID]][cX], cpData[modes[modeid][mCheckpointID]][cY], cpData[modes[modeid][mCheckpointID]][cZ]);
					
					if (distance < cpData[modes[modeid][mCheckpointID]][cSize] * 2.5) {
						SetLastSpawnTick(i);
					}*/
					if (IsPlayerInCheckpointID(i, modes[modeid][mCheckpointID])) {
						SetLastSpawnTick(i);
					}
					
					mpData[i][mpLastJoinModeID] = INVALID_MODE_ID;
					mpData[i][mpVoteMinimode] = INVALID_MODE_ID;
				}
			}
			
			RemoveCheckpoint(modes[modeid][mCheckpointID]);
			modes[modeid][mCheckpointID] = INVALID_CHECKPOINT_ID;
			
			if (modes[modeid][mWaitZoneUsed] >= 0) {
				modeWaitZones[modes[modeid][mWaitZoneUsed]][mzInUse] = 0;
				modes[modeid][mWaitZoneUsed] = -1;
			}
			
			// Send stopped information
			new message[MAX_INPUT];
			format(message, sizeof(message), "%s could not be started.", modes[modeid][mName]);
			SendClientMessageToAll(COLOUR_ERROR, message);
			//SideChatAddLine(message, COLOUR_MODE_STOP);
			return 1;
		}
		
		case MODE_STATE_PROGRESS: { // Bring to unloaded state
			new rtn;
			switch (modes[modeid][mType]) {
				case MODE_TYPE_DMFFA: rtn = dmffa_OnStop(modes[modeid][mMapID]);
				case MODE_TYPE_RACE: rtn = race_OnStop(modes[modeid][mMapID]);
				case MODE_TYPE_DMTEAM: rtn = dmteam_OnStop(modes[modeid][mMapID]);
				case MODE_TYPE_DERBY: rtn = derby_OnStop(modes[modeid][mMapID]);
				case MODE_TYPE_CTF: rtn = ctf_OnStop(modes[modeid][mMapID]);
				default: rtn = 0;
			}
			
			if (rtn) {
				// Minimode stopped
				ToggleVirtualWorldUsed(modes[modeid][mVirtualWorld], 0);
				modes[modeid][mVirtualWorld] = 0;
				RemoveCheckpoint(modes[modeid][mCheckpointID]);
				modes[modeid][mCheckpointID] = INVALID_CHECKPOINT_ID;
				
				if (modes[modeid][mWaitZoneUsed] >= 0) {
					modeWaitZones[modes[modeid][mWaitZoneUsed]][mzInUse] = 0;
					modes[modeid][mWaitZoneUsed] = -1;
				}
				
				// Destroy text draw
				TextDrawDestroy(modes[modeid][mModeName]);
				
				new logline[MAX_INPUT];
				new tmpstr[MAX_REAL + 1];
				new count;
				format(logline, sizeof(logline), "[FDM] Minimode: %s stopped - Players(", modes[modeid][mName]);
				
				// Mark players as not being in this minimode
				LoopPlayers(i) {
					if (mpData[i][mpModeID] == modeid) {
						format(tmpstr, sizeof(tmpstr), "%i,", i);
						strcat(logline, tmpstr);
						
						// Spectators
						LoopPlayersEx(j, bb) {
							if (GetPlayerSpectatingPlayer(j) != i) continue;
							mode_OnPlayerUnspectate(j, i);
						}
						
						mpData[i][mpModeID] = INVALID_MODE_ID;
						/*if (!mpData[i][mpStartDelay] || GetTickCount() - mpData[i][mpStartDelay] > MIN_START_AFTER_FINISH) { // Delay in starting minimodes
							mpData[i][mpStartDelay] = GetTickCount() + MIN_START_AFTER_FINISH;
						}*/
						
						if (GetPlayerSpectatingPlayer(i) == INVALID_PLAYER_ID) {
							if (modes[modeid][mTimeChanged])
								ResetPlayerTime(i);
							if (modes[modeid][mWeatherChanged])
								ResetPlayerWeather(i);
						}
						
						if (GetPlayerInterior(i) != modes[modeid][mLocation][mwInteriorID]) {
							DisableNextInteriorChange(i);
						}
						
						SetLastSpawnTick(i);
						
						if (IsPlayerSpawned(i)) {
							StartGodSpawning(i);
							LoadPlayerInfo(i);
						} else {
							mpData[i][mpLoadWeapons] = 1;
						}
						
						#if defined _YSI_included
							SetPlayerGravity(i, 0.008);
						#endif
						
						if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
							SetPlayerDrunkLevel(i, 0);
						}
						
						SetPlayerInterior(i, modes[modeid][mLocation][mwInteriorID]);
						SetPlayerVirtualWorld(i, 0);
						
						// ALAR stuff
						if (kGetAdminState(i) & ADMIN_STATE_JAILED || IsAdminSpectating(i)) {
							ClearAdminSpawnData(i);
						}
						
						count++;
					}
				}
				
				if (count) {
					logline[strlen(logline) - 1] = ')';
				} else {
					logline[strlen(logline)] = ')';
				}
				print(logline);
				
				modes[modeid][mState] = MODE_STATE_UNLOADED;
				modes[modeid][mPlayerCount] = 0;
				modes[modeid][mFlags] = 0;
				UpdateModeArrays();
				
				for (new o; o < MODE_OBJECT_MAX; o++) {
					if (modeObjects[modeid][o][oObjectID] != INVALID_OBJECT_ID) {
						DestroyDynamicObject(modeObjects[modeid][o][oObjectID]);
						modeObjects[modeid][o][oObjectID] = INVALID_OBJECT_ID;
					} else {
						break;
					}
				}
				
				for (new o; o < MODE_VEHICLE_MAX; o++) {
					if (modeVehicles[modeid][o][mvVehicleID] && modeVehicles[modeid][o][mvModel]) {
						DestroyVehicle(modeVehicles[modeid][o][mvVehicleID]);
						modeVehicles[modeid][o][mvVehicleID] = 0;
					} else {
						break;
					}
				}
				
				// Send stopped information
				new message[MAX_INPUT];
				format(message, sizeof(message), "%s has ended.", modes[modeid][mName]);
				SendClientMessageToAll(COLOUR_ERROR, message);
				SideChatAddLine(message, COLOUR_MODE_STOP);
				
				OnMinimodeStop(modeid);
				
				return 1;
			} else {
				// Minimode could not be stopped
				return 0;
			}
		}
		
		default: return 0;
	}
	
	return 0;
}

PutPlayerInMode(playerid, modeid) { // Put a player into the minimode
	if (IsPlayerNPC(playerid)) return 0;
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return 0;
	if (modes[modeid][mState] != MODE_STATE_PROGRESS) return 0; // Minimode must be already running
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID) return 0; // Player already in minimode
	if (GetPlayerVirtualWorld(playerid)) return 0;
	
	SavePlayerInfo(playerid);
	//new godmodeon = pGodData[playerid][pgIsOnGodmode];
	
	new rtn;
	switch (modes[modeid][mType]) {
		case MODE_TYPE_INVALID: rtn = 0;
		case MODE_TYPE_DMFFA: rtn = dmffa_OnPlayerEnter(modes[modeid][mMapID], playerid);
		case MODE_TYPE_RACE: rtn = race_OnPlayerEnter(modes[modeid][mMapID], playerid);
		case MODE_TYPE_DMTEAM: rtn = dmteam_OnPlayerEnter(modes[modeid][mMapID], playerid);
		case MODE_TYPE_DERBY: rtn = derby_OnPlayerEnter(modes[modeid][mMapID], playerid);
		case MODE_TYPE_CTF: rtn = ctf_OnPlayerEnter(modes[modeid][mMapID], playerid);
		default: rtn = 0;
	}
	
	if (rtn) {
		// Enter minimode successful
		mpData[playerid][mpModeID] = modeid;
		
		/*if (godmodeon && pGodData[playerid][pgIsOnGodmode]) {
			DisableGodmode(playerid);
		}*/
		
		// Destroy waiting time text draw if any
		if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID) {
			if (mpData[playerid][mpTextExists]) {
				TextDrawDestroy(mpData[playerid][mpText]);
				mpData[playerid][mpTextExists] = 0;
			}
			mpData[playerid][mpLastJoinModeID] = INVALID_MODE_ID;
		}
		
		new votemodeid = mpData[playerid][mpVoteMinimode];
		if (votemodeid != modeid && mpData[playerid][mpVoteMinimode] != INVALID_MODE_ID) {
			mpData[playerid][mpVoteMinimode] = INVALID_MODE_ID;
			CheckMinimodeVoting(votemodeid);
		}
		
		new Float:x,
			Float:y,
			Float:z,
			vehicleid = GetPlayerVehicleID(playerid);
		
		if (vehicleid) {
			GetVehiclePos(vehicleid, x, y ,z);
		} else {
			GetPlayerPos(playerid, x, y ,z);
		}
		
		#if defined _YSI_included
			if (modes[modeid][mGravity] != INVALID_GRAVITY) {
				SetPlayerGravity(playerid, modes[modeid][mGravity]);
			} else {
				SetPlayerGravity(playerid, DEFAULT_GRAVITY);
			}
		#endif
		
		if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
			SetPlayerDrunkLevel(playerid, MODE_DRUNK_LEVEL);
		}
		
		SetPlayerInterior(playerid, modes[modeid][mInteriorID]);
		//StreamObjects(playerid, x, y, z, modes[modeid][mVirtualWorld], modes[modeid][mInteriorID]);
		
		// Show text draw
		TextDrawShowForPlayer(playerid, modes[modeid][mModeName]);
		
		if (modes[modeid][mTimeChanged])
			SetPlayerTime(playerid, modes[modeid][mTimeHour], modes[modeid][mTimeMinute] + ((GetTickCount() - modes[modeid][mTimeBaseTick]) / 1000));
		if (modes[modeid][mWeatherChanged])
			SetPlayerWeather(playerid, modes[modeid][mWeather]);
		
		// Spectators
		LoopPlayers(i) {
			if (GetPlayerSpectatingPlayer(i) != playerid) continue;
			mode_OnPlayerSpectate(i, playerid);
		}
		
		modes[modeid][mPlayerCount]++;
		
		new message[MAX_INPUT];
		format(message, sizeof(message), "%s joined %s",
			pData[playerid][pLoginName], modes[modeid][mName]);
		SideChatAddLine(message, COLOUR_MODE_JOIN);
		printf("[FDM] %s", message);
		
		OnPlayerJoinMinimode(playerid, modeid);
		
		return 1;
	} else {
		// Cannot enter minimode
		return 0;
	}
}

RemovePlayerFromMode(playerid, action[] = "left", aftertext[] = "") { // Remove the player from the minimode
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) return 0; // Player already not in minimode
	
	new rtn;
	new modeid = mpData[playerid][mpModeID];
	switch (modes[modeid][mType]) {
		case MODE_TYPE_INVALID: rtn = 0;
		case MODE_TYPE_DMFFA: rtn = dmffa_OnPlayerExit(modes[modeid][mMapID], playerid);
		case MODE_TYPE_RACE: rtn = race_OnPlayerExit(modes[modeid][mMapID], playerid, action);
		case MODE_TYPE_DMTEAM: rtn = dmteam_OnPlayerExit(modes[modeid][mMapID], playerid);
		case MODE_TYPE_DERBY: rtn = derby_OnPlayerExit(modes[modeid][mMapID], playerid);
		case MODE_TYPE_CTF: rtn = ctf_OnPlayerExit(modes[modeid][mMapID], playerid);
		default: rtn = 0;
	}
	
	if (rtn) {
		// Exit minimode successful
		new message[MAX_INPUT];
		
		if (!action[0]) {
			strcpy(action, "left", 5);
		}
		
		if (aftertext[0]) {
			format(message, sizeof(message), "%s %s %s %s",
				pData[playerid][pLoginName], action, modes[modeid][mName], aftertext);
		} else {
			format(message, sizeof(message), "%s %s %s",
				pData[playerid][pLoginName], action, modes[modeid][mName]);
		}
		SideChatAddLine(message, COLOUR_MODE_LEAVE);
		printf("[FDM] %s", message);
		
		// Spectators
		LoopPlayers(i) {
			if (GetPlayerSpectatingPlayer(i) != playerid) continue;
			mode_OnPlayerUnspectate(i, playerid);
		}
		
		mpData[playerid][mpModeID] = INVALID_MODE_ID;
		
		// Hide text draw
		TextDrawHideForPlayer(playerid, modes[modeid][mModeName]);
		
		/*if (!mpData[playerid][mpStartDelay] || GetTickCount() - mpData[playerid][mpStartDelay] > MIN_START_AFTER_FINISH) { // Delay in starting minimodes
			mpData[playerid][mpStartDelay] = GetTickCount() + MIN_START_AFTER_FINISH;
		}*/
		
		if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) {
			if (modes[modeid][mTimeChanged])
				ResetPlayerTime(playerid);
			if (modes[modeid][mWeatherChanged])
				ResetPlayerWeather(playerid);
		}
		
		if (GetPlayerInterior(playerid) != modes[modeid][mLocation][mwInteriorID]) {
			DisableNextInteriorChange(playerid);
		}
		
		SetLastSpawnTick(playerid);
		
		if (IsPlayerSpawned(playerid)) {
			StartGodSpawning(playerid);
			LoadPlayerInfo(playerid);
		} else {
			mpData[playerid][mpLoadWeapons] = 1;
		}
		
		#if defined _YSI_included
			SetPlayerGravity(playerid, 0.008);
		#endif
		
		if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
			SetPlayerDrunkLevel(playerid, 0);
		}
		
		SetPlayerInterior(playerid, modes[modeid][mLocation][mwInteriorID]);
		
		modes[modeid][mPlayerCount]--;
		
		OnPlayerLeaveMinimode(playerid, modeid);
		
		// Check for remaining players
		if (!modes[modeid][mPlayerCount]) StopMinimode(modeid);
		
		return 1;
	} else {
		// Cannot exit minimode
		return 0;
	}
}

FindPlayerMinimodeWithMsg(playerid, const usagemsg[], const notfoundmsg[], const modename[], modetype = MODE_TYPE_INVALID, modestate = -1) {
	new modeid = FindPlayerMinimode(playerid, modename, modetype, modestate);
	
	if (modeid == INVALID_MODE_ID) {
		if (isnull(modename)) {
			SendClientMessage(playerid, COLOUR_HELP, usagemsg);
		} else {
			SendClientMessage(playerid, COLOUR_ERROR, notfoundmsg);
		}
	}
	
	return modeid;
}

FindPlayerMinimode(playerid, const modename[], modetype = MODE_TYPE_INVALID, modestate = -1) {
	if (!isnull(modename)) {
		return FindMinimode(modename, modetype, modestate);
	}
	
	if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID && (modetype == MODE_TYPE_INVALID || modetype == modes[mpData[playerid][mpLastJoinModeID]][mType])) {
		return mpData[playerid][mpLastJoinModeID];
	}
	
	new specid = GetPlayerSpectatingPlayer(playerid);
	if (specid != INVALID_PLAYER_ID && mpData[specid][mpModeID] != INVALID_MODE_ID && (modetype == MODE_TYPE_INVALID || modetype == modes[mpData[specid][mpModeID]][mType])) {
		return mpData[specid][mpModeID];
	}
	
	new checkpointid = GetPlayerCurrentCheckpoint(playerid);
	if (checkpointid != INVALID_CHECKPOINT_ID) {
		LoopMinimodesProgress(modeid) {
			if (modes[modeid][mCheckpointID] == checkpointid) {
				return modeid;
			}
		}
		
		LoopMinimodesWaiting(modeid) {
			//if (IsPlayerInCheckpointID(playerid, modes[modeid][mCheckpointID])) {
			if (modes[modeid][mCheckpointID] == checkpointid) {
				return modeid;
			}
		}
	}
	
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID || (modetype == MODE_TYPE_INVALID || modetype == modes[mpData[playerid][mpModeID]][mType])) {
		return mpData[playerid][mpModeID];
	}
	
	if (specid != INVALID_PLAYER_ID && mpData[specid][mpLastJoinModeID] != INVALID_MODE_ID) {
		return mpData[specid][mpLastJoinModeID];
	}
	
	return INVALID_MODE_ID;
}

FindMinimode(const modename[], modetype = MODE_TYPE_INVALID, modestate = -1) { // Finds a modeid from its name
	if (!modename[0]) return INVALID_MODE_ID;
	
	if (isNumeric(modename)) {
		new modeid = strval(modename);
		if (modeid < 0 || modeid >= MAX_MINIMODES) return INVALID_MODE_ID;
		if (modes[modeid][mType] == MODE_TYPE_INVALID) {
			return INVALID_MODE_ID;
		} else if (modetype != MODE_TYPE_INVALID && modes[modeid][mType] != modetype) {
			return INVALID_MODE_ID;
		} else if (modestate != -1 && modes[modeid][mState] != modestate) {
			return INVALID_MODE_ID;
		}
		if (!modes[modeid][mName][0]) return INVALID_MODE_ID;
		return modeid;
	}
	
	new idx;
	new keywords[12][24];
	new keywordcount;
	for (new i; i < sizeof(keywords); i++) {
		splitcpy(keywords[i], modename, idx, ' ', 24);
		if (keywords[i][0]) keywordcount++;
	}
	
	if (!keywordcount) return INVALID_MODE_ID;
	
	new modeid = INVALID_MODE_ID;
	new modestrlength;
	
	for (new i; i < MAX_MINIMODES; i++) {
		if (modes[i][mType] == MODE_TYPE_INVALID) continue;
		if (modetype != MODE_TYPE_INVALID && modes[i][mType] != modetype) continue;
		if (modestate != -1 && modes[i][mState] != modestate) continue;
		if (!modes[i][mName][0]) continue;
		
		//if (strfind(modes[i][mName], modename, true) != -1) return i;
		
		idx = 0;
		new keywordsfound;
		for (new j; j < sizeof(keywords); j++) {
			if (!keywords[j][0]) continue;
			
			// Primary mode name search
			idx = strfind(modes[i][mName], keywords[j], true, idx);
			if (idx != -1) {
				idx = idx + strlen(keywords[j]);
				keywordsfound++;
				if (idx >= MAX_MODE_NAME) break;
			} else {
				break;
			}
		}
		
		if (keywordsfound >= keywordcount) {
			new len = strlen(modes[i][mName]);
			
			if (!modestrlength || len < modestrlength) {
				modeid = i;
				modestrlength = len;
			}
		}
	}
	
	return modeid;
}

IsMinimodeJoinable(modeid) {
	if (modes[modeid][mState] == MODE_STATE_UNLOADED) {
		return 0;
	} else if (modes[modeid][mState] == MODE_STATE_WAITING) {
		return 1;
	} else if (modes[modeid][mState] == MODE_STATE_PROGRESS) {
		switch (modes[modeid][mType]) {
			case MODE_TYPE_INVALID: return 0;
			case MODE_TYPE_DMFFA: return dmffa_IsJoinable(modes[modeid][mMapID]);
			case MODE_TYPE_RACE: return race_IsJoinable(modes[modeid][mMapID]);
			case MODE_TYPE_DMTEAM: return dmteam_IsJoinable(modes[modeid][mMapID]);
			case MODE_TYPE_DERBY: return derby_IsJoinable(modes[modeid][mMapID]);
			case MODE_TYPE_CTF: return ctf_IsJoinable(modes[modeid][mMapID]);
		}
		return 0;
	} else {
		return 0;
	}
}

CheckMinimodeVoting(modeid, &playersVoted = 0, &playersInMode = 0, allowStart = 1) {
	if (modeid == INVALID_MODE_ID || modes[modeid][mState] != MODE_STATE_WAITING) return 0;
	
	playersInMode = 0;
	playersVoted = 0;
	
	LoopPlayers(playerid) {
		if (mpData[playerid][mpLastJoinModeID] == modeid) {
			playersInMode++;
		}
		if (mpData[playerid][mpVoteMinimode] == modeid) {
			playersVoted++;
		}
	}
	
	if (playersVoted > 0 && playersInMode >= modes[modeid][mMinPlayers] && MODE_START_TIME - (modes[modeid][mStartTime] - GetTickCount()) > MIN_QUICKSTART_INTERVAL && ((playersVoted * 100) / playersInMode >= modeStartVotePercent || playersInMode >= modes[modeid][mMaxPlayers])) {
		if (allowStart) StartMinimode(modeid);
		return 1;
	} else {
		return 0;
	}
}

stock SendMinimodeInfoToAll(modeid) {
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return 0;
	
	new message[MAX_STRING];
	
	#if MODE_LEVEL_REQUIREMENTS
		format(message, sizeof(message), "%s | Min. Level: %i | Cost: $%i | ",
			modes[modeid][mName], modes[modeid][mMinLevel], modes[modeid][mCost]);
	#else
		format(message, sizeof(message), "%s | Cost: $%i | ",
			modes[modeid][mName], modes[modeid][mCost]);
	#endif
	if (modes[modeid][mNotes][0]) {
		format(message, sizeof(message), "%s%s | ",
			message, modes[modeid][mNotes], modeid);
	}
	if (modes[modeid][mAuthor][0]) {
		format(message, sizeof(message), "%sAuthor: %s | ",
			message, modes[modeid][mAuthor]);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
		format(message, sizeof(message), "%sDRUNK! | ",
			message);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_HALFXP) {
		format(message, sizeof(message), "%sHALF XP | ",
			message);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L1) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L1);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L2) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L2);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L3) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L3);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L1) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L1);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L2) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L2);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L3) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L3);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_DAMAGED) {
		format(message, sizeof(message), "%sDAMAGED! | ",
			message);
	}
	
	format(message, sizeof(message), "%s/join %i", message, modeid);
	
	/*#if defined _irc_included
		CallRemoteFunction("ircSendClientMessageToAll", "is", colour, message);
	#endif*/
	
	//SideChatAddLine(message, COLOUR_MODE_ALLOWED);
	#if MODE_LEVEL_REQUIREMENTS
		new colour;
		LoopPlayers(i) {
			if (pData[i][pLevel] < modes[modeid][mMinLevel]) {
				colour = COLOUR_MODE_BLOCKED;
			} else {
				colour = COLOUR_MODE_ALLOWED;
			}
			
			SendWrappedMessageToPlayer(i, colour, message, 128);
		}
	#else
		SendWrappedMessageToAll(COLOUR_MODE_ALLOWED, message, 128);
	#endif
	return 1;
}

stock SendMinimodeInfoToPlayer(playerid, modeid) {
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return 0;
	// Send minimode information
	new message[MAX_STRING];
	new colour;
	
	/*
	if (modes[modeid][mNotes][0]) {
		format(message, sizeof(message), "%s | Min. Level: %i | Cost: $%i | %s | Starts in %is | /join %i",
			modes[modeid][mName], modes[modeid][mMinLevel], modes[modeid][mCost], modes[modeid][mNotes], (modes[modeid][mStartTime] - GetTickCount()) / 1000, modeid);
	} else {
		format(message, sizeof(message), "%s | Min. Level: %i | Cost: $%i | Starts in %is | /join %i",
			modes[modeid][mName], modes[modeid][mMinLevel], modes[modeid][mCost], (modes[modeid][mStartTime] - GetTickCount()) / 1000, modeid);
	}
	*/
	
	#if MODE_LEVEL_REQUIREMENTS
		format(message, sizeof(message), "%s | Min. Level: %i | Cost: $%i | ",
			modes[modeid][mName], modes[modeid][mMinLevel], modes[modeid][mCost]);
	#else
		format(message, sizeof(message), "%s | Cost: $%i | ",
			modes[modeid][mName], modes[modeid][mCost]);
	#endif
	if (modes[modeid][mNotes][0]) {
		format(message, sizeof(message), "%s%s | ",
			message, modes[modeid][mNotes], modeid);
	}
	if (modes[modeid][mAuthor][0]) {
		format(message, sizeof(message), "%sAuthor: %s | ",
			message, modes[modeid][mAuthor]);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
		format(message, sizeof(message), "%sDRUNK! | ",
			message);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_HALFXP) {
		format(message, sizeof(message), "%sHALF XP | ",
			message);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L1) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L1);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L2) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L2);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L3) {
		format(message, sizeof(message), "%s%iX XP! | ",
			message, MODE_MULTIPLIER_XP_L3);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L1) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L1);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L2) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L2);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L3) {
		format(message, sizeof(message), "%s%iX CASH! | ",
			message, MODE_CASH_MULTIPLIER_L3);
	}
	if (modes[modeid][mFlags] & MODE_FLAG_DAMAGED) {
		format(message, sizeof(message), "%sDAMAGED! | ",
			message);
	}
	
	switch (modes[modeid][mState]) {
		case MODE_STATE_UNLOADED: format(message, sizeof(message), "%sNot Loaded | /start %i", message, modeid);
		case MODE_STATE_WAITING: format(message, sizeof(message), "%sStarts in %is | /join %i", message, (modes[modeid][mStartTime] - GetTickCount()) / 1000, modeid);
		case MODE_STATE_PROGRESS: {
			if (IsMinimodeJoinable(modeid)) {
				format(message, sizeof(message), "%sIn Progress | /join %i", message, modeid);
			} else {
				format(message, sizeof(message), "%sIn Progress | /spec %i", message, modeid);
			}
		}
	}
	
	#if MODE_LEVEL_REQUIREMENTS
		if (pData[playerid][pLevel] < modes[modeid][mMinLevel]) {
			colour = COLOUR_MODE_BLOCKED;
		} else {
			if (IsMinimodeJoinable(modeid)) {
				colour = COLOUR_MODE_ALLOWED;
			} else {
				colour = COLOUR_MODE_BLOCKED;
			}
		}
	#else
		if (IsMinimodeJoinable(modeid)) {
			colour = COLOUR_MODE_ALLOWED;
		} else {
			colour = COLOUR_MODE_BLOCKED;
		}
	#endif
	SendWrappedMessageToPlayer(playerid, colour, message, 128);
	return 1;
}

stock IsPlayerInMinimode(playerid) {
	if (mpData[playerid][mpModeID] == INVALID_MODE_ID) {
		return 0;
	} else {
		if (modes[mpData[playerid][mpModeID]][mState] == MODE_STATE_PROGRESS) {
			return 1;
		} else {
			return 0;
		}
	}
}

stock IsVehicleAllowedTeleport(vehicletype) {
	if (vehicletype < 400 || vehicletype > 611) return 0;
	switch (vehicletype) {
		case 403,406,407,408,414,417,425,427,428,430,431,432,433,435,437,443,444,447,449,450,452,453,454,455,456,464,465,469,472,476,484,487,488,493,497,501,508,511,512,513,514,515,519,520,524,525,532,538,537,544,548,553,556,557,563,569,570,573,577,578,584,588,590,591,592,593,595,601,609: return 0;
	}
	return 1;
}

stock MinimodeProcessFlagsXP(modeid, xp) {
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L1) xp = xp * MODE_MULTIPLIER_XP_L1;
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L2) xp = xp * MODE_MULTIPLIER_XP_L2;
	if (modes[modeid][mFlags] & MODE_FLAG_XP_L3) xp = xp * MODE_MULTIPLIER_XP_L3;
	if (modes[modeid][mFlags] & MODE_FLAG_HALFXP) xp = xp / 2;
	return xp;
}

stock MinimodeProcessFlagsMoney(modeid, money) {
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L1) money = money * MODE_CASH_MULTIPLIER_L1;
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L2) money = money * MODE_CASH_MULTIPLIER_L2;
	if (modes[modeid][mFlags] & MODE_FLAG_CASH_L3) money = money * MODE_CASH_MULTIPLIER_L3;
	return money;
}

//-----------------------------------------------------------------------------------------------------
// SPECTATE EVENTS
//-----------------------------------------------------------------------------------------------------

mode_OnPlayerSpectate(playerid, specid) {
	new playerMode = mpData[playerid][mpModeID];
	if (playerMode == INVALID_MODE_ID) playerMode = mpData[playerid][mpLastJoinModeID];
	new specMode = mpData[specid][mpModeID];
	if (specMode == INVALID_MODE_ID) specMode = mpData[specid][mpLastJoinModeID];
	
	if (playerMode != specMode) {
		if (playerMode != INVALID_MODE_ID) {
			TextDrawHideForPlayer(playerid, modes[playerMode][mModeName]);
		}
		if (specMode != INVALID_MODE_ID) {
			TextDrawShowForPlayer(playerid, modes[specMode][mModeName]);
		}
	}
	
	if (mpData[playerid][mpModeID] != mpData[specid][mpModeID]) {
		#define modeid mpData[specid][mpModeID]
		if (modeid == INVALID_MODE_ID) {
			ResetPlayerTime(playerid);
			ResetPlayerWeather(playerid);
		} else {
			if (modes[modeid][mTimeChanged])
				SetPlayerTime(playerid, modes[modeid][mTimeHour], modes[modeid][mTimeMinute] + ((GetTickCount() - modes[modeid][mTimeBaseTick]) / 1000));
			else
				ResetPlayerTime(playerid);
			
			if (modes[modeid][mWeatherChanged])
				SetPlayerWeather(playerid, modes[modeid][mWeather]);
			else
				ResetPlayerWeather(playerid);
		}
		#undef modeid
	}
	
	if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[playerid][mpTextExists]) {
		TextDrawHideForPlayer(playerid, mpData[playerid][mpText]);
	}
	if (mpData[specid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[specid][mpTextExists]) {
		TextDrawShowForPlayer(playerid, mpData[specid][mpText]);
	}
	
	return 1;
}

mode_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	new oldMode = mpData[oldspecid][mpModeID];
	if (oldMode == INVALID_MODE_ID) oldMode = mpData[oldspecid][mpLastJoinModeID];
	new newMode = mpData[newspecid][mpModeID];
	if (newMode == INVALID_MODE_ID) newMode = mpData[newspecid][mpLastJoinModeID];
	
	if (oldMode != newMode) {
		if (oldMode != INVALID_MODE_ID) {
			TextDrawHideForPlayer(playerid, modes[oldMode][mModeName]);
		}
		if (newMode != INVALID_MODE_ID) {
			TextDrawShowForPlayer(playerid, modes[newMode][mModeName]);
		}
	}
	
	if (mpData[oldspecid][mpModeID] != mpData[newspecid][mpModeID]) {
		#define modeid mpData[newspecid][mpModeID]
		if (modeid == INVALID_MODE_ID) {
			ResetPlayerTime(playerid);
			ResetPlayerWeather(playerid);
		} else {
			if (modes[modeid][mTimeChanged])
				SetPlayerTime(playerid, modes[modeid][mTimeHour], modes[modeid][mTimeMinute] + ((GetTickCount() - modes[modeid][mTimeBaseTick]) / 1000));
			else
				ResetPlayerTime(playerid);
			
			if (modes[modeid][mWeatherChanged])
				SetPlayerWeather(playerid, modes[modeid][mWeather]);
			else
				ResetPlayerWeather(playerid);
		}
		#undef modeid
	}
	
	if (mpData[oldspecid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[oldspecid][mpTextExists]) {
		TextDrawHideForPlayer(playerid, mpData[oldspecid][mpText]);
	}
	if (mpData[newspecid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[newspecid][mpTextExists]) {
		TextDrawShowForPlayer(playerid, mpData[newspecid][mpText]);
	}
	
	return 1;
}

mode_OnPlayerUnspectate(playerid, specid) {
	new playerMode = mpData[playerid][mpModeID];
	if (playerMode == INVALID_MODE_ID) playerMode = mpData[playerid][mpLastJoinModeID];
	new specMode = mpData[specid][mpModeID];
	if (specMode == INVALID_MODE_ID) specMode = mpData[specid][mpLastJoinModeID];
	
	if (playerMode != specMode) {
		if (specMode != INVALID_MODE_ID) {
			TextDrawHideForPlayer(playerid, modes[specMode][mModeName]);
		}
		if (playerMode != INVALID_MODE_ID) {
			TextDrawShowForPlayer(playerid, modes[playerMode][mModeName]);
		}
	}
	
	if (mpData[playerid][mpModeID] != mpData[specid][mpModeID]) {
		#define modeid mpData[playerid][mpModeID]
		if (modeid == INVALID_MODE_ID) {
			ResetPlayerTime(playerid);
			SetPlayerWeather(playerid, DEFAULT_WEATHER_ID);
		} else {
			if (modes[modeid][mTimeChanged])
				SetPlayerTime(playerid, modes[modeid][mTimeHour], modes[modeid][mTimeMinute] + ((GetTickCount() - modes[modeid][mTimeBaseTick]) / 1000));
			else
				ResetPlayerTime(playerid);
			
			if (modes[modeid][mWeatherChanged])
				SetPlayerWeather(playerid, modes[modeid][mWeather]);
			else
				ResetPlayerWeather(playerid);
		}
		#undef modeid
	}
	
	if (mpData[specid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[specid][mpTextExists]) {
		TextDrawHideForPlayer(playerid, mpData[specid][mpText]);
	}
	if (mpData[playerid][mpLastJoinModeID] != INVALID_MODE_ID && mpData[playerid][mpTextExists]) {
		TextDrawShowForPlayer(playerid, mpData[playerid][mpText]);
	}
	
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:j(playerid, text[]) return kcmd_join(playerid, text);
kcmd:join(playerid, text[]) { // Join a minimode
	new modeid = FindPlayerMinimode(playerid, text, MODE_TYPE_INVALID, MODE_STATE_WAITING);
	if (modeid == INVALID_MODE_ID) {
		modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /join [minimode id/name]", "MINIMODES: Invalid minimode ID/Name.", text, MODE_TYPE_INVALID, MODE_STATE_PROGRESS);
		if (modeid == INVALID_MODE_ID) return 1;
	}
	
	if (!IsPlayerSpawned(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must be spawned to join minimodes.");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must registered to join minimodes (/register [password])");
		return 1;
	}
	new message[MAX_INPUT];
	#if MODE_LEVEL_REQUIREMENTS
		if (pData[playerid][pLevel] < modes[modeid][mMinLevel]) {
			format(message, sizeof(message), "MINIMODES: You need to be at least level %i to join %s", modes[modeid][mMinLevel], modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
	#endif
	if (modes[modeid][mCost] >= 0 && GetPlayerMoney(playerid) < modes[modeid][mCost]) {
		format(message, sizeof(message), "MINIMODES: You need to have $%i to join %s", modes[modeid][mCost], modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID) {
		format(message, sizeof(message), "MINIMODES: You are already in %s.", modes[mpData[playerid][mpModeID]][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (GetPlayerInterior(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must not be in an interior to join minimodes.");
		return 1;
	}
	if (combatBlockTeleport && GetPlayerCombatPlayer(playerid) != INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You cannot join minimodes while in combat.");
		return 1;
	}
	
	#if MODE_ALLOW_TELEPORT
		if (modes[modeid][mState] == MODE_STATE_WAITING) { // Teleport player
			
			#if ISOLATE_VIRTUAL_WORLDS
				if (GetPlayerVirtualWorld(playerid)) {
					SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must be in the main world to teleport to minimodes.");
					return 1;
				}
			#endif
			
			if (GetPlayerState(playerid) == PLAYER_STATE_PASSENGER) {
				SetPlayerPosRandom(playerid,
					modes[modeid][mLocation][mX], modes[modeid][mLocation][mY], modes[modeid][mLocation][mZ], 5);
			} else {
				new vid = GetPlayerVehicleID(playerid);
				if (vid && IsVehicleAllowedTeleport(GetVehicleModel(vid))) {
					SetVehiclePosRandom(vid,
						modes[modeid][mLocation][mX], modes[modeid][mLocation][mY], modes[modeid][mLocation][mZ], 5);
					
					// Flip vehicle
					new Float:ang;
					GetVehicleZAngle(vid, ang);
					SetVehicleZAngle(vid, ang);
				} else {
					SetPlayerPosRandom(playerid,
						modes[modeid][mLocation][mX], modes[modeid][mLocation][mY], modes[modeid][mLocation][mZ], 5);
				}
			}
			
			SetPlayerInterior(playerid, modes[modeid][mLocation][mwInteriorID]);
			format(message, sizeof(message), "MINIMODES: You have teleported to %s.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
			return 1;
		}
	#endif
	
	#if ISOLATE_VIRTUAL_WORLDS
		if (GetPlayerVirtualWorld(playerid)) {
			SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must be in the main world to join minimodes.");
			return 1;
		}
	#endif
	
	if (!PutPlayerInMode(playerid, modeid)) { // Join minimode
		format(message, sizeof(message), "MINIMODES: %s cannot be joined.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	format(message, sizeof(message), "MINIMODES: %s joined.", modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:l(playerid, text[]) return kcmd_leave(playerid, text);
kcmd:leave(playerid, text[]) { // Leave a minimode
	new message[MAX_INPUT];
	new modeid = mpData[playerid][mpModeID];
	if (modeid == INVALID_MODE_ID) {
		if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && spData[playerid][spSpecPlayer] != ALAR_SPEC_ID) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPEC: You have left spectate mode.");
			UnspecPlayer(playerid);
			
			format(message, sizeof(message), "%s left spectate mode", pData[playerid][pLoginName]);
			SideChatAddLine(message, COLOUR_SPECTATE);
			printf("[FDM] %s", message);
			return 1;
		}
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You are not in a minimode.");
		return 1;
	}
	if (!RemovePlayerFromMode(playerid)) { // Leave minimode
		format(message, sizeof(message), "MINIMODES: %s cannot be left.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	format(message, sizeof(message), "MINIMODES: %s left.", modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	return 1;
}

kcmd:modeinfo(playerid, text[]) { // Minimode detailed info
	// Get minimode
	new modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /modeinfo [minimode id/name]", "MINIMODES: Invalid minimode ID/Name.", text);
	if (modeid == INVALID_MODE_ID) return 1;
	
	new msg[MAX_INPUT],
		players[MAX_PLAYERS],
		anyplayers;
	
	format(msg, sizeof(msg), "%s Information:", modes[modeid][mName]);
	SendClientMessage(playerid, COLOUR_TITLE, msg);
	
	// Check minimode state
	if (modes[modeid][mState] == MODE_STATE_UNLOADED) {
		msg = "Status: Unloaded";
	} else if (modes[modeid][mState] == MODE_STATE_WAITING) {
		msg = "Status: Waiting for players";
		
		LoopPlayers(i) { // Check for players in minimode
			if (mpData[i][mpLastJoinModeID] == modeid && IsPlayerInCheckpointID(i, modes[modeid][mCheckpointID])) {
				players[i] = 1;
				anyplayers = 1;
			}
		}
	} else {
		msg = "Status: In Progress";
		
		LoopPlayers(i) { // Check for players in minimode
			if (mpData[i][mpModeID] == modeid) {
				players[i] = 1;
				anyplayers = 1;
			}
		}
	}
	
	#if MODE_LEVEL_REQUIREMENTS
		format(msg, sizeof(msg), "%s | Min. Level: %i | Cost: $%i", msg, modes[modeid][mMinLevel], modes[modeid][mCost]);
	#else
		format(msg, sizeof(msg), "%s | Cost: $%i", msg, modes[modeid][mCost]);
	#endif
	if (modes[modeid][mAuthor][0]) {
		format(msg, sizeof(msg), "%s | Author: %s", msg, modes[modeid][mAuthor]);
	}
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	if (modes[modeid][mFlags]) {
		msg = "Flags: ";
		if (modes[modeid][mFlags] & MODE_FLAG_DRUNK) {
			format(msg, sizeof(msg), "%sDRUNK! | ",
				msg);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_HALFXP) {
			format(msg, sizeof(msg), "%sHALF XP | ",
				msg);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_XP_L1) {
			format(msg, sizeof(msg), "%s%iX XP! | ",
				msg, MODE_MULTIPLIER_XP_L1);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_XP_L2) {
			format(msg, sizeof(msg), "%s%iX XP! | ",
				msg, MODE_MULTIPLIER_XP_L2);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_XP_L3) {
			format(msg, sizeof(msg), "%s%iX XP! | ",
				msg, MODE_MULTIPLIER_XP_L3);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_CASH_L1) {
			format(msg, sizeof(msg), "%s%iX CASH! | ",
				msg, MODE_CASH_MULTIPLIER_L1);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_CASH_L2) {
			format(msg, sizeof(msg), "%s%iX CASH! | ",
				msg, MODE_CASH_MULTIPLIER_L2);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_CASH_L3) {
			format(msg, sizeof(msg), "%s%iX CASH! | ",
				msg, MODE_CASH_MULTIPLIER_L3);
		}
		if (modes[modeid][mFlags] & MODE_FLAG_DAMAGED) {
			format(msg, sizeof(msg), "%sDAMAGED! | ",
				msg);
		}
		msg[strlen(msg) - 3] = 0;
		SendClientMessage(playerid, COLOUR_HELP, msg);
	}
	
	if (modes[modeid][mDescription][0]) {
		format(msg, sizeof(msg), "Description: %s", modes[modeid][mDescription]);
		SendWrappedMessageToPlayer(playerid, COLOUR_HELP, msg, 128);
	}
	
	switch (modes[modeid][mType]) {
		case MODE_TYPE_DMFFA: dmffa_ShowModeInfo(modes[modeid][mMapID], playerid);
		case MODE_TYPE_RACE: race_ShowModeInfo(modes[modeid][mMapID], playerid);
		case MODE_TYPE_DMTEAM: dmteam_ShowModeInfo(modes[modeid][mMapID], playerid);
		case MODE_TYPE_DERBY: derby_ShowModeInfo(modes[modeid][mMapID], playerid);
		case MODE_TYPE_CTF: ctf_ShowModeInfo(modes[modeid][mMapID], playerid);
	}
	
	// Show players in minimode
	if (anyplayers) {
		new playersinfo[MAX_STRING];
		playersinfo = "Players: ";
		LoopPlayers(i) { // Check for players in minimode
			if (players[i]) {
				strcat(playersinfo, pData[i][pLoginName]);
				strcat(playersinfo, ", ");
			}
		}
		
		new len = strlen(playersinfo);
		if (len > 250) {
			playersinfo[247] = '.';
			playersinfo[248] = '.';
			playersinfo[249] = '.';
			playersinfo[250] = 0;
		} else {
			playersinfo[len - 2] = 0;
		}
		
		SendWrappedMessageToPlayer(playerid, COLOUR_HELP, playersinfo);
	}
	
	return 1;
}

kcmd:modes(playerid, text[]) { // List of minimodes
	if (isnull(text)) { // Loaded minimodes search
		
		SendClientMessage(playerid, COLOUR_TITLE, "Minimodes Loaded:");
		
		new modecount;
		LoopMinimodesWaiting(i) {
			SendMinimodeInfoToPlayer(playerid, i);
			modecount++;
		}
		
		LoopMinimodesProgress(i) {
			SendMinimodeInfoToPlayer(playerid, i);
			modecount++;
		}
		
		if (!modecount) {
			SendClientMessage(playerid, COLOUR_HELP, "No minimodes avaliable.");
		}
		
	} else if (isNumeric(text)) { // Page search
		
		new page = strval(text);
		if (page < 1) {
			SendClientMessage(playerid, COLOUR_ERROR, "MODES: Invalid page number.");
			return 1;
		}
		
		new msg[MAX_INPUT];
		format(msg, sizeof(msg), "Avaliable Minimodes (Page %i):", page);
		SendClientMessage(playerid, COLOUR_TITLE, msg);
		
		new skip = (page - 1) * MODES_PER_PAGE;
		new skipcount;
		new modecount;
		
		for (new i; i < MAX_MINIMODES; i++) {
			if (modes[i][mType] == MODE_TYPE_INVALID) continue;
			
			if (skipcount < skip) {
				skipcount++;
			} else {
				SendMinimodeInfoToPlayer(playerid, i);
				modecount++;
				if (modecount >= MODES_PER_PAGE) break;
			}
		}
		
		if (!modecount) {
			SendClientMessage(playerid, COLOUR_HELP, "No minimodes on this page.");
		}
		
	} else { // Advanced find search
		
		SendClientMessage(playerid, COLOUR_TITLE, "Avaliable Minimodes Found:");
		
		new idx;
		new keywords[12][24];
		new keywordcount;
		
		for (new i; i < sizeof(keywords); i++) {
			splitcpy(keywords[i], text, idx, ' ', 24);
			if (keywords[i][0]) keywordcount++;
		}
		
		if (!keywordcount) return INVALID_MODE_ID;
		
		new modecount;
		new modelisted[MAX_MINIMODES];
		
		// Primary mode name search
		for (new i; i < MAX_MINIMODES; i++) {
			if (modes[i][mType] == MODE_TYPE_INVALID) continue;
			if (!modes[i][mName][0]) continue;
			
			idx = 0;
			new keywordsfound;
			
			for (new j; j < sizeof(keywords); j++) {
				if (!keywords[j][0]) continue;
				
				idx = strfind(modes[i][mName], keywords[j], true, idx);
				if (idx != -1) {
					idx = idx + strlen(keywords[j]);
					keywordsfound++;
					if (idx >= MAX_MODE_NAME) break;
				} else {
					break;
				}
			}
			
			if (keywordsfound >= keywordcount) {
				// Minimode name search hit
				modecount++;
				
				if (modecount > MODE_MAX_SEARCH_RESULTS) {
					SendClientMessage(playerid, COLOUR_ERROR, "Maximum results reached.");
					break;
				} else {
					SendMinimodeInfoToPlayer(playerid, i);
					modelisted[i] = 1;
				}
			}
		}
		
		// Secondary author name search
		if (modecount <= MODE_MAX_SEARCH_RESULTS) {
			for (new i; i < MAX_MINIMODES; i++) {
				if (modes[i][mType] == MODE_TYPE_INVALID) continue;
				if (!modes[i][mAuthor][0]) continue;
				if (modelisted[i]) continue;
				
				idx = 0;
				new authormatch;
				
				for (new j; j < sizeof(keywords); j++) {
					if (!keywords[j][0]) continue;
					
					// Secondary author search
					if (!authormatch &&
						modes[i][mAuthor][0] &&
						(strfind(modes[i][mAuthor], keywords[j], true, 0) != -1)
						) {
							authormatch = 1;
					}
				}
				
				if (authormatch) {
					// Minimode author search hit
					modecount++;
					
					if (modecount > MODE_MAX_SEARCH_RESULTS) {
						SendClientMessage(playerid, COLOUR_ERROR, "Maximum results reached.");
						break;
					} else {
						SendMinimodeInfoToPlayer(playerid, i);
						modelisted[i] = 1;
					}
				}
			}
			
			// Third notes search
			if (modecount <= MODE_MAX_SEARCH_RESULTS) {
				for (new i; i < MAX_MINIMODES; i++) {
					if (modes[i][mType] == MODE_TYPE_INVALID) continue;
					if (!modes[i][mNotes][0]) continue;
					if (modelisted[i]) continue;
					
					idx = 0;
					new notesmatch;
					
					for (new j; j < sizeof(keywords); j++) {
						if (!keywords[j][0]) continue;
						
						// Secondary author search
						if (!notesmatch &&
							modes[i][mNotes][0] &&
							(strfind(modes[i][mNotes], keywords[j], true, 0) != -1)
							) {
								notesmatch = 1;
						}
					}
					
					if (notesmatch) {
						// Minimode notes search hit
						modecount++;
						
						if (modecount > MODE_MAX_SEARCH_RESULTS) {
							SendClientMessage(playerid, COLOUR_ERROR, "Maximum results reached.");
							break;
						} else {
							SendMinimodeInfoToPlayer(playerid, i);
							//modelisted[i] = 1;
						}
					}
				}
			}
			
		}
		
		if (!modecount) {
			SendClientMessage(playerid, COLOUR_HELP, "No minimodes found.");
		}
		
	}
	return 1;
}

kcmd:s(playerid, text[]) return cmd_start(playerid, text, 0);
kcmd:dstart(playerid, text[]) return cmd_start(playerid, text, 1); // drunken mode
kcmd:start(playerid, text[]) return cmd_start(playerid, text, 0);
cmd_start(playerid, text[], drunk) { // Player start a minimode
	new message[MAX_INPUT];
	
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You must be registered to use /start (/register [password])");
		return 1;
	}
	if (modesLocked) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: Minimodes are currently locked from starting.");
		return 1;
	}
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "MINIMODES: You cannot use /start while in a minimode.");
		return 1;
	}
	
	new modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /start [minimode id/name]", "MINIMODES: Invalid minimode ID/Name.", text);
	if (modeid == INVALID_MODE_ID) return 1;
	
	if (modes[modeid][mState] == MODE_STATE_PROGRESS) {
		format(message, sizeof(message), "MINIMODES: %s is currently in progress.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (modes[modeid][mState] == MODE_STATE_WAITING) {
		if (drunk) {
			format(message, sizeof(message), "MINIMODES: %s is already waiting for players.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
		
		if (mpData[playerid][mpLastJoinModeID] == modeid) {
			/*if (MODE_START_TIME - (modes[modeid][mStartTime] - GetTickCount()) < MIN_QUICKSTART_INTERVAL) {
				format(message, sizeof(message), "MINIMODES: You cannot quick-start minimodes within the first %is of starting. (%is to go)", MIN_QUICKSTART_INTERVAL / 1000, (MIN_QUICKSTART_INTERVAL - (MODE_START_TIME - (modes[modeid][mStartTime] - GetTickCount()))) / 1000);
				SendClientMessage(playerid, COLOUR_ERROR, message);
				return 1;
			} else*/
			if (!IsPlayerInCheckpointID(playerid, modes[modeid][mCheckpointID])) {
				format(message, sizeof(message), "MINIMODES: You must be in %s to quick-start it.", modes[modeid][mName]);
				SendClientMessage(playerid, COLOUR_ERROR, message);
				return 1;
			}
		} else {
			format(message, sizeof(message), "MINIMODES: You must be in %s to quick-start it.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
		
		if (mpData[playerid][mpVoteMinimode] == modeid) {
			format(message, sizeof(message), "MINIMODES: You have already voted %s to be quick-started.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
		
		mpData[playerid][mpVoteMinimode] = modeid;
		
		new playersVoted, playersInMode, rtn;
		rtn = CheckMinimodeVoting(modeid, playersVoted, playersInMode, 0);
		
		format(message, sizeof(message), "MINIMODES: You have voted to quick-start %s (%i/%i)", modes[modeid][mName], playersVoted, playersInMode);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s voted to quick-start %s (%i/%i)", pData[playerid][pLoginName], modes[modeid][mName], playersVoted, playersInMode);
		SideChatAddLine(message, COLOUR_MODE_VOTE);
		printf("[FDM] %s", message);
		
		if (rtn) {
			if (modeStartHalfXP) {
				StartMinimode(modeid, MODE_FLAG_HALFXP);
			} else {
				StartMinimode(modeid);
			}
		}
		
	} else {
		if (mpData[playerid][mpStartDelay] && mpData[playerid][mpStartDelay] > GetTickCount()) {
			//format(message, sizeof(message), "MINIMODES: You cannot start a minimode more than once every %is or %is after finishing a minimode. (%is to go)", MIN_START_INTERVAL / 1000, MIN_START_AFTER_FINISH / 1000, (mpData[playerid][mpStartDelay] - GetTickCount()) / 1000);
			format(message, sizeof(message), "MINIMODES: You cannot start a minimode more than once every %is. (%is to go)", MIN_START_INTERVAL / 1000, (mpData[playerid][mpStartDelay] - GetTickCount()) / 1000);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
		if (modesWaiting >= MODE_MAX_WAITING) {
			format(message, sizeof(message), "MINIMODES: More than %i modes are already waiting for players.", MODE_MAX_WAITING);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
		
		// Start minimode
		if (modeStartHalfXP) {
			StartMinimode(modeid, drunk ? (MODE_FLAG_DRUNK | MODE_FLAG_HALFXP) : MODE_FLAG_HALFXP);
		} else {
			StartMinimode(modeid, drunk ? MODE_FLAG_DRUNK : 0);
		}
		
		
		if (modes[modeid][mState] == MODE_STATE_WAITING) {
			printf("[FDM] %s loaded %s", pData[playerid][pLoginName], modes[modeid][mName]);
			mpData[playerid][mpStartDelay] = GetTickCount() + MIN_START_INTERVAL;
		} else {
			format(message, sizeof(message), "MINIMODES: Could not start %s.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
		}
	}
	
	/*if (modes[modeid][mState] == MODE_STATE_WAITING) {
		printf("[FDM] %s loaded %s", pData[playerid][pLoginName], modes[modeid][mName]);
		mpData[playerid][mpStartDelay] = GetTickCount() + MIN_START_INTERVAL;
	} else {
		printf("[FDM] %s quick-started %s", pData[playerid][pLoginName], modes[modeid][mName]);
	}*/
	
	return 1;
}

kcmd:astart(playerid, text[]) return cmd_astart(playerid, text, 0);
kcmd:adstart(playerid, text[]) return cmd_astart(playerid, text, 1);
cmd_astart(playerid, text[], drunk) { // Admin start a minimode
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) {
		format(message, sizeof(message), "MINIMODES: You must be at least admin level %i to use /astart.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /astart [minimode id/name]", "MINIMODES: Invalid minimode ID/Name.", text);
	if (modeid == INVALID_MODE_ID) return 1;
	
	if (modes[modeid][mState] == MODE_STATE_PROGRESS) {
		format(message, sizeof(message), "MINIMODES: %s is currently in progress.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	//new oldstate = modes[modeid][mState];
	new flags;
	if (modes[modeid][mState] == MODE_STATE_UNLOADED) {
		if (modeStartHalfXP) {
			flags = drunk ? (MODE_FLAG_DRUNK | MODE_FLAG_HALFXP) : MODE_FLAG_HALFXP;
		} else {
			flags = drunk ? MODE_FLAG_DRUNK : 0;
		}
	} else if (modes[modeid][mState] == MODE_STATE_WAITING) {
		if (drunk) {
			format(message, sizeof(message), "MINIMODES: %s is already waiting for players.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
	}
	
	if (!StartMinimode(modeid, flags)) { // Start minimode
		/*if (oldstate != MODE_STATE_UNLOADED && modes[modeid][mState] == MODE_STATE_UNLOADED) {
			GetPlayerName(playerid, message, sizeof(message));
			format(message, sizeof(message), "%s admin stopped %s", message, modes[modeid][mName]);
			AddAdminLogLine(COLOUR_LOG, message);
			printf("[FDM] %s", message);
		}*/
		if (modes[modeid][mState] == MODE_STATE_WAITING) {
			format(message, sizeof(message), "MINIMODES: %s cannot be started - not enough players are waiting.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
		}
		return 1;
	}
	
	// Minimode started
	GetPlayerName(playerid, message, sizeof(message));
	if (modes[modeid][mState] == MODE_STATE_WAITING) {
		format(message, sizeof(message), "%s admin loaded %s", message, modes[modeid][mName]);
	} else {
		format(message, sizeof(message), "%s admin quick-started %s", message, modes[modeid][mName]);
	}
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	
	//format(message, sizeof(message), "MINIMODES: %s started.", modes[modeid][mName]);
	//SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:astop(playerid, text[]) { // Admin stop a minimode
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MODERATOR) {
		format(message, sizeof(message), "MINIMODES: You must be at least admin level %i to use /astop.", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /astop [minimode id/name]");
		return 1;
	}
	
	new modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /astop [minimode id/name]", "MINIMODES: Invalid minimode ID/Name.", text);
	if (modeid == INVALID_MODE_ID) return 1;
	
	if (!StopMinimode(modeid)) { // Start minimode
		format(message, sizeof(message), "MINIMODES: %s cannot be stopped.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	// Minimode stopped
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s admin stopped %s", message, modes[modeid][mName]);
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	
	//format(message, sizeof(message), "MINIMODES: %s stopped.", modes[modeid][mName]);
	//SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:amodekick(playerid, text[]) { // Admin kick a player from a minimode
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) {
		format(message, sizeof(message), "MODE KICK: You must be at least admin level %i to use /amodekick.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /amodekick [player] (reason)");
		return 1;
	}
	
	new tmpnum,
		pid,
		reason[48],
		adminname[MAX_NAME],
		pname[MAX_NAME],
		modeid;
	
	pid = FindPlayer(split(text, tmpnum, ' '));
	if (pid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "MODE KICK: Invalid player ID/Name.");
		return 1;
	}
	if (pid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "MODE KICK: You cannot kick yourself from minimodes. Use /leave.");
		return 1;
	}
	
	modeid = mpData[pid][mpModeID];
	GetPlayerName(pid, pname, sizeof(pname));
	
	if (modeid == INVALID_MODE_ID) {
		format(message, sizeof(message), "MODE KICK: %s is not in a minimode.", pname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	splitcpy(reason, text, tmpnum, 0);
	
	if (reason[0]) {
		format(message, sizeof(message), "(Kicked: %s)", reason);
		tmpnum = RemovePlayerFromMode(pid, "left", message);
	} else {
		reason = "No Reason";
		tmpnum = RemovePlayerFromMode(pid, "left", "(Kicked)");
	}
	
	if (!tmpnum) {
		SendClientMessage(playerid, COLOUR_ERROR, "MODE KICK: Error - %s cannot be kicked from the minimode.");
		return 1;
	}
	
	// Player kicked
	GetPlayerName(playerid, adminname, sizeof(adminname));
	format(message, sizeof(message), "%s kicked %s from %s", adminname, pname, modes[modeid][mName]);
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	
	if (tmpnum) {
		format(message, sizeof(message), "MINIMODES: %s kicked you from %s. (Reason: %s)", adminname, modes[modeid][mName], reason);
		SendClientMessage(pid, COLOUR_ATTENTION, message);
	}
	
	return 1;
}

kcmd:amodelock(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		format(message, sizeof(message), "MODE LOCK: You must be at least admin level %i to use /amodelock.", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (modesLocked) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "MODE LOCK: Minimodes are already locked.");
		return 1;
	}
	
	modesLocked = 1;
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, "MODE LOCK: All new minimodes are locked from starting.");
	
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s has locked new minimodes from starting", message);
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	return 1;
}
	
kcmd:amodeunlock(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		format(message, sizeof(message), "MODE UNLOCK: You must be at least admin level %i to use /amodelock.", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (!modesLocked) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "MODE UNLOCK: Minimodes are already not locked.");
		return 1;
	}
	
	modesLocked = 0;
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, "MODE UNLOCK: All new minimodes are unlocked from starting.");
	
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s has unlocked new minimodes from starting", message);
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	return 1;
}
