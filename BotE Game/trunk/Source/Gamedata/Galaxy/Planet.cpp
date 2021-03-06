// Planet.cpp: Implementierung der Klasse CPlanet.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Planet.h"
#include "GraphicPool.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL (CPlanet, CObject, 1)
//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
CPlanet::CPlanet(void)
{
	Reset();
}

CPlanet::~CPlanet(void)
{
}

///////////////////////////////////////////////////////////////////////
// Kopierkonstruktor
///////////////////////////////////////////////////////////////////////
CPlanet::CPlanet(const CPlanet & rhs)
{
	m_bIsTerraforming = rhs.m_bIsTerraforming;
	m_dCurrentHabitant = rhs.m_dCurrentHabitant;
	m_dMaxHabitant = rhs.m_dMaxHabitant;
	m_iSize = rhs.m_iSize;
	m_iType = rhs.m_iType;
	m_strName = rhs.m_strName;
	m_dGrowing = rhs.m_dGrowing;
	m_iGraphicType = rhs.m_iGraphicType;
	m_iNeededTerraformPoints = rhs.m_iNeededTerraformPoints;
	m_iStartTerraformPoints = rhs.m_iStartTerraformPoints;
	m_bHasIndividualGraphic = rhs.m_bHasIndividualGraphic;
	for (int i = 0; i < 8; i++)
		m_bBoni[i] = rhs.m_bBoni[i];
}

///////////////////////////////////////////////////////////////////////
// Zuweisungsoperator
///////////////////////////////////////////////////////////////////////
CPlanet & CPlanet::operator=(const CPlanet & rhs)
{
	if (this == &rhs)
		return *this;
	m_bIsTerraforming = rhs.m_bIsTerraforming;
	m_dCurrentHabitant = rhs.m_dCurrentHabitant;
	m_dMaxHabitant = rhs.m_dMaxHabitant;
	m_iSize = rhs.m_iSize;
	m_iType = rhs.m_iType;
	m_strName = rhs.m_strName;
	m_dGrowing = rhs.m_dGrowing;
	m_iGraphicType = rhs.m_iGraphicType;
	m_iNeededTerraformPoints = rhs.m_iNeededTerraformPoints;
	m_iStartTerraformPoints = rhs.m_iStartTerraformPoints;
	for (int i = 0; i < 8; i++)
		m_bBoni[i] = rhs.m_bBoni[i];
	return *this;
}

///////////////////////////////////////////////////////////////////////
// Speichern / Laden
///////////////////////////////////////////////////////////////////////
void CPlanet::Serialize(CArchive &ar)
{
	CObject::Serialize(ar);
	// wenn gespeichert wird
	if (ar.IsStoring())
	{
		ar << m_bIsTerraforming;
		ar << m_dCurrentHabitant;
		ar << m_dMaxHabitant;
		ar << m_iSize;
		ar << m_iType;
		ar << m_strName;
		ar << m_dGrowing;
		ar << m_iGraphicType;
		ar << m_iNeededTerraformPoints;
		ar << m_iStartTerraformPoints;
		ar << m_bHasIndividualGraphic;
		for (int i = 0; i < 8; i++)
			ar << m_bBoni[i];
	}
	// wenn geladen wird
	if (ar.IsLoading())
	{
		ar >> m_bIsTerraforming;
		ar >> m_dCurrentHabitant;
		ar >> m_dMaxHabitant;
		int nSize;
		ar >> nSize;
		m_iSize = (PLANT_SIZE::Typ)nSize;
		ar >> m_iType;
		ar >> m_strName;
		ar >> m_dGrowing;
		ar >> m_iGraphicType;
		ar >> m_iNeededTerraformPoints;
		ar >> m_iStartTerraformPoints;
		ar >> m_bHasIndividualGraphic;
		for (int i = 0; i < 8; i++)
			ar >> m_bBoni[i];
	}
}

/// Funktion gibt einen Dateinamen f�r die Planetengrafik zur�ck. Dieser wird automatisch aus der Nummer der
/// Grafik und der Planetenklasse generiert.
CString CPlanet::GetGraphicFile() const
{
	CString fileName;
	fileName.Format("Planets\\class%c%02d.bop", GetClass(), m_iGraphicType);
	return fileName;
}

