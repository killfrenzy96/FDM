#include <a_samp>

#define COLOUR_SIDECHAT 0x1188DDAA
#define SIDECHAT_UPDATE_DELAY 150
#define SIDECHAT_SHOW_TIME 10

enum sidechatTextInfo {
	scExists,
	Text:scText,
	scString[MAX_INPUT],
	scColour,
	scTick
}
enum sidechatQueInfo {
	scqString[MAX_INPUT],
	scqColour
}
enum sidechatPlayerInfo {
	scpLines,
	scpShowLines[SIDECHAT_MAX_LINES],
	scpShowTime
}
new scData[SIDECHAT_MAX_LINES][sidechatTextInfo];
new scQue[SIDECHAT_MAX_LINES][sidechatQueInfo];
new scPData[MAX_PLAYERS][sidechatPlayerInfo];
new scQueIdx;
// new scTimer;

SidechatInit() {
	// Create the text draws
	new Float:ty = 410.0;
	
	for (new i; i < SIDECHAT_MAX_LINES; i++) {
		if (scData[i][scExists]) continue;
		scData[i][scString][0] = ' ';
		scData[i][scText] = TextDrawCreate(150.0, ty, scData[i][scString]);
		TextDrawLetterSize(scData[i][scText], 0.265, 0.85);
		TextDrawColor(scData[i][scText], COLOUR_SIDECHAT);
		TextDrawBackgroundColor(scData[i][scText], COLOUR_SIDECHAT & 0x000000FF);
		scData[i][scColour] = COLOUR_SIDECHAT;
		scData[i][scTick] = 0;
		TextDrawSetShadow(scData[i][scText], 1);
		//TextDrawSetProportional(scData[i][scText], 1);
		//TextDrawShowForAll(scData[i][scText]);
		scData[i][scExists] = 1;
		ty = ty - 8.0;
	}
	
	kEnableTimer(16); // scTimer = SetTimer("fdm_SidechatUpdate", SIDECHAT_UPDATE_DELAY, 1);
	return 1;
}

SidechatExit() {
	for (new i; i < SIDECHAT_MAX_LINES; i++) {
		if (!scData[i][scExists]) continue;
		TextDrawDestroy(scData[i][scText]);
		scData[i][scExists] = 0;
		scData[i][scColour] = 0;
		scData[i][scTick] = 0;
		strclr(scData[i][scString]);
		strclr(scQue[i][scqString]);
		scQue[i][scqColour] = 0;
	}
	LoopPlayers(i) {
		sidechat_OnPlayerDisconnect(i);
	}
	kDisableTimer(16); // KillTimer(scTimer);
	return 1;
}

sidechat_OnPlayerDisconnect(playerid) {
	scPData[playerid][scpLines] = 0;
	scPData[playerid][scpShowTime] = 0;
	for (new i; i < SIDECHAT_MAX_LINES; i++) {
		scPData[playerid][scpShowLines][i] = 0;
	}
	return 1;
}

stock SideChatAddLine(text[], colour = 0) {
	if (!colour) colour = COLOUR_SIDECHAT;
	
	if (scQueIdx > SIDECHAT_MAX_LINES - 1) {
		fdm_SidechatUpdate();
	}
	
	strcpy(scQue[scQueIdx][scqString], text, MAX_INPUT);
	scQue[scQueIdx][scqColour] = colour;
	scQueIdx++;
	return 1;
}

forward fdm_SidechatUpdate();
public fdm_SidechatUpdate() {
	
	// Hiding player texts before, and showing text after improves bandwidth efficiency
	new tick = GetTickCount();
	
	// Update player's texts (hide)
	LoopPlayers(i) {
		for (new j; j < scPData[i][scpLines]; j++) {
			if (!scData[j][scExists]) continue;
			if (scPData[i][scpShowTime] && (scData[j][scTick] && tick - scData[j][scTick] > scPData[i][scpShowTime])) {
				if (scPData[i][scpShowLines][j]) {
					scPData[i][scpShowLines][j] = 0;
					TextDrawHideForPlayer(i, scData[j][scText]);
				}
			}
		}
	}
	
	// Merge que with current text
	if (scQueIdx) {
		
		// Move the text up to make room for the qued text
		for (new i = SIDECHAT_MAX_LINES - 1; i >= scQueIdx; i--) {
			SideChatCopyText(scData[i][scText], scData[i][scString], scData[i][scColour], i,
				scData[i - scQueIdx][scString], scData[i - scQueIdx][scColour]);
			
			scData[i][scTick] = scData[i - scQueIdx][scTick];
		}
		
		// Add the qued text into that space
		new count = scQueIdx;
		for (new i; i < scQueIdx; i++) {
			count--;
			SideChatCopyText(scData[i][scText], scData[i][scString], scData[i][scColour], i,
				scQue[count][scqString], scQue[count][scqColour]);
			
			scData[i][scTick] = tick;
		}
		
		scQueIdx = 0;
		
	}
	
	// Update player's texts (show)
	LoopPlayers(i) {
		for (new j; j < scPData[i][scpLines]; j++) {
			if (!scData[j][scExists]) continue;
			if (!scPData[i][scpShowTime] || (scData[j][scTick] && tick - scData[j][scTick] <= scPData[i][scpShowTime])) {
				if (!scPData[i][scpShowLines][j]) {
					scPData[i][scpShowLines][j] = 1;
					TextDrawShowForPlayer(i, scData[j][scText]);
				}
			}
		}
	}
	
	return 1;
}

