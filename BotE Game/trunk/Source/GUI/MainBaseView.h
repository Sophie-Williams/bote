/*
 *   Copyright (C)2004-2009 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "MyButton.h"
#include "FontLoader.h"
#include "Botf2Doc.h"

// CMainBaseView view

class CMainBaseView : public CView
{
	DECLARE_DYNCREATE(CMainBaseView)

protected:
	CMainBaseView();           // protected constructor used by dynamic creation
	virtual ~CMainBaseView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	/// Funktion setzt ein Untermen�. Diese Funktion sollte von der abgeleiteten Klasse �berschrieben werden.
	/// @param menuID Nummer des Untermen�s
	virtual void SetSubMenu(BYTE menuID) {};

	/// Funktion gibt das aktuelle Untermen� zur�ck. Diese Funktion sollte von der abgeleiteten Klasse �berschrieben werden.
	/// @return <code>NULL</code>
	virtual BYTE GetSubMenu() const {return NULL;}

	/// Funktion f�hrt Aufgaben aus, welche zu jeder neuen Runde von den Views ausgef�hrt werden m�ssen. Sollte von jeder
	/// abgeleiteten Klasse abgeleitet werden.
	virtual void OnNewRound() {};

protected:
	/// Funktion zeichnet die Button und macht alle m�glichen �berpr�fungen.
	/// @param pDC aktueller Zeichenkontext
	/// @param buttonArray Zeiger auf das Feld mit den Buttons
	/// @param counter Z�hlvariable die meinst angibt in welchem aktuellen Men� man sich befindet
	void DrawButtons(CDC* pDC, CArray<CMyButton*>* buttonArray, int counter);
	void DrawGDIButtons(Graphics* g, CArray<CMyButton*>* buttonArray, int counter, Gdiplus::Font &font, Gdiplus::SolidBrush &fontBrush);

	/// Funktion l��t einen Button auf ein MouseOver reagieren
	/// @param point Referenz auf den Punkt, �ber dem sich die Maus aktuell befindet
	/// @param buttonArray Zeiger auf das Feld mit den Buttons
	void ButtonReactOnMouseOver(const CPoint &point, CArray<CMyButton*>* buttonArray);

	/// Funktion l��t einen Button auf einen linken Mausklick reagieren
	/// @param point Referenz auf den Punkt, �ber dem sich die Maus aktuell befindet
	/// @param buttonArray Zeiger auf das Feld mit den Buttons
	/// @param counter Z�hlvariable die meinst angibt in welchem aktuellen Men� man sich befindet
	/// @param invalidate gibt an ob die Funktion automatisch die View neuzeichnen soll
	/// @param onlyActivate gibt an ob der Button beim Klick nur aktiviert, nicht deaktiviert wird
	/// @return wurde ein Button angeklickt gibt die Funktion <code>TRUE</code> zur�ck, ansonsten <code>FALSE</code>
	BOOLEAN ButtonReactOnLeftClick(const CPoint &point, CArray<CMyButton*>* buttonArray, int &counter, BOOLEAN invalidate = TRUE, BOOLEAN onlyActivate = FALSE);

	/// Funktion rechnet einen �bergebenen Punkt in logische Koordinaten um.
	/// @param point Referenz auf umzurechnenden Punkt
	void CalcLogicalPoint(CPoint &point);

	/// Funktion rechnet ein �bergebenes Rechteck in logische Koordinaten um.
	/// @param point Referenz auf umzurechnendes Rechteck
	void CalcDeviceRect(CRect &rect);

	/// Funktion l�dt die richtigen Fonts und Buttonbilder f�r die gro�en Buttons
	/// @param pDC aktueller Zeichenkontext
	void LoadFontForBigButton(CDC* pDC);

	/// Funktion l�dt die richtigen Fonts und Buttonbilder f�r die kleinen Buttons
	/// @param pDC aktueller Zeichenkontext
	void LoadFontForLittleButton(CDC* pDC);

	/// Funktion l�dt die Schriftart inkl. deren Farbe f�r die gespielte Rasse.
	/// @param pDC aktueller Zeichenkontext
	void LoadRaceFont(CDC* pDC);
	
	/// Funktion generiert alle Buttons f�r das Men�. Sollte von der abgeleiteten Klasse �berschrieben werden.
	virtual void CreateButtons() {};	

	// Attribute
	CSize m_TotalSize;				///< Gr��e der View in logischen Koordinaten
	CFont m_Font;					///< aktuelle Font in der View
	
	CBitmap bm;						// meist f�r die Buttons genutzt
	CBitmap bm_dark;	

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
};