CString CPlanet::GetPlanetGraphicFile() const
{
	CString fileName;
	if(m_bHasIndividualGraphic)
		fileName = "Planets\\" + m_strName + ".bop";
	else
		fileName = GetGraphicFile();
	return fileName;
}

/// Funktion erzeugt einen Planeten.
/// @param sSectorName Sektorname
/// @param nLastZone Zone des zuletzt erzeugten Planeten (neuer Planet kann nicht in einer vorherigen Zone sein)
/// @param byPlanetNumer Anzahl schon erzeugeter Planeten in diesem Sektor
/// @param bMinor Minorrace im Sektor
/// @return Zone des erzeugten Planeten
PLANET_ZONE::Typ CPlanet::Create(const CString& sSectorName, PLANET_ZONE::Typ nLastZone, BYTE byPlanetNumber, BOOLEAN bMinor)
{
	// Standardwerte initialisieren
	Reset();

	BYTE m_iRandomType = 0;	// Zufallszahl, die den Type bestimmt (abh�ngug vom DEFINE!)
	PLANET_ZONE::Typ nZone = nLastZone; // Zone in der der generierte Planet ist
	// Planetenklassen bestimmen, abh�ngig von der Zone und dem vorherigen Planeten

	// Es k�nnten alle Klassen auftreten, aber halt mit unterschiedlicher Wahrscheinlichkeit
	if (nLastZone == PLANET_ZONE::HOT)
	{
		int WhatPlanet = rand()%75;
		// Planeten aus kalter Zone
		nZone = PLANET_ZONE::COOL;
		if (WhatPlanet == 0) m_iRandomType = PLANET_CLASSES::PLANETCLASS_I;
		else if (WhatPlanet == 1) m_iRandomType = PLANET_CLASSES::PLANETCLASS_J;
		else if (WhatPlanet == 2) m_iRandomType = PLANET_CLASSES::PLANETCLASS_S;
		else if (WhatPlanet == 3) m_iRandomType = PLANET_CLASSES::PLANETCLASS_T;
		else if (WhatPlanet == 4) m_iRandomType = PLANET_CLASSES::PLANETCLASS_A;
		else if (WhatPlanet == 5) m_iRandomType = PLANET_CLASSES::PLANETCLASS_C;
		else if (WhatPlanet == 6) m_iRandomType = PLANET_CLASSES::PLANETCLASS_P;
		else
		{
			// Planeten aus gem��igter Zone
			nZone = PLANET_ZONE::TEMPERATE;
			if (WhatPlanet > 6)	m_iRandomType = PLANET_CLASSES::PLANETCLASS_E;
			if (WhatPlanet > 11) m_iRandomType = PLANET_CLASSES::PLANETCLASS_F;
			if (WhatPlanet > 16) m_iRandomType = PLANET_CLASSES::PLANETCLASS_G;
			if (WhatPlanet > 21) m_iRandomType = PLANET_CLASSES::PLANETCLASS_K;
			if (WhatPlanet > 26) m_iRandomType = PLANET_CLASSES::PLANETCLASS_L;
			if (WhatPlanet > 31) m_iRandomType = PLANET_CLASSES::PLANETCLASS_M;	// seltener
			if (WhatPlanet > 35) m_iRandomType = PLANET_CLASSES::PLANETCLASS_O;
			if (WhatPlanet > 40) m_iRandomType = PLANET_CLASSES::PLANETCLASS_Q;	// selten
			if (WhatPlanet > 43) m_iRandomType = PLANET_CLASSES::PLANETCLASS_R; // selten
			// Planeten aus hei�er Zone
			if (WhatPlanet > 46)
			{
				m_iRandomType = PLANET_CLASSES::PLANETCLASS_H;
				nZone = PLANET_ZONE::HOT;
			}
			if (WhatPlanet > 53) m_iRandomType = PLANET_CLASSES::PLANETCLASS_B;
			if (WhatPlanet > 60) m_iRandomType = PLANET_CLASSES::PLANETCLASS_N;
			if (WhatPlanet > 67) m_iRandomType = PLANET_CLASSES::PLANETCLASS_Y;
		}
	}
	else if (nLastZone == PLANET_ZONE::TEMPERATE)
	{
		int WhatPlanet = rand()%47;
		// Planeten aus kalter Zone
		nZone = PLANET_ZONE::COOL;
		if (WhatPlanet == 0) m_iRandomType = PLANET_CLASSES::PLANETCLASS_I;
		else if (WhatPlanet == 1) m_iRandomType = PLANET_CLASSES::PLANETCLASS_J;
		else if (WhatPlanet == 2) m_iRandomType = PLANET_CLASSES::PLANETCLASS_S;
		else if (WhatPlanet == 3) m_iRandomType = PLANET_CLASSES::PLANETCLASS_T;
		else if (WhatPlanet == 4) m_iRandomType = PLANET_CLASSES::PLANETCLASS_A;
		else if (WhatPlanet == 5) m_iRandomType = PLANET_CLASSES::PLANETCLASS_C;
		else if (WhatPlanet == 6) m_iRandomType = PLANET_CLASSES::PLANETCLASS_P;
		else
		{
			// Planeten aus gem��igter Zone
			nZone = PLANET_ZONE::TEMPERATE;
			if (WhatPlanet > 6) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_E;
			if (WhatPlanet > 11) m_iRandomType = PLANET_CLASSES::PLANETCLASS_F;
			if (WhatPlanet > 16) m_iRandomType = PLANET_CLASSES::PLANETCLASS_G;
			if (WhatPlanet > 21) m_iRandomType = PLANET_CLASSES::PLANETCLASS_K;
			if (WhatPlanet > 26) m_iRandomType = PLANET_CLASSES::PLANETCLASS_L;
			if (WhatPlanet > 31) m_iRandomType = PLANET_CLASSES::PLANETCLASS_M;	// seltener
			if (WhatPlanet > 35) m_iRandomType = PLANET_CLASSES::PLANETCLASS_O;
			if (WhatPlanet > 40) m_iRandomType = PLANET_CLASSES::PLANETCLASS_Q;
			if (WhatPlanet > 43) m_iRandomType = PLANET_CLASSES::PLANETCLASS_R;
		}
	}
	else if (nLastZone == PLANET_ZONE::COOL)
	{
		nZone = PLANET_ZONE::COOL;
		int WhatPlanet = rand()%18;
		if (WhatPlanet == 0) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_I;
		else if (WhatPlanet == 1) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_J;
		else if (WhatPlanet == 2) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_S;
		else if (WhatPlanet == 3) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_T;
		else
		{
			if (WhatPlanet >= 4) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_A;
			if (WhatPlanet >= 8) m_iRandomType  = PLANET_CLASSES::PLANETCLASS_C;
			if (WhatPlanet >= 12) m_iRandomType = PLANET_CLASSES::PLANETCLASS_P;
			if (WhatPlanet >= 17) m_iRandomType = PLANET_CLASSES::PLANETCLASS_R;
		}
	}

	BYTE m_iRandomSize = rand()%3;	// Zufallszahl, die die Gr��e bestimmt (abh�ngig vom DEFINE!)

	// zuf�llige Planetengrafik ausw�hlen
	m_iGraphicType = rand()%PLANET_CLASSES::GRAPHICNUMBER;

	m_iSize = (PLANT_SIZE::Typ)m_iRandomSize;
	bool habitable = true;
	// Bestimmen, ob der Planet �berhaupt bewohnbar ist! A,B,E,I,J,S,T,Y
	if (m_iRandomType == PLANET_CLASSES::PLANETCLASS_A || m_iRandomType == PLANET_CLASSES::PLANETCLASS_B || m_iRandomType == PLANET_CLASSES::PLANETCLASS_E
		|| m_iRandomType == PLANET_CLASSES::PLANETCLASS_I || m_iRandomType == PLANET_CLASSES::PLANETCLASS_J || m_iRandomType == PLANET_CLASSES::PLANETCLASS_S
		|| m_iRandomType == PLANET_CLASSES::PLANETCLASS_T || m_iRandomType == PLANET_CLASSES::PLANETCLASS_Y)
		habitable = false;
	// Ein erd�hnlicher Planet ist schon terraformed

	if (m_iRandomType == PLANET_CLASSES::PLANETCLASS_I || m_iRandomType == PLANET_CLASSES::PLANETCLASS_J || m_iRandomType == PLANET_CLASSES::PLANETCLASS_S || m_iRandomType == PLANET_CLASSES::PLANETCLASS_T)
		m_iSize = PLANT_SIZE::GIANT;			// Gasriesen sind immer riesig

	if (!habitable)
	{
		m_dMaxHabitant = 0;
	}
	else
	{
		if (m_iRandomType < PLANET_CLASSES::PLANETCLASS_A)	// Ab Klasse A sind alle Planeten eh nicht mehr kolonisierbar
		{
			// Einen Muliplikator anlegen (0,8 ; 1,0 ; 1,2)
			float multi = 0.0f;
			// Welcher Multiplikator soll gew�hlt werden?
			int miniRandom = rand()%3;
			if (miniRandom == 0) multi = 0.8f;
			else if (miniRandom == 1) multi = 1.0f;
			else if (miniRandom == 2) multi = 1.2f;

			// Zufallszahl ermitteln, abh�ngig von der max. Einwohnerzahl, die sp�ter draufaddiert wird
			int random = rand()%(((m_iRandomSize+1)*(12-m_iRandomType))/6 + 1);
			// Max. Einwohner werden berechnet, inkl. "random " Zufallszahl
			float Habitants = ((((m_iRandomSize+1)*(12-m_iRandomType)+random)*1000)*multi*0.5f);
			// z.B. f�r kleine Erde (M): 1*12+z.B. 2 = 11 * 1000 = 9000 -> 11000 Mill. Einwohner (11 Mrd.)
			m_dMaxHabitant = Habitants;
			m_dMaxHabitant = m_dMaxHabitant/1000;

			// hier noch die ben�tigten Terraformingpunkte berechnen. Umso besser die Klasse des Planeten, umso weniger
			// Punkte brauchen wir. Klasse M braucht gar keine Terraformpunkte weil er ja schon geterraformt ist
			m_iNeededTerraformPoints = (int)(m_iRandomType * (m_iRandomSize + 1) * (rand()%11 +5) / 10);
			if (m_iNeededTerraformPoints == 0 && m_iRandomType != PLANET_CLASSES::PLANETCLASS_M)
				m_iNeededTerraformPoints++;
			m_iStartTerraformPoints = m_iNeededTerraformPoints;
		}
		else
		{
			m_dMaxHabitant = 0;
		}
	}

	// Wenn eine MinorRace da ist, dann ein paar Planeten schon geterraformt machen
	if (bMinor && habitable)
	{
		BYTE random;
		random = rand()%5+1;
		if (random >= 5)
		{
			m_iNeededTerraformPoints = 0;
			m_iStartTerraformPoints = 0;
		}
	}
	// Wenn eine MinorRace in dem System ist, dann gleich ein paar Einwohner generieren
	if (bMinor && GetTerraformed())
	{
		BYTE randDiv;
		randDiv = rand()%8+1;
		m_dCurrentHabitant = m_dMaxHabitant/randDiv;
	}

	m_iType = m_iRandomType;

	// Wachstumsprozent des Planeten berechnen
	SetPlanetGrowth();

	// Namen f�r den Planeten geben, besteht aus Namen des Sonnensystems und der Nummer
	m_strName.Format("%s %i",sSectorName, byPlanetNumber + 1);

	// eventuelle Boni durch den Planeten berechnen
	GenerateBoni();

	return nZone;
}

