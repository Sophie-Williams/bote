/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
// Planet.h: Schnittstelle f�r die Klasse CPlanet.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLANET_H__DE14ED1A_DE68_4FB1_9C0C_CDFFA6FD4F4C__INCLUDED_)
#define AFX_PLANET_H__DE14ED1A_DE68_4FB1_9C0C_CDFFA6FD4F4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "GraphicPool.h"
#include "Options.h"


//	A ... Gas �berriese							- kalte Zone								z.B. Jupiter
//  B ... Gas Riese								- kalte Zone								z.B. Saturn, Uranus
//	C ... Reducing	(hei�, wasserdampf, CO2)	- normale Zone								z.B. Venus
//	D ... Geo-Plastic (hei�, fl��iges Gestein)	- normale Zone			Abk�hlung -> E		
//	E ... Geo-Metalic (warm, d�nne Oberfl�che)	- normale Zone			Abk�hlung -> F
//	F ... Geo-Crystaline (warm, junge Erde)		- normale Zone			Abk�hlung -> C,M,N
//  G ... Desert								- hei�e Zone
//	H ... Geo-Thermal (junge, fl��iges Gestein)	- kalte Zone			Abk�hlung -> L
//	I ... Asteroiden, Monde						- �berall									z.B. Mond
//	J ... Geo-Morteus (sehr hei�, d�nne Atmo.)	- hei�e Zone								z.B. Merkur
//  K ... Adaptable (warm, Wasser vorhanden)	- normale Zone								z.B. Mars
//  L ... Geo-Inactive (kalt, gefroren, eis)	- kalte Zone								z.B. Pluto
//  M ... Terrestrial (erd�hnlich)				- normale Zone								z.B. Erde
//	N ... Pelagic (wasserreich)					- normale Zone
//  S ... Gas Ultrariesen						- kalte Zone
//  T ... Gas �berriesen (gr��er als A)			- kalte Zone
//	Y ... D�mon (hei�, toxisch, strahlt)		- hei�e Zone

// unbewohnbar sind: A,B,D,H,I,S,T,Y  = 7 (ohne I)
// bewohnbar w�ren: C,E,F,G,J,K,L,M,N = 9 
//enum {M,N,K,C,F,L,G,E,J,A,B,D,H,S,T,Y};	// 16 St�ck
// M ist am besten zum Kolonisieren, am schlechtesten geht J, ab A bis Y sind nicht kolonisierbar
// I wurde entfernt, weil es gibt keine Asteroiden hier


// ******************** nochmal neu mit Klassen von TNB *********************
// A ... Geo-Thermal (junge, fl��iges Gestein)  - kalte Zone			Abk�hlung -> C
// B ... Geo-Morteus (sehr hei�, d�nne Atmo.)	- hei�e Zone
// C ... Geo-Inactive (kalt, teilweise gefroren)- kalte Zone
// D ... Asteroiden								- �berall
// E ... Geo-Plastic (hei�, fl��iges Gestein)	- normale Zone			Abk�hlung -> F
// F ... Geo-Matallic (warm, d�nne Oberfl�che)	- normale Zone			Abk�hlung -> G
// G ... Geo-Crystaline (warm, junge Erde)		- normale Zone			Abk�hlung -> K,L,M,N,O,P
// H ... Desert									- hei�e Zone
// I ... Gas �berriese							- kalte Zone
// J ... Gas Riese								- kalte Zone
// K ... Adaptable (warm, Wasser vorhanden)		- normale Zone
// L ... Dschungel								- normale Zone
// M ... Terrestrial (erd�hnlich)				- normale Zone
// N ... Reducing (hei�, Wasserdampf, CO2)		- hei�e Zone
// O ... Pelagic (wasserreich)					- normale Zone
// P ... Eis									- kalte Zone
// Q ... keine Eigenrotation					- normale Zone
// R ... interstellarer Wanderer				- �berall
// S ... Gas Ultrariesen						- kalte Zone
// T ... Gas �berriesen (gr��er als I)			- kalte Zone
// Y ... D�mon (hei�, toxisch, strahlt)			- hei�e Zone

// M ist am besten, am schlechtesten zu terraformen ist N, von A bis Y gar nicht!
// unbewohnbar sind : A,B,E,I,J,S,T,Y			= 8
// bewohnbar sind:    C,F,G,H,K,L,M,N,O,P,Q,R	= 12

typedef unsigned short USHORT;

class CPlanet  : public CObject  
{
public:
	DECLARE_SERIAL (CPlanet)
	CPlanet();								// Der Standardkonstruktor
	virtual ~CPlanet();						// Der Destruktor
	CPlanet(const CPlanet & rhs);			// Der Kopierkonstruktor
	CPlanet & operator=(const CPlanet &);	// Zuweisungsoperator
	virtual void Serialize(CArchive &ar);	// Die Serialisierungsfunktion
	
	// zum Lesen der Membervaribalen
	BYTE GetSize() const {return m_iSize;}
	BYTE GetType() const {return m_iType;}
	float GetMaxHabitant() const {return m_dMaxHabitant;}
	float GetCurrentHabitant() const {return m_dCurrentHabitant;}
	char GetClass() const {return m_cClass;}
	const CString& GetPlanetName() const {return m_strName;}
	float GetPlanetGrowth() const {return m_dGrowing;}
	BOOLEAN GetTerraformed() const {return m_bTerraformed;}
	BOOLEAN GetIsTerraforming() const {return m_bIsTerraforming;}
	BOOLEAN GetHabitable() const {return m_bHabitable;}
	BOOLEAN GetColonized() const {return m_bColonisized;}
	BYTE GetNeededTerraformPoints() const {return m_iNeededTerraformPoints;}
	BYTE GetStartdTerraformPoints() const {return m_iStartTerraformPoints;}
	const BOOLEAN* GetBoni() {return m_bBoni;}

