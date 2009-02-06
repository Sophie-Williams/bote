/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "MainBaseView.h"


// CTradeMenuView view

class CTradeMenuView : public CMainBaseView
{
	DECLARE_DYNCREATE(CTradeMenuView)

protected:
	CTradeMenuView();           // protected constructor used by dynamic creation
	virtual ~CTradeMenuView();

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

	/// Funktion zeichnet das B�rsenmenu
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawGlobalTradeMenue(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Monopolmen�
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawMonopolMenue(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet das Transfermen� mit den Aktivit�ten an der Handelsb�rse
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawTradeTransferMenue(CDC* pDC, CRect theClientRect);
	// Attribute

	// Grafiken
	CBitmap bg_trademenu;			// Handelsmen� (B�rse)
	CBitmap bg_monopolmenu;			// Monopolmen�
	CBitmap bg_tradetransfermenu;	// Handelstransfermen�
	CBitmap bg_empty1;				// genereller leerer Bildschirm 

	// Hier die Rechtecke und Variablen, die wir ben�tigen wenn wir in der Handelsansicht sind
	USHORT m_bySubMenu;				// in welchem Untermen� der Handelsansicht befinden wir uns
	double m_dMonopolCosts[5];		// die einzelnen Monopolkosten
	BOOLEAN   m_bCouldBuyMonopols;	// k�nnen wir schon Monopole kaufen

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


