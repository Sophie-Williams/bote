/*
 *   Copyright (C)2004-2008 Sir Pustekuchen
 *
 *   Author   :  Sir Pustekuchen
 *   Home     :  http://birth-of-the-empires.de.vu
 *
 */
#pragma once

#include "Empire.h"
#include "MinorRace.h"
#include "MoralObserver.h"

typedef struct {USHORT latinum; USHORT ressource[5]; CString shownText; short type; CPoint KO; BYTE duration;} ComputerOfferStruct;

class CMajorRace : public CObject  
{
public:
	DECLARE_SERIAL (CMajorRace)					// Klasse serialisierbar machen
	/// Konstruktor
	CMajorRace(void);

	/// Destruktor
	virtual ~CMajorRace(void);
	
	/// Kopierkonstruktor
	CMajorRace(const CMajorRace & rhs);
	
	/// Zuweisungsoperator
	CMajorRace & operator=(const CMajorRace &);
	
	/// Serialisierungsfunktion
	virtual void Serialize(CArchive &ar);

	// Zugriffsfunktionen
	// zum Lesen der Membervariablen
	BYTE GetRaceNumber() const {return m_iRaceNumber;}
	BYTE GetKind() const {return m_iRaceProberty;}
	int GetAttributes() const {return m_Attributes;}

	/// Funktion gibt den Diplomatiebonus der Hauptrasse bei Verhandlungen zur�ck. <code>NULL</code> entspricht keinen
	/// Bonus oder Malus.
	short GetDiplomacyBonus() const {return m_iDiplomacyBonus;}
	
	/// Funktion gibt das Feld zur�ck, in dem steht ob wir die anderen Majorraces kennen.
//	BOOLEAN* GetKnownMajorRaces() {return m_bKnownMajorRaces;}
	BOOLEAN GetKnownMajorRace(BYTE race) const
	{
		switch (race)
		{
		case HUMAN:		return (m_Attributes & RACE_KNOWNHUMAN) == RACE_KNOWNHUMAN;
		case FERENGI:	return (m_Attributes & RACE_KNOWNFERENGI) == RACE_KNOWNFERENGI;
		case KLINGON:	return (m_Attributes & RACE_KNOWNKLINGON) == RACE_KNOWNKLINGON;
		case ROMULAN:	return (m_Attributes & RACE_KNOWNROMULAN) == RACE_KNOWNROMULAN;
		case CARDASSIAN:return (m_Attributes & RACE_KNOWNCARDASSIAN) == RACE_KNOWNCARDASSIAN;
		case DOMINION:	return (m_Attributes & RACE_KNOWNDOMINION) == RACE_KNOWNDOMINION;
		default:		return FALSE;
		}
	}
	
	/// Funktion gibt die Beziehung zu anderer Majorrace wieder. Als Paramter <code>race</code> wird die gew�nschte
	/// Rasse �bergeben.
	USHORT GetRelationshipToMajorRace(USHORT race) const {return m_iRelationshipOtherMajor[race];}
	
	/// Funktion gibt die diplomatische Beziehung zu anderer Majorrace wieder. Als Paramter <code>race</code> wird 
	/// die gew�nschte Rasse �bergeben.
	short GetDiplomacyStatus(BYTE race) const {return m_iDiplomacyStatus[race];}

	/// Funktion gibt die verbleibende Vertragsdauer eines Vertrages mit einer bestimmten Majorrace zur�ck.
	/// Als Paramter <code>withWhichRace</code> wird die gew�nschte Rasse �bergeben.
	USHORT GetDurationOfAgreement(USHORT withWhichRace) const {return m_iDurationOfAgreement[withWhichRace];}

