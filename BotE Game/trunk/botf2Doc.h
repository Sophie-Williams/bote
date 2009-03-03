// botf2Doc.h : Schnittstelle der Klasse CBotf2Doc
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "GenShipName.h"
#include "Diplomacy.h"
#include "Trade.h"
#include "TradeHistory.h"
#include "ShipHistory.h"
#include "Statistics.h"
#include "Fleet.h"
#include "WeaponObserver.h"
#include "GlobalBuildings.h"
#include "Combat.h"
#include "AttackSystem.h"
#include "Starmap.h"
#include "GlobalStorage.h"
#include "SoundManager.h"
#include "IniLoader.h"
#include "GraphicPool.h"
#include "Network.h"
#include "PeerData.h"
#include "LZMA_BotE.h"
#include "SystemAI.h"
#include "SectorAI.h"
#include "ShipAI.h"
#include "AIPrios.h"
#include "IntelCalc.h"
#include "MyTimer.h"
#include "Botf2.h"
#include "MainFrm.h"

class CMainDlg;
class CNetworkHandler;

class CBotf2Doc : public CDocument, public network::CPeerData
{
protected: // Nur aus Serialisierung erzeugen
	DECLARE_DYNCREATE(CBotf2Doc)

	// Attribute
	CSoundManager *m_pSoundManager;		///< der Soundmanager f�r BotE
	CIniLoader *m_pIniLoader;			///< Objekt verwaltet die ini Datei
	CGraphicPool *m_pGraphicPool;		///< Objekt verwaltet Grafiken f�r BotE
	USHORT m_iRound;					///< aktuelle Rundenanzahl
	float m_fDifficultyLevel;			///< der Schwierigkeitsgrad eines Spiels
	float m_fStardate;			// Startrek Sternzeit
	CPoint KO;					// Koordinaten des Sektors
	CPoint raceKO[7];			// Startkoordinaten des Hauptsystems der Majorraces
	
	BYTE m_iShowWhichTechInView3;// Welche Tech soll in View3 angezeigt werden?
	short m_iShowWhichShipInfoInView3; // Welche Schiffsinfo soll in View 3 angezeigt werden
	
	USHORT m_iSelectedView[7];	// Welche View soll in der MainView angezeigt werden? z.B. Galaxie oder System
	CStarmap* m_pStarmap;		// Zeiger auf die Starmap, welche dann auch in den Views abgefragt wird
	CStarmap* starmap[7];		// Die Starmaps der einzelnen Majors. Wird bei der NextRound Berechnung ben�tigt.
	CPoint m_ScrollPoint;		// aktuelle Scrollposition der Galaxieansicht wird auf diesen Point gespeichert
	CSector m_Sector[30][20];	// Matrix von Sektoren der Klasse CSector anlegen
	CSystem m_System[30][20];	// auf jeden Sektor ein potentielles System anlegen
	CEmpire m_Empire[7];		// Die einzelnen Imperien (F�d, Ferengi usw.) 7, weil 0 w�re NOBODOY
	CDiplomacy m_Diplomacy[7];	// Die einzelnen Diplomatieobjekte (F�d, Ferengi usw.) 7, weil 0 w�re NOBODOY
	CTrade m_Trade[7];			// Die einzelnen Handelsobjekte (F�d, Ferengi usw.) 7, weil 0 w�re NOBODY
	CTradeHistory m_TradeHistory[7];	// Die einzelnen Handelshistoryobjekte (beinhalten alle Kurse des Spiels)
	CMajorRace m_MajorRace[7];  // Die einzelnen Majorraceobjekte (F�d, Ferengi usw.) 7, weil 0 w�re NOBODY
	CWeaponObserver m_WeaponObserver[7];// beobachtet die baubaren Waffen f�r Schiffe. Wird ben�tigt wenn wir Schiffe designen wollen
	CGlobalStorage m_GlobalStorage[7];	// das globale Lager der einzelnen Imperien
	UINT m_iCombatShipPower[7];	// die st�rken der Kampfschiffe der einzelnen Imperien
	CGenShipName m_GenShipName;	// Variable, die alle m�glichen Schiffsnamen beinhaltet
	BYTE m_iPlayersRace;		// Varibale auf die die gew�hlte Majorrace des Spielers gespeichert wird
	CArray<CTroopInfo> m_TroopInfo;	// In diesem Feld werden alle Informationen zu den Truppen gespeichert
	ShipInfoArray m_ShipInfoArray;	// dynamisches Feld, in dem die ganzen Informationen zu den Schiffen gespeichert sind
	ShipArray m_ShipArray;		// dynamisches Feld, in das die ganzen Schiffe gespeichert werden
	CShipHistory m_ShipHistory[7];	// Objekt f�r jede einzelne Rasse, in der alle statistischen Daten ihrer Schiffe zu finden sind
	BuildingInfoArray BuildingInfo;	// alle Geb�udeinfos zu allen Geb�uden im Spiel
	CGlobalBuildings m_GlobalBuildings;	// alle gebauten Geb�ude aller Rassen im Spiel
	short m_iNumberOfFleetShip;		// Das Schiff welches sozusagen die Flotte anf�hrt
	short m_iNumberOfTheShipInFleet;// Nummber des Schiffes in der Flotte, wenn wir ein Flotte haben
	short m_NumberOfTheShipInArray; // Hilfsvariable, mit der auf ein spezielles Schiff im Array zugekriffen werden kann
	CMessage message;			// eine einzelne Nachricht
	MinorRaceArray m_MinorRaceArray;// dynamisches Feld, in welches die einzelnen Minorraces geschrieben werden
	CStatistics m_Statistics;	// Statisticsobjekt, in dem Statistiken des Spiels gespeichert sind
	CCombat combat;
	CAIPrios* m_pAIPrios;		// zus�tzliche Priotit�ten, welche f�r die SystemKI-Berechnung ben�tigt werden-

