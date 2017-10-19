/*

By KillFrenzy.

Miscellaneous functions.

*/

#include <a_samp>
#include "YSI_bit.inc"

#if !defined MAX_STRING
	#define MAX_STRING 256
#endif
#if !defined MAX_INPUT
	#define MAX_INPUT 129
#endif
#if !defined MAX_WEAPON_SLOTS
	#define MAX_WEAPON_SLOTS 13
#endif

#define IsKeyJustDown(%1,%2,%3)		(((%2)&(%1))&&!((%3)&(%1)))
#define IsKeyJustUp(%1,%2,%3)		(!((%2)&(%1))&&((%3)&(%1)))

#if !defined isnull
	#define isnull(%1) (%1[0] == 0 || (%1[0] == 1 && %1[1] == 0))
#endif

#if !defined dcmd
	#define dcmd(%1,%2,%3) if (!strcmp((%3)[1], #%1, true, (%2)) && ((((%3)[(%2) + 1] == '\0') && (dcmd_%1(playerid, ""))) || (((%3)[(%2) + 1] == ' ') && (dcmd_%1(playerid, (%3)[(%2) + 2]))))) return 1
#endif

// KCMD
kcmd_OnPlayerCommandText(playerid, cmdtext[])
{
	new pos,
		funcname[32];
	do { cmdtext[pos] |= 0x20; } while(cmdtext[++pos] > ' ');
	if(cmdtext[pos] == '\0') {
		format(funcname, sizeof(funcname), "kcmd_%s", cmdtext[1]);
	} else {
		format(funcname, sizeof(funcname), "kcmd_%.*s", pos-1, cmdtext[1]);
		while('\0' < cmdtext[++pos] <= ' ') {}
		if(cmdtext[pos]) {
			return CallLocalFunction(funcname, "isi", playerid, cmdtext[pos], false);
		}
	}
	return CallLocalFunction(funcname, "isi", playerid, "\1", false);
}

#define kcmd:%1(%2,%3)				\
			forward kcmd_%1(%2,%3);	\
			public kcmd_%1(%2,%3)
// -----

stock kstrval(const string[])
{
	/*new i, j, temp[12];
	for(; i < sizeof(temp); i++) {
		if ((string[i] >= '0' && string[i] <= '9') || (j == 0 && string[i] == '-')) {
			temp[j] = string[i];
			j++;
		}
	}
	if (j > 0) {
		print(string);
		return strval(temp);
	} else {
		return 0;
	}*/
	new temp[14];
	strcpy(temp, string, sizeof(temp));
	return strval(temp);
}
#define strval(%1) kstrval(%1)


// Loop Players

//#define LoopPlayers(%1) for (new %1; %1 < MAX_PLAYERS; %1++) if (IsPlayerConnected(%1))
//#define LoopPlayersEx(%1,%2) for (new %1; %1 < MAX_PLAYERS; %1++) if (IsPlayerConnected(%1))

#define LoopPlayers(%1) for (new aa, %1; pConnected[aa] != INVALID_PLAYER_ID; aa++) if ((%1 = pConnected[aa]) || true)
#define LoopPlayersEx(%1,%2) for (new %2, %1; pConnected[%2] != INVALID_PLAYER_ID; %2++) if ((%1 = pConnected[%2]) || true)

new pConnected[MAX_PLAYERS + 1];

stock UpdateConnectedPlayers(excludeplayer = INVALID_PLAYER_ID) {
	new cIndex = 0;
	
	for (new i; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i) && i != excludeplayer) {
			pConnected[cIndex] = i;
			cIndex++;
		}
	}
	
	pConnected[cIndex] = INVALID_PLAYER_ID;
	return 1;
}

// ------------

stock GetColourShade(colour, varyamount = 32) {
	new r, g, b;
	r = (colour >> 24 & 0xFF) + random(varyamount << 1) - varyamount;
	g = (colour >> 16 & 0xFF) + random(varyamount << 1) - varyamount;
	b = (colour >> 8 & 0xFF) + random(varyamount << 1) - varyamount;
	
	if (r < 0) r = 0; else if (r > 255) r = 255;
	if (g < 0) g = 0; else if (g > 255) g = 255;
	if (b < 0) b = 0; else if (b > 255) b = 255;
	
	return (r << 24 | g << 16 | b << 8 | (colour & 0x000000FF));
}

stock teleportPlayer(playerid, Float:x, Float:y, Float:z) {
	if (!IsPlayerSpawned(playerid)) return 0;
	new carID = GetPlayerVehicleID(playerid);
	if (carID) {
		SetVehiclePos(carID, x, y, z);
	} else {
		SetPlayerPos(playerid, x, y, z);
	}
	return 1;
}

stock teleportPlayerRandom(playerid, Float:x, Float:y, Float:z, radius) {
	new Float:x1 = x + (random(radius) - (radius * 0.5));
	new Float:y1 = y + (random(radius) - (radius * 0.5));
	teleportPlayer(playerid, x1, y1, z);
}

