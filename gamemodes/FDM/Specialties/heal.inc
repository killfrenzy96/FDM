#include <a_samp>

#define HEAL_MAX_DISTANCE 2.0

enum playerHealData {
	phHealing,
	phHealPlayer,
	//phHealedBy,
	phLastMove,
	Float:phHealSpeed
}
new phData[MAX_PLAYERS][playerHealData];
// new healTimer;

HealInit() {
	kEnableTimer(18); // healTimer = SetTimer("fdm_HealTimer", 750, true);
	return 1;
}

HealExit() {
	kDisableTimer(18); // KillTimer(healTimer);
	LoopPlayers(i) {
		PlayerStopHealing(i);
	}
	return 1;
}

heal_OnPlayerDisconnect(playerid) {
	phData[playerid][phLastMove] = 0;
	if (phData[playerid][phHealing]) {
		PlayerStopHealing(playerid);
	}
	return 1;
}

heal_OnPlayerStateChange(playerid, newstate, oldstate) {
	#pragma unused newstate
	
	if (oldstate == PLAYER_STATE_ONFOOT && phData[playerid][phHealing]) {
		PlayerStopHealing(playerid);
	}
	return 1;
}

heal_OnPlayerUpdate(playerid) {
	if (phData[playerid][phHealing]) {
		new keys, leftright, updown;
		GetPlayerKeys(playerid, keys, leftright, updown);
		
		if (leftright || updown) {
			PlayerStopHealing(playerid);
		}
	}
}

heal_OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	if ((newkeys & KEY_SPRINT || newkeys & KEY_FIRE || newkeys & 128) && phData[playerid][phHealing]) {
		if (phData[playerid][phHealing]) {
			PlayerStopHealing(playerid);
		}
	}
	if (oldkeys & KEY_SPRINT || oldkeys & KEY_FIRE || oldkeys & 128) {
		phData[playerid][phLastMove] = GetTickCount();
	}
	return 1;
}

PlayerStartHealingPlayer(playerid, healid, Float:speed) {
	if (phData[playerid][phHealing]) PlayerStopHealing(playerid);
	
	new msg[MAX_INPUT], Float:health;
	GetPlayerHealth(healid, health);
	format(msg, sizeof(msg), "~r~Health: ~w~%0.1f", health);
	TextDrawSetString(sppData[playerid][spTextDraw], msg);
	
	if (playerid == healid) {
		phData[playerid][phHealing] = 1;
		phData[playerid][phHealSpeed] = speed;
		phData[playerid][phHealPlayer] = playerid;
		//phData[playerid][phHealedBy] = INVALID_PLAYER_ID;
		
		//ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.0, 0, 0, 0, 0, 0);
		ApplyAnimation(playerid, "PARK", "Tai_Chi_Loop", 4.0, 1, 0, 0, 0, 0);
	} else {
		phData[playerid][phHealing] = 1;
		phData[healid][phHealing] = 1;
		phData[playerid][phHealSpeed] = speed;
		
		phData[playerid][phHealPlayer] = healid;
		//phData[playerid][phHealedBy] = INVALID_PLAYER_ID;
		
		phData[healid][phHealPlayer] = INVALID_PLAYER_ID;
		//phData[healid][phHealedBy] = playerid;
		
		new Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2;
		GetPlayerPos(playerid, x1, y1, z1);
		//SetPlayerPos(playerid, x1, y1, z1);
		GetPlayerPos(healid, x2, y2, z2);
		//SetPlayerPos(healid, x2, y2, z2);
		
		new Float:angle = ReturnAngle(x1, y1, x2, y2);
		SetPlayerFacingAngle(playerid, angle - 90.0);
		//SetPlayerFacingAngle(healid, angle + 90.0);
		
		TextDrawSetString(sppData[healid][spTextDraw], msg);
		
		ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.0, 0, 0, 0, 0, 0);
		ApplyAnimation(healid, "CRACK", "crckdeth2", 4.0, 1, 0, 0, 0, 0);
	}
	
	return 1;
}

