#include "cbase.h"
#include "vgui_bitmappanel.h"
#include "vgui_bitmapimage.h"
#include "bitmap/bitmap.h"
#include "vgui/isurface.h"
#include "vgui_controls/menu.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/AnimationController.h"
#include "matsys_controls/mdlpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

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

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, as if that menu item had been selected by the user
// Input  : itemID - itemID from AddItem in list of dropdown items
//-----------------------------------------------------------------------------
void ComboBox::ActivateItemByRow(int row)
{
	m_pDropDown->ActivateItemByRow(row);
}

//-----------------------------------------------------------------------------
// Purpose: Activate the item in the menu list, without sending a TextChanged message
// Input  : row - row to activate
//-----------------------------------------------------------------------------
void ComboBox::SilentActivateItemByRow(int row)
{
	int itemID = GetItemIDFromRow( row );
	if ( itemID >= 0 )
	{
		SilentActivateItem( itemID );
	}
}

MenuBuilder::MenuBuilder( Menu *pMenu, Panel *pActionTarget )
	: m_pMenu( pMenu )
	, m_pActionTarget( pActionTarget )
	, m_pszLastCategory( NULL )
{}

MenuItem* MenuBuilder::AddMenuItem( const char *pszButtonText, const char *pszCommand, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddMenuItem( pszButtonText, pszCommand, m_pActionTarget ) );
}

MenuItem* MenuBuilder::AddMenuItem( const char *pszButtonText, KeyValues *kvUserData, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddMenuItem( pszButtonText, kvUserData, m_pActionTarget ) );
}

MenuItem* MenuBuilder::AddMenuItem( const wchar_t *pwszButtonText, const char *pszCommand, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddMenuItem( CStrAutoEncode( pwszButtonText ).ToString(), pwszButtonText, pszCommand, m_pActionTarget ) );
}

MenuItem* MenuBuilder::AddMenuItem( const wchar_t *pwszButtonText, KeyValues *kvUserData, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddMenuItem( CStrAutoEncode( pwszButtonText ).ToString(), pwszButtonText, kvUserData, m_pActionTarget ) );
}

MenuItem* MenuBuilder::AddCascadingMenuItem( const char *pszButtonText, Menu *pSubMenu, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddCascadingMenuItem( pszButtonText, m_pActionTarget, pSubMenu ) );
}

MenuItem* MenuBuilder::AddCascadingMenuItem( const wchar_t *pwszButtonText, Menu *pSubMenu, const char *pszCategoryName )
{
	AddSepratorIfNeeded( pszCategoryName );
	return m_pMenu->GetMenuItem( m_pMenu->AddCascadingMenuItem( CStrAutoEncode( pwszButtonText ).ToString(), pwszButtonText, (KeyValues*)NULL, m_pActionTarget, pSubMenu ) );
}


void MenuBuilder::AddSepratorIfNeeded( const char *pszCategoryName )
{
	// Add a separator if the categories are different
	if ( m_pszLastCategory && V_stricmp( pszCategoryName, m_pszLastCategory ) != 0 )
	{
		m_pMenu->AddSeparator();
	}

	m_pszLastCategory = pszCategoryName;
}


//-----------------------------------------------------------------------------
// Purpose: stops an animation sequence script
//-----------------------------------------------------------------------------
bool AnimationController::StopAnimationSequence( Panel *pWithinParent, const char *sequenceName )
{
	Assert( pWithinParent );
#if 0 // TF_SWARM: FIXME!
	// lookup the symbol for the name
	UtlSymId_t seqName = g_ScriptSymbols.Find( sequenceName );
	if (seqName == UTL_INVAL_SYMBOL)
		return false;

	// remove the existing command from the queue
	RemoveQueuedAnimationCommands( seqName, pWithinParent );

	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Runs a custom command from code, not from a script file
//-----------------------------------------------------------------------------
void AnimationController::CancelAnimationsForPanel( Panel *pWithinParent )
{
	// Msg("Removing queued anims for sequence %s\n", g_ScriptSymbols.String(seqName));

	// remove messages posted by this sequence
	// if pWithinParent is specified, remove only messages under that parent
	{
		for (int i = 0; i < m_PostedMessages.Count(); i++)
		{
			if ( m_PostedMessages[i].parent == pWithinParent )
			{
				m_PostedMessages.Remove(i);
				--i;
			}
		}
	}

	// remove all animations
	// if pWithinParent is specified, remove only animations under that parent
	for (int i = 0; i < m_ActiveAnimations.Count(); i++)
	{
		Panel *animPanel = m_ActiveAnimations[i].panel;

		if ( !animPanel )
			continue;

		Panel *foundPanel = pWithinParent->FindChildByName(animPanel->GetName(),true);

		if ( foundPanel != animPanel )
			continue;

		m_ActiveAnimations.Remove(i);
		--i;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMDL *CMDLPanel::GetMergeMDL( MDLHandle_t handle )
{
	int nMergeCount = m_aMergeMDLs.Count();
	for ( int iMerge = 0; iMerge < nMergeCount; ++iMerge )
	{
		if ( m_aMergeMDLs[iMerge].m_MDL.GetMDL() == handle )
			return (&m_aMergeMDLs[iMerge].m_MDL);
	}

	return NULL;
}