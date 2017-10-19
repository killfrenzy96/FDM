/*

By KillFrenzy.

Fully customized skin spawning.

*/

#include <a_samp>

#define SELECTION_INTERVAL 250
#define SELECTION_INTERVAL_FAST 100
#define SELECTION_SOUND 1132

/*#define SELECTION_XYZ 2047.0413,-1155.1902,23.5340
#define SELECTION_ROTZ 331.0055
#define SELECTION_CAMERA_XYZ 2050.4949,-1152.0750,23.8171
#define SELECTION_CAMERA_LOOK_XYZ 2048.0413,-1155.1902,23.5340
#define SELECTION_INTERIOR 0*/

#define SELECTION_XYZ -1378.5808,1610.2385,1053.7109
#define SELECTION_ROTZ 275
#define SELECTION_CAMERA_XYZ -1376,1611,1054
#define SELECTION_CAMERA_LOOK_XYZ -1378.5808,1610.2385,1053.7109
#define SELECTION_INTERIOR 14

#define SELECTION_ANIMATION "ON_LOOKERS", "wave_loop", 4.0, 1

enum spawnPosInfo {
	Float:sX,
	Float:sY,
	Float:sZ,
	Float:sRot
}
enum skinPlayerInfo {
	pSelecting,
	pCurrentClass,
	pLastPressKey,
	pLastPressTime,
	pFirstSelect,
	pSpawnPos[spawnPosInfo],
	pClassFound,
	pLastSpawn
}
new skinPlayerData[MAX_PLAYERS][skinPlayerInfo];
// new skinTimer;

new skinList[] = {
	0,1,2,7,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,
	39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,71,72,
	73,75,76,77,78,79,80,81,82,83,84,85,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,
	105,106,107,108,109,110,111,112,113,114,115,116,117,118,120,121,122,123,124,125,126,127,128,129,
	130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,150,151,152,153,154,
	155,156,157,158,159,160,161,162,163,164,165,167,168,169,170,171,172,173,174,175,176,177,178,179,
	180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,
	204,205,206,207,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,
	229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,
	253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,269,270,271,272,274,275,276,277,278,
	279,280,281,282,283,284,285,286,287,288,290,291,292,293,294,295,296,297,298,299
};

SkinsInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		
		SetLastSpawnTick(i);
		ClearSpawnPoint(i);
		
		if (IsPlayerConnected(i)) {
			SetPlayerClassFromSkin(i, GetPlayerSkin(i));
			skinPlayerData[i][pFirstSelect] = 0;
		} else {
			skinPlayerData[i][pFirstSelect] = 1;
		}
	}
	
	kEnableTimer(17); // skinTimer = SetTimer("fdm_SkinsUpdate", 16, true);
	return 1;
}

SkinsExit() {
	LoopPlayers(i) {
		if (skinPlayerData[i][pSelecting]) {
			StopSkinSelection(i);
		}
	}
	kDisableTimer(17); // KillTimer(skinTimer);
	return 1;
}

stock SkinsSetNextSpawnPos(playerid, Float:x, Float:y, Float:z, Float:rot) {
	skinPlayerData[playerid][pSpawnPos][sX] = x;
	skinPlayerData[playerid][pSpawnPos][sY] = y;
	skinPlayerData[playerid][pSpawnPos][sZ] = z;
	skinPlayerData[playerid][pSpawnPos][sRot] = rot;
	kSetSpawnInfo(playerid, x, y, z, rot);
	return 1;
}

stock SkinsIsFirstTimeSelection(playerid) {
	return skinPlayerData[playerid][pFirstSelect];
}

SetPlayerClassFromSkin(playerid, playerskin) {
	//if (!IsPlayerSpawned(playerid)) return 0;
	
	for (new p; p < sizeof(skinList); p++) {
		if (skinList[p] == playerskin) {
			skinPlayerData[playerid][pCurrentClass] = p;
			skinPlayerData[playerid][pClassFound] = 1;
			
			kSetPlayerSkin(playerid, playerskin);
			
			return 1;
		}
	}
	skinPlayerData[playerid][pCurrentClass] = 0;
	skinPlayerData[playerid][pClassFound] = 0;
	return 0;
}

