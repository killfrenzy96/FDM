#include <a_samp>

enum passengerGlitchPlayerData {
	pgpVehicleID,
	pgpTick
}
new pgpData[MAX_PLAYERS][passengerGlitchPlayerData];
new disablePassengerGlitch;

PassengerGlitchInit() {
	disablePassengerGlitch = INI_ReadInt("BlockPassengerGlitch");
	if (disablePassengerGlitch) {
		kEnableTimer(27);
	}
	return 1;
}

PassengerGlitchExit() {
	if (disablePassengerGlitch) {
		kDisableTimer(27);
	}
	return 1;
}

pg_OnPlayerDisconnect(playerid) {
	pgpData[playerid][pgpVehicleID] = 0;
	return 1;
}

pg_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger) {
	if (ispassenger && GetVehicleModel(vehicleid)) {
		pgpData[playerid][pgpVehicleID] = vehicleid;
		pgpData[playerid][pgpTick] = GetTickCount();
	}
	return 1;
}

fdm_PassengerGlitchCheck() {
	LoopPlayers(playerid) {
		if (!pgpData[playerid][pgpVehicleID]) continue;
		
		if (!GetVehicleModel(pgpData[playerid][pgpVehicleID])) continue;
		
		if (GetTickCount() - pgpData[playerid][pgpTick] < 2500 - GetPlayerPing(playerid)) {
			if (IsPlayerInAnyVehicle(playerid)) {
				pgpData[playerid][pgpVehicleID] = 0;
			} else {
				new keys, leftright, updown;
				GetPlayerKeys(playerid, keys, leftright, updown);
				
				if (leftright || updown) {
					pgpData[playerid][pgpVehicleID] = 0;
				}
			}
			
		} else {
			if (!IsPlayerInAnyVehicle(playerid)) {
				new Float: playerRotZ, Float: vehicleRotZ;
				GetPlayerFacingAngle(playerid, playerRotZ);
				GetVehicleZAngle(pgpData[playerid][pgpVehicleID], vehicleRotZ);
				
				if (playerRotZ < vehicleRotZ - 0.2 || playerRotZ > vehicleRotZ + 0.2) {
					ClearAnimations(playerid);
					pgpData[playerid][pgpVehicleID] = 0;
				}
				
			} else {
				pgpData[playerid][pgpVehicleID] = 0;
			}
		}
	}
	
	return 1;
}
