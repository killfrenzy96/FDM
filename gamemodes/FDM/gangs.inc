#include <a_samp>

#define FILE_GANGS "FDM/Gangs/gang_"

#define MIN_LASTGANGLOGIN_DELETE_TIME 5184000 // 2 months before gang can be overridden (Gang expiration time)

#define COLOUR_GANGCHAT 0xFF8010AA
#define COLOUR_NOGANG 0xAAAAAA60

#define MAX_GANG_LOADED MAX_PLAYERS // Maximum amount of gangs that can be loaded at once
#define MAX_GANG_MEMBERS 128
#define MAX_MEMBERS_PER_LINE 6

#define MIN_GANGCREATE_LEVEL 5 // Minimum level before they can create a gang - helps prevent too many 'unused' gangs from being created

#define KEY_LASTLOAD "LastLoadTime"
#define KEY_COLOUR "Colour"
#define KEY_CHATUSECOLOUR "ChatUseColour"

#define INVALID_GANG_ID -1
#define INVALID_MEMBER_ID -1

enum memberCacheInfo {
	cGangID,
	cName[MAX_NAME],
	cIsLeader,
	cInvitedGangID
}
enum gangInfo {
	gName[MAX_GANG_NAME],
	gColour,
	gChatUseColour,
}
enum gangMemberInfo {
	mName[MAX_NAME],
	mIsLeader
}
enum gangColourInfo {
	cColourName[MAX_NAME],
	cColourID
}

new cData[MAX_PLAYERS][memberCacheInfo];
new gData[MAX_GANG_LOADED][gangInfo];
new gMembers[MAX_GANG_LOADED][MAX_GANG_MEMBERS][gangMemberInfo];
new gangColours[][gangColourInfo] = {
	{"Red", 0xDD000060},
	{"Orange", 0xFF7F0060},
	{"Yellow", 0xDDDD0060},
	{"Green", 0x00DD0060},
	{"Aqua", 0x00DDDD60},
	{"Blue", 0x0000DD60},
	{"Indigo", 0x4B008260},
	{"Violet", 0xEE82EE60}
};

GangsInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		cData[i][cGangID] = INVALID_GANG_ID;
		cData[i][cInvitedGangID] = INVALID_GANG_ID;
	}
}

gangs_OnPlayerDisconnect(playerid) {
	ResetMemberData(playerid);
}

gangs_OnPlayerDeath(playerid, killerid) {
	if (cData[playerid][cGangID] == INVALID_GANG_ID) return 1;
	if (killerid == INVALID_PLAYER_ID) return 1;
	
	if (cData[playerid][cGangID] == cData[killerid][cGangID]) { // Same gang
		// Gang member killed
		new message[MAX_INPUT];
		
		format(message, sizeof(message), "%s killed a gang member %s!", pData[killerid][pLoginName], pData[playerid][pLoginName]);
		SendMessageToGang(cData[playerid][cGangID], message);
		return 0;
	}
	return 1;
}

gangs_OnPlayerSpawn(playerid) {
	if (pData[playerid][pGang][0] || !IsPlayerNPC(playerid)) {
		GivePlayerGangColour(playerid);
	}
	return 1;
}

gangs_OnPlayerLogin(playerid) { // Login player into the gang
	if (pData[playerid][pGang][0]) { // They are in a gang
		new tmpstr[MAX_INPUT];
		if (GangPlayerLogin(playerid)) {
			format(tmpstr, sizeof(tmpstr), "GANG: You have logged into the gang '%s'", pData[playerid][pGang]);
			SendClientMessage(playerid, COLOUR_GANGCHAT, tmpstr);
		} else {
			format(tmpstr, sizeof(tmpstr), "GANG: You no longer in the gang '%s'.", pData[playerid][pGang]);
			strclr(pData[playerid][pGang]);
			SendClientMessage(playerid, COLOUR_GANGCHAT, tmpstr);
		}
	}
	if (pData[playerid][pGang][0] || !IsPlayerNPC(playerid)) {
		GivePlayerGangColour(playerid);
	}
	return 1;
}