	CNetworkHandler *m_pNetworkHandler;
	BOOLEAN m_bDataReceived;	// hat der Client die Daten komplett vom Server erhalten
	BOOLEAN m_bRoundEndPressed;	// Wurde der Rundenendebutton gedr�ckt
	BOOLEAN m_bDontExit;		// hartes Exit verhindern, wenn Spiel beginnt
	BOOLEAN m_bGameLoaded;		// wurde im Dialog ein zu ladendes Spiel ausgew�hlt
	
	CArray<SNDMGR_MESSAGEENTRY> m_SoundMessages[7];	///< Die einzelnen Sprachmitteilungen zur neuen Runde

public:
	// Operationen
public:
	// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CBotf2Doc)
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	/// Serialisiert die Daten, welche am Anfang des Spiels einmal gesendet werden m�ssen.
	void SerializeBeginGameData(CArchive& ar);

	/// Serialisiert die Daten f�r die <code>CNextRound</code>-Nachricht.
	void SerializeNextRoundData(CArchive &ar);

	/// Serialisiert die Daten f�r die <code>CEndOfRound</code>-Nachricht �ber das angegebene Volk.
	/// <code>race</code> wird beim Schreiben ignoriert.
	void SerializeEndOfRoundData(CArchive &ar, network::RACE race);
	//}}AFX_VIRTUAL
	
	/**
	 *  L�dt die angegebene Datei, deserialisiert die in der Datei enthaltenen Daten ins Dokument.
	 *  Ruft unmittelbar vor dem Deserialisieren <code>Reset()</code> auf. <code>Reset()</code> wird
	 *  nicht aufgerufen, wenn zuvor ein Fehler aufgetreten ist.
	 *  Aufbau eines Savegames:
	 *  <pre>
	 *  BYTE[]		"BotE"			Magic Number
	 *  UINT		version			Versionsnummer des Spielstandes
	 * 	UINT		size			L�nge von data
	 *  BYTE[]		data			size Bytes, komprimierte serialisierte Daten des Dokuments
	 *  </pre>
	 */
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	/**
	 * Speichert die serialisierten Daten des Dokuments komprimiert in einer Datei. Das Format des
	 * Savegames ist bei <code>OnOpenDocument()</code> angegeben. Existierende Dateien werden �berschrieben.
	 */
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	// Implementierung
	CBotf2Doc();
	virtual ~CBotf2Doc();

