/*
 *   Copyright (C)2004-2013 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "resource.h"


/// CSettingsDlg-Dialogfeld
class CSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	/// Standardkonstruktor
	CSettingsDlg(bool bDisableNonWorking = false, CWnd* pParent = NULL);

	/// Standarddestruktor
	virtual ~CSettingsDlg();

// Dialogfelddaten
	enum { IDD = IDD_SETTINGSDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	static const int max_alien_frequency = 60;

private:
	bool m_bDisable;

	// General
	CSliderCtrl m_ctrlDifficultySlider;	///< Schiebregler zum Einstellen des Schwierigkeitsgrades
	CString m_sDifficulty;				///< Anzeige des aktuell eingestellten Schwierigkeitsgrades
	BOOL m_bAutoave;					///< Autosave aktivieren/deaktivieren

	// Audio
	BOOL m_bHardwaresound;
	BOOL m_bSound;
	BOOL m_bMusic;
	CSliderCtrl m_ctrlMusicvolume;

	// Video
	BOOL m_bShowTraderoutes;
	BOOL m_bAnimatedIcon;
	BOOL m_bShowMiniMap;
	BOOL m_bShowEventPictures;
	CSliderCtrl m_ctrlTooltipDelay;

	// Control
	BOOL m_bShowScrollBars;
	BOOL m_bInvertMouse;
	BOOL m_bHideMenu;

	//Galaxy
	CEdit m_edtNearbysystems;
	CSliderCtrl m_ctrlStarDensity;
	CSliderCtrl m_ctrlMinorDensity;
	CSliderCtrl m_ctrlAnomalyDensity;
	CComboBox m_comboGalaxysize;
	CComboBox m_comboGalaxyshape;

	// Special (Ingame)
	CEdit m_edtRandomSeed;
	CSliderCtrl m_ctrlAlienFrequency;
	BOOL m_bRandomEvents;
	CComboBox m_comboExpansionspeed;

	// Victory Conditions
	BOOL m_bVCElimination;
	BOOL m_bVCDiplomacy;
	BOOL m_bVCConquest;
	BOOL m_bVCResearch;
	BOOL m_bVCCombat;
	BOOL m_bVCSabotage;

protected:
	virtual void OnOK();

public:
	afx_msg void OnNMCustomdrawSliderDifficulty(NMHDR *pNMHDR, LRESULT *pResult);
};
