#include <a_samp>

//-----------------------------------------------------------------------------------------------------
// PAUSING FUNCTIONS
//-----------------------------------------------------------------------------------------------------

new miscPlayerLastUpdate[MAX_PLAYERS];
new miscPlayerIsPaused[MAX_PLAYERS];
// new miscPauseTimer;

stock miscPauseEventsInit() {
	kEnableTimer(5); // miscPauseTimer = SetTimer("fdm_TimerUpdate", 100, true);
	LoopPlayers(playerid) {
		miscPlayerLastUpdate[playerid] = GetTickCount();
	}
}
stock miscPauseEventsExit() {
	kDisableTimer(5); // KillTimer(miscPauseTimer);
}

stock miscPause_OnPlayerConnect(playerid) {
	miscPlayerLastUpdate[playerid] = 0;
}

forward fdm_TimerUpdate();
public fdm_TimerUpdate() {
	LoopPlayers(playerid) {
		new tick = GetTickCount();
		
		if (!miscPlayerLastUpdate[playerid] || tick - miscPlayerLastUpdate[playerid] < 1000) {
			
			if (miscPlayerIsPaused[playerid]) OnPlayerUnPause(playerid);
			miscPlayerIsPaused[playerid] = 0;
		} else {
			//if (!miscPlayerIsPaused[playerid]) OnPlayerPause(playerid);
			miscPlayerIsPaused[playerid] = 1;
		}
	}
	return 1;
}

stock misc_OnPlayerUpdate(playerid) {
	miscPlayerLastUpdate[playerid] = GetTickCount();
	return 1;
}

stock misc_OnPlayerSpawn(playerid) {
	miscPlayerLastUpdate[playerid] = GetTickCount();
	return 1;
}

stock misc_OnPlayerDeath(playerid) {
	miscPlayerLastUpdate[playerid] = GetTickCount();
	return 1;
}

stock IsPlayerPaused(playerid, sensitivy = 500) {
	new tick = GetTickCount();
	
	if (!miscPlayerLastUpdate[playerid] || tick - miscPlayerLastUpdate[playerid] < sensitivy) {
		return 0;
	}
	return 1;
}

stock GetPlayerLastUpdateTime(playerid) {
	return miscPlayerLastUpdate[playerid];
}
