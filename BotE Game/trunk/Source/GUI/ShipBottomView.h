/*
 *   Copyright (C)2004-2011 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
#pragma once
#include "BottomBaseView.h"

// CShipBottomView view

class CShipBottomView : public CBottomBaseView
{
	DECLARE_DYNCREATE(CShipBottomView)

protected:
	CShipBottomView();           // protected constructor used by dynamic creation
	virtual ~CShipBottomView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	/// Funktion f�hrt Aufgaben aus, welche zu jeder neuen Runde von den Views ausgef�hrt werden m�ssen.
	virtual void OnNewRound(void);

	/// Funktion l�dt die rassenspezifischen Grafiken.
	virtual void LoadRaceGraphics();

	/// Funktion gibt zur�ck, ob in der View die Station gezeigt werden soll.
	/// Ist dies nicht der Fall, so werden Schiffe gezeigt.
	/// @return <code>TRUE</code> wenn Station gezeigt wird, bei Schiffen <code>FALSE</code>
	static BOOLEAN IsShowStation(void) {return m_bShowStation;}

	/// Funktion legt fest, ob eine Station oder Schiffe in der View angezeigt werden sollen.
	/// @param is Wahrheitswert: <code>TRUE</code> f�r Stationen, <code>FALSE</code> f�r Schiffe
	static void SetShowStation(BOOLEAN is) {m_bShowStation = is;}

	///	Funktion erstellt zur aktuellen Mouse-Position einen HTML Tooltip
	/// @return	der erstellte Tooltip-Text
	virtual CString CreateTooltip(void);

private:
	// Funktionen
	/// Funktion ermittelt die Nummer des Schiffes im Array, �ber welches die Maus bewegt wurde.
	/// @param pt Mauskoordinate
	/// @return Nummer des Schiffes im Schiffsarray (<code>-1</code> wenn kein Schiff markiert wurde)
	//int GetMouseOverShip(CPoint& pt);

	// Attribute
	CRect m_RectForTheShip;						///< Das Rechteck f�r das Schiff wor�ber wir die Maus bewegt haben
	USHORT m_iPage;								///< bei mehr als 9 Schiffen k�nnen wir auf die n�chste Seite wechseln
	CPoint m_LastKO;							///< Hilfsvariable, die den zuletzt angeklickten Sektor sich merkt
	BOOLEAN m_bShowNextButton;
	Bitmap* m_pShipOrderButton;					///< kleiner Button f�r die Schiffbefehle
	CRect m_ShipOrders[SHIP_ORDER::REPAIR + 1];			///< die ganzen Rechtecke f�r die Schiffsbefehlsbuttons
	CRect m_MainShipOrders[3];					///< die 3 Hauptbuttons f�r die Schiffsbefehle
	short m_iTimeCounter;
	short m_iWhichMainShipOrderButton;			///< welchen Hauptschiffsbefehlsbutton haben wir gedr�ckt
	static BOOLEAN m_bShowStation;

	vector<pair<CRect, CShips*> > m_vShipRects;	///< alle angezeigten Schiffe mit ihren Rechtecken zu draufklicken
	CRect m_rLastMarkedRect;					///< zuletzt markiertes Rechteck

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


