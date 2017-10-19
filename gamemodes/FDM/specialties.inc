#include <a_samp>

#define FILE_SPECIALTIES "FDM/Settings/Specialties.ini"
#define COLOUR_SPECIALTY 0xFF8010AA

#define SP_NAME_MAX 24

#define SP_TYPES_MAX 12
#define SP_ABILITY_MAX 8
#define SP_ABILITY_LEVELS_MAX 12

#define SP_TYPE_INVALID -1
#define SP_ABILITY_LEVEL_INVALID -1
#define SP_ABILITY_INVALID -1

#define SP_ABILITY_HEAL 0
#define SP_ABILITY_HEALSELF 1
#define SP_ABILITY_GIVELIFE 2
#define SP_ABILITY_REPAIR 3
#define SP_ABILITY_REPAIRDRIVER 4
#define SP_ABILITY_TARGETPLAYER 5
#define SP_ABILITY_STEALTH 6
#define SP_ABILITY_RAPIDREPAIR 7

enum specialtyPlayerData {
	spType,
	spTextDrawExist,
	Text:spTextDraw
}
enum specialtyTypeData {
	stName[SP_NAME_MAX],
	stMinLevel,
	stAbilities[SP_ABILITY_MAX],
	stText[MAX_INPUT]
}
enum specialAbilitiesData {
	saLevels[SP_ABILITY_LEVELS_MAX]
}
enum specialAbilitiesNamesData {
	snShortName[SP_NAME_MAX],
	snLongName[MAX_INPUT]
}
new sTypeCount;
new sppData[MAX_PLAYERS][specialtyPlayerData];
new sTypes[SP_TYPES_MAX][specialtyTypeData];
new sAbilities[SP_ABILITY_MAX][specialAbilitiesData];
new sAbilityNames[SP_ABILITY_MAX][specialAbilitiesNamesData] = {
	{"Heal", "Heal Another Player"},
	{"HealSelf", "Heal Yourself"},
	{"GiveLife", "Give Life"}, 
	{"Repair", "Repair As Passenger"},
	{"RepairDriver", "Repair As Driver"},
	{"TargetPlayer", "Target Player"},
	{"Stealth", "Stealth"},
	{"RapidRepair", "Rapid Repair"}
};

#include "Specialties\heal.inc"
#include "Specialties\repairvehicle.inc"
#include "Specialties\target.inc"
#include "Specialties\stealthing.inc" //TODO

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

SpecialtyInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		SpecialtyResetPlayerData(i);
		if (IsPlayerConnected(i)) {
			if (!sppData[i][spTextDrawExist]) {
				sppData[i][spTextDrawExist] = 1;
				sppData[i][spTextDraw] = TextDrawCreate(320.0, 280.0, " ");
				TextDrawAlignment(sppData[i][spTextDraw], 2);
			}
			TextDrawShowForPlayer(i, sppData[i][spTextDraw]);
		}
	}
	LoadSpecialties();
	HealInit();
	RepairInit();
	TargetInit();
	StealthingInit();
	
	LoopPlayers(i) {
		if (pData[i][pIsLoggedIn]) {
			specialty_OnPlayerLogin(i);
		}
	}
	
	return 1;
}

SpecialtyExit() {
	HealExit();
	RepairExit();
	TargetExit();
	StealthingExit();
	
	new tmp[specialtyPlayerData];
	new tmp2[specialtyTypeData];
	new tmp3[specialAbilitiesData];
	
	LoopPlayers(i) {
		if (sppData[i][spTextDrawExist]) {
			sppData[i][spTextDrawExist] = 0;
			TextDrawDestroy(sppData[i][spTextDraw]);
		}
		sppData[i] = tmp;
	}
	for (new i; i < SP_TYPES_MAX; i++) {
		sTypes[i] = tmp2;
	}
	for (new i; i < SP_ABILITY_MAX; i++) {
		sAbilities[i] = tmp3;
	}
	
	return 1;
}