/*forward checkKeys();
public checkKeys() {
	new keys;
	new updown;
	new leftright;
	
	LoopPlayers(i) {
		
		if (!skinPlayerData[i][pSelecting]) continue;
		
		GetPlayerKeys(i, keys, updown, leftright);
		
		
		// Reduce selection speed
		if (skinPlayerData[i][pLastPressKey] == leftright) {
			if (GetTickCount() - skinPlayerData[i][pLastPressTime] < SELECTION_INTERVAL) {
				continue;
			}
		}
		skinPlayerData[i][pLastPressKey] = leftright;
		skinPlayerData[i][pLastPressTime] = GetTickCount();
		
		// Next skin / previous skin in skin selection
		
		if (leftright == KEY_LEFT) {
			SkinPrevious(i);
			continue;
		}
		
		if (leftright == KEY_RIGHT) {
			SkinNext(i);
			continue;
		}
	}
}*/

forward fdm_SkinsUpdate();
public fdm_SkinsUpdate() {
	LoopPlayers(i) {
		if (skinPlayerData[i][pSelecting]) {
			skin_UpdatePlayer(i);
		}
	}
	return 1;
}

//skin_OnPlayerUpdate(playerid) {
skin_UpdatePlayer(playerid) {
	//if (!skinPlayerData[playerid][pSelecting]) return 1;
	
	new keys,
		updown,
		leftright;
	
	GetPlayerKeys(playerid, keys, updown, leftright);
	
	// Control selection speed
	if (skinPlayerData[playerid][pLastPressKey] == leftright) {
		new interval = SELECTION_INTERVAL;
		if (keys & KEY_SPRINT) interval = SELECTION_INTERVAL_FAST;
		if (GetTickCount() - skinPlayerData[playerid][pLastPressTime] < interval) {
			return 1;
		}
	}
	
	skinPlayerData[playerid][pLastPressKey] = leftright;
	skinPlayerData[playerid][pLastPressTime] = GetTickCount();
	
	// Next skin / previous skin in skin selection
	if (leftright == KEY_LEFT) {
		SkinPrevious(playerid);
	} else if (leftright == KEY_RIGHT) {
		SkinNext(playerid);
	}
	
	return 1;
}

skin_OnPlayerKeyStateChange(playerid, newkeys) {
	if (!skinPlayerData[playerid][pSelecting]) return 1;
	
	if ((newkeys & KEY_JUMP) || (newkeys & KEY_SECONDARY_ATTACK)) { // Default: Shift or Enter
		StopSkinSelection(playerid);
		return 1;
	}
	return 1;
}

skin_OnPlayerRequestClass(playerid) {
	// Setup for skin selection
	if (!skinPlayerData[playerid][pClassFound]) {
		// No autologin
		StartSkinSelection(playerid);
		GetNewSpawnPoint(playerid);
	} else {
		if (skinPlayerData[playerid][pFirstSelect]) {
			// Autologin
			StopSkinSelection(playerid);
		} else {
			if (IsPlayerInMinimode(playerid)) {
				// No class selection
				StopSkinSelection(playerid);
				SendClientMessage(playerid, COLOUR_ERROR, "CLASS SELECTION: You are not allowed to go to class selection during minimodes.");
			} else {
				if (GetTickCount() - skinPlayerData[playerid][pLastSpawn] < 2500) {
					// Bug fix
					kSetPlayerSkin(playerid, skinList[skinPlayerData[playerid][pCurrentClass]]);
				} else {
					// Player chose selection
					StartSkinSelection(playerid);
					GetNewSpawnPoint(playerid);
				}
			}
		}
	}
	return 1;
}

skin_OnPlayerSpawn(playerid) {
	if (skinPlayerData[playerid][pSelecting]) {
		if (skinPlayerData[playerid][pFirstSelect]) return 1;
		EnableGodmode(playerid);
		/*skinPlayerData[playerid][pSelecting] = 0;
		SetPlayerVirtualWorld(playerid, 0);
		TogglePlayerControllable(playerid, true);*/
		
		SetTimerEx("fdmApplySkinActions", 50, false, "i", playerid);
		SetTimerEx("fdmApplyCameraPos", 200, false, "i", playerid);
		SetTimerEx("fdmApplyCameraPos", 500, false, "i", playerid);
		SetTimerEx("fdmApplyCameraPos", 1000, false, "i", playerid);
		return 1;
	} else {
		SetCameraBehindPlayer(playerid);
	}
	
	if (!IsPlayerNPC(playerid)) {
		SetSpawnInfo(playerid, NO_TEAM, skinList[skinPlayerData[playerid][pCurrentClass]],
			skinPlayerData[playerid][pSpawnPos][sX], skinPlayerData[playerid][pSpawnPos][sY], skinPlayerData[playerid][pSpawnPos][sZ], skinPlayerData[playerid][pSpawnPos][sRot], 0, 0, 0, 0, 0, 0);
	}
	SetLastSpawnTick(playerid);
	
	//SetTimerEx("fdmApplySpawnPos", 50, false,
	//	"iffff", playerid, skinPlayerData[playerid][pSpawnPos][sX], skinPlayerData[playerid][pSpawnPos][sY], skinPlayerData[playerid][pSpawnPos][sZ], skinPlayerData[playerid][pSpawnPos][sRot]);
	return 0;
}