PlayerStopHealing(playerid) {
	if (!phData[playerid][phHealing]) return 1;
	
	phData[playerid][phHealing] = 0;
	if (!IsPlayerInAnyVehicle(playerid)) ClearAnimations(playerid);
	TextDrawSetString(sppData[playerid][spTextDraw], " ");
	
	if (phData[playerid][phHealPlayer] != INVALID_PLAYER_ID) {
		new healCount;
		LoopPlayers(i) {
			if (phData[i][phHealing] && phData[i][phHealPlayer] == phData[playerid][phHealPlayer]) {
				healCount++;
			}
		}
		if (!healCount) {
			PlayerStopHealing(phData[playerid][phHealPlayer]);
		}
		/*phData[phData[playerid][phHealPlayer]][phHealing] = 0;
		if (!IsPlayerInAnyVehicle(phData[playerid][phHealPlayer])) ClearAnimations(phData[playerid][phHealPlayer]);
		TextDrawSetString(sppData[phData[playerid][phHealPlayer]][spTextDraw], " ");*/
	}
	//if (phData[playerid][phHealedBy] != INVALID_PLAYER_ID) {
	//	PlayerStopHealing(phData[playerid][phHealedBy]);
		/*phData[phData[playerid][phHealedBy]][phHealing] = 0;
		if (!IsPlayerInAnyVehicle(phData[playerid][phHealedBy])) ClearAnimations(phData[playerid][phHealedBy]);
		TextDrawSetString(sppData[phData[playerid][phHealedBy]][spTextDraw], " ");*/
	//}
	LoopPlayers(i) {
		if (phData[i][phHealing] && phData[i][phHealPlayer] == playerid) {
			PlayerStopHealing(i);
		}
	}
	return 1;
}

forward fdm_HealTimer();
public fdm_HealTimer() {
	LoopPlayers(i) {
		if (phData[i][phHealing] && phData[i][phHealPlayer] != INVALID_PLAYER_ID) {
			
			if (i != phData[i][phHealPlayer]) {
				if (GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(phData[i][phHealPlayer])) {
					PlayerStopHealing(i);
					continue;
				}
				
				new Float:x, Float:y, Float:z;
				GetPlayerPos(i, x, y, z);
				if (!IsPlayerInRangeOfPoint(phData[i][phHealPlayer], HEAL_MAX_DISTANCE, x, y, z)) {
					PlayerStopHealing(i);
					continue;
				}
			}
			
			if (IsPlayerPaused(i, 5000) || IsPlayerPaused(phData[i][phHealPlayer], 5000)) {
				PlayerStopHealing(i);
				continue;
			}
			
			new keys, leftright, updown;
			GetPlayerKeys(i, keys, leftright, updown);
			if (leftright || updown) {
				PlayerStopHealing(i);
				continue;
			}
			
			GetPlayerKeys(phData[i][phHealPlayer], keys, leftright, updown);
			if (leftright || updown) {
				PlayerStopHealing(i);
				continue;
			}
			
			new Float:maxhp = 100.0;
			if (IsPlayerInMinimode(phData[i][phHealPlayer])) {
				if (modes[mpData[phData[i][phHealPlayer]][mpModeID]][mType] == MODE_TYPE_DMFFA) {
					maxhp = dmffaMaps[modes[mpData[phData[i][phHealPlayer]][mpModeID]][mMapID]][dmfPlayerHealth];
				} else if (modes[mpData[phData[i][phHealPlayer]][mpModeID]][mType] == MODE_TYPE_DMTEAM) {
					maxhp = dmtMaps[modes[mpData[phData[i][phHealPlayer]][mpModeID]][mMapID]][dmtPlayerHealth];
				}
			}
			
			new Float:playerhealth;
			GetPlayerHealth(phData[i][phHealPlayer], playerhealth);
			if (playerhealth >= maxhp) {
				PlayerStopHealing(i);
				continue;
			}
			
			playerhealth += phData[i][phHealSpeed];
			if (playerhealth >= maxhp) playerhealth = maxhp;
			
			SetPlayerHealth(phData[i][phHealPlayer], playerhealth);
			
			if (playerhealth >= maxhp) {
				PlayerStopHealing(i);
				continue;
			}
			
			new msg[MAX_INPUT];
			format(msg, sizeof(msg), "~r~Health: ~w~%0.1f", playerhealth);
			TextDrawSetString(sppData[i][spTextDraw], msg);
			
			if (i == phData[i][phHealPlayer]) {
				ApplyAnimation(i, "PARK", "Tai_Chi_Loop", 4.0, 1, 0, 0, 0, 0);
			} else {
				TextDrawSetString(sppData[phData[i][phHealPlayer]][spTextDraw], msg);
				ApplyAnimation(i, "BOMBER", "BOM_Plant", 4.0, 0, 0, 0, 0, 0);
				ApplyAnimation(phData[i][phHealPlayer], "CRACK", "crckdeth2", 4.0, 1, 0, 0, 0, 0);
			}
		}
	}
	return 1;
}

