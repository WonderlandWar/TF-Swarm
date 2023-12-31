//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "tf_shareddefs.h"
#include "tf_gamerules.h"
#include "c_triggers.h"
#include "c_func_brush.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_FuncRespawnRoom : public C_BaseTrigger
{
	DECLARE_CLASS( C_FuncRespawnRoom, C_BaseTrigger );
public:
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( C_FuncRespawnRoom, DT_FuncRespawnRoom, CFuncRespawnRoom )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_FuncRespawnRoomVisualizer : public C_FuncBrush
{
	DECLARE_CLASS( C_FuncRespawnRoomVisualizer, C_FuncBrush );
public:
	DECLARE_CLIENTCLASS();

	virtual int DrawModel( int flags, const RenderableInstance_t &instance );

	virtual bool ShouldCollide( int collisionGroup, int contentsMask ) const;
};

IMPLEMENT_CLIENTCLASS_DT( C_FuncRespawnRoomVisualizer, DT_FuncRespawnRoomVisualizer, CFuncRespawnRoomVisualizer )
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Purpose: Don't draw for friendly players
//-----------------------------------------------------------------------------
int C_FuncRespawnRoomVisualizer::DrawModel( int flags, const RenderableInstance_t &instance )
{
	// Don't draw for anyone in endround
	if ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN )
	{
		return 1;
	}

	// Don't draw for teammates of the visualizer
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pLocalPlayer && pLocalPlayer->GetTeamNumber() == GetTeamNumber() )
	{
		return 1;
	}

	return BaseClass::DrawModel( flags, instance );
}

//-----------------------------------------------------------------------------
// Purpose: Enemy players collide with us, except in endround
//-----------------------------------------------------------------------------
bool C_FuncRespawnRoomVisualizer::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	// Respawn rooms are open in win state
	if ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN )
		return false;

	if ( GetTeamNumber() == TEAM_UNASSIGNED )
		return false;

	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT )
	{
		switch( GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			if ( !(contentsMask & CONTENTS_BLUETEAM) )
				return false;
			break;

		case TF_TEAM_RED:
			if ( !(contentsMask & CONTENTS_REDTEAM) )
				return false;
			break;
		}

		return true;
	}

	return false;
}