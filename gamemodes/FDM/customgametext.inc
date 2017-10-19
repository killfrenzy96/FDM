#include <a_samp>

// This script makes use of a text draw for game text

enum gameTextPlayerInfo {
	Text:gtpTextDraw,
	gtpEndTick,
	Float:gtpX,
	Float:gtpY
}
new gtpData[MAX_PLAYERS][gameTextPlayerInfo];


gametextInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		gtpData[i][gtpTextDraw] = Text:INVALID_TEXT_DRAW;
	}
	kEnableTimer(4); // SetTimer("fdm_GameTextUpdate", 10, true);
	return 1;
}

gametext_OnPlayerDisconnect(playerid) {
	if (gtpData[playerid][gtpTextDraw] != Text:INVALID_TEXT_DRAW) {
		TextDrawDestroy(gtpData[playerid][gtpTextDraw]);
		gtpData[playerid][gtpTextDraw] = Text:INVALID_TEXT_DRAW;
	}
	return 1;
}

kGameTextForPlayer(playerid, string[], time, Float:X, Float:Y) {
	if (gtpData[playerid][gtpTextDraw] != Text:INVALID_TEXT_DRAW && (gtpData[playerid][gtpX] != X || gtpData[playerid][gtpY] != Y)) {
		TextDrawDestroy(gtpData[playerid][gtpTextDraw]);
		gtpData[playerid][gtpTextDraw] = Text:INVALID_TEXT_DRAW;
	}
	
	if (gtpData[playerid][gtpTextDraw] == Text:INVALID_TEXT_DRAW) {
		gtpData[playerid][gtpTextDraw] = TextDrawCreate(X, Y, string);
		gtpData[playerid][gtpX] = X;
		gtpData[playerid][gtpY] = Y;
		TextDrawAlignment(gtpData[playerid][gtpTextDraw], 2);
		TextDrawShowForPlayer(playerid, gtpData[playerid][gtpTextDraw]);
	} else {
		TextDrawSetString(gtpData[playerid][gtpTextDraw], string);
	}
	gtpData[playerid][gtpEndTick] = GetTickCount() + time;
	
	return 1;
}

kGameTextHide(playerid) {
	if (gtpData[playerid][gtpTextDraw] != Text:INVALID_TEXT_DRAW) {
		TextDrawDestroy(gtpData[playerid][gtpTextDraw]);
		gtpData[playerid][gtpTextDraw] = Text:INVALID_TEXT_DRAW;
	}
	return 1;
}

forward fdm_GameTextUpdate();
public fdm_GameTextUpdate() {
	new tick = GetTickCount();
	
	LoopPlayers(playerid) {
		if (gtpData[playerid][gtpTextDraw] != Text:INVALID_TEXT_DRAW && tick >= gtpData[playerid][gtpEndTick]) {
			TextDrawDestroy(gtpData[playerid][gtpTextDraw]);
			gtpData[playerid][gtpTextDraw] = Text:INVALID_TEXT_DRAW;
		}
	}
	return 1;
}