stock SetVehiclePosRandom(vehicleid, Float:x, Float:y, Float:z, radius) {
	new Float:x1 = x + (random(radius) - (radius * 0.5));
	new Float:y1 = y + (random(radius) - (radius * 0.5));
	SetVehiclePos(vehicleid, x1, y1, z);
	return 1;
}

stock SetPlayerPosRandom(playerid, Float:x, Float:y, Float:z, radius) {
	new Float:x1 = x + (random(radius) - (radius * 0.5));
	new Float:y1 = y + (random(radius) - (radius * 0.5));
	SetPlayerPos(playerid, x1, y1, z);
	return 1;
}

stock combineString(string1[], string2[]) {
	new tempstring[MAX_STRING];
	format(tempstring, sizeof(tempstring), "%s%s", string1, string2);
	return tempstring;
}

stock GetValidFilenameFromString(string[], allowdirectories = 1) {
	new returnstring[MAX_STRING];
	for (new i = 0; i < sizeof(returnstring); i++) {
		if (string[i] == 0) return returnstring;
		
		if ((string[i] >= '0' && string[i] <= '9') ||
			(string[i] >= 'A' && string[i] <= 'Z') ||
			(string[i] >= 'a' && string[i] <= 'z') ||
			(string[i] == '[') || (string[i] == ']') ||
			(string[i] == '.') || (string[i] == ' ') ||
			(string[i] == '-')) {
			
			returnstring[i] = string[i];
		} else {
			if (allowdirectories && string[i] == '\\') {
				returnstring[i] = '/';
			} else {
				returnstring[i] = '_';
			}
		}
	}
	return returnstring;
}

stock IsValidFilename(string[], allowdirectories = 1) {
	if (!string[0]) return 0;
	
	new i;
	while (string[i]) {
		
		if ((string[i] >= '0' && string[i] <= '9') ||
			(string[i] >= 'A' && string[i] <= 'Z') ||
			(string[i] >= 'a' && string[i] <= 'z') ||
			(string[i] == '[') || (string[i] == ']') ||
			(string[i] == '.') || (string[i] == ' ') ||
			(string[i] == '-')) {
		} else {
			if (allowdirectories && string[i] == '\\') {
			} else {
				return 0;
			}
		}
		
		i++;
	}
	return 1;
}

forward Float:FindDistanceXY(Float:x1, Float:y1, Float:x2, Float:y2);
stock Float:FindDistanceXY(Float:x1, Float:y1, Float:x2, Float:y2) {
	new Float: distance;
	distance = floatsqroot(floatpower(x1 - x2, 2) + floatpower(y1 - y2, 2));
	return distance;
}

forward Float:FindDistance(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2);
stock Float:FindDistance(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2) {
	new Float: distance;
	distance = floatsqroot(floatpower(x1 - x2, 2) + floatpower(y1 - y2, 2) + floatpower(z1 - z2, 2));
	return distance;
}
//#define FindDistance(%1,%2,%3,%4,%5,%6) floatsqroot(floatpower(%1-%4, 2)+floatpower(%2-%5,2)+floatpower(%3-%6,2));

forward Float:GetXYOffsetRelativeToPlayer(playerid, Float:xoffset, Float:yoffset, &Float:x, &Float:y);
stock Float:GetXYOffsetRelativeToPlayer(playerid, Float:xoffset, Float:yoffset, &Float:x, &Float:y) {
	new Float:angle;
	new vehicleid;
	
	GetPlayerPos(playerid, x, y, angle);
	
	vehicleid = GetPlayerVehicleID(playerid);
	if (vehicleid) {
		GetVehicleZAngle(vehicleid, angle);
	} else {
		GetPlayerFacingAngle(playerid, angle);
	}
	
	x += yoffset * floatsin(-angle, degrees);
	y += yoffset * floatcos(-angle, degrees);
	x += xoffset * floatsin(-angle + 90.0, degrees);
	y += xoffset * floatcos(-angle + 90.0, degrees);
	
	return angle;
}

stock GetWeaponSlot(weaponid) {
	if ((weaponid < 2) || (weaponid > 46)) return 0;
	if ((weaponid >= 2) && (weaponid <= 9)) return 1;
	if ((weaponid >= 22) && (weaponid <= 24)) return 2;
	if ((weaponid >= 25) && (weaponid <= 27)) return 3;
	if (((weaponid >= 28) && (weaponid <= 29)) || (weaponid == 32)) return 4;
	if ((weaponid >= 30) && (weaponid <= 31)) return 5;
	if ((weaponid >= 33) && (weaponid <= 34)) return 6;
	if ((weaponid >= 35) && (weaponid <= 38)) return 7;
	if (((weaponid >= 16) && (weaponid <= 18)) || (weaponid == 39)) return 8;
	if ((weaponid >= 41) && (weaponid <= 43)) return 9;
	if ((weaponid >= 10) && (weaponid <= 15)) return 10;
	if ((weaponid >= 44) && (weaponid <= 46)) return 11;
	if (weaponid == 40) return 12;
	return 0;
}

