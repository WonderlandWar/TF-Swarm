//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Creates a Message box with a question in it and yes/no buttons
//
// $NoKeywords: $
//=============================================================================//

#ifndef EXTRA_TOOLTIP_H
#define EXTRA_TOOLTIP_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Controls.h>
#include <utlvector.h>

#include <vgui_controls/Tooltip.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Tooltip for a panel - shows text when cursor hovers over a panel
//-----------------------------------------------------------------------------
class BaseTooltip : public Tooltip
{
public:
	BaseTooltip(Panel *parent, const char *text = NULL);
	virtual ~BaseTooltip();
	
	bool		 ShouldLayout( void );
	virtual void PositionWindow( Panel *pTipPanel );

	void ResetDelay();

private:
	virtual void ApplySchemeSettings(IScheme *pScheme) {};
};

class TextTooltip : public BaseTooltip
{
public:
	TextTooltip(Panel *parent, const char *text = NULL);
	~TextTooltip();

	virtual void SetText(const char *text);
	virtual void ShowTooltip(Panel *currentPanel);
	virtual void HideTooltip();
	virtual void SizeTextWindow();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(IScheme *pScheme);
};

};

#endif // EXTRA_TOOLTIP_H