void CPlanet::DrawPlanet(Graphics &g, const CRect& rect, CGraphicPool* graphicPool) const
{
	Gdiplus::PixelOffsetMode oldPixelOffsetMode = g.GetPixelOffsetMode();

	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	AssertBotE(graphicPool);

	Bitmap* planet = graphicPool->GetGDIGraphic(GetPlanetGraphicFile());

	if (planet)
		g.DrawImage(planet, rect.left, rect.top, rect.Width(), rect.Height());

	Color c;
	if (!GetHabitable())
		c.SetFromCOLORREF(RGB(0,0,255));
	else if (GetTerraformed())
	{
		if (m_dCurrentHabitant > 0.0f)
			c.SetFromCOLORREF(RGB(0,255,30));
		else
			c.SetFromCOLORREF(RGB(180,255,180));
	}
	else if (m_bIsTerraforming == TRUE)
	{
		c.SetFromCOLORREF(RGB(200,200,0));
		// Gitternetz zeichnen
		if (planet)
		{
			Bitmap* sphere = graphicPool->GetGDIGraphic("Planets\\TerraformSphere.bop");
			if (sphere)
				g.DrawImage(sphere, rect.left, rect.top, rect.Width(), rect.Height());
		}
	}
	else
		c.SetFromCOLORREF(RGB(255,40,40));
	SolidBrush brush(c);

	Gdiplus::Font font(L"Arial", 8.5);
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentFar);

	CRect planetRect = rect;
	// Rechteck ein wenig nach unten schieben
	planetRect.OffsetRect(0, 20);

	CString s;
	// Planetenklasse unter den Planeten zeichnen
	s.Format("%c", GetClass());
	g.DrawString(CComBSTR(s), -1, &font, RectF((REAL)planetRect.left, (REAL)planetRect.top, (REAL)planetRect.Width(), (REAL)planetRect.Height()), &format, &brush);

	// prozentuale Angabe des Terraformfortschrittes anzeigen
	if (m_bIsTerraforming == TRUE)
	{
		planetRect.OffsetRect(0, 15);
		s.Format("%d %%",100 - (short)(this->m_iNeededTerraformPoints * 100 / this->m_iStartTerraformPoints));
		g.DrawString(CComBSTR(s), -1, &font, RectF((REAL)planetRect.left, (REAL)planetRect.top, (REAL)planetRect.Width(), (REAL)planetRect.Height()), &format, &brush);
	}

	// Symbole f�r eventuell vohandene Boni zeichnen
	int x = rect.CenterPoint().x - 5;
	int y = rect.top - 23;
	// erstmal schauen, wieviele Boni �berhaupt vorhanden sind
	BYTE n = 0;
	for (int i = 0; i < 8; i++)
		n += m_bBoni[i];
	x -= (n*9-4);

	Bitmap* graphic = NULL;
	for (int i = 0; i < 8; i++)
		if (m_bBoni[i])
		{
			switch(i)
			{
				case RESOURCES::TITAN:		graphic = graphicPool->GetGDIGraphic("Other\\titanSmall.bop"); break;
				case RESOURCES::DEUTERIUM: graphic = graphicPool->GetGDIGraphic("Other\\deuteriumSmall.bop"); break;
				case RESOURCES::DURANIUM:	graphic = graphicPool->GetGDIGraphic("Other\\duraniumSmall.bop"); break;
				case RESOURCES::CRYSTAL:	graphic = graphicPool->GetGDIGraphic("Other\\crystalSmall.bop"); break;
				case RESOURCES::IRIDIUM:	graphic = graphicPool->GetGDIGraphic("Other\\iridiumSmall.bop"); break;
				case RESOURCES::DERITIUM:	graphic = graphicPool->GetGDIGraphic("Other\\Deritium.bop"); break;
				case 6:			graphic = graphicPool->GetGDIGraphic("Other\\foodSmall.bop"); break;
				case 7:			graphic = graphicPool->GetGDIGraphic("Other\\energySmall.bop"); break;
			}
			if (graphic)
			{
				g.DrawImage(graphic, x, y, 20, 16);
				x += 18;
				graphic = NULL;
			}
		}

	g.SetPixelOffsetMode(oldPixelOffsetMode);
}