	/// Funktion gibt einen Wahrheitswert zur�ck, ob man einen Verteidigungspakt mit einer anderer Majorrace hat.
	/// Als Paramter <code>withWhichRace</code> wird die gew�nschte Rasse �bergeben.
	BOOLEAN GetDefencePact(USHORT withWhichRace) const
	{
		switch (withWhichRace)
		{
		case HUMAN:		return (m_Attributes & RACE_DEFENCEPACTHUMAN) == RACE_DEFENCEPACTHUMAN;
		case FERENGI:	return (m_Attributes & RACE_DEFENCEPACTFERENGI) == RACE_DEFENCEPACTFERENGI;
		case KLINGON:	return (m_Attributes & RACE_DEFENCEPACTKLINGON) == RACE_DEFENCEPACTKLINGON;
		case ROMULAN:	return (m_Attributes & RACE_DEFENCEPACTROMULAN) == RACE_DEFENCEPACTROMULAN;
		case CARDASSIAN:return (m_Attributes & RACE_DEFENCEPACTCARDASSIAN) == RACE_DEFENCEPACTCARDASSIAN;
		case DOMINION:	return (m_Attributes & RACE_DEFENCEPACTDOMINION) == RACE_DEFENCEPACTDOMINION;
		default:		return FALSE;
		}		
	}
	
	/// Funktion gibt die verbleibende Vertragsdauer eines Verteidigungspaktes mit einer bestimmten Majorrace zur�ck.
	/// Als Paramter <code>withWhichRace</code> wird die gew�nschte Rasse �bergeben.
	USHORT GetDurationOfDefencePact(USHORT withWhichRace) const {return m_iDurationOfDefencePact[withWhichRace];}
	
	/// Funktion gibt einen Zeiger auf den <code>m_MoralObserver<code> zur�ck
	CMoralObserver* GetMoralObserver() {return &m_MoralObserver;}

	// zum Schreiben der Membervariablen
	/// Funktion setzt die Nummer der Rasse (HUMAN == 1 usw.) und ihre Haupteigenschaft z.B. WARLIKE bei Klingonen.
	void SetRaceNumber(BYTE RaceNumber);
		
	/// Funktion setzt die neue Beziehung zur Majorrace.
	void SetRelationshipToMajorRace(USHORT race, short newRelationAdd);
	
	/// Funktion setzt den neuen Status zur Majorrace, wenn wir aber Krieg erkl�ren, so wird der 
	/// m�glicherweise vorhandene Verteidigungspakt aufgehoben.
	void SetDiplomaticStatus(USHORT race, short newDiplomacyStatus);

	/// Funktion legt fest, ob die Majorrace eine andere Majorrace kennt.
	/// @param race andere Rasse
	/// @param known <code>TRUE</code> wenn sie sie kennenlernt, ansonsten <code>FALSE</code>
	void SetKnownMajorRace(USHORT race, BOOLEAN is = TRUE)
	{
		ASSERT(race >= HUMAN && race <= DOMINION);
		switch (race)
		{
		case HUMAN:		SetAttributes(is, RACE_KNOWNHUMAN, m_Attributes);		break;
		case FERENGI:	SetAttributes(is, RACE_KNOWNFERENGI, m_Attributes);		break;
		case KLINGON:	SetAttributes(is, RACE_KNOWNKLINGON, m_Attributes);		break;
		case ROMULAN:	SetAttributes(is, RACE_KNOWNROMULAN, m_Attributes);		break;
		case CARDASSIAN:SetAttributes(is, RACE_KNOWNCARDASSIAN, m_Attributes);	break;
		case DOMINION:	SetAttributes(is, RACE_KNOWNDOMINION, m_Attributes);	break;
		}	
	}
	void SetDurationOfAgreement(USHORT withWhichRace, USHORT duration) {m_iDurationOfAgreement[withWhichRace] = duration;} // Setzt anf�ngliche Vertragsdauer mit andere Majorrace
	void SetDefencePact(USHORT withWhichRace, BOOLEAN is)
	{
		ASSERT(withWhichRace >= HUMAN && withWhichRace <= DOMINION);
		switch (withWhichRace)
		{
		case HUMAN:		SetAttributes(is, RACE_DEFENCEPACTHUMAN, m_Attributes);			break;
		case FERENGI:	SetAttributes(is, RACE_DEFENCEPACTFERENGI, m_Attributes);		break;
		case KLINGON:	SetAttributes(is, RACE_DEFENCEPACTKLINGON, m_Attributes);		break;
		case ROMULAN:	SetAttributes(is, RACE_DEFENCEPACTROMULAN, m_Attributes);		break;
		case CARDASSIAN:SetAttributes(is, RACE_DEFENCEPACTCARDASSIAN, m_Attributes);	break;
		case DOMINION:	SetAttributes(is, RACE_DEFENCEPACTDOMINION, m_Attributes);		break;
		}		
	}
	void SetDurationOfDefencePact(USHORT withWhichRace, USHORT duration) {m_iDurationOfDefencePact[withWhichRace] = duration;} // Setzt anf�ngliche Vertragsdauer mit andere Majorrace
// sonstige Funktionen
	/// Funktion gibt den Namen des Heimatsystems der Rasse zur�ck.
	CString GetHomeSystemName();

