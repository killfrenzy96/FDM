#include <a_samp>

enum repairPlayerData {
	rpVehicleID,
	rpDriver,
	Float:rpRepairSpeed,
	rpExplodeChance
}
new rpData[MAX_PLAYERS][repairPlayerData];
// new repairTimer;

RepairInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		rpData[i][rpVehicleID] = INVALID_VEHICLE_ID;
	}
	kEnableTimer(19); // repairTimer = SetTimer("fdm_RepairTimer", 750, true);
	return 1;
}

RepairExit() {
	kDisableTimer(19); // KillTimer(repairTimer);
	LoopPlayers(i) {
		if (rpData[i][rpVehicleID] != INVALID_VEHICLE_ID) {
			PlayerStopRepair(i);
		}
	}
	return 1;
}

repair_OnPlayerDisconnect(playerid) {
	if (rpData[playerid][rpVehicleID] != INVALID_VEHICLE_ID) {
		PlayerStopRepair(playerid);
	}
	return 1;
}

repair_OnPlayerStateChange(playerid, newstate, oldstate) {
	#pragma unused newstate
	
	if (oldstate == PLAYER_STATE_PASSENGER && rpData[playerid][rpVehicleID] != INVALID_VEHICLE_ID) {
		PlayerStopRepair(playerid);
	}
	
	return 1;
}

PlayerRepairVehicle(playerid, vehicleid, driver, Float:speed) {
	//if (rpData[playerid][rpVehicleID] != vehicleid) PlayerStopRepair(playerid);
	rpData[playerid][rpVehicleID] = vehicleid;
	rpData[playerid][rpDriver] = driver;
	rpData[playerid][rpRepairSpeed] = speed;
	
	new msg[MAX_INPUT], Float:health;
	GetVehicleHealth(vehicleid, health);
	format(msg, sizeof(msg), "~r~Health: ~w~%0.1f", health);
	TextDrawSetString(sppData[playerid][spTextDraw], msg);
	
	return 1;
}

PlayerStopRepair(playerid) {
	if (rpData[playerid][rpVehicleID] != INVALID_VEHICLE_ID) {
		rpData[playerid][rpVehicleID] = INVALID_VEHICLE_ID;
		TextDrawSetString(sppData[playerid][spTextDraw], " ");
	}
	return 1;
}

forward fdm_RepairTimer();
public fdm_RepairTimer() {
	LoopPlayers(i) {
		if (rpData[i][rpVehicleID] != INVALID_VEHICLE_ID) {
			if ((rpData[i][rpDriver] && GetPlayerState(i) != PLAYER_STATE_DRIVER) ||
				(!rpData[i][rpDriver] && GetPlayerState(i) != PLAYER_STATE_PASSENGER)) {
				PlayerStopRepair(i);
				continue;
			}
			if (!rpData[i][rpDriver] && kGetVehicleDriver(rpData[i][rpVehicleID]) != INVALID_PLAYER_ID) {
				PlayerStopRepair(i);
				continue;
			}
			if (GetPlayerVehicleID(i) != rpData[i][rpVehicleID]) {
				PlayerStopRepair(i);
				continue;
			}
			if (IsPlayerPaused(i, 5000)) {
				PlayerStopRepair(i);
				continue;
			}
			
			new Float:vehiclehealth;
			GetVehicleHealth(rpData[i][rpVehicleID], vehiclehealth);
			
			if (rpData[i][rpDriver] == 2) {
				if (vehiclehealth >= 1000.0) {
					PlayerStopRepair(i);
					continue;
				}
				
				if (!pGodData[i][pgIsOnGodmode]) {
					new firechance;
					
					if (vehiclehealth < 250.0) {
						firechance = 50;
					} else {
						firechance = 10;
					}
					
					if (random(100) < firechance) {
						SetVehicleHealth(rpData[i][rpVehicleID], 0.0);
						
						if (random(100) < 20) {
							PlayerStopRepair(i);
							KillPlayer(i, 16);
							SendClientMessage(i, COLOUR_ATTENTION, "SPECIALTIES: You failed to rapidly repair your vehicle and got yourself killed!");
							continue;
						} else {
							vehiclehealth = 0.0;
							SendClientMessage(i, COLOUR_ATTENTION, "SPECIALTIES: You failed to rapidly repair your vehicle!");
						}
					}
				}
			} else {
				if (vehiclehealth >= 1000.0 || vehiclehealth <= 250.0) {
					PlayerStopRepair(i);
					continue;
				}
			}
			
			vehiclehealth += rpData[i][rpRepairSpeed];
			if (vehiclehealth >= 1000.0) vehiclehealth = 1000.0;
			
			SetVehicleHealth(rpData[i][rpVehicleID], vehiclehealth);
			
			if (vehiclehealth >= 1000.0) {
				PlayerStopRepair(i);
				continue;
			}
			
			new msg[MAX_INPUT];
			format(msg, sizeof(msg), "~r~Health: ~w~%0.1f", vehiclehealth);
			TextDrawSetString(sppData[i][spTextDraw], msg);
		}
	}
	return 1;
}