stock Float:GetWeaponMaxDamage(weaponid) {
	switch (weaponid) {
		// melee
		case 4: return 1833.4;
		//case 9: return 30.0;
		
		// pistols
		case 22: return 8.3;
		case 23: return 13.2;
		case 24: return 46.2;
		
		// shotguns
		case 25: return 49.6;
		case 26: return 49.6;
		case 27: return 39.7;
		
		// smg's
		case 28: return 6.6;
		case 29: return 8.3;
		case 32: return 6.6;
		
		// assualt
		case 30: return 9.9;
		case 31: return 9.9;
		
		// rifle
		case 33: return 24.8;
		case 34: return 41.3;
		
		// heavy
		case 37: return 1.0;
		case 38: return 46.2;
		
		// other
		case 41: return 2.7;
		case 42: return 2.7;
		
		default: {
			if (weaponid >= 0 && weaponid <= 15) {
				//return 6.6;
				return 55.0;
			}
			return 0.0;
		}
	}
	
	return 0.0;
}

stock IsWeaponMelee(weaponid) {
	//if ((weaponid >= 0) && (weaponid <= 15)) return 1;
	//if (weaponid == 46) return 1;
	switch (weaponid) {
		case 0..15, 40, 44..46: return 1;
	}
	
	return 0;
}

stock IsVowel(character) {
	switch (character) {
		case 'A','a','E','e','I','i','O','o','U','u': return 1;
	}
	return 0;
}

stock DoesWeaponHaveAmmo(weaponid) {
	//if ((weaponid >= 0) && (weaponid <= 15)) return 1;
	//if (weaponid == 46) return 1;
	switch (weaponid) {
		case 0..15, 40, 44..46: return 1;
	}
	
	return 0;
}

stock IsWeaponSlotShared(weaponslot) {
	if (weaponslot == 3) return 1;
	if (weaponslot == 4) return 1;
	if (weaponslot == 5) return 1;
	return 0;
}

stock IsWeaponWastable(weaponid) { // Can the player lose this weapon
	if ((weaponid >= 1) && (weaponid <= 15)) return 0;
	return 1;
}

stock FindWeaponID(weaponname[]) {
	if (!weaponname[0]) return 0;
	
	new weaponid = strval(weaponname);
	if (weaponid) return weaponid;
	
	new wname[MAX_WEAPONNAME];
	for (new i; i < 47; i++) {
		wname = ReturnWeaponName(i);
		if (!wname[0]) continue;
		if (strfind(ReturnWeaponName(i), weaponname, true) != -1) {
			// Weapon found
			return i;
		}
	}
	
	// 'Deagle'
	if (strfind("Deagle", weaponname, true) != -1) {
		// Weapon found
		return 24;
	}
	
	return 0;
}

stock IsPlayerInArea(playerid, Float:x1, Float:y1, Float:x2, Float:y2, Float:z1, Float:z2) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	
	if (x1 < x2) {
		if (px < x1 || px > x2) return 0;
	} else {
		if (px > x1 || px < x2) return 0;
	}
	
	if (y1 < y2) {
		if (py < y1 || py > y2) return 0;
	} else {
		if (py > y1 || py < y2) return 0;
	}
	
	if (z1 < z2) {
		if (pz < z1 || pz > z2) return 0;
	} else {
		if (pz > z1 || pz < z2) return 0;
	}
	
	return 1;
}

stock GetTimeString(milliseconds, secondsaccurate = 1) {
	new hours, minutes;
	new Float:seconds;
	hours = milliseconds / 3600000;
	minutes = (milliseconds - (hours * 3600000)) / 60000;
	seconds = (milliseconds - (hours * 3600000) - (minutes * 60000)) * 0.001;
	
	new string[24];
	if (hours) {
		if (minutes == 0 && seconds == 0.0) {
			format(string, sizeof(string), "%ih", hours);
		} else {
			if (seconds == 0.0) {
				format(string, sizeof(string), "%ih %imin", hours, minutes);
			} else {
				if (secondsaccurate) {
					format(string, sizeof(string), "%ih %imin %0.3fs", hours, minutes, seconds);
				} else {
					format(string, sizeof(string), "%ih %imin %0.0fs", hours, minutes, seconds);
				}
			}
		}
	} else if (minutes) {
		if (!secondsaccurate && seconds == 0.0) {
			format(string, sizeof(string), "%imin", minutes);
		} else {
			if (secondsaccurate) {
				format(string, sizeof(string), "%imin %0.3fs", minutes, seconds);
			} else {
				format(string, sizeof(string), "%imin %0.0fs", minutes, seconds);
			}
		}
	} else {
		if (secondsaccurate) {
			format(string, sizeof(string), "%0.3fs", seconds);
		} else {
			format(string, sizeof(string), "%0.0fs", seconds);
		}
	}
	return string;
}