/*skin_OnPlayerConnect(playerid) {
	GetNewSpawnPointIfInvalid(playerid);
	return 1;
}*/

skin_OnPlayerDisconnect(playerid) {
	SkinResetPlayerData(playerid);
	return 1;
}

skin_OnPlayerRequestSpawn(playerid) {
	GetNewSpawnPointIfInvalid(playerid);
	return 1;
}

ClearSpawnPoint(playerid) {
	skinPlayerData[playerid][pSpawnPos][sX] = 0.0;
	skinPlayerData[playerid][pSpawnPos][sY] = 0.0;
	skinPlayerData[playerid][pSpawnPos][sZ] = 5.0;
	skinPlayerData[playerid][pSpawnPos][sRot] = 0.0;
	return 1;
}

GetNewSpawnPointIfInvalid(playerid) {
	if (skinPlayerData[playerid][pSpawnPos][sX] != 0.0) return 1;
	if (skinPlayerData[playerid][pSpawnPos][sY] != 0.0) return 1;
	if (skinPlayerData[playerid][pSpawnPos][sZ] != 5.0) return 1;
	if (skinPlayerData[playerid][pSpawnPos][sRot] != 0.0) return 1;
	GetNewSpawnPoint(playerid);
	return 1;
}

skin_OnPlayerLogin(playerid, playerskin) {
	if (!SetPlayerClassFromSkin(playerid, playerskin)) return 0;
	if (!skinPlayerData[playerid][pSelecting]) return 1;
	StopSkinSelection(playerid);
	return 1;
}

skin_OnPlayerRegister(playerid) {
	if (!IsPlayerSpawned(playerid)) {
		GetNewSpawnPoint(playerid);
	}
}

SkinResetPlayerData(playerid) {
	new tmpdata[skinPlayerInfo];
	skinPlayerData[playerid] = tmpdata;
	skinPlayerData[playerid][pFirstSelect] = 1;
	SkinsSetNextSpawnPos(playerid, 0.0, 0.0, 5.0, 0.0);
	return 1;
}

StartSkinSelection(playerid) {
	if (IsPlayerNPC(playerid)) return 0;
	
	skinPlayerData[playerid][pSelecting] = 1;
	if (!OnPlayerStartSelection(playerid)) {
		skinPlayerData[playerid][pSelecting] = 0;
		return 1;
	}
	
	SetSpawnInfo(playerid, NO_TEAM, skinList[skinPlayerData[playerid][pCurrentClass]], SELECTION_XYZ - 1, SELECTION_ROTZ, 0, 0, 0, 0, 0, 0);
	
	if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) {
		SpawnPlayer(playerid);
	}
	
	if (!skinPlayerData[playerid][pSelecting]) return 0;
	
	EnableGodmode(playerid);
	SetPlayerInterior(playerid, SELECTION_INTERIOR);
	
	SetTimerEx("fdmApplySkinActions", 50, false, "i", playerid);
	SetTimerEx("fdmApplyCameraPos", 200, false, "i", playerid);
	SetTimerEx("fdmApplyCameraPos", 500, false, "i", playerid);
	SetTimerEx("fdmApplyCameraPos", 1000, false, "i", playerid);
	return 1;
}