LoadSpecialties() {
	if (!INI_Exist(FILE_SPECIALTIES)) {
		printf("[FDM] WARNING: Specialties Load Fail: '%s' - File not found.", FILE_SPECIALTIES);
		return 0;
	}
	
	if (!INI_Open(FILE_SPECIALTIES)) {
		printf("[FDM] WARNING: Specialties Load Fail: '%s' - File cannot be opened.", FILE_SPECIALTIES);
		return 0;
	}
	
	new idx,
		tmpstr[MAX_STRING],
		tmpstr2[MAX_INPUT];
	
	// Load types of specialty types
	sTypeCount = 0;
	for (new i; i < SP_TYPES_MAX; i++) {
		format(tmpstr2, sizeof(tmpstr2), "SpecialtyType%i", i + 1);
		if (!INI_ReadString(tmpstr, tmpstr2) || !tmpstr[0]) {
			sTypes[i][stMinLevel] = SP_ABILITY_LEVEL_INVALID;
			continue;
		}
		idx = 0;
		
		sTypes[i][stMinLevel] = strval(split(tmpstr, idx, ','));
		splitcpy(sTypes[i][stName], tmpstr, idx, ',', SP_NAME_MAX);
		
		for (new j; j < SP_ABILITY_MAX; j++) {
			splitcpy(tmpstr2, tmpstr, idx, ',');
			if (!tmpstr2[0]) continue;
			
			new abilityid = GetAbilityFromShortName(tmpstr2);
			if (abilityid != SP_ABILITY_INVALID) {
				sTypes[i][stAbilities][abilityid] = 1;
			}
		}
		
		sTypeCount++;
		
		format(tmpstr2, sizeof(tmpstr2), "SpecialtyType%iText", i + 1);
		if (!INI_ReadString(tmpstr, tmpstr2) || !tmpstr[0]) {
			strclr(sTypes[i][stText]);
			continue;
		}
		
		strcpy(sTypes[i][stText], tmpstr, MAX_INPUT);
	}
	
	// Load ability level data
	for (new i; i < SP_ABILITY_MAX; i++) {
		format(tmpstr2, sizeof(tmpstr2), "%sLevels", sAbilityNames[i][snShortName]);
		if (!INI_ReadString(tmpstr, tmpstr2)) {
			sAbilities[i][saLevels][0] = SP_ABILITY_LEVEL_INVALID;
			continue;
		}
		idx = 0;
		
		for (new j; j < SP_ABILITY_LEVELS_MAX; j++) {
			splitcpy(tmpstr2, tmpstr, idx, ',');
			if (!tmpstr2[0]) {
				sAbilities[i][saLevels][j] = SP_ABILITY_LEVEL_INVALID;
				break;
			}
			
			sAbilities[i][saLevels][j] = strval(tmpstr2);
		}
	}
	
	return 1;
}

GetAbilityFromShortName(const text[]) {
	for (new i; i < SP_ABILITY_MAX; i++) {
		if (sAbilityNames[i][snShortName][0] && !strcmp(sAbilityNames[i][snShortName], text, true)) {
			return i;
		}
	}
	return SP_ABILITY_INVALID;
}

specialty_OnPlayerConnect(playerid) {
	SpecialtyResetPlayerData(playerid);
	
	sppData[playerid][spTextDrawExist] = 1;
	sppData[playerid][spTextDraw] = TextDrawCreate(320.0, 280.0, " ");
	TextDrawAlignment(sppData[playerid][spTextDraw], 2);
	
	TextDrawShowForPlayer(playerid, sppData[playerid][spTextDraw]);
	return 1;
}

specialty_OnPlayerDisconnect(playerid) {
	SpecialtyResetPlayerData(playerid);
	return 1;
}

SpecialtyResetPlayerData(playerid) {
	if (sppData[playerid][spTextDrawExist]) {
		sppData[playerid][spTextDrawExist] = 0;
		TextDrawDestroy(sppData[playerid][spTextDraw]);
	}
	new tmp[specialtyPlayerData];
	sppData[playerid] = tmp;
	sppData[playerid][spType] = SP_TYPE_INVALID;
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// ABILITIES FUNCTIONS
//-----------------------------------------------------------------------------------------------------

IsPlayerAbilityAllowed(playerid, ability) {
	if (sppData[playerid][spType] == SP_TYPE_INVALID || // Invalid specialty type
		!sTypes[sppData[playerid][spType]][stAbilities][ability] || // Ability not allowed for player specialty type
		sTypes[sppData[playerid][spType]][stMinLevel] == SP_ABILITY_LEVEL_INVALID || // Player specialty type not allowed
		(pData[playerid][pLevel] < sTypes[sppData[playerid][spType]][stMinLevel]) || // Player level too low for specialty type
		sAbilities[ability][saLevels][0] == SP_ABILITY_LEVEL_INVALID || // Ability not allowed
		sAbilities[ability][saLevels][0] > pData[playerid][pLevel] // Player level too low for ability
		) {
		
		return 0;
	} else {
		return 1;
	}
}

GetPlayerAbilityLevel(playerid, ability) {
	if (sAbilities[ability][saLevels][0] == SP_ABILITY_LEVEL_INVALID) return 0;
	for (new i; i < SP_ABILITY_LEVELS_MAX; i++) {
		if (pData[playerid][pLevel] < sAbilities[ability][saLevels][i] || sAbilities[ability][saLevels][i] == -1) {
			return i;
		}
	}
	return 0;
}

SpecialtyAllowedCheck(playerid, ability, action[]) {
	new msg[MAX_INPUT];
	if (!IsPlayerAbilityAllowed(playerid, ability)) {
		if (sAbilities[ability][saLevels][0] == SP_ABILITY_LEVEL_INVALID) { // Ability is not allowed
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: This ability is disabled.");
			
		} else if (sppData[playerid][spType] == SP_TYPE_INVALID || !sTypes[sppData[playerid][spType]][stAbilities][ability]) { // Wrong player specialty
			if (GetAbilitySpecialtyList(ability, msg)) {
				if (IsVowel(msg[0])) {
					format(msg, sizeof(msg), "SPECIALTIES: You must be an %s to %s.", msg, action);
				} else {
					format(msg, sizeof(msg), "SPECIALTIES: You must be a %s to %s.", msg, action);
				}
				SendClientMessage(playerid, COLOUR_ERROR, msg);
			} else {
				format(msg, sizeof(msg), "SPECIALTIES: You are not allowed to %s.", action);
				SendClientMessage(playerid, COLOUR_ERROR, msg);
			}
			
		} else if (pData[playerid][pLevel] < sAbilities[ability][saLevels][0]) { // Player not high level enough
			format(msg, sizeof(msg), "SPECIALTIES: You must be level %i to %s.", sAbilities[ability][saLevels][0], action);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			
		} else { // Other error
			format(msg, sizeof(msg), "SPECIALTIES: You are not allowed to %s.", action);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
		}
		return 0;
	} else { // Successful
		new level = GetPlayerAbilityLevel(playerid, ability);
		if (!level) {
			format(msg, sizeof(msg), "SPECIALTIES: You are not allowed to %s.", action);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 0;
		} else {
			return level;
		}
	}
}

//-----------------------------------------------------------------------------------------------------
// ABILITIES HANDLING
//-----------------------------------------------------------------------------------------------------

ShowSpecialtyList(playerid) {
	new msg[MAX_INPUT];
	for (new i; i < SP_TYPES_MAX; i++) {
		if (sTypes[i][stMinLevel] != SP_ABILITY_LEVEL_INVALID) {
			/*if (sTypes[i][stText][0]) {
				format(msg, sizeof(msg), "%s - %s (", sTypes[i][stName], sTypes[i][stText]);
			} else {
				format(msg, sizeof(msg), "%s - ", sTypes[i][stName]);
			}
			
			for (new j; j < SP_ABILITY_MAX; j++) {
				if (sTypes[i][stAbilities][j]) {
					strcat(msg, sAbilityNames[j][snLongName]);
					strcat(msg, ", ");
				}
			}
			new len = strlen(msg);
			msg[len - 2] = ')';
			msg[len - 1] = 0;*/
			
			if (sTypes[i][stText][0]) {
				format(msg, sizeof(msg), "%s - %s", sTypes[i][stName], sTypes[i][stText]);
			} else {
				strcpy(msg, sTypes[i][stName]);
			}
			
			SendClientMessage(playerid, COLOUR_HELP, msg);
		}
	}
}

ShowSpecialtyListShort(playerid) {
	new msg[MAX_INPUT];
	for (new i; i < SP_TYPES_MAX; i++) {
		if (sTypes[i][stMinLevel] != SP_ABILITY_LEVEL_INVALID) {
			strcat(msg, sTypes[i][stName]);
			strcat(msg, ", ");
		}
	}
	
	msg[strlen(msg) - 2] = 0;
	
	SendClientMessage(playerid, COLOUR_HELP, msg);
	return 1;
}

GetAbilitySpecialtyList(ability, list[], maxlen = sizeof(list)) {
	strclr(list);
	
	for (new i; i < SP_TYPES_MAX; i++) {
		if (sTypes[i][stMinLevel] == SP_ABILITY_LEVEL_INVALID) continue;
		
		if (sTypes[i][stAbilities][ability]) {
			strcat(list, sTypes[i][stName], maxlen);
			strcat(list, "/", maxlen);
			continue;
		}
	}
	
	list[strlen(list) - 1] = 0;
	
	if (list[0]) {
		return 1;
	} else {
		return 0;
	}
}

specialty_OnPlayerLogin(playerid) {
	if (sTypeCount) {
		sppData[playerid][spType] = pData[playerid][pSpecialty] - 1;
		if (sppData[playerid][spType] < 0 || sppData[playerid][spType] >= sTypeCount) {
			// Invalid specialty
			sppData[playerid][spType] = SP_TYPE_INVALID;
			pData[playerid][pSpecialty] = 0;
			
			SendClientMessage(playerid, COLOUR_SPECIALTY, "SPECIALTIES: You do not have a specialty. Choose one with /specialty [type].");
		}
	}
	return 1;
}

specialty_OnPlayerSpawn(playerid) {
	if (sppData[playerid][spType] != pData[playerid][pSpecialty] - 1) {
		new msg[MAX_INPUT];
		sppData[playerid][spType] = pData[playerid][pSpecialty] - 1;
		
		if (sppData[playerid][spType] == SP_TYPE_INVALID) {
			SendClientMessage(playerid, COLOUR_SPECIALTY, "SPECIALTIES: You no longer have a specialty.");
		} else {
			if (IsVowel(sTypes[sppData[playerid][spType]][stName][0])) {
				if (sTypes[sppData[playerid][spType]][stText][0]) {
					format(msg, sizeof(msg), "SPECIALTIES: You now specialise as an %s - %s", sTypes[sppData[playerid][spType]][stName], sTypes[sppData[playerid][spType]][stText]);
				} else {
					format(msg, sizeof(msg), "SPECIALTIES: You now specialise as an %s.", sTypes[sppData[playerid][spType]][stName]);
				}
			} else {
				if (sTypes[sppData[playerid][spType]][stText][0]) {
					format(msg, sizeof(msg), "SPECIALTIES: You now specialise as a %s - %s", sTypes[sppData[playerid][spType]][stName], sTypes[sppData[playerid][spType]][stText]);
				} else {
					format(msg, sizeof(msg), "SPECIALTIES: You now specialise as a %s.", sTypes[sppData[playerid][spType]][stName]);
				}
			}
			SendClientMessage(playerid, COLOUR_SPECIALTY, msg);
		}
	}
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// SPECTATE SUPPORT
//-----------------------------------------------------------------------------------------------------

specialty_OnPlayerSpectate(playerid, specid) {
	if (sppData[playerid][spTextDrawExist]) {
		TextDrawHideForPlayer(playerid, sppData[playerid][spTextDraw]);
	}
	if (sppData[specid][spTextDrawExist]) {
		TextDrawShowForPlayer(playerid, sppData[specid][spTextDraw]);
	}
	return 1;
}

spl_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (sppData[oldspecid][spTextDrawExist]) {
		TextDrawHideForPlayer(playerid, sppData[oldspecid][spTextDraw]);
	}
	if (sppData[newspecid][spTextDrawExist]) {
		TextDrawShowForPlayer(playerid, sppData[newspecid][spTextDraw]);
	}
	return 1;
}

specialty_OnPlayerUnspectate(playerid, specid) {
	if (sppData[playerid][spTextDrawExist]) {
		TextDrawShowForPlayer(playerid, sppData[playerid][spTextDraw]);
	}
	if (sppData[specid][spTextDrawExist]) {
		TextDrawHideForPlayer(playerid, sppData[specid][spTextDraw]);
	}
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:specialty(playerid, text[]) {
	if (!sTypeCount) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: This command is disabled.");
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid specialty. Specialty list:");
		ShowSpecialtyListShort(playerid);
		return 1;
	}
	
	new specialtytype = SP_TYPE_INVALID;
	for (new i; i < SP_TYPES_MAX; i++) {
		if (sTypes[i][stMinLevel] != SP_ABILITY_LEVEL_INVALID &&
			strfind(sTypes[i][stName], text, true) != -1) {
			
			specialtytype = i;
			break;
		}
	}
	
	if (specialtytype == SP_TYPE_INVALID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid specialty. Specialty list:");
		ShowSpecialtyListShort(playerid);
		return 1;
	}
	
	new msg[MAX_INPUT];
	if (pData[playerid][pLevel] < sTypes[specialtytype][stMinLevel]) {
		if (IsVowel(sTypes[specialtytype][stName][0])) {
			format(msg, sizeof(msg), "SPECIALTIES: You must be at least level %i to be an %s.", sTypes[specialtytype][stMinLevel], sTypes[specialtytype][stName]);
		} else {
			format(msg, sizeof(msg), "SPECIALTIES: You must be at least level %i to be a %s.", sTypes[specialtytype][stMinLevel], sTypes[specialtytype][stName]);
		}
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	} else if (specialtytype == sppData[playerid][spType] && specialtytype == pData[playerid][pSpecialty] - 1) {
		if (IsVowel(sTypes[specialtytype][stName][0])) {
			format(msg, sizeof(msg), "SPECIALTIES: You are already an %s.", sTypes[specialtytype][stName]);
		} else {
			format(msg, sizeof(msg), "SPECIALTIES: You are already a %s.", sTypes[specialtytype][stName]);
		}
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		return 1;
	} else if (specialtytype == pData[playerid][pSpecialty] - 1) {
		if (IsVowel(sTypes[specialtytype][stName][0])) {
			format(msg, sizeof(msg), "SPECIALTIES: You are already set to spawn as an %s.", sTypes[specialtytype][stName]);
		} else {
			format(msg, sizeof(msg), "SPECIALTIES: You are already set to spawn as a %s.", sTypes[specialtytype][stName]);
		}
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		return 1;
	}
	
	if (IsVowel(sTypes[specialtytype][stName][0])) {
		format(msg, sizeof(msg), "SPECIALTIES: You will specialise as an %s next time you spawn.", sTypes[specialtytype][stName]);
	} else {
		format(msg, sizeof(msg), "SPECIALTIES: You will specialise as a %s next time you spawn.", sTypes[specialtytype][stName]);
	}
	
	pData[playerid][pSpecialty] = specialtytype + 1;
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	return 1;
}

kcmd:specialtylist(playerid, text[]) {
	if (!sTypeCount) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: This command is disabled.");
		return 1;
	}
	
	SendClientMessage(playerid, COLOUR_TITLE, "Specialty List:");
	ShowSpecialtyList(playerid);
	return 1;
}

