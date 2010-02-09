/*
 *   Copyright (C)2004-2010 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de
 *
 */
#pragma once
#include "afx.h"

// forward declaration
class CBotf2Doc;
class CMajor;

class CTradeRoute :	public CObject
{
public:
	DECLARE_SERIAL (CTradeRoute)
	/// Konstruktor
	CTradeRoute(void);

	/// Destruktor
	~CTradeRoute(void);

	/// Kopierkonstruktor
	CTradeRoute(const CTradeRoute & rhs);
	
	/// Zuweisungsoperatur
	CTradeRoute & operator=(const CTradeRoute &);

	/// Serialisierungsfunktion
	virtual void Serialize(CArchive &ar);

	/// Funktion legt die Handelsroute zum System mit der Koordinate <code>ko</code> an.
	void GenerateTradeRoute(CPoint ko);

	/// Funktion gibt die Zielkoordinate der Handelsroute zur�ck
	CPoint GetDestKO() const {return m_KO;}
	
	/// Funktion gibt das Latinum inkl. Boni auf Handelroute, welches man durch die Handelsroute bekommt zur�ck
	USHORT GetLatinum(short boni) const;

	/// Funktion gibt die verbleibende Dauer der Handelsroute zur�ck. Ist die Dauer negativ, so wird sie zum Ende 
	/// dieser Zeit aufgehoben.
	short GetDuration() const {return m_iDuration;}

	/// Funktion setzt die Dauer der Handelsroute. Ein positiver Paramter <code>duration</code> bedeutet, dass die
	/// Handelsroute noch solange Bestand hat, ein negativer hingegen bedeutet, dass die Handelsroute bei erreichen
	/// dieser Dauer aufgel�st wird.
	void SetDuration(short duration) {m_iDuration = duration;}

	/// Funktion legt manuell das erzeugte Latinum fest.
	/// @param latinum erzeugtes Latinum der Handelsroute
	void SetLatinum(USHORT latinum) {m_iLatinum = latinum;}

	/// Funktion �berpr�ft, ob die Handelsroute noch Bestand haben darf und setzt das Latinum, welches
	/// diese Handelsroute fabriziert. Dabei werden noch keinerlei Boni auf die Latinumproduktion angerechnet.
	/// Die Funktion gibt einen Wahrheitswert zur�ck, der sagt, ob die Handelsroute noch Bestand haben darf.
	BOOLEAN CheckTradeRoute(const CPoint& pFrom, const CPoint& pDest, CBotf2Doc* pDoc);

	/// Diese Funktion verbessert manchmal die Beziehung zu der Minorrace, die in dem betroffenem Sektor lebt.
	/// Als Parameter wird hierf�r die Ausgangskoordinate der Handerlsroute, die Zielkoordinate <code>pDest</code>
	/// und ein Zeiger auf das Dokument <code>pDoc</code> �bergeben.
	void PerhapsChangeRelationship(const CPoint& pFrom, const CPoint& pDest, CBotf2Doc* pDoc);

	/// Funktion zeichnet die Handelsroute auf der Galaxiekarte. �bergeben werden daf�r die Koordinate des Systems
	/// von wo die Handelsroute startet <code>start</code> sowie auch der Besitzer dieses Systems <code>pMajor</code>.
	void DrawTradeRoute(CDC* pDC, CPoint start, const CMajor* pMajor);
	
private:
	/// Die Koordinate des Sektors, zu dem die Handelsroute verl�uft
	CPoint m_KO;

	/// Das fabrizierte Latinum der Handelsroute ohne irgendwelche Boni
	USHORT m_iLatinum;

	/// Wieviele Runden l�uft die Handelsroute noch? Beim Anlegen l�uft sie 20 Runden lang, wird sie in dieser
	/// Zeit nicht gek�ndigt, so verl�ngert sich diese Zeit wieder um 20 Runden. Eine K�ndigung dauert 5 Runden.
	short m_iDuration;
};
