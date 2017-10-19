#include <a_samp>

#define COLOUR_TARGET 0xFF8800FF

enum playerTargetData {
	targetPlayer,
	PlayerText3D:target3dText
}
new ptData[MAX_PLAYERS][playerTargetData];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

TargetInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		ResetPlayerTargetData(i);
	}
	return 1;
}

TargetExit() {
	LoopPlayers(i) {
		PlayerStopTarget(i);
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------------------------------

target_OnPlayerDisconnect(playerid) {
	PlayerStopTarget(playerid);
	LoopPlayers(i) {
		if (ptData[i][targetPlayer] == playerid) {
			PlayerStopTarget(playerid);
		}
	}
	return 1;
}

target_OnPlayerSpawn(playerid) {
	if (ptData[playerid][targetPlayer] != INVALID_PLAYER_ID && !IsPlayerAbilityAllowed(playerid, SP_ABILITY_TARGETPLAYER)) {
		PlayerStopTarget(playerid);
	}
}

//-----------------------------------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------------------------------

ResetPlayerTargetData(playerid) {
	ptData[playerid][targetPlayer] = INVALID_PLAYER_ID;
	ptData[playerid][target3dText] = PlayerText3D:INVALID_3DTEXT_ID;
	return 1;
}

PlayerTargetPlayer(showid, targetid) {
	PlayerStopTarget(showid);
	ptData[showid][target3dText] = CreatePlayer3DTextLabel(showid, ">> X <<", COLOUR_TARGET, 0.0, 0.0, 0.0, 100000.0, targetid);
	ptData[showid][targetPlayer] = targetid;
	return 1;
}

PlayerStopTarget(playerid) {
	if (ptData[playerid][targetPlayer] == INVALID_PLAYER_ID) return 1;
	
	if (ptData[playerid][target3dText] != PlayerText3D:INVALID_3DTEXT_ID) {
		DeletePlayer3DTextLabel(playerid, ptData[playerid][target3dText]);
	}
	
	ResetPlayerTargetData(playerid);
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:tg(playerid, text[]) return kcmd_target(playerid, text);
kcmd:target(playerid, text[]) {
	if (!SpecialtyAllowedCheck(playerid, SP_ABILITY_TARGETPLAYER, "target another player")) return 1;
	
	if (isnull(text)) {
		if (ptData[playerid][targetPlayer] == INVALID_PLAYER_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid player ID/Name.");
		} else {
			new msg[MAX_INPUT];
			format(msg, sizeof(msg), "SPECIALTIES: You have stopped targeting %s.", ReturnPlayerName(ptData[playerid][targetPlayer]));
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
			PlayerStopTarget(playerid);
		}
		return 1;
	}
	
	new targetid = FindPlayer(text);
	if (targetid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid player ID/Name.");
		return 1;
	}
	
	if (targetid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You cannot target yourself.");
		return 1;
	}
	
	if (ptData[playerid][targetPlayer] == targetid) {
		new msg[MAX_INPUT];
		format(msg, sizeof(msg), "SPECIALTIES: You are already targeting %s.", ReturnPlayerName(targetid));
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		return 1;
	}
	
	PlayerTargetPlayer(playerid, targetid);
	
	new msg[MAX_INPUT];
	format(msg, sizeof(msg), "SPECIALTIES: You are now targeting %s.", ReturnPlayerName(targetid));
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	return 1;
}