stock GetTimeString2(milliseconds) {
	new hours, minutes, Float:seconds;
	hours = milliseconds / 3600000;
	minutes = (milliseconds - (hours * 3600000)) / 60000;
	seconds = float(milliseconds - (hours * 3600000) - (minutes * 60000)) / 1000.0;
	
	new string[24];
	if (hours) {
		format(string, sizeof(string), "%i:%02i:%02i", hours, minutes, floatround(seconds, floatround_round));
	} else if (minutes) {
		format(string, sizeof(string), "%i:%02i", minutes, floatround(seconds, floatround_round));
	} else {
		format(string, sizeof(string), "%i", floatround(seconds, floatround_round));
	}
	return string;
}

stock GetTimeString3(milliseconds) {
	new hours, minutes, seconds, secondpart;
	hours = milliseconds / 3600000;
	minutes = (milliseconds - (hours * 3600000)) / 60000;
	seconds = (milliseconds - (hours * 3600000) - (minutes * 60000)) / 1000;
	secondpart = (milliseconds - (hours * 3600000) - (minutes * 60000) - (seconds * 1000)) / 10;
	
	new string[24];
	if (hours) {
		format(string, sizeof(string), "%02i.%02i.%02i.02i", hours, minutes, seconds, secondpart);
	} else if (minutes) {
		format(string, sizeof(string), "%02i.%02i.%02i", minutes, seconds, secondpart);
	} else {
		format(string, sizeof(string), "%02i.%02i", seconds, secondpart);
	}
	return string;
}

stock GetTimeString4(seconds) {
	new hours, minutes;
	hours = seconds / 3600;
	minutes = (seconds - (hours * 3600)) / 60;
	seconds = seconds - (hours * 3600) - (minutes * 60);
	
	new string[24];
	if (hours) {
		format(string, sizeof(string), "%i:%02i:%02i", hours, minutes, seconds);
	} else if (minutes) {
		format(string, sizeof(string), "%i:%02i", minutes, seconds);
	} else {
		format(string, sizeof(string), "%i", seconds);
	}
	return string;
}

stock abs(value) {
	if (value < 0) return value * -1;
	return value;
}

/*stock kvalstr(value) {
	new str[16], rtn[16], count, len, j = 1;
	format(str, sizeof(str), "%i", value);
	
	if (-10000 < value < 10000) return str;
	len = strlen(str);
	
	rtn[0] = str[0];
	if (value < 0) {
		rtn[1] = str[1];
		j = 2;
	}
	
	count = len - j;
	for (new i = j; i < len; i++) {
		if (!(count % 3)) {
			rtn[j++] = ',';
		}
		rtn[j++] = str[i];
		count--;
	}
	
	return rtn;
}*/
stock kvalstr(value) // Dabombber
{
	new string[16];
	format(string, sizeof(string), "%i", value);
	if(-10000 < value < 10000) return string;
	for(new pos=strlen(string), count, start = value < 0 ? 1 : 0; pos > start; pos--, count++) {
		if(count == 3) {
			strins(string, ",", pos);
			count = 0;
		}
	}
	return string;
}


stock ByteInt(integer) {
	if (integer <= 0) {
		return 0;
	} else if (integer >= 255) {
		return 255;
	}
	return integer;
}

stock ReturnPlayerName(playerid) {
	new pname[MAX_PLAYER_NAME];
	GetPlayerName(playerid, pname, sizeof(pname));
	return pname;
}

stock floatswap(&Float:float1, &Float:float2) {
	new Float:tmp = float1;
	float1 = float2;
	float2 = tmp;
	return 1;
}

stock racestr(position) {
	new posstr[MAX_REAL + 2],
		lastdigit;
	
	if (position >= 11 && position <= 13) {
		lastdigit = 5;
	} else {
		lastdigit = position % 10;
	}
	
	switch (lastdigit) {
		case 1: format(posstr, sizeof(posstr), "%ist", position);
		case 2: format(posstr, sizeof(posstr), "%ind", position);
		case 3: format(posstr, sizeof(posstr), "%ird", position);
		default: format(posstr, sizeof(posstr), "%ith", position);
	}
	
	return posstr;
}

stock strcpy(dest[], const source[], maxlength = sizeof(dest))
{
	new len = strlen(source) + 1;
	if (maxlength > len) maxlength = len;
	memcpy(dest, source, 0, maxlength << 2, maxlength);
	dest[maxlength - 1] = 0;
	return 1;
}

stock split(const string[], &index, seperator) {
	new result[MAX_STRING],
		findstr[2],
		endpos;
	
	findstr[0] = seperator;
	endpos = strfind(string, findstr, false, index);
	
	if (endpos == -1) {
		endpos = strlen(string) + 1; // Copy string from index to end
	} else {
		endpos++; // Copy string from index to pos
	}
	
	new cpylen = endpos - index;
	if (cpylen <= 0) return result;
	if (cpylen >= sizeof(result)) cpylen = sizeof(result);
	
	memcpy(result, string[index], 0, cpylen << 2, endpos);
	result[cpylen - 1] = 0;
	index = endpos;
	
	return result;
}

