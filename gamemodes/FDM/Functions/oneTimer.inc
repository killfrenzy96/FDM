#include <a_samp>

#if !defined MAX_TIMER
	#define MAX_TIMER 256
#endif

#define TIMER_START_ID -2 - MAX_TIMER


enum timerInfo {
	tEnabled,
	tStartTick
}
new timerData[MAX_TIMER][timerInfo];


OneTimerInit() {
	SetTimer("fdm_CheckTimers", 1, true);
	return 1;
}

stock kEnableTimer(oneTimerID) {
	timerData[oneTimerID][tEnabled] = 1;
	timerData[oneTimerID][tStartTick] = GetTickCount();
	return 1;
}

stock kDisableTimer(oneTimerID) {
	timerData[oneTimerID][tEnabled] = 0;
	return 1;
}

//#define TimerCheckR(%1,%2) if (timerData[%1][tEnabled] && tickCount - timerData[%1][tStartTick] >= %2 && ((timerData[%1][tStartTick] += %2) || true)) // Repeating timer
//#define TimerCheckN(%1,%2) if (timerData[%1][tEnabled] && tickCount - timerData[%1][tStartTick] >= %2 && ((timerData[%1][tEnabled] = 0) || true)) // Non-repeating timer

#define TimerCheckR(%1,%2) if (timerData[%1][tEnabled] && tickCount - timerData[%1][tStartTick] >= %2 && ((timerData[%1][tStartTick] += %2) || CheckCPU(%1,tickCount))) // Repeating timer
#define TimerCheckN(%1,%2) if (timerData[%1][tEnabled] && tickCount - timerData[%1][tStartTick] >= %2 && ((timerData[%1][tEnabled] = 0) || CheckCPU(%1,tickCount))) // Non-repeating timer

CheckCPU(id, &tickCount) {
	new newTick = GetTickCount();
	
	if (newTick - tickCount > 1) {
		new msg[128];
		format(msg, sizeof(msg), "[FDM] Warning: High CPU usage in timer id %i (%i ms)", id, newTick - tickCount);
		LoopPlayers(playerid) {
			if (IsPlayerAdmin(playerid)) SendClientMessage(playerid, 0xFF0000AA, msg);
		}
		print(msg);
		tickCount = GetTickCount();
	} else {
		tickCount = newTick;
	}
	
	return 1;
}

forward fdm_CheckTimers();
public fdm_CheckTimers() {
	new tickCount = GetTickCount();
	
	TimerCheckN(0, 10000) fdm_anticheatEnableSync();
	TimerCheckR(1, anticheatSyncInterval) fdm_anticheatSync();
	TimerCheckR(2, 250) fdm_CombatUpdate();
	TimerCheckR(3, 100) fdm_CheckpointCheck();
	TimerCheckR(4, 100) fdm_GameTextUpdate();
	TimerCheckR(5, 500) fdm_TimerUpdate();
	TimerCheckR(6, 200) fdm_GodTimerCheck();
	TimerCheckR(7, 1000) fdm_dmffa_checkDM();
	TimerCheckR(8, 1000) fdm_dmteam_checkDM();
	TimerCheckR(9, 1000) fdm_derby_checkDerbys();
	TimerCheckR(10, 10) fdm_derby_PickupCheck();
	TimerCheckR(11, 1000) fdm_race_checkRaces();
	TimerCheckR(12, 200) fdm_race_raceTimes();
	TimerCheckR(13, 10) fdm_race_PickupCheck();
	TimerCheckR(14, 1000) fdm_CheckMinimodes();
	TimerCheckR(15, 2050) fdm_playersSave();
	TimerCheckR(16, SIDECHAT_UPDATE_DELAY) fdm_SidechatUpdate();
	TimerCheckR(17, 10) fdm_SkinsUpdate();
	TimerCheckR(18, 750) fdm_HealTimer();
	TimerCheckR(19, 750) fdm_RepairTimer();
	TimerCheckR(20, 5000) fdm_TimeSync();
	TimerCheckR(21, 8000) fdm_XPZoneProcess();
	TimerCheckR(22, 250) fdm_dmffa_checkDMFast();
	TimerCheckR(23, 250) fdm_dmteam_checkDMFast();
	TimerCheckR(24, 250) fdm_stealthing_Check();
	TimerCheckR(25, 1000) fdm_ctf_checkDM();
	TimerCheckR(26, 250) fdm_ctf_checkDMFast();
	TimerCheckR(27, 250) fdm_PassengerGlitchCheck();
	
	return 1;
}
