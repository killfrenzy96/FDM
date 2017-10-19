#include <a_samp>

#define DEFAULT_WEATHER_ID 1
#define INVALID_WEATHER_ID 0x80000000

new tWorldWeather;
new tPlayerWeather[MAX_PLAYERS];


WeatherInit() {
	SetWeather(DEFAULT_WEATHER_ID);
	for (new i; i < MAX_PLAYERS; i++) {
		tPlayerWeather[i] = INVALID_WEATHER_ID;
	}
	return 1;
}

weather_OnPlayerConnect(playerid) {
	SyncWeather(playerid);
	return 1;
}

weather_OnPlayerDisconnect(playerid) {
	tPlayerWeather[playerid] = INVALID_WEATHER_ID;
	return 1;
}

weather_OnPlayerSpawn(playerid) {
	SyncWeather(playerid);
	return 1;
}

SyncWeather(playerid) {
	TogglePlayerClock(playerid, 0);
	if (tPlayerWeather[playerid] == INVALID_WEATHER_ID) {
		SetPlayerWeather(playerid, tWorldWeather);
	} else {
		SetPlayerWeather(playerid, tPlayerWeather[playerid]);
	}
	TogglePlayerClock(playerid, 1);
	return 1;
}

stock kGetWeather() {
	return tWorldWeather;
}

stock kSetWeather(weatherid) {
	LoopPlayers(playerid) {
		if (tPlayerWeather[playerid] == INVALID_WEATHER_ID) {
			TogglePlayerClock(playerid, 0);
			SetPlayerWeather(playerid, weatherid);
			TogglePlayerClock(playerid, 1);
		}
	}
	tWorldWeather = weatherid;
	return 1;
}

stock kGetPlayerWeather(playerid) {
	if (tPlayerWeather[playerid] == INVALID_WEATHER_ID) {
		return tWorldWeather;
	} else {
		return tPlayerWeather[playerid];
	}
}

stock kSetPlayerWeather(playerid, weatherid) {
	if (IsPlayerConnected(playerid)) {
		TogglePlayerClock(playerid, 0);
		SetPlayerWeather(playerid, weatherid);
		TogglePlayerClock(playerid, 1);
		tPlayerWeather[playerid] = weatherid;
		return 1;
	} else {
		return 0;
	}
}

stock kResetPlayerWeather(playerid) {
	TogglePlayerClock(playerid, 0);
	SetPlayerWeather(playerid, tWorldWeather);
	TogglePlayerClock(playerid, 1);
	tPlayerWeather[playerid] = INVALID_WEATHER_ID;
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// REDEFINES
//-----------------------------------------------------------------------------------------------------

#define GetWeather kGetWeather
#define SetWeather kSetWeather
#define GetPlayerWeather kGetPlayerWeather
#define SetPlayerWeather kSetPlayerWeather
#define ResetPlayerWeather kResetPlayerWeather

