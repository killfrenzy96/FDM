/*  
	xStreamer by Boylett
 */

#if defined _xstreamer_included
	#endinput
#endif
#define _xstreamer_included
#pragma library xstreamer

#include <a_samp>

//------------------------------------------------------------------------------

native CreateStreamedObject(modelid,Float:x,Float:y,Float:z,Float:rx,Float:ry,Float:rz,virtualworld = 0,interior = 0);
native DestroyStreamedObject(objectid);
native MoveStreamedObject(objectid,Float:x,Float:y,Float:z,Float:speed);
native StopStreamedObject(objectid);
native DestroyObjectsCreatedByMe();
native StreamObjects(playerid,Float:x,Float:y,Float:z,virtualworld = 0,interior = 0);
native UpdateMovingObjects();

forward OnStreamedObjectMoved(objectid);

stock xStreamer_SetPlayerPos(playerid,Float:x,Float:y,Float:z)
{
	UpdateMovingObjects();
	StreamObjects(playerid,x,y,z,GetPlayerVirtualWorld(playerid),GetPlayerInterior(playerid));
	SetPlayerPos(playerid,x,y,z);
}

stock xStreamer_SetPlayerInterior(playerid,interiorid)
{
	new
		Float:x,
		Float:y,
		Float:z;
	GetPlayerPos(playerid,x,y,z);
	UpdateMovingObjects();
	StreamObjects(playerid,x,y,z,GetPlayerVirtualWorld(playerid),interiorid);
	SetPlayerInterior(playerid,interiorid);
}

stock xStreamer_SetPlayerVirtualWorld(playerid,vwid)
{
	new
		Float:x,
		Float:y,
		Float:z;
	GetPlayerPos(playerid,x,y,z);
	UpdateMovingObjects();
	StreamObjects(playerid,x,y,z,vwid,GetPlayerInterior(playerid));
	SetPlayerVirtualWorld(playerid,vwid);
}

#if !defined DONT_REPLACE_NATIVES

	#if defined _ALS_SetPlayerPos
		#undef SetPlayerPos
	#else
		#define _ALS_SetPlayerPos
	#endif
	#define SetPlayerPos xStreamer_SetPlayerPos

	#if defined _ALS_SetPlayerInterior
		#undef SetPlayerInterior
	#else
		#define _ALS_SetPlayerInterior
	#endif
	#define SetPlayerInterior xStreamer_SetPlayerInterior

	#if defined _ALS_SetPlayerVirtualWorld
		#undef SetPlayerVirtualWorld
	#else
		#define _ALS_SetPlayerVirtualWorld
	#endif
	#define SetPlayerVirtualWorld xStreamer_SetPlayerVirtualWorld

	#if defined _ALS_CreateObject
		#undef CreateObject
	#else
		#define _ALS_CreateObject
	#endif
	#define CreateObject CreateStreamedObject

	#if defined _ALS_DestroyObject
		#undef DestroyObject
	#else
		#define _ALS_DestroyObject
	#endif
	#define DestroyObject DestroyStreamedObject

	#if defined _ALS_MoveObject
		#undef MoveObject
	#else
		#define _ALS_MoveObject
	#endif
	#define MoveObject MoveStreamedObject

	#if defined _ALS_StopObject
		#undef StopObject
	#else
		#define _ALS_StopObject
	#endif
	#define StopObject StopStreamedObject

#endif