kcmd:rv(playerid, text[]) return kcmd_repair(playerid, text);
kcmd:repair(playerid, text[]) {
	new msg[MAX_INPUT];
	
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID && modes[mpData[playerid][mpModeID]][mType] == MODE_TYPE_DERBY) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You cannot repair your vehicle in derbies.");
		return 1;
	}
	
	new driver, ability, level;
	if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) driver = 1;
	
	if (driver) {
		level = SpecialtyAllowedCheck(playerid, SP_ABILITY_REPAIRDRIVER, "repair vehicles as driver");
		if (!level) return 1;
		
		ability = SP_ABILITY_REPAIRDRIVER;
	} else {
		if (GetPlayerState(playerid) == PLAYER_STATE_PASSENGER) {
			level = SpecialtyAllowedCheck(playerid, SP_ABILITY_REPAIR, "repair vehicles as passenger");
		} else {
			level = SpecialtyAllowedCheck(playerid, SP_ABILITY_REPAIR, "repair vehicles");
		}
		if (!level) return 1;
		
		if (kGetVehicleDriver(GetPlayerVehicleID(playerid)) != INVALID_PLAYER_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: There must be no driver to repair vehicles as passenger.");
			return 1;
		}
		
		ability = SP_ABILITY_REPAIR;
	}
	
	new vehicleid = GetPlayerVehicleID(playerid);
	if (!vehicleid || !GetVehicleModel(vehicleid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must be in a vehicle to repair vehicles.");
		return 1;
	}
	if (vehicleid == rpData[playerid][rpVehicleID] && rpData[playerid][rpDriver] != 2) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You are already repairing this vehicle.");
		return 1;
	}
	
	new Float:vehiclehealth;
	GetVehicleHealth(vehicleid, vehiclehealth);
	if (vehiclehealth >= 1000.0) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: This vehicle already has full health.");
		return 1;
	}
	if (vehiclehealth <= 250.0) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: This vehicle is too trashed to repair.");
		return 1;
	}
	
	new Float:repairspeed;
	if (driver) {
		repairspeed = float(level) * 4.0;
	} else {
		repairspeed = float(level) * 15.0;
	}
	PlayerRepairVehicle(playerid, vehicleid, driver, repairspeed);
	
	if (driver) {
		format(msg, sizeof(msg), "SPECIALTIES: You are repairing your %s. (%s - Level %i)", GetVehicleName(GetVehicleModel(vehicleid)), sAbilityNames[ability][snLongName], level);
	} else {
		format(msg, sizeof(msg), "SPECIALTIES: You are repairing your %s. (%s - Level %i)", GetVehicleName(GetVehicleModel(vehicleid)), sAbilityNames[ability][snLongName], level);
	}
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	return 1;
}


kcmd:rr(playerid, text[]) return kcmd_rapidrepair(playerid, text);
kcmd:rapidrepair(playerid, text[]) {
	new msg[MAX_INPUT];
	
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID && modes[mpData[playerid][mpModeID]][mType] == MODE_TYPE_DERBY) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You cannot repair your vehicle in derbies.");
		return 1;
	}
	
	new level = SpecialtyAllowedCheck(playerid, SP_ABILITY_RAPIDREPAIR, "rapid repair vehicles as driver");
	if (!level) return 1;
	
	new vehicleid = GetPlayerVehicleID(playerid);
	if (!vehicleid || !GetVehicleModel(vehicleid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must be in a vehicle to repair vehicles.");
		return 1;
	}
	if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You must be the driver to rapidly repair vehicles.");
		return 1;
	}
	if (vehicleid == rpData[playerid][rpVehicleID] && rpData[playerid][rpDriver] == 2) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: You are already rapidly repairing this vehicle.");
		return 1;
	}
	
	new Float:vehiclehealth;
	GetVehicleHealth(vehicleid, vehiclehealth);
	if (vehiclehealth >= 1000.0) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: This vehicle already has full health.");
		return 1;
	}
	
	PlayerRepairVehicle(playerid, vehicleid, 2, 20.0 * level);
	format(msg, sizeof(msg), "SPECIALTIES: You are rapid repairing your %s. (%s - Level %i)", GetVehicleName(GetVehicleModel(vehicleid)), sAbilityNames[SP_ABILITY_RAPIDREPAIR][snLongName], level);
	
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	return 1;
}

