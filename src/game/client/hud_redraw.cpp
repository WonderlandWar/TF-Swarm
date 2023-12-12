//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
//
// hud_redraw.cpp
//
#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "itextmessage.h"
#include "vgui_basepanel.h"
#include "hud_crosshair.h"
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//For progress bar orientations
const int CHud::HUDPB_HORIZONTAL = 0;
const int CHud::HUDPB_VERTICAL = 1;
const int CHud::HUDPB_HORIZONTAL_INV = 2;

// Called when a ConVar changes value
static void FovChanged_Callback( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	ConVarRef var( pConVar );
	if ( engine->IsInGame() )
	{
		engine->ServerCmd( VarArgs( "fov %f\n", var.GetFloat() ) );
	}
}

static ConVar fov_watcher( "_fov", "0", 0, "Automates fov command to server.", FovChanged_Callback );

//-----------------------------------------------------------------------------
// Purpose: Think
//-----------------------------------------------------------------------------
void CHud::Think(void)
{
	// Determine the visibility of all hud elements
	CUtlVector< CHudElement * > & list = GetHudList();
	CUtlVector< vgui::Panel * > & hudPanelList = GetHudPanelList();

	int c = list.Count();
	Assert( c == hudPanelList.Count() );

	m_bEngineIsInGame = engine->IsInGame() && ( engine->IsLevelMainMenuBackground() == false );

	for ( int i = 0; i < c; ++i )
	{
		CHudElement *element = list[i];
		vgui::Panel *pPanel = hudPanelList[ i ];

		// Visible?
		bool visible = element->ShouldDraw();
		element->SetActive( visible );
		
		if (pPanel->IsVisible() != visible )
		{
			pPanel->SetVisible( visible );
		}
		
		if ( visible )
		{
			element->ProcessInput();
		}
	}

	// Let the active weapon at the keybits
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if ( pWeapon )
		{
			pWeapon->HandleInput();
		}
	}

	if ( ( m_flScreenShotTime > 0 ) && ( m_flScreenShotTime < gpGlobals->curtime ) )
	{
		if ( !IsX360() )
		{
			engine->ClientCmd( "screenshot" );
		}

		m_flScreenShotTime = -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose:  The percentage passed in is expected and clamped to 0.0f to 1.0f
// Input  : x - 
//			y - 
//			width - 
//			height - 
//			percentage - 
//			clr - 
//			type - 
//-----------------------------------------------------------------------------
void CHud::DrawProgressBar( int x, int y, int width, int height, float percentage, Color& clr, unsigned char type )
{
	//Clamp our percentage
	percentage = MIN( 1.0f, percentage );
	percentage = MAX( 0.0f, percentage );

	Color lowColor = clr;
	lowColor[ 0 ] /= 2;
	lowColor[ 1 ] /= 2;
	lowColor[ 2 ] /= 2;

	//Draw a vertical progress bar
	if ( type == HUDPB_VERTICAL )
	{
		int	barOfs = height * percentage;

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x, y, x + width, y + barOfs );

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x, y + barOfs, x + width, y + height );
	}
	else if ( type == HUDPB_HORIZONTAL )
	{
		int	barOfs = width * percentage;

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x, y, x + barOfs, y +  height );

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x + barOfs, y, x + width, y + height );
	}
	else if ( type == HUDPB_HORIZONTAL_INV )
	{
		int	barOfs = width * percentage;

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x, y, x + barOfs, y + height );

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x + barOfs, y, x + width, y +  height );
	}
}

//-----------------------------------------------------------------------------
// Purpose:  The percentage passed in is expected and clamped to 0.0f to 1.0f
// Input  : x - 
//			y - 
//			*icon - 
//			percentage - 
//			clr - 
//			type - 
//-----------------------------------------------------------------------------
void CHud::DrawIconProgressBar( int x, int y, CHudTexture *icon, CHudTexture *icon2, float percentage, Color& clr, int type )
{
	if ( icon == NULL )
		return;

	//Clamp our percentage
	percentage = MIN( 1.0f, percentage );
	percentage = MAX( 0.0f, percentage );

	int	height = icon->Height();
	int	width  = icon->Width();

	//Draw a vertical progress bar
	if ( type == HUDPB_VERTICAL )
	{
		int	barOfs = height * percentage;

		icon2->DrawSelfCropped( 
			x, y,  // Pos
			0, 0, width, barOfs, // Cropped subrect
			clr );

		icon->DrawSelfCropped( 
			x, y + barOfs, 
			0, barOfs, width, height - barOfs, // Cropped subrect
			clr );
	}
	else if ( type == HUDPB_HORIZONTAL )
	{
		int	barOfs = width * percentage;

		icon2->DrawSelfCropped( 
			x, y,  // Pos
			0, 0, barOfs, height, // Cropped subrect
			clr );

		icon->DrawSelfCropped( 
			x + barOfs, y, 
			barOfs, 0, width - barOfs, height, // Cropped subrect
			clr );
	}
}