GangPlayerLogin(playerid) {
	if (!pData[playerid][pGang][0]) return 0;
	
	new gangid = GetGangIDFromName(pData[playerid][pGang]);
	if (gangid == INVALID_GANG_ID) {
		// Load the gang
		new filename[MAX_FILENAME];
		strcpy(filename, GetGangFileFromName(pData[playerid][pGang]));
		
		gangid = GetFreeGangID();
		if (gangid == INVALID_GANG_ID) return 0; // No free gang slots avaliable
		
		// Non existant gang
		if (!INI_Exist(filename)) return 0;
		if (!INI_Open(filename)) return 0;
		
		strcpy(gData[gangid][gName], pData[playerid][pGang], MAX_GANG_NAME);
		gData[gangid][gColour] = INI_ReadInt(KEY_COLOUR);
		gData[gangid][gChatUseColour] = INI_ReadInt(KEY_CHATUSECOLOUR);
		
		new membercount;
		new tmpstr[MAX_STRING];
		new tmpstr2[MAX_NAME];
		new key[INI_MAX_KEY];
		
		for (new i; i < MAX_GANG_MEMBERS; i++) {
			
			// Get key
			format(key, sizeof(key), "M%i", i);
			if (!INI_ReadString(tmpstr, key)) continue; // Invalid member
			if (!tmpstr[0]) continue;
			
			new idx;
			splitcpy(tmpstr2, tmpstr, idx, ' ');
			if (!tmpstr2[0]) continue; // Invalid member
			
			strcpy(gMembers[gangid][i][mName], tmpstr2, MAX_NAME);
			
			if (strval(split(tmpstr, idx, ' '))) {
				gMembers[gangid][i][mIsLeader] = 1;
			} else {
				gMembers[gangid][i][mIsLeader] = 0;
			}
			
			membercount++;
		}
		
		if (membercount) {
			INI_WriteInt(KEY_LASTLOAD, gettime());
			INI_Save();
			INI_Close();
		} else {
			ResetGangData(gangid);
			INI_Remove(filename);
			return 0;
		}
	}
	
	// Gang loaded successfully, login player into gang.
	new memberid = GetPlayerGangMemberID(playerid, gangid);
	
	if (memberid == INVALID_MEMBER_ID) {
		// If there are no members in that gang, unload gang
		ResetMemberData(playerid);
		return 0;
	}
	
	cData[playerid][cGangID] = gangid;
	strcpy(cData[playerid][cName], pData[playerid][pLoginName], MAX_NAME);
	cData[playerid][cIsLeader] = gMembers[gangid][memberid][mIsLeader];
	return 1;
}

SaveGang(gangid) {
	if (!gData[gangid][gName][0]) return 0; // Invalid gang name
	
	new filename[MAX_FILENAME];
	filename = GetGangFileFromName(gData[gangid][gName]);
	
	if (!INI_Open(filename)) return 0;
	
	new key[INI_MAX_KEY];
	new tmpstr[MAX_STRING];
	
	INI_WriteInt(KEY_COLOUR, gData[gangid][gColour]);
	INI_WriteInt(KEY_LASTLOAD, gettime());
	INI_WriteInt(KEY_CHATUSECOLOUR, gData[gangid][gChatUseColour]);
	
	new membercount;
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		format(key, sizeof(key), "M%i", i);
		
		if (gMembers[gangid][i][mIsLeader]) { // Integrity check
			gMembers[gangid][i][mIsLeader] = 1;
		} else {
			gMembers[gangid][i][mIsLeader] = 0;
		}
		
		if (gMembers[gangid][i][mName][0]) {
			format(tmpstr, sizeof(tmpstr), "%s %i", gMembers[gangid][i][mName], gMembers[gangid][i][mIsLeader]);
			INI_WriteString(key, tmpstr);
			membercount++;
		} else {
			INI_RemoveEntry(key);
			//INI_WriteString(key, "");
		}
	}
	
	if (membercount) {
		INI_Save();
		INI_Close();
		return 1;
	} else {
		INI_Remove(filename);
		return 0;
	}
}

CreateGang(playerid, gangname[], gangcolour) {
	if (!pData[playerid][pIsLoggedIn]) return INVALID_GANG_ID;
	new gangid = GetFreeGangID();
	if (gangid == INVALID_GANG_ID) return INVALID_GANG_ID;
	
	// Create gang member data
	strcpy(gData[gangid][gName], gangname, MAX_GANG_NAME);
	gData[gangid][gColour] = gangcolour;
	gData[gangid][gChatUseColour]= 0;
	strcpy(gMembers[gangid][0][mName], pData[playerid][pLoginName], MAX_NAME);
	gMembers[gangid][0][mIsLeader] = 1;
	
	// Create member cache data
	cData[playerid][cGangID] = gangid;
	strcpy(cData[playerid][cName], pData[playerid][pLoginName], MAX_NAME);
	cData[playerid][cIsLeader] = 1;
	cData[playerid][cInvitedGangID] = INVALID_GANG_ID;
	
	SetPlayerColor(playerid, gangcolour);
	
	strcpy(pData[playerid][pGang], gangname, MAX_GANG_NAME);
	
	printf("[FDM] GANG: %s created a gang '%s'", pData[playerid][pLoginName], gangname);
	SaveGang(gangid);
	return gangid;
}

