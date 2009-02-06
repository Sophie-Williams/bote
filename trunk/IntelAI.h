/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once
#include "Options.h"

/**
 * Diese Klasse beinhaltet bestimmte Priorit�ten, welche im Zusammenhang des Geheimdienstes stehen. Zus�tzlich finden
 * hier die Berechnungen der KI f�r den Geheimdienst statt.
 */
class CBotf2Doc;
class CIntelAI
{
public:
	/// Konstruktor
	CIntelAI(void);
	
	/// Destruktor
	~CIntelAI(void);

	// Zugriffsfunktionen
	/// Funktion gibt die Priorit�t f�r den Geheimdienst f�r eine Rasse zur�ck.
	/// @param race gew�nschte Rasse
	BYTE GetIntelPrio(BYTE race) {ASSERT(race); return m_byIntelPrio[race-1];}

	// sonstige Funktionen
	/// Funktion berechnet den Umgang mit dem Geheimdienst f�r die KI. Sie trifft Zuteilungen f�r die einzelnen Rassen.
	/// @param pDoc Zeiger auf das Dokument
	void CalcIntelligence(CBotf2Doc* pDoc);

	/// Resetfunktion f�r ein CIntelAI-Objekt.
	void Reset(void);

private:
	// Attribute
	BYTE m_byIntelPrio[DOMINION];		///< beinhaltet die Priorit�t zum Bau von Geheimdienstgeb�uden


};