	/// Funktion gibt mir die Beschreibung der Hauptrasse zur�ck.
	CString GetRaceDescription();
	
	/// Diese Funktion liefert den Namen des Grafikfiles f�r die Majorrace.
	CString GetGraphicFileName();
	
	/// Funktion liefert den gleichen Namen wie <code>GetGraphicFileName()</code>. Nur ist diese hier eine 
	/// statische Funktion.
	static CString GetRaceName(USHORT NumberOfRace);
	
	/// Funktion gibt die Anzahl dem Imperium bekannter MajorRaces zur�ck. Unser Imperium selbst wird nicht mitgez�hlt.
	USHORT GetNumberOfKnownMajorRaces(USHORT playersRace);
	
	/// Funktion verringert die Anzahl der noch verbleibenden Runden der laufenden Vertr�ge um eins.
	/// Au�er der Vertrag l�uft auf unbestimmte Zeit.
	BOOLEAN DekrementDurationOfAllAgreements(CEmpire* EmpireOfMajor);
	
	/// Diese Funktion berechnet, wie eine computergesteuerte Majorrace, auf ein Angebot reagiert
	/// Das ist hier sozusagen die komplette KI der Reaktion auf irgendwelche Angebote.
	/// R�ckgabewert ist einfach <code>TRUE</code> f�r Annahme bzw. <code>FALSE</code> bei Ablehnung.
	USHORT CalculateDiplomaticOffer(USHORT offerFrom, short type, USHORT warpactEnemy, const UINT* shipPowers);
	
	/// Diese Funktion berechnet wie eine computergesteuerte Majorrace auf eine Forderung reagiert.
	/// R�ckgabewert ist einfach <code>TRUE</code> f�r Annahme bzw. <code>FALSE</code> bei Ablehnung.
	USHORT CalculateDiplomaticRequest(USHORT requestFrom, USHORT requestedLatinum, USHORT* requestedRessource, CEmpire* empire, ULONG* averageRessourceStorages, CSystem systems[30][20], const UINT* shipPowers);
	
	///	Funktion wird f�r jede computergesteuerte Majorrace in der <code>NextRound()</code>Funktion der Doc-Klasse aufgerufen.
	///	Diese Funktion berechnet, ob die MajorRace einer anderen MajorRace ein Angebot aufgrund der 
	/// Beziehung macht, oder ob sie ihr sogar den Krieg erkl�rt. Diese Funktion wird f�r jede einzelne andere Majorrace
	///	aufgerufen, weil ich ja in einer Runde an mehrere Majorraces Angebote machen kann. Mit <code>race</code> wird immer
	///	die andere Majorrace �bergeben, der vielleicht an Angebot gemacht wird.
	ComputerOfferStruct PerhapsMakeOfferToMajorRace(USHORT race, USHORT relation, CEmpire* empire, 
		CSystem systems[30][20], USHORT averageTechlevel, ULONG* averageRessourceStorages, const UINT* shipPowers);
	