StopSkinSelection(playerid) {
	if (IsPlayerNPC(playerid)) return 0;
	
	skinPlayerData[playerid][pSelecting] = 0;
	if (!OnPlayerStopSelection(playerid)) {
		skinPlayerData[playerid][pSelecting] = 1;
		return 1;
	}
	
	skinPlayerData[playerid][pFirstSelect] = 0;
	skinPlayerData[playerid][pClassFound] = 1;
	
	SetSpawnInfo(playerid, NO_TEAM, skinList[skinPlayerData[playerid][pCurrentClass]],
		skinPlayerData[playerid][pSpawnPos][sX], skinPlayerData[playerid][pSpawnPos][sY], skinPlayerData[playerid][pSpawnPos][sZ], skinPlayerData[playerid][pSpawnPos][sRot], 0, 0, 0, 0, 0, 0);
	
	skin_OnPlayerRequestSpawn(playerid);
	//SetPlayerHealth(playerid, 100.0);
	
	new playerState = GetPlayerState(playerid);
	if (!playerState || playerState == PLAYER_STATE_WASTED) {
		SpawnPlayer(playerid);
	} else {
		OnPlayerSpawn(playerid);
	}
	
	if (skinPlayerData[playerid][pSelecting]) return 0;
	
	SetLastSpawnTick(playerid);
	SetCameraBehindPlayer(playerid);
	
	SetPlayerPos(playerid, skinPlayerData[playerid][pSpawnPos][sX], skinPlayerData[playerid][pSpawnPos][sY], skinPlayerData[playerid][pSpawnPos][sZ]);
	SetPlayerFacingAngle(playerid, skinPlayerData[playerid][pSpawnPos][sRot]);
	SetCameraBehindPlayer(playerid);
	
	if (!IsPlayerInMinimode(playerid)) {
		SetPlayerVirtualWorld(playerid, 0);
		SetPlayerInterior(playerid, 0);
	}
	TogglePlayerControllable(playerid, true);
	
	SetTimerEx("fdmApplySpawnPos", 50, false, "iffff",
		playerid,
		skinPlayerData[playerid][pSpawnPos][sX],
		skinPlayerData[playerid][pSpawnPos][sY],
		skinPlayerData[playerid][pSpawnPos][sZ],
		skinPlayerData[playerid][pSpawnPos][sRot]);
	
	return 1;
}

forward fdmApplySpawnPos(playerid, Float:x, Float:y, Float:z, Float:rot);
public fdmApplySpawnPos(playerid, Float:x, Float:y, Float:z, Float:rot) {
	SetPlayerPos(playerid, x, y, z);
	SetPlayerFacingAngle(playerid, rot);
	SetCameraBehindPlayer(playerid);
	StartGodSpawning(playerid);
	return 1;
}

SkinPrevious(playerid) {
	skinPlayerData[playerid][pCurrentClass]--;
	if (skinPlayerData[playerid][pCurrentClass] < 0) {
		skinPlayerData[playerid][pCurrentClass] = sizeof(skinList) - 1;
	}
	kSetPlayerSkin(playerid, skinList[skinPlayerData[playerid][pCurrentClass]]);
	fdmApplySkinActions(playerid);
	return 1;
}

SkinNext(playerid) {
	skinPlayerData[playerid][pCurrentClass]++;
	if (skinPlayerData[playerid][pCurrentClass] > sizeof(skinList) - 1) {
		skinPlayerData[playerid][pCurrentClass] = 0;
	}
	kSetPlayerSkin(playerid, skinList[skinPlayerData[playerid][pCurrentClass]]);
	fdmApplySkinActions(playerid);
	return 1;
}

forward fdmApplySkinActions(playerid);
public fdmApplySkinActions(playerid) {
	if (!skinPlayerData[playerid][pSelecting]) return 0;
	
	SetPlayerVirtualWorld(playerid, playerid + 1);
	SetPlayerInterior(playerid, SELECTION_INTERIOR);
	TogglePlayerControllable(playerid, false);
	
	SetPlayerPos(playerid, SELECTION_XYZ);
	SetPlayerFacingAngle(playerid, SELECTION_ROTZ);
	
	SetPlayerCameraPos(playerid, SELECTION_CAMERA_XYZ);
	SetPlayerCameraLookAt(playerid, SELECTION_CAMERA_LOOK_XYZ);
	
	PlayerPlaySound(playerid, SELECTION_SOUND, SELECTION_CAMERA_XYZ);
	ApplyAnimation(playerid, SELECTION_ANIMATION, 0, 0, 0, 0);
	return 1;
}

forward fdmApplyCameraPos(playerid);
public fdmApplyCameraPos(playerid) {
	if (!skinPlayerData[playerid][pSelecting]) return 0;
	
	SetPlayerCameraPos(playerid, SELECTION_CAMERA_XYZ);
	SetPlayerCameraLookAt(playerid, SELECTION_CAMERA_LOOK_XYZ);
	ApplyAnimation(playerid, SELECTION_ANIMATION, 0, 0, 0, 0);
	
	return 1;
}