PutPlayerInGang(playerid, gangid, leader) {
	if (!pData[playerid][pIsLoggedIn]) return 0; // Not registered
	if (gangid == INVALID_GANG_ID) return 0;
	
	// Find spare gang member slot
	for (new i = 1; i < MAX_GANG_MEMBERS; i++) { // No gang members may be invited into gang slot 0, even if noone is in it
		if (gMembers[gangid][i][mName][0]) continue; // Slot is taken up
		// Free slot avaliable
		
		// Update member list
		strcpy(gMembers[gangid][i][mName], pData[playerid][pLoginName], MAX_NAME);
		
		// Update player profile
		strcpy(pData[playerid][pGang], gData[gangid][gName], MAX_GANG_NAME);
		
		// Update cache
		strcpy(cData[playerid][cName], pData[playerid][pLoginName], MAX_NAME);
		cData[playerid][cGangID] = gangid;
		cData[playerid][cIsLeader] = leader;
		cData[playerid][cInvitedGangID] = INVALID_GANG_ID;
		
		// Update the player's colour
		SetPlayerColor(playerid, gData[gangid][gColour]);
		
		// Save data
		SaveGang(gangid);
		return 1;
	}
	// Spare slot not found
	return 0;
}

RemovePlayerFromGang(playerid) {
	if (!pData[playerid][pIsLoggedIn]) return 0; // Not registered
	if (cData[playerid][cGangID] == INVALID_GANG_ID) return 0; // Player is not in gang
	
	// Find gang member in member list
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		if (!gMembers[cData[playerid][cGangID]][i][mName][0]) continue;
		if (strcmp(gMembers[cData[playerid][cGangID]][i][mName], cData[playerid][cName])) continue; // Not gang member
		
		// Gang member found
		// Remove from member list
		strclr(gMembers[cData[playerid][cGangID]][i][mName]);
		gMembers[cData[playerid][cGangID]][i][mIsLeader] = 0;
		
		// Reset player account gang
		strclr(pData[playerid][pGang]);
		
		// Reset their player colour
		SetPlayerColor(playerid, COLOUR_NOGANG);
		
		// Save data
		SaveGang(cData[playerid][cGangID]);
		
		ResetMemberData(playerid); // Reset their cache data
		return 1;
	}
	return 0;
}

GivePlayerGangColour(playerid) {
	if (cData[playerid][cGangID] == INVALID_GANG_ID) {
		SetPlayerColor(playerid, COLOUR_NOGANG);
	} else {
		SetPlayerColor(playerid, gData[cData[playerid][cGangID]][gColour]);
	}
}

ResetMemberData(playerid) {
	// Check if any members of this id are still loaded - if not, unload the gang
	if (cData[playerid][cGangID] != INVALID_GANG_ID) {
		new samegangfound;
		LoopPlayers(i) {
			if (i == playerid) continue;
			if (cData[playerid][cGangID] != cData[i][cGangID]) continue;
			// Same gangid found for member
			samegangfound = 1;
			break;
		}
		if (!samegangfound) ResetGangData(cData[playerid][cGangID]);
	}
	
	new tmpdata[memberCacheInfo];
	cData[playerid] = tmpdata;
	cData[playerid][cGangID] = INVALID_GANG_ID;
	cData[playerid][cInvitedGangID] = INVALID_GANG_ID;
	return 1;
}

ResetGangData(gangid) {
	// This should not be done while players are still logged into the gang
	// Find any outstanding invites for this gang and reset them
	LoopPlayers(i) {
		if (cData[i][cInvitedGangID] == gangid) cData[i][cInvitedGangID] = INVALID_GANG_ID;
	}
	
	new tmpdata[gangInfo];
	gData[gangid] = tmpdata;
	new tmpdata2[gangMemberInfo];
	for (new l; l < MAX_GANG_MEMBERS; l++) {
		gMembers[gangid][l] = tmpdata2;
	}
	return 1;
}

GetGangIDFromName(gangname[]) { // Get player gangid, from their gang name
	if (!gangname[0]) return INVALID_GANG_ID;
	for (new i; i < MAX_GANG_LOADED; i++) {
		if (!gData[i][gName][0]) continue; // No gang exists there
		if (!strcmp(gData[i][gName], gangname)) return i;
	}
	return INVALID_GANG_ID;
}

GetPlayerGangMemberID(playerid, gangid) {
	if (playerid == INVALID_PLAYER_ID || gangid == INVALID_GANG_ID) return INVALID_MEMBER_ID;
	if (!pData[playerid][pIsLoggedIn]) return INVALID_MEMBER_ID;
	
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		if (!gMembers[gangid][i][mName][0]) continue;
		if (!strcmp(gMembers[gangid][i][mName], pData[playerid][pLoginName], true, MAX_NAME)) {
			return i;
		}
	}
	
	return INVALID_MEMBER_ID;
}

GetFreeGangID() {
	for (new i; i < MAX_GANG_LOADED; i++) {
		if (gData[i][gName][0]) continue; // Gang exists there
		return i;
	}
	return INVALID_GANG_ID;
}

