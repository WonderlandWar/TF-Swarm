#include "cbase.h"
#include "vgui_bitmappanel.h"
#include "vgui_bitmapimage.h"
#include "bitmap/bitmap.h"
#include "vgui/isurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void BitmapImage::DestroyTexture()
{
	if ( m_nTextureId != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nTextureId );
		m_nTextureId = -1;
		//m_bProcedural = false;
	}
}

//-----------------------------------------------------------------------------
void BitmapImage::SetBitmap( const Bitmap_t &bitmap )
{
	//if ( m_nTextureId == -1 || !m_bProcedural )
	{
		DestroyTexture();
		m_nTextureId = vgui::surface()->CreateNewTextureID( true );
		//m_bProcedural = true;
	}

	vgui::surface()->DrawSetTextureRGBA( m_nTextureId, bitmap.GetBits(), bitmap.Width(), bitmap.Height() );

	// Initialize render size, if we don't already have one
	if ( m_Size[0] == 0 )
	{
		m_Size[0] = bitmap.Width();
		m_Size[1] = bitmap.Height();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set image data directly
//-----------------------------------------------------------------------------
void CBitmapPanel::SetBitmap( const Bitmap_t &bitmap )
{
	// Make sure we have an image that we own
	if ( m_pImage == NULL || !m_bOwnsImage )
	{
		delete m_pImage;
		m_pImage = new BitmapImage( GetVPanel(), NULL );
		m_bOwnsImage = true;
	}

	// Set the bitmap
	m_pImage->SetBitmap( bitmap );
}