stock splitcpy(result[], const string[], &index, seperator, maxsize = sizeof(result)) {
	new findstr[2],
		endpos;
	
	findstr[0] = seperator;
	endpos = strfind(string, findstr, false, index);
	
	if (endpos == -1) {
		endpos = strlen(string) + 1; // Copy string from index to end
	} else {
		endpos++; // Copy string from index to pos
	}
	
	new cpylen = endpos - index;
	if (cpylen <= 0) {
		result[0] = 0;
		return 0;
	}
	if (cpylen >= maxsize) cpylen = maxsize;
	
	memcpy(result, string[index], 0, cpylen << 2, endpos);
	result[cpylen - 1] = 0;
	index = endpos;
	
	return 1;
}

// Lower case conversion
stock LCase(dest[], src[], size = sizeof(dest)) {
	for (new i; i < size || !src[i]; i++) {
		if (src[i] >= 'A' && src[i] <= 'Z') {
			dest[i] = src[i] + 32;
		} else {
			dest[i] = src[i];
		}
	}
}

// Upper case conversion
stock UCase(dest[], src[], size = sizeof(dest)) {
	for (new i; i < size || !src[i]; i++) {
		if (src[i] >= 'a' && src[i] <= 'z') {
			dest[i] = src[i] - 32;
		} else {
			dest[i] = src[i];
		}
	}
}

//-----------------------------------------------------------------------------------------------------
// Functions from other respective owners
//-----------------------------------------------------------------------------------------------------

/*stock strcpy(dest[], const source[], maxlength = sizeof(dest))
{
	new idx;
	while (source[idx]!='\0' && idx<maxlength) {
		dest[idx] = source[idx];
		idx ++;
	}
	dest[idx] = '\0';
}*/

stock hexstr(string[]) // YSI by Y_Less
{
	new
		ret,
		val,
		i;
	if (string[0] == '0' && (string[1] == 'x' || string[1] == 'X')) i = 2;
	while (string[i])
	{
		ret <<= 4;
		val = string[i++] - '0';
		if (val > 0x09) val -= 0x07;
		if (val > 0x0F) val -= 0x20;
		if (val < 0x01) continue;
		if (val < 0x10) ret += val;
	}
	return ret;
}

stock ishex(str[]) // YSI by Y_Less
{
	new
		i,
		cur;
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) i = 2;
	while (str[i])
	{
		cur = str[i++];
		if ((cur < '0') || (cur > '9' && cur < 'A') || (cur > 'F' && cur < 'a') || (cur > 'f')) return 0;
	}
	return 1;
}

stock strvalEx(const string[]) // By Dabombber - fixes 'uber glitch'
{
	new val, i, neg = 1;
	for(; i < 30 && string[i] != 0 && ((string[i] < '0' || string[i] > '9') && string[i] != '-') ; i++) {} // This part by KillFrenzy - the ability to handle numbers with spaces in front
	if(string[0] == '-') {
		i++;
		neg = -1;
	}
	for(; i < 30 && string[i] >= '0' && string[i] <= '9'; i++) {
		val = (val * 10) + string[i] - '0';
	}
	return val * neg;
}

stock strclr(string[])
{
	for (new i, length = strlen(string); i < length; i++)
	{
		string[i] = '\0';
	}
}

stock RGBAToHex(r, g, b, a)
{
	return (r<<24 | g<<16 | b<<8 | a);
}

stock ReturnServerVar(const string[])
{
	new buffer[MAX_INPUT];
	GetServerVarAsString(string, buffer, sizeof(buffer));
	return buffer;
}

/*stock split(string[], &index, seperator) {
	new result[MAX_STRING];
	new idxpos = index;
	for (new i=index;i<strlen(string);i++) {
	    index++;
		if (string[i] == seperator) break;
		if (string[i] == 0) break;
		result[i-idxpos] = string[i];
	}
	return result;
}*/

stock FindPlayer(const string[])
{
	new id = strval(string);
	if(isNumeric(string) && id >= 0 && id < GetMaxPlayers()) {
		return IsPlayerConnected(id) ? id : INVALID_PLAYER_ID;
	}
	return ReturnPlayerID(string);
}

stock ReturnPlayerID(const playername[])
{
	if(playername[0] == 0) return INVALID_PLAYER_ID;
	new pid = INVALID_PLAYER_ID;
	for(new i, maxp = GetMaxPlayers(), bool:found, pname[MAX_PLAYER_NAME], length = strlen(playername); i < maxp; i++) {
		if(!IsPlayerConnected(i)) continue;
		GetPlayerName(i, pname, sizeof(pname));
		new j = strfind(pname, playername, true);
		if((j == 0) && (strlen(pname) == length)) return i;
		if(!found) {
			if(j == 0) {
				pid = i;
				found = true;
			}
			else if(j != -1 && pid == INVALID_PLAYER_ID) {
				pid = i;
			}
		}
	}
	return pid;
}

stock isNumeric(const string[])
{
	if(string[0] == '\0') return 0;
	for(new i; string[i] != '\0'; i++) if(string[i] > '9' || string[i] < '0') return 0;
	return 1;
}

stock isInteger(const string[])
{
	if(string[0] == '\0') return 0;
	for(new i = ((string[0] == '-' || string[0] == '+') && string[1] != '\0'); string[i] != '\0'; i++) if(string[i] > '9' || string[i] < '0') return 0;
	return 1;
}

