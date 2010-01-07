/*
 *   Copyright (C)2004-2010 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
// Statistcs.h: Schnittstelle f�r die Klasse CStatistcs.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include "Options.h"

// forward declaration
class CBotf2Doc;

class CStatistics : public CObject  
{
public:
	DECLARE_SERIAL (CStatistics)
	
	/// Standardkonstruktor
	CStatistics(void);
	/// Standarddestruktor
	virtual ~CStatistics(void);
	/// Serialisierungsfunktion
	virtual void Serialize(CArchive &ar);
	
	// Zugriffsfunktionen	
	// zum Lesen der Membervariablen

	/// Funktion gibt das universumweite Durchschnittstechlevel zur�ck.
	/// @return durchschnittliches Techlevel aller Hauptrassen
	BYTE GetAverageTechLevel(void) const {return m_byAverageTechLevel;}

	/// Funktion gibt Feld mit durchschnittlichen Ressourcenlagern zur�ck.
	/// @return Feld der durchschnittliches Menge im Ressourcenlager
	UINT* GetAverageResourceStorages(void) {return m_nAverageResourceStorages;}

	/// Funktion gibt Map mit den Schiffsst�rken aller Rassen zur�ck.
	/// @param sRaceID Rassen-ID f�r die die Schiffsst�rke erfragt werden soll
	/// @return Schiffsst�rke der Kriegsschiffe
	UINT GetShipPower(const CString& sRaceID) const;
	
	// zum Schreiben der Membervariablen
	/// Funktion zum Berechnen aller Statistiken.
	/// @param pDoc Zeiger auf das Dokument
	void CalcStats(CBotf2Doc* pDoc);

	/// Funktion zum zur�cksetzen aller Werte auf Ausgangswerte.
	void Reset(void);

private:
	// private Funktionen

	/// Funktion zum Berechnen des universumweiten Techdurchschnittlevels.
	/// @param pDoc Zeiger auf das Dokument
	void CalcAverageTechLevel(CBotf2Doc* pDoc);

	/// Funktion zum Berechnen der durchschnittlichen Bef�llung der Ressourcenlager.
	/// @param pDoc Zeiger auf das Dokument
	void CalcAverageResourceStorages(CBotf2Doc* pDoc);

	/// Funktion zum Berechnen der gesamten milit�rischen Schiffsst�rken aller Rassen.
	/// @param pDoc Zeiger auf das Dokument
	void CalcShipPowers(CBotf2Doc* pDoc);
	
	// Attribute
	BYTE m_byAverageTechLevel;						///< Durchschnittliches Techlevel aller Rassen
	
	UINT m_nAverageResourceStorages[DILITHIUM + 1];	///< Durschschnittlicher Inhalt der Ressourcenlager	
	
	std::map<CString, UINT> m_mShipPowers;			///< Schiffsst�rken aller Rassen
};