GetGangFileFromName(gangname[]) {
	new filename[MAX_FILENAME];
	format(filename, sizeof(filename), "%s%s.txt", FILE_GANGS, GetValidFilenameFromString(gangname, 0));
	return filename;
}

stock ArePlayersInSameGang(player1, player2) {
	if (cData[player1][cGangID] == INVALID_GANG_ID) return 0;
	if (cData[player1][cGangID] == cData[player2][cGangID]) {
		return 1;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------------------------------------------

kcmd:gangcreate(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangcreate [colour] [gangname]");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: You must be registered to create a gang.");
		return 1;
	}
	if (cData[playerid][cGangID] != INVALID_GANG_ID) { // Player is already in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: You are already in a gang.");
		return 1;
	}
	new tmpstr[MAX_INPUT];
	if (pData[playerid][pLevel] < MIN_GANGCREATE_LEVEL) { // Player isn't high level enough to make a gang
		format(tmpstr, sizeof(tmpstr), "GANG CREATE: You must be at least level %i to make a gang.", MIN_GANGCREATE_LEVEL);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	
	new gangname[MAX_INPUT];
	new colourstr[MAX_INPUT];
	new colour;
	new idx;
	
	splitcpy(colourstr, text, idx, ' ');
	splitcpy(gangname, text, idx, 0);
	
	if (!colourstr[0] || !gangname[0]) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangcreate [colour] [gangname]");
		return 1;
	}
	if (strlen(gangname) >= MAX_GANG_NAME - 1) { // Gang name too long, will have problems with filenames
		format(tmpstr, sizeof(tmpstr), "GANG CREATE: Gang name is too long. Maximum %i characters", MAX_GANG_NAME);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (!IsValidFilename(gangname, 0)) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: Invalid gang name.");
		return 1;
	}
	// Get gang colour
	for (new i; i < sizeof(gangColours); i++) {
		if (strfind(gangColours[i][cColourName], colourstr, true) != -1) { // Part name found
			colour = GetColourShade(gangColours[i][cColourID]);
			break;
		}
	}
	if (!colour) { // Colour not found
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: Invalid colour. Use /gangcolours for a list of gang colours.");
		return 1;
	}
	new filename[MAX_FILENAME];
	filename = GetGangFileFromName(gangname);
	if (INI_Exist(filename)) { // Gang already exists
		if (INI_Open(filename)) {
			if (INI_ReadInt(KEY_LASTLOAD) > gettime() - MIN_LASTGANGLOGIN_DELETE_TIME) { // Gang has not expired
				SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: Gang name already exists.");
				INI_Close();
				return 1;
			}
			INI_Close();
		}
	}
	if (CreateGang(playerid, gangname, colour) == INVALID_GANG_ID) { // Create the gang
		// Gang not created
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CREATE: No more gangs can be loaded at once.");
		return 1;
	}
	
	// Gang created
	format(tmpstr, sizeof(tmpstr), "GANG CREATE: Gang '%s' has been created.", gangname);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	
	return 1;
}