stock isFloat(const string[])
{
	if(string[0] == '\0') return 0;
	new i = ((string[0] == '-' || string[0] == '+') && string[1] != '\0');
	for(new bool:decimal; string[i] != '\0'; i++) {
		if(string[i] == '.') {
			if(decimal) return 0;
			decimal = true;
		}
		else if(string[i] > '9' || string[i] < '0') return 0;
	}
	if(i == 1 && string[0] == '.') return 0;
	if(i == 2 && (string[0] == '-' || string[0] == '+') && string[1] == '.') return 0;
	return 1;
}

stock ReturnWeaponName(weaponid)
{
	new wname[20];
	switch(weaponid) {
		case 11: wname = "Small Vibrator";
		case 18: wname = "Molotovs";
		case 40: wname = "Detonator";
		case 44: wname = "Nightvision Goggles";
		case 45: wname = "Thermal Goggles";
		default: GetWeaponName(weaponid, wname, sizeof(wname));
	}
	return wname;
}

stock SendWrappedMessageToPlayer(playerid, colour, const msg[], maxlength=85, const prefix[]="    ")
{
	new length = strlen(msg);
	if(length <= maxlength) {
		SendClientMessage(playerid, colour, msg);
		return 1;
	}
	new string[MAX_STRING], idx, space, plen, bool:useprefix, lines;
	for(new i;i<length;i++) {
		if(i-idx+plen >= maxlength) {
			if(idx == space || i-space >= 20) {
				strmid(string, msg, idx, i);
				idx = i;
			}
			else {
				strmid(string, msg, idx, space);
				idx = space+1;
			}
			if(useprefix) strins(string, prefix, 0);
			else {
				plen = strlen(prefix);
				useprefix = true;
			}
			SendClientMessage(playerid, colour, string);
			lines++;
		}
		else if(msg[i]==' ') space = i;
	}
	if(idx < length) {
		strmid(string, msg, idx, length);
		strins(string, prefix, 0);
		SendClientMessage(playerid, colour, string);
		lines++;
	}
	return lines;
}

stock SendWrappedMessageToClients(Bit:players[], colour, const msg[], maxlength=85, const prefix[]="    ")
{
	new length = strlen(msg);
	if(length <= maxlength) {
		for(new i, j = GetMaxPlayers(); i < j; i++) {
			if(Bit_GetBit(players, i)) {
				SendClientMessage(i, colour, msg);
			}
		}
		return 1;
	}
	new string[MAX_INPUT], idx, space, plen, bool:useprefix, lines;
	for(new i;i<length;i++) {
		if(i-idx+plen >= maxlength) {
			if(idx == space || i-space >= 20) {
				strmid(string, msg, idx, i);
				idx = i;
			}
			else {
				strmid(string, msg, idx, space);
				idx = space+1;
			}
			if(useprefix) strins(string, prefix, 0);
			else {
				plen = strlen(prefix);
				useprefix = true;
			}
			for(new j, k = GetMaxPlayers(); j < k; j++) {
				if(Bit_GetBit(players, j)) {
					SendClientMessage(j, colour, string);
					lines++;
				}
			}
		}
		else if(msg[i]==' ') space = i;
	}
	if(idx < length) {
		strmid(string, msg, idx, length);
		strins(string, prefix, 0);
		for(new i, j = GetMaxPlayers(); i < j; i++) {
			if(Bit_GetBit(players, i)) {
				SendClientMessage(i, colour, string);
				lines++;
			}
		}
	}
	return lines;
}

stock SendWrappedMessageToAll(colour, const msg[], maxlength=85, const prefix[]="    ")
{
	new length = strlen(msg);
	if(length <= maxlength) {
		SendClientMessageToAll(colour, msg);
		return 1;
	}
	new string[MAX_INPUT], idx, space, plen, bool:useprefix, lines;
	for(new i;i<length;i++) {
		if(i-idx+plen >= maxlength) {
			if(idx == space || i-space >= 20) {
				strmid(string, msg, idx, i);
				idx = i;
			}
			else {
				strmid(string, msg, idx, space);
				idx = space+1;
			}
			if(useprefix) strins(string, prefix, 0);
			else {
				plen = strlen(prefix);
				useprefix = true;
			}
			SendClientMessageToAll(colour, string);
			lines++;
		}
		else if(msg[i]==' ') space = i;
	}
	if(idx < length) {
		strmid(string, msg, idx, length);
		strins(string, prefix, 0);
		SendClientMessageToAll(colour, string);
		lines++;
	}
	return lines;
}

#define ReturnAngle(%1,%2,%3,%4) atan2(%4-%2,%3-%1)

stock power(x, y) {
	/* returns x raised to the power of y */
	assert y >= 0;
	new r = 1;
	for (new i = 0; i < y; i++) r *= x;
	return r;
}

