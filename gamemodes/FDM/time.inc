#include <a_samp>

#define DEFAULT_WEATHER_ID 1

new tWorldTimeOffset;
new tPlayerTimeOffset[MAX_PLAYERS];


TimeInit() {
	new gamehours, gameminutes;
	GetWorldTime(gamehours, gameminutes);
	
	LoopPlayers(i) {
		TogglePlayerClock(i, 1);
		SetPlayerTime(i, gamehours, gameminutes);
	}
	
	kEnableTimer(20); // SetTimer("fdm_TimeSync", 5000, true);
	return 1;
}

time_OnPlayerConnect(playerid) {
	tPlayerTimeOffset[playerid] = 0;
	TogglePlayerClock(playerid, 1);
	SyncPlayerTime(playerid);
	return 1;
}

time_OnPlayerSpawn(playerid) {
	SyncPlayerTime(playerid);
	
	// Stupid SAMP
	SetTimerEx("fdm_TimeSyncPlayer", GetPlayerPing(playerid) + 100, false, "i", playerid);
	return 1;
}

time_OnPlayerDeath(playerid) {
	SyncPlayerTime(playerid);
	
	// Stupid SAMP
	SetTimerEx("fdm_TimeSyncPlayer", GetPlayerPing(playerid) + 100, false, "i", playerid);
	return 1;
}

time_OnPlayerUnPause(playerid) {
	SyncPlayerTime(playerid);
	return 1;
}

SyncPlayerTime(playerid) {
	new gamehours, gameminutes;
	kGetPlayerTime(playerid, gamehours, gameminutes);
	SetPlayerTime(playerid, gamehours, gameminutes);
	return 1;
}

forward fdm_TimeSync();
public fdm_TimeSync() {
	LoopPlayers(i) {
		SyncPlayerTime(i);
	}
}

forward fdm_TimeSyncPlayer(playerid);
public fdm_TimeSyncPlayer(playerid) {
	if (!IsPlayerConnected(playerid)) return 1;
	SyncPlayerTime(playerid);
	return 1;
}

GetWorldTime(&gamehours, &gameminutes) {
	new realseconds;
	realseconds = gettime();
	
	gamehours = realseconds / 60;
	gamehours %= 24;
	gameminutes = realseconds % 60;
	
	return 1;
}

stock GetWorldTimeWithOffset(&gamehours, &gameminutes) {
	new realseconds;
	
	realseconds = gettime() + tWorldTimeOffset;
	
	gamehours = realseconds / 60;
	gamehours %= 24;
	gameminutes = realseconds % 60;
	
	return 1;
}

stock kSetWorldTime(hour) {
	new oldgamehours, oldgameminutes;
	GetWorldTime(oldgamehours, oldgameminutes);
	tWorldTimeOffset = -((oldgamehours * 60) + oldgameminutes) + (hour * 60);
	
	LoopPlayers(i) {
		SyncPlayerTime(i);
	}
	
	return 1;
}

stock kSetPlayerTime(playerid, hour, minute) {
	new oldgamehours, oldgameminutes;
	GetWorldTime(oldgamehours, oldgameminutes);
	tPlayerTimeOffset[playerid] = -((oldgamehours * 60) + oldgameminutes) + ((hour * 60) + minute);
	
	SyncPlayerTime(playerid);
	return 0;
}

stock kGetPlayerTime(playerid, &gamehours, &gameminutes) {
	new realseconds;
	
	if (tPlayerTimeOffset[playerid]) {
		realseconds = gettime() + tPlayerTimeOffset[playerid];
	} else {
		realseconds = gettime() + tWorldTimeOffset;
	}
	
	gamehours = realseconds / 60;
	gamehours %= 24;
	gameminutes = realseconds % 60;
	
	return 1;
}

stock kResetPlayerTime(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	tPlayerTimeOffset[playerid] = 0;
	SyncPlayerTime(playerid);
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// REDEFINES
//-----------------------------------------------------------------------------------------------------

#define SetWorldTime kSetWorldTime
#define SetPlayerTime kSetPlayerTime
#define GetPlayerTime kGetPlayerTime
#define ResetPlayerTime kResetPlayerTime