SideChatCopyText(Text:desttext, deststr[], &destcolour, destchatidx, const sourcestr[], sourcecolour) {
	new colourchanged;
	
	if (destcolour != sourcecolour) { // Colour is different
		TextDrawColor(desttext, sourcecolour);
		TextDrawBackgroundColor(desttext, sourcecolour & 0x000000FF);
		destcolour = sourcecolour;
		colourchanged = 1;
	}
	if (strcmp(deststr, sourcestr)) { // Text is different
		if (colourchanged) {
			LoopPlayers(i) {
				if (destchatidx >= scPData[i][scpLines]) continue;
				if (!scPData[i][scpShowLines][destchatidx]) continue;
				TextDrawHideForPlayer(i, desttext);
			}
		}
		
		strcpy(deststr, sourcestr, MAX_INPUT);
		TextDrawSetString(desttext, deststr);
	}
	
	if (colourchanged) {
		LoopPlayers(i) {
			if (destchatidx >= scPData[i][scpLines]) continue;
			if (!scPData[i][scpShowLines][destchatidx]) continue;
			TextDrawShowForPlayer(i, desttext);
		}
	}
	return 1;
}

SideChatSetLinesForPlayer(playerid, lines) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (lines > scPData[playerid][scpLines]) {
		/*for (new i = scPData[playerid][scpLines]; i < lines; i++) {
			if (!scData[i][scExists]) continue;
			TextDrawShowForPlayer(playerid, scData[i][scText]);
		}*/
		new tick = GetTickCount();
		
		for (new i = scPData[playerid][scpLines]; i < lines; i++) {
			if (!scData[i][scExists]) continue;
			if (!scPData[playerid][scpShowTime] || (scData[i][scTick] && scData[i][scTick] > tick - scPData[playerid][scpShowTime])) {
				if (!scPData[playerid][scpShowLines][i]) {
					scPData[playerid][scpShowLines][i] = 1;
					TextDrawShowForPlayer(playerid, scData[i][scText]);
				}
			}
		}
	} else {
		/*for (new i = lines; i < scPData[playerid][scpLines]; i++) {
			if (!scData[i][scExists]) continue;
			TextDrawHideForPlayer(playerid, scData[i][scText]);
		}*/
		for (new i = lines; i < scPData[playerid][scpLines]; i++) {
			if (!scData[i][scExists]) continue;
			scPData[playerid][scpShowLines][i] = 0;
			TextDrawHideForPlayer(playerid, scData[i][scText]);
		}
	}
	
	scPData[playerid][scpLines] = lines;
	
	return 1;
}

sidechat_OnPlayerLogin(playerid) {
	if (!pData[playerid][pSideChat]) return 1;
	if (pData[playerid][pSideChatLines] < 1 || pData[playerid][pSideChatLines] > SIDECHAT_MAX_LINES) {
		pData[playerid][pSideChatLines] = SIDECHAT_DEFAULT_LINES;
	}
	
	scPData[playerid][scpShowTime] = pData[playerid][pSideChatTime];
	SideChatSetLinesForPlayer(playerid, pData[playerid][pSideChatLines]);
	return 1;
}

kcmd:stext(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "SIDE TEXT: You must be registered to use /stext. (/register [password])");
		return 1;
	}
	
	if (pData[playerid][pSideChat]) {
		pData[playerid][pSideChat] = 0;
		SideChatSetLinesForPlayer(playerid, 0);
		
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SIDE TEXT: Side text disabled.");
	} else {
		pData[playerid][pSideChat] = 1;
		scPData[playerid][scpShowTime] = pData[playerid][pSideChatTime];
		SideChatSetLinesForPlayer(playerid, pData[playerid][pSideChatLines]);
		
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SIDE TEXT: Side text enabled.");
	}
	return 1;
}

kcmd:stextlines(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "SIDE TEXT: You must be registered to use /stextlines. (/register [password])");
		return 1;
	}
	
	new msg[MAX_INPUT];
	
	if (isnull(text)) {
		format(msg, sizeof(msg), "USAGE: /stext [1-%i]", SIDECHAT_MAX_LINES);
		SendClientMessage(playerid, COLOUR_HELP, msg);
		return 1;
	}
	
	new lines = strval(text);
	
	if (lines < 1 || lines > SIDECHAT_MAX_LINES) {
		SendClientMessage(playerid, COLOUR_ERROR, "SIDE TEXT: Invalid text lines.");
		return 1;
	}
	
	if (pData[playerid][pSideChat]) {
		SideChatSetLinesForPlayer(playerid, lines);
	}
	
	pData[playerid][pSideChatLines] = lines;
	format(msg, sizeof(msg), "SIDE TEXT: Side text lines changed to %i", pData[playerid][pSideChatLines]);
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	return 1;
}

kcmd:stexttime(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "SIDE TEXT: You must be registered to use /stexttime. (/register [password])");
		return 1;
	}
	
	if (isnull(text)) {
		if (pData[playerid][pSideChatTime]) {
			pData[playerid][pSideChatTime] = 0;
			scPData[playerid][scpShowTime] = 0;
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SIDE TEXT: Side texts no longer dissapears.");
		} else {
			SendClientMessage(playerid, COLOUR_HELP, "USAGE: /stexttime (seconds)");
		}
		return 1;
	}
	
	new Float:floattime = floatstr(text);
	new time = floatround(floattime * 1000.0, floatround_floor);
	
	if (time <= 0) {
		SendClientMessage(playerid, COLOUR_ERROR, "SIDE TEXT: Invalid time interval.");
		return 1;
	}
	
	pData[playerid][pSideChatTime] = time;
	scPData[playerid][scpShowTime] = time;
	
	new msg[MAX_INPUT];
	format(msg, sizeof(msg), "SIDE TEXT: Side texts will dissapear within %0.1fs.", floattime);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	return 1;
}