/// Planetenwachstum durchf�hren
void CPlanet::PlanetGrowth(void)
{
	float tempCurrentHabitant = m_dCurrentHabitant;
	if (m_dCurrentHabitant < m_dMaxHabitant)
		m_dCurrentHabitant = m_dCurrentHabitant+m_dCurrentHabitant*m_dGrowing/100;
	if (m_dCurrentHabitant < (tempCurrentHabitant + 0.1f) && GetInhabited())
		m_dCurrentHabitant = tempCurrentHabitant + 0.1f;	// immer minimales Wachstum von 0.1 Mrd. pro Runde
	if (m_dCurrentHabitant > m_dMaxHabitant)
		m_dCurrentHabitant = m_dMaxHabitant;				// Wenn MaxHabitant erreicht ist, nat�rlich kein Wachstum mehr
}

/// Subtrahiert "sub" von den Terraformpoints, bei kleiner 0 wird der Plani auf m_bTerraformed = TRUE gesetzt und ein
/// TRUE zur�ckgegeben, ansonsten wird ein FALSE zur�ckgegeben
/// Wenn der Planet noch nicht fertig geterraformt ist, wird m_bIsTerraforming auf TRUE gesetzt
BOOLEAN CPlanet::SetNeededTerraformPoints(const unsigned sub)
{
	const int result = m_iNeededTerraformPoints - sub;
	m_iNeededTerraformPoints = max(result, 0);
	if (m_iNeededTerraformPoints == 0)
	{
		m_bIsTerraforming = FALSE;
		return TRUE;
	}
	m_bIsTerraforming = TRUE;
	return FALSE;
}