stock kSetSpawnInfo(playerid, Float:X, Float:Y, Float:Z, Float:RotZ) {
	if (!IsPlayerNPC(playerid)) {
		return SetSpawnInfo(playerid, NO_TEAM, skinList[skinPlayerData[playerid][pCurrentClass]], X, Y, Z, RotZ, 0, 0, 0, 0, 0, 0);
	}
	return 0;
}

/*stock kSpawnPlayer(playerid) {
	if ((skinPlayerData[playerid][pSpawnPos][sX] == 0.0) &&
		(skinPlayerData[playerid][pSpawnPos][sY] == 0.0) &&
		(skinPlayerData[playerid][pSpawnPos][sZ] == 5.0) &&
		(skinPlayerData[playerid][pSpawnPos][sRot] == 0.0)) {
		
		GetNewSpawnPoint(playerid);
	}
	
	if (skinPlayerData[playerid][pSelecting]) {
		StopSkinSelection(playerid);
	} else {
		SpawnPlayer(playerid);
		SetLastSpawnTick(playerid);
	}
	return 1;
}*/

/*stock kSpawnPlayer(playerid, skinid, Float:X, Float:Y, Float:Z, Float:RotZ) {
	SetSpawnInfo(playerid, NO_TEAM, skinid, X, Y, Z, RotZ, 0, 0, 0, 0, 0, 0);
	SpawnPlayer(playerid);
	SetLastSpawnTick(playerid);
	SetPlayerClassFromSkin(playerid);
	return 1;
}*/

stock SetLastSpawnTick(playerid) {
	skinPlayerData[playerid][pLastSpawn] = GetTickCount();
}

stock kSetPlayerSkin(playerid, skinid) {
	//if (!IsPlayerConnected(playerid)) return 0;
	
	if (GetPlayerSkin(playerid) == skinid) return 1;
	
	new playerstate = GetPlayerState(playerid);
	
	if ((playerstate >= PLAYER_STATE_ONFOOT && playerstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || playerstate == PLAYER_STATE_SPAWNED) {
		
		if (playerstate == PLAYER_STATE_PASSENGER) ClearAnimations(playerid);
		
		SetPlayerSkin(playerid, skinid);
		
		if (playerstate == PLAYER_STATE_ONFOOT) ClearAnimations(playerid);
		
	} else {
		if (skinPlayerData[playerid][pSelecting]) {
			kSetSpawnInfo(playerid,
				skinPlayerData[playerid][pSpawnPos][sX],
				skinPlayerData[playerid][pSpawnPos][sY],
				skinPlayerData[playerid][pSpawnPos][sZ],
				skinPlayerData[playerid][pSpawnPos][sRot]);
		} else {
			kSetSpawnInfo(playerid, SELECTION_XYZ, SELECTION_ROTZ);
		}
	}
	
	return 1;
}

kcmd:askinselect(playerid, text[]) {
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) {
		new message[MAX_INPUT];
		format(message, sizeof(message), "SKIN SELECT: You must be at least admin level %i to use /amodekick.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /askinselect [player]");
		return 1;
	}
	
	new pid = FindPlayer(text);
	if (pid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SKIN SELECT: Invalid player ID/Name.");
		return 1;
	}
	
	new playername[MAX_NAME],
		message[MAX_INPUT];
	GetPlayerName(pid, playername, sizeof(playername));
	
	if (skinPlayerData[pid][pSelecting]) {
		format(message, sizeof(message), "SKIN SELECT: %s is already in skin selection.", playername);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (IsPlayerInMinimode(pid)) {
		format(message, sizeof(message), "SKIN SELECT: %s is in a minimode.", playername);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (!IsPlayerSpawned(pid)) {
		format(message, sizeof(message), "SKIN SELECT: %s must be spawned.", playername);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	// Put player into skin selection
	StartSkinSelection(pid);
	
	new adminname[MAX_NAME];
	GetPlayerName(playerid, adminname, sizeof(adminname));
	
	// Messages
	if (playerid == pid) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SKIN SELECT: You have forced yourself into skin selection.");
		
		format(message, sizeof(message), "%s has forced themself into skin selection", adminname, playername);
	} else {
		format(message, sizeof(message), "SKIN SELECT: %s has forced you into skin selection.", adminname);
		SendClientMessage(pid, COLOUR_SUCCESSFUL, message);
		format(message, sizeof(message), "SKIN SELECT: You have forced %s into skin selection.", playername);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has forced %s into skin selection", adminname, playername);
	}
	
	// Log it
	AddAdminLogLine(COLOUR_LOG, message);
	
	printf("[FDM] %s", message);
	
	return 1;
}

//#define SpawnPlayer kSpawnPlayer