forward Unsigned:IP2Code(const IP[]);
stock Unsigned:IP2Code(const IP[])
{
	new IPCode;
	for(new pos, num;; pos++) {
		switch(IP[pos]) {
			case '0'..'9': num = (num * 10) + (IP[pos] - '0');
			case '.': {
				IPCode = (IPCode << 8) | num;
				num = 0;
			}
			case '\0': {
				IPCode = (IPCode << 8) | num;
				break;
			}
			default: return Unsigned:0;
		}
	}
	return Unsigned:IPCode;
}

forward Float:AngDif(Float:a1, Float:a2);
stock Float:AngDif(Float:a1, Float:a2) { //angle difference
	a2 = (a2 - a1);
	if (a2 < 0) a2 = a2 * -1;
	if (a2 > 180) a2 = a2 - 360;
	return a2;
}

stock sscanf(string[], format[], {Float,Unsigned,_}:...)
{
	if (isnull(string))
	{
		return format[0];
	}
	new
		formatPos = 0,
		stringPos = 0,
		paramPos = 2,
		paramCount = numargs(),
		delim = ' ';
	while (paramPos < paramCount && string[stringPos])
	{
		switch (format[formatPos++])
		{
			case '\0':
			{
				return 0;
			}
			case 'i', 'd':
			{
				new
					neg = 1,
					num = 0,
					ch = string[stringPos];
				if (ch == '-')
				{
					neg = -1;
					ch = string[++stringPos];
				}
				do
				{
					stringPos++;
					if (ch >= '0' && ch <= '9')
					{
						num = (num * 10) + (ch - '0');
					}
					else
					{
						return -1;
					}
				}
				while ((ch = string[stringPos]) > ' ' && ch != delim);
				setarg(paramPos, 0, num * neg);
			}
			case 'u':
			{
				new
					num = 0,
					ch = string[stringPos];
				do
				{
					stringPos++;
					if (ch >= '0' && ch <= '9')
					{
						num = (num * 10) + (ch - '0');
					}
					else
					{
						return -1;
					}
				}
				while ((ch = string[stringPos]) > ' ' && ch != delim);
				setarg(paramPos, 0, num);
			}
			case 'h', 'x':
			{
				new
					num = 0,
					ch = string[stringPos];
				do
				{
					stringPos++;
					switch (ch)
					{
						case 'x', 'X':
						{
							num = 0;
							continue;
						}
						case '0' .. '9':
						{
							num = (num << 4) | (ch - '0');
						}
						case 'a' .. 'f':
						{
							num = (num << 4) | (ch - ('a' - 10));
						}
						case 'A' .. 'F':
						{
							num = (num << 4) | (ch - ('A' - 10));
						}
						default:
						{
							return -1;
						}
					}
				}
				while ((ch = string[stringPos]) > ' ' && ch != delim);
				setarg(paramPos, 0, num);
			}
			case 'c':
			{
				setarg(paramPos, 0, string[stringPos++]);
			}
			case 'f':
			{
				setarg(paramPos, 0, _:floatstr(string[stringPos]));
			}
			case 'p':
			{
				delim = format[formatPos++];
				continue;
			}
			case 's', 'z':
			{
				new
					i = 0,
					len = getarg(paramPos + 1, 0) - 1,
					ch;
				if (format[formatPos])
				{
					while (i < len && (ch = string[stringPos]) > ' ' && ch != delim)
					{
						setarg(paramPos, i++, ch);
						stringPos++;
					}
					if (!i && len > 1)
					{
						return -1;
					}
				}
				else
				{
					while (i < len && (ch = string[stringPos]))
					{
						setarg(paramPos, i++, ch);
						stringPos++;
					}
				}
				setarg(paramPos++, i, '\0');
			}
			case '_':
			{
				goto Dengo;	// Eh, screw good practice. How bad can it be?
			}
			default:
			{
				continue;
			}
		}
		paramPos++;
		Dengo:
		while (string[stringPos] && string[stringPos] != delim && string[stringPos] > ' ')
		{
			stringPos++;
		}
		while (string[stringPos] && (string[stringPos] == delim || string[stringPos] <= ' '))
		{
			stringPos++;
		}
	}
	while (format[formatPos] == 'z')
	{
		formatPos++;
	}
	if (format[formatPos] > ' ')
	{
		return format[formatPos];
	}
	return 0;
}

stock GetPlayerLeadPos(playerid, &Float:lX, &Float:lY, &Float:lZ) {
	new Float:pX, Float:pY, Float:pZ, Float:vX, Float:vY, Float:vZ, vehicleid;
	
	vehicleid = GetPlayerVehicleID(playerid);
	if (vehicleid) {
		GetVehiclePos(vehicleid, pX, pY, pZ);
		GetVehicleVelocity(vehicleid, vX, vY, vZ);
	} else {
		GetPlayerPos(playerid, pX, pY, pZ);
		GetPlayerVelocity(playerid, vX, vY, vZ);
	}
	
	GetPosLeadPos(lX, lY, lZ, pX, pY, pZ, vX, vY, vZ, GetPlayerPing(playerid));
	return 1;
}

