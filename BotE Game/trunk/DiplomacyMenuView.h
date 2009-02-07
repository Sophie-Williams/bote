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

// CDiplomacyMenuView view

class CDiplomacyMenuView : public CMainBaseView
{
	DECLARE_DYNCREATE(CDiplomacyMenuView)

protected:
	CDiplomacyMenuView();           // protected constructor used by dynamic creation
	virtual ~CDiplomacyMenuView();

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
	void OnNewRound(void);

	void SetSubMenu(BYTE menuID) {m_bySubMenu = menuID;}

	BYTE GetSubMenu() const {return m_bySubMenu;}

private:
	// Funktionen
	/// Funkion legt alle Buttons f�r die Geheimdienstansichten an.
	void CreateButtons();

	/// Funktion zum Zeichnen der Diplomatieansicht
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawDiplomacyMenue(CDC* pDC, CRect theClientRect);

	/// Funktion zeichnet alles f�r die Majorraces in der Diplomatieansicht
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawMajorDiplomacyMenue(CDC* pDC, CRect theClientRect);
	
	/// Funktion zeichnet alles f�r die Minorraces in der Diplomatieansicht
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	void DrawMinorDiplomacyMenue(CDC* pDC, CRect theClientRect);
	
	/// Funktion zeichnet das Informationsmen� in der Diplomatieansicht
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	/// @param whichRace aktuell angeklickte Rasse
	void DrawDiplomacyInfoMenue(CDC* pDC, CRect theClientRect, USHORT whichRace);
	
	/// Funktion zeichnet das Angebotsmen� der Diplomatieansicht
	/// @param pDC Zeiger auf den aktuellen Zeichenkontext
	/// @param theClientRect die komplette Zeichenfl�che
	/// @param whichRace aktuell angeklickte Rasse
	void DrawDiplomacyOfferMenue(CDC* pDC, CRect theClientRect, USHORT whichRece);

	/// Funktion, die einen CString mit dem Status einer MinorRace gegen�ber einer anderen Rasse zur�ckgibt
	/// @param Minorracenummer im Array
	/// @param i Majorrace
	/// @param theClientRect die komplette Zeichenfl�che
	CString PrintDiplomacyStatus(USHORT race, USHORT i, CDC* pDC);

	/// Funktion nimmt die Ressourcen und das Latinum, welches f�r verschenken k�nnen aus den Lagern oder gibt es zur�ck
	/// @param take soll etwas genommen oder zur�ckgegeben werden?
	/// @param i Rasse
	void TakeOrGetbackResLat(BOOL take, USHORT i);

	// Attribute 

	// Grafiken
	CBitmap bg_diploinfomenu;		// Diplomatieinformationsmen�
	CBitmap bg_diploinmenu;			// Diplomatieeingangsmen�
	CBitmap bg_diplooutmenu;		// Diplomatieausgangsmen�

	// Buttons
	CArray<CMyButton*> m_DiplomacyMainButtons;		///< die unteren Buttons in den Diplomatieansichten
	CArray<CMyButton*> m_DiplomacyMajorOfferButtons;///< die einzelnen Angebotsbuttons f�r die Majorraces
	CArray<CMyButton*> m_DiplomacyMinorOfferButtons;///< die einzelnen Angebotsbuttons f�r die Minorraces

	// Hier Variablen f�r die Diplomatieansicht
	USHORT m_bySubMenu;						///< Welcher Button im Diplomatiemenue wurde gedr�ckt, 0 f�r Information, 1 f�r Angebote usw.
	BOOL m_bMajorOrMinor;					// Variable, die mir sagt, ob ich auf eine MinorRace oder eine MajorRace geklickt habe (0 -> Major, 1 -> Minor)
	short m_iClickedOnRace;					// Variable, die mir sagt auf welche Rasse ich geklickt habe
	short m_iClickedOnMajorRace;			// Hilfsvariable, die mir sagt auf welche Hauptrasse ich geklickt habe
	short m_iWhichOfferButtonIsPressed;		// Welcher Angebotsbutton wurde geklicked z.B. 0 f�r Handelsvertag, 6 f�r Geschenk usw.
	USHORT m_iLatinumPayment;				// Betrag an Latinum, welches wir schenken wollen
	USHORT m_iRessourcePayment[5];			// Wenn wir Ressourcen als Geschenk geben wollen, der jeweilige Betrag
	USHORT m_iWhichRessourceIsChosen;		// Welche Ressource haben wir f�rs Geschenk ausgew�hlt? Titan = 0, Deuterium = 1 usw.
	CPoint m_RessourceFromSystem;			// Aus welchem System werden die zu verschenkenden Ressourcen abgekn�pft
	BOOL m_bShowSendButton;					// Soll der Abschicken Button in der Angebotssicht angezeigt werden?
	BOOL m_bShowDeclineButton;				// Soll der Ablehnen Button in der Eingangsansicht angezeigt werden?
	CString m_strDiplomaticText;			// Der Text, der immer im Diplomatiefenster angezeigt wird
	short help,help2;						// Hilfsvariable die Mitz�hlt, auf welche Rasse geklickt wurde
	USHORT m_iWhichRaceCorruption;			// bei Bestechung benutzt, mit wem hat die kleine Rasse den Vertrag der gek�ndigt werden k�nnte
	USHORT m_iDurationOfAgreement;			// Rundendauer des Vertrags mit einer anderen Majorrace (NULL == unbegrenzt, Wert x 10 nehmen!!!)

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
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


