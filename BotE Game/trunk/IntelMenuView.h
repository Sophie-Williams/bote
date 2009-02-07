/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "Botf2Doc.h"
#include "MainBaseView.h"

// CIntelMenuView view

class CIntelMenuView : public CMainBaseView
{
	DECLARE_DYNCREATE(CIntelMenuView)

protected:
	CIntelMenuView();           // protected constructor used by dynamic creation
	virtual ~CIntelMenuView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	void SetSubMenu(BYTE menuID) {m_bySubMenu = menuID;}

	BYTE GetSubMenu() const {return m_bySubMenu;}

private:
	// Funktionen
	/// Funkion legt alle Buttons f�r die Geheimdienstansichten an.
	void CreateButtons();

	/// Funktion zeichnet die kleinen Rassensymbole in den Geheimdienstansichten
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param highlightPlayersRace soll die eigene Rasse aktiviert dargestellt werden
	void DrawRaceLogosInIntelView(CDC* pDC, BOOLEAN highlightPlayersRace = FALSE);

	/// Funktion zeichnet die Geheimdienstinformationen an den rechten Rand der Geheimdienstansichten
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	void DrawIntelInformation(CDC* pDC);

	/// Funktion zeichnet das Geheimdienstmen�, in welchem man die globale prozentuale Zuteilung machen kann.
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelAssignmentMenu(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Spionagegeheimdienstmen�.
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelSpyMenu(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Sabotagegeheimdienstmen�.
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelSabotageMenu(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Men� mit den Geheimdienstberichten.
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelReportsMenu(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Men� f�r die Geheimdienstanschl�ge.
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelAttackMenu(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Men� f�r die Geheimdienstinformationen
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawIntelInfoMenu(CDC* pDC, CRect theClientRect);

	// Attribute
	// Grafiken
	CBitmap bg_intelassignmenu;		///< globales Geheimdienstzuweisungsmen�
	CBitmap bg_intelspymenu;		///< Spionagegeheimdienstmen�
	CBitmap bg_intelsabmenu;		///< Sabotagegeheimdienstmen�
	CBitmap bg_intelreportmenu;		///< Geheimdienstnachrichtenmen�
	CBitmap bg_intelinfomenu;		///< Geheimdienstinformationsmen�
	CBitmap bg_intelattackmenu;		///< Geheimdienstanschlagsmen�

	// Buttons
	CArray<CMyButton*> m_IntelligenceMainButtons;	///< die Buttons unter den Geheimdienstmen�s

	// Hier die Variablen f�r die Geheimdienstansicht
	BYTE m_bySubMenu;						///< welches Untermen� im Geheimdienstmen� wurde aktiviert
	BYTE m_byActiveIntelRace;				///< auf welche gegnerische Rasse beziehen sich die Intelaktionen
	short m_iOldClickedIntelReport;			///< auf welchen Bericht wurde vorher geklickt, braucht man als Modifikator
	CBitmap m_RaceLogos[DOMINION];			///< die Rassensymbole der Hauptrassen
	CBitmap m_RaceLogosDark[DOMINION];		///< die Rassensymbole der Hauptrassen im dunklen Zustand

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	virtual void OnInitialUpdate();
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};