GetPlayerFacingPlayer(playerid, Float:maxdistance = 100.0) {
	
	if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return INVALID_PLAYER_ID;
	
	new Float:px, Float:py, Float:pz, Float:protz, pworld,
		Float:tx, Float:ty, Float:tz,
		Float:distance;
	
	GetPlayerPos(playerid, px, py, pz);
	GetPlayerFacingAngle(playerid, protz);
	pworld = GetPlayerVirtualWorld(playerid);
	
	new closestplayer = INVALID_PLAYER_ID;
	new Float:closest;
	
	LoopPlayers(i) {
		if (playerid != i && GetPlayerState(i) == PLAYER_STATE_ONFOOT && GetPlayerVirtualWorld(i) == pworld) {
			GetPlayerPos(i, tx, ty, tz);
			distance = FindDistance(px, py, pz, tx, ty, tz);
			
			if (distance > maxdistance) continue;
			
			// Angle check
			new Float:angle = ReturnAngle(px, py, tx, ty) - 90.0,
				Float:angledifference = AngDif(protz, angle);
			
			if (angledifference < 0.0) angledifference = angledifference * -1;
			if (angledifference == 0.0) angledifference = 0.00001;
			
			new Float:multiplier = 300.0;
			if (IsPlayerInAnyVehicle(i)) multiplier = 500.0;
			
			if ((distance < (1.0 / angledifference) * multiplier) &&
				(closestplayer == INVALID_PLAYER_ID || angledifference < closest)) {
				
				closest = angledifference;
				closestplayer = i;
			}
		}
	}
	
	return closestplayer;
}

