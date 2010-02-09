/*
 *   Copyright (C)2004-2010 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
#pragma once
#include "afx.h"
#include "afxtempl.h"

class CGlobalBuildings : public CObject
{
public:
	DECLARE_SERIAL (CGlobalBuildings)
	// Standardkonstruktor
	CGlobalBuildings(void);

	// Destruktor
	~CGlobalBuildings(void);

	// Kopierkonstruktor
	CGlobalBuildings(const CGlobalBuildings & rhs);
	
	// Zuweisungsoperatur
	CGlobalBuildings & operator=(const CGlobalBuildings &);

	// Die Serialisierungsfunktion
	virtual void Serialize(CArchive &ar);

	// Zugriffsfunktionen
// zum Lesen der Membervariablen
	// Funktion gibt einen Zeiger auf das Feld aller globalen Geb�ude zur�ck
	CArray<USHORT,USHORT>* GetGlobalBuildings() {return &m_GlobalBuildings;}

// zum Schreiben der Membervariablen
	void AddGlobalBuilding(USHORT id) {m_GlobalBuildings.Add(id);}

// sonstige Funktionen
	// Funktion l�scht ein globales Geb�ude aus dem Feld der globalen Geb�ude. Diese Funktion sollte aufgerufen
	// werden, wenn wir ein solches Geb�ude abrei�en oder verlieren.
	void DeleteGlobalBuilding(USHORT id);

	// Resetfunktion f�r die Klasse CGlobalBuildings
	void Reset();

private:
	// Dieses Feld beinhaltet die Geb�ude, welche ein imperienweites Attribut haben und in einer Bauliste stehen, sowei
	// alle Geb�ude die in allen Systemen stehen. Kommen mehrere gleiche Geb�ude vor, so ist deren ID auch mehrmals
	// hier in dem Feld vorhanden.
	CArray<USHORT,USHORT> m_GlobalBuildings;
};