/// Diese Funktion generiert einen eventuellen Bonus anhand einer speziellen Wahrscheinlichkeitstabelle.
void CPlanet::GenerateBoni()
{
	// die Wahrscheinlichkeitstablle wird angelegt
	// Matrix aus allen Planetenklassen und aller m�glichen Boni
	BYTE probs[PLANET_CLASSES::PLANETCLASS_T+1][8] = {

	// Testweise die Deritiumprozente um 10 erh�ht (nur dort wo nicht NULL drin stand)
	//		RESOURCES::TITAN		RESOURCES::DEUTERIUM	RESOURCES::DURANIUM	KRISTALLE	RESOURCES::IRIDIUM		RESOURCES::DERITIUM	FOOD		ENERGY
	//+-----------------------------------------------------------------------------------------------+
	/*M*/	0,			10,			0,			0,			0,			11,			50,			0,
	/*O*/	0,			50,			0,			0,			0,			0,			25,			0,
	/*L*/	0,			5,			0,			0,			0,			0,			10,			0,
	/*P*/	5,			0,			0,			5,			5,			15,			0,			50,
	/*H*/	0,			0,			0,			0,			50,			0,			0,			50,
	/*Q*/	0,			0,			0,			50,			0,			15,			5,			0,
	/*K*/	5,			0,			5,			0,			0,			15,			0,			0,
	/*G*/	10,			0,			10,			25,			0,			20,			0,			0,
	/*R*/	0,			0,			50,			0,			0,			15,			0,			0,
	/*F*/	25,			0,			25,			0,			0,			35,			0,			0,
	/*C*/	50,			0,			0,			0,			25,			60,			0,			0,
	/*N*/	0,			25,			0,			0,			0,			60,			0,			50,
	/*A*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*B*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*E*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*Y*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*I*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*J*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*S*/	0,			0,			0,			0,			0,			0,			0,			0,
	/*T*/	0,			0,			0,			0,			0,			0,			0,			0
	};
	// Zahl[0,99] generieren und vergleichen
	for (int i = RESOURCES::TITAN; i < 8; i++)
	{
		if (rand()%100 >= (100 - probs[this->m_iType][i]))
			m_bBoni[i] = TRUE;
		else
			m_bBoni[i] = FALSE;
	}
}

void CPlanet::SetBoni(BOOLEAN titan, BOOLEAN deuterium, BOOLEAN duranium, BOOLEAN crystal, BOOLEAN iridium, BOOLEAN deritium,
		BOOLEAN food, BOOLEAN energy)
{
	m_bBoni[RESOURCES::TITAN]		= titan;
	m_bBoni[RESOURCES::DEUTERIUM]	= deuterium;
	m_bBoni[RESOURCES::DURANIUM]	= duranium;
	m_bBoni[RESOURCES::CRYSTAL]	= crystal;
	m_bBoni[RESOURCES::IRIDIUM]	= iridium;
	m_bBoni[RESOURCES::DERITIUM]	= deritium;
	m_bBoni[6]			= food;
	m_bBoni[7]			= energy;
}

/// Funktion ermittelt die vorhandenen Ressourcen auf dem Planeten
/// @param res Feld in das geschrieben wird, welche Ressource auf dem Planeten vorhanden ist
void CPlanet::GetAvailableResources(BOOLEAN res[RESOURCES::DERITIUM + 1]) const
{
	if (this->GetClass() == 'C')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = TRUE;}
	else if (this->GetClass() == 'F')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = TRUE;  res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'G')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = TRUE;  res[RESOURCES::CRYSTAL] = TRUE;  res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'H')
	{	res[RESOURCES::TITAN] = FALSE; res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = TRUE;}
	else if (this->GetClass() == 'K')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = TRUE;  res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'L')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = TRUE;  res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'M')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = TRUE;  res[RESOURCES::DURANIUM] = TRUE;  res[RESOURCES::CRYSTAL] = TRUE;  res[RESOURCES::IRIDIUM] = TRUE;}
	else if (this->GetClass() == 'N')
	{	res[RESOURCES::TITAN] = FALSE; res[RESOURCES::DEUTERIUM] = TRUE;  res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'O')
	{	res[RESOURCES::TITAN] = FALSE; res[RESOURCES::DEUTERIUM] = TRUE;  res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'P')
	{	res[RESOURCES::TITAN] = TRUE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = TRUE;  res[RESOURCES::IRIDIUM] = TRUE;}
	else if (this->GetClass() == 'Q')
	{	res[RESOURCES::TITAN] = FALSE;  res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = FALSE; res[RESOURCES::CRYSTAL] = TRUE;  res[RESOURCES::IRIDIUM] = FALSE;}
	else if (this->GetClass() == 'R')
	{	res[RESOURCES::TITAN] = FALSE; res[RESOURCES::DEUTERIUM] = FALSE; res[RESOURCES::DURANIUM] = TRUE;  res[RESOURCES::CRYSTAL] = FALSE; res[RESOURCES::IRIDIUM] = FALSE;}

	// Deritium ist klassenunabh�ngig und wird als Boni angegeben
	if (this->GetBoni()[RESOURCES::DERITIUM])
		res[RESOURCES::DERITIUM] = TRUE;
}

/// Die Resetfunktion f�r die CPlanet Klasse, welche alle Werte wieder auf Ausgangswerte setzt.
void CPlanet::Reset(void)
{
	// Standardwerte initialisieren
	m_bIsTerraforming = FALSE;
	m_dCurrentHabitant = NULL;
	m_dMaxHabitant = NULL;
	m_iSize = PLANT_SIZE::NORMAL;
	m_iType = PLANET_CLASSES::PLANETCLASS_I;
	m_strName = "";
	m_dGrowing = 0;
	m_iGraphicType = 0;
	m_iNeededTerraformPoints = 0;
	m_iStartTerraformPoints = 0;
	m_bHasIndividualGraphic = false;
	for (int i = 0; i < 8; i++)
		m_bBoni[i] = FALSE;
}

bool CPlanet::IsColonizable() const
{
	return GetTerraformed() && !GetInhabited();
}

namespace
{
	// M,O,L,P,H,Q,K,G,R,F,C,N,A,B,E,N,Y,I,J,S,T
	const std::pair<PLANET_CLASSES::TYPE, char> values[] = {
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_M, 'M'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_O, 'O'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_L, 'L'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_P, 'P'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_H, 'H'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_Q, 'Q'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_K, 'K'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_G, 'G'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_R, 'R'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_F, 'F'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_C, 'C'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_N, 'N'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_A, 'A'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_B, 'B'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_E, 'E'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_Y, 'Y'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_I, 'I'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_J, 'J'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_S, 'S'),
		std::pair<PLANET_CLASSES::TYPE, char>(PLANET_CLASSES::PLANETCLASS_T, 'T')
	};
	const int num_elems = sizeof values / sizeof values[0];
}
const std::map<PLANET_CLASSES::TYPE, char> CPlanet::m_TypeToChar(values, values + num_elems);

char CPlanet::GetClass() const
{
	const std::map<PLANET_CLASSES::TYPE, char>::const_iterator it =
		m_TypeToChar.find(static_cast<PLANET_CLASSES::TYPE>(m_iType));
	AssertBotE(it != m_TypeToChar.end());
	return it->second;
}