	/// Funktion gibt einen Dateinamen f�r die Planetengrafik zur�ck. Dieser wird automatisch aus der Nummer der
	/// Grafik und der Planetenklasse generiert.
	CString GetGraphicFile() const;

	// zum Schreiben der Membervariablen
	void SetSize(BYTE Size) {m_iSize = Size;}
	void SetMaxHabitant(float MaxHabitant) {m_dMaxHabitant = MaxHabitant;}
	void SetCurrentHabitant(float CurrentHabitant) {m_dCurrentHabitant = CurrentHabitant;}
	void SetType(BYTE Type) {m_iType = Type;}
	void SetHabitable(BOOLEAN is) {m_bHabitable = is;}
	void SetTerraformed(BOOLEAN is) {m_bTerraformed = is;}
	void SetIsTerraforming(BOOLEAN is) {m_bIsTerraforming = is;}
	void SetColonisized(BOOLEAN is) {m_bColonisized = is;}
	void SetName(CString Name) {m_strName = Name;}
	void SetClass(char Class) {m_cClass = Class;}
	void SetPlanetGrowth() {m_dGrowing = m_dMaxHabitant/4;} // Wenn hier was ge�ndert, auch in GeneratePlanet was �ndern
	BOOLEAN SetNeededTerraformPoints(BYTE sub);				// Subtrahiert "sub" von den Terraformpoints, bei kleiner 0 wird der Plani auf m_bTerraformed = TRUE gesetzt
	void SetBoni(BOOLEAN titan, BOOLEAN deuterium, BOOLEAN duranium, BOOLEAN crystal, BOOLEAN iridium, BOOLEAN dilithium,
		BOOLEAN food, BOOLEAN energy);
	
	/// Funktion setzt die Nummer f�r das Graphikfile des Planeten.
	/// @param n Nummer der Grafik
	void SetGraphicType(BYTE n) {m_iGraphicType = n;}
	
	/**
	* Funktion setzt die anf�nglichen Terraformpunkte f�r einen Planeten fest. Normalerweise geschieht das bei der 
	* Generierung des Planeten automatisch, wenn wir aber manuell Planeten anlegen wollen, dann m�ssen wir auch
	* diese Funktion aufrufen.
	*/
	void SetStartTerraformPoints(BYTE startPoints = 0) {m_iStartTerraformPoints = m_iNeededTerraformPoints = startPoints;}
	
	// Sonstige Funktionen
	void PlanetGrowth();
	const BYTE GeneratePlanet(BYTE KindOfLastPlanet, CString NameOfSunSystem, BYTE NumberOfPlanet, BOOLEAN MinorRace);
	
	/// Funktion zeichnet den Planeten in die untere Planetenansicht.
	/// @param g Referenz auf das Grafikobjekt
	/// @param planetRect das Rechteck um den Planeten
	/// @param graphicPool Zeiger auf die Sammlung aller Grafiken
	void DrawPlanet(Graphics &g, CRect planetRect, CGraphicPool* graphicPool);
	
	/**
	 * Diese Funktion generiert einen eventuellen Bonus anhand einer speziellen Wahrscheinlichkeitstabelle.
	 */
	void GenerateBoni();
	
	/**
	 * Die Resetfunktion f�r die CPlanet Klasse, welche alle Werte wieder auf Ausgangswerte setzt.
	 */
	void Reset();

private:
	BYTE m_iPosition;			// Position des Planeten = Nummer, die bei Generate �bergeben wird!
	BYTE m_iSize;				// Gr��e das Planeten, siehe ENUM dazu
	BYTE m_iGraphicType;		// Grafiknummer des Planeten
	BYTE m_iType;				// Typ des Planeten, siehe ENUM dazu (ohne SUN)
	float m_dMaxHabitant;		// maximale Anzahl der Bewohner
	float m_dCurrentHabitant;	// aktuelle Anzahl der Bewohner
	float m_dGrowing;			// Das prozentuale Wachstum der Bev�lkerung des Planeten, z.B. 0.2%
	BOOLEAN m_bHabitable;		// Ist der Planet kolonisierbar?
	BOOLEAN m_bColonisized;		// Ist der Planet bewohnt?
	BOOLEAN m_bTerraformed;		// Wurde der Planet terraformt?
	BOOLEAN m_bIsTerraforming;	// Wird der Planet gerade geterraformt?
	CString m_strName;			// Name des Planeten
	BYTE m_iNeededTerraformPoints;// n�tige Terraformpunkte um den Planeten zu terraformen
	BYTE m_iStartTerraformPoints; // n�tigen Terraformpunkte am Anfang, brauchen wir um den prozentualen Fortschritt berechnen zu k�nnen
	char m_cClass;				// Die Klasse nochmal als Buchstabe
	BOOLEAN m_bBoni[8];			// Gibt es einen bestimmten Bonus auf dem Planeten, TITAN, ..., IRDIUM, DILITHIUM, FOOD, ENERGY
};

#endif // !defined(AFX_PLANET_H__DE14ED1A_DE68_4FB1_9C0C_CDFFA6FD4F4C__INCLUDED_)
