#include <a_samp>

new disableDriveBy;

DriveByInit() {
	disableDriveBy = INI_ReadInt("BlockDriveBy");
	return 1;
}

driveby_OnPlayerUpdate(playerid) {
	
	// Driveby disabling
	
	if (!disableDriveBy || IsPlayerNPC(playerid)) return 1;
	
	if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 1;
	
	new keys, updown, leftright;
	GetPlayerKeys(playerid, keys, updown, leftright);
	
	switch (GetVehicleModel(GetPlayerVehicleID(playerid))) {
		case 581,509,481,462,521,463,510,522,461,448,471,468,586: {
			if (keys & KEY_ACTION || keys & KEY_FIRE) {
				return 0;
			}
		}
		case
			406,407,417,425,430,432,435,441,447,449,450,453,454,460,464,
			465,469,476,484,497,501,511,512,513,519,520,537,538,548,553,
			563,564,569,570,577,584,590..595,601,606..608,610,611: {
			
			return 1;
		}
		default: {
			if ((keys & KEY_ACTION || keys & KEY_FIRE) && (keys & KEY_LOOK_RIGHT || keys & KEY_LOOK_LEFT)) {
				return 0;
			}
		}
	}
	
	return 1;
}

driveby_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (!disableDriveBy || IsPlayerNPC(playerid)) return 1;
	
	if (newstate == PLAYER_STATE_DRIVER) {
		kDisablePlayerWeapons(playerid);
	} else if (oldstate == PLAYER_STATE_DRIVER && pGodData[playerid][pgIsOnGodmode] != 1) {
		kEnablePlayerWeapons(playerid);
	}
	
	return 1;
}
