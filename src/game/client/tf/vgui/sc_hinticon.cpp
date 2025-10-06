//========= Copyright Valve Corporation, All rights reserved. =========	===//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sc_hinticon.h"
#include <vgui/IVGui.h>
#include "inputsystem/iinputsystem.h"
#include <vgui_controls/ImageList.h>
#include "imageutils.h"
#include "bitmap/bitmap.h"
#include <vgui/ISurface.h>


using namespace vgui;

DECLARE_BUILD_FACTORY( CSCHintIcon );

//-----------------------------------------------------------------------------
CSCHintIcon::CSCHintIcon( vgui::Panel *parent, const char* panelName ) :
	vgui::Panel( parent, panelName )
	, m_nGlyphTexture( 0 )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSCHintIcon::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	auto szActionName = inResourceData->GetString( "actionName", nullptr );
	auto szActionSet = inResourceData->GetString( "actionSet", nullptr );

	// Msg( "actionName = %s actionSet = %s panel = %s\n", szActionName, szActionSet, this->GetName() );

	SetAction( szActionName, szActionSet );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSCHintIcon::SetAction( const char* szActionName, const char* szActionSet )
{
	if ( szActionSet )
	{
		m_strActionSet.Set( szActionSet );
	}

	if ( szActionName )
	{
		m_strActionName.Set( szActionName );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSCHintIcon::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetAction( nullptr, nullptr );					// nullptr = keep the same as it is now, but will cause origin glyph to be refreshed
}

void CSCHintIcon::PaintBackground()
{
	if ( m_nGlyphTexture )
	{
		vgui::surface()->DrawSetTexture( m_nGlyphTexture );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		int wide, tall;
		GetSize( wide, tall );
		int size = MIN( wide, tall );
		int x = (wide - size) / 2;
		int y = (tall - size) / 2;
		vgui::surface()->DrawTexturedRect( x, y, x + size, y + size );
		vgui::surface()->DrawSetTexture( 0 );
	}
}