stock GetPlayerLeadPosOnPlayer(playerid, targetid, &Float:lX, &Float:lY, &Float:lZ) {
	new Float:pX, Float:pY, Float:pZ, Float:vX, Float:vY, Float:vZ, vehicleid;
	
	vehicleid = GetPlayerVehicleID(targetid);
	if (vehicleid) {
		GetVehiclePos(vehicleid, pX, pY, pZ);
		GetVehicleVelocity(vehicleid, vX, vY, vZ);
	} else {
		GetPlayerPos(targetid, pX, pY, pZ);
		GetPlayerVelocity(targetid, vX, vY, vZ);
	}
	
	GetPosLeadPos(lX, lY, lZ, pX, pY, pZ, vX, vY, vZ, GetPlayerPing(playerid) + GetPlayerPing(targetid));
	return 1;
}

stock GetPosLeadPos(&Float:lX, &Float:lY, &Float:lZ, Float:pX, Float:pY, Float:pZ, Float:vX, Float:vY, Float:vZ, milliSeconds) {
	lX = pX + ((vX * 0.1) * milliSeconds);
	lY = pY + ((vY * 0.1) * milliSeconds);
	lZ = pZ + ((vZ * 0.1) * milliSeconds);
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// POLYGON FUNCTIONS (By Dabombber)
//-----------------------------------------------------------------------------------------------------

#define MAX_VERTICIES	24
#define PI				3.1415926535898

enum E_POLYGON {
	E_VERTICIES,
	Float:E_VERTEX_X[MAX_VERTICIES],
	Float:E_VERTEX_Y[MAX_VERTICIES]
};

stock IsPlayerInPolygon(playerid, polygon[E_POLYGON])
{
	new Float:angle, Float:px, Float:py, Float:tmp;
	GetPlayerPos(playerid, px, py, tmp);
	for(new i=1; i < polygon[E_VERTICIES]; i++) {
		tmp = atan2(polygon[E_VERTEX_X][i] - px, polygon[E_VERTEX_Y][i] - py) - atan2(polygon[E_VERTEX_X][i-1] - px, polygon[E_VERTEX_Y][i-1] - py);
		tmp *= PI / 180;
		if(tmp > PI) {
			tmp -= 2 * PI;
		} else if(tmp < -PI) {
			tmp += 2 * PI;
		}
		angle += tmp;
	}
	tmp = atan2(polygon[E_VERTEX_X][0] - px, polygon[E_VERTEX_Y][0] - py) - atan2(polygon[E_VERTEX_X][polygon[E_VERTICIES]-1] - px, polygon[E_VERTEX_Y][polygon[E_VERTICIES]-1] - py);
	tmp *= PI / 180;
	if(tmp > PI) {
		tmp -= 2 * PI;
	} else if(tmp < -PI) {
		tmp += 2 * PI;
	}
	angle += tmp;
	return !(-0.1 < angle < 0.1);
}

stock IsPointInPoly(Float:px, Float:py, polygon[E_POLYGON])
{
	new Float:angle, Float:tmp;
	for(new i=1; i < polygon[E_VERTICIES]; i++) {
		tmp = atan2(polygon[E_VERTEX_X][i] - px, polygon[E_VERTEX_Y][i] - py) - atan2(polygon[E_VERTEX_X][i-1] - px, polygon[E_VERTEX_Y][i-1] - py);
		tmp *= PI / 180;
		if(tmp > PI) {
			tmp -= 2 * PI;
		} else if(tmp < -PI) {
			tmp += 2 * PI;
		}
		angle += tmp;
	}
	tmp = atan2(polygon[E_VERTEX_X][0] - px, polygon[E_VERTEX_Y][0] - py) - atan2(polygon[E_VERTEX_X][polygon[E_VERTICIES]-1] - px, polygon[E_VERTEX_Y][polygon[E_VERTICIES]-1] - py);
	tmp *= PI / 180;
	if(tmp > PI) {
		tmp -= 2 * PI;
	} else if(tmp < -PI) {
		tmp += 2 * PI;
	}
	angle += tmp;
	return !(-0.1 < angle < 0.1);
}

stock StripNewLine(string[])
{
	if (string[0] == '\0') return;
	new len = strlen(string);
	if ((string[len - 1] == '\n') || (string[len - 1] == '\r')) {
		string[len - 1] = '\0';
		if (string[0] == '\0') return;
		if ((string[len - 2] == '\n') || (string[len - 2] == '\r')) string[len - 2] = '\0';
	}
}

stock SetReturnString(const string[])
{
	for(new i = 0xFFFF; i < 0x7FFFFFFF; i++) {
		if(!existproperty(.value = i)) {
			new tmp[64];
			strpack(tmp, string);
			setproperty(0, tmp, i);
			return i;
		}
	}
	return -1;
}

stock GetReturnString(idx, string[], maxlength=sizeof(string))
{
	if(idx != -1) {
		if(existproperty(.value = idx)) {
			new tmp[64];
			getproperty(.value = idx, .string = tmp);
			deleteproperty(.value = idx);
			strunpack(string, tmp, maxlength);
			return 1;
		}
	}
	string[0] = '\0';
	return 0;
}