	/// Funktion berechnet die Beziehungsverbesserungen durch die �bergabe von Latinum. Das Latinum wird hier aber nicht
	/// gutgeschrieben, sondern "nur" die Beziehung zur Majorrace verbessert. <code>race</code> ist die Rasse, die das
	/// Geschenk �bergegeben hat.
	void GiveLatinumPresent(USHORT latinum, CEmpire* empires, USHORT race);	// Funktion berechnet die Beziehungsverbesserungen durch �bergabe von Latinum
	
	/// Funktion berechnet wie viel die Ressource in Latinum wert ist.
	USHORT GiveResourcePresent(USHORT* ressource, CEmpire* empires, USHORT race); // Funktion berechnet wie viel die Ressource in Latinum wert ist
	
	/// Funktion berechnet, ob und was f�r ein Angebot die Majorrace an die Minorrace macht.
	ComputerOfferStruct PerhapsMakeOfferToMinorRace(CMinorRace* minor, BOOLEAN isFavoritMinor = FALSE, CEmpire* empire = 0,
		CSystem systems[30][20] = 0, USHORT averageTechlevel = 0, ULONG* averageRessourceStorages = 0); // Funktion berechnet, ob und was f�r ein Angebot die Majorrace an die Minorrace macht
	
	/// Funktion berechnet die Minorrace, der die Majorrace am liebsten Mitgifte geben w�rde. Also nicht die Minorrace,
	/// die wir am besten leiden k�nnen, sondern die, denen wir ein Geschenk bzw. Mitgifte machen w�rden.
	short CalculateFavoritMinor(MinorRaceArray* minors);	// Funktion berechnet die Minorrace, der die Majorrace am liebsten Mitgifte geben w�rde
	
	/// Die Resetfunktion der CMajorrace-Klasse.
	void Reset();

private:
	// private Funktionen
	/// Funktion die die Daten aus der Datei 'MajorRace.data' ausliest und diese in die
	/// Variable read schreibt.
	void ReadDataFromFile(CString* read);
	
	/// Funktion berechnet den n�tigen Wert, ob wir ein Angebot machen. Der Wert ist abh�ngig von der Rasseneigenschaft.
	USHORT CalculateValueByProberty(short type);
	
	/// Funktion berechnet wieviel und welche Mitgifte die KI bei einem Angebot mitgbt.
	void PerhapsGiveDowry(ComputerOfferStruct* theOffer, USHORT race, CEmpire* empire, CSystem systems[30][20], USHORT averageTechlevel, ULONG* averageRessourceStorages, BOOLEAN flag = FALSE);
	
	/// Funktion berechnet ob und was f�r eine Forderung wir stellen.
	void PerhapsHaveRequest(ComputerOfferStruct* theOffer, USHORT race, CEmpire* empire, CSystem systems[30][20], USHORT averageTechlevel, ULONG* averageRessourceStorages, const UINT* shipPowers);
	
	// Attribute
	BYTE m_iRaceNumber;					///< Welche Nummber hat die Rasse z.B. HUMAN == 1 usw.
	
	BYTE m_iRaceProberty;				///< Haupteigenschaft der Majorrace z.B. Klingonen sind WARLIKE, Wert wird in SetRaceNumber() mit gesetzt

	short m_iDiplomacyBonus;			///< Bonus bei diplomatischen Verhandlungen, NULL == kein Bonus/kein Malus
	
	USHORT m_iRelationshipOtherMajor[7];///< Beziehung zu den anderen Rassen

	int m_Attributes;					///< diese Variable beinhaltet alle booleschen Rassenattribute
	
	short m_iDiplomacyStatus[7];		///< diplomatischer Status (Vereinbarung) zu anderen Hauptrassen
	
	USHORT m_iDurationOfAgreement[7];	///< noch verbleibende Runden des Vertrags mit jeweiliger Majorrace (NULL == unbegrenzt)
	
	USHORT m_iDurationOfDefencePact[7];	///< Dauer des Verteidigungspaktes, einzeln speichern, weil wir ihn sepertat zu den anderen Vertr�gen haben k�nnen

	/// In den MoralObserver werden alle Events geschrieben, die die Moral der Majorrace irgendwie beeinflussen
	CMoralObserver m_MoralObserver;
};