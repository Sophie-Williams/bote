/*
 *   Copyright (C)2004-2009 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#if !defined(AFX_CSmallInfoView_H__ACC3A8E0_2540_4B19_91C2_7274772391FB__INCLUDED_)
#define AFX_CSmallInfoView_H__ACC3A8E0_2540_4B19_91C2_7274772391FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "FontLoader.h"
#include "Planet.h"
#include "Ship.h"
#endif // _MSC_VER > 1000
// CSmallInfoView.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Ansicht CSmallInfoView 

class CSmallInfoView : public CView
{
protected:
	CSmallInfoView();           // Dynamische Erstellung verwendet gesch�tzten Konstruktor
	DECLARE_DYNCREATE(CSmallInfoView)

// Attribute
	CSize m_TotalSize;					///< Gr��e der View in logischen Koordinaten
	static BOOLEAN m_bShowShipInfo;		///< sollen Schiffsinformationen angezeigt werden
	static BOOLEAN m_bShowPlanetInfo;	///< sollen Planeteninformationen angezeigt werden
	static BOOLEAN m_bShowPlanetStats;	///< sollen Planetenstatistiken angezeigt werden
	static CPlanet* m_pPlanet;			///< Planet �ber den Informationen bzw. Statistiken angezeigt werden sollen
	static CShip* m_pShip;				///< Schiff �ber das Informationen angezeigt werden sollen
	int	m_nTimer;						///< Variable welche per Timerbefehl hochgez�hlt wird

public:
	/// Funktion f�hrt Aufgaben aus, welche zu jeder neuen Runde von den Views ausgef�hrt werden m�ssen.
	void OnNewRound(void) {};

	/// Funktion legt fest, ob Schiffsinformationen angezeigt werden sollen
	/// @param is Wahrheitswert
	static void SetShipInfo(BOOLEAN is) {m_bShowShipInfo = is; if (is) m_bShowPlanetInfo = m_bShowPlanetStats = false;}

	/// Funktion legt fest, ob Planeteninformationen angezeigt werden sollen
	/// @param is Wahrheitswert
	static void SetPlanetInfo(BOOLEAN is) {m_bShowPlanetInfo = is; if (is) m_bShowShipInfo = m_bShowPlanetStats = false;}

	/// Funktion legt fest, ob Planetenstatistiken angezeigt werden sollen
	/// @param is Wahrheitswert
	static void SetPlanetStats(BOOLEAN is) {m_bShowPlanetStats = is; if (is) m_bShowShipInfo = m_bShowPlanetInfo = false;}

	/// Funktion legt den anzuzeigenden Planeten fest.
	/// @param planet Zeiger auf den anzuzeigenden Planeten
	static void SetPlanet(CPlanet* planet) { m_pPlanet = planet; }

	/// Funktion legt das anzuzeigende Schiff fest.
	/// @param planet Zeiger auf den anzuzeigenden Planeten
	static void SetShip(CShip* ship) { m_pShip = ship; }

	/// Funktion liefert einen Zeiger auf den aktuell angezeigten Planeten.
	/// @return Zeiger auf Planeten
	static const CPlanet* GetPlanet(void) { return m_pPlanet; }

	/// Funktion liefert einen Zeiger auf das aktuell angezeigte Schiff.
	/// @return Zeiger auf Planeten
	static const CShip* GetShip(void) { return m_pShip; }

// �berschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions�berschreibungen
	//{{AFX_VIRTUAL(CSmallInfoView)
	protected:
	virtual void OnDraw(CDC* pDC);      // �berschrieben zum Zeichnen dieser Ansicht
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CSmallInfoView();	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	/// Funktion rechnet einen �bergebenen Punkt in logische Koordinaten um.
	void CalcLogicalPoint(CPoint &point);

	const CString CheckPlanetClassForInfo(char PlanetClass);		// W�hlt den richtigen Text anhand der Klasse des Planeten
	const CString CheckPlanetClassForInfoHead(char PlanetClass);	// W�hlt die richtige �berschrift anhand der KLasse des Planeten 

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSmallInfoView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // AFX_CSmallInfoView_H__ACC3A8E0_2540_4B19_91C2_7274772391FB__INCLUDED_