kcmd:ganginvite(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /ganginvite [player]");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG INVITE: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG INVITE: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not a gang leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG INVITE: You must be a gang leader.");
		return 1;
	}
	new inviteid = FindPlayer(text);
	if (inviteid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG INVITE: Player cannot be found.");
		return 1;
	}
	if (inviteid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG INVITE: You cannot invite yourself.");
		return 1;
	}
	
	new tmpstr[MAX_INPUT];
	
	if (!pData[inviteid][pIsLoggedIn]) { // Invited player is not registered
		new invitename[MAX_NAME];
		GetPlayerName(inviteid, invitename, sizeof(invitename));
		format(tmpstr, sizeof(tmpstr), "GANG INVITE: %s is not registered.", invitename);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[inviteid][cGangID] != INVALID_GANG_ID) { // Invited player is in a gang
		format(tmpstr, sizeof(tmpstr), "GANG INVITE: %s is already in a gang '%s'.", pData[inviteid][pLoginName], gData[cData[inviteid][cGangID]][gName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[inviteid][cInvitedGangID] == cData[playerid][cGangID]) { // Already invited
		format(tmpstr, sizeof(tmpstr), "GANG INVITE: You have already invited %s into your gang '%s'.", pData[inviteid][pLoginName], gData[cData[playerid][cGangID]][gName]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
		return 1;
	}
	
	// Player can invite
	cData[inviteid][cInvitedGangID] = cData[playerid][cGangID];
	
	// Player invited, send invite messages
	format(tmpstr, sizeof(tmpstr), "GANG INVITE: You have invited %s into your gang '%s', awaiting player to join...", pData[inviteid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	format(tmpstr, sizeof(tmpstr), "GANG INVITE: You have been invited to '%s' by %s. Type /gangjoin to join.", gData[cData[playerid][cGangID]][gName], pData[playerid][pLoginName]);
	SendClientMessage(inviteid, COLOUR_SUCCESSFUL, tmpstr);
	
	return 1;
}

kcmd:gangkick(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangkick [player]");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player not in gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG KICK: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG KICK: You must a gang leader to kick players.");
		return 1;
	}
	new tmpstr[MAX_INPUT];
	new kickid = FindPlayer(text);
	if (kickid == INVALID_PLAYER_ID) {
		//Check offline players
		for (new i; i < MAX_GANG_MEMBERS; i++) {
			if (!gMembers[cData[playerid][cGangID]][i][mName][0]) continue;
			if (!strcmp(gMembers[cData[playerid][cGangID]][i][mName], pData[playerid][pLoginName])) continue;
			if (strfind(gMembers[cData[playerid][cGangID]][i][mName], text, true) == -1) continue;
			
			if (!i) { // Original gang leader
				format(tmpstr, sizeof(tmpstr), "GANG KICK: %s is the original gang leader and cannot be kicked.", gMembers[cData[playerid][cGangID]][i][mName]);
				SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
				return 1;
			}
			
			// Offline player found
			format(tmpstr, sizeof(tmpstr), "GANG KICK: You have kicked %s from the gang '%s'", gMembers[cData[playerid][cGangID]][i][mName], gData[cData[playerid][cGangID]][gName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
			format(tmpstr, sizeof(tmpstr), "%s has been kicked from the gang.", gMembers[cData[playerid][cGangID]][i][mName]);
			SendMessageToGang(cData[playerid][cGangID], tmpstr);
			
			strclr(gMembers[cData[playerid][cGangID]][i][mName]);
			gMembers[cData[playerid][cGangID]][i][mIsLeader] = 0;
			SaveGang(cData[playerid][cGangID]);
			return 1;
		}
		SendClientMessage(playerid, COLOUR_ERROR, "GANG KICK: Player cannot be found.");
		return 1;
	}
	if (kickid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG KICK: You cannot kick yourself from the gang. Just use /gangquit.");
		return 1;
	}
	if (!pData[kickid][pIsLoggedIn]) {
		new kickname[MAX_NAME];
		GetPlayerName(kickid, kickname, sizeof(kickname));
		format(tmpstr, sizeof(tmpstr), "GANG KICK: %s is not registered", kickname);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[playerid][cGangID] != cData[kickid][cGangID]) {
		format(tmpstr, sizeof(tmpstr), "GANG KICK: You are not in the same gang as %s", pData[kickid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (gMembers[cData[playerid][cGangID]][0][mName][0] && !strcmp(gMembers[cData[playerid][cGangID]][0][mName], cData[kickid][cName])) { // Original gang leader
		format(tmpstr, sizeof(tmpstr), "GANG KICK: %s is the original gang leader and cannot be kicked.", cData[kickid][cName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	
	// Proceed to kicking
	if (!RemovePlayerFromGang(kickid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG KICK: Unknown cause of error.");
		return 1;
	}
	
	// Quit gang successful
	format(tmpstr, sizeof(tmpstr), "GANG KICK: You have kicked %s from the gang '%s'", pData[kickid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	format(tmpstr, sizeof(tmpstr), "%s kicked you from the gang '%s'", pData[playerid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(kickid, COLOUR_ATTENTION, tmpstr);
	
	format(tmpstr, sizeof(tmpstr), "%s has been kicked from the gang.", pData[kickid][pLoginName]);
	SendMessageToGang(cData[playerid][cGangID], tmpstr);
	return 1;
}

kcmd:gangleadergive(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangleadergive [player]");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not a gang leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: You must be a gang leader.");
		return 1;
	}
	new tmpstr[MAX_INPUT];
	new leaderid = FindPlayer(text);
	if (leaderid == INVALID_PLAYER_ID) { // Player to give leader is not found, search offline members
		for (new i; i < MAX_GANG_MEMBERS; i++) { // Search offline members
			if (strfind(gMembers[cData[playerid][cGangID]][i][mName], text, true) == -1) continue;
			if (gMembers[cData[playerid][cGangID]][i][mIsLeader]) {
				format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is already a gang leader of '%s'.", gMembers[cData[playerid][cGangID]][i][mName], gData[cData[playerid][cGangID]][gName]);
				SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
				return 1;
			}
			gMembers[cData[playerid][cGangID]][i][mIsLeader] = 1;
			
			SaveGang(cData[playerid][cGangID]);
			format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is now a gang leader of '%s'.", gMembers[cData[playerid][cGangID]][i][mName], gData[cData[playerid][cGangID]][gName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
			return 1;
		}
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: Player not found.");
		return 1;
	}
	if (leaderid == playerid) { // Player to give leader to themselves
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: You cannot give yourself leader, you're already a gang leader.");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Unregistered
		new leadername[MAX_NAME];
		GetPlayerName(leaderid, leadername, sizeof(leadername));
		format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is not registered.", leadername);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[playerid][cGangID] != cData[leaderid][cGangID]) { // Players aren't in the same gang
		format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is not in your gang.", pData[leaderid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[leaderid][cIsLeader]) { // Already a leader
		format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is already a gang leader.", pData[leaderid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	
	// Give that player gang leader
	cData[leaderid][cIsLeader] = 1;
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		if (gMembers[cData[playerid][cGangID]][i][mName][0] && !strcmp(gMembers[cData[playerid][cGangID]][i][mName], cData[leaderid][cName])) {
			gMembers[cData[playerid][cGangID]][i][mIsLeader] = 1;
			break;
		}
	}
	
	SaveGang(cData[playerid][cGangID]);
	format(tmpstr, sizeof(tmpstr), "GANG LEADER GIVE: %s is now a gang leader of '%s'.", pData[leaderid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	format(tmpstr, sizeof(tmpstr), "You are now a gang leader of '%s'.", gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(leaderid, COLOUR_SUCCESSFUL, tmpstr);
	return 1;
}

kcmd:gangleadertake(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangleadertake [player]");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER TAKE: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER TAKE: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not a gang leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER TAKE: You must be a gang leader.");
		return 1;
	}
	new tmpstr[MAX_INPUT];
	new leaderid = FindPlayer(text);
	if (leaderid == INVALID_PLAYER_ID) { // Player to take leader is not found
		for (new i; i < MAX_GANG_MEMBERS; i++) { // Search offline members
			if (strfind(gMembers[cData[playerid][cGangID]][i][mName], text, true) == -1) continue;
			if (!gMembers[cData[playerid][cGangID]][i][mIsLeader]) {
				format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is already not a gang leader of '%s'.", gMembers[cData[playerid][cGangID]][i][mName], gData[cData[playerid][cGangID]][gName]);
				SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
				return 1;
			}
			if (!i) { // Original gang leader
				format(tmpstr, sizeof(tmpstr), "GANG KICK TAKE: %s is the original gang leader, you cannot take away leadership.", gMembers[cData[playerid][cGangID]][i][mName]);
				SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
				return 1;
			}
			gMembers[cData[playerid][cGangID]][i][mIsLeader] = 0;
			
			SaveGang(cData[playerid][cGangID]);
			format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is no longer a gang leader of '%s'.", gMembers[cData[playerid][cGangID]][i][mName], gData[cData[playerid][cGangID]][gName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
			return 1;
		}
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER GIVE: Player not found.");
		return 1;
	}
	if (leaderid == playerid) { // Player to give leader to themselves
		SendClientMessage(playerid, COLOUR_ERROR, "GANG LEADER TAKE: You cannot remove gang leadership from yourself.");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Unregistered
		new leadername[MAX_NAME];
		GetPlayerName(leaderid, leadername, sizeof(leadername));
		format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is not registered.", leadername);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (cData[playerid][cGangID] != cData[leaderid][cGangID]) { // Players aren't in the same gang
		format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is not in your gang.", pData[leaderid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (!cData[leaderid][cIsLeader]) { // Not a leader
		format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is not a gang leader.", pData[leaderid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	if (gMembers[cData[playerid][cGangID]][0][mName][0] && !strcmp(gMembers[cData[playerid][cGangID]][0][mName], cData[leaderid][cName])) {
		format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is the original gang leader and cannot be demoted.", pData[leaderid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	
	// Remove that player gang leader status
	cData[leaderid][cIsLeader] = 0;
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		if (gMembers[cData[playerid][cGangID]][i][mName][0] && !strcmp(gMembers[cData[playerid][cGangID]][i][mName], cData[leaderid][cName])) {
			gMembers[cData[playerid][cGangID]][i][mIsLeader] = 0;
			break;
		}
	}
	
	SaveGang(cData[playerid][cGangID]);
	format(tmpstr, sizeof(tmpstr), "GANG LEADER TAKE: %s is no longer a gang leader of '%s'.", pData[leaderid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	format(tmpstr, sizeof(tmpstr), "You are no longer a gang leader of '%s'.", gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(leaderid, COLOUR_ATTENTION, tmpstr);
	return 1;
}

kcmd:gangchangecolour(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /gangchangecolour [colour]");
		return 1;
	}
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHANGE COLOUR: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHANGE COLOUR: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not a gang leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHANGE COLOUR: You must be a gang leader.");
		return 1;
	}
	// Get gang colour
	new colour;
	new colourid = -1;
	for (new i; i < sizeof(gangColours); i++) {
		if (strfind(gangColours[i][cColourName], text, true) != -1) { // Part name found
			colour = GetColourShade(gangColours[i][cColourID]);
			colourid = i;
			break;
		}
	}
	if (colourid == -1) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHANGE COLOUR: Invalid colour. Type /gangcolours for a list of colours.");
		return 1;
	}
	
	// Change the gang colour
	gData[cData[playerid][cGangID]][gColour] = colour;
	LoopPlayers(p) { // Update every gang member's colour
		if (cData[p][cGangID] == cData[playerid][cGangID]) {
			SetPlayerColor(p, colour);
		}
	}
	SaveGang(cData[playerid][cGangID]);
	
	// Send player message of success
	new tmpstr[MAX_INPUT];
	format(tmpstr, sizeof(tmpstr), "GANG CHANGE COLOUR: Gang colour changed to %s.", gangColours[colourid][cColourName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	return 1;
}

kcmd:gangchatcolour(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHAT COLOUR: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHAT COLOUR: You must be in a gang.");
		return 1;
	}
	if (!cData[playerid][cIsLeader]) { // Player is not a gang leader
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHAT COLOUR: You must be a gang leader.");
		return 1;
	}
	
	if (isnull(text)) {
		gData[cData[playerid][cGangID]][gChatUseColour] = !gData[cData[playerid][cGangID]][gChatUseColour];
	} else {
		if (strval(text)) {
			if (gData[cData[playerid][cGangID]][gChatUseColour]) {
				SendClientMessage(playerid, COLOUR_SUCCESSFUL, "GANG CHAT COLOUR: Using gang colour for chat is already enabled.");
				return 1;
			}
			gData[cData[playerid][cGangID]][gChatUseColour] = 1;
		} else {
			if (!gData[cData[playerid][cGangID]][gChatUseColour]) {
				SendClientMessage(playerid, COLOUR_SUCCESSFUL, "GANG CHAT COLOUR: Using gang colour for chat is already disabled.");
				return 1;
			}
			gData[cData[playerid][cGangID]][gChatUseColour] = 0;
		}
	}
	
	if (gData[cData[playerid][cGangID]][gChatUseColour]) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "GANG CHAT COLOUR: Using gang colour for chat enabled.");
	} else {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "GANG CHAT COLOUR: Using gang colour for chat disabled.");
	}
	SaveGang(cData[playerid][cGangID]);
	return 1;
}
	
kcmd:gangmembers(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG MEMBERS: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player has not been invited
		SendClientMessage(playerid, COLOUR_ERROR, "GANG MEMBERS: You must be in a gang to use /gangmembers.");
		return 1;
	}
	
	// Show the player the gang members
	new linestr[MAX_INPUT];
	new tmpstr[MAX_INPUT];
	new linemembercount;
	format(tmpstr, sizeof(tmpstr), "List of gang members in '%s':", gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_TITLE, tmpstr);
	
	for (new i; i < MAX_GANG_MEMBERS; i++) {
		if (!gMembers[cData[playerid][cGangID]][i][mName][0]) continue;
		
		if (linemembercount >= MAX_MEMBERS_PER_LINE) {
			SendClientMessage(playerid, COLOUR_HELP, linestr);
			strclr(linestr);
			linemembercount = 0;
		}
		
		if (!linestr[0]) {
			if (gMembers[cData[playerid][cGangID]][i][mIsLeader]) {
				format(linestr, sizeof(linestr), WRAPPED_MESSAGE_PREFIX "%s (Leader),", gMembers[cData[playerid][cGangID]][i][mName]);
				linemembercount++;
			} else {
				format(linestr, sizeof(linestr), WRAPPED_MESSAGE_PREFIX "%s,", gMembers[cData[playerid][cGangID]][i][mName]);
			}
		} else {
			if (gMembers[cData[playerid][cGangID]][i][mIsLeader]) {
				format(tmpstr, sizeof(tmpstr), "  %s (Leader),", gMembers[cData[playerid][cGangID]][i][mName]);
				linemembercount++;
			} else {
				format(tmpstr, sizeof(tmpstr), "  %s,", gMembers[cData[playerid][cGangID]][i][mName]);
			}
			strcat(linestr, tmpstr);
		}
		
		linemembercount++;
	}
	
	if (linemembercount) {
		linestr[strlen(linestr) - 1] = 0;
		SendClientMessage(playerid, COLOUR_HELP, linestr);
	}
	return 1;
}

kcmd:gangcolours(playerid, text[]) {
	SendClientMessage(playerid, COLOUR_TITLE, "List of gang colours:");
	
	for (new i; i < sizeof(gangColours); i++) {
		SendClientMessage(playerid, gangColours[i][cColourID], gangColours[i][cColourName]);
	}
	
	return 1;
}
	
kcmd:gangjoin(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG JOIN: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cInvitedGangID] == INVALID_GANG_ID) { // Player has not been invited
		SendClientMessage(playerid, COLOUR_ERROR, "GANG JOIN: You have not been invited into any gang.");
		return 1;
	}
	if (!gData[cData[playerid][cInvitedGangID]][gName][0]) { // Gang the player is trying to join does not exist
		SendClientMessage(playerid, COLOUR_ERROR, "GANG JOIN: The gang you are trying to join does not exist.");
		return 1;
	}
	if (cData[playerid][cGangID] != INVALID_GANG_ID) { // Player is already in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG JOIN: You are already in a gang.");
		return 1;
	}
	
	// Attempt to put the player into the gang
	new tmpstr[MAX_INPUT];
	if (!PutPlayerInGang(playerid, cData[playerid][cInvitedGangID], 0)) {
		format(tmpstr, sizeof(tmpstr), "GANG JOIN: The gang '%s' is full.", gData[cData[playerid][cInvitedGangID]][gName]);
		SendClientMessage(playerid, COLOUR_ERROR, tmpstr);
		return 1;
	}
	
	// Gang join successful
	format(tmpstr, sizeof(tmpstr), "GANG JOIN: You have joined the gang '%s'.", gData[cData[playerid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	
	format(tmpstr, sizeof(tmpstr), "%s has joined your gang '%s'.", pData[playerid][pLoginName], gData[cData[playerid][cGangID]][gName]);
	SendMessageToGang(cData[playerid][cGangID], tmpstr);
	return 1;
}

kcmd:gangquit(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) { // Player is not registered
		SendClientMessage(playerid, COLOUR_ERROR, "GANG QUIT: You must be logged in.");
		return 1;
	}
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG QUIT: You must in a gang to quit a gang.");
		return 1;
	}
	// Quit gang
	new gangname[MAX_GANG_NAME];
	strcpy(gangname, gData[cData[playerid][cGangID]][gName]);
	if (!RemovePlayerFromGang(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG QUIT: Unknown cause of error.");
		return 1;
	}
	
	// Quit gang successful
	new tmpstr[MAX_INPUT];
	format(tmpstr, sizeof(tmpstr), "GANG QUIT: You have left the gang '%s'", gangname);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	return 1;
}

gangs_OnPlayerCommandText(playerid, cmd[]) {
	// Prefix must be '/gang'
	if (strcmp("/gang", cmd, true, 5)) return 0;
	
	// Commands help, if /gang is in front and they don't get any of the commands right
	SendClientMessage(playerid, COLOUR_TITLE, "Gang Commands Help");
	SendClientMessage(playerid, COLOUR_HELP, "/gangcreate [colour] [gangname] - Create a gang");
	SendClientMessage(playerid, COLOUR_HELP, "/ganginvite [player] - Invite a player into your gang (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangkick [player] - Kick a player from your gang (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangleadergive [player] - Promote a gang member to leader (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangleadertake [player] - Demote a gang member to member (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangchangecolour [colour] - Change the colour of your gang (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangchatcolour - Toggle using the gang colour for gang chat (leader only)");
	SendClientMessage(playerid, COLOUR_HELP, "/gangmembers - A list of the current gang members in your gang");
	SendClientMessage(playerid, COLOUR_HELP, "/gangcolours - List of avaliable gang colours");
	SendClientMessage(playerid, COLOUR_HELP, "/gangjoin - Join the gang that invited you last");
	SendClientMessage(playerid, COLOUR_HELP, "/gangquit - Quit your gang");
	SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	return 1;
}

//-----------------------------------------------------------------------------------------------------------------

gangs_OnPlayerText(playerid, text[]) {
	if (text[0] != '!') return 1; // Not gang chat
	strmid(text, text, 1, strlen(text), strlen(text));
	SendPrivateMessageToGang(playerid, text);
	return 0;
}

kcmd:g(playerid, text[]) {
	SendPrivateMessageToGang(playerid, text);
	return 1;
}

SendPrivateMessageToGang(playerid, text[]) {
	if (cData[playerid][cGangID] == INVALID_GANG_ID) { // Player is not in a gang
		SendClientMessage(playerid, COLOUR_ERROR, "GANG CHAT: You are not in a gang");
		return 0;
	}
	
	new message[MAX_INPUT];
	
	format(message, sizeof(message), "<GangChat> %s: %s", pData[playerid][pLoginName], text);
	if (gData[cData[playerid][cGangID]][gChatUseColour]) {
		SendMessageToGang(cData[playerid][cGangID], message, gData[cData[playerid][cGangID]][gColour]);
	} else {
		SendMessageToGang(cData[playerid][cGangID], message);
	}
	
	printf("[FDM] %s", message);
	return 0;
}

SendMessageToGang(gangid, message[], colour = COLOUR_GANGCHAT) {
	if (!gData[gangid][gName][0]) return 0; // Invalid gang
	
	new Bit:players[Bit_Bits(MAX_PLAYERS)];
	LoopPlayers(i) {
		if (cData[i][cGangID] == gangid) { // Same gang
			Bit_Set(players, i, 1, sizeof(players));
		}
	}
	SendWrappedMessageToClients(players, colour, message);
	return 1;
}