	/// Funktion liefert einen Zeiger auf den GraphicPool f�r BotE.
	CGraphicPool* GetGraphicPool() {return m_pGraphicPool;}

	/// Funktion gibt einen Zeiger auf das Rahmenfenster (CMainFrame) zur�ck.
	/// @return Rahmenfenster MainFrame
	CMainFrame* GetMainFrame() const {return (CMainFrame*)AfxGetApp()->GetMainWnd();}
	
	/// Funktion gibt den Schwierigkeitsgrad des Spiels zur�ck.
	float GetDifficultyLevel() const {return m_fDifficultyLevel;}
	
	const CShip& GetShip(int number) const {return m_ShipArray.GetAt(number);}
	
	const CPoint& GetKO(){return KO;} 
	void SetKO(int m, int n);
	void SetKO(CPoint ko) { SetKO(ko.x, ko.y); }
	const CPoint& GetRaceKO(int Race) {return raceKO[Race];}
	
	CStarmap* GetStarmap() const {return m_pStarmap;}
	CSector& GetSector(int x, int y) {return m_Sector[x][y];}
	CSector& GetSector(CPoint ko) {return m_Sector[ko.x][ko.y];}
	CSystem& GetSystem(int x, int y) {return m_System[x][y];}	
	CSystem& GetSystem(CPoint ko) {return m_System[ko.x][ko.y];}
	
	/// Funktion liefert einen Zeiger auf ein gew�nschtes Imperium.
	/// @param race Nummer des Imperiums
	CEmpire* GetEmpire(BYTE race) {ASSERT(race); return &m_Empire[race];}

	/// Funktion liefert Zeiger auf eine gew�nschte Hauptrasse.
	/// @param race gew�nschte Rasse
	CMajorRace* GetMajorRace(BYTE race) {ASSERT(race); return &m_MajorRace[race];}

	/// Funktion liefert einen Zeiger auf das gloable Lager eines Imperiums.
	/// @param race gew�nschtes Imperium
	CGlobalStorage* GetGlobalStorage(BYTE race) {ASSERT(race); return &m_GlobalStorage[race];}
	
	CBuildingInfo& GetBuildingInfo(int id) {ASSERT(id); return BuildingInfo[id-1];}
	const CString& GetBuildingName(int id) const {ASSERT(id); return BuildingInfo[id-1].GetBuildingName();}
	const CString& GetBuildingDescription(int id) const {ASSERT(id); return BuildingInfo[id-1].GetBuildingDescription();}
	
	/// Funktion veranlasst die Views zu jeder neuen Runde ihr Aufgaben zu erledigen, welche zu jeder neuen Runde ausgef�hrt
	/// werden m�ssen. Es werden zum Beispiel Variablen wieder zur�ckgesetzt.
	void DoViewWorkOnNewRound();
	
	MinorRaceArray* GetMinorRaceArray() {return &m_MinorRaceArray;}	// Fkt. liefert das komplette MinorRaceArray
	CMinorRace* GetMinorRace(const CString& homePlanetName);	// Fkt. leifert die Minorrace, welche in dem System "Homeplanetname" lebt
	
	void PrepareData();			// generiert ein neues Spiel
	void NextRound();			// zur N�chsten Runde voranschreiten
	void ApplyShipsAtStartup();	// Die Schiffe zum Start anlegen, �bersichtshalber nicht alles in NewDocument
	void ApplyBuildingsAtStartup();	// Die Geb�ude zum Start in den Hauptsystemen anlegen
	void ReadBuildingInfosFromFile();	// Die Infos zu den Geb�uden aus den Datein einlesen
	void ReadShipInfosFromFile();		// Die Infos zu den Schiffen aus der Datei einlesen
	void BuildBuilding(USHORT id, CPoint KO); // Das jeweilige Geb�ude bauen
	void BuildShip(int ID, CPoint KO, BYTE owner);	// Das jeweilige Schiff im System KO bauen
	void GenerateStarmap(void);	// Funktion generiert die Starmaps, so wie sie nach Rundenberechnung auch angezeigt werden k�nnen.
	
