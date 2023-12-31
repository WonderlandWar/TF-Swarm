//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef BASESAVEGAMEDIALOG_H
#define BASESAVEGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui/MouseCode.h"
#include "KeyValues.h"
#include "UtlVector.h"
#include "tf/basemodui.h"
#include "tf/VFlyoutMenu.h"
#include "gameui_util.h"
#include "tf/vfooterpanel.h"

#define SAVEGAME_MAPNAME_LEN 32
#define SAVEGAME_COMMENT_LEN 80
#define SAVEGAME_ELAPSED_LEN 32


struct SaveGameDescription_t
{
	char szShortName[64];
	char szFileName[128];
	char szMapName[SAVEGAME_MAPNAME_LEN];
	char szComment[SAVEGAME_COMMENT_LEN];
	char szType[64];
	char szElapsedTime[SAVEGAME_ELAPSED_LEN];
	char szFileTime[32];
	unsigned int iTimestamp;
	unsigned int iSize;
};


int SaveReadNameAndComment( FileHandle_t f, char *name, char *comment );
static int __cdecl SaveGameSortFunc( const void *lhs, const void *rhs )
{
	const SaveGameDescription_t *s1 = (const SaveGameDescription_t *)lhs;
	const SaveGameDescription_t *s2 = (const SaveGameDescription_t *)rhs;

	if (s1->iTimestamp < s2->iTimestamp)
		return 1;
	else if (s1->iTimestamp > s2->iTimestamp)
		return -1;

	// timestamps are equal, so just sort by filename
	return strcmp(s1->szFileName, s2->szFileName);
};
using namespace BaseModUI;
using namespace vgui;
class CNB_Button;
class CNB_Header_Footer;
//-----------------------------------------------------------------------------
// Purpose: Base class for save & load game dialogs
//-----------------------------------------------------------------------------
class CBaseSaveGameDialog : public vgui::Frame/*vgui::Frame*///CBaseModFrame//, public FlyoutMenuListener
{
	DECLARE_CLASS_SIMPLE( CBaseSaveGameDialog, vgui::Frame );

public:
	CBaseSaveGameDialog( vgui::Panel *parent, const char *name );

	void ScanSavedGames();


	//FloutMenuListener
	/*virtual void OnNotifyChildFocus( vgui::Panel* child );
	virtual void OnFlyoutMenuClose( vgui::Panel* flyTo );
	virtual void OnFlyoutMenuCancelled();

	Panel* NavigateBack();
	*/
	virtual void Activate()
	{
		BaseClass::Activate();
	}
protected:
	CUtlVector<SaveGameDescription_t> m_SaveGames;
	vgui::PanelListPanel *m_pGameList;

	void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );
	}
	virtual void	OnCommand( const char *command );
	virtual void OnScanningSaveGames() {}
	//void UpdateFooter( void );
	void DeleteSaveGame( const char *fileName );
	void CreateSavedGamesList();
	int GetSelectedItemSaveIndex();
	void AddSaveGameItemToList( int saveIndex );

	bool ParseSaveData( char const *pszFileName, char const *pszShortName, SaveGameDescription_t &save );

private:
	MESSAGE_FUNC( OnPanelSelected, "PanelSelected" );
};

#endif // BASESAVEGAMEDIALOG_H