kcmd:h(playerid, text[]) return kcmd_heal(playerid, text);
kcmd:heal(playerid, text[]) {
	new msg[MAX_INPUT];
	
	if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must be spawned and on foot to heal yourself.");
		return 1;
	}
	
	new healid;
	if (!isnull(text)) {
		healid = FindPlayer(text);
		if (healid == INVALID_PLAYER_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid player ID/Name.");
			return 1;
		}
		
		if (healid != playerid) {
			if (GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(healid)) {
				GetPlayerName(healid, msg, sizeof(msg));
				format(msg, sizeof(msg), "SPECIALTIES: %s is not in the world you are in.", msg);
				SendClientMessage(playerid, COLOUR_ERROR, msg);
				return 1;
			}
			
			new Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2, Float:distance;
			GetPlayerPos(playerid, x1, y1, z1);
			GetPlayerPos(healid, x2, y2, z2);
			distance = FindDistance(x1, y1, z1, x2, y2, z2);
			if (distance > HEAL_MAX_DISTANCE) {
				GetPlayerName(healid, msg, sizeof(msg));
				format(msg, sizeof(msg), "SPECIALTIES: %s is too far to heal.", msg);
				SendClientMessage(playerid, COLOUR_ERROR, msg);
				return 1;
			}
			
			if (GetPlayerState(healid) != PLAYER_STATE_ONFOOT) {
				GetPlayerName(healid, msg, sizeof(msg));
				format(msg, sizeof(msg), "SPECIALTIES: %s must be on foot.", msg);
				SendClientMessage(playerid, COLOUR_ERROR, msg);
				return 1;
			}
		}
	} else {
		healid = GetPlayerFacingPlayer(playerid, HEAL_MAX_DISTANCE);
		if (healid != INVALID_PLAYER_ID) {
			new modeid = mpData[healid][mpModeID];
			if (modeid != INVALID_MODE_ID) {
				if (mpData[playerid][mpModeID] == modeid) {
					if (modes[modeid][mType] == MODE_TYPE_DMFFA) {
						healid = INVALID_PLAYER_ID;
						
					} else if (modes[modeid][mType] == MODE_TYPE_DMTEAM &&
						dmtPStats[playerid][dmtpTeamID] != dmtPStats[healid][dmtpTeamID]) {
						
						healid = INVALID_PLAYER_ID;
					}
				}
			}
		}
	}
	
	new ability, level;
	
	if (healid == INVALID_PLAYER_ID || healid == playerid) {
		level = SpecialtyAllowedCheck(playerid, SP_ABILITY_HEALSELF, "heal yourself");
		if (!level) return 1;
		
		healid = playerid;
		ability = SP_ABILITY_HEALSELF;
		
	} else {
		level = SpecialtyAllowedCheck(playerid, SP_ABILITY_HEAL, "heal another player");
		if (!level) return 1;
		
		new keys, leftright, updown;
		GetPlayerKeys(healid, keys, leftright, updown);
		
		if ((phData[healid][phLastMove] && GetTickCount() - phData[healid][phLastMove] < 500) ||
			keys & 128 || keys & KEY_SPRINT || keys & KEY_FIRE || leftright || updown) {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: %s is busy.", msg);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 1;
		}
		
		ability = SP_ABILITY_HEAL;
	}
	
	if (phData[playerid][phHealing] && healid == phData[playerid][phHealPlayer]) {
		if (healid == playerid) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: You are already healing yourself.");
		} else {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: You are already healing %s.", msg);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		}
		return 1;
	}
	
	if (GetPlayerState(healid) != PLAYER_STATE_ONFOOT) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s must be spawned and on foot to heal.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (IsPlayerPaused(playerid, 5000)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must not be paused.");
		return 1;
	}
	
	if (IsPlayerPaused(healid, 5000)) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s must not be paused.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	new Float:maxhp = 100.0;
	if (IsPlayerInMinimode(healid)) {
		if (modes[mpData[healid][mpModeID]][mType] == MODE_TYPE_DMFFA) {
			maxhp = dmffaMaps[modes[mpData[healid][mpModeID]][mMapID]][dmfPlayerHealth];
		} else if (modes[mpData[healid][mpModeID]][mType] == MODE_TYPE_DMTEAM) {
			maxhp = dmtMaps[modes[mpData[healid][mpModeID]][mMapID]][dmtPlayerHealth];
		}
	}
	
	new Float:health;
	GetPlayerHealth(healid, health);
	if (health >= maxhp) {
		if (playerid == healid) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: You are already full health.");
		} else {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: %s is already full health.", msg);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		}
		return 1;
	}
	
	PlayerStartHealingPlayer(playerid, healid, float(level));
	
	if (healid == playerid) {
		format(msg, sizeof(msg), "SPECIALTIES: You are healing yourself. (%s - Level %i)", sAbilityNames[ability][snLongName], level);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	} else {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: You are healing %s. (%s - Level %i)", msg, sAbilityNames[ability][snLongName], level);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		
		GetPlayerName(playerid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s is healing you. (%s - Level %i)", msg, sAbilityNames[ability][snLongName], level);
		SendClientMessage(healid, COLOUR_SUCCESSFUL, msg);
	}
	
	return 1;
}

kcmd:gl(playerid, text[]) return kcmd_givelife(playerid, text);
kcmd:givelife(playerid, text[]) {
	new msg[MAX_INPUT];
	
	if (!SpecialtyAllowedCheck(playerid, SP_ABILITY_GIVELIFE, "give life")) return 1;
	
	if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must be spawned and on foot to give life.");
		return 1;
	}
	
	new healid;
	if (!isnull(text)) {
		healid = FindPlayer(text);
		if (healid == INVALID_PLAYER_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Invalid player ID/Name.");
			return 1;
		}
		
		if (healid == playerid) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You cannot give life to yourself.");
			return 1;
		}
		
		if (GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(healid)) {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: %s is not in the world you are in.", msg);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 1;
		}
		
		new Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2, Float:distance;
		GetPlayerPos(playerid, x1, y1, z1);
		GetPlayerPos(healid, x2, y2, z2);
		distance = FindDistance(x1, y1, z1, x2, y2, z2);
		if (distance > HEAL_MAX_DISTANCE) {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: %s is too far to heal.", msg);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 1;
		}
		
		if (GetPlayerState(healid) != PLAYER_STATE_ONFOOT) {
			GetPlayerName(healid, msg, sizeof(msg));
			format(msg, sizeof(msg), "SPECIALTIES: %s must be on foot.", msg);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 1;
		}
	} else {
		healid = GetPlayerFacingPlayer(playerid, HEAL_MAX_DISTANCE);
		if (healid != INVALID_PLAYER_ID) {
			new modeid = mpData[healid][mpModeID];
			if (modeid != INVALID_MODE_ID) {
				if (mpData[playerid][mpModeID] == modeid) {
					if (modes[modeid][mType] == MODE_TYPE_DMFFA) {
						healid = INVALID_PLAYER_ID;
						
					} else if (modes[modeid][mType] == MODE_TYPE_DMTEAM &&
						dmtPStats[playerid][dmtpTeamID] != dmtPStats[healid][dmtpTeamID]) {
						
						healid = INVALID_PLAYER_ID;
					}
				}
			}
		}
	}
	
	if (healid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: There are no players to give life to.");
		return 1;
	}
	
	if (GetPlayerState(healid) != PLAYER_STATE_ONFOOT) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s must be spawned and on foot to give life.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (IsPlayerPaused(playerid, 5000)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must not be paused.");
		return 1;
	}
	
	if (IsPlayerPaused(healid, 5000)) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s is busy.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	new Float:phealth, Float:hhealth, Float:healamount;
	GetPlayerHealth(playerid, phealth);
	GetPlayerHealth(healid, hhealth);
	
	new Float:maxhp = 100.0;
	if (IsPlayerInMinimode(healid)) {
		if (modes[mpData[healid][mpModeID]][mType] == MODE_TYPE_DMFFA) {
			maxhp = dmffaMaps[modes[mpData[healid][mpModeID]][mMapID]][dmfPlayerHealth];
		} else if (modes[mpData[healid][mpModeID]][mType] == MODE_TYPE_DMTEAM) {
			maxhp = dmtMaps[modes[mpData[healid][mpModeID]][mMapID]][dmtPlayerHealth];
		}
	}
	
	if (hhealth >= maxhp) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: %s already has full health.", msg);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		return 1;
	}
	
	healamount = maxhp - hhealth;
	if (healamount >= phealth) healamount = phealth - 1.0;
	
	if (healamount == 0.0) {
		GetPlayerName(healid, msg, sizeof(msg));
		format(msg, sizeof(msg), "SPECIALTIES: You do not have enough health to give life to %s.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	SetPlayerHealth(playerid, phealth - healamount);
	SetPlayerHealth(healid, hhealth + healamount);
	
	GetPlayerName(healid, msg, sizeof(msg));
	format(msg, sizeof(msg), "SPECIALTIES: You have given %0.1f health to %s.", healamount, msg);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	GetPlayerName(playerid, msg, sizeof(msg));
	format(msg, sizeof(msg), "SPECIALTIES: %s has given you %0.1f health.", msg, healamount);
	SendClientMessage(healid, COLOUR_SUCCESSFUL, msg);
	
	return 1;
}