	/// Die Truppe mit der ID <code>ID</code> wird im System mit der Koordinate <code>ko</code> gebaut.
	void BuildTroop(BYTE ID, CPoint ko);

	USHORT GetCurrentRound() const {return m_iRound;}
	BYTE GetPlayersRace() const;
	void SetPlayersRace(int PlayersRace) {m_iPlayersRace = PlayersRace;}
	
	USHORT GetNumberOfTheShipInArray() const {return m_NumberOfTheShipInArray;}
	USHORT GetNumberOfFleetShip() const {return m_iNumberOfFleetShip;}
	USHORT GetNumberOfTheShipInFleet() const {return m_iNumberOfTheShipInFleet;}
	void SetNumberOfTheShipInArray(int NumberOfTheShipInArray);
	void SetNumberOfFleetShip(int NumberOfFleetShip);
	void SetNumberOfTheShipInFleet(int NumberOfTheShipInFleet);

protected:
	// Private Funktionen die bei der NextRound Berechnung aufgerufen werden. Dadurch wird die NextRound Funktion
	// um einiges verkleinert
	
	/// Diese Funktion f�hrt allgemeine Berechnung durch, die immer zu Beginn der NextRound-Calculation stattfinden
	/// m�ssen. So werden z.B. alte Nachrichten gel�scht, die Statistiken berechnet usw..
	void CalcPreDataForNextRound();	
	
	/// Diese Funktion berechnet den kompletten Systemangriff.
	void CalcSystemAttack();
	
	/// Diese Funktion berechnet die Angebote von Majorraces an andere Majorraces.
	void CalcMajorOffers();
	
	/// Diese Funktion berechnet alles im Zusammenhang mit dem Geheimdienst.
	void CalcIntelligence();
	
	/// Diese Funktion berechnet die Forschung eines Imperiums
	void CalcResearch();
	
	/// Diese Funktion berechnet die Auswirkungen von diplomatischen Angeboten und ob Minorraces Angebote an
	/// Majorraces abgeben.
	void CalcDiplomacy();
	
	/// Diese Funktion berechnet das Planetenwachstum, die Auftr�ge in der Bauliste und sonstige Einstellungen aus der
	/// alten Runde.
	void CalcOldRoundData();
	
	/// Diese Funktion berechnet die Produktion der Systeme, was in den Baulisten gebaut werden soll und sonstige
	/// Daten f�r die neue Runde.
	void CalcNewRoundData();
	
	/// Diese Funktion berechnet die kompletten Handelsaktivit�ten.
	void CalcTrade();
	
	/// Diese Funktion berechnet die Schiffsbefehle. Der Systemangriffsbefehl ist davon ausgenommen.
	void CalcShipOrders();
	
	/// Diese Funktion berechnet die Schiffsbewegung und noch weitere kleine Sachen im Zusammenhang mit Schiffen.
	void CalcShipMovement();
	
	/// Diese Funktion berechnet einen m�glichen Weltraumkampf und dessen Auswirkungen.
	void CalcShipCombat();
	
	/// Diese Funktion berechnet die Auswirkungen von Schiffen und Stationen auf der Karte. So werden hier z.B. Sektoren
	/// gescannt, Rassen kennengelernt und die Schiffe den Sektoren bekanntgegeben.
	void CalcShipEffects();

	/// Diese Funktion berechnet die Schiffserfahrung in einer neuen Runde. Au�er Erfahrung im Kampf, diese werden nach einem
	/// Kampf direkt verteilt.
	/// @param ship Zeiger auf Schiff (inkl. Flotte) f�r welches die Erfahrung berechnet werden soll
	void CalcShipExp(CShip* ship);

	/// Funktion generiert die Galaxiemap inkl. der ganzen Systeme und Planeten zu Beginn eines neuen Spiels.
	void GenerateGalaxy();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generierte Message-Map-Funktionen
protected:
	//{{AFX_MSG(CBotf2Doc)
	// HINWEIS - An dieser Stelle werden Member-Funktionen vom Klassen-Assistenten eingef�gt und entfernt.
	//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VER�NDERN!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateFileNew(CCmdUI *pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////