kcmd:specialtyinfo(playerid, text[]) {
	if (!sTypeCount) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: This command is disabled.");
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid specialty. Specialty list:");
		ShowSpecialtyListShort(playerid);
		return 1;
	}
	
	new specialtytype = SP_TYPE_INVALID;
	for (new i; i < SP_TYPES_MAX; i++) {
		if (sTypes[i][stMinLevel] != SP_ABILITY_LEVEL_INVALID &&
			strfind(sTypes[i][stName], text, true) != -1) {
			
			specialtytype = i;
			break;
		}
	}
	
	if (specialtytype == SP_TYPE_INVALID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid specialty. Specialty list:");
		ShowSpecialtyListShort(playerid);
		return 1;
	}
	
	new msg[MAX_INPUT];
	format(msg, sizeof(msg), "Specialty Information for %s", sTypes[specialtytype][stName]);
	SendClientMessage(playerid, COLOUR_TITLE, msg);
	
	format(msg, sizeof(msg), "Min. Level: %s", sTypes[specialtytype][stMinLevel]);
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	format(msg, sizeof(msg), "Description: %s", sTypes[specialtytype][stText]);
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	new abilitycount;
	for (new i; i < SP_ABILITY_MAX; i++) {
		if (sTypes[specialtytype][stAbilities][i] && sAbilities[i][saLevels][0] != SP_ABILITY_LEVEL_INVALID) {
			
			if (!abilitycount) {
				SendClientMessage(playerid, COLOUR_HELP, "Abilities:");
			}
			abilitycount++;
			
			new currentlevel;
			new maxlevel;
			
			for (new j; j < SP_ABILITY_LEVELS_MAX; j++) {
				if (!currentlevel && (pData[playerid][pLevel] < sAbilities[i][saLevels][j] || sAbilities[i][saLevels][j] == -1)) {
					currentlevel = j;
					maxlevel = j;
				} else if (sAbilities[i][saLevels][j] == -1) {
					maxlevel = j;
					break;
				}
			}
			
			format(msg, sizeof(msg), WRAPPED_MESSAGE_PREFIX "%s (Current Level: %i / Max Level: %i)",
				sAbilityNames[i][snLongName], currentlevel, maxlevel);
			
			SendClientMessage(playerid, COLOUR_HELP, msg);
		}
	}
	
	if (abilitycount > 8) {
		SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	}
	
	return 1;
}
