// botf2Doc.cpp : Implementierung der Klasse CBotf2Doc
//

#include "stdafx.h"
#include "botf2.h"
#include "botf2Doc.h"
#include "LZMA_BotE.h"
#include "GalaxyMenuView.h"
#include "SmallInfoView.h"
#include "MenuChooseView.h"
#include "MainBaseView.h"
#include "PlanetBottomView.h"
#include "NetworkHandler.h"
#include "MainFrm.h"
#include "IniLoader.h"
#include "ImageStone/ImageStone.h"

#include "Races\RaceController.h"
#include "Races\DiplomacyController.h"
#include "Ships\Combat.h"
#include "Ships\Fleet.h"
#include "System\AttackSystem.h"
#include "Intel\IntelCalc.h"
#include "ShipSanity.h"

#include "AI\AIPrios.h"
#include "AI\SectorAI.h"
#include "AI\SystemAI.h"
#include "AI\ShipAI.h"
#include "AI\CombatAI.h"
#include "AI\ResearchAI.h"

#include "Galaxy\Anomaly.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBotf2Doc

IMPLEMENT_DYNCREATE(CBotf2Doc, CDocument)

BEGIN_MESSAGE_MAP(CBotf2Doc, CDocument)
	//{{AFX_MSG_MAP(CBotf2Doc)
		// HINWEIS - Hier werden Mapping-Makros vom Klassen-Assistenten eingef�gt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VER�NDERN!
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CBotf2Doc::OnUpdateFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CBotf2Doc::OnUpdateFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBotf2Doc Konstruktion/Destruktion

#pragma warning(push)
#pragma warning (disable:4351)
CBotf2Doc::CBotf2Doc() :
	m_ptKO(0, 0),
	m_ptCurrentCombatSector(-1, -1),
	m_NumberOfTheShipInArray(-1),
	m_bCombatCalc(false),
	m_bDataReceived(false),
	m_bDontExit(false),
	m_bGameLoaded(false),
	m_bGameOver(false),
	m_bNewGame(true),
	m_bRoundEndPressed(false),
	m_fDifficultyLevel(1.0f),
	m_fStardate(121000.0f),
	m_iNumberOfFleetShip(-1),
	m_iNumberOfTheShipInFleet(-1),
	m_iRound(1),
	m_iSelectedView(),
	m_iShowWhichShipInfoInView3(0),
	m_iShowWhichTechInView3(0)
{
	//Init MT with single log file
	CString sLogPath = CIOData::GetInstance()->GetLogPath();
	const CCommandLineParameters* const clp = dynamic_cast<CBotf2App*>(AfxGetApp())->GetCommandLineParameters();
	const std::set<const std::string>& domains = clp->LogDomains();
	MT::CMyTrace::Init(sLogPath,
		domains.empty() ? std::set<const std::string>(MT::DEFAULT_LOG_DOMAINS,
			MT::DEFAULT_LOG_DOMAINS + sizeof(MT::DEFAULT_LOG_DOMAINS) / sizeof(*MT::DEFAULT_LOG_DOMAINS))
			: domains, clp->ActiveDomains());
	MT::CMyTrace::SetLevel(clp->LogLevel());

	// ZU ERLEDIGEN: Hier Code f�r One-Time-Konstruktion einf�gen
	CResourceManager::Init();

	m_pGraphicPool = new CGraphicPool(CIOData::GetInstance()->GetAppPath() + "Graphics\\");

	m_pRaceCtrl = new CRaceController();

	m_pAIPrios = new CAIPrios(this);
	m_pSectorAI= new CSectorAI(this);

	m_pNetworkHandler = new CNetworkHandler(this);
	server.AddServerListener(m_pNetworkHandler);
	client.AddClientListener(m_pNetworkHandler);

	AllocateSectorsAndSystems();
}
#pragma warning(pop)

CBotf2Doc::~CBotf2Doc()
{
	if (m_pGraphicPool)
		delete m_pGraphicPool;
	if (m_pRaceCtrl)
		delete m_pRaceCtrl;
	if (m_pAIPrios)
		delete m_pAIPrios;
	if (m_pSectorAI)
		delete m_pSectorAI;

	m_pGraphicPool	= NULL;
	m_pRaceCtrl		= NULL;
	m_pAIPrios		= NULL;
	m_pSectorAI		= NULL;

	m_ShipArray.RemoveAll();
	m_ShipInfoArray.RemoveAll();

	if (m_pNetworkHandler)
	{
		server.RemoveServerListener(m_pNetworkHandler);
		client.RemoveClientListener(m_pNetworkHandler);
		delete m_pNetworkHandler;
		m_pNetworkHandler = NULL;
	}

	for(std::vector<CSector>::iterator sector = m_Sectors.begin();
		sector != m_Sectors.end(); ++sector) {
		sector->Reset();
	}
	for(std::vector<CSystem>::iterator system = m_Systems.begin();
		system != m_Systems.end(); ++system) {
		system->ResetSystem();;
	}

	// statische Variablen der Starmap freigeben
	CStarmap::DeleteStatics();

	// stop MT
	MYTRACE_DEINIT;
}

BOOL CBotf2Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// ZU ERLEDIGEN: Hier Code zur Reinitialisierung einf�gen
	m_bDataReceived				= false;
	m_bDontExit					= false;
	m_bGameLoaded				= false;
	m_fStardate					= 121000.0f;
	m_bCombatCalc				= false;
	m_bNewGame					= true;

	CIniLoader* pIni = CIniLoader::GetInstance();
	ASSERT(pIni);

	bool bHardwareSound;
	pIni->ReadValue("Audio", "HARDWARESOUND", bHardwareSound);
	CSoundManager::GetInstance()->Init(!bHardwareSound);

	int nSeed = -1;
	pIni->ReadValue("Special", "RANDOMSEED", nSeed);

	//Kartengr��e aus ini lesen
	//int mapHeight=20;
	//pIni->ReadValue("Special", "MAPSIZEV", mapHeight);
	//STARMAP_SECTORS_VCOUNT=mapHeight;

	//int mapWidth=30;
	//pIni->ReadValue("Special", "MAPSIZEH", mapWidth);
	//STARMAP_SECTORS_HCOUNT=mapWidth;

	STARMAP_TOTALWIDTH=STARMAP_SECTORS_HCOUNT*80;
	STARMAP_TOTALHEIGHT=STARMAP_SECTORS_VCOUNT*80;


	// festen vorgegeben Seed verwenden
	if (nSeed >= 0)
		srand(nSeed);
	// zuf�lligen Seed verwenden
	else
	{
		nSeed = (unsigned)time(NULL);
		srand(nSeed);
	}
	MYTRACE("general")(MT::LEVEL_INFO, "Used seed for randomgenerator: %i", nSeed);

	// Standardwerte setzen
	m_ptKO = CPoint(0,0);
	m_bRoundEndPressed			= false;
	m_iShowWhichTechInView3		= 0;
	m_iShowWhichShipInfoInView3 = 0;
	m_NumberOfTheShipInArray	= -1;
	m_iNumberOfFleetShip		= -1;
	m_iNumberOfTheShipInFleet	= -1;
	for (int i = network::RACE_1; i < network::RACE_ALL; i++)
		m_iSelectedView[i] = START_VIEW;

	return TRUE;
}

/// Funktion gibt die Rassen-ID der lokalen Spielerrasse zur�ck.
/// @return Zeiger auf Majorrace-Rassenobjekt
CString CBotf2Doc::GetPlayersRaceID(void) const
{
	return m_pRaceCtrl->GetMappedRaceID((network::RACE)client.GetClientRace());
}

/// Funktion gibt die Rassen-ID der lokalen Spielerrasse zur�ck.
/// @return Zeiger auf Majorrace-Rassenobjekt
CMajor* CBotf2Doc::GetPlayersRace(void) const
{
	// zuerst muss eine Netzwerknummer, also RACE1 bis RACE6 (1-6)
	// auf eine bestimmte Rassen-ID gemappt werden. Dies ist dann
	// die Rassen-ID.
	CString s = m_pRaceCtrl->GetMappedRaceID((network::RACE)client.GetClientRace());
	CMajor* pPlayersRace = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(s));

	return pPlayersRace;
}

/////////////////////////////////////////////////////////////////////////////
// CBotf2Doc Serialisierung

void CBotf2Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// ZU ERLEDIGEN: Hier Code zum Speichern einf�gen
		ar << m_iRound;
		ar << m_fStardate;
		//not here...MYTRACE("general")(MT::LEVEL_INFO, "Stardate: %f", m_fStardate);
		ar << m_ptKO;
		ar << STARMAP_SECTORS_HCOUNT;
		ar << STARMAP_SECTORS_VCOUNT;
		// Zeitstempel
		MYTRACE("logging")(MT::LEVEL_INFO, _T("Time: %s"), CTime(time(NULL)).Format("%c"));


		// Hauptrassen-Koordinaten speichern
		ar << m_mRaceKO.size();
		for (map<CString, pair<int, int> >::const_iterator it = m_mRaceKO.begin(); it != m_mRaceKO.end(); ++it)
			ar << it->first << it->second.first << it->second.second;

		for (int i = network::RACE_1; i < network::RACE_ALL; i++)
		{
			ar << m_iSelectedView[i];
		}

		ar << m_ShipInfoArray.GetSize();
		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
			m_ShipInfoArray.GetAt(i).Serialize(ar);
		ar << m_ShipArray.GetSize();
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
			m_ShipArray.GetAt(i).Serialize(ar);

		ar<< m_TroopInfo.GetSize();//Truppen in Savegame speichern
		for (int i = 0; i < m_TroopInfo.GetSize(); i++)
			m_TroopInfo.GetAt(i).Serialize(ar);

		// statische Variablen serialisieren
		for (int j = TITAN; j <= IRIDIUM; j++)
			ar << CTrade::GetMonopolOwner(j);

		map<CString, short>* mMoralMap = CSystemProd::GetMoralProdEmpireWide();
		ar << mMoralMap->size();
		for (map<CString, short>::const_iterator it = mMoralMap->begin(); it != mMoralMap->end(); ++it)
			ar << it->first << it->second;
	}
	else
	{
		int number;
		// ZU ERLEDIGEN: Hier Code zum Laden einf�gen
		ar >> m_iRound;
		ar >> m_fStardate;
		ar >> m_ptKO;
		ar >> STARMAP_SECTORS_HCOUNT;
		ar >> STARMAP_SECTORS_VCOUNT;
		//AllocateSectorsAndSystems();

		// Hauptrassen-Koordinaten laden
		m_mRaceKO.clear();
		size_t mapSize = 0;
		ar >> mapSize;
		for (size_t i = 0; i < mapSize; i++)
		{
			CString key;
			pair<int, int> value;
			ar >> key;
			ar >> value.first;
			ar >> value.second;
			m_mRaceKO[key] = value;
		}

		for (int i = network::RACE_1; i < network::RACE_ALL; i++)
		{
			ar >> m_iSelectedView[i];
		}

		ar >> number;
		m_ShipInfoArray.RemoveAll();
		m_ShipInfoArray.SetSize(number);
		for (int i = 0; i < number; i++)
			m_ShipInfoArray.GetAt(i).Serialize(ar);
		ar >> number;
		m_ShipArray.RemoveAll();
		m_ShipArray.SetSize(number);
		for (int i = 0; i < number; i++)
			m_ShipArray.GetAt(i).Serialize(ar);
		ar >> number;
		m_TroopInfo.RemoveAll();
		m_TroopInfo.SetSize(number);
		for (int i = 0; i<number; i++)
			m_TroopInfo.GetAt(i).Serialize(ar);

		// Geb�udeinfos werden nun beim Laden neu eingelesen
		BuildingInfo.RemoveAll();
		this->ReadBuildingInfosFromFile();

		// statische Variablen serialisieren
		for (int j = TITAN; j <= IRIDIUM; j++)
		{
			CString sOwnerID;
			ar >> sOwnerID;
			CTrade::SetMonopolOwner(j, sOwnerID);
		}
		CSystemProd::GetMoralProdEmpireWide()->clear();
		ar >> mapSize;
		for (size_t i = 0; i < mapSize; i++)
		{
			CString key;
			short value;
			ar >> key;
			ar >> value;
			(*CSystemProd::GetMoralProdEmpireWide())[key] = value;
		}
	}

	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			if (ar.IsLoading())
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).ResetSystem();
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
			{
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != "" || m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() != "" || m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMinorRace() == TRUE)
					m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
			}
		}

	CMoralObserver::SerializeStatics(ar);

	m_GenShipName.Serialize(ar);
	m_GlobalBuildings.Serialize(ar);
	m_Statistics.Serialize(ar);

	m_pRaceCtrl->Serialize(ar);

	if (ar.IsLoading())
	{
		// Spieler den Majors zuweisen
		map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
			it->second->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
	}

	m_VictoryObserver.Serialize(ar);
}

/// Serialisiert die Daten, welche am Anfang des Spiels einmal gesendet werden m�ssen.
void CBotf2Doc::SerializeBeginGameData(CArchive& ar)
{
	m_bDataReceived = false;
	// senden auf Serverseite
	if (ar.IsStoring())
	{
		//Kartengr��e
		ar<<STARMAP_SECTORS_HCOUNT;
		ar<<STARMAP_SECTORS_VCOUNT;
		// Hauptrassen-Koordinaten senden
		ar << m_mRaceKO.size();
		for (map<CString, pair<int, int> >::const_iterator it = m_mRaceKO.begin(); it != m_mRaceKO.end(); ++it)
			ar << it->first << it->second.first << it->second.second;

		ar << BuildingInfo.GetSize();
		for (int i = 0; i < BuildingInfo.GetSize(); i++)
			BuildingInfo.GetAt(i).Serialize(ar);
		ar << m_TroopInfo.GetSize();
		for (int i = 0; i < m_TroopInfo.GetSize(); i++)
			m_TroopInfo.GetAt(i).Serialize(ar);
	}
	// Empfangen auf Clientseite
	else
	{
		//Kartengr��e und initialisierung
		ar>>STARMAP_SECTORS_HCOUNT;
		ar>>STARMAP_SECTORS_VCOUNT;
		STARMAP_TOTALWIDTH=STARMAP_SECTORS_HCOUNT*80;
		STARMAP_TOTALHEIGHT=STARMAP_SECTORS_VCOUNT*80;
		//AllocateSectorsAndSystems();
		// Hauptrassen-Koordinaten empfangen
		m_mRaceKO.clear();
		size_t mapSize = 0;
		ar >> mapSize;
		for (size_t i = 0; i < mapSize; i++)
		{
			CString key;
			pair<int, int> value;
			ar >> key;
			ar >> value.first;
			ar >> value.second;
			m_mRaceKO[key] = value;
		}

		int number;
		ar >> number;
		BuildingInfo.RemoveAll();
		BuildingInfo.SetSize(number);
		for (int i = 0; i < number; i++)
			BuildingInfo.GetAt(i).Serialize(ar);

		ar >> number;
		m_TroopInfo.RemoveAll();
		m_TroopInfo.SetSize(number);
		for (int i = 0; i < number; i++)
			m_TroopInfo.GetAt(i).Serialize(ar);
	}

	CMoralObserver::SerializeStatics(ar);
}

void CBotf2Doc::SerializeNextRoundData(CArchive &ar)
{
	m_bDataReceived = false;
	// TODO Daten der n�chsten Runde serialisieren; auf Server-Seite senden, auf Client-Seite empfangen
	if (ar.IsStoring())
	{
		ar << m_bCombatCalc;
		// Wenn es einen Kampf gab, dann Schiffe �bertragen
		if (m_bCombatCalc)
		{
			MYTRACE("general")(MT::LEVEL_INFO, "Server is sending CombatData to client...\n");
			// Sektor des Kampfes �bertragen
			ar << m_ptCurrentCombatSector;
			int nCount = 0;
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
				if (m_ShipArray[i].GetKO() == m_ptCurrentCombatSector)
					nCount++;
			ar << nCount;
			// nur Schiffe aus diesem Sektor senden
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
				if (m_ShipArray[i].GetKO() == m_ptCurrentCombatSector)
					m_ShipArray.GetAt(i).Serialize(ar);
			return;
		}

		MYTRACE("general")(MT::LEVEL_INFO, "Server is sending NextRoundData to client...\n");
		// Server-Dokument
		// ZU ERLEDIGEN: Hier Code zum Speichern einf�gen
		ar << m_iRound;
		ar << m_fStardate;
		for (int i = network::RACE_1; i < network::RACE_ALL; i++)
			ar << m_iSelectedView[i];
		ar << m_ShipInfoArray.GetSize();
		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
			m_ShipInfoArray.GetAt(i).Serialize(ar);
		ar << m_ShipArray.GetSize();
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
			m_ShipArray.GetAt(i).Serialize(ar);

		// statische Variablen serialisieren
		for (int j = TITAN; j <= IRIDIUM; j++)
			ar << CTrade::GetMonopolOwner(j);

		map<CString, short>* mMoralMap = CSystemProd::GetMoralProdEmpireWide();
		ar << mMoralMap->size();
		for (map<CString, short>::const_iterator it = mMoralMap->begin(); it != mMoralMap->end(); ++it)
			ar << it->first << it->second;
	}
	else
	{
		ar >> m_bCombatCalc;
		if (m_bCombatCalc)
		{
			MYTRACE("general")(MT::LEVEL_INFO, "Client is receiving CombatData from server...\n");
			ar >> m_ptCurrentCombatSector;
			// Es werden nur Schiffe aus dem aktuellen Kampfsektor empfangen
			int nCount;
			ar >> nCount;
			// alle Schiffe aus dem Kampfsektor entfernen
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
				if (m_ShipArray.GetAt(i).GetKO() == m_ptCurrentCombatSector)
					m_ShipArray.RemoveAt(i--);
			int nSize = m_ShipArray.GetSize();
			// empfangene Schiffe wieder hinzuf�gen
			m_ShipArray.SetSize(nSize + nCount);
			for (int i = nSize; i < m_ShipArray.GetSize(); i++)
				m_ShipArray.GetAt(i).Serialize(ar);

			return;
		}

		// Client-Dokument
		MYTRACE("general")(MT::LEVEL_INFO, "Client is receiving NextRoundData from server...\n");
		int number;
		// ZU ERLEDIGEN: Hier Code zum Laden einf�gen
		ar >> m_iRound;
		ar >> m_fStardate;
		for (int i = network::RACE_1; i < network::RACE_ALL; i++)
			ar >> m_iSelectedView[i];
		ar >> number;
		m_ShipInfoArray.RemoveAll();
		m_ShipInfoArray.SetSize(number);
		for (int i = 0; i < number; i++)
			m_ShipInfoArray.GetAt(i).Serialize(ar);
		ar >> number;
		m_ShipArray.RemoveAll();
		m_ShipArray.SetSize(number);
		for (int i = 0; i < number; i++)
			m_ShipArray.GetAt(i).Serialize(ar);
		// statische Variablen serialisieren
		for (int j = TITAN; j <= IRIDIUM; j++)
		{
			CString sOwnerID;
			ar >> sOwnerID;
			CTrade::SetMonopolOwner(j, sOwnerID);
		}
		CSystemProd::GetMoralProdEmpireWide()->clear();
		size_t mapSize = 0;
		ar >> mapSize;
		for (size_t i = 0; i < mapSize; i++)
		{
			CString key;
			short value;
			ar >> key;
			ar >> value;
			(*CSystemProd::GetMoralProdEmpireWide())[key] = value;
		}
	}

	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			if (ar.IsLoading())
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).ResetSystem();
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
			{
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != "" || m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() != "" || m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMinorRace() == TRUE)
					m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
			}
		}

	m_pRaceCtrl->Serialize(ar);

	for (int i = HUMAN; i <= DOMINION; i++)
		m_SoundMessages[i].Serialize(ar);


	m_GenShipName.Serialize(ar);
	m_GlobalBuildings.Serialize(ar);
	m_Statistics.Serialize(ar);
	m_VictoryObserver.Serialize(ar);

	if (ar.IsLoading())
	{
		CSmallInfoView::SetPlanet(NULL);
		//GenerateStarmap();

		m_bGameOver = false;
		CMajor* pPlayer = GetPlayersRace();
		// bekommt der Client hier keine Spielerrasse zur�ck, so ist er ausgeschieden
		ASSERT(pPlayer);
		if (pPlayer == NULL)
		{
			AfxMessageBox("Fatal Error ... exit game now");
			m_bGameOver = true;
			return;
		}

		network::RACE client = m_pRaceCtrl->GetMappedClientID(pPlayer->GetRaceID());

		// Ausgehend vom Pfad des Schiffes den Sektoren mitteilen, das durch sie ein Schiff fliegt
		for (int y = 0; y < m_ShipArray.GetSize(); y++)
			if (m_ShipArray.GetAt(y).GetOwnerOfShip() == pPlayer->GetRaceID())
				for (int i = 0; i < m_ShipArray[y].GetPath()->GetSize(); i++)
					m_Sectors.at(m_ShipArray[y].GetPath()->GetAt(i).x+(m_ShipArray[y].GetPath()->GetAt(i).y)*STARMAP_SECTORS_HCOUNT).AddShipPathPoints(1);
		// Sprachmeldungen an den Soundmanager schicken
		CSoundManager* pSoundManager = CSoundManager::GetInstance();
		ASSERT(pSoundManager);
		pSoundManager->ClearMessages();
		for (int i = 0; i < m_SoundMessages[client].GetSize(); i++)
		{
			SNDMGR_MESSAGEENTRY* entry = &m_SoundMessages[client].GetAt(i);
			pSoundManager->AddMessage(entry->nMessage, entry->nRace, entry->nPriority, entry->fVolume);
		}

		// Systemliste der Imperien erstellen
		map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
			it->second->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
	}
	MYTRACE("general")(MT::LEVEL_INFO, "... serialization of NextRoundData succesfull\n");
}

void CBotf2Doc::SerializeEndOfRoundData(CArchive &ar, network::RACE race)
{
	if (ar.IsStoring())
	{
		if (m_bCombatCalc)
		{
			MYTRACE("general")(MT::LEVEL_INFO, "Client %d sending CombatData to server...\n", race);

			// nur Informationen �ber die Taktik der Schiffe bzw. die Taktik des Kampfes senden
			ar << m_nCombatOrder;
			return;
		}

		MYTRACE("general")(MT::LEVEL_INFO, "Client %d sending EndOfRoundData to server...\n", race);
		CMajor* pPlayer = GetPlayersRace();
		// Client-Dokument
		// Anzahl der eigenen Schiffsinfoobjekte ermitteln
		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
			if (m_ShipInfoArray.GetAt(i).GetRace() == pPlayer->GetRaceShipNumber())
				m_ShipInfoArray.GetAt(i).Serialize(ar);

		int number = 0;
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
			if (m_ShipArray.GetAt(i).GetOwnerOfShip() == pPlayer->GetRaceID())
				number++;
		ar << number;
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
			if (m_ShipArray.GetAt(i).GetOwnerOfShip() == pPlayer->GetRaceID())
				m_ShipArray.GetAt(i).Serialize(ar);

		number = 0;
		for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			{
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
				{
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == pPlayer->GetRaceID() && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == pPlayer->GetRaceID())
						number++;
				}
			}
		ar << number;
		for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			{
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
				{
					if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == pPlayer->GetRaceID())
					{
						ar << CPoint(x,y);
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
					}
				}
			}
		network::RACE client = m_pRaceCtrl->GetMappedClientID(pPlayer->GetRaceID());
		pPlayer->Serialize(ar);
		// aktuelle View mit zum Server senden
		ar << m_iSelectedView[client];
	}
	else
	{
		// vom Client gespielte Majorrace-ID ermitteln
		CString sMajorID = m_pRaceCtrl->GetMappedRaceID(race);

		if (m_bCombatCalc)
		{
			MYTRACE("general")(MT::LEVEL_INFO, "Server receiving CombatData from client %d...\n", race);

			// Informationen �ber die Taktik der Schiffe bzw. die Taktik des Kampfes empfangen
			int nOrder;
			ar >> nOrder;
			COMBAT_ORDER::Typ nCombatOrder = (COMBAT_ORDER::Typ)nOrder;
			if (nCombatOrder != COMBAT_ORDER::NONE)
				m_mCombatOrders[sMajorID] = nCombatOrder;

			/*
			// Server stellt nun alle zum Client geh�renden Schiffe im Kampfsektor auf
			// den erhaltenen Befehl um
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
			{
				if (m_ShipArray.GetAt(i).GetOwnerOfShip() == sMajorID && m_ShipArray.GetAt(i).GetKO() == m_ptCurrentCombatSector)
				{

				}
			}
			*/

			return;
		}

		MYTRACE("general")(MT::LEVEL_INFO, "Server receiving EndOfRoundData from client %d...\n", race);
		// Server-Dokument
		CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sMajorID));
		ASSERT(pMajor);
		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
			if (m_ShipInfoArray.GetAt(i).GetRace() == pMajor->GetRaceShipNumber())
			{
				m_ShipInfoArray.GetAt(i).Serialize(ar);
				ASSERT(m_ShipInfoArray.GetAt(i).GetRace() == pMajor->GetRaceShipNumber());
			}

		int number = 0;
		ar >> number;
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
			if (m_ShipArray.GetAt(i).GetOwnerOfShip() == sMajorID)
				m_ShipArray.RemoveAt(i--);
		int oldSize = m_ShipArray.GetSize();
		m_ShipArray.SetSize(oldSize + number);
		for (int i = oldSize; i < m_ShipArray.GetSize(); i++)
		{
			m_ShipArray.GetAt(i).Serialize(ar);
			ASSERT(m_ShipArray.GetAt(i).GetOwnerOfShip() == sMajorID);
		}

		number = 0;
		ar >> number;
		for (int i = 0; i < number; i++)
		{
			CPoint p;
			ar >> p;
			m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).Serialize(ar);
		}

		pMajor->Serialize(ar);
		ar >> m_iSelectedView[race];
	}
	MYTRACE("general")(MT::LEVEL_INFO, "... serialization of RoundEndData succesfull\n", race);
}

/// Funktion liest die Ini-Datei neu ein und legt die Werte neu fest.
void CBotf2Doc::ResetIniSettings(void)
{
	CIniLoader* pIni = CIniLoader::GetInstance();
	ASSERT(pIni);

	CString difficulty = "EASY";
	pIni->ReadValue("General", "DIFFICULTY", difficulty);
	difficulty.MakeUpper();
	MYTRACE("general")(MT::LEVEL_INFO, "relevant only at new game: Bote.ini: DIFFICULTY: %s", difficulty);
	if (difficulty == "BABY")
		m_fDifficultyLevel			= 1.5f;
	else if (difficulty == "EASY")
		m_fDifficultyLevel			= 1.0f;
	else if (difficulty == "NORMAL")
		m_fDifficultyLevel			= 0.5f;
	else if (difficulty == "HARD")
		m_fDifficultyLevel			= 0.33f;
	else if (difficulty == "IMPOSSIBLE")
		m_fDifficultyLevel			= 0.2f;
	else
		m_fDifficultyLevel			= 0.5f;
	MYTRACE("general")(MT::LEVEL_INFO, "relevant only at new game: m_fDifficultyLevel: %f", m_fDifficultyLevel);


	CSoundManager* pSoundManager = CSoundManager::GetInstance();
	ASSERT(pSoundManager);

	bool bHardwareSound;
	pIni->ReadValue("Audio", "HARDWARESOUND", bHardwareSound);
	CSoundManager::GetInstance()->Init(!bHardwareSound);

	bool bUseMusic;
	pIni->ReadValue("Audio", "MUSIC", bUseMusic);
	if (bUseMusic)
	{
		CMajor* pPlayer = GetPlayersRace();
		ASSERT(pPlayer);
		network::RACE client = m_pRaceCtrl->GetMappedClientID(pPlayer->GetRaceID());

		float fMusicVolume;
		pIni->ReadValue("Audio", "MUSICVOLUME", fMusicVolume);
		pSoundManager->StartMusic(client, fMusicVolume);
	}
	else
		pSoundManager->StopMusic();


	bool bUseSound;
	pIni->ReadValue("Audio", "SOUND", bUseSound);
	if (!bUseSound)
		pSoundManager->SetSoundMasterVolume(NULL);
	else
		pSoundManager->SetSoundMasterVolume(0.5f);
	MYTRACE("general")(MT::LEVEL_INFO, "Init sound ready...\n");

	int nSeed = -1;
	pIni->ReadValue("Special", "RANDOMSEED", nSeed);

	// festen vorgegeben Seed verwenden
	if (nSeed >= 0)
		srand(nSeed);
	// zuf�lligen Seed verwenden
	else
	{
		nSeed = (unsigned)time(NULL);
		srand(nSeed);
	}
	MYTRACE("general")(MT::LEVEL_INFO, "Used seed for randomgenerator: %i", nSeed);
}

/// Funktion gibt die Koordinate des Hauptsystems einer Majorrace zur�ck.
/// @param sMajor Rassen-ID
/// @return Koordinate auf der Galaxiemap
CPoint CBotf2Doc::GetRaceKO(const CString& sMajorID) const
{
	const std::map<CString, std::pair<int, int>>::const_iterator race = m_mRaceKO.find(sMajorID);
	if (race == m_mRaceKO.end())
		return CPoint(-1,-1);
	return CPoint(race->second.first, race->second.second);
}

void CBotf2Doc::SetKO(int x, int y)
{
	m_ptKO = CPoint(x, y);
	CSmallInfoView::SetPlanet(NULL);

	if (GetMainFrame()->GetActiveView(1, 1) == PLANET_BOTTOM_VIEW)
		GetMainFrame()->InvalidateView(RUNTIME_CLASS(CPlanetBottomView));
}

void CBotf2Doc::SetCurrentShipIndex(int NumberOfTheShipInArray)
{
	m_NumberOfTheShipInArray = NumberOfTheShipInArray;
	((CGalaxyMenuView*)GetMainFrame()->GetView(RUNTIME_CLASS(CGalaxyMenuView)))->SetNewShipPath();
	CSmallInfoView::SetShip(&m_ShipArray.GetAt(NumberOfTheShipInArray));
	//re-activation later...MYTRACE("general")(MT::LEVEL_INFO, "m_NumberOfTheShipInArray: %i", m_NumberOfTheShipInArray);
}

void CBotf2Doc::SetNumberOfFleetShip(int NumberOfFleetShip)
{
	m_iNumberOfFleetShip = NumberOfFleetShip;
	CSmallInfoView::SetShip(&m_ShipArray.GetAt(NumberOfFleetShip));
	//re-activation later...MYTRACE("general")(MT::LEVEL_INFO, "m_iNumberOfFleetShip: %i", m_iNumberOfFleetShip);

}

void CBotf2Doc::SetNumberOfTheShipInFleet(int NumberOfTheShipInFleet)
{
	m_iNumberOfTheShipInFleet = NumberOfTheShipInFleet;
	if (NumberOfTheShipInFleet > 0)
		CSmallInfoView::SetShip(m_ShipArray.GetAt(m_iNumberOfFleetShip).GetFleet()->GetShipFromFleet(NumberOfTheShipInFleet - 1));
	else if (NumberOfTheShipInFleet == 0)
		CSmallInfoView::SetShip(&m_ShipArray.GetAt(m_iNumberOfFleetShip));
}

/// Funktion l�dt f�r die ausgew�hlte Spielerrasse alle Grafiken f�r die Views.
void CBotf2Doc::LoadViewGraphics(void)
{
	CMajor* pPlayersRace = GetPlayersRace();
	ASSERT(pPlayersRace);
	MYTRACE("general")(MT::LEVEL_INFO, "pPlayersRace: %s", pPlayersRace->GetRaceName());

	CGalaxyMenuView::SetPlayersRace(pPlayersRace);
	CMainBaseView::SetPlayersRace(pPlayersRace);
	CBottomBaseView::SetPlayersRace(pPlayersRace);
	CMenuChooseView::SetPlayersRace(pPlayersRace);

	// Views die rassenspezifischen Grafiken laden lassen
	std::map<CWnd *, UINT>* views = &GetMainFrame()->GetSplitterWindow()->views;
	for (std::map<CWnd *, UINT>::iterator it = views->begin(); it != views->end(); ++it)
	{
		if (it->second == GALAXY_VIEW)
			continue;
		else if (it->second == MENUCHOOSE_VIEW)
			((CMenuChooseView*)(it->first))->LoadRaceGraphics();
		else if (IS_MAIN_VIEW(it->second))
			((CMainBaseView*)(it->first))->LoadRaceGraphics();
		else if (IS_BOTTOM_VIEW(it->second))
			((CBottomBaseView*)(it->first))->LoadRaceGraphics();
	}

	// Ini-Werte lesen und setzen
	ResetIniSettings();

	// ab jetzt m�ssen keine neuen Grafiken mehr geladen werden
	m_bNewGame = false;

	// Views ihre Arbeit zu Beginn jeder neuen Runde machen lassen
	DoViewWorkOnNewRound();

	network::RACE client = m_pRaceCtrl->GetMappedClientID(pPlayersRace->GetRaceID());
	// wenn neues Spiel gestartet wurde, dann initial auf die Galaxiekarte stellen
	if (m_iSelectedView[client] == 0)
	{
		// zum Schluss die Galxieview ausw�hlen (nicht eher, da gibts manchmal Probleme beim Scrollen ganz nach rechts)
		GetMainFrame()->SelectMainView(GALAXY_VIEW, pPlayersRace->GetRaceID());
	}
}

void CBotf2Doc::DoViewWorkOnNewRound()
{
	// Playersrace in Views festlegen
	CMajor* pPlayersRace = GetPlayersRace();
	ASSERT(pPlayersRace);

	CGalaxyMenuView::SetPlayersRace(pPlayersRace);
	CMainBaseView::SetPlayersRace(pPlayersRace);
	CBottomBaseView::SetPlayersRace(pPlayersRace);
	CMenuChooseView::SetPlayersRace(pPlayersRace);

	// Views ihre Arbeiten zu Beginn einer neuen Runde durchf�hren lassen
	std::map<CWnd *, UINT>* views = &GetMainFrame()->GetSplitterWindow()->views;
	for (std::map<CWnd *, UINT>::iterator it = views->begin(); it != views->end(); ++it)
	{
		if (it->second == GALAXY_VIEW)
			((CGalaxyMenuView*)(it->first))->OnNewRound();
		else if (IS_MAIN_VIEW(it->second))
			((CMainBaseView*)(it->first))->OnNewRound();
		else if (IS_BOTTOM_VIEW(it->second))
			((CBottomBaseView*)(it->first))->OnNewRound();
	}

	network::RACE client = m_pRaceCtrl->GetMappedClientID(pPlayersRace->GetRaceID());

	// anzuzeigende View in neuer Runde ausw�hlen
	// Wenn EventScreens f�r den Spieler vorhanden sind, so werden diese angezeigt.
	if (pPlayersRace->GetEmpire()->GetEventMessages()->GetSize() > 0)
	{
		GetMainFrame()->FullScreenMainView(true);
		GetMainFrame()->SelectMainView(EVENT_VIEW, pPlayersRace->GetRaceID());
	}
	else
	{
		GetMainFrame()->FullScreenMainView(false);
		GetMainFrame()->SelectMainView(m_iSelectedView[client], pPlayersRace->GetRaceID());
		m_iSelectedView[client] = 0;
	}

	// wurde Rundenende geklickt zur�cksetzen
	m_bRoundEndPressed = false;
	m_bDataReceived = true;

	// alle angezeigten Views neu zeichnen lassen
	UpdateAllViews(NULL);
}

// Generiert ein neues Spiel
void CBotf2Doc::PrepareData()
{
	MYTRACE("general")(MT::LEVEL_INFO, "Begin preparing game data...\n");

	if (!m_bGameLoaded)
	{
		// neue Majors anlegen
		if (!m_pRaceCtrl->Init())
		{
			AfxMessageBox("CBotf2Doc::PrepareData(): Could not initiate races!");
			exit(1);
		}
		// Spieler den Majors zuweisen
		map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
			// wird das Imperium von einem Menschen oder vom Computer gespielt
			if (client != network::RACE_NONE && server.IsPlayedByClient(client))
				it->second->SetHumanPlayer(true);
			else
				it->second->SetHumanPlayer(false);
		}

		// ALPHA6 DEBUG alle Rassen untereinander bekanntgeben
		const CCommandLineParameters* const clp = dynamic_cast<CBotf2App*>(AfxGetApp())->GetCommandLineParameters();
		if(clp->SeeAllOfMap()) {
			map<CString, CRace*>* pmRaces = m_pRaceCtrl->GetRaces();
			for (map<CString, CRace*>::iterator it = pmRaces->begin(); it != pmRaces->end(); ++it)
				for (map<CString, CRace*>::const_iterator jt = pmRaces->begin(); jt != pmRaces->end(); ++jt)
					if (it->first != jt->first && it->second->GetType() == MAJOR && jt->second->GetType() == MAJOR)
						it->second->SetIsRaceContacted(jt->first, true);
		}

		m_iRound = 1;

		//Neuberechnung der Galaxiengr��e falls im Einstellungsmen� ge�ndert
		//CIniLoader* pIni = CIniLoader::GetInstance();
		//int mapHeight=20;
		//pIni->ReadValue("Special", "MAPSIZEV", mapHeight);
		//STARMAP_SECTORS_VCOUNT=mapHeight;

		//int mapWidth=30;
		//pIni->ReadValue("Special", "MAPSIZEH", mapWidth);
		//STARMAP_SECTORS_HCOUNT=mapWidth;

		STARMAP_TOTALWIDTH=STARMAP_SECTORS_HCOUNT*80;
		STARMAP_TOTALHEIGHT=STARMAP_SECTORS_VCOUNT*80;

		//AllocateSectorsAndSystems();

		// Generierungssektornamenklasse wieder neu starten
		m_ShipArray.RemoveAll();
		m_ShipInfoArray.RemoveAll();

		ReadBuildingInfosFromFile();	// Geb�ude einlesen aus data-Datei
		ReadShipInfosFromFile();		// Schiffe einlesen aus data-Datei
		ReadTroopInfosFromFile();		// Truppen einlesen aus data-Datei

		// Schiffsnamen einlesen
		m_GenShipName.Init(this);

		// Werte f�r Moral�nderungen auf verschiedene Ereignisse laden
		CMoralObserver::InitMoralMatrix();

		// Sektoren generieren
		GenerateGalaxy();
		ApplyShipsAtStartup();
		ApplyBuildingsAtStartup();
		ApplyTroopsAtStartup();

		// Siegbedingungen initialisieren und erstmalig �berwachen
		m_VictoryObserver.Init();
		m_VictoryObserver.Observe();

		MYTRACE("general")(MT::LEVEL_INFO, "Preparing game data ready...\n");
		/*
		double habis = 0;
		CString s;
		double hab = 0;
		double temp;
		CPoint poi;
		for (int y = 0; y < 20; y++)
			for (int x = 0; x < 30; x++)
			{
				temp = 0;
				hab = 0;
				int number=m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetNumberOfPlanets();
				for (int i=0;i<number;i++)
				{
					CPlanet* planet = m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanet(i);
					temp+=planet->GetMaxHabitant();
					hab = temp;
				}
				if (habis < hab)
				{
					habis = hab;
					poi.x = x; poi.y = y;
				}
			}
		s.Format("Gr��tes System ist %s mit %lf Bev�lkerung\nin Sektor %d:%d",m_Sectors.at(poi.x+(poi.y)*STARMAP_SECTORS_HCOUNT).GetName(),habis,poi.x,poi.y);
		AfxMessageBox(s);
		*/
	}
	// wenn geladen wird
	else
	{
		// Spieler den Majors zuweisen
		map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
			// wird das Imperium von einem Menschen oder vom Computer gespielt
			if (client != network::RACE_NONE && server.IsPlayedByClient(client))
				it->second->SetHumanPlayer(true);
			else
				it->second->SetHumanPlayer(false);
		}
	}
}

/// Funktion generiert die Galaxiemap inkl. der ganzen Systeme und Planeten zu Beginn eines neuen Spiels.
void CBotf2Doc::GenerateGalaxy()
{
	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			// Alle Werte der Systemklasse wieder auf NULL setzen
			m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).ResetSystem();
			// Alle Werte der Sektorklasse wieder auf NULL setzen
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).Reset();
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetKO(x,y);
		}
	m_mRaceKO.clear();


	int nGenerationMode=0;//0==Standart 1==Circle

	std::vector<std::vector<bool>> nGenField
		(STARMAP_SECTORS_HCOUNT, std::vector<bool>(STARMAP_SECTORS_VCOUNT, true));

	CIniLoader::GetInstance()->ReadValue("Special", "GENERATIONMODE", nGenerationMode);

	if(nGenerationMode!=0)
	{
		CString sAppPath = CIOData::GetInstance()->GetAppPath();
		CString filePath = "";
		filePath.Format("%sGraphics\\Galaxies\\pattern%d.boj",sAppPath,nGenerationMode);
		std::auto_ptr<Bitmap> GalaxyPattern(Bitmap::FromFile(CComBSTR(filePath)));
		if(GalaxyPattern.get()==NULL)
		{
			sAppPath.Format("pattern%d.boj not found! using standart pattern", nGenerationMode);
			AfxMessageBox(sAppPath);

		}else{
			FCObjImage img;
			FCWin32::GDIPlus_LoadBitmap(*GalaxyPattern, img);
			if(img.IsValidImage())
			{
				img.Stretch(STARMAP_SECTORS_HCOUNT,STARMAP_SECTORS_VCOUNT);
				GalaxyPattern.reset(FCWin32::GDIPlus_CreateBitmap(img));
				Gdiplus::Color nColor;
				for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
					for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
					{
						GalaxyPattern->GetPixel(x,y,&nColor);
						if(nColor.GetR()>50&&nColor.GetB()>50&&nColor.GetG()>50) nGenField[x][y]=false;
					}
			}
			else
			{
				sAppPath.Format("pattern%d.boj not found! using standart pattern", nGenerationMode);
				AfxMessageBox(sAppPath);
			}
		}
	}

	// Die sechs Hauptrassen werden zuf�llig auf der Karte verteilt. Dabei ist aber zu beachten, dass die Entfernungen
	// zwischen den Systemen gro� genug sind. Au�erdem m�ssen um jedes der Hauptsysteme zwei weitere Systeme liegen.
	// Diese werden wenn n�tig auch generiert.
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();

	// minimaler Abstand der Majorraceheimatsysteme
	int nMinDiff =(int)sqrt((double)STARMAP_SECTORS_VCOUNT*STARMAP_SECTORS_HCOUNT)/2+2  - pmMajors->size(); //Term der Abstand ungef�hr an Feld gr��e anpasst
	// minimal 4 Felder abstand
	nMinDiff = max(nMinDiff, 4);

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); )
	{
		// Zuf�llig einen Sektor ermitteln. Dies wird solange getan, solange der ermittelte Sektor noch nicht die
		// if Bedingung erf�llt oder die Abbruchbedingung erreicht ist.
		bool bRestart = false;
		int nCount = 0;

		do
		{
			m_mRaceKO[it->first].first = rand()%(STARMAP_SECTORS_HCOUNT);
			m_mRaceKO[it->first].second= rand()%(STARMAP_SECTORS_VCOUNT);

			for (map<CString, CMajor*>::const_iterator jt = pmMajors->begin(); jt != pmMajors->end(); ++jt)
			{
				if (it->first != jt->first && GetRaceKO(jt->first) != CPoint(-1,-1) && abs(GetRaceKO(it->first).x - GetRaceKO(jt->first).x) < nMinDiff && abs(GetRaceKO(it->first).y - GetRaceKO(jt->first).y) < nMinDiff||GetRaceKO(it->first) != CPoint(-1,-1)&&nGenField[GetRaceKO(it->first).x][GetRaceKO(it->first).y]==false)
					m_mRaceKO[it->first] = pair<int,int>(-1,-1);
			}
			// Abbruchbedingung
			if (++nCount > max((STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT) / 4, 20))
				bRestart = true;
		}
		while (GetRaceKO(it->first) == CPoint(-1,-1) && bRestart == false);

		// n�chsten Major ausw�hlen
		++it;

		// Es konnte kein Sektor nach 250 Durchl�ufen gefunden werden, welcher der obigen if Bedingung gen�gt, so
		// wird die for Schleife nochmal neu gestartet.
		if (bRestart)
		{
			m_mRaceKO.clear();
			it = pmMajors->begin();
		}
	}

	// alle potentiellen Minorracesystemnamen holen
	map<CString, CMinor*>* pmMinors = m_pRaceCtrl->GetMinors();
	CStringArray vMinorRaceSystemNames;
	for (map<CString, CMinor*>::const_iterator it = pmMinors->begin(); it != pmMinors->end(); ++it)
	{
		CMinor* pMinor = it->second;
		if (!pMinor)
		{
			ASSERT(pMinor);
			continue;
		}

		// keine Minors ohne Heimatsystem ins Spiel bringen (Aliens haben kein Heimatsystem)
		if (!pMinor->IsAlienRace())
			vMinorRaceSystemNames.Add(pMinor->GetHomesystemName());
	}

	// Namensgenerator initialisieren
	CGenSectorName::GetInstance()->Init(vMinorRaceSystemNames);

	int nStarDensity = 35;
	int nMinorDensity = 30;
	int nAnomalyDensity = 9;
	CIniLoader::GetInstance()->ReadValue("Special", "STARDENSITY", nStarDensity);
	CIniLoader::GetInstance()->ReadValue("Special", "MINORDENSITY", nMinorDensity);
	CIniLoader::GetInstance()->ReadValue("Special", "ANOMALYDENSITY", nAnomalyDensity);
	MYTRACE("general")(MT::LEVEL_INFO, "relevant only at new game: Bote.ini: STARDENSITY: %i", nStarDensity);
	MYTRACE("general")(MT::LEVEL_INFO, "relevant only at new game: Bote.ini: MINORDENSITY: %i", nMinorDensity);
	MYTRACE("general")(MT::LEVEL_INFO, "relevant only at new game: Bote.ini: ANOMALYDENSITY: %i", nAnomalyDensity);

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor const* const pMajor = it->second;
		const CPoint& raceKO = GetRaceKO(it->first);
		CSector& sector = GetSector(raceKO);
		CSystem& system = GetSystem(raceKO);

		sector.SetSectorsName(pMajor->GetHomesystemName());
		sector.SetSunSystem(TRUE);
		sector.SetFullKnown(it->first);
		sector.SetOwned(TRUE);
		sector.SetOwnerOfSector(it->first);
		sector.SetColonyOwner(it->first);
		sector.CreatePlanets(it->first);
		system.SetOwnerOfSystem(it->first);
		system.SetResourceStore(TITAN, 1000);
		system.SetResourceStore(DERITIUM, 3);

		// Zwei Sonnensysteme in unmittelbarer Umgebung des Heimatsystems anlegen
		BYTE nextSunSystems = 0;
		while (nextSunSystems < 2)
		{
			// Punkt mit Koordinaten zwischen -1 und +1 generieren
			CPoint ko(rand()%3 - 1, rand()%3 - 1);
			if (raceKO.x + ko.x < STARMAP_SECTORS_HCOUNT && raceKO.x + ko.x > -1
				&& raceKO.y + ko.y < STARMAP_SECTORS_VCOUNT && raceKO.y + ko.y > -1)
				if (!m_Sectors.at(raceKO.x + ko.x+(raceKO.y + ko.y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
				{
					m_Sectors.at(raceKO.x + ko.x+(raceKO.y + ko.y)*STARMAP_SECTORS_HCOUNT).GenerateSector(100, nMinorDensity);
					nextSunSystems++;
				}
		};

		// In einem Radius von einem Feld um das Hauptsystem die Sektoren scannen
		for (int y = -1; y <= 1; y++)
			for (int x = -1; x <= 1; x++)
				if (raceKO.y + y < STARMAP_SECTORS_VCOUNT && raceKO.y + y > -1
					&& raceKO.x + x < STARMAP_SECTORS_HCOUNT && raceKO.x + x > -1)
						if (raceKO.x + x != raceKO.x || raceKO.y + y != raceKO.y)
							m_Sectors.at(raceKO.x + x+(raceKO.y + y)*STARMAP_SECTORS_HCOUNT).SetScanned(it->first);
	}

	// Vektor der verwendeten Minors, diese nehmen aktiv am Spiel teil.
	set<CString> sUsedMinors;
	// nun die Sektoren generieren
	vector<CPoint> vSectorsToGenerate;
	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if ((!m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwned())&&nGenField[x][y]==true)
				vSectorsToGenerate.push_back(CPoint(x,y));

	while (!vSectorsToGenerate.empty())
	{
		int nSector = rand()%vSectorsToGenerate.size();
		int x = vSectorsToGenerate[nSector].x;
		int y = vSectorsToGenerate[nSector].y;
		// behandelten Sektor entfernen
		vSectorsToGenerate.erase(vSectorsToGenerate.begin() + nSector);

		// den Sektor generieren
		// die Wahrscheinlichkeiten sind abh�ngig von den Systemen in der unmittelbaren N�he. Ist ein Majorrace-
		// hauptsystem in der N�he, so wird hier kein System generiert, da diese schon weiter oben angelegt
		// wurden
		int sunSystems = 0;
		int minorRaces = 0;
		int nAnomalys  = 0;
		for (int j = -1; j <= 1; j++)
			for (int i = -1; i <= 1; i++)
				if (y + j < STARMAP_SECTORS_VCOUNT && y + j > -1 && x + i < STARMAP_SECTORS_HCOUNT && x + i > -1)
				{
					if (m_Sectors.at(x + i+(y + j)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
					{
						if (m_Sectors.at(x + i+(y + j)*STARMAP_SECTORS_HCOUNT).GetMinorRace())
							minorRaces++;
						sunSystems++;
					}
					else if (m_Sectors.at(x + i+(y + j)*STARMAP_SECTORS_HCOUNT).GetAnomaly())
						nAnomalys++;

					for (map<CString, pair<int, int> >::const_iterator it = m_mRaceKO.begin(); it != m_mRaceKO.end(); ++it)
						if (it->second.first == x + i && it->second.second == y + j)
						{
							sunSystems	+= 100;
							nAnomalys	+= 100;
						}
				}
		int sunSystemProb = nStarDensity  - sunSystems * 15;
		int minorRaceProb = nMinorDensity - minorRaces * 15;
		if (minorRaceProb < 0)
			minorRaceProb = 0;
		if (sunSystemProb > 0)
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GenerateSector(sunSystemProb, minorRaceProb);
		if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMinorRace())
		{
			// Nun die Minorrace parametrisieren
			CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
			if (!pMinor)
				AfxMessageBox("Error in function CBotf2Doc::GenerateGalaxy(): Could not create Minorrace");
			else
			{
				pMinor->SetRaceKO(CPoint(x,y));
				m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector(pMinor->GetRaceID());
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSystem("");
				sUsedMinors.insert(pMinor->GetRaceID());
				// wenn die Minorrace Schiffe bauen kann, sie aber kein Deritium im System besitzt, so wird
				// ein Deritium auf dem ersten kolonisierten Planeten hinzugef�gt
				if (pMinor->GetSpaceflightNation())
				{
					BOOLEAN bRes[DERITIUM + 1] = {FALSE};
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAvailableResources(bRes, true);
					// gibt es kein Deritium=
					if (!bRes[DERITIUM])
					{
						for (int p = 0; p < static_cast<int>(m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanets().size()); p++)
							if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanet(p)->GetCurrentHabitant() > 0 && m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanet(p)->GetColonized())
							{
								m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanet(p)->SetBoni(DERITIUM, TRUE);
								break;
							}
					}
				}
			}
		}
		// m�glicherweise eine Anomalie im Sektor generieren
		else if (!m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
			if (rand()%100 >= (100 - (nAnomalyDensity - nAnomalys * 10)))
				m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).CreateAnomaly();
	}

	// nun k�nnen alle nicht verwendeten Minors entfernt werden
	vector<CString> vDelMinors;
	for (map<CString, CMinor*>::iterator it = pmMinors->begin(); it != pmMinors->end(); ++it)
	{
		if (sUsedMinors.find(it->first) == sUsedMinors.end())
		{
			// keine Aliens ohne Heimatsystem rausl�schen
			if (!it->second->IsAlienRace())
				vDelMinors.push_back(it->first);
		}
	}

	for (UINT i = 0; i < vDelMinors.size(); i++)
		m_pRaceCtrl->RemoveRace(vDelMinors[i]);
}

void CBotf2Doc::NextRound()
{
	// gibt es f�r diese Runde Sektoren in welchen ein Kampf stattfand
	bool bCombatInCurrentRound = !m_sCombatSectors.empty();

	// Es fand noch kein Kampf die Runde statt. Dies tritt ein, wenn entweder wirklich kein Kampf diese Runde war
	// oder das erste Mal in diese Funktion gesprungen wurde.
	if (bCombatInCurrentRound == false)
	{
		MYTRACE("general")(MT::LEVEL_INFO, "#### START NEXT ROUND (round: %d) ####", GetCurrentRound());

		// Seed initialisieren
		int nSeed = -1;
		CIniLoader::GetInstance()->ReadValue("Special", "RANDOMSEED", nSeed);
		// festen vorgegeben Seed verwenden
		if (nSeed >= 0)
			srand(nSeed);
		// zuf�lligen Seed verwenden
		else
		{
			nSeed = (unsigned)time(NULL);
			srand(nSeed);
		}

		MYTRACE("general")(MT::LEVEL_INFO, "Used seed for randomgenerator: %i", nSeed);

		// Soundnachrichten aus alter Runde l�schen
		for (int i = network::RACE_1; i < network::RACE_ALL; i++)
			m_SoundMessages[i].RemoveAll();

		// ausgel�schte Rassen gleich zu Beginn der neuen Runde entfernen. Menschliche Spieler sollten jetzt schon disconnected sein!!!
		vector<CString> vDelMajors;
		map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
			if (it->second->GetEmpire()->GetNumberOfSystems() == 0)
				vDelMajors.push_back(it->first);
		for (UINT i = 0; i < vDelMajors.size(); i++)
		{
			MYTRACE("general")(MT::LEVEL_INFO, "Race %d is out of game", vDelMajors[i]);
			m_pRaceCtrl->RemoveRace(vDelMajors[i]);
			pmMajors = m_pRaceCtrl->GetMajors();
		}

		// Starmap f�r alle Rassen berechnen und anlegen
		GenerateStarmap();

		// KI-Berechnung
		m_pSectorAI->Clear();
		m_pSectorAI->CalculateDangers();
		m_pSectorAI->CalcualteSectorPriorities();

		CShipAI ShipAI(this);
		ShipAI.CalculateShipOrders(m_pSectorAI);

		m_pAIPrios->Clear();
		m_pAIPrios->CalcShipPrios(m_pSectorAI);
		m_pAIPrios->GetIntelAI()->CalcIntelligence(this);

		// Alle Statistiken des Spiels berechnen (erst nachdem die Sector-AI berechnet wurde!
		// Nimmt von dort gleich die Schiffsst�rken)
		m_Statistics.CalcStats(this);

		this->CalcPreDataForNextRound();
		// Diplomatie nach dem Hochz�hlen der Runde aber noch vor der Schiffsbewegung durchf�hren
		this->CalcDiplomacy();
		this->CalcShipOrders();
		this->CalcShipMovement();
		// pr�fen ob ein Kampf stattfindet
		if (IsShipCombat())
		{
			// Ist ein menschlicher Spieler beteiligt?
			bool bHumanPlayerInCombat = false;
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
			{
				if (m_ShipArray[i].GetKO() == m_ptCurrentCombatSector)
				{
					CString sOwnerID = m_ShipArray[i].GetOwnerOfShip();
					if (pmMajors->find(sOwnerID) != pmMajors->end())
					{
						if (pmMajors->find(sOwnerID)->second->IsHumanPlayer())
						{
							bHumanPlayerInCombat = true;
							break;
						}
					}
				}
			}
			// kein menschlicher Spieler beteiligt -> gleich weiter
			if (!bHumanPlayerInCombat)
				NextRound();

			// findet ein Kampf statt, so sofort aus der Funktion rausgehen und die Kampfberechnungen durchf�hren
			return;
		}
	}
	// Es findet ein Kampf statt
	else
	{
		MYTRACE("general")(MT::LEVEL_INFO, "COMBAT ROUND\n");
		// Kampf berechnen
		CalcShipCombat();
		// Wenn wieder ein Kampf stattfindet, so aus der Funktion springen
		if (IsShipCombat())
		{
			map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
			// Ist ein menschlicher Spieler beteiligt?
			bool bHumanPlayerInCombat = false;
			for (int i = 0; i < m_ShipArray.GetSize(); i++)
			{
				if (m_ShipArray[i].GetKO() == m_ptCurrentCombatSector)
				{
					CString sOwnerID = m_ShipArray[i].GetOwnerOfShip();
					if (pmMajors->find(sOwnerID) != pmMajors->end())
					{
						if (pmMajors->find(sOwnerID)->second->IsHumanPlayer())
						{
							bHumanPlayerInCombat = true;
							break;
						}
					}
				}
			}
			// kein menschlicher Spieler beteiligt -> gleich weiter
			if (!bHumanPlayerInCombat)
				NextRound();

			// findet ein Kampf statt, so sofort aus der Funktion rausgehen und die Kampfberechnungen durchf�hren
			return;
		}
	}

	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	// Minors erst nach einem Kampf berechnen, so dass nicht in der gleichen Runde deren Schiff gegen ein anderes K�mpfen kann
	// Minors ausbreiten, Akzeptanzpunkte berechnen und Ressourcen verbrauchen
	map<CString, CMinor*>* pmMinors = m_pRaceCtrl->GetMinors();
	for (map<CString, CMinor*>::const_iterator it = pmMinors->begin(); it != pmMinors->end(); ++it)
	{
		CMinor* pMinor = it->second;

		// Die Punkte f�r l�ngere gute Beziehungen berechnen
		pMinor->CalcAcceptancePoints(this);

		// wurde die Rasse erobert oder geh�rt jemanden, dann nicht hier weitermachen
		if (pMinor->GetSubjugated())
			continue;
		bool bMember = false;
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			if (pMinor->GetAgreement(it->first) == DIPLOMATIC_AGREEMENT::MEMBERSHIP)
			{
				bMember = true;
				break;
			}
		}
		if (bMember)
			continue;

		CPoint ko = pMinor->GetRaceKO();

		if (ko != CPoint(-1,-1) && m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == "" && m_Sectors.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == pMinor->GetRaceID())
		{
			// Vielleicht kolonisiert die Minorrace weitere Planeten in ihrem System
			if (pMinor->PerhapsExtend(this))
				// dann sind im System auch weitere Einwohner hinzugekommen
				m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).SetHabitants(this->GetSector(ko).GetCurrentHabitants());

			// Den Verbrauch der Rohstoffe der kleinen Rassen in jeder Runde berechnen
			pMinor->ConsumeResources(this);
			// Vielleicht baut die Rasse ein Raumschiff
			pMinor->PerhapsBuildShip(this);
		}
	}
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)//Random Events berechenn
	{
		CMajor* pMajor = it->second;
		m_RandomEventManager.CalcEvents(pMajor);
	};

	this->CalcSystemAttack();
	this->CalcIntelligence();
	this->CalcResearch();

	// alten Creditbestand festhalten
	map<CString, long> mOldCredits;
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		mOldCredits[it->first] = pMajor->GetEmpire()->GetCredits();
	}

	// Auswirkungen von Alienschiffen beachten
	this->CalcAlienShipEffects();
	// alle Systeme berechnen (Bauliste, Moral, Energie usw.)
	this->CalcOldRoundData();
	// Aliens zuf�llig ins Spiel bringen (vor der Berechnung der Schiffsauswirkungen)
	this->CalcRandomAlienEntities();
	// Schiffsauswirkungen berechnen (Scanst�rken, Erfahrung usw.)
	this->CalcShipEffects();
	this->CalcNewRoundData();
	this->CalcTrade();

	this->CalcEndDataForNextRound();

	// Credit�nderung berechnen
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		pMajor->GetEmpire()->SetCreditsChange((int)(pMajor->GetEmpire()->GetCredits() - mOldCredits[pMajor->GetRaceID()]));
	}

	// In dieser Runde stattgefundene K�mpfe l�schen
	m_sCombatSectors.clear();
	m_bCombatCalc = false;

	bool bAutoSave = false;
	CIniLoader::GetInstance()->ReadValue("General", "AUTOSAVE", bAutoSave);
	if (bAutoSave)
		DoSave(CIOData::GetInstance()->GetAutoSavePath(m_iRound), FALSE);
	SetModifiedFlag();

	MYTRACE("general")(MT::LEVEL_INFO, "\nNEXT ROUND calculation successfull\n", GetCurrentRound());
}

void CBotf2Doc::ApplyShipsAtStartup()
{
	// Name des zu �ffnenden Files
	CString fileName= CIOData::GetInstance()->GetAppPath() + "Data\\Ships\\StartShips.data";
	CStdioFile file;
	// Datei wird ge�ffnet
	if (file.Open(fileName, CFile::modeRead | CFile::typeText))
	{
		// auf csInput wird die jeweilige Zeile gespeichert
		CString csInput;
		while (file.ReadString(csInput))
		{
			if (csInput.Left(2) == "//" || csInput.IsEmpty())
				continue;
			int pos = 0;
			// Besitzer des Schiffes auslesen
			CString s = csInput.Tokenize(":", pos);
			CString sOwner = s;
			// Namen des Systems holen
			CString systemName = s = csInput.Tokenize(":", pos);
			// Systemnamen auf der Map suchen
			bool bFoundSector = false;
			for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			{
				for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
					// Sektornamen gefunden
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(TRUE) == s)
					{
						bFoundSector = true;
						// Schiffsklassen durchgehen, die dort stationiert werden sollen
						while (pos < csInput.GetLength())
						{
							s = csInput.Tokenize(",", pos);
							bool bFoundShip = false;
							for (int j = 0; j < m_ShipInfoArray.GetSize(); j++)
							{
								// Wurde eine entsprechende Schiffsklasse gefunden, diese bauen
								if (m_ShipInfoArray.GetAt(j).GetShipClass() == s)
								{
									bFoundShip = true;
									BuildShip(m_ShipInfoArray.GetAt(j).GetID(), CPoint(x,y), sOwner);
									break;
								}
							}
							// Wurde die Schiffsklasse nicht gefunden, sprich es ist ein Fehler in der Datei
							if (!bFoundShip)
							{
								CString shipClass;
								shipClass.Format("Could not find ship class \"%s\"\nPlease check your StartShips.data file!", s);
								AfxMessageBox(shipClass);
							}
						}
						break;
					}

				if (bFoundSector)
					break;
			}
			// Wurde das System nicht gefunden, sprich es ist ein Fehler in der Datei
			if (!bFoundSector)
			{
				s.Format("Could not find system with name \"%s\"\nPlease check your StartShips.data file!", systemName);
				AfxMessageBox(s);
			}
		}
	}
	else
		AfxMessageBox("ERROR! Could not open file \"StartShips.data\"...");
	// Datei schlie�en
	file.Close();
}


void CBotf2Doc::ApplyTroopsAtStartup()
{
	CString fileName = CIOData::GetInstance()->GetAppPath() + "Data\\Troops\\StartTroops.data";
	CStdioFile file;
	if (file.Open(fileName, CFile::modeRead | CFile::typeText))// Datei StartTroops.data �ffnen um Start Truppen zu definieren
	{
		CString csInput;
		while (file.ReadString(csInput))
		{
			if (csInput.Left(2) == "//" || csInput.IsEmpty())
				continue;
			int pos = 0;
			CString s = csInput.Tokenize(":", pos);
			CString systemName = s;
			// Systemnamen auf der Map suchen
			BOOLEAN found = FALSE;
			for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			{
				for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName() == s)//Wenn der Name gefunden wurde
					{
						while (pos < csInput.GetLength())
						{
							// ID des Geb�udes holen
							s = csInput.Tokenize(",", pos);
							BuildTroop(atoi(s), CPoint(x,y));
						}
						found = TRUE;
						break;
					}
				if (found)
					break;
			}
			// Wurde das System nicht gefunden, sprich es ist ein Fehler in der Datei
			if (!found)
			{
				s.Format("Could not find system with name \"%s\"\nPlease check your StartTroops.data file!", systemName);
				AfxMessageBox(s);
			}
		}
	}
	else
		AfxMessageBox("ERROR! Could not open file \"StartTroops.data\"...");
	// Datei schlie�en
	file.Close();
}



void CBotf2Doc::ApplyBuildingsAtStartup()
{
	// Name des zu �ffnenden Files
	CString fileName = CIOData::GetInstance()->GetAppPath() + "Data\\Buildings\\StartBuildings.data";
	CStdioFile file;
	// Datei wird ge�ffnet
	if (file.Open(fileName, CFile::modeRead | CFile::typeText))
	{
		// auf csInput wird die jeweilige Zeile gespeichert
		CString csInput;
		while (file.ReadString(csInput))
		{
			if (csInput.Left(2) == "//" || csInput.IsEmpty())
				continue;
			int pos = 0;
			CString s = csInput.Tokenize(":", pos);
			CString systemName = s;
			// Systemnamen auf der Map suchen
			BOOLEAN found = FALSE;
			for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			{
				for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName() == s)
					{
						while (pos < csInput.GetLength())
						{
							// ID des Geb�udes holen
							s = csInput.Tokenize(",", pos);
							BuildBuilding(atoi(s), CPoint(x,y));
						}
						found = TRUE;
						break;
					}
				if (found)
					break;
			}
			// Wurde das System nicht gefunden, sprich es ist ein Fehler in der Datei
			if (!found)
			{
				s.Format("Could not find system with name \"%s\"\nPlease check your StartBuildings.data file!", systemName);
				AfxMessageBox(s);
			}
		}
	}
	else
		AfxMessageBox("ERROR! Could not open file \"StartBuildings.data\"...");
	// Datei schlie�en
	file.Close();

	// testweise mal in allen Systemen alles berechnen
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	// Starmaps berechnen, sofern diese noch nicht existieren
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		pMajor->CreateStarmap();
	}
	// Anomalien beachten (ist f�r jede Starmap gleich, daher statisch)
	CStarmap::SynchronizeWithAnomalies(m_Sectors);

	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem() == TRUE)
			{
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetHabitants(m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetCurrentHabitants());
				for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
				{
					if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == it->first)
					{
						CMajor* pMajor = it->second;
						ASSERT(pMajor);
						// Anzahl aller Farmen, Bauh�fe usw. im System berechnen
						// baubare Geb�ude, Schiffe und Truppen berechnen
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateNumberOfWorkbuildings(&this->BuildingInfo);
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetWorkersIntoBuildings();
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateVariables(&this->BuildingInfo, pMajor->GetEmpire()->GetResearch()->GetResearchInfo(), m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanets(), pMajor, CTrade::GetMonopolOwner());
						// alle produzierten FP und SP der Imperien berechnen und zuweisen
						int currentPoints;
						currentPoints = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetResearchProd();
						pMajor->GetEmpire()->AddFP(currentPoints);
						currentPoints = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetSecurityProd();
						pMajor->GetEmpire()->AddSP(currentPoints);
						// Schiffsunterst�tzungskosten eintragen
						float fCurrentHabitants = m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetCurrentHabitants();
						pMajor->GetEmpire()->AddPopSupportCosts((USHORT)fCurrentHabitants * POPSUPPORT_MULTI);
					}
				}
				for (int i = 0; i < m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAllBuildings()->GetSize(); i++)
				{
					USHORT nID = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAllBuildings()->GetAt(i).GetRunningNumber();
					CString sRaceID = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem();
					if (GetBuildingInfo(nID).GetMaxInEmpire() > 0)
						m_GlobalBuildings.AddGlobalBuilding(sRaceID, nID);
				}
			}

	this->CalcNewRoundData();
	this->CalcShipEffects();

	// hier das Schiffinformationsfeld durchgehen und in die WeaponObserver-Klasse des jeweiligen Imperiums
	// die baubaren Waffen eintragen. Wir brauchen dies um selbst Schiffe designen zu k�nnen
	// Dies gilt nur f�r Majorsraces.
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		BYTE researchLevels[6] =
		{
			pMajor->GetEmpire()->GetResearch()->GetBioTech(),
			pMajor->GetEmpire()->GetResearch()->GetEnergyTech(),
			pMajor->GetEmpire()->GetResearch()->GetCompTech(),
			pMajor->GetEmpire()->GetResearch()->GetPropulsionTech(),
			pMajor->GetEmpire()->GetResearch()->GetConstructionTech(),
			pMajor->GetEmpire()->GetResearch()->GetWeaponTech()
		};

		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
			if (m_ShipInfoArray.GetAt(i).GetRace() == pMajor->GetRaceShipNumber())
			{
				// nur aktuell baubare Schiffe d�rfen �berpr�ft werden, wenn wir die Beamwaffen checken
				if (m_ShipInfoArray.GetAt(i).IsThisShipBuildableNow(researchLevels))
				{
					// Wenn die jeweilige Rasse dieses technologisch bauen k�nnte, dann Waffen des Schiffes checken
					pMajor->GetWeaponObserver()->CheckBeamWeapons(&m_ShipInfoArray.GetAt(i));
					pMajor->GetWeaponObserver()->CheckTorpedoWeapons(&m_ShipInfoArray.GetAt(i));
				}
			}

		// Systemliste erstellen und baubare Geb�ude, Schiffe und Truppen berechnen
		pMajor->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
		for (int i = 0; i < pMajor->GetEmpire()->GetSystemList()->GetSize(); i++)
		{
			CPoint p = pMajor->GetEmpire()->GetSystemList()->GetAt(i).ko;
			m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableBuildings(&m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &BuildingInfo, pMajor, &m_GlobalBuildings);
			m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableShips(this, p);
			m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableTroops(&m_TroopInfo, pMajor->GetEmpire()->GetResearch());
		}
	}
}

void CBotf2Doc::ReadTroopInfosFromFile()
{
//Neu: Truppen werden aus Datei gelesen
	CString fileName = CIOData::GetInstance()->GetAppPath() + "Data\\Troops\\Troops.data";
	CStdioFile file;
	// Datei wird ge�ffnet
	if (file.Open(fileName, CFile::modeRead | CFile::typeText))
	{
		CString data[22];
		CString csInput;
		int i=0;
		CTroopInfo* troopInfo;
		m_TroopInfo.RemoveAll();
		while (file.ReadString(csInput))
		{
			if (csInput.Left(2) == "//" || csInput.IsEmpty())
				continue;
			// Daten lesen
			data[i++] = csInput;
			if (i == 22)
			{
				i = 0;
				BYTE techs[6]={atoi(data[7]),atoi(data[8]),atoi(data[9]),atoi(data[10]),atoi(data[11]),atoi(data[12])};
				USHORT res[5] = {atoi(data[13]),atoi(data[14]),atoi(data[15]),atoi(data[16]),atoi(data[17])};
				troopInfo = new CTroopInfo(CResourceManager::GetString(data[1]), CResourceManager::GetString(data[2]),data[3],atoi(data[4]),atoi(data[5]),atoi(data[6]),techs,res,atoi(data[18]),atoi(data[19]),data[0].GetString(),atoi(data[20]),atoi(data[21]));
				m_TroopInfo.Add(*troopInfo);
				delete troopInfo;
			}
		}

	}
	else
	{
		AfxMessageBox("ERROR! Could not open file \"Troops.data\"...");
		exit(1);
	}
	file.Close();
}


void CBotf2Doc::ReadBuildingInfosFromFile()
{
	for (int i = 0; i < BuildingInfo.GetSize(); )
		BuildingInfo.RemoveAt(i);
	BuildingInfo.RemoveAll();
	CBuildingInfo info;
	CString csInput;
	CString data[140];
	CString fileName = CIOData::GetInstance()->GetAppPath() + "Data\\Buildings\\Buildings.data";		// Name des zu �ffnenden Files
	CStdioFile file;													// Varibale vom Typ CStdioFile
	if (file.Open(fileName, CFile::modeRead | CFile::typeBinary))			// Datei wird ge�ffnet
	{
		USHORT i = 0;
		while (file.ReadString(csInput))
		{
			// Daten lesen
			data[i++] = csInput;
			if (i == 140)
			{
				i = 0;
				info.SetRunningNumber(atoi(data[0]));
				info.SetOwnerOfBuilding(atoi(data[1]));
				info.SetBuildingName(data[2]);
				info.SetBuildingDescription(data[4]);
				info.SetUpgradeable(atoi(data[6]));
				info.SetGraphikFileName(data[7]);
				info.SetMaxInSystem(atoi(data[8]),atoi(data[9]));
				info.SetMaxInEmpire(atoi(data[10]),atoi(data[11]));
				if (atoi(data[10]) > 0 && atoi(data[11]) != atoi(data[0]))
				{
					CString s;
					s.Format("Error in Buildings.data: Building \"%s\": ID from \"max in empire\" has to be the same as the building id", info.GetBuildingName());
					AfxMessageBox(s);
				}
				info.SetOnlyHomePlanet(atoi(data[12]));
				info.SetOnlyOwnColony(atoi(data[13]));
				info.SetOnlyMinorRace(atoi(data[14]));
				info.SetOnlyTakenSystem(atoi(data[15]));
				info.SetOnlyInSystemWithName(data[16]);
				info.SetMinHabitants(atoi(data[17]));
				info.SetMinInSystem(atoi(data[18]),atoi(data[19]));
				info.SetMinInEmpire(atoi(data[20]),atoi(data[21]));
				if (atoi(data[20]) > 0)
				{
					CString s;
					s.Format("Error in Buildings.data: Building \"%s\": \"Min in empire\" is not supported in this version!", info.GetBuildingName());
					AfxMessageBox(s);
				}
				info.SetOnlyRace(atoi(data[22]));
				info.SetPlanetTypes(PLANETCLASS_A,atoi(data[23]));
				info.SetPlanetTypes(PLANETCLASS_B,atoi(data[24]));
				info.SetPlanetTypes(PLANETCLASS_C,atoi(data[25]));
				info.SetPlanetTypes(PLANETCLASS_E,atoi(data[26]));
				info.SetPlanetTypes(PLANETCLASS_F,atoi(data[27]));
				info.SetPlanetTypes(PLANETCLASS_G,atoi(data[28]));
				info.SetPlanetTypes(PLANETCLASS_H,atoi(data[29]));
				info.SetPlanetTypes(PLANETCLASS_I,atoi(data[30]));
				info.SetPlanetTypes(PLANETCLASS_J,atoi(data[31]));
				info.SetPlanetTypes(PLANETCLASS_K,atoi(data[32]));
				info.SetPlanetTypes(PLANETCLASS_L,atoi(data[33]));
				info.SetPlanetTypes(PLANETCLASS_M,atoi(data[34]));
				info.SetPlanetTypes(PLANETCLASS_N,atoi(data[35]));
				info.SetPlanetTypes(PLANETCLASS_O,atoi(data[36]));
				info.SetPlanetTypes(PLANETCLASS_P,atoi(data[37]));
				info.SetPlanetTypes(PLANETCLASS_Q,atoi(data[38]));
				info.SetPlanetTypes(PLANETCLASS_R,atoi(data[39]));
				info.SetPlanetTypes(PLANETCLASS_S,atoi(data[40]));
				info.SetPlanetTypes(PLANETCLASS_T,atoi(data[41]));
				info.SetPlanetTypes(PLANETCLASS_Y,atoi(data[42]));
				info.SetBioTech(atoi(data[43]));
				info.SetEnergyTech(atoi(data[44]));
				info.SetCompTech(atoi(data[45]));
				info.SetPropulsionTech(atoi(data[46]));
				info.SetConstructionTech(atoi(data[47]));
				info.SetWeaponTech(atoi(data[48]));
				info.SetNeededIndustry(atoi(data[49]));
				info.SetNeededEnergy(atoi(data[50]));
				info.SetNeededTitan(atoi(data[51]));
				info.SetNeededDeuterium(atoi(data[52]));
				info.SetNeededDuranium(atoi(data[53]));
				info.SetNeededCrystal(atoi(data[54]));
				info.SetNeededIridium(atoi(data[55]));
				info.SetNeededDeritium(atoi(data[56]));
				info.SetFoodProd(atoi(data[57]));
				info.SetIPProd(atoi(data[58]));
				info.SetEnergyProd(atoi(data[59]));
				info.SetSPProd(atoi(data[60]));
				info.SetFPProd(atoi(data[61]));
				info.SetTitanProd(atoi(data[62]));
				info.SetDeuteriumProd(atoi(data[63]));
				info.SetDuraniumProd(atoi(data[64]));
				info.SetCrystalProd(atoi(data[65]));
				info.SetIridiumProd(atoi(data[66]));
				info.SetDeritiumProd(atoi(data[67]));
				info.SetCreditsProd(atoi(data[68]));
				info.SetMoralProd(atoi(data[69]));
				info.SetMoralProdEmpire(atoi(data[70]));
				info.SetFoodBoni(atoi(data[71]));
				info.SetIndustryBoni(atoi(data[72]));
				info.SetEnergyBoni(atoi(data[73]));
				info.SetSecurityBoni(atoi(data[74]));
				info.SetResearchBoni(atoi(data[75]));
				info.SetTitanBoni(atoi(data[76]));
				info.SetDeuteriumBoni(atoi(data[77]));
				info.SetDuraniumBoni(atoi(data[78]));
				info.SetCrystalBoni(atoi(data[79]));
				info.SetIridiumBoni(atoi(data[80]));
				info.SetDeritiumBoni(atoi(data[81]));
				info.SetAllRessourceBoni(atoi(data[82]));
				info.SetCreditsBoni(atoi(data[83]));
				info.SetBioTechBoni(atoi(data[84]));
				info.SetEnergyTechBoni(atoi(data[85]));
				info.SetCompTechBoni(atoi(data[86]));
				info.SetPropulsionTechBoni(atoi(data[87]));
				info.SetConstructionTechBoni(atoi(data[88]));
				info.SetWeaponTechBoni(atoi(data[89]));
				info.SetInnerSecurityBoni(atoi(data[90]));
				info.SetEconomySpyBoni(atoi(data[91]));
				info.SetEconomySabotageBoni(atoi(data[92]));
				info.SetResearchSpyBoni(atoi(data[93]));
				info.SetResearchSabotageBoni(atoi(data[94]));
				info.SetMilitarySpyBoni(atoi(data[95]));
				info.SetMilitarySabotageBoni(atoi(data[96]));
				info.SetShipYard(atoi(data[97]));
				info.SetBuildableShipTypes((SHIP_SIZE::Typ)atoi(data[98]));
				info.SetShipYardSpeed(atoi(data[99]));
				info.SetBarrack(atoi(data[100]));
				info.SetBarrackSpeed(atoi(data[101]));
				info.SetHitpoints(atoi(data[102]));
				info.SetShieldPower(atoi(data[103]));
				info.SetShieldPowerBoni(atoi(data[104]));
				info.SetShipDefend(atoi(data[105]));
				info.SetShipDefendBoni(atoi(data[106]));
				info.SetGroundDefend(atoi(data[107]));
				info.SetGroundDefendBoni(atoi(data[108]));
				info.SetScanPower(atoi(data[109]));
				info.SetScanPowerBoni(atoi(data[110]));
				info.SetScanRange(atoi(data[111]));
				info.SetScanRangeBoni(atoi(data[112]));
				info.SetShipTraining(atoi(data[113]));
				info.SetTroopTraining(atoi(data[114]));
				info.SetResistance(atoi(data[115]));
				info.SetAddedTradeRoutes(atoi(data[116]));
				info.SetIncomeOnTradeRoutes(atoi(data[117]));
				info.SetShipRecycling(atoi(data[118]));
				info.SetBuildingBuildSpeed(atoi(data[119]));
				info.SetUpdateBuildSpeed(atoi(data[120]));
				info.SetShipBuildSpeed(atoi(data[121]));
				info.SetTroopBuildSpeed(atoi(data[122]));
				info.SetPredecessor(atoi(data[123]));
				info.SetAllwaysOnline(atoi(data[124]));
				info.SetWorker(atoi(data[125]));
				info.SetNeverReady(atoi(data[126]));
				info.SetEquivalent(0,0);		// niemand-index immer auf NULL setzen
				for (int p = HUMAN; p <= DOMINION; p++)
					info.SetEquivalent(p,atoi(data[126+p]));
				for (int res = TITAN; res <= DERITIUM; res++)
					info.SetResourceDistributor(res, atoi(data[133+res]));
				info.SetNeededSystems(atoi(data[139]));

				// Information in das Geb�udeinfofeld schreiben
				BuildingInfo.Add(info);
			}
		}
	}
	else
	{
		AfxMessageBox("ERROR! Could not open file \"Buildings.data\"...");
		exit(1);
	}
	// Datei wird geschlossen
	file.Close();

	/*	for (int i = 0; i < BuildingInfo.GetSize(); i++)
	{
		CString test;
		test.Format("FoodProd: %i\nName: %s\nGrafikdatei: %s\nlaufende Nummer: %i\nn�tige Biotechstufe: %i\nn�tige Energietechstufe %i\nn�tige Computertechstufe %i\nn�tige Antriebstechstufe %i\nn�tige Konstruktionstechstufe %i\nn�tige Waffentechstufe %i\nKlasse K: %i\nKlasse M: %i\nKlasse O: %i\nVorg�nger: %i\nArbeiter: %i"
			,BuildingInfo.GetAt(i).GetFoodProd(),
			BuildingInfo.GetAt(i).GetBuildingName(),
			BuildingInfo.GetAt(i).GetGraphikFileName(),
			BuildingInfo.GetAt(i).GetRunningNumber(),
			BuildingInfo.GetAt(i).GetBioTech(),
			BuildingInfo.GetAt(i).GetEnergyTech(),
			BuildingInfo.GetAt(i).GetCompTech(),
			BuildingInfo.GetAt(i).GetPropulsionTech(),
			BuildingInfo.GetAt(i).GetConstructionTech(),
			BuildingInfo.GetAt(i).GetWeaponTech(),
			BuildingInfo.GetAt(i).GetPlanetTypes(K),
			BuildingInfo.GetAt(i).GetPlanetTypes(M),
			BuildingInfo.GetAt(i).GetPlanetTypes(O),
			BuildingInfo.GetAt(i).GetPredecessorID(),
			BuildingInfo.GetAt(i).GetWorker());
			AfxMessageBox(test);
	}*/
}

// Funktion lie�t aus der Datei die Informationen zu allen Schiffen ein und speichert diese im dynamischen Feld
// sp�ter k�nnen wir ingame diese Informationen �ndern und uns unsere eigenen Schiffchen bauen
void CBotf2Doc::ReadShipInfosFromFile()
{
	CShipInfo ShipInfo;
	int i = 0;
	USHORT j = 0;
	int weapons = 0;	// Z�hler um Waffen hinzuzuf�gen
	BOOL torpedo = FALSE;
	CString csInput;													// auf csInput wird die jeweilige Zeile gespeichert
	CString data[40];
	CString torpedoData[9];
	CString beamData[12];
	CString fileName = CIOData::GetInstance()->GetAppPath() + "Data\\Ships\\Shiplist.data";				// Name des zu �ffnenden Files
	CStdioFile file;														// Varibale vom Typ CStdioFile
	if (file.Open(fileName, CFile::modeRead | CFile::typeBinary))			// Datei wird ge�ffnet
	{
		while (file.ReadString(csInput))
		{
			if (csInput != "$END_OF_SHIPDATA$")
			{
				if (csInput == "$Torpedo$")
				{
					weapons = 9;	// weil wir 9 Informationen f�r einen Torpedo brauchen
					torpedo = TRUE;
				}
				else if (csInput == "$Beam$")
				{
					weapons = 12;	// weil wir 12 Informationen f�r einen Beam brauchen
					torpedo = FALSE;
				}
				else if (torpedo == TRUE && weapons > 0)
				{
					torpedoData[9-weapons] = csInput;
					weapons--;
					if (weapons == 0)
					{
						// Torpedodaten hinzuf�gen
						CTorpedoWeapons torpedoWeapon;
						torpedoWeapon.ModifyTorpedoWeapon(atoi(torpedoData[0]),atoi(torpedoData[1]),
							atoi(torpedoData[2]),atoi(torpedoData[3]),torpedoData[4],atoi(torpedoData[5]),atoi(torpedoData[6]));

						// folgende Zeile neu in Alpha5
						torpedoWeapon.GetFirearc()->SetValues(atoi(torpedoData[7]), atoi(torpedoData[8]));

						ShipInfo.GetTorpedoWeapons()->Add(torpedoWeapon);
					}
				}
				else if (torpedo == FALSE && weapons > 0)
				{
					beamData[12-weapons] = csInput;
					weapons--;
					if (weapons == 0)
					{
						// Beamdaten hinzuf�gen
						CBeamWeapons beamWeapon;
						beamWeapon.ModifyBeamWeapon(atoi(beamData[0]),atoi(beamData[1]),atoi(beamData[2]),
							beamData[3],atoi(beamData[4]),atoi(beamData[5]),atoi(beamData[6]),atoi(beamData[7]),atoi(beamData[8]),atoi(beamData[9]));
						// folgende Zeile neu in Alpha5
						beamWeapon.GetFirearc()->SetValues(atoi(beamData[10]), atoi(beamData[11]));

						ShipInfo.GetBeamWeapons()->Add(beamWeapon);
					}
				}
				else
				{
					data[i] = csInput;
					i++;
				}
			}
			else
			{
				// sonstige Informationen an das Objekt �bergeben
				ShipInfo.SetRace(atoi(data[0]));
				// ALPHA5	-> Alle Minorraceschiffe haben im Editor noch die Nummer UNKNOWN.
				//			-> diese Schiffe werden nun auf die MINORNUMBER gesetzt
				if (ShipInfo.GetRace() == UNKNOWN)
					ShipInfo.SetRace(MINORNUMBER);
				ShipInfo.SetID(j);
				ShipInfo.SetShipClass(data[1]);
				ShipInfo.SetShipDescription(data[2]);
				ShipInfo.SetShipType((SHIP_TYPE::Typ)atoi(data[3]));
				ShipInfo.SetShipSize((SHIP_SIZE::Typ)atoi(data[4]));
				ShipInfo.SetManeuverability(atoi(data[5]));
				ShipInfo.SetBioTech(atoi(data[6]));
				ShipInfo.SetEnergyTech(atoi(data[7]));
				ShipInfo.SetComputerTech(atoi(data[8]));
				ShipInfo.SetPropulsionTech(atoi(data[9]));
				ShipInfo.SetConstructionTech(atoi(data[10]));
				ShipInfo.SetWeaponTech(atoi(data[11]));
				ShipInfo.SetNeededIndustry(atoi(data[12]));
				ShipInfo.SetNeededTitan(atoi(data[13]));
				ShipInfo.SetNeededDeuterium(atoi(data[14]));
				ShipInfo.SetNeededDuranium(atoi(data[15]));
				ShipInfo.SetNeededCrystal(atoi(data[16]));
				ShipInfo.SetNeededIridium(atoi(data[17]));
				ShipInfo.SetNeededDeritium(atoi(data[18]));
				ShipInfo.SetOnlyInSystem(data[19]);
				ShipInfo.GetHull()->ModifyHull(atoi(data[22]),atoi(data[20]),atoi(data[21]),atoi(data[23]),atoi(data[24]));
				ShipInfo.GetShield()->ModifyShield(atoi(data[25]),atoi(data[26]),atoi(data[27]));
				ShipInfo.SetSpeed(atoi(data[28]));
				ShipInfo.SetRange((SHIP_RANGE::Typ)atoi(data[29]));
				ShipInfo.SetScanPower(atoi(data[30]));
				ShipInfo.SetScanRange(atoi(data[31]));
				ShipInfo.SetStealthPower(atoi(data[32]));
				ShipInfo.SetStorageRoom(atoi(data[33]));
				ShipInfo.SetColonizePoints(atoi(data[34]));
				ShipInfo.SetStationBuildPoints(atoi(data[35]));
				ShipInfo.SetMaintenanceCosts(atoi(data[36]));
				ShipInfo.SetSpecial(0, (SHIP_SPECIAL::Typ)atoi(data[37]));
				ShipInfo.SetSpecial(1, (SHIP_SPECIAL::Typ)atoi(data[38]));
				ShipInfo.SetObsoleteShipClass(data[39]);
				ShipInfo.CalculateFinalCosts();
				ShipInfo.SetStartOrder();
				m_ShipInfoArray.Add(ShipInfo);
				ShipInfo.DeleteWeapons();
				i = 0;
				j++;
			}
		}
	}
	else
	{
		AfxMessageBox("ERROR! Could not open file \"Shiplist.data\"...");
		exit(1);
	}
	// Datei wird geschlossen
	file.Close();

	// gibt es mehrere Majors mit dem gleichen Shipset dann m�ssen die Schiffe dupliziert werden und dem Feld
	// nochmals hinzugef�gt werden. Dabei wird die Schiffsbaurasse auf die Rasse mit dem doppelten Shipset gesetzt.
	int nAdd = 1;
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		for (map<CString, CMajor*>::iterator jt = pmMajors->begin(); jt != pmMajors->end(); ++jt)
			if (it->first != jt->first && it->second->GetRaceShipNumber() == jt->second->GetRaceShipNumber())
			{
				jt->second->SetRaceShipNumber(pmMajors->size() + nAdd);
				// nun alle Schiffe mit der Raceshipnumber des ersten Majors f�r den zweiten Major anh�ngen
				for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
					if (m_ShipInfoArray.GetAt(i).GetRace() == it->second->GetRaceShipNumber())
					{
						CShipInfo shipInfo = m_ShipInfoArray[i];
						shipInfo.SetRace(pmMajors->size() + nAdd);
						shipInfo.SetID(m_ShipInfoArray.GetSize());
						m_ShipInfoArray.Add(shipInfo);
					}
				nAdd++;
			}
}

// Sp�ter noch hinzuf�gen, dass auch andere Rassen bauen k�nnen
void CBotf2Doc::BuildBuilding(USHORT id, const CPoint& KO)
{
	CBuilding building(id);
	BOOLEAN isOnline = this->GetBuildingInfo(id).GetAllwaysOnline();
	building.SetIsBuildingOnline(isOnline);
	m_Systems.at(KO.x+(KO.y)*STARMAP_SECTORS_HCOUNT).AddNewBuilding(building);
}

void CBotf2Doc::BuildShip(int nID, const CPoint& KO, const CString& sOwnerID)
{
	CRace* pOwner = m_pRaceCtrl->GetRace(sOwnerID);
	if (!pOwner)
	{
		AfxMessageBox("Error in BuildShip(): RaceID " + sOwnerID + " doesn't exist!");
		return;
	}

	ASSERT(nID >= 10000);
	nID -= 10000;

	// unbedingt Kopie machen, da die Refernz verlorengeht, sobald ein neues Schiff hinzugef�gt wurde
	CString sOwner = sOwnerID;
	m_ShipArray.Add((CShip)m_ShipInfoArray.GetAt(nID));
	int n = m_ShipArray.GetUpperBound();
	m_ShipArray[n].SetOwnerOfShip(sOwner);
	m_ShipArray[n].SetKO(KO);

	// Schiffsnamen vergeben
	if (m_ShipArray.GetAt(n).GetShipType() != SHIP_TYPE::OUTPOST && m_ShipArray.GetAt(n).GetShipType() != SHIP_TYPE::STARBASE)
		m_ShipArray.ElementAt(n).SetShipName(m_GenShipName.GenerateShipName(sOwner, FALSE));
	else
		m_ShipArray.ElementAt(n).SetShipName(m_GenShipName.GenerateShipName(sOwner, TRUE));

	// den Rest nur machen, wenn das Schiff durch eine Majorrace gebaut wurde
	if (pOwner->GetType() != MAJOR)
		return;

	CMajor* pMajor = dynamic_cast<CMajor*>(pOwner);
	ASSERT(pMajor);

	// Spezialforschungsboni dem Schiff hinzuf�gen
	AddSpecialResearchBoniToShip(&m_ShipArray[n], pMajor);

	for (int i = 0; i < 4; i++)
		m_ShipArray.ElementAt(n).SetTargetKO(CPoint(-1,-1), i);
	pMajor->GetShipHistory()->AddShip(&m_ShipArray.GetAt(n), m_Sectors.at(KO.x+(KO.y)*STARMAP_SECTORS_HCOUNT).GetName(), m_iRound);
}

void CBotf2Doc::RemoveShip(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_ShipArray.GetSize());

	CShip* pShip = &m_ShipArray[nIndex];
	if (pShip->GetFleet())
	{
		// alten Befehl merken
		SHIP_ORDER::Typ nOldOrder = pShip->GetCurrentOrder();
		// Kopie der Flotte holen
		CFleet* pFleetCopy = pShip->GetFleet();
		// erstes Schiff aus der Flotte holen
		CShip* pFleetShip = pFleetCopy->GetShipFromFleet(0);
		// f�r dieses eine Flotte erstellen
		pFleetShip->CreateFleet();
		for (USHORT i = 1; i < pFleetCopy->GetFleetSize(); i++)
			pFleetShip->GetFleet()->AddShipToFleet(pFleetCopy->GetShipFromFleet(i));
		pFleetShip->CheckFleet();
		// alten Befehl �bergeben
		pFleetShip->SetCurrentOrder(nOldOrder);

		m_ShipArray.Add(*pFleetShip);
		// Schiff nochmal neu holen, da der Vektor ver�ndert wurde und so sich auch der Zeiger �ndern kann
		pShip = &m_ShipArray[nIndex];
		// Flotte l�schen
		pShip->DeleteFleet();
	}
	m_ShipArray.RemoveAt(nIndex);
}

/// Funktion beachtet die erforschten Spezialforschungen einer Rasse und verbessert die
/// Eigenschaften der �bergebenen Schiffes.
/// @param pShip Schiff welches durch Spezialforschungen eventuell verbessert wird
/// @param pShipOwner Zeiger auf den Besitzer des Schiffes
void CBotf2Doc::AddSpecialResearchBoniToShip(CShip* pShip, CMajor* pShipOwner) const
{
	if (!pShip || !pShipOwner)
		return;

	CResearchInfo* pInfo = pShipOwner->GetEmpire()->GetResearch()->GetResearchInfo();
	if (!pInfo)
		return;

	// m�gliche Verbesserungen durch die Spezialforschung werden hier beachtet
	// Spezialforschung #0: "Waffentechnik"
	if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetComplexStatus() == RESEARCH_STATUS::RESEARCHED)
	{
		// 20% erhoehter Phaserschaden
		if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			for (int i = 0; i < pShip->GetBeamWeapons()->GetSize(); i++)
			{
				USHORT oldPower = pShip->GetBeamWeapons()->GetAt(i).GetBeamPower();
				pShip->GetBeamWeapons()->GetAt(i).SetBeamPower(oldPower + (oldPower * pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetBonus(1) / 100));
			}
		}
		// 20% erhoehte Torpedogenauigkeit
		else if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			for (int i = 0; i < pShip->GetTorpedoWeapons()->GetSize(); i++)
			{
				BYTE oldAcc = pShip->GetTorpedoWeapons()->GetAt(i).GetAccuracy();
				pShip->GetTorpedoWeapons()->GetAt(i).SetAccuracy(oldAcc + (oldAcc * pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetBonus(2) / 100));
			}
		}
		// 20% erhoehte Schussfreuquenz
		else if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		{
			for (int i = 0; i < pShip->GetBeamWeapons()->GetSize(); i++)
			{
				BYTE oldRate = pShip->GetBeamWeapons()->GetAt(i).GetRechargeTime();
				pShip->GetBeamWeapons()->GetAt(i).SetRechargeTime(oldRate	- (oldRate * pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetBonus(3) / 100));
			}
			for (int i = 0; i < pShip->GetTorpedoWeapons()->GetSize(); i++)
			{
				BYTE oldRate = pShip->GetTorpedoWeapons()->GetAt(i).GetTupeFirerate();
				pShip->GetTorpedoWeapons()->GetAt(i).SetTubeFirerate(oldRate - (oldRate * pInfo->GetResearchComplex(RESEARCH_COMPLEX::WEAPONS_TECHNOLOGY)->GetBonus(3) / 100));
			}
		}
	}
	// Spezialforschung #1: "Konstruktionstechnik"
	if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetComplexStatus() == RESEARCH_STATUS::RESEARCHED)
	{
		// 20% bessere Schilde
		if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			UINT maxShield = pShip->GetShield()->GetMaxShield();
			BYTE shieldType = pShip->GetShield()->GetShieldType();
			BOOLEAN regenerative = pShip->GetShield()->GetRegenerative();
			pShip->GetShield()->ModifyShield((maxShield + (maxShield * pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetBonus(1) / 100)), shieldType, regenerative);
		}
		// 20% bessere H�lle
		else if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			BOOLEAN doubleHull = pShip->GetHull()->GetDoubleHull();
			BOOLEAN ablative = pShip->GetHull()->GetAblative();
			BOOLEAN polarisation = pShip->GetHull()->GetPolarisation();
			UINT baseHull = pShip->GetHull()->GetBaseHull();
			BYTE hullMaterial = pShip->GetHull()->GetHullMaterial();
			pShip->GetHull()->ModifyHull(doubleHull, (baseHull + (baseHull * pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetBonus(2) / 100)), hullMaterial,ablative,polarisation);
		}
		// 50% st�rkere Scanner
		else if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		{
			USHORT scanPower = pShip->GetScanPower();
			pShip->SetScanPower(scanPower + (scanPower * pInfo->GetResearchComplex(RESEARCH_COMPLEX::CONSTRUCTION_TECHNOLOGY)->GetBonus(3) / 100));
		}
	}
	// Spezialforschung #2: "allgemeine Schiffstechnik"
	if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::GENERAL_SHIP_TECHNOLOGY)->GetComplexStatus() == RESEARCH_STATUS::RESEARCHED)
	{
		// erhoehte Reichweite f�r Schiffe mit zuvor kurzer Reichweite
		if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::GENERAL_SHIP_TECHNOLOGY)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			if (pShip->GetRange() == SHIP_RANGE::SHORT)
				pShip->SetRange((SHIP_RANGE::Typ)(pInfo->GetResearchComplex(RESEARCH_COMPLEX::GENERAL_SHIP_TECHNOLOGY)->GetBonus(1)));
		}
		// erhoehte Geschwindigkeit f�r Schiffe mit Geschwindigkeit 1
		else if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::GENERAL_SHIP_TECHNOLOGY)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			if (pShip->GetSpeed() == 1)
				pShip->SetSpeed((BYTE)(pInfo->GetResearchComplex(RESEARCH_COMPLEX::GENERAL_SHIP_TECHNOLOGY)->GetBonus(2)));
		}
	}
	// Spezialforschung #3: "friedliche Schiffstechnik"
	if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::PEACEFUL_SHIP_TECHNOLOGY)->GetComplexStatus() == RESEARCH_STATUS::RESEARCHED && pShip->GetShipType() <= SHIP_TYPE::COLONYSHIP)
	{
		// 25% erhoehte Transportkapazitaet
		if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::PEACEFUL_SHIP_TECHNOLOGY)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			USHORT storage = pShip->GetStorageRoom();
			pShip->SetStorageRoom(storage + (storage * pInfo->GetResearchComplex(RESEARCH_COMPLEX::PEACEFUL_SHIP_TECHNOLOGY)->GetBonus(1) / 100));
		}
		// keine Unterhaltskosten
		if (pInfo->GetResearchComplex(RESEARCH_COMPLEX::PEACEFUL_SHIP_TECHNOLOGY)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			pShip->SetMaintenanceCosts(0);
		}
	}
}

void CBotf2Doc::AddToLostShipHistory(const CShip* pShip, const CString& sEvent, const CString& sStatus)
{
	CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(pShip->GetOwnerOfShip()));
	if (pMajor)
	{
		pMajor->GetShipHistory()->ModifyShip(pShip,	m_Sectors.at(pShip->GetKO().x+(pShip->GetKO().y)*STARMAP_SECTORS_HCOUNT).GetName(TRUE), m_iRound, sEvent, sStatus);
	}
}

/// Die Truppe mit der ID <code>ID</code> wird im System mit der Koordinate <code>ko</code> gebaut.
void CBotf2Doc::BuildTroop(BYTE ID, CPoint ko)
{
	// Mal Testweise paar Truppen anlegen
	m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).AddTroop((CTroop*)&m_TroopInfo.GetAt(ID));
	CString sRace = m_Sectors.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector();
	if (sRace == "")
		return;

	CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sRace));
	ASSERT(pMajor);

	int n = m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetTroops()->GetUpperBound();

	// Spezialforschung #4: "Truppen"
	if (pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TROOPS)->GetComplexStatus() == RESEARCH_STATUS::RESEARCHED)
	{
		// 20% verbesserte Offensive
		if (pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TROOPS)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			BYTE power = m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetTroops()->GetAt(n).GetOffense();
			m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetTroops()->GetAt(n).SetOffense(
				power + (power * pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TROOPS)->GetBonus(1) / 100));
		}
		// 500 Erfahrungspunkte dazu -> erste Stufe
		else if (pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TROOPS)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			m_Systems.at(ko.x+(ko.y)*STARMAP_SECTORS_HCOUNT).GetTroops()->GetAt(n).AddExperiancePoints(pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TROOPS)->GetBonus(2));
		}
	}

}

// Funktion generiert die Starmaps, so wie sie nach Rundenberechnung auch angezeigt werden k�nnen.
void CBotf2Doc::GenerateStarmap(const CString& sOnlyForRaceID)
{
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	// Starmaps aller Majors l�schen und neu anlegen
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		if (sOnlyForRaceID != "" && sOnlyForRaceID != it->first)
			continue;

		CMajor* pMajor = it->second;
		pMajor->CreateStarmap();
	}
	// Anomalien beachten (ist f�r jede Starmap gleich, daher statisch)
	CStarmap::SynchronizeWithAnomalies(m_Sectors);

	// Starmaps generieren
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
			{
				if (sOnlyForRaceID != "" && sOnlyForRaceID != it->first)
					continue;

				CMajor* pMajor = it->second;
				// Wenn in dem System eine aktive Werft ist bzw. eine Station/Werft im Sektor ist
				if ((m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetShipYard() == TRUE && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == pMajor->GetRaceID())
					|| m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetShipPort(pMajor->GetRaceID()))
				{
					pMajor->GetStarmap()->AddBase(Sector(x,y), pMajor->GetEmpire()->GetResearch()->GetPropulsionTech());
				}

				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
				{
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == it->first || m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == "")
					{
						CMajor* pMajor = it->second;
						pMajor->GetStarmap()->AddKnownSystem(Sector(x,y));
					}
				}
			}
		}
	// Jetzt die Starmap abgleichen, das wir nicht auf Gebiete fliegen k�nnen, wenn wir einen NAP mit einer Rasse haben
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		if (sOnlyForRaceID != "" && sOnlyForRaceID != it->first)
			continue;

		set<CString> NAPRaces;
		for (map<CString, CMajor*>::const_iterator itt = pmMajors->begin(); itt != pmMajors->end(); ++itt)
			if (it->first != itt->first && it->second->GetAgreement(itt->first) == DIPLOMATIC_AGREEMENT::NAP)
				NAPRaces.insert(itt->first);
		// interne Starmap f�r KI syncronisieren
		it->second->GetStarmap()->SynchronizeWithMap(m_Sectors, &NAPRaces);
	}

	// nun die Berechnung f�r den Au�enpostenbau vornehmen
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		if (sOnlyForRaceID != "" && sOnlyForRaceID != it->first)
			continue;

		CMajor* pMajor = it->second;
		if (!pMajor->IsHumanPlayer())
			pMajor->GetStarmap()->SetBadAIBaseSectors(m_Sectors, it->first);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBotf2Doc Diagnose

#ifdef _DEBUG
void CBotf2Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBotf2Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBotf2Doc Befehle

/////////////////////////////////////////////////////////////////////////////
// private Funktionen
/////////////////////////////////////////////////////////////////////////////

/// Diese Funktion f�hrt allgemeine Berechnung durch, die immer zu Beginn der NextRound-Calculation stattfinden
/// m�ssen. So werden z.B. alte Nachrichten gel�scht, die Statistiken berechnet usw..
void CBotf2Doc::CalcPreDataForNextRound()
{
	m_iRound++;

	ASSERT(GetPlayersRace());

	// Berechnungen der neuen Runde
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		// Alle alten Nachrichten  aus letzter Runde l�schen
		pMajor->GetEmpire()->ClearMessagesAndEvents();
		// Bev�lkerungsunterst�tzungskosten auf NULL setzen
		pMajor->GetEmpire()->SetPopSupportCosts(0);
		// verbleibende Vertragsdauer mit anderen Majorraces berechnen und gegebenenfalls Nachrichten und diplomatische Auswirkungen anwenden
		network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
		if (pMajor->DecrementAgreementsDuration(pmMajors))
			if (pMajor->IsHumanPlayer())
				m_iSelectedView[client] = EMPIRE_VIEW;
		// wird das Imperium von einem Menschen oder vom Computer gespielt
		if (client != network::RACE_NONE && server.IsPlayedByClient(client))
			pMajor->SetHumanPlayer(true);
		else
			pMajor->SetHumanPlayer(false);
	}

	// Schiffe, welche nur auf einem bestimmten System baubar sind, z.B. Schiffe von Minorraces, den Besitzer wieder
	// auf MINORNUMBER setzen. In der Funktion, welche in einem System die baubaren Schiffe berechnet, wird dieser
	// Wert dann auf die richtige Rasse gesetzt. Jeder der das System dann besitzt, kann dieses Schiff bauen
	for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
		if (!m_ShipInfoArray.GetAt(i).GetOnlyInSystem().IsEmpty())
			m_ShipInfoArray.GetAt(i).SetRace(MINORNUMBER);

	// Systeme durchgehen und die Blockadewert des Systems zur�cksetzen
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem())
			{
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetBlockade(0);
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).ClearDisabledProductions();
			}


	//f(x):=min(731,max(14,trunc(743-x^3)))
	m_fStardate += (float)(min(731, max(14, 743-pow((float)m_Statistics.GetAverageTechLevel(),3.0f))));
}

/// Diese Funktion berechnet den kompletten Systemangriff.
void CBotf2Doc::CalcSystemAttack()
{
	// Systemangriff durchf�hren
	// Set mit allen Minors, welche w�hrend eines Systemangriffs vernichtet wurden. Diese werden am Ende der
	// Berechnung aus der Liste entfernt
	set<CString> sKilledMinors;
	CArray<CPoint> fightInSystem;
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
		if (m_ShipArray.GetAt(y).GetCurrentOrder() == SHIP_ORDER::ATTACK_SYSTEM)
		{
			BOOLEAN okay = TRUE;
			// Checken dass in diesem System nicht schon ein Angriff durchgef�hrt wurde
			for (int x = 0; x < fightInSystem.GetSize(); x++)
				if (fightInSystem.GetAt(x) == m_ShipArray.GetAt(y).GetKO())
				{
					okay = FALSE;
					break;
				}

			// nur wenn das Schiff und Schiffe in der Flotte ungetarnt sind
			if (m_ShipArray[y].GetCloak() == TRUE || (m_ShipArray.GetAt(y).GetFleet() != 0 && m_ShipArray.GetAt(y).GetFleet()->CheckOrder(&m_ShipArray.GetAt(y), SHIP_ORDER::ATTACK_SYSTEM) == FALSE))
			{
				m_ShipArray.ElementAt(y).SetCurrentOrder(SHIP_ORDER::ATTACK);
				okay = FALSE;
			}

			// wenn in dem System noch kein Angriff durchgef�hrt wurde kann angegriffen werden
			if (okay)
			{
				CPoint p = m_ShipArray.GetAt(y).GetKO();
				// Besitzer des Systems (hier Sector wegen Minors) vor dem Systemangriff
				CString sDefender = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector();
				// Angreifer bzw. neuer Besitzer des Systems nach dem Angriff
				set<CString> attackers;
				for (int i = 0; i < m_ShipArray.GetSize(); i++)
					if (m_ShipArray.GetAt(i).GetKO() == p && m_ShipArray.GetAt(i).GetCurrentOrder() == SHIP_ORDER::ATTACK_SYSTEM)
					{
						CString sOwner = m_ShipArray.GetAt(i).GetOwnerOfShip();
						if (!sOwner.IsEmpty() && m_pRaceCtrl->GetRace(sOwner)->GetType() == MAJOR)
							attackers.insert(sOwner);
					}

				CAttackSystem* attackSystem = new CAttackSystem();

				CRace* defender = NULL;
				if (!sDefender.IsEmpty())
					defender = m_pRaceCtrl->GetRace(sDefender);
				// Wenn eine Minorrace in dem System lebt und dieser nicht schon erobert wurde
				if (defender && defender->GetType() == MINOR && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetTakenSector() == FALSE)
				{
					attackSystem->Init(defender, &m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &m_ShipArray, &m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &this->BuildingInfo, CTrade::GetMonopolOwner());
				}
				// Wenn eine Majorrace in dem System lebt
				else if (defender && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(sDefender, &attackers))
				{
					attackSystem->Init(defender, &m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &m_ShipArray, &m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &this->BuildingInfo, CTrade::GetMonopolOwner());
				}
				// Wenn niemand mehr in dem System lebt, z.B. durch Rebellion
				else
				{
					attackSystem->Init(NULL, &m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &m_ShipArray, &m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &this->BuildingInfo, CTrade::GetMonopolOwner());
				}
				// Ein Systemangriff verringert die Moral in allen Systemen, die von uns schon erobert wurden und zuvor
				// der Majorrace geh�rten, deren System hier angegriffen wird
				if (!sDefender.IsEmpty())
					for (int j = 0 ; j < STARMAP_SECTORS_VCOUNT; j++)
						for (int i = 0; i < STARMAP_SECTORS_HCOUNT; i++)
							if (m_Sectors.at(i+(j)*STARMAP_SECTORS_HCOUNT).GetTakenSector() == TRUE && m_Sectors.at(i+(j)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() == sDefender
								&& attackSystem->IsDefenderNotAttacker(sDefender, &attackers))
								m_Systems.at(i+(j)*STARMAP_SECTORS_HCOUNT).SetMoral(-rand()%5);

				// Wurde das System mit Truppen erobert, so wechselt es den Besitzer
				if (attackSystem->Calculate())
				{
					CString attacker = m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem();
					CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(attacker));
					ASSERT(pMajor);
					//* Der Besitzer des Systems wurde in der Calculate() Funktion schon auf den neuen Besitzer
					//* umgestellt. Der Besitzer des Sektors ist aber noch der alte, wird hier dann auf einen
					//* eventuell neuen Besitzer umgestellt.

					// Wenn in dem System eine Minorrace beheimatet ist und das System nicht vorher schon von jemand
					// anderem milit�risch erobert wurde oder die Minorace bei einem anderen Imperium schon vermitgliedelt
					// wurde, dann muss diese zuerst die Geb�ude bauen
					if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMinorRace() == TRUE && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetTakenSector() == FALSE && defender != NULL && defender->GetType() == MINOR)
					{
						CMinor* pMinor = dynamic_cast<CMinor*>(defender);
						ASSERT(pMinor);
						pMinor->SetSubjugated(true);
						// Wenn das System noch keiner Majorrace geh�rt, dann Geb�ude bauen
						m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).BuildBuildingsForMinorRace(&m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT), &BuildingInfo, m_Statistics.GetAverageTechLevel(), pMinor);
						// Sektor gilt ab jetzt als erobert.
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(TRUE);
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwned(TRUE);
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector(attacker);
						// Beziehung zu dieser Minorrace verschlechtert sich auf 0 Punkte
						pMinor->SetRelation(attacker, -100);
						// Moral in diesem System verschlechtert sich um rand()%25+10 Punkte
						m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(-rand()%25-10);
						// ist die Moral unter 50, so wird sie auf 50 gesetzt
						if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral() < 50)
							m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(50 - m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral());
						CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
						CString eventText = "";

						// Alle diplomatischen Nachrichten der Minorrace aus den Feldern l�schen und an der Minorrace
						// bekannte Imperien die Nachricht der Unterwerfung senden
						pMinor->GetIncomingDiplomacyNews()->clear();
						pMinor->GetOutgoingDiplomacyNews()->clear();
						map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
						for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
						{
							// ausgehende Nachrichten l�schen
							for (UINT i = 0; i < it->second->GetOutgoingDiplomacyNews()->size(); i++)
								if (it->second->GetOutgoingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
									|| it->second->GetOutgoingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
									it->second->GetOutgoingDiplomacyNews()->erase(it->second->GetOutgoingDiplomacyNews()->begin() + i--);
							// eingehende Nachrichten l�schen
							for (UINT i = 0; i < it->second->GetIncomingDiplomacyNews()->size(); i++)
								if (it->second->GetIncomingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
									|| it->second->GetIncomingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
									it->second->GetIncomingDiplomacyNews()->erase(it->second->GetIncomingDiplomacyNews()->begin() + i--);

							// An alle Majors die die Minor kennen die Nachricht schicken, dass diese unterworfen wurde
							if (it->second->IsRaceContacted(pMinor->GetRaceID()))
							{
								CMessage message;
								message.GenerateMessage(CResourceManager::GetString("MINOR_SUBJUGATED", FALSE, pMinor->GetRaceName()), MESSAGE_TYPE::MILITARY, param, p, 0);
								it->second->GetEmpire()->AddMessage(message);
								if (it->second->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
						}
						// Eventnachricht an den Eroberer (System erobert)
						eventText = pMajor->GetMoralObserver()->AddEvent(11, pMajor->GetRaceMoralNumber(), param);
						// Eventnachricht hinzuf�gen
						if (!eventText.IsEmpty())
						{
							CMessage message;
							message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0, 1);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer())
							{
								network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
						}
					}
					// Wenn das System einer Minorrace geh�rt, eingenommen wurde und somit befreit wird
					else if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMinorRace() == TRUE && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetTakenSector() == TRUE && defender != NULL && defender->GetType() == MAJOR)
					{
						// Die Beziehung zur der Majorrace, die das System vorher besa� verschlechtert sich
						defender->SetRelation(attacker, -rand()%50);
						// Die Beziehung zu der Minorrace verbessert sich auf Seiten des Retters
						CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName());
						ASSERT(pMinor);
						pMinor->SetRelation(attacker, rand()%50);
						pMinor->SetSubjugated(false);
						// Eventnachricht an den, der das System verloren hat (erobertes Minorracesystem wieder verloren)
						CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
						CString eventText = "";

						CMajor* def = dynamic_cast<CMajor*>(defender);
						eventText = def->GetMoralObserver()->AddEvent(17, def->GetRaceMoralNumber(), param);
						// Eventnachricht hinzuf�gen
						if (!eventText.IsEmpty())
						{
							CMessage message;
							message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
							def->GetEmpire()->AddMessage(message);
							if (def->IsHumanPlayer())
							{
								network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
						}
						// Eventnachricht an den Eroberer (Minorracesystem befreit)
						param = pMinor->GetRaceName();
						eventText = pMajor->GetMoralObserver()->AddEvent(13, pMajor->GetRaceMoralNumber(), param);
						// Eventnachricht hinzuf�gen
						if (!eventText.IsEmpty())
						{
							CMessage message;
							message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer())
							{
								network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
						}
						// Sektor gilt ab jetzt als nicht mehr erobert.
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(FALSE);
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwned(FALSE);
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector(pMinor->GetRaceID());
						m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSystem("");
						// Moral in dem System um rand()%50+25 erh�hen
						m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(rand()%50+25);
					}
					// Eine andere Majorrace besa� das System
					else
					{
						// Beziehung zum ehemaligen Besitzer verschlechtert sich
						if (defender != NULL && defender->GetRaceID() != attacker)
							defender->SetRelation(attacker, -rand()%50);
						// Wenn das System zur�ckerobert wird, dann gilt es als befreit
						if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() == attacker)
						{
							m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(FALSE);
							CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
							// Eventnachricht an den Eroberer (unser ehemaliges System wieder zur�ckerobert)
							CString eventText = "";
							eventText = pMajor->GetMoralObserver()->AddEvent(14, pMajor->GetRaceMoralNumber(), param);
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0, 1);
								pMajor->GetEmpire()->AddMessage(message);
								if (pMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
							if (defender != NULL && defender->GetRaceID() != attacker && defender->GetType() == MAJOR)
							{
								CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
								// Eventnachricht an den, der das System verloren hat (unser erobertes System verloren)
								eventText = pDefenderMajor->GetMoralObserver()->AddEvent(17, pDefenderMajor->GetRaceMoralNumber(), param);
								// Eventnachricht hinzuf�gen
								if (!eventText.IsEmpty())
								{
									CMessage message;
									message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
									pDefenderMajor->GetEmpire()->AddMessage(message);
									if (pDefenderMajor->IsHumanPlayer())
									{
										network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
										m_iSelectedView[client] = EMPIRE_VIEW;
									}
								}
							}
							// Moral in dem System um rand()%25+10 erh�hen
							m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(rand()%25+10);
						}
						// Handelte es sich dabei um das Heimatsystem einer Rasse
						else if (defender != NULL && defender->GetType() == MAJOR && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == defender->GetRaceID() && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName() == dynamic_cast<CMajor*>(defender)->GetHomesystemName())
						{
							CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
							// Eventnachricht an den ehemaligen Heimatsystembesitzer (Heimatsystem verloren)
							CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
							CString eventText = "";
							eventText = pDefenderMajor->GetMoralObserver()->AddEvent(15, pDefenderMajor->GetRaceMoralNumber(), param);
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
								pDefenderMajor->GetEmpire()->AddMessage(message);
								if (pDefenderMajor->IsHumanPlayer())
								{
										network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
										m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
							// Eventnachricht an den Eroberer (System erobert)
							eventText = "";
							eventText = pMajor->GetMoralObserver()->AddEvent(11, pMajor->GetRaceMoralNumber(), param);
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0, 1);
								pMajor->GetEmpire()->AddMessage(message);
								if (pMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
							// Sektor gilt ab jetzt als erobert.
							m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(TRUE);
							// Moral in diesem System verschlechtert sich um rand()%25+10 Punkte
							m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(-rand()%25-10);
							// ist die Moral unter 50, so wird sie auf 50 gesetzt
							if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral() < 50)
								m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(50 - m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral());
						}
						// wurde das System erobert und obere Bedingungen traten nicht ein
						else
						{
							CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
							// Hat eine andere Majorrace die Minorrace vermitgliedelt, so unterwerfen wir auch diese Minorrace
							if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMinorRace())
							{
								CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName());
								ASSERT(pMinor);
								pMinor->SetSubjugated(true);
								// Beziehung zu dieser Minorrace verschlechtert sich auf 0 Punkte
								pMinor->SetRelation(attacker, -100);

								CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();

								// Alle diplomatischen Nachrichten der Minorrace aus den Feldern l�schen und an der Minorrace
								// bekannte Imperien die Nachricht der Unterwerfung senden
								pMinor->GetIncomingDiplomacyNews()->clear();
								pMinor->GetOutgoingDiplomacyNews()->clear();
								map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
								for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
								{
									// ausgehende Nachrichten l�schen
									for (UINT i = 0; i < it->second->GetOutgoingDiplomacyNews()->size(); i++)
										if (it->second->GetOutgoingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
											|| it->second->GetOutgoingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
											it->second->GetOutgoingDiplomacyNews()->erase(it->second->GetOutgoingDiplomacyNews()->begin() + i--);
									// eingehende Nachrichten l�schen
									for (UINT i = 0; i < it->second->GetIncomingDiplomacyNews()->size(); i++)
										if (it->second->GetIncomingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
											|| it->second->GetIncomingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
											it->second->GetIncomingDiplomacyNews()->erase(it->second->GetIncomingDiplomacyNews()->begin() + i--);

									// An alle Majors die die Minor kennen die Nachricht schicken, dass diese unterworden wurde
									if (it->second->IsRaceContacted(pMinor->GetRaceID()))
									{
										CMessage message;
										message.GenerateMessage(CResourceManager::GetString("MINOR_SUBJUGATED", FALSE, pMinor->GetRaceName()), MESSAGE_TYPE::MILITARY, param, p, 0);
										it->second->GetEmpire()->AddMessage(message);
										if (it->second->IsHumanPlayer())
										{
											network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
											m_iSelectedView[client] = EMPIRE_VIEW;
										}
									}
								}
							}

							// Sektor gilt ab jetzt als erobert.
							m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(TRUE);
							// Moral in diesem System verschlechtert sich um rand()%25+10 Punkte
							m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(-rand()%25-10);
							// ist die Moral unter 50, so wird sie auf 50 gesetzt
							if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral() < 50)
								m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMoral(50 - m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMoral());

							CString eventText = "";
							// Eventnachricht an den ehemaligen Besitzer (eigenes System verloren)

							//We later were dereferencing defender anyway; after casting to CMajor.
							//Not sure whether defender should be allowed to be NULL here.
							assert(defender);
							if (defender->GetRaceID() != attacker && defender->GetType() == MAJOR) {
								CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
								eventText = pDefenderMajor->GetMoralObserver()->AddEvent(16, pDefenderMajor->GetRaceMoralNumber(), param);
							}
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
								//defender seems to be of type MAJOR here for sure ?
								assert(defender->GetType() == MAJOR);
								CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
								assert(pDefenderMajor);
								pDefenderMajor->GetEmpire()->AddMessage(message);
								if (pDefenderMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}

							// Eventnachricht an den Eroberer (System erobert)
							eventText = "";
							eventText = pMajor->GetMoralObserver()->AddEvent(11, pMajor->GetRaceMoralNumber(), param);
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0, 1);
								pMajor->GetEmpire()->AddMessage(message);
								if (pMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
						}
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector(attacker);
					}
					// Geb�ude die nach Eroberung automatisch zerst�rt werden
					m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).RemoveSpecialRaceBuildings(&this->BuildingInfo);
					// Variablen berechnen und Geb�ude besetzen
					m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).CalculateNumberOfWorkbuildings(&this->BuildingInfo);
					m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetWorkersIntoBuildings();

					// war der Verteidiger eine Majorrace und wurde sie durch die Eroberung komplett ausgel�scht,
					// so bekommt der Eroberer einen kr�ftigen Moralschub
					if (defender != NULL && defender->GetType() == MAJOR && !attacker.IsEmpty() && pMajor && attackSystem->IsDefenderNotAttacker(sDefender, &attackers))
					{
						CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
						// Anzahl der noch verbleibenden Systeme berechnen
						pDefenderMajor->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
						// hat der Verteidiger keine Systeme mehr, so bekommt der neue Besitzer den Bonus
						if (pDefenderMajor->GetEmpire()->GetSystemList()->GetSize() == 0)
						{
							CString param = pDefenderMajor->GetRaceName();
							CString eventText = pMajor->GetMoralObserver()->AddEvent(0, pMajor->GetRaceMoralNumber(), param);
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0, 1);
								pMajor->GetEmpire()->AddMessage(message);
								if (pMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(attacker);
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
						}
					}

					// erfolgreiches Invasionsevent f�r den Angreifer einf�gen (sollte immer ein Major sein)
					if (!attacker.IsEmpty() && pMajor && pMajor->IsHumanPlayer())
						pMajor->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(attacker, "InvasionSuccess", CResourceManager::GetString("INVASIONSUCCESSEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("INVASIONSUCCESSEVENT_TEXT_" + pMajor->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));

					// Invasionsevent f�r den Verteidiger einf�gen
					if (defender != NULL && defender->GetType() == MAJOR && dynamic_cast<CMajor*>(defender)->IsHumanPlayer() && attackSystem->IsDefenderNotAttacker(sDefender, &attackers))
						dynamic_cast<CMajor*>(defender)->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(sDefender, "InvasionSuccess", CResourceManager::GetString("INVASIONSUCCESSEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("INVASIONSUCCESSEVENT_TEXT_" + defender->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));
				}
				// Wurde nur bombardiert, nicht erobert
				else
				{
					CString param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
					CString eventText = "";
					for (set<CString>::const_iterator it = attackers.begin(); it != attackers.end(); ++it)
					{
						CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(*it));
						ASSERT(pMajor);

						// Erstmal die Beziehung zu der Rasse verschlechtern, der das System geh�rt
						if (defender != NULL && defender->GetRaceID() != pMajor->GetRaceID())
							defender->SetRelation(pMajor->GetRaceID(), -rand()%10);
					}
					// Wenn die Bev�lkerung des Systems auf NULL geschrumpft ist, dann ist dieses System verloren
					if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetHabitants() <= 0.000001f)
					{
						// Bei einer Minorrace wird es komplizierter. Wenn diese keine Systeme mehr hat, dann ist diese
						// aus dem Spiel verschwunden. Alle Eintr�ge in der Diplomatie m�ssen daher gel�scht werden
						if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMinorRace())
						{
							CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName());
							ASSERT(pMinor);
							m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetMinorRace(FALSE);

							pMinor->GetIncomingDiplomacyNews()->clear();
							pMinor->GetOutgoingDiplomacyNews()->clear();
							map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
							for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
							{
								// ausgehende Nachrichten l�schen
								for (UINT i = 0; i < it->second->GetOutgoingDiplomacyNews()->size(); i++)
									if (it->second->GetOutgoingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
										|| it->second->GetOutgoingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
										it->second->GetOutgoingDiplomacyNews()->erase(it->second->GetOutgoingDiplomacyNews()->begin() + i--);
								// eingehende Nachrichten l�schen
								for (UINT i = 0; i < it->second->GetIncomingDiplomacyNews()->size(); i++)
									if (it->second->GetIncomingDiplomacyNews()->at(i).m_sFromRace == pMinor->GetRaceID()
										|| it->second->GetIncomingDiplomacyNews()->at(i).m_sToRace == pMinor->GetRaceID())
										it->second->GetIncomingDiplomacyNews()->erase(it->second->GetIncomingDiplomacyNews()->begin() + i--);

								// An alle Majors die die Minor kennen die Nachricht schicken, dass diese vernichtet wurde
								// Eventnachricht: #21	Eliminate a Minor Race Entirely
								if (attackers.find(it->first) != attackers.end())
								{
									CString param = pMinor->GetRaceName();
									CString eventText = it->second->GetMoralObserver()->AddEvent(21, it->second->GetRaceMoralNumber(), param);
									CMessage message;
									message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
									it->second->GetEmpire()->AddMessage(message);
								}

								// alle anderen Majorrassen, die diese Minor kannten, bekommen eine Nachricht �ber deren Vernichtung
								if (pMinor->IsRaceContacted(it->first))
								{
									CString news = CResourceManager::GetString("ELIMINATE_MINOR", FALSE, pMinor->GetRaceName());
									CMessage message;
									message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "", 0, 0);
									it->second->GetEmpire()->AddMessage(message);
									if (it->second->IsHumanPlayer())
									{
										// Event �ber die Rassenausl�schung einf�gen
										CEventRaceKilled* eventScreen = new CEventRaceKilled(it->first, pMinor->GetRaceID(), pMinor->GetRaceName(), pMinor->GetGraphicFileName());
										it->second->GetEmpire()->GetEventMessages()->Add(eventScreen);

										network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
										m_iSelectedView[client] = EMPIRE_VIEW;
									}
								}
								// alle Majors durchgehen und die vernichtete Minor aus deren Maps entfernen
								CMajor* pMajor = it->second;
								pMajor->SetIsRaceContacted(pMinor->GetRaceID(), false);
								pMajor->SetAgreement(pMinor->GetRaceID(), DIPLOMATIC_AGREEMENT::NONE);
							}
							// Alle Schiffe der Minorrace entfernen
							for (int j = 0; j < m_ShipArray.GetSize(); j++)
								if (m_ShipArray.GetAt(j).GetOwnerOfShip() == pMinor->GetRaceID())
									m_ShipArray.RemoveAt(j--);

							// Rasse zum l�schen vormerken
							sKilledMinors.insert(pMinor->GetRaceID());
						}
						// Bei einer Majorrace verringert sich nur die Anzahl der Systeme (auch konnte dies das
						// Minorracesystem von oben gewesen sein, hier verliert es aber die betroffene Majorrace)
						if (defender != NULL && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(defender->GetRaceID(), &attackers))
						{
							CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
							eventText = "";
							if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName() == pDefenderMajor->GetHomesystemName())
							{
								// Eventnachricht an den ehemaligen Heimatsystembesitzer (Heimatsystem verloren)
								param = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName();
								eventText = pDefenderMajor->GetMoralObserver()->AddEvent(15, pDefenderMajor->GetRaceMoralNumber(), param);
							}
							else
							{
								// Eventnachricht an den ehemaligen Besitzer (eigenes System verloren)
								eventText = pDefenderMajor->GetMoralObserver()->AddEvent(16, pDefenderMajor->GetRaceMoralNumber(), param);
							}
							// Eventnachricht hinzuf�gen
							if (!eventText.IsEmpty())
							{
								CMessage message;
								message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
								pDefenderMajor->GetEmpire()->AddMessage(message);
								if (pDefenderMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
						}
						m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSystem("");
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector("");
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetColonyOwner("");
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(FALSE);
						m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOwned(FALSE);

						// war der Verteidiger eine Majorrace und wurde sie durch den Verlust des Systems komplett ausgel�scht,
						// so bekommt der Eroberer einen kr�ftigen Moralschub
						if (defender != NULL && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(defender->GetRaceID(), &attackers))
						{
							CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
							for (set<CString>::const_iterator it = attackers.begin(); it != attackers.end(); ++it)
							{
								CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(*it));
								ASSERT(pMajor);

								// Anzahl der noch verbleibenden Systeme berechnen
								pDefenderMajor->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
								// hat der Verteidiger keine Systeme mehr, so bekommt der neue Besitzer den Bonus
								if (pDefenderMajor->GetEmpire()->GetSystemList()->IsEmpty())
								{
									CString sParam		= pDefenderMajor->GetRaceName();
									CString sEventText	= pMajor->GetMoralObserver()->AddEvent(0, pMajor->GetRaceMoralNumber(), sParam);
									// Eventnachricht hinzuf�gen
									if (!sEventText.IsEmpty())
									{
										CMessage message;
										message.GenerateMessage(sEventText, MESSAGE_TYPE::MILITARY, sParam, p, 0, 1);
										pMajor->GetEmpire()->AddMessage(message);
										if (pMajor->IsHumanPlayer())
										{
											network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
											m_iSelectedView[client] = EMPIRE_VIEW;
										}
									}
								}
							}
						}
					}
					// Bombardierung hat die Rasse nicht komplett ausgel�scht
					else
					{
						// Eventnachrichten nicht jedesmal, sondern nur wenn Geb�ude vernichtet wurden oder
						// mindst. 3% der Bev�lkerung vernichtet wurden
						if (attackSystem->GetDestroyedBuildings() > 0 || attackSystem->GetKilledPop() >= m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetHabitants() * 0.03)
						{
							for (set<CString>::const_iterator it = attackers.begin(); it != attackers.end(); ++it)
							{
								CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(*it));
								ASSERT(pMajor);

								// Wenn das System nicht durch eine Rebellion verloren ging, sondern noch irgendwem geh�rt
								if (defender != NULL)
									eventText = pMajor->GetMoralObserver()->AddEvent(19, pMajor->GetRaceMoralNumber(), param);
								// Wenn das System mal uns geh�rt hatte und durch eine Rebellion verloren ging
								else if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() == pMajor->GetRaceID() && defender == NULL)
									eventText = pMajor->GetMoralObserver()->AddEvent(20, pMajor->GetRaceMoralNumber(), param);
								// Eventnachricht f�r Agressor hinzuf�gen
								if (!eventText.IsEmpty())
								{
									CMessage message;
									message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, param, p, 0);
									pMajor->GetEmpire()->AddMessage(message);
									if (pMajor->IsHumanPlayer())
									{
										network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
										m_iSelectedView[client] = EMPIRE_VIEW;
									}
								}
							}
							// Eventnachricht �ber Bombardierung f�r Verteidiger erstellen und hinzuf�gen
							if (defender != NULL && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(defender->GetRaceID(), &attackers))
							{
								CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
								eventText = pDefenderMajor->GetMoralObserver()->AddEvent(22, pDefenderMajor->GetRaceMoralNumber(), param);
								if (pDefenderMajor->IsHumanPlayer())
								{
									network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
									m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
						}
					}
					// Eventgrafiken hinzuf�gen
					// f�r den/die Angreifer
					for (set<CString>::const_iterator it = attackers.begin(); it != attackers.end(); ++it)
					{
						CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(*it));
						ASSERT(pMajor);

						// reine Bombardierung
						if (pMajor->IsHumanPlayer())
						{
							if (!attackSystem->IsTroopsInvolved())
								pMajor->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(pMajor->GetRaceID(), "Bombardment", CResourceManager::GetString("BOMBARDEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("BOMBARDEVENT_TEXT_AGRESSOR_" + pMajor->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));
							// gescheitere Invasion
							else if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetHabitants() > 0.000001f)
								pMajor->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(pMajor->GetRaceID(), "InvasionFailed", CResourceManager::GetString("INVASIONFAILUREEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("INVASIONFAILUREEVENT_TEXT_" + pMajor->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));
						}

					}
					// f�r den Verteidiger
					if (defender != NULL && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(defender->GetRaceID(), &attackers))
					{
						CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
						if (pDefenderMajor->IsHumanPlayer())
						{
							// reine Bombardierung
							if (!attackSystem->IsTroopsInvolved())
								pDefenderMajor->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(defender->GetRaceID(), "Bombardment", CResourceManager::GetString("BOMBARDEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("BOMBARDEVENT_TEXT_DEFENDER_" + defender->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));
							// gescheitere Invasion
							else if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetHabitants() > 0.000001f)
								pDefenderMajor->GetEmpire()->GetEventMessages()->Add(new CEventBombardment(defender->GetRaceID(), "InvasionFailed", CResourceManager::GetString("INVASIONFAILUREEVENT_HEADLINE", FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName()), CResourceManager::GetString("INVASIONFAILUREEVENT_TEXT_" + defender->GetRaceID(), FALSE, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName())));
						}
					}
				}

				// Nachrichten hinzuf�gen
				for (int i = 0; i < attackSystem->GetNews()->GetSize(); )
				{
					for (set<CString>::const_iterator it = attackers.begin(); it != attackers.end(); ++it)
					{
						CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(*it));
						ASSERT(pMajor);

						CMessage message;
						message.GenerateMessage(attackSystem->GetNews()->GetAt(i), MESSAGE_TYPE::MILITARY, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName(), p, 0);
						pMajor->GetEmpire()->AddMessage(message);
						if (pMajor->IsHumanPlayer())
						{
							network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
							m_iSelectedView[client] = EMPIRE_VIEW;
						}
					}
					if (defender != NULL && defender->GetType() == MAJOR && attackSystem->IsDefenderNotAttacker(defender->GetRaceID(), &attackers))
					{
						CMajor* pDefenderMajor = dynamic_cast<CMajor*>(defender);
						CMessage message;
						message.GenerateMessage(attackSystem->GetNews()->GetAt(i), MESSAGE_TYPE::MILITARY, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName(), p, 0);
						pDefenderMajor->GetEmpire()->AddMessage(message);
						if (pDefenderMajor->IsHumanPlayer())
						{
							network::RACE client = m_pRaceCtrl->GetMappedClientID(defender->GetRaceID());
							m_iSelectedView[client] = EMPIRE_VIEW;
						}
					}
					attackSystem->GetNews()->RemoveAt(i);
				}
				delete attackSystem;
				fightInSystem.Add(p);
			}
		}

	// alle vernichteten Minorraces nun aus dem Feld l�schen
	for (set<CString>::const_iterator it = sKilledMinors.begin(); it != sKilledMinors.end(); ++it)
		m_pRaceCtrl->RemoveRace(*it);

	// Schiffsfeld nochmal durchgehen und alle Schiffe ohne H�lle aus dem Feld entfernen.
	// Aufpassen mu� ich dabei, wenn das Schiff eine Flotte anf�hrte.
	if (fightInSystem.GetSize() > 0)
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
		{
			// Wenn das Schiff eine Flotte hatte, dann erstmal nur die Schiffe in der Flotte beachten
			// Wenn davon welche zerst�rt wurden diese aus der Flotte nehmen
			if (m_ShipArray.GetAt(i).GetFleet())
			{
				for (int x = 0; x < m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); x++)
					if (m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(x)->GetHull()->GetCurrentHull() < 1)
					{
						// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
						AddToLostShipHistory(m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(x), CResourceManager::GetString("SYSTEMATTACK"), CResourceManager::GetString("DESTROYED"));
						// Schiff entfernen
						m_ShipArray[i].GetFleet()->RemoveShipFromFleet(x--);
					}
				m_ShipArray[i].CheckFleet();
			}
			// Wenn das Schiff selbst zerst�rt wurde
			if (m_ShipArray.GetAt(i).GetHull()->GetCurrentHull() < 1)
			{
				// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
				AddToLostShipHistory(&m_ShipArray[i], CResourceManager::GetString("SYSTEMATTACK"), CResourceManager::GetString("DESTROYED"));
				// Schiff entfernen
				RemoveShip(i--);
			}
		}
}

/// Diese Funktion berechnet alles im Zusammenhang mit dem Geheimdienst.
void CBotf2Doc::CalcIntelligence()
{
	// der Geheimdienst muss vor der Forschung abgehandelt werden, da es durch Geheimdienstaktionen dazu kommen kann,
	// dass aktuell produzierte Forschungspunkte gestohlen werden. Diese werden dem Opfer abgezogen und dem Akteur
	// hinzugef�gt. Erst danach sollte die Forschung behandelt werden. Wird die Forschung zuvor aufgerufen macht es
	// spielmechanisch keinen Sinn.
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	// das Objekt f�r alle Berechnung mit Geheimdienstkontext anlegen
	CIntelCalc* intel = new CIntelCalc(this);
	if (intel)
	{
		// zuerst werden die ganzen Berechnungen durchgef�hrt, ohne das Punkte vorher von irgendeiner Rasse dazugerechnet werden.
		// Dadurch haben alle Rassen die selbe Chance.
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
			intel->StartCalc(it->second);
		// danach werden die Punkte dazuaddiert und zum Schluss werden die einzelnen Depotpunkte etwas reduziert.
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			intel->AddPoints(it->second);
			// die Intelpunkte in den Lagern werden ganz am Ende abgezogen.
			intel->ReduceDepotPoints(it->second);
			CIntelligence* pIntel = it->second->GetEmpire()->GetIntelligence();
			// Boni wieder l�schen, damit f�r die neue und n�chste Runde neue berechnet werden k�nnen
			pIntel->ClearBoni();
			// wenn neue Geheimdienstnachrichten vorhanden sind die Meldung im Impieriumsmen� erstellen
			if (pIntel->GetIntelReports()->IsReportAdded())
			{
				// die Sortierung der Geheimdienstberichte
				pIntel->GetIntelReports()->SortAllReports();

				CMessage message;
				message.GenerateMessage(CResourceManager::GetString("WE_HAVE_NEW_INTELREPORTS"), MESSAGE_TYPE::SECURITY, "", NULL, FALSE, 4);
				it->second->GetEmpire()->AddMessage(message);

				network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
				SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_INTELNEWS, client, 0, 1.0f};
				m_SoundMessages[client].Add(entry);

				BOOLEAN addSpy = FALSE;
				BOOLEAN addSab = FALSE;
				for (int j = pIntel->GetIntelReports()->GetAllReports()->GetUpperBound(); j >= 0; j--)
				{
					CIntelObject* intelObj = pIntel->GetIntelReports()->GetReport(j);
					if (intelObj->GetEnemy() != it->first && intelObj->GetRound() == this->GetCurrentRound())
					{
						CString eventText = "";
						if (intelObj->GetIsSpy() && !addSpy)
						{
							eventText = it->second->GetMoralObserver()->AddEvent(59, it->second->GetRaceMoralNumber());
							addSpy = TRUE;
						}
						else if (intelObj->GetIsSabotage() && !addSab)
						{
							eventText = it->second->GetMoralObserver()->AddEvent(60, it->second->GetRaceMoralNumber());
							addSab = TRUE;
						}
						// Eventnachricht hinzuf�gen
						if (!eventText.IsEmpty())
						{
							message.GenerateMessage(eventText, MESSAGE_TYPE::SECURITY, "", NULL, FALSE, 4);
							it->second->GetEmpire()->AddMessage(message);
						}
					}
					if (addSpy && addSab)
						break;
				}
			}
		}
		delete intel;
		intel = NULL;
	}

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		if (it->second->IsHumanPlayer())
			if (it->second->GetEmpire()->GetIntelligence()->GetIntelReports()->IsReportAdded())
			{
				network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
				m_iSelectedView[client] = EMPIRE_VIEW;
			}
}

/// Diese Funktion berechnet die Forschung eines Imperiums
void CBotf2Doc::CalcResearch()
{
	// Forschungsboni, die die Systeme machen holen. Wir ben�tigen diese dann f�r die CalculateResearch Funktion
	struct RESEARCHBONI { short nBoni[6]; };
	map<CString, RESEARCHBONI> researchBonis;
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() != "")
			{
				// hier Forschungsboni besorgen
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[0] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetBioTechBoni();
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[1] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetEnergyTechBoni();
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[2] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetCompTechBoni();
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[3] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetPropulsionTechBoni();
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[4] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetConstructionTechBoni();
				researchBonis[m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()].nBoni[5] += m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetWeaponTechBoni();
			}

	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;

		pMajor->GetEmpire()->GetResearch()->SetResearchBoni(researchBonis[it->first].nBoni);
		CString *news = 0;
		news = pMajor->GetEmpire()->GetResearch()->CalculateResearch(pMajor->GetEmpire()->GetFP());
		network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

		for (int j = 0; j < 8; j++)		// aktuell 8 verschiedene Nachrichten mgl, siehe CResearch Klasse
		{
			// Wenn irgendwas erforscht wurde, wir also eine Nachricht erstellen wollen
			if (news[j] != "")
			{
				CMessage message;

				// flag setzen, wenn wir eine Spezialforschung erforschen d�rfen
				if (j == 7)
				{
					// Spezialforschung kann erforscht werden
					if (pMajor->IsHumanPlayer())
					{
						SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_SCIENTISTNEWS, client, 0, 1.0f};
						m_SoundMessages[client].Add(entry);
						m_iSelectedView[client] = EMPIRE_VIEW;
					}
					message.GenerateMessage(news[j], MESSAGE_TYPE::RESEARCH, "", 0, FALSE, 1);
				}
				else
				{
					if (pMajor->IsHumanPlayer())
					{
						SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_NEWTECHNOLOGY, client, 0, 1.0f};
						m_SoundMessages[client].Add(entry);
						m_iSelectedView[client] = EMPIRE_VIEW;

						// Eventscreen f�r Forschung erstellen
						// gilt nur f�r die sechs normalen Forschungen
						if (j < 6)
							pMajor->GetEmpire()->GetEventMessages()->Add(new CEventResearch(pMajor->GetRaceID(), CResourceManager::GetString("RESEARCHEVENT_HEADLINE"), j));
					}
					message.GenerateMessage(news[j], MESSAGE_TYPE::RESEARCH, "", 0, FALSE);
				}

				pMajor->GetEmpire()->AddMessage(message);
			}
		}
	}

	// k�nstliche Intelligenz f�r Forschung
	CResearchAI AI;
	AI.Calc(this);
}

/// Diese Funktion berechnet die Auswirkungen von diplomatischen Angeboten und ob Minorraces Angebote an
/// Majorraces abgeben.
void CBotf2Doc::CalcDiplomacy()
{
	using namespace network;

	// zuerst alle Angebote senden
	CDiplomacyController::Send();

	// danach empfangen und reagieren
	CDiplomacyController::Receive();

	// Hinweis das Nachrichten eingegangen sind erstellen
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		if (pMajor->IsHumanPlayer() && pMajor->GetIncomingDiplomacyNews()->size() > 0)
		{
			network::RACE client = m_pRaceCtrl->GetMappedClientID(it->first);
			SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_DIPLOMATICNEWS, client, 0, 1.0f};
			m_SoundMessages[client].Add(entry);
			m_iSelectedView[client] = EMPIRE_VIEW;
		}
	}
}

/// Diese Funktion berechnet das Planetenwachstum, die Auftr�ge in der Bauliste und sonstige Einstellungen aus der
/// alten Runde.
void CBotf2Doc::CalcOldRoundData()
{
	m_GlobalBuildings.Reset();

	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		// hier setzen wir wieder die gesamten FP, SP und die gesamten Lager auf 0
		it->second->GetEmpire()->ClearAllPoints();
	}

	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			// M�gliche Is? Variablen f�r Terraforming und Stationbau erstmal auf FALSE setzen
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).ClearAllPoints();

			// Wenn im Sektor ein Sonnensystem existiert
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem() == TRUE)
			{
				// Jetzt das produzierte Credits im System dem jeweiligen Imperium geben
				if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() != "")
				{
					CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()));
					if (pMajor)
					{
						network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

						// spielt es der Computer, so bekommt er etwas mehr Credits
						if (pMajor->IsHumanPlayer() == false)
							pMajor->GetEmpire()->SetCredits((int)(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetCreditsProd() / m_fDifficultyLevel));
						else
							pMajor->GetEmpire()->SetCredits(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetCreditsProd());

						// Hier die Geb�ude abrei�en, die angeklickt wurden
						if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).DestroyBuildings())
							m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateNumberOfWorkbuildings(&this->BuildingInfo);

						// Variablen berechnen lassen, bevor der Planet w�chst -> diese ins Lager
						// nur berechnen, wenn das System auch jemandem geh�rt, ansonsten w�rde auch die Mind.Prod. ins Lager kommen
						// In CalculateStorages wird auch die Systemmoral berechnet. Wenn wir einen Auftrag mit
						// NeverReady (z.B. Kriegsrecht) in der Bauliste haben, und dieser Moral produziert, dann diese
						// auf die Moral anrechnen. Das wird dann in CalculateStorages gemacht.
						int list = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(0);
						if (list > 0 && list < 10000 && BuildingInfo[list-1].GetNeverReady() && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMoral() <= 85)
							m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->AddMoralProd(BuildingInfo[list-1].GetMoralProd());

						// KI Anpassungen (KI bekommt zuf�lig etwas Deritium geschenkt)
						int diliAdd = 0;
						if (pMajor->IsHumanPlayer() == false && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetDeritiumProd() == 0)
						{
							// umso h�her der Schwierigkeitsgrad, desto h�her die Wahrscheinlichkeit, das die KI
							// Deritium auf ihrem Systemen geschenkt bekommt
							int temp = rand()%((int)(m_fDifficultyLevel * 7.5));
							// TRACE("KI: System: %s - DiliAddProb: %d (NULL for adding Dili) - Difficulty: %.2lf\n",m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(), temp, m_fDifficultyLevel);
							if (temp == NULL)
								diliAdd = 1;
						}

						// Das Lager berechnen
						BOOLEAN bIsRebellion = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateStorages(pMajor->GetEmpire()->GetResearch()->GetResearchInfo(), diliAdd);
						// Wenn wir true zur�ckbekommen, dann hat sich das System losgesagt
						if (bIsRebellion)
						{
							m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwned(FALSE);
							m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetShipPort(FALSE, pMajor->GetRaceID());

							CString news = CResourceManager::GetString("REBELLION_IN_SYSTEM", FALSE, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
							CMessage message;
							message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "", CPoint(x,y), FALSE);
							pMajor->GetEmpire()->AddMessage(message);

							// zus�tzliche Eventnachricht (Lose a System to Rebellion #18) wegen der Moral an das Imperium
							message.GenerateMessage(pMajor->GetMoralObserver()->AddEvent(18, pMajor->GetRaceMoralNumber(), m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName()), MESSAGE_TYPE::SOMETHING, "", CPoint(x,y), FALSE);
							pMajor->GetEmpire()->AddMessage(message);

							if (pMajor->IsHumanPlayer())
								m_iSelectedView[client] = EMPIRE_VIEW;

							if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMinorRace() == TRUE)
							{
								CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
								if (pMinor)
								{
									m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector(pMinor->GetRaceID());

									if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetTakenSector() == FALSE)
									{
										pMinor->SetAgreement(pMajor->GetRaceID(), DIPLOMATIC_AGREEMENT::NONE);
										pMajor->SetAgreement(pMinor->GetRaceID(), DIPLOMATIC_AGREEMENT::NONE);

										pMinor->SetRelation(pMajor->GetRaceID(), (-(rand()%50+20)));
										news = CResourceManager::GetString("MINOR_CANCELS_MEMBERSHIP", FALSE, pMinor->GetRaceName());
										CMessage message;
										message.GenerateMessage(news, MESSAGE_TYPE::DIPLOMACY, "", CPoint(x,y), FALSE);
										pMajor->GetEmpire()->AddMessage(message);
									}
								}
							}
							else
							{
								m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector("");
								m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSystem("");
								m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(FALSE);
							}
						}

						// Hier mit einbeziehen, wenn die Bev�lkerung an Nahrungsmangel stirbt
						if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetFoodStore() < 0)
						{
							m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).LetPlanetsShrink((float)(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetFoodStore()) * 0.01f);
							// nur wenn die Moral �ber 50 ist sinkt die Moral durch Hungersn�te
							if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMoral() > 50)
								m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetMoral((short)(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetFoodStore() / (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetHabitants() + 1))); // +1, wegen Division durch NULL umgehen
							m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetFoodStore(0);

							CString news = CResourceManager::GetString("FAMINE", FALSE, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
							CMessage message;
							message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "", CPoint(x,y), FALSE, 1);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer())
								m_iSelectedView[client] = EMPIRE_VIEW;
						}
						else
							// Planetenwachstum f�r Spielerrassen durchf�hren
							m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).LetPlanetsGrowth();
					}
				}
				else
					// Planetenwachstum f�r andere Sektoren durchf�hren
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).LetPlanetsGrowth();

				if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() != "")
				{
					CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()));
					if (pMajor)
					{
						network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

						float fCurrentHabitants = m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetCurrentHabitants();
						pMajor->GetEmpire()->AddPopSupportCosts((USHORT)fCurrentHabitants * POPSUPPORT_MULTI);
						// Funktion gibt TRUE zur�ck, wenn wir durch die Bev�lkerung eine neue Handelsroute anlegen k�nnen
						if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetHabitants(fCurrentHabitants))
						{
							// wenn die Spezialforschung "mindestens 1 Handelsroute erforscht wurde, dann die Meldung erst bei
							// der 2.ten Handelroute bringen
							// Hier die Boni durch die Uniqueforschung "Handel" -> mindestens eine Handelsroute
							bool bMinOneRoute = pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED;
							if (bMinOneRoute == false || (bMinOneRoute == true && (int)(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetHabitants() / TRADEROUTEHAB) > 1))
							{
								CString news = CResourceManager::GetString("ENOUGH_HABITANTS_FOR_NEW_TRADEROUTE", FALSE, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
								CMessage message;
								message.GenerateMessage(news, MESSAGE_TYPE::ECONOMY, "", CPoint(x,y), FALSE, 4);
								pMajor->GetEmpire()->AddMessage(message);
								if (pMajor->IsHumanPlayer())
									m_iSelectedView[client] = EMPIRE_VIEW;
							}
						}
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateVariables(&this->BuildingInfo, pMajor->GetEmpire()->GetResearch()->GetResearchInfo(), m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanets(), pMajor, CTrade::GetMonopolOwner());

						// hier k�nnte die Energie durch Weltraummonster weggenommen werden!

						// Geb�ude die Energie ben�tigen checken
						if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CheckEnergyBuildings(&this->BuildingInfo))
						{
							CString news = CResourceManager::GetString("BUILDING_TURN_OFF",FALSE,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
							CMessage message;
							message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "", CPoint(x,y), FALSE, 2);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer())
								m_iSelectedView[client] = EMPIRE_VIEW;
						}

						// Die Bauauftr�ge in dem System berechnen. Au�erdem wird hier auch die System-KI ausgef�hrt.
						if (pMajor->IsHumanPlayer() == false || m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAutoBuild())
						{
							CSystemAI* SAI = new CSystemAI(this);
							SAI->ExecuteSystemAI(CPoint(x,y));
							delete SAI;
						}
						// Hier berechnen, wenn was in der Bauliste ist, und ob es fertig wird usw.
						if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(0) != 0)	// wenn was drin ist
						{
							int list = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(0);
							int IPProd = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetIndustryProd();
							// Wenn ein Auftrag in der Bauliste ist, der niemals fertig ist z.B. Kriegsrecht, dann dies
							// einzeln beachten
							if (list > 0 && list < 10000 && BuildingInfo[list-1].GetNeverReady() == TRUE)
							{
								// dann z�hlen die industriellen Baukosten des Auftrags als Anzahl Runden, wie lange
								// es in der Liste stehen bleibt und den Bonus gibt
								// Also wird immer nur ein IP abgezogen, au�er wenn die Moral gr��er gleich 80 ist,
								// dann wird der Auftrag gleich abgebrochen
								if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetMoral() >= 85)
									IPProd = (int)m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetNeededIndustryForBuild();
								else
									IPProd = 1;
							}
							// Wenn ein Update in der Liste ist die m�gliche UpdateBuildSpeed-Boni beachten
							else if (list < 0)
								IPProd = (int)floor((float)IPProd * (100 + m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetUpdateBuildSpeed()) / 100);
							// Wenn ein Geb�ude in der Liste ist die m�gliche BuildingBuildSpeed-Boni beachten
							else if (list < 10000)
								IPProd = (int)floor((float)IPProd * (100 + m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetBuildingBuildSpeed()) / 100);
							// Wenn es ein Schiff ist, dann die Effiziens der Werft und m�glichen ShipBuildSpeed-Boni beachten
							else if (list < 20000)
								IPProd = (int)floor((float)IPProd * m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetShipYardEfficiency() / 100
											* (100 + m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetShipBuildSpeed()) / 100);
							// Wenn es eine Truppe ist, dann Effiziens der Werft und m�glichen TroopBuildSpeed-Boni beachten
							else
								IPProd = (int)floor((float)IPProd * m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetBarrackEfficiency() / 100
											* (100 + m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetTroopBuildSpeed()) / 100);
							// Ein Bauauftrag ist fertig gestellt worden
							if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->CalculateBuildInAssemblyList(IPProd))
							{
								// Wenn Geb�ude gekauft wurde, dann die in der letzten Runde noch erbrachte IP-Leistung
								// den Credits des Imperiums gutschreiben, IP-Leistung darf aber nicht gr��er der Baukosten sein
								if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetWasBuildingBought() == TRUE)
								{
									int goods = IPProd;
									if (goods > m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetBuildCosts())
										goods = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetBuildCosts();
									pMajor->GetEmpire()->SetCredits(goods);
									m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->SetWasBuildingBought(FALSE);
								}
								// Ab jetzt die Abfrage ob Geb�ude oder ein Update fertig wurde
								if (list > 0 && list < 10000 && !BuildingInfo[list-1].GetNeverReady())	// Es wird ein Geb�ude gebaut
								{
									// Die Nachricht, dass neues Geb�ude fertig ist mit allen Daten generieren
									CMessage message;
									message.GenerateMessage(BuildingInfo[list-1].GetBuildingName(), MESSAGE_TYPE::ECONOMY, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(), CPoint(x,y), FALSE);
									pMajor->GetEmpire()->AddMessage(message);
									// Geb�ude bauen
									BuildBuilding(list, CPoint(x,y));
									// und Geb�ude (welches letztes im Feld) ist auch gleich online setzen, wenn
									// gen�gend Arbeiter da sind
									unsigned short CheckValue = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetNewBuildingOnline(&this->BuildingInfo);
									// Nachricht generierenm das das Geb�ude nicht online genommen werden konnte
									if (CheckValue == 1)
									{
										CString news = CResourceManager::GetString("NOT_ENOUGH_WORKER", FALSE, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
										message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "",CPoint(x,y), FALSE, 1);
										pMajor->GetEmpire()->AddMessage(message);
										if (pMajor->IsHumanPlayer())
											m_iSelectedView[client] = EMPIRE_VIEW;
									}
									else if (CheckValue == 2)
									{
										CString news = CResourceManager::GetString("NOT_ENOUGH_ENERGY", FALSE, m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
										message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "",CPoint(x,y), FALSE, 2);
										pMajor->GetEmpire()->AddMessage(message);
										if (pMajor->IsHumanPlayer())
											m_iSelectedView[client] = EMPIRE_VIEW;
									}
								}
								else if (list < 0)	// Es wird ein Update gemacht
								{
									list *= (-1);
									// Die Nachricht, dass neues Geb�udeupdate fertig wurde, mit allen Daten generieren
									CMessage message;
									message.GenerateMessage(BuildingInfo[list-1].GetBuildingName(),MESSAGE_TYPE::ECONOMY,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(),CPoint(x,y),TRUE);
									pMajor->GetEmpire()->AddMessage(message);
									// Vorg�nger von "list" holen
									// Geb�ude mit RunningNumbner == pre werden durch UpdateBuilding() gel�scht und
									// deren Anzahl wird zur�ckgegeben.
									USHORT pre = BuildingInfo[list-1].GetPredecessorID();
									int NumberOfNewBuildings = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).UpdateBuildings(pre);
									// So, nun bauen wir so viel mal das n�chste
									for (int z = 0; z < NumberOfNewBuildings; z++)
									{
										BuildBuilding(list,CPoint(x,y));
										// falls das geupgradete Geb�ude Energie ben�tigt wird versucht es gleich online zu setzen
										if (GetBuildingInfo(list).GetNeededEnergy() > NULL && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetNewBuildingOnline(&this->BuildingInfo) == 2)
										{
											CString news = CResourceManager::GetString("NOT_ENOUGH_ENERGY",FALSE,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
											message.GenerateMessage(news,MESSAGE_TYPE::SOMETHING,"",CPoint(x,y),FALSE,2);
											pMajor->GetEmpire()->AddMessage(message);
											if (pMajor->IsHumanPlayer())
												m_iSelectedView[client] = EMPIRE_VIEW;
										}
									}
								}
								else if (list >= 10000 && list < 20000)	// Es wird ein Schiff gebaut
								{
									BuildShip(list, CPoint(x,y), pMajor->GetRaceID());
									CString s = CResourceManager::GetString("SHIP_BUILDING_FINISHED",FALSE,
										m_ShipInfoArray[list-10000].GetShipTypeAsString(),m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
									CMessage message;
									message.GenerateMessage(s,MESSAGE_TYPE::MILITARY,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(),CPoint(x,y),FALSE);
									pMajor->GetEmpire()->AddMessage(message);
								}
								else if (list >= 20000)					// Es wird eine Truppe gebaut
								{
									BuildTroop(list-20000, CPoint(x,y));
									CString s = CResourceManager::GetString("TROOP_BUILDING_FINISHED",FALSE,
										m_TroopInfo[list-20000].GetName(),m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
									CMessage message;
									message.GenerateMessage(s,MESSAGE_TYPE::MILITARY,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName(),CPoint(x,y),FALSE);
									pMajor->GetEmpire()->AddMessage(message);
								}
								// Nach CalculateBuildInAssemblyList wird ClearAssemblyList() aufgerufen, wenn der Auftrag fertig wurde.
								// Normalerweise wird nach ClearAssemblyList() die Funktion CalculateVariables() aufgerufen, wegen Geld durch
								// Handelsg�ter wenn nix mehr drin steht. Hier mal testweise weggelassen, weil diese Funktion
								// sp�ter eh f�r das System aufgerufen wird und wir bis jetzt glaub ich keine Notwendigkeit
								// haben die Funktion CalculateVariables() aufzurufen.
								m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->ClearAssemblyList(CPoint(x,y), m_Systems);
								// Wenn die Bauliste nach dem letzten gebauten Geb�ude leer ist, eine Nachricht generieren
								if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(0) == 0)
								{
									CString news = CResourceManager::GetString("EMPTY_BUILDLIST",FALSE,m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetName());
									CMessage message;
									message.GenerateMessage(news,MESSAGE_TYPE::SOMETHING,"",CPoint(x,y),FALSE);
									pMajor->GetEmpire()->AddMessage(message);
									if (pMajor->IsHumanPlayer())
										m_iSelectedView[client] = EMPIRE_VIEW;
								}
							}
							m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->CalculateNeededRessourcesForUpdate(&BuildingInfo, m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAllBuildings(), pMajor->GetEmpire()->GetResearch()->GetResearchInfo());
						}
						// Anzahl aller Farmen, Bauh�fe usw. im System berechnen
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateNumberOfWorkbuildings(&this->BuildingInfo);
						// freie Arbeiter den Geb�uden zuweisen
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetWorkersIntoBuildings();
					}
				}

				// Jedes Sonnensystem wird durchgegangen und alle Geb�ude des Systems werden in die Variable
				// m_GlobalBuildings geschrieben. Damit wissen welche Geb�ude in der Galaxie stehen. Ben�tigt wird
				// dies z.B. um zu �berpr�fen, ob max. X Geb�ude einer bestimmten ID in einem Imperium stehen.
				for (int i = 0; i < m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAllBuildings()->GetSize(); i++)
				{
					USHORT nID = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAllBuildings()->GetAt(i).GetRunningNumber();
					CString sRaceID = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem();
					if (GetBuildingInfo(nID).GetMaxInEmpire() > 0)
						m_GlobalBuildings.AddGlobalBuilding(sRaceID, nID);
				}
				// Alle Geb�ude und Updates, die sich aktuell auch in der Bauliste befinden, werden dem Feld hinzugef�gt
				for (int i = 0; i < ALE; i++)
					if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(i) > 0 && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(i) < 10000)
					{
						USHORT nID = abs(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAssemblyList()->GetAssemblyListEntry(i));
						CString sRaceID = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem();
						if (GetBuildingInfo(nID).GetMaxInEmpire() > 0)
							m_GlobalBuildings.AddGlobalBuilding(sRaceID, nID);
					}
				}
		}
}

///////BEGINN: HELPER FUNCTIONS FOR CBotf2Doc::CalcNewRoundData()///////
void CBotf2Doc::CalcNewRoundDataPreLoop() {
	CSystemProd::ResetMoralEmpireWide();
	// Hier m�ssen wir nochmal die Systeme durchgehen und die imperienweite Moralproduktion auf die anderen System
	// �bertragen
	for(std::vector<CSector>::const_iterator se = m_Sectors.begin(); se != m_Sectors.end(); ++se) {
		CSystem* sy = &GetSystemForSector(*se);
		if(se->GetSunSystem() && sy->GetOwnerOfSystem() != "") {
			// imperiumsweite Moralproduktion aus diesem System berechnen
			sy->CalculateEmpireWideMoralProd(&BuildingInfo);
		}
		//Building scan power and range in a system isn't influenced by other systems, is it...?
		//This needs to be here in the first loop, since when calculating the scan power that
		//other majors get due to affiliation, the scan powers in all sectors are not yet calculated correctly.
		//For instance, if the system (1,1) scans the sector (0,0) since the loop
		//starts at (0,0) in the top left; so when transferring the scanpower in (0,0)
		//it is not yet updated.
		const CSystemProd& production = *sy->GetProduction();
		PutScannedSquareOverCoords(se->GetKO(), production.GetScanRange(), production.GetScanPower(), sy->GetOwnerOfSystem());
	}
}
void CBotf2Doc::AddShipPortsFromMinors(const std::map<CString, CMajor*>& pmMajors) {
	const map<CString, CMinor*>* pmMinors = m_pRaceCtrl->GetMinors();
	for (map<CString, CMajor*>::const_iterator i = pmMajors.begin(); i != pmMajors.end(); ++i)
	{
		CMajor* pMajor = i->second;
		for (map<CString, CMinor*>::const_iterator j = pmMinors->begin(); j != pmMinors->end(); ++j)
		{
			const CMinor* pMinor = j->second;
			const DIPLOMATIC_AGREEMENT::Typ agreement = pMinor->GetAgreement(pMajor->GetRaceID());
			if (pMinor->GetSpaceflightNation()&& (
					agreement == DIPLOMATIC_AGREEMENT::COOPERATION ||
					agreement == DIPLOMATIC_AGREEMENT::AFFILIATION))
			{
				CPoint p = pMinor->GetRaceKO();
				if (p != CPoint(-1,-1))
					GetSector(p).SetShipPort(TRUE, pMajor->GetRaceID());
			}
		}
	}
}
static void EmitLostRouteMessage(unsigned deletedRoutes, const CString& single_key, const CString& multi_key,
		const CString& sectorname, const CPoint& co, CEmpire* pEmpire) {
	CString news;
	if (deletedRoutes == 1)
		news = CResourceManager::GetString(single_key,FALSE,sectorname);
	else
	{
		CString lost; lost.Format("%u",deletedRoutes);
		news = CResourceManager::GetString(multi_key,FALSE,lost,sectorname);
	}
	CMessage message;
	message.GenerateMessage(news, MESSAGE_TYPE::ECONOMY, "", co, FALSE, 4);
	pEmpire->AddMessage(message);
}
void CBotf2Doc::CheckRoutes(const CSector& sector, CSystem& system, CMajor* pMajor) {
	CEmpire* pEmpire = pMajor->GetEmpire();
	const CPoint& co = sector.GetKO();
	const CString& sector_name = sector.GetName();
	bool select_empire_view = false;

	unsigned deletedTradeRoutes = 0;
	deletedTradeRoutes += system.CheckTradeRoutesDiplomacy(*this, co);
	deletedTradeRoutes += system.CheckTradeRoutes(pEmpire->GetResearch()->GetResearchInfo());
	if(deletedTradeRoutes > 0) {
		select_empire_view = true;
		EmitLostRouteMessage(deletedTradeRoutes, "LOST_ONE_TRADEROUTE", "LOST_TRADEROUTES",
			sector_name, co, pEmpire);
	}
	unsigned deletedResourceRoutes = 0;
	deletedResourceRoutes += system.CheckResourceRoutesExistence(*this);
	deletedResourceRoutes += system.CheckResourceRoutes(pEmpire->GetResearch()->GetResearchInfo());
	if(deletedResourceRoutes > 0) {
		select_empire_view = true;
		EmitLostRouteMessage(deletedResourceRoutes, "LOST_ONE_RESOURCEROUTE", "LOST_RESOURCEROUTES",
			sector_name, co, pEmpire);
	}

	if(select_empire_view && pMajor->IsHumanPlayer()) {
		const network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
		m_iSelectedView[client] = EMPIRE_VIEW;
	}
}

static void CalcExtraVisibilityAndRangeDueToDiplomacy(CSector& sector, const std::map<CString, CMajor*>* pmMajors) {
	for (map<CString, CMajor*>::const_iterator i = pmMajors->begin(); i != pmMajors->end(); ++i)
	{
		for (map<CString, CMajor*>::const_iterator j = pmMajors->begin(); j != pmMajors->end(); ++j)
		{
			if(i == j) continue;
			const DIPLOMATIC_AGREEMENT::Typ agreement = i->second->GetAgreement(j->first);
			if (sector.GetScanned(i->first))
			{
				if (agreement >= DIPLOMATIC_AGREEMENT::COOPERATION)
					sector.SetScanned(j->first);
				if (agreement >= DIPLOMATIC_AGREEMENT::AFFILIATION)
				{
					const short iscanpower = sector.GetScanPower(i->first);
					const short jscanpower = sector.GetScanPower(j->first);
					if(iscanpower > jscanpower)
						sector.SetScanPower(iscanpower, j->first);
				}
			}
			if (sector.GetKnown(i->first))
			{
				if (agreement >= DIPLOMATIC_AGREEMENT::FRIENDSHIP)
					sector.SetScanned(j->first);
				if (agreement >= DIPLOMATIC_AGREEMENT::COOPERATION)
					sector.SetKnown(j->first);
			}
			if (sector.GetOwnerOfSector() == i->first)
			{
				if (agreement >= DIPLOMATIC_AGREEMENT::TRADE)
					sector.SetScanned(j->first);
				if (agreement >= DIPLOMATIC_AGREEMENT::FRIENDSHIP)
					sector.SetKnown(j->first);
			}
			if (sector.GetShipPort(i->first))
				if (agreement >= DIPLOMATIC_AGREEMENT::COOPERATION)
					sector.SetShipPort(TRUE, j->first);
		}//for (map<CString, CMajor*>::const_iterator j = pmMajors->begin(); j != pmMajors->end(); ++j)
	}//for (map<CString, CMajor*>::const_iterator i = pmMajors->begin(); i != pmMajors->end(); ++i)
}
static void CalcNewRoundDataMoral(const CSector& sector, CSystem& system, CArray<CTroopInfo>& TroopInfo) {
	// Wurde das System milit�risch erobert, so verringert sich die Moral pro Runde etwas
	if (sector.GetTakenSector() && system.GetMoral() > 70)
		system.SetMoral(-rand()%2);
	// m�glicherweise wird die Moral durch stationierte Truppen etwas stabilisiert
	system.IncludeTroopMoralValue(&TroopInfo);
}

void CBotf2Doc::PutScannedSquareOverCoords(const CPoint& co, int range, const unsigned power, const CString& race_id,
					bool ship, bool bBetterScanner, bool patrolship) {
	if(!ship && power == 0) return;
	float boni = 1.0f;
	if(bBetterScanner) {
		range *= 1.5;
		boni += 0.5;
	}
	for (int i = -range; i <= range; ++i) {
		const int x = co.x + i;
		if(0 <= x && x < STARMAP_SECTORS_HCOUNT) {
			for (int j = -range; j <= range; ++j) {
				const int y = co.y + j;
				if(0 <= y && y < STARMAP_SECTORS_VCOUNT) {
					CSector& scanned_sector = GetSector(x, y);
					scanned_sector.SetScanned(race_id);
					// Wenn das Schiff die Patrouillieneigenschaft besitzt und sich in einem eigenen Sektor befindet,
					// dann wird die Scanleistung um 20% erh�ht.
					if(patrolship && scanned_sector.GetOwnerOfSector() == race_id)
						boni += 0.2f;
					// Teiler f�r die Scanst�rke berechnen
					const unsigned div = max(max(abs(i), abs(j)), 1);
					const unsigned old_scan_power = scanned_sector.GetScanPower(race_id);
					unsigned new_scan_power = (power * boni) / div;
					new_scan_power = max(old_scan_power, new_scan_power);
					if(ship && x == co.x && y == co.y)
						++new_scan_power;
					scanned_sector.SetScanPower(new_scan_power, race_id);
				}//if(0 <= y && y < STARMAP_SECTORS_VCOUNT)
			}//for (int j = -range; j <= range; ++j)
		}//if(0 <= x && x < STARMAP_SECTORS_HCOUNT)
	}//for (int i = -range; i <= range; ++i)
}

static void CalcNewRoundDataIntelligenceBoni(const CSystemProd* production, CIntelligence* intelligence) {
	// Hier die gesamten Sicherheitsboni der Imperien berechnen
	intelligence->AddInnerSecurityBonus(production->GetInnerSecurityBoni());
	intelligence->AddEconomyBonus(production->GetEconomySpyBoni(), 0);
	intelligence->AddEconomyBonus(production->GetEconomySabotageBoni(), 1);
	intelligence->AddScienceBonus(production->GetScienceSpyBoni(), 0);
	intelligence->AddScienceBonus(production->GetScienceSabotageBoni(), 1);
	intelligence->AddMilitaryBonus(production->GetMilitarySpyBoni(), 0);
	intelligence->AddMilitaryBonus(production->GetMilitarySabotageBoni(), 1);
}
static void GetResearchBoniFromSpecialTechsAndSetThem(
	std::map<CString, CSystemProd::RESEARCHBONI>& researchBonis, const std::map<CString, CMajor*>* pmMajors) {
	// Forschungsboni aus Spezialforschungen setzen, nachdem wir diese aus allen Systemen geholt haben
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CResearch* pResearch = it->second->GetEmpire()->GetResearch();
		const CResearchComplex* research_complex = pResearch->GetResearchInfo()
			->GetResearchComplex(RESEARCH_COMPLEX::RESEARCH);
		// Die Boni auf die einzelnen Forschungsgebiete durch Spezialforschungen addieren
		if (research_complex->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			researchBonis[it->first].nBoni[0] += research_complex->GetBonus(1);
			researchBonis[it->first].nBoni[1] += research_complex->GetBonus(1);
		}
		else if (research_complex->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			researchBonis[it->first].nBoni[2] += research_complex->GetBonus(2);
			researchBonis[it->first].nBoni[3] += research_complex->GetBonus(2);
		}
		else if (research_complex->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		{
			researchBonis[it->first].nBoni[4] += research_complex->GetBonus(3);
			researchBonis[it->first].nBoni[5] += research_complex->GetBonus(3);
		}
		pResearch->SetResearchBoni(researchBonis[it->first].nBoni);
	}
}
static void GetIntelligenceBoniFromSpecialTechsAndSetThem(const std::map<CString, CMajor*>* pmMajors) {
	// Geheimdienstboni aus Spezialforschungen holen
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CIntelligence* pIntelligence = it->second->GetEmpire()->GetIntelligence();
		const CResearchComplex* pResearchComplex = it->second->GetEmpire()->GetResearch()->GetResearchInfo()
			->GetResearchComplex(RESEARCH_COMPLEX::SECURITY);
		// Die Boni auf die einzelnen Geheimdienstgebiete berechnen
		if (pResearchComplex->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
			pIntelligence->AddInnerSecurityBonus(pResearchComplex->GetBonus(1));
		else if (pResearchComplex->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
		{
			pIntelligence->AddEconomyBonus(pResearchComplex->GetBonus(2), 1);
			pIntelligence->AddMilitaryBonus(pResearchComplex->GetBonus(2), 1);
			pIntelligence->AddScienceBonus(pResearchComplex->GetBonus(2), 1);
		}
		else if (pResearchComplex->GetFieldStatus(3) == RESEARCH_STATUS::RESEARCHED)
		{
			pIntelligence->AddEconomyBonus(pResearchComplex->GetBonus(3), 0);
			pIntelligence->AddMilitaryBonus(pResearchComplex->GetBonus(3), 0);
			pIntelligence->AddScienceBonus(pResearchComplex->GetBonus(3), 0);
		}
	}
}
///////END: HELPER FUNCTIONS FOR CBotf2Doc::CalcNewRoundData()///////

/// Diese Funktion berechnet die Produktion der Systeme, was in den Baulisten gebaut werden soll und sonstige
/// Daten f�r die neue Runde.
void CBotf2Doc::CalcNewRoundData()
{
	CalcNewRoundDataPreLoop();

	map<CString, CSystemProd::RESEARCHBONI> researchBonis;
	const map<CString, CMajor*>* const pmMajors = m_pRaceCtrl->GetMajors();

	// Hier werden jetzt die baubaren Geb�ude f�r die n�chste Runde und auch die Produktion in den einzelnen
	// Systemen berechnet. K�nnen das nicht in obiger Hauptschleife machen, weil dort es alle globalen Geb�ude
	// gesammelt werden m�ssen und ich deswegen alle Systeme mit den fertigen Bauauftr�gen in dieser Runde einmal
	// durchgegangen sein mu�.
	for(std::vector<CSector>::iterator sector = m_Sectors.begin(); sector != m_Sectors.end(); ++sector) {
		CSystem& system = GetSystemForSector(*sector);
		const CString& system_owner = system.GetOwnerOfSystem();
		if (sector->GetSunSystem() && system_owner != "")
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(system_owner));
			assert(pMajor);
			CEmpire* empire = pMajor->GetEmpire();

			// Hier die Credits durch Handelsrouten berechnen und
			// Ressourcenrouten checken
			CheckRoutes(*sector, system, pMajor);

			system.CalculateVariables(&this->BuildingInfo,
				empire->GetResearch()->GetResearchInfo(),
				sector->GetPlanets(), pMajor, CTrade::GetMonopolOwner());

			const CSystemProd* const production = system.GetProduction();
			// Haben wir eine online Schiffswerft im System, dann ShipPort in dem Sektor setzen
			if (production->GetShipYard())
				sector->SetShipPort(TRUE, system_owner);
			CalcNewRoundDataMoral(*sector, system, m_TroopInfo);

			// Hier die gesamten Forschungsboni der Imperien berechnen
			// Forschungsboni, die die Systeme machen holen. Wir ben�tigen diese dann f�r die CalculateResearch Funktion
			// hier Forschungsboni besorgen
			researchBonis[system_owner] += production->GetResearchBoni();

			// Hier die gesamten Sicherheitsboni der Imperien berechnen
			CalcNewRoundDataIntelligenceBoni(production, empire->GetIntelligence());

			// Anzahl aller Ressourcen in allen eigenen Systemen berechnen
			for (int res = TITAN; res <= DERITIUM; res++)
				empire->SetStorageAdd(res, system.GetResourceStore(res));
		}//if (sector.GetSunSystem() && system_owner != "")

		// f�r jede Rasse Sektorsachen berechnen
		// Hier wird berechnet, was wir von der Karte alles sehen, welche Sektoren wir durchfliegen k�nnen
		// alles abh�ngig von unseren diplomatischen Beziehungen
		CalcExtraVisibilityAndRangeDueToDiplomacy(*sector, pmMajors);
	}//for(std::vector<CSector>::iterator sector = m_Sectors.begin(); sector != m_Sectors.end(); ++sector) {

	// Forschungsboni aus Spezialforschungen setzen, nachdem wir diese aus allen Systemen geholt haben
	GetResearchBoniFromSpecialTechsAndSetThem(researchBonis, pmMajors);
	// Geheimdienstboni aus Spezialforschungen holen
	GetIntelligenceBoniFromSpecialTechsAndSetThem(pmMajors);
	// Nun �berpr�fen, ob sich unsere Grenzen erweitern, wenn die MinorRasse eine Spaceflight-Rasse ist und wir mit
	// ihr eine Kooperations oder ein B�ndnis haben
	AddShipPortsFromMinors(*pmMajors);
}

/// Diese Funktion berechnet die kompletten Handelsaktivit�ten.
void CBotf2Doc::CalcTrade()
{
	// Hier berechnen wir alle Handelsaktionen
	USHORT taxMoney[] = {0,0,0,0,0};	// alle Steuern auf eine Ressource

	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	if (pmMajors->empty())
		AfxMessageBox("Error in CBotf2Doc::CalcTrade(): Could not get any major from race controller!");

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		ASSERT(pMajor);

		///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
		// Hier die Boni durch die Uniqueforschung "Handel" -> keine Handelsgeb�hr
		if (pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetFieldStatus(1) == RESEARCH_STATUS::RESEARCHED)
		{
			float newTax = (float)pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::TRADE)->GetBonus(1);
			newTax = 1.0f + newTax / 100;
			pMajor->GetTrade()->SetTax(newTax);
		}
		pMajor->GetTrade()->CalculateTradeActions(pMajor, m_Systems, m_Sectors, taxMoney);
		for (int j = TITAN; j <= IRIDIUM; j++)
		{
			// plus Steuern, die durch Sofortk�ufe von Bauauftr�gen entstanden sind holen
			if (CTrade::GetMonopolOwner(j).IsEmpty() == false)
				if (CTrade::GetMonopolOwner(j) == pMajor->GetRaceID() || pMajor->IsRaceContacted(CTrade::GetMonopolOwner(j)) == true)
					taxMoney[j] += pMajor->GetTrade()->GetTaxesFromBuying()[j];
		}
	}
	// die Steuern durch den Handel den Monopolbesitzern gutschreiben und nach Monopolk�ufen Ausschau halten
	for (int i = TITAN; i <= IRIDIUM; i++)
	{
		CString resName;
		switch(i)
		{
		case TITAN:		resName = CResourceManager::GetString("TITAN");		break;
		case DEUTERIUM: resName = CResourceManager::GetString("DEUTERIUM");	break;
		case DURANIUM:	resName = CResourceManager::GetString("DURANIUM");	break;
		case CRYSTAL:	resName = CResourceManager::GetString("CRYSTAL");	break;
		case IRIDIUM:	resName = CResourceManager::GetString("IRIDIUM");	break;
		}

		if (CTrade::GetMonopolOwner(i).IsEmpty() == false)
		{
			//CString hh;
			//hh.Format("Steuern auf %d: %d Credits",i,taxMoney[i]);
			//AfxMessageBox(hh);
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(CTrade::GetMonopolOwner(i)));
			ASSERT(pMajor);
			if (pMajor)
				pMajor->GetEmpire()->SetCredits(taxMoney[i]);
		}

		// Hier die gekauften Monopole den Rassen zuschreiben. Wer am meisten bezahlt hat (falls mehrere Rassen
		// in der selben Runde ein Monopol kaufen m�chten) bekommt das Monopol. Die anderen bekommen ihr Credits zur�ck
		double max = 0.0f;				// meiste Credits was f�r ein Monopol gegeben wurde
		CString sMonopolRace = "";		// Rasse die das Monopol erlangt hat


		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			CMajor* pMajor = it->second;
			ASSERT(pMajor);

			if (pMajor->GetTrade()->GetMonopolBuying()[i] > max)
			{
				max = pMajor->GetTrade()->GetMonopolBuying()[i];
				sMonopolRace = pMajor->GetRaceID();
				CTrade::SetMonopolOwner(i, sMonopolRace);
			}
		}

		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			CMajor* pMajor = it->second;
			ASSERT(pMajor);

			CString sNews = "";
			// Die anderen Rassen bekommen ihr Geld zur�ck
			if (pMajor->GetRaceID() != sMonopolRace && pMajor->GetTrade()->GetMonopolBuying()[i] != 0)
			{
				pMajor->GetEmpire()->SetCredits((long)pMajor->GetTrade()->GetMonopolBuying()[i]);
				// Nachricht generieren, dass wir es nicht geschafft haben ein Monopol zu kaufen
				sNews = CResourceManager::GetString("WE_DONT_GET_MONOPOLY",FALSE,resName);
			}
			// Nachricht an unser Imperium, dass wir ein Monopol erlangt haben
			else if (pMajor->GetRaceID() == sMonopolRace)
				sNews = CResourceManager::GetString("WE_GET_MONOPOLY",FALSE,resName);

			if (!sNews.IsEmpty())
			{
				CMessage message;
				message.GenerateMessage(sNews,MESSAGE_TYPE::SOMETHING,"",0,FALSE);
				pMajor->GetEmpire()->AddMessage(message);
				if (pMajor->IsHumanPlayer())
				{
					network::RACE clientID = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
					m_iSelectedView[clientID] = EMPIRE_VIEW;
				}
				pMajor->GetTrade()->SetMonopolBuying(i,0.0f);
			}


			// Nachrichten an die einzelnen Imperien verschicken, das eine Rasse das Monopol erlangt hat
			if (sMonopolRace.IsEmpty() == false && sMonopolRace != pMajor->GetRaceID())
			{
				CMajor* pMonopolRace = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sMonopolRace));
				ASSERT(pMonopolRace);

				CString sRace = CResourceManager::GetString("UNKNOWN");
				if (pMajor->IsRaceContacted(sMonopolRace))
					sRace = pMonopolRace->GetRaceNameWithArticle();

				CString news = CResourceManager::GetString("SOMEBODY_GET_MONOPOLY",TRUE,sRace,resName);
				CMessage message;
				message.GenerateMessage(news,MESSAGE_TYPE::SOMETHING,"",0,FALSE);
				pMajor->GetEmpire()->AddMessage(message);
				if (pMajor->IsHumanPlayer())
				{
					network::RACE clientID = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
					m_iSelectedView[clientID] = EMPIRE_VIEW;
				}
			}
		}
	}
	// Hier den neuen Kurs der Waren an den Handelsb�rsen berechnen, d�rfen wir erst machen, wenn wir f�r alle Tradeobjekte
	// die einzelnen Kurse berechnet haben
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		ASSERT(pMajor);

		pMajor->GetTrade()->CalculatePrices(pmMajors, pMajor);
		// Hier die aktuellen Kursdaten in die History schreiben

		USHORT* resPrices = pMajor->GetTrade()->GetRessourcePrice();
		pMajor->GetTrade()->GetTradeHistory()->SaveCurrentPrices(resPrices, pMajor->GetTrade()->GetTax());
	}
}

/// Diese Funktion berechnet die Schiffsbefehle. Der Systemangriffsbefehl ist davon ausgenommen.
void CBotf2Doc::CalcShipOrders()
{
	// Array beinhaltet die Aussenposten, welche nach dem Bau einer Sternbasis aus dem Feld verschwinden m?ssen.
	// Diese k�nnen nicht direkt in der Schleife entfernt werden, da sonst beim der n�chsten Iteration die Schleife nicht mehr hinhaut.
	std::vector<CString> vRemoveableOutposts;

	// Hier kommt die Auswertung der Schiffsbefehle
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
	{
		CShipSanity::SanityCheckFleet(m_ShipArray.GetAt(y));

		// Hier wird �berpr�ft, ob der Systemattack-Befehl noch g�ltig ist
		// Alle Schiffe, welche einen Systemangriffsbefehl haben �berpr�fen, ob dieser Befehl noch g�ltig ist
		CSector* pSector = &GetSector(m_ShipArray[y].GetKO());
  		CSystem* pSystem = &GetSystem(m_ShipArray[y].GetKO());

		if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::ATTACK_SYSTEM)
		{
			if (pSector->GetSunSystem())
			{
				// Wenn die Bev�lkerung komplett vernichtet wurde
				if (pSystem->GetHabitants() == 0.0f)
					m_ShipArray.ElementAt(y).SetCurrentOrder(SHIP_ORDER::ATTACK);
				// Wenn das System der angreifenden Rasse geh�rt
				else if (pSystem->GetOwnerOfSystem() == m_ShipArray.GetAt(y).GetOwnerOfShip())
					m_ShipArray.ElementAt(y).SetCurrentOrder(SHIP_ORDER::ATTACK);
				// Wenn eine Rasse in dem System lebt
				else if (pSector->GetOwnerOfSector() != "" && pSector->GetOwnerOfSector() != m_ShipArray.GetAt(y).GetOwnerOfShip())
				{
					CRace* pRace = m_pRaceCtrl->GetRace(pSector->GetOwnerOfSector());
					if (pRace != NULL && pRace->GetAgreement(m_ShipArray.GetAt(y).GetOwnerOfShip()) != DIPLOMATIC_AGREEMENT::WAR)
						m_ShipArray.ElementAt(y).SetCurrentOrder(SHIP_ORDER::ATTACK);
				}
			}
			else
				m_ShipArray.ElementAt(y).SetCurrentOrder(SHIP_ORDER::ATTACK);
		}

		// Flotten checken, falls keine mehr existiert, dann wird der Zeiger auf die Flotte aufgel�st
		// muss nicht unbedingt gemacht werden, h�lt die Objekte aber sauberer
		m_ShipArray[y].CheckFleet();

		// wenn der Befehl "Terraform" ist und kein Planet ausgew?hlt ist, dann Befehl wieder auf "AVOID"
		// setzen
		if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::TERRAFORM && m_ShipArray[y].GetTerraformingPlanet() == -1)
			m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);

		// Haben wir eine Flotte, den aktuellen Befehl an alle Schiffe in der Flotte weitergeben
		if (m_ShipArray[y].GetFleet() != 0)
			m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);

		 // Planet soll kolonisiert werden
		if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::COLONIZE)
		{
			// �berpr�fen das der Sector auch nur mir oder niemandem geh?rt
			if ((pSector->GetOwnerOfSector() == m_ShipArray[y].GetOwnerOfShip() || pSector->GetOwnerOfSector() == ""))
			{
				// Wieviele Einwohner bekommen wir in dem System durch die Kolonisierung?
				float startHabitants = (float)(m_ShipArray[y].GetColonizePoints() * 4);
				// Wenn keine Nummer eines Planeten zum Kolonisieren angegeben ist, dann werden alle geterraformten
				// Planeten kolonisiert. Dazu wird die Bev�lkerung, welche bei der Kolonisierung auf das System kommt
				// auf die einzelnen Planeten gleichm��ig aufgeteilt.
				if (m_ShipArray[y].GetTerraformingPlanet() == -1)
				{
					BYTE terraformedPlanets = 0;
					for (int i = 0; i < pSector->GetNumberOfPlanets(); i++)
						if (pSector->GetPlanets().at(i).GetTerraformed() == TRUE
							&& pSector->GetPlanets().at(i).GetColonized() == FALSE)
							terraformedPlanets++;
					if (terraformedPlanets > 0)
					{
						float tmpHab = startHabitants /= terraformedPlanets;
						float tmpHab2 = 0.0f;
						float oddHab = 0.0f;	// �bersch�ssige Kolonisten, wenn ein Planet zu klein ist
						// Geterraformte Planeten durchgehen und die Bev�lkerung auf diese verschieben
						for (int i = 0; i < pSector->GetNumberOfPlanets(); i++)
							if (pSector->GetPlanets().at(i).GetTerraformed() == TRUE
								&& pSector->GetPlanets().at(i).GetColonized() == FALSE)
							{
								if (startHabitants > pSector->GetPlanet(i)->GetMaxHabitant())
								{
									oddHab += (startHabitants - pSector->GetPlanet(i)->GetMaxHabitant());
									startHabitants = pSector->GetPlanet(i)->GetMaxHabitant();
								}
								tmpHab2 += startHabitants;
								pSector->GetPlanet(i)->SetCurrentHabitant(startHabitants);
								pSector->GetPlanet(i)->SetColonisized(TRUE);
								startHabitants = tmpHab;
							}
						startHabitants = tmpHab2;
						// die �brigen Kolonisten auf die Planeten verteilen
						if (oddHab > 0.0f)
							for (int i = 0; i < pSector->GetNumberOfPlanets(); i++)
								if (pSector->GetPlanets().at(i).GetTerraformed() == TRUE
									&& pSector->GetPlanets().at(i).GetCurrentHabitant() > 0.0f)
								{
									if ((oddHab + pSector->GetPlanets().at(i).GetCurrentHabitant())
										> pSector->GetPlanet(i)->GetMaxHabitant())
									{
										oddHab -= (pSector->GetPlanet(i)->GetMaxHabitant()
											- pSector->GetPlanets().at(i).GetCurrentHabitant());
										pSector->GetPlanet(i)->SetCurrentHabitant(pSector->GetPlanet(i)->GetMaxHabitant());
									}
									else
									{
										pSector->GetPlanet(i)->SetCurrentHabitant(
											pSector->GetPlanets().at(i).GetCurrentHabitant() + oddHab);
										break;
									}
								}
					}
					else
					{
						m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
						m_ShipArray[y].SetTerraformingPlanet(-1);
						if (m_ShipArray[y].GetFleet() != 0)
							m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
						continue;
					}
				}
				else
				{
					if (pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetColonized() == FALSE
						&& pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetTerraformed() == TRUE)
					{
						if (startHabitants > pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetMaxHabitant())
							startHabitants = pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetMaxHabitant();
						pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->SetCurrentHabitant(startHabitants);
						pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->SetColonisized(TRUE);
					}
					else
					{
						m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
						m_ShipArray[y].SetTerraformingPlanet(-1);
						if (m_ShipArray[y].GetFleet() != 0)
							m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
						continue;
					}
				}
				CString s;
				CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
				ASSERT(pMajor);
				network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

				// Geb�ude bauen, wenn wir das System zum ersten Mal kolonisieren, sprich das System noch niemanden geh?rt
				if (pSystem->GetOwnerOfSystem() == "")
				{
					// Sector- und Systemwerte �ndern
					pSector->SetOwned(TRUE);
					pSector->SetOwnerOfSector(m_ShipArray[y].GetOwnerOfShip());
					pSector->SetColonyOwner(m_ShipArray[y].GetOwnerOfShip());
					pSystem->SetOwnerOfSystem(m_ShipArray[y].GetOwnerOfShip());
					// Geb�ude nach einer Kolonisierung bauen
					pSystem->BuildBuildingsAfterColonization(pSector,&BuildingInfo,m_ShipArray[y].GetColonizePoints());
					// Nachricht an das Imperium senden, das ein System neu kolonisiert wurde
					s = CResourceManager::GetString("FOUND_COLONY_MESSAGE",FALSE,pSector->GetName());
					CMessage message;
					message.GenerateMessage(s,MESSAGE_TYPE::SOMETHING,pSector->GetName(),pSector->GetKO(),FALSE);
					pMajor->GetEmpire()->AddMessage(message);

					// zus�tzliche Eventnachricht (Colonize a system #12) wegen der Moral an das Imperium
					message.GenerateMessage(pMajor->GetMoralObserver()->AddEvent(12, pMajor->GetRaceMoralNumber(), pSector->GetName()), MESSAGE_TYPE::SOMETHING, "", pSector->GetKO(), FALSE);
					pMajor->GetEmpire()->AddMessage(message);
					if (pMajor->IsHumanPlayer())
					{
						SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_CLAIMSYSTEM, client, 0, 1.0f};
						m_SoundMessages[client].Add(entry);
						m_iSelectedView[client] = EMPIRE_VIEW;

						CEventColonization* eventScreen = new CEventColonization(pMajor->GetRaceID(), CResourceManager::GetString("COLOEVENT_HEADLINE", FALSE, pSector->GetName()), CResourceManager::GetString("COLOEVENT_TEXT_" + pMajor->GetRaceID(), FALSE, pSector->GetName()));
						pMajor->GetEmpire()->GetEventMessages()->Add(eventScreen);
						s.Format("Added Colonization-Eventscreen for Race %s in System %s", pMajor->GetRaceName(), pSector->GetName());
						MYTRACE("general")(MT::LEVEL_INFO, s);
					}
				}
				else
				{
					// Nachricht an das Imperium senden, das ein Planet kolonisiert wurde
					s = CResourceManager::GetString("NEW_PLANET_COLONIZED",FALSE,pSector->GetName());
					CMessage message;
					message.GenerateMessage(s,MESSAGE_TYPE::SOMETHING,pSector->GetName(),pSector->GetKO(),FALSE);
					pMajor->GetEmpire()->AddMessage(message);
					if (pMajor->IsHumanPlayer())
						m_iSelectedView[client] = EMPIRE_VIEW;
				}
				pSystem->SetHabitants(pSector->GetCurrentHabitants());

				pSystem->CalculateNumberOfWorkbuildings(&this->BuildingInfo);
				pSystem->CalculateVariables(&this->BuildingInfo, pMajor->GetEmpire()->GetResearch()->GetResearchInfo(), pSector->GetPlanets(), pMajor, CTrade::GetMonopolOwner());

				// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
				s.Format("%s %s",CResourceManager::GetString("COLONIZATION"), pSector->GetName());
				AddToLostShipHistory(&m_ShipArray[y], s, CResourceManager::GetString("DESTROYED"));
				// Schiff entfernen
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
				m_ShipArray[y].SetTerraformingPlanet(-1);
				if (m_ShipArray[y].GetFleet() != 0)
					m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
				RemoveShip(y--);
				continue;
			}
			else
			{
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
				m_ShipArray[y].SetTerraformingPlanet(-1);
			}
		}
		// hier wird ein Planet geterraformed
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::TERRAFORM && m_ShipArray[y].GetTerraformingPlanet() != -1)	// Planet soll terraformed werden
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			if (pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetTerraformed() == FALSE)
			{
				if (pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->SetNeededTerraformPoints(m_ShipArray[y].GetColonizePoints()))
				{
					// Hier wurde ein Planet erfolgreich geterraformt
					m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
					m_ShipArray[y].SetTerraformingPlanet(-1);
					// Nachricht generieren, dass Terraforming abgeschlossen wurde
					CString s = CResourceManager::GetString("TERRAFORMING_FINISHED",FALSE,pSector->GetName());
					CMessage message;
					message.GenerateMessage(s,MESSAGE_TYPE::SOMETHING,pSector->GetName(),pSector->GetKO(),FALSE);
					pMajor->GetEmpire()->AddMessage(message);
					if (pMajor->IsHumanPlayer())
					{
						SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_TERRAFORM_COMPLETE, client, 0, 1.0f};
						m_SoundMessages[client].Add(entry);
						m_iSelectedView[client] = EMPIRE_VIEW;
					}
					// Wenn wir einer Rasse beim Terraformen helfen, so gibt es einen Beziehungsboost
					if (pSector->GetOwnerOfSector() != "" && pSector->GetMinorRace() == TRUE && pSystem->GetOwnerOfSystem() == "")
					{
						CMinor* pMinor = m_pRaceCtrl->GetMinorRace(pSector->GetName());
						if (pMinor)
							pMinor->SetRelation(pMajor->GetRaceID(), +rand()%11);
					}
				}
			}
			else	// wenn der Plani aus irgendeinen Grund schon geterraformed ist
			{
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
				m_ShipArray[y].SetTerraformingPlanet(-1);
				if (m_ShipArray[y].GetFleet() != 0)
					m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
			}
			// Wenn das Schiff eine Flotte anf?hrt, dann k?nnen auch die Schiffe in der Flotte ihre Terraformpunkte mit
			// einbringen
			if (m_ShipArray[y].GetFleet() != 0 && m_ShipArray[y].GetTerraformingPlanet() != -1)
			{
				unsigned colonize_points_sum = m_ShipArray[y].GetColonizePoints();
				for (USHORT x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
				{
					CShip* pFleetShip = m_ShipArray[y].GetFleet()->GetShipFromFleet(x);
					if (pSector->GetPlanet(m_ShipArray[y].GetTerraformingPlanet())->GetTerraformed() == FALSE)
					{
						const unsigned colonize_points = pFleetShip->GetColonizePoints();
						colonize_points_sum += colonize_points;
						if (pSector->GetPlanet(pFleetShip->GetTerraformingPlanet())->SetNeededTerraformPoints(colonize_points))
						{
							m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
							m_ShipArray[y].SetTerraformingPlanet(-1);
							m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
							// Nachricht generieren, dass Terraforming abgeschlossen wurde
							CString s = CResourceManager::GetString("TERRAFORMING_FINISHED",FALSE,pSector->GetName());
							CMessage message;
							message.GenerateMessage(s,MESSAGE_TYPE::SOMETHING,pSector->GetName(),pSector->GetKO(),FALSE);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_TERRAFORM_COMPLETE, client, 0, 1.0f};
								m_SoundMessages[client].Add(entry);
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
							// Wenn wir einer Rasse beim Terraformen helfen, so gibt es einen Beziehungsboost
							if (pSector->GetOwnerOfSector() != "" && pSector->GetMinorRace() == TRUE && pSystem->GetOwnerOfSystem() == "")
							{
								CMinor* pMinor = m_pRaceCtrl->GetMinorRace(pSector->GetName());
								if (pMinor)
									pMinor->SetRelation(pMajor->GetRaceID(), +rand()%11);
							}
							break;
						}
					}
					else	// wenn der Plani aus irgendeinen Grund schon geterraformed ist
					{
						m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
						m_ShipArray[y].SetTerraformingPlanet(-1);
						m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
						break;
					}
				}
				//Gib eine Warnung aus falls Kolonisierungspunkte verschwendet werden w�rden.
				//Es ist hoffentlich nicht m�glich, dass ein Schiff einer Schiffsgruppe einen anderen Planeten
				//terraformt als das die Gruppe anf�hrende Schiff...
				const unsigned terraforming_planet = m_ShipArray[y].GetTerraformingPlanet();
				if (terraforming_planet != -1)//wird immernoch geterraformt ?
				{
					const unsigned needed_terraform_points = pSector->GetPlanet(terraforming_planet)->GetNeededTerraformPoints();
					if(colonize_points_sum > needed_terraform_points)
					{
						CString s;
						s.Format("%u", colonize_points_sum - needed_terraform_points);
						s = CResourceManager::GetString("TERRAFORMING_POINTS_WASTED",FALSE,pSector->GetName(), s);
						CMessage message;
						message.GenerateMessage(s,MESSAGE_TYPE::SOMETHING,pSector->GetName(),pSector->GetKO(),FALSE);
						pMajor->GetEmpire()->AddMessage(message);
					}
				}
			}
		}
		// hier wird ein Aussenposten gebaut
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::BUILD_OUTPOST)	// es soll eine Station gebaut werden
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			// jetzt m�ssen wir die Schiffsinfos durchgehen und schauen, welche Station wir technologisch bauen k�nnten.
			// hier wird vereinfacht angenommen, das an teurerer Aussenposten auch ein besserer ist
			short id = -1;
			// Wenn wir in dem Sektor noch keinen Au?enposten und noch keine Sternbasis stehen haben
			if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == FALSE
				&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
			{
				USHORT costs = 0;
				const BYTE researchLevels[6] =
				{
					pMajor->GetEmpire()->GetResearch()->GetBioTech(),
					pMajor->GetEmpire()->GetResearch()->GetEnergyTech(),
					pMajor->GetEmpire()->GetResearch()->GetCompTech(),
					pMajor->GetEmpire()->GetResearch()->GetPropulsionTech(),
					pMajor->GetEmpire()->GetResearch()->GetConstructionTech(),
					pMajor->GetEmpire()->GetResearch()->GetWeaponTech()
				};
				for (int l = 0; l < m_ShipInfoArray.GetSize(); l++)
					if (m_ShipInfoArray.GetAt(l).GetRace() == pMajor->GetRaceShipNumber()
						&& m_ShipInfoArray.GetAt(l).GetShipType() == SHIP_TYPE::OUTPOST
						&& m_ShipInfoArray.GetAt(l).GetBaseIndustry() > costs
						&& m_ShipInfoArray.GetAt(l).IsThisShipBuildableNow(researchLevels))
						{
							costs = m_ShipInfoArray.GetAt(l).GetBaseIndustry();
							id = m_ShipInfoArray.GetAt(l).GetID();
						}
			}
			// Wenn wir eine baubare Station gefunden haben und in dem Sektor nicht gerade eine andere (durch andere Rasse)
			// Station fertig wurde, k?nnen wir diese dort auch errichten
			if (id != -1)
			{
				BOOL buildable = TRUE;
				map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
				for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
				{
					if (pSector->GetOutpost(it->first) == TRUE ||
					   (pSector->GetOutpost(it->first) == FALSE && pSector->GetStarbase(it->first) == TRUE))
					{
						buildable = FALSE;
						break;
					}
				}
				if (buildable == TRUE)
				{
					// Wenn wir also an einer Station gerade bauen -> Variable auf TRUE setzen
					pSector->SetIsStationBuilding(TRUE, m_ShipArray[y].GetOwnerOfShip());
					// Wenn wir gerade anfangen an einer Station zu bauen, also noch keine BuildPoints zusammenhaben
					if (pSector->GetStartStationPoints(m_ShipArray[y].GetOwnerOfShip()) == 0)
						// dann Industriekosten der Station als StationBuildPoints nehmen
						pSector->SetStartStationPoints(m_ShipInfoArray.GetAt((id-10000)).GetBaseIndustry(),m_ShipArray[y].GetOwnerOfShip());
					// Wenn das Schiff eine Flotte anf?hrt, dann erstmal die Au?enpostenbaupunkte der Schiffe in der Flotte
					// beachten und gegebenfalls das Schiff aus der Flotte entfernen
					if (m_ShipArray[y].GetFleet() != 0)
					{
						for (USHORT x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
							if (pSector->SetNeededStationPoints(m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetStationBuildPoints(),m_ShipArray[y].GetOwnerOfShip()))
							{
								// Station ist fertig, also bauen (wurde durch ein Schiff in der Flotte fertiggestellt)
								if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == FALSE
									&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
								{
									pSector->SetOutpost(TRUE, m_ShipArray[y].GetOwnerOfShip());
									pSector->SetOwnerOfSector(m_ShipArray[y].GetOwnerOfShip());
									pSector->SetScanned(m_ShipArray[y].GetOwnerOfShip());
									pSector->SetOwned(TRUE);
									pSector->SetShipPort(TRUE,m_ShipArray[y].GetOwnerOfShip());
									// Nachricht generieren, dass der Aussenpostenbau abgeschlossen wurde
									CMessage message;
									message.GenerateMessage(CResourceManager::GetString("OUTPOST_FINISHED"),MESSAGE_TYPE::MILITARY,"",pSector->GetKO(),FALSE);
									pMajor->GetEmpire()->AddMessage(message);
									// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
									AddToLostShipHistory(m_ShipArray[y].GetFleet()->GetShipFromFleet(x), CResourceManager::GetString("OUTPOST_CONSTRUCTION"), CResourceManager::GetString("DESTROYED"));
									if (pMajor->IsHumanPlayer())
									{
										SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_OUTPOST_READY, client, 0, 1.0f};
										m_SoundMessages[client].Add(entry);
										m_iSelectedView[client] = EMPIRE_VIEW;
									}
									// Wenn eine Station fertig wurde f�r alle Rassen die Punkte wieder canceln
									for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
									{
										pSector->SetIsStationBuilding(FALSE, it->first);
										pSector->SetStartStationPoints(0, it->first);
									}
									// Das Schiff, welches die Station fertiggestellt hat aus der Flotte entfernen
									m_ShipArray[y].GetFleet()->RemoveShipFromFleet(x);
									m_ShipArray[y].CheckFleet();
									BuildShip(id, pSector->GetKO(), m_ShipArray[y].GetOwnerOfShip());
									// Wenn hier ein Au?enposten gebaut wurde den Befehl f?r die Flotte auf Meiden stellen
									m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
									break;
								}
							}
					}
					if (pSector->GetIsStationBuilding(m_ShipArray[y].GetOwnerOfShip()) == TRUE
						&& pSector->SetNeededStationPoints(m_ShipArray[y].GetStationBuildPoints(),m_ShipArray[y].GetOwnerOfShip()))
					{
						// Station ist fertig, also bauen (wurde NICHT!!! durch ein Schiff in der Flotte fertiggestellt)
						if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == FALSE
							&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
						{
							pSector->SetOutpost(TRUE,m_ShipArray[y].GetOwnerOfShip());
							pSector->SetOwnerOfSector(m_ShipArray[y].GetOwnerOfShip());
							pSector->SetScanned(m_ShipArray[y].GetOwnerOfShip());
							pSector->SetOwned(TRUE);
							pSector->SetShipPort(TRUE,m_ShipArray[y].GetOwnerOfShip());
							// Nachricht generieren, dass der Aussenpostenbau abgeschlossen wurde
							CMessage message;
							message.GenerateMessage(CResourceManager::GetString("OUTPOST_FINISHED"),MESSAGE_TYPE::MILITARY,"",pSector->GetKO(),FALSE);
							pMajor->GetEmpire()->AddMessage(message);
							// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
							AddToLostShipHistory(&m_ShipArray[y], CResourceManager::GetString("OUTPOST_CONSTRUCTION"), CResourceManager::GetString("DESTROYED"));
							if (pMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_OUTPOST_READY, client, 0, 1.0f};
								m_SoundMessages[client].Add(entry);
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
							// Wenn eine Station fertig wurde f�r alle Rassen die Punkte wieder canceln
							for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
							{
								pSector->SetIsStationBuilding(FALSE, it->first);
								pSector->SetStartStationPoints(0, it->first);
							}
							// Hier den Aussenposten bauen
							BuildShip(id, pSector->GetKO(), m_ShipArray[y].GetOwnerOfShip());

							// Wenn hier ein Aussenposten gebaut wurde den Befehl f�r die Flotte auf Meiden stellen
							m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
							RemoveShip(y--);
							continue;
						}
					}
				}
				else
					m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
			}
			else
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
		}
		// hier wird eine Sternbasis gebaut
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::BUILD_STARBASE)	// es soll eine Sternbasis gebaut werden
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			// jetzt m�ssen wir die Schiffsinfos durchgehen und schauen, welche Station wir technologisch bauen k?nnten.
			// um eine Sternbasis bauen zu k?nnen mu? schon ein Aussenposten in dem Sektor stehen
			// hier wird vereinfacht angenommen, das eine teurere Sternbasis auch eine bessere ist
			// oder wir haben einen Aussenposten und wollen diesen zur Sternbasis updaten
			short id = -1;
			if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == TRUE
				&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
			{
				USHORT costs = 0;
				const BYTE researchLevels[6] =
				{
					pMajor->GetEmpire()->GetResearch()->GetBioTech(),
					pMajor->GetEmpire()->GetResearch()->GetEnergyTech(),
					pMajor->GetEmpire()->GetResearch()->GetCompTech(),
					pMajor->GetEmpire()->GetResearch()->GetPropulsionTech(),
					pMajor->GetEmpire()->GetResearch()->GetConstructionTech(),
					pMajor->GetEmpire()->GetResearch()->GetWeaponTech()
				};
				for (int l = 0; l < m_ShipInfoArray.GetSize(); l++)
					if (m_ShipInfoArray.GetAt(l).GetRace() == pMajor->GetRaceShipNumber()
						&& m_ShipInfoArray.GetAt(l).GetShipType() == SHIP_TYPE::STARBASE
						&& m_ShipInfoArray.GetAt(l).GetBaseIndustry() > costs
						&& m_ShipInfoArray.GetAt(l).IsThisShipBuildableNow(researchLevels))
						{
							costs = m_ShipInfoArray.GetAt(l).GetBaseIndustry();
							id = m_ShipInfoArray.GetAt(l).GetID();
						}
			}
			// Wenn wir eine baubare Station gefunden haben und in dem Sektor nicht gerade eine andere (durch andere Rasse)
			// Station fertig wurde, k�nnen wir diese dort auch errichten
			if (id != -1)
			{
				BOOL buildable = TRUE;
				map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
				for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
				{
					if (pSector->GetStarbase(it->first) == TRUE
						|| (pSector->GetOutpost(it->first) == TRUE && it->first != m_ShipArray[y].GetOwnerOfShip()))
					{
						buildable = FALSE;
						break;
					}
				}
				if (buildable == TRUE)
				{
					// Wenn wir also an einer Station gerade bauen -> Variable auf TRUE setzen
					pSector->SetIsStationBuilding(TRUE, m_ShipArray[y].GetOwnerOfShip());
					// Wenn wir gerade anfangen an einer Station zu bauen, also noch keine BuildPoints zusammenhaben
					if (pSector->GetStartStationPoints(m_ShipArray[y].GetOwnerOfShip()) == 0)
						// dann Industriekosten der Station als StationBuildPoints nehmen
						pSector->SetStartStationPoints(m_ShipInfoArray.GetAt(id-10000).GetBaseIndustry(),m_ShipArray[y].GetOwnerOfShip());
					// Wenn das Schiff eine Flotte anf?hrt, dann erstmal die Au?enpostenbaupunkte der Schiffe in der Flotte
					// beachten und gegebenfalls das Schiff aus der Flotte entfernen
					if (m_ShipArray[y].GetFleet() != 0)
					{
						for (USHORT x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
							if (pSector->SetNeededStationPoints(m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetStationBuildPoints(),m_ShipArray[y].GetOwnerOfShip()))
							{
								// Station ist fertig, also bauen (wurde durch ein Schiff in der Flotte fertiggestellt)
								if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == TRUE
									&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
								{
									pSector->SetOutpost(FALSE,m_ShipArray[y].GetOwnerOfShip());
									pSector->SetStarbase(TRUE,m_ShipArray[y].GetOwnerOfShip());
									pSector->SetOwnerOfSector(m_ShipArray[y].GetOwnerOfShip());
									pSector->SetScanned(m_ShipArray[y].GetOwnerOfShip());
									pSector->SetOwned(TRUE);
									pSector->SetShipPort(TRUE,m_ShipArray[y].GetOwnerOfShip());
									// Nachricht generieren, dass der Sternbasisbau abgeschlossen wurde
									CMessage message;
									message.GenerateMessage(CResourceManager::GetString("STARBASE_FINISHED"),MESSAGE_TYPE::MILITARY,"",pSector->GetKO(),FALSE);
									pMajor->GetEmpire()->AddMessage(message);
									// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
									AddToLostShipHistory(m_ShipArray[y].GetFleet()->GetShipFromFleet(x), CResourceManager::GetString("STARBASE_CONSTRUCTION"), CResourceManager::GetString("DESTROYED"));
									if (pMajor->IsHumanPlayer())
									{
										SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_STARBASE_READY, client, 0, 1.0f};
										m_SoundMessages[client].Add(entry);
										m_iSelectedView[client] = EMPIRE_VIEW;
									}
									// Wenn eine Station fertig wurde f�r alle Rassen die Punkte wieder canceln
									for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
									{
										pSector->SetIsStationBuilding(FALSE, it->first);
										pSector->SetStartStationPoints(0, it->first);
									}
									// Das Schiff, welches die Station fertiggestellt hat aus der Flotte entfernen
									m_ShipArray[y].GetFleet()->RemoveShipFromFleet(x);
									m_ShipArray[y].CheckFleet();
									this->BuildShip(id, pSector->GetKO(), m_ShipArray[y].GetOwnerOfShip());
									// Wenn wir jetzt die Sternbasis gebaut haben, dann m�ssen wir den alten Aussenposten aus der
									// Schiffsliste nehmen
									for (int k = 0; k <= m_ShipArray.GetSize(); k++)
										if (m_ShipArray[k].GetShipType() == SHIP_TYPE::OUTPOST && m_ShipArray[k].GetKO() == pSector->GetKO())
										{
											// ebenfalls muss der Au?enposten aus der Shiphistory der aktuellen Schiffe entfernt werden
											pMajor->GetShipHistory()->RemoveShip(&m_ShipArray[k]);
											vRemoveableOutposts.push_back(m_ShipArray[k].GetShipName());
											break;
										}
									// Wenn hier eine Station gebaut wurde den Befehl f�r die Flotte auf Meiden stellen
									m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
									break;
								}
							}
					}
					if (pSector->GetIsStationBuilding(m_ShipArray[y].GetOwnerOfShip()) == TRUE
						&& pSector->SetNeededStationPoints(m_ShipArray[y].GetStationBuildPoints(),m_ShipArray[y].GetOwnerOfShip()))
					{
						// Station ist fertig, also bauen (wurde NICHT!!! durch ein Schiff in der Flotte fertiggestellt)
						if (pSector->GetOutpost(m_ShipArray[y].GetOwnerOfShip()) == TRUE
							&& pSector->GetStarbase(m_ShipArray[y].GetOwnerOfShip()) == FALSE)
						{
							pSector->SetOutpost(FALSE,m_ShipArray[y].GetOwnerOfShip());
							pSector->SetStarbase(TRUE,m_ShipArray[y].GetOwnerOfShip());
							pSector->SetOwnerOfSector(m_ShipArray[y].GetOwnerOfShip());
							pSector->SetScanned(m_ShipArray[y].GetOwnerOfShip());
							pSector->SetOwned(TRUE);
							pSector->SetShipPort(TRUE,m_ShipArray[y].GetOwnerOfShip());
							// Nachricht generieren, dass der Sternbasisbau abgeschlossen wurde
							CMessage message;
							message.GenerateMessage(CResourceManager::GetString("STARBASE_FINISHED"),MESSAGE_TYPE::MILITARY,"",pSector->GetKO(),FALSE);
							pMajor->GetEmpire()->AddMessage(message);
							// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
							AddToLostShipHistory(&m_ShipArray[y], CResourceManager::GetString("STARBASE_CONSTRUCTION"), CResourceManager::GetString("DESTROYED"));
							if (pMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_STARBASE_READY, client, 0, 1.0f};
								m_SoundMessages[client].Add(entry);
								m_iSelectedView[client] = EMPIRE_VIEW;
							}
							// Wenn eine Station fertig wurde f�r alle Rassen die Punkte wieder canceln
							for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
							{
								pSector->SetIsStationBuilding(FALSE, it->first);
								pSector->SetStartStationPoints(0, it->first);
							}
							// Sternbasis bauen
							BuildShip(id, pSector->GetKO(), m_ShipArray[y].GetOwnerOfShip());
							// Wenn hier eine Station gebaut wurde den Befehl f�r die Flotte auf Meiden stellen
							m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
							RemoveShip(y--);

							// Wenn die Sternbasis gebaut haben, dann den alten Au?enposten aus der Schiffsliste nehmen
							for (int k = 0; k < m_ShipArray.GetSize(); k++)
								if (m_ShipArray[k].GetShipType() == SHIP_TYPE::OUTPOST && m_ShipArray[k].GetKO() == pSector->GetKO())
								{
									// ebenfalls muss der Au?enposten aus der Shiphistory der aktuellen Schiffe entfernt werden
									pMajor->GetShipHistory()->RemoveShip(&m_ShipArray[k]);
									vRemoveableOutposts.push_back(m_ShipArray[k].GetShipName());
									break;
								}
							continue;
						}
					}
				}
				else
					m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
			}
			else
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
		}
		// Wenn wir das Schiff abracken/zerst?ren/demontieren wollen
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::DESTROY_SHIP)	// das Schiff wird demontiert
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
			ASSERT(pMajor);
			/*network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());*/

			// wenn wir in dem Sector wo wir das Schiff demoniteren ein uns geh�rendes System haben, dann bekommen wir
			// teilweise Rohstoffe aus der Demontage zur�ck (vlt. auch ein paar Credits)
			if (pSystem->GetOwnerOfSystem() == m_ShipArray[y].GetOwnerOfShip())
			{
				USHORT proz = rand()%26 + 50;	// Wert zwischen 50 und 75 ausw?hlen
				// Wenn in dem System Geb�ude stehen, wodurch der Prozentsatz erh�ht wird, dann hier addieren
				proz += pSystem->GetProduction()->GetShipRecycling();
				USHORT id = m_ShipArray[y].GetID() - 10000;
				pSystem->SetTitanStore((int)(m_ShipInfoArray.GetAt(id).GetNeededTitan() * proz / 100));
				pSystem->SetDeuteriumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededDeuterium() * proz / 100));
				pSystem->SetDuraniumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededDuranium() * proz / 100));
				pSystem->SetCrystalStore((int)(m_ShipInfoArray.GetAt(id).GetNeededCrystal() * proz / 100));
				pSystem->SetIridiumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededIridium() * proz / 100));
				pMajor->GetEmpire()->SetCredits((int)(m_ShipInfoArray.GetAt(id).GetNeededIndustry() * proz / 100));
			}
			// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
			pMajor->GetShipHistory()->ModifyShip(&m_ShipArray[y], pSector->GetName(TRUE), m_iRound, CResourceManager::GetString("DISASSEMBLY"),	CResourceManager::GetString("DESTROYED"));

			// Wenn das Schiff eine Flotte anf?hrt, dann auch die Schiffe in der Flotte demontieren
			if (m_ShipArray[y].GetFleet() != 0)
			{
				for (USHORT x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
				{
					if (pSystem->GetOwnerOfSystem() == m_ShipArray[y].GetOwnerOfShip())
					{
						USHORT proz = rand()%26 + 50;	// Wert zwischen 50 und 75 ausw?hlen
						// Wenn in dem System Geb�ude stehen, wodurch der Prozentsatz erh�ht wird, dann hier addieren
						proz += pSystem->GetProduction()->GetShipRecycling();
						USHORT id = m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetID() - 10000;
						pSystem->SetTitanStore((int)(m_ShipInfoArray.GetAt(id).GetNeededTitan() * proz / 100));
						pSystem->SetDeuteriumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededDeuterium() * proz / 100));
						pSystem->SetDuraniumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededDuranium() * proz / 100));
						pSystem->SetCrystalStore((int)(m_ShipInfoArray.GetAt(id).GetNeededCrystal() * proz / 100));
						pSystem->SetIridiumStore((int)(m_ShipInfoArray.GetAt(id).GetNeededIridium() * proz / 100));
						pMajor->GetEmpire()->SetCredits((int)(m_ShipInfoArray.GetAt(id).GetNeededIndustry() * proz / 100));
					}
					// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
					pMajor->GetShipHistory()->ModifyShip(m_ShipArray[y].GetFleet()->GetShipFromFleet(x), pSector->GetName(TRUE), m_iRound, CResourceManager::GetString("DISASSEMBLY"), CResourceManager::GetString("DESTROYED"));
				}
			}

			// Wenn es ein Au?enposten oder eine Sternbasis ist, dann dem Sektor bekanntgeben, dass in ihm keine Station mehr ist
			if (m_ShipArray[y].GetShipType() == SHIP_TYPE::OUTPOST || m_ShipArray[y].GetShipType() == SHIP_TYPE::STARBASE)
			{
				pSector->SetOutpost(FALSE, m_ShipArray[y].GetOwnerOfShip());
				pSector->SetStarbase(FALSE, m_ShipArray[y].GetOwnerOfShip());
			}

			m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
			m_ShipArray.RemoveAt(y--);
			continue;	// continue, damit wir am Ende der Schleife nicht sagen, dass ein Schiff im Sektor ist
		}

		// Wenn wir ein Schiff zum Flagschiff ernennen wollen (nur ein Schiff pro Imperium kann ein Flagschiff sein!)
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::ASSIGN_FLAGSHIP && m_ShipArray[y].GetFleet() == 0)
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip()));
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			// Das ganze Schiffsarray und auch die Flotten durchgehen, wenn wir ein altes Flagschiff finden, diesem den
			// Titel wegnehmen
			for (USHORT n = 0; n < m_ShipArray.GetSize(); n++)
			{
				if (m_ShipArray[n].GetOwnerOfShip() == m_ShipArray[y].GetOwnerOfShip())
				{
					if (m_ShipArray[n].GetIsShipFlagShip() == TRUE)
					{
						m_ShipArray[n].SetIsShipFlagShip(FALSE);
						break;
					}
					// �berpr�fen ob ein Flagschiff in einer Flotte ist
					else if (m_ShipArray[n].GetFleet() != 0)
					{
						bool bFoundFlagShip = false;
						for (USHORT m = 0; m < m_ShipArray[n].GetFleet()->GetFleetSize(); m++)
						{
							if (m_ShipArray[n].GetFleet()->GetShipFromFleet(m)->GetIsShipFlagShip() == TRUE)
							{
								m_ShipArray[n].GetFleet()->GetShipFromFleet(m)->SetIsShipFlagShip(FALSE);
								bFoundFlagShip = true;
								break;
							}
						}
						if (bFoundFlagShip)
							break;
					}
				}
			}
			// Jetzt das neue Schiff zum Flagschiff ernennen
			m_ShipArray[y].SetIsShipFlagShip(TRUE);
			if (m_ShipArray[y].IsNonCombat())
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::AVOID);
			else
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::ATTACK);
			// Nachricht generieren, dass ein neues Schiff zum Flagschiff ernannt wurde
			CString s = CResourceManager::GetString("ASSIGN_FLAGSHIP_MESSAGE",FALSE,m_ShipArray[y].GetShipName(),m_ShipArray[y].GetShipTypeAsString());
			CMessage message;
			message.GenerateMessage(s,MESSAGE_TYPE::MILITARY,"",pSector->GetKO(),FALSE);
			pMajor->GetEmpire()->AddMessage(message);
			if (pMajor->IsHumanPlayer())
				m_iSelectedView[client] = EMPIRE_VIEW;
		}
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::TRAIN_SHIP)
		{
			// Checken ob der Befehl noch G�ltigkeit hat
			if (pSector->GetSunSystem() == TRUE &&
				pSystem->GetOwnerOfSystem() == m_ShipArray[y].GetOwnerOfShip())
			{
				// Wenn ein Schiff mit Veteranenstatus (Level 4) in der Trainingsflotte ist, dann verdoppelt sich der Erfahrungsgewinn
				// f�r die niedrigstufigen Schiffe
				int XP = pSystem->GetProduction()->GetShipTraining();
				bool isVeteran = false;
				if (m_ShipArray[y].GetExpLevel() >= 4)
					isVeteran = true;
				else if (m_ShipArray[y].GetFleet() != 0)
				{
					for (int x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
						if (m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetExpLevel() >= 4)
						{
							isVeteran = true;
							break;
						}
				}

				if (isVeteran == false || m_ShipArray[y].GetExpLevel() >= 4)
					m_ShipArray[y].SetCrewExperiance(XP);
				else
					m_ShipArray[y].SetCrewExperiance(XP * 2);
				// Wenn das Schiff eine Flotte anf�hrt, Schiffstraining auf alle Schiffe in der Flotte anwenden
				if (m_ShipArray[y].GetFleet() != 0)
					for (int x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
					{
						if (isVeteran == false || m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetExpLevel() >= 4)
							m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->SetCrewExperiance(XP);
						else
							m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->SetCrewExperiance(XP * 2);
					}
			}
		}
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::CLOAK)
		{
			m_ShipArray[y].SetCloak();
			// Wenn das Schiff eine Flotte anf�hrt, checken ob der Tarnenbefehl noch G?ltigkeit hat. Wenn ja, dann
			// alle Schiffe in der Flotte tarnen
			if (m_ShipArray[y].GetCloak() == TRUE)
				if (m_ShipArray[y].GetFleet() != 0)
					if (m_ShipArray[y].GetFleet()->CheckOrder(&m_ShipArray[y], SHIP_ORDER::CLOAK) == TRUE)
						for (int x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
							if (m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetCloak() == FALSE)
								m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->SetCloak();

			// Wenn das Schiff enttarnt wurde, dann alle Schiffe in der Flotte entarnen. Dies sollte nicht h�ufig vorkommen. Selbst
			// kann man es so nicht einstellen, aber die KI enttarnt so die Schiffe in der Flotte
			if (m_ShipArray[y].GetCloak() == FALSE)
				if (m_ShipArray[y].GetFleet() != 0)
					for (int x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
						if (m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->GetCloak() == TRUE)
							m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->SetCloak();
			// Befehl wieder auf Angreifen stellen
			m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::ATTACK);
		}
		// Blockadebefehl
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::BLOCKADE_SYSTEM)
		{
			BOOLEAN blockadeStillActive = FALSE;
			// �berpr�fen ob der Blockadebefehl noch G�ltigkeit hat
			if (pSystem->GetOwnerOfSystem() != m_ShipArray[y].GetOwnerOfShip())
				// handelt es sich beim Systembesitzer um eine andere Majorrace
				if (pSystem->GetOwnerOfSystem() != "")
				{
					CString systemOwner = pSystem->GetOwnerOfSystem();
					CString shipOwner   = m_ShipArray[y].GetOwnerOfShip();
					CRace* pShipOwner	= m_pRaceCtrl->GetRace(shipOwner);
					// haben wir einen Vertrag kleiner einem Freundschaftsvertrag mit der Majorrace
					if (pShipOwner->GetAgreement(systemOwner) < DIPLOMATIC_AGREEMENT::FRIENDSHIP)
					{
						int blockadeValue = pSystem->GetBlockade();
						if (m_ShipArray[y].HasSpecial(SHIP_SPECIAL::BLOCKADESHIP))
						{
							blockadeValue += rand()%20 + 1;
							blockadeStillActive = TRUE;
							m_ShipArray[y].CalcExp();
						}
						// Wenn das Schiff eine Flotte anf?hrt, dann erh?hen auch alle Schiffe in der Flotte mit
						// Blockadeeigenschaft den Blockadewert
						if (m_ShipArray[y].GetFleet() != 0)
							for (int x = 0; x < m_ShipArray[y].GetFleet()->GetFleetSize(); x++)
							{
								if (m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->HasSpecial(SHIP_SPECIAL::BLOCKADESHIP))
								{
									blockadeValue += rand()%20 + 1;
									blockadeStillActive = TRUE;
									m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->CalcExp();
								}
								else
									m_ShipArray[y].GetFleet()->GetShipFromFleet(x)->SetCurrentOrder(SHIP_ORDER::ATTACK);
							}
						pSystem->SetBlockade((BYTE)blockadeValue);
						// Die Beziehung zum Systembesitzer verringert sich um bis zu maximal 10 Punkte
						CRace* pSystemOwner	= m_pRaceCtrl->GetRace(systemOwner);
						pSystemOwner->SetRelation(shipOwner, -rand()%(blockadeValue/10 + 1));
					}
				}
			// kann der Blockadebefehl nicht mehr ausgef?hrt werden, so wird der Befehl automatisch gel�scht
			if (!blockadeStillActive)
			{
				m_ShipArray[y].SetCurrentOrder(SHIP_ORDER::ATTACK);
				if (m_ShipArray[y].GetFleet() != 0)
					m_ShipArray[y].GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);
			}
			// wird das System schlussendlich blockiert, so produzieren die Handelsrouten kein Credits mehr
			if (pSystem->GetBlockade() > NULL)
			{
				// Wird das System blockiert, so generiert die Handelsroute kein Credits
				for (int i = 0; i < pSystem->GetTradeRoutes()->GetSize(); i++)
					pSystem->GetTradeRoutes()->GetAt(i).SetCredits(NULL);

				// Eventscreen f�r den Angreifer und den Blockierten anlegen
				CRace* pShipOwner = m_pRaceCtrl->GetRace(m_ShipArray[y].GetOwnerOfShip());
				CMajor* pShipOwnerMajor = NULL;
				if (pShipOwner != NULL && pShipOwner->GetType() == MAJOR && (pShipOwnerMajor = dynamic_cast<CMajor*>(pShipOwner))->IsHumanPlayer())
				{
					CEventBlockade* eventScreen = new CEventBlockade(m_ShipArray[y].GetOwnerOfShip(), CResourceManager::GetString("BLOCKADEEVENT_HEADLINE", FALSE, pSector->GetName()), CResourceManager::GetString("BLOCKADEEVENT_TEXT_" + pShipOwner->GetRaceID(), FALSE, pSector->GetName()));
					pShipOwnerMajor->GetEmpire()->GetEventMessages()->Add(eventScreen);
				}
				if (pSystem->GetOwnerOfSystem() != "")
				{
					CRace* pSystemOwner = m_pRaceCtrl->GetRace(pSystem->GetOwnerOfSystem());
					CMajor* pSystemOwnerMajor = NULL;
					if (pSystemOwner != NULL && pSystemOwner->GetType() == MAJOR && (pSystemOwnerMajor = dynamic_cast<CMajor*>(pSystemOwner))->IsHumanPlayer())
					{
						CEventBlockade* eventScreen = new CEventBlockade(pSystem->GetOwnerOfSystem(), CResourceManager::GetString("BLOCKADEEVENT_HEADLINE", FALSE, pSector->GetName()), CResourceManager::GetString("BLOCKADEEVENT_TEXT_" + pSystemOwner->GetRaceID(), FALSE, pSector->GetName()));
						pSystemOwnerMajor->GetEmpire()->GetEventMessages()->Add(eventScreen);
					}
				}
			}
		}
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::WAIT_SHIP_ORDER)
		{
			//Do nothing, but only for this round.
			m_ShipArray[y].UnsetCurrentOrder();
		}
		//else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::SENTRY_SHIP_ORDER)
			//Do nothing for this and all following rounds until an explicit player input.
		else if (m_ShipArray[y].GetCurrentOrder() == SHIP_ORDER::REPAIR)
		{
			//The actual Hull reparing is currenty done in CalcShipMovement(),
			//after the call to this function.
		}

		// Vor der Schiffsbewegung aber nach einer m�glichen Demontage dort ?berall einen ShipPort setzen wo
		// eine Sternbasis oder ein Au?enposten steht
		if (m_ShipArray[y].GetShipType() == SHIP_TYPE::OUTPOST || m_ShipArray[y].GetShipType() == SHIP_TYPE::STARBASE)
		{
			pSector->SetShipPort(TRUE, m_ShipArray[y].GetOwnerOfShip());
		}
	}

	// jetzt alle durch einen Sternbasisbau verschwundenen Aussenposten aus dem Feld entfernen
	for (unsigned int i = 0; i < vRemoveableOutposts.size(); i++)
	{
		for (int y = 0; y < m_ShipArray.GetSize(); y++)
		{
			if (vRemoveableOutposts[i] == m_ShipArray.GetAt(y).GetShipName())
			{
				m_ShipArray.RemoveAt(y);
				break;
			}
		}
	}
}

/// Diese Funktion berechnet die Schiffsbewegung und noch weitere kleine Sachen im Zusammenhang mit Schiffen.
void CBotf2Doc::CalcShipMovement()
{
	// CHECK WW: Das kann theoretisch weg, die Diplomatie wird erst nach der Bewegung berechnet
	// Jetzt die Starmap abgleichen, das wir nicht auf Gebiete fliegen k�nnen, wenn wir einen NAP mit einer Rasse haben
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	bool bAnomaly = false;
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		// Schiffunterst�tzungskosten auf NULL setzen
		pMajor->GetEmpire()->SetShipCosts(0);
		set<CString> races;
		for (map<CString, CMajor*>::const_iterator itt = pmMajors->begin(); itt != pmMajors->end(); ++itt)
			if (it->first != itt->first && pMajor->GetAgreement(itt->first) == DIPLOMATIC_AGREEMENT::NAP)
				races.insert(itt->first);
		pMajor->GetStarmap()->SynchronizeWithMap(m_Sectors, &races);
	}

	// Hier kommt die Schiffsbewegung (also keine anderen Befehle werden hier noch ausgewertet, lediglich wird �berpr�ft,
	// dass manche Befehle noch ihre G�ltigkeit haben
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
	{
		CShip* pShip = &m_ShipArray[y];

		// Pr�fen, dass ein Terraformbefehl noch g�ltig ist
		if (pShip->GetCurrentOrder() == SHIP_ORDER::TERRAFORM)
		{
			CPoint p = pShip->GetKO();
			if (GetSector(pShip->GetKO()).GetPlanet(pShip->GetTerraformingPlanet())->GetTerraformed() == TRUE)
			{
				pShip->SetCurrentOrder(SHIP_ORDER::AVOID);
				pShip->SetTerraformingPlanet(-1);
			}
		}
		// Pr�fen, dass ein Aussenpostenbaubefehl noch g�ltig ist
		else if (pShip->GetCurrentOrder() == SHIP_ORDER::BUILD_OUTPOST)
		{
			if (GetSector(pShip->GetKO()).GetOutpost(pShip->GetOwnerOfShip()) == TRUE)
				pShip->SetCurrentOrder(SHIP_ORDER::AVOID);
		}
		// Pr�fen, dass ein Sternbasenbaubefehl noch g�ltig ist
		else if (pShip->GetCurrentOrder() == SHIP_ORDER::BUILD_STARBASE)
		{
			if (GetSector(pShip->GetKO()).GetStarbase(pShip->GetOwnerOfShip()) == TRUE)
				pShip->SetCurrentOrder(SHIP_ORDER::AVOID);
		}
		// weiter mit Schiffsbewegung
		Sector shipKO((char)pShip->GetKO().x,(char)pShip->GetKO().y);
		Sector targetKO((char)pShip->GetTargetKO().x,(char)pShip->GetTargetKO().y);
		Sector nextKO(-1,-1);

		if (shipKO == targetKO)
			targetKO = Sector(-1,-1);

		// Weltraummonster gesondert behandeln
		if(pShip->GetAlienType()!=ALIEN_TYPE::NONE)
		{
			// wenn bei einem Weltraummonster kein Ziel vorhanden ist, dann wird zuf�llig ein neues generiert
			if (targetKO.x == -1)
			{
				// irgend ein zuf�lliges neues Ziel generieren, welches nicht auf einer Anomalie endet
				while (true)
				{
					targetKO = Sector(rand()%STARMAP_SECTORS_HCOUNT, rand()%STARMAP_SECTORS_VCOUNT);
					if (targetKO == shipKO)
						continue;

					if (GetSector(targetKO.x, targetKO.y).GetAnomaly())
						continue;

					pShip->SetTargetKO(CPoint(targetKO.x, targetKO.y), 0);
					break;
				}
			}
			// nur in ca. jeder dritten Runde fliegt das Weltraummonster weiter
			else if (rand()%3 != 0)
			{
				targetKO = Sector(-1,-1);
			}
		}

		if (targetKO.x != -1)
		{
			char range;
			char speed;
			// Unterscheiden, ob das Schiff eine Flotte anf�hrt oder nicht
			if (pShip->GetFleet() != 0)
			{
				range = (char)(3 - pShip->GetFleet()->GetFleetRange(&m_ShipArray[y]));
				speed = (char)(pShip->GetFleet()->GetFleetSpeed(&m_ShipArray[y]));
			}
			else
			{
				range = (char)(3 - pShip->GetRange());
				speed = (char)(pShip->GetSpeed());
			}

			CRace* pRace = NULL;

			// Weltraummonster gesondert behandeln
			if (pShip->GetShipType() == SHIP_TYPE::ALIEN)
			{
				CStarmap* pStarmap = new CStarmap(0);
				pStarmap->SetFullRangeMap();

				// Anomalien werden schon beachtet, da dies eine statische Variable ist und in NextRound() schon
				// berechnet wurde.
				nextKO = pStarmap->CalcPath(shipKO,targetKO,range,speed,*pShip->GetPath());

				delete pStarmap;
				pStarmap = NULL;
			}
			else
			{
				pRace = m_pRaceCtrl->GetRace(pShip->GetOwnerOfShip());
				if (pRace != NULL && pRace->GetType() == MAJOR)
				{
					nextKO = dynamic_cast<CMajor*>(pRace)->GetStarmap()->CalcPath(shipKO,targetKO,range,speed,*pShip->GetPath());
				}
			}

			// Ziel zum Anfliegen vorhanden
			if (nextKO != Sector(-1,-1))
			{
				pShip->SetKO(CPoint((int)nextKO.x,(int)nextKO.y));
				// Die Anzahl speed ersten Felder in Pfad des Schiffes l?schen
				if (nextKO == targetKO)
				{
					pShip->GetPath()->RemoveAll();
					pShip->SetTargetKO(CPoint(-1,-1), 0);
				}
				if (pRace != NULL && pRace->GetType() == MAJOR && !(this->GetSector(nextKO.x,nextKO.y).GetFullKnown(pShip->GetOwnerOfShip()))) //Berechnet Zufalls entdeckung in dem Sector den das Schiff anfliegt
					m_RandomEventManager.CalcExploreEvent(CPoint((int)nextKO.x,(int)nextKO.y),dynamic_cast<CMajor*>(pRace),&m_ShipArray);

				int high = speed;
				while (high > 0 && high < pShip->GetPath()->GetSize())
				{
					pShip->GetPath()->RemoveAt(0);
					high--;
				}
			}
		}

		// Gibt es eine Anomalie, wodurch die Schilde schneller aufgeladen werden
		bool bFasterShieldRecharge = false;
		if (GetSector(pShip->GetKO()).GetAnomaly())
			if (GetSector(pShip->GetKO()).GetAnomaly()->GetType() == BINEBULA)
				bFasterShieldRecharge = true;

		// Nach der Bewegung, aber noch vor einem m�glichen Kampf werden die Schilde nach ihrem Typ wieder aufgeladen,
		// wenn wir auf einem Shipport sind, dann wird auch die H�lle teilweise wieder repariert
		//FIXME: The shipports are not yet updated for changes due to diplomacy at this spot.
		//If we declared war and are on a shipport of the former friend, the ship is repaired,
		//and a possible repair command isn't unset though it can no longer be set by the player this turn then.
		pShip->Repair(GetSector(pShip->GetKO()).GetShipPort(pShip->GetOwnerOfShip()), bFasterShieldRecharge);
		// Befehle an alle Schiffe in der Flotte weitergeben
		if (pShip->GetFleet())
			pShip->GetFleet()->AdoptCurrentOrders(&m_ShipArray[y]);

		// wenn eine Anomalie vorhanden, deren m?gliche Auswirkungen auf das Schiff berechnen
		if (GetSector(pShip->GetKO()).GetAnomaly())
		{
			GetSector(pShip->GetKO()).GetAnomaly()->CalcShipEffects(&m_ShipArray[y]);
			bAnomaly = true;
		}
	}


	if (!bAnomaly)
		return;

	// pr�fen ob irgendwelche Schiffe durch eine Anomalie zerst�rt wurden
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		CShip* pShip = &m_ShipArray.GetAt(i);
		// Wenn das Schiff eine Flotte hatte, dann erstmal nur die Schiffe in der Flotte beachten
		// Wenn davon welche zerst�rt wurden diese aus der Flotte nehmen
		if (pShip->GetFleet())
		{
			for (int x = 0; x < pShip->GetFleet()->GetFleetSize(); x++)
			{
				if (pShip->GetFleet()->GetShipFromFleet(x)->GetHull()->GetCurrentHull() < 1)
				{
					// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
					CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(pShip->GetOwnerOfShip()));
					if (pMajor)
					{
						// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
						AddToLostShipHistory(pShip->GetFleet()->GetShipFromFleet(x), GetSector(pShip->GetKO()).GetAnomaly()->GetMapName(pShip->GetKO()), CResourceManager::GetString("DESTROYED"));

						CString sShip;
						sShip.Format("%s (%s)", pShip->GetFleet()->GetShipFromFleet(x)->GetShipName(), pShip->GetFleet()->GetShipFromFleet(x)->GetShipTypeAsString());
						CString s = CResourceManager::GetString("ANOMALY_SHIP_LOST", FALSE, sShip, GetSector(pShip->GetKO()).GetAnomaly()->GetMapName(pShip->GetKO()));
						CMessage message;
						message.GenerateMessage(s, MESSAGE_TYPE::MILITARY, "", 0, 0);
						pMajor->GetEmpire()->AddMessage(message);
						if (pMajor->IsHumanPlayer())
						{
							network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
							m_iSelectedView[client] = EMPIRE_VIEW;
						}
					}
					pShip->GetFleet()->RemoveShipFromFleet(x--);
				}
			}
			pShip->CheckFleet();
		}

		// Wenn das Schiff selbst zerst�rt wurde
		if (pShip->GetHull()->GetCurrentHull() < 1)
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(pShip->GetOwnerOfShip()));
			if (pMajor)
			{
				// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
				AddToLostShipHistory(pShip, GetSector(pShip->GetKO()).GetAnomaly()->GetMapName(pShip->GetKO()), CResourceManager::GetString("DESTROYED"));

				CString sShip;
				sShip.Format("%s (%s)", pShip->GetShipName(), pShip->GetShipTypeAsString());
				CString s = CResourceManager::GetString("ANOMALY_SHIP_LOST", FALSE, sShip, GetSector(pShip->GetKO()).GetAnomaly()->GetMapName(pShip->GetKO()));
				CMessage message;
				message.GenerateMessage(s, MESSAGE_TYPE::MILITARY, "", 0, 0);
				pMajor->GetEmpire()->AddMessage(message);
				if (pMajor->IsHumanPlayer())
				{
					network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
					m_iSelectedView[client] = EMPIRE_VIEW;
				}
			}

			RemoveShip(i--);
		}
	}
}

/// Funktion �berpr�ft, ob irgendwo ein Schiffskampf stattfindet. Wenn ja und es sind menschliche Spieler beteiligt,
/// so werden ihnen alle jeweils beteiligten Schiffe geschickt, so dass sie dort Befehle geben k�nnen.
bool CBotf2Doc::IsShipCombat()
{
	using namespace network;
	m_bCombatCalc = false;

	// Jetzt gehen wir nochmal alle Sektoren durch, wenn in einem Sektor Schiffe mehrerer verschiedener Rassen sind,
	// die Schiffe nicht auf Meiden gestellt sind und die Rassen untereinander nicht alle mindst. einen Freundschafts-
	// vertrag haben, dann kommt es in diesem Sektor zum Kampf
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
	{
		// Wenn unser Schiff auf Angreifen gestellt ist
		if (m_ShipArray.GetAt(y).GetCurrentOrder() != SHIP_ORDER::ATTACK)
			continue;

		CPoint p = m_ShipArray.GetAt(y).GetKO();
		// Wenn in dem Sektor des Schiffes schon ein Kampf stattgefunden hat, dann findet hier keiner mehr statt
		if (m_sCombatSectors.find(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName(TRUE)) != m_sCombatSectors.end())
			continue;

		// Wenn noch kein Kampf in dem Sektor stattfand, dann kommt es m�glicherweise hier zum Kampf
		for (int i = 0; i < m_ShipArray.GetSize(); i++)
		{
			// Wenn das Schiff nicht unserer Rasse geh�rt
			if (m_ShipArray.GetAt(i).GetOwnerOfShip() != m_ShipArray.GetAt(y).GetOwnerOfShip())
				// Wenn das Schiff sich im gleichen Sektor befindet
				if (m_ShipArray.GetAt(i).GetKO() == m_ShipArray.GetAt(y).GetKO())
				{
					CRace* pRace1 = m_pRaceCtrl->GetRace(m_ShipArray.GetAt(y).GetOwnerOfShip());
					CRace* pRace2 = m_pRaceCtrl->GetRace(m_ShipArray.GetAt(i).GetOwnerOfShip());
					// Wenn sich die Rassen aus diplomatischer Beziehung heraus angreifen k�nnen
					if (CCombat::CheckDiplomacyStatus(pRace1, pRace2))
					{
						m_bCombatCalc = true;
						m_ptCurrentCombatSector = p;
						m_sCombatSectors.insert(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName(TRUE));
						m_mCombatOrders.clear();
						MYTRACE("general")(MT::LEVEL_INFO, "Combat in Sector %d/%d\n", p.x, p.y);
						return true;
					}
				}
		}
	}

	// Wenn nur die KI am Kampf beteiligt ist, so kann der Kampf gleich berechnet werden
	// Dadurch m�ssen die Daten nicht immer an die ganzen Clients geschickt werden
	// TODO

	return false;
}

/// Diese Funktion berechnet einen m�glichen Weltraumkampf und dessen Auswirkungen.
void CBotf2Doc::CalcShipCombat()
{
	if (!m_bCombatCalc)
		return;

	// Alle Schiffe im zuvor berechneten Kampfsektor holen
	CArray<CShip*> vInvolvedShips;
	CPoint p = m_ptCurrentCombatSector;
	// Jetzt gehen wir nochmal alle Sektoren durch und schauen ob ein Schiff im Kampfsektor ist
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		CShip* pShip = &m_ShipArray[i];
		if (pShip->GetKO() != m_ptCurrentCombatSector)
			continue;

		vInvolvedShips.Add(pShip);

		// Wenn das Schiff eine Flotte anf�hrt, dann auch die Zeiger auf die Schiffe in der Flotte reingeben
		if (pShip->GetFleet())
			for (int j = 0; j < pShip->GetFleet()->GetFleetSize(); j++)
				vInvolvedShips.Add(pShip->GetFleet()->GetShipFromFleet(j));
		// CHECK WW:
		// folgendes Zeug kann weg, wenn der Kampf vor der Systemberechnung dramkommt

		// Haben wir einen Outpost oder eine Starbase hinzugef�gt, dann entfernen wir erstmal
		// die Auswirkungen dieser Stationen. Diese werden sp�ter wieder hinzugef�gt, falls
		// die Station nicht zerst�rt wurde.
		/*if (pShip->GetShipType() == OUTPOST || pShip->GetShipType() == STARBASE)
		{
			m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetShipPort(FALSE, pShip->GetOwnerOfShip());
			m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetOutpost(FALSE,  pShip->GetOwnerOfShip());
			m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetStarbase(FALSE, pShip->GetOwnerOfShip());
		}
		// Haben wir aber in diesem Sektor ein System mit aktiver Werft stehen, dann f�gen
		// wir den ShipPort aber gleich jetzt wieder hinzu
		if (m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetShipYard() == TRUE && m_Systems.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == pShip->GetOwnerOfShip())
			m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).SetShipPort(TRUE, pShip->GetOwnerOfShip());*/
	}

	// es sollten immer Schiffe im Array sein, sonst h�tte in diesem Sektor kein Kampf stattfinden d�rfen
	ASSERT(vInvolvedShips.GetSize());
	if (vInvolvedShips.GetSize() == 0)
		return;

	// Kampf-KI
	CCombatAI AI;
	bool bCombat = AI.CalcCombatTactics(vInvolvedShips, m_pRaceCtrl->GetRaces(), m_mCombatOrders, m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetAnomaly());
	if (!bCombat)
		return;

	// Jetzt k�nnen wir einen Kampf stattfinden lassen
	CCombat Combat;
	Combat.SetInvolvedShips(&vInvolvedShips, m_pRaceCtrl->GetRaces(), m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetAnomaly());
	if (!Combat.GetReadyForCombat())
		return;

	Combat.PreCombatCalculation();
	map<CString, BYTE> winner;
	// Kampf berechnen
	Combat.CalculateCombat(winner);

	// M�glichen R�ckzugssektor f�r Rasse aus diesem Kampf ermitteln
	// Diese Schiffe werden auf einem zuf�lligen Feld um den Kampfsektor platziert
	for (map<CString, COMBAT_ORDER::Typ>::const_iterator it = m_mCombatOrders.begin(); it != m_mCombatOrders.end(); ++it)
	{
		CPoint pt = m_ptCurrentCombatSector;
		pair<int, int> ptCombatSector(pt.x, pt.y);
		CString sRace = it->first;

		// Zufallig einen Sektor um den Kampfsektor ermitteln.
		while (true)
		{
			int x = rand()%3 - 1;
			int y = rand()%3 - 1;
			if (CPoint(pt.x + x, pt.y + y) != pt && pt.y + y < STARMAP_SECTORS_VCOUNT && pt.y + y > -1 && pt.x + x < STARMAP_SECTORS_HCOUNT && pt.x + x > -1)
			{
				CPoint ptRetreatSector = CPoint(pt.x + x, pt.y + y);

				// ermittelten R�ckzugssektor f�r diese Rasse in diesem Sektor festlegen
				m_mShipRetreatSectors[sRace][ptCombatSector] = ptRetreatSector;
				break;
			}
		}
	}

	map<CString, CRace*>* pmRaces = m_pRaceCtrl->GetRaces();
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();

	for (map<CString, CRace*>::const_iterator it = pmRaces->begin(); it != pmRaces->end(); ++it)
	{
		CString sSectorName;
		// ist der Sektor bekannt?
		if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetKnown(it->first))
			sSectorName = m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName(true);
		else
			sSectorName.Format("%s %c%i", CResourceManager::GetString("SECTOR"), (char)(p.y+97), p.x + 1);

		// gewonnen
		if (winner[it->first] == 1 && it->second->GetType() == MAJOR)
		{
			// dem Siegbedingungs�berwacher den Sieg mitteilen
			m_VictoryObserver.AddCombatWin(it->first);

			CMajor* pMajor = dynamic_cast<CMajor*>(it->second);
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			CMessage message;
			message.GenerateMessage(CResourceManager::GetString("WIN_COMBAT", false, sSectorName), MESSAGE_TYPE::MILITARY, "", NULL, FALSE);
			pMajor->GetEmpire()->AddMessage(message);
			// win a minor battle
			CString eventText = pMajor->GetMoralObserver()->AddEvent(3, pMajor->GetRaceMoralNumber());
			message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, "", 0, 0);
			pMajor->GetEmpire()->AddMessage(message);
			if (pMajor->IsHumanPlayer())
				m_iSelectedView[client] = EMPIRE_VIEW;
		}
		// verloren
		else if (winner[it->first] == 2)
		{
			if (it->second->GetType() == MAJOR)
			{
				CMajor* pMajor = dynamic_cast<CMajor*>(it->second);
				ASSERT(pMajor);
				network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

				CMessage message;
				message.GenerateMessage(CResourceManager::GetString("LOSE_COMBAT", false, sSectorName), MESSAGE_TYPE::MILITARY, "", 0,0);
				pMajor->GetEmpire()->AddMessage(message);
				// lose a minorbattle
				CString eventText = pMajor->GetMoralObserver()->AddEvent(6, pMajor->GetRaceMoralNumber());
				message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, "", 0, 0);
				pMajor->GetEmpire()->AddMessage(message);
				if (pMajor->IsHumanPlayer())
					m_iSelectedView[client] = EMPIRE_VIEW;
			}
			// Die Beziehung zum Gewinner verschlechtert sich dabei. Treffen zwei computergesteuerte Rassen
			// aufeinander, so ist die Beziehungsveringerung geringer
			for (map<CString, CMajor*>::const_iterator itt = pmMajors->begin(); itt != pmMajors->end(); ++itt)
			{
				if (it->first != itt->first && winner[itt->first] == 1)
				{
					CMajor* pMajorWin = itt->second;
					if (pMajorWin->IsHumanPlayer() == false)
						it->second->SetRelation(pMajorWin->GetRaceID(), -(rand()%4));
					else
						it->second->SetRelation(pMajorWin->GetRaceID(), -(rand()%6 + 5));
				}
			}
		}
		// unentschieden
		else if (winner[it->first] == 3 && it->second->GetType() == MAJOR)
		{
			CMajor* pMajor = dynamic_cast<CMajor*>(it->second);
			ASSERT(pMajor);
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());

			CMessage message;
			message.GenerateMessage(CResourceManager::GetString("DRAW_COMBAT", false, sSectorName), MESSAGE_TYPE::MILITARY, "", 0,0);
			pMajor->GetEmpire()->AddMessage(message);
			if (pMajor->IsHumanPlayer())
				m_iSelectedView[client] = EMPIRE_VIEW;
		}
	}

	// Nach einem Kampf mu� ich das Feld der Schiffe durchgehen und alle Schiffe aus diesem nehmen, die
	// keine H�lle mehr besitzen. Aufpassen mu� ich dabei, wenn das Schiff eine Flotte anf�hrte
	CStringArray destroyedShips;
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		if (m_ShipArray.GetAt(i).GetKO() != m_ptCurrentCombatSector)
			continue;
		// Wenn das Schiff eine Flotte hatte, dann erstmal nur die Schiffe in der Flotte beachten
		// Wenn davon welche zerst�rt wurden diese aus der Flotte nehmen
		if (m_ShipArray.GetAt(i).GetFleet())
		{
			for (int x = 0; x < m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); x++)
				if (m_ShipArray.GetAt(i).GetFleet()->GetShipFromFleet(x)->GetHull()->GetCurrentHull() < 1)
				{
					// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
					AddToLostShipHistory(m_ShipArray[i].GetFleet()->GetShipFromFleet(x), CResourceManager::GetString("COMBAT"), CResourceManager::GetString("DESTROYED"));
					destroyedShips.Add(m_ShipArray[i].GetFleet()->GetShipFromFleet(x)->GetShipName()+" ("+m_ShipArray[i].GetFleet()->GetShipFromFleet(x)->GetShipTypeAsString()+")");

					// Wenn es das Flagschiff war, so ein Event �ber dessen Verlust hinzuf�gen
					if (m_ShipArray[i].GetFleet()->GetShipFromFleet(x)->GetIsShipFlagShip())
					{
						CRace* pOwner = m_pRaceCtrl->GetRace(m_ShipArray[i].GetFleet()->GetShipFromFleet(x)->GetOwnerOfShip());
						if (pOwner && pOwner->GetType() == MAJOR)
						{
							CMajor* pMajor = dynamic_cast<CMajor*>(pOwner);
							CString eventText = pMajor->GetMoralObserver()->AddEvent(7, pMajor->GetRaceMoralNumber(), m_ShipArray[i].GetFleet()->GetShipFromFleet(x)->GetShipName());
							CMessage message;
							message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, "", 0, 0);
							pMajor->GetEmpire()->AddMessage(message);
						}
					}

					m_ShipArray[i].GetFleet()->RemoveShipFromFleet(x--);
				}
			m_ShipArray[i].CheckFleet();
		}

		// Wenn das Schiff selbst zerst�rt wurde
		if (m_ShipArray.GetAt(i).GetHull()->GetCurrentHull() < 1)
		{
			// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
			AddToLostShipHistory(&m_ShipArray[i], CResourceManager::GetString("COMBAT"), CResourceManager::GetString("DESTROYED"));
			destroyedShips.Add(m_ShipArray[i].GetShipName()+" ("+m_ShipArray[i].GetShipTypeAsString()+")");
			// Wenn es das Flagschiff war, so ein Event �ber dessen Verlust hinzuf�gen
			if (m_ShipArray[i].GetIsShipFlagShip())
			{
				CRace* pOwner = m_pRaceCtrl->GetRace(m_ShipArray[i].GetOwnerOfShip());
				if (pOwner && pOwner->GetType() == MAJOR)
				{
					CMajor* pMajor = dynamic_cast<CMajor*>(pOwner);
					CString eventText = pMajor->GetMoralObserver()->AddEvent(7, pMajor->GetRaceMoralNumber(), m_ShipArray[i].GetShipName());
					CMessage message;
					message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, "", 0, 0);
					pMajor->GetEmpire()->AddMessage(message);
				}
			}
			// Wenn es ein Au�enposten oder Sternbasis war, so ein Event �ber dessen Verlust hinzuf�gen
			if (m_ShipArray[i].GetShipType() == SHIP_TYPE::OUTPOST || m_ShipArray[i].GetShipType() == SHIP_TYPE::STARBASE)
			{
				CRace* pOwner = m_pRaceCtrl->GetRace(m_ShipArray[i].GetOwnerOfShip());
				if (pOwner && pOwner->GetType() == MAJOR)
				{
					CMajor* pMajor = dynamic_cast<CMajor*>(pOwner);
					CString eventText;
					if (m_ShipArray[i].GetShipType() == SHIP_TYPE::OUTPOST)
						eventText = pMajor->GetMoralObserver()->AddEvent(8, pMajor->GetRaceMoralNumber());
					else
						eventText = pMajor->GetMoralObserver()->AddEvent(9, pMajor->GetRaceMoralNumber());
					CMessage message;
					message.GenerateMessage(eventText, MESSAGE_TYPE::MILITARY, "", 0, 0);
					pMajor->GetEmpire()->AddMessage(message);
				}
			}

			RemoveShip(i--);
		}
	}

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		// Hat die Rasse an dem Kampf teilgenommen, also gewonnen oder verloren oder unentschieden
		if (winner[it->first] != 0)
			for (int j = 0; j < destroyedShips.GetSize(); j++)
			{
				CString s;
				s.Format("%s", CResourceManager::GetString("DESTROYED_SHIPS_IN_COMBAT",0,destroyedShips[j]));
				CMessage message;
				message.GenerateMessage(s, MESSAGE_TYPE::MILITARY, "", 0, 0);
				pMajor->GetEmpire()->AddMessage(message);
			}
	}

	// allen Schiffen mit R�ckzugsbfehl den aktuellen Befehl zur�cknehmen
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		CShip* pShip = &m_ShipArray[i];
		// Hat das Schiff den R�ckzugsbefehl
		if (pShip->GetCombatTactic() == COMBAT_TACTIC::CT_RETREAT)
		{
			// Schiff auf Meiden stellen
			if (pShip->IsNonCombat())
				pShip->SetCurrentOrder(SHIP_ORDER::AVOID);
			else
				pShip->SetCurrentOrder(SHIP_ORDER::ATTACK);

			// wom�gicher Terraformplanet oder Stationsbau zur�cknehmen
			pShip->SetTerraformingPlanet(-1);
		}
	}
}

/////BEGIN: HELPER FUNCTIONS FOR void CBotf2Doc::CalcShipEffects()
void CBotf2Doc::CalcShipRetreat() {
	// Schiffe mit R�ckzugsbefehl auf ein Feld neben dem aktuellen Feld setzen
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		CShip* pShip = &m_ShipArray.GetAt(i);
		const CString& ship_owner = pShip->GetOwnerOfShip();
		// Hat das Schiff den R�ckzugsbefehl
		if (pShip->GetCombatTactic() != COMBAT_TACTIC::CT_RETREAT)
			continue;

		// R�ckzugsbefehl zur�cknehmen
		pShip->SetCombatTactic(COMBAT_TACTIC::CT_ATTACK);
		// Schiff auf Meiden/Angriff stellen entsprechend seinem Typ
		pShip->UnsetCurrentOrder();

		// wom�gicher Terraformplanet oder Stationsbau zur�cknehmen
		pShip->SetTerraformingPlanet(-1);

		// R�ckzugssektor f�r dieses Schiff in diesem Sektor holen
		if (m_mShipRetreatSectors.find(ship_owner) == m_mShipRetreatSectors.end())
			continue;

		const CPoint& co = pShip->GetKO();
		const pair<int, int> CurrentSector(co.x, co.y);
		if (m_mShipRetreatSectors[ship_owner].find(CurrentSector)
			== m_mShipRetreatSectors[ship_owner].end())
			continue;

		CPoint ptRetreatSector = m_mShipRetreatSectors[ship_owner][CurrentSector];
		// Kann das Schiff �berhaupt fliegen?
		if (pShip->GetSpeed() > 0)
		{
			pShip->SetKO(ptRetreatSector);
			// aktuell eingestellten Kurs l�schen (nicht das das Schiff wieder in den Gefahrensektor fliegt)
			pShip->SetTargetKO(CPoint(-1, -1), 0);
		}

		// sind alle Schiffe in einer Flotte im R�ckzug, so kann die ganze Flotte
		// in den R�ckzugssektor
		bool bCompleteFleetRetreat = true;
		pShip->CheckFleet();
		if (pShip->GetFleet())
		{
			for (int j = 0; j < pShip->GetFleet()->GetFleetSize(); j++)
			{
				if (pShip->GetFleet()->GetShipFromFleet(j)->GetCombatTactic() != COMBAT_TACTIC::CT_RETREAT
					|| pShip->GetFleet()->GetFleetSpeed(pShip) == 0)
				{
					bCompleteFleetRetreat = false;
					break;
				}
			}
		}

		// haben alle Schiffe in der Flotte den R�ckzugsbefehl oder hat das Schiff keine Flotte
		// -> R�ckzugssektor festlegen
		if (bCompleteFleetRetreat)
		{
			// R�ckzugsbefehl in Flotte zur�cknehmen
			if (pShip->GetFleet())
				for (int j = 0; j < pShip->GetFleet()->GetFleetSize(); j++)
				{
					CShip* pFleetShip = pShip->GetFleet()->GetShipFromFleet(j);
					pFleetShip->SetCombatTactic(COMBAT_TACTIC::CT_ATTACK);

					// Schiff auf Meiden/Angriff stellen entsprechend seinem Typ
					pFleetShip->UnsetCurrentOrder();

					if (pFleetShip->GetSpeed() > 0)
					{
						pFleetShip->SetKO(ptRetreatSector);
						// aktuell eingestellten Kurs l�schen (nicht das das Schiff wieder in den Gefahrensektor fliegt)
						pFleetShip->SetTargetKO(CPoint(-1, -1), 0);
					}

					// wom�gicher Terraformplanet oder Stationsbau zur�cknehmen
					pFleetShip->SetTerraformingPlanet(-1);
				}
		}
		// Schiffe aus der Flotte nehmen und ans Ende des Schiffsarrays packen. Diese werden
		// dann auch noch behandelt
		else
		{
			// nicht mehr auf pShip arbeiten, da sich der Vektor hier ver�ndern kann und somit
			// der Zeiger nicht mehr gleich ist!
			if (m_ShipArray[i].GetFleet())
				while (m_ShipArray[i].GetFleet()->GetFleetSize())
				{
					m_ShipArray.Add(*m_ShipArray[i].GetFleet()->GetShipFromFleet(0));;
					m_ShipArray[i].GetFleet()->RemoveShipFromFleet(0);
				}
			m_ShipArray[i].DeleteFleet();
		}
	}//	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	m_mShipRetreatSectors.clear();
}

//most of the stuff from CalcShipEffects() for either a ship from the shiparray or a ship of its fleet
void CBotf2Doc::CalcShipEffectsForSingleShip(CShip& ship, CSector& sector, CMajor* pMajor, const CString& sRace,
			bool bDeactivatedShipScanner, bool bBetterScanner, bool fleetship) {
	// nur wenn das Schiff von einer Majorrace ist
	if (pMajor) {
		if(!fleetship)
			sector.SetFullKnown(sRace);
		if (!bDeactivatedShipScanner)
			// Scanst�rke auf die Sektoren abh�ngig von der Scanrange �bertragen
			PutScannedSquareOverCoords(ship.GetKO(), ship.GetScanRange(), ship.GetScanPower(),
				sRace, true, bBetterScanner, ship.HasSpecial(SHIP_SPECIAL::PATROLSHIP));
	}
	// Schiffe, wenn wir dort nicht eine ausreichend hohe Scanpower haben. Ab Stealthstufe 4 muss das Schiff getarnt
	// sein, ansonsten gilt dort nur Stufe 3.
	if (!ship.IsBase()) {
		// Im Sektor die NeededScanPower setzen, die wir brauchen um dort Schiffe zu sehen. Wir sehen ja keine getarnten
		// Schiffe, wenn wir dort nicht eine ausreichend hohe Scanpower haben. Ab Stealthstufe 4 muss das Schiff getarnt
		// sein, ansonsten gilt dort nur Stufe 3.
		short stealthPower = ship.GetStealthPower();
		if(!ship.GetCloak() && stealthPower > 3)
			stealthPower = 3;
		const short NeededScanPower = stealthPower * 20;
		if (NeededScanPower < sector.GetNeededScanPower(sRace))
			sector.SetNeededScanPower(NeededScanPower, sRace);
	}
	if(!fleetship) {
		// Wenn das Schiff gerade eine Station baut, so dies dem Sektor mitteilen
		const SHIP_ORDER::Typ current_order = ship.GetCurrentOrder();
		if (current_order == SHIP_ORDER::BUILD_OUTPOST || current_order == SHIP_ORDER::BUILD_STARBASE)
			sector.SetIsStationBuilding(TRUE, sRace);
		// Wenn das Schiff gerade Terraform, so dies dem Planeten mitteilen
		else if (current_order == SHIP_ORDER::TERRAFORM) {
			const short nPlanet = ship.GetTerraformingPlanet();
			std::vector<CPlanet>& planets = sector.GetPlanets();
			if (nPlanet != -1 && nPlanet < static_cast<int>(planets.size()))
				planets.at(nPlanet).SetIsTerraforming(TRUE);
			else {
				ship.SetTerraformingPlanet(-1);
				ship.SetCurrentOrder(SHIP_ORDER::AVOID);
			}
		}
	}
	if (pMajor) {
		// Schiffunterst�tzungkosten dem jeweiligen Imperium hinzuf�gen.
		pMajor->GetEmpire()->AddShipCosts(ship.GetMaintenanceCosts());
		// die Schiffe in der Flotte beim modifizieren der Schiffslisten der einzelnen Imperien beachten
		pMajor->GetShipHistory()->ModifyShip(&ship, sector.GetName(TRUE));
	}
	// Erfahrunspunkte der Schiffe anpassen
	ship.CalcExp();
}
/////END: HELPER FUNCTIONS FOR void CBotf2Doc::CalcShipEffects()

/// Diese Funktion berechnet die Auswirkungen von Schiffen und Stationen auf der Karte. So werden hier z.B. Sektoren
/// gescannt, Rassen kennengelernt und die Schiffe den Sektoren bekanntgegeben.
void CBotf2Doc::CalcShipEffects()
{
	CalcShipRetreat();

	// Nach einem m�glichen Kampf, aber nat�rlich auch generell die Schiffe und Stationen den Sektoren bekanntgeben
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
	{
		const CString sRace = m_ShipArray[y].GetOwnerOfShip();
		CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sRace));
		CShip& ship = m_ShipArray.GetAt(y);
		const CPoint& p = ship.GetKO();
		CSector& sector = GetSector(p);

		// Anomalien beachten
		bool bDeactivatedShipScanner = false;
		bool bBetterScanner = false;
		const CAnomaly* const anomaly = sector.GetAnomaly();
		if(anomaly) {
			bDeactivatedShipScanner = anomaly->IsShipScannerDeactivated();
			bBetterScanner = anomaly->GetType() == QUASAR;
		}

		CalcShipEffectsForSingleShip(ship, sector, pMajor, sRace, bDeactivatedShipScanner, bBetterScanner, false);
		// wenn das Schiff eine Flotte besitzt, dann die Schiffe in der Flotte auch beachten
		CFleet* fleet = ship.GetFleet();
		if(fleet)
		{
			// Scanst�rke der Schiffe in der Flotte auf die Sektoren abh�ngig von der Scanrange �bertragen
			for (int x = 0; x < fleet->GetFleetSize(); x++)
			{
				CShip* fleetship = fleet->GetShipFromFleet(x);
				CalcShipEffectsForSingleShip(*fleetship, sector, pMajor, sRace,
					bDeactivatedShipScanner, bBetterScanner, true);
			}
		}
		// Dem Sektor nochmal bekanntgeben, dass in ihm eine Sternbasis oder ein Au�enposten steht. Weil wenn im Kampf
		// eine Station teilnahm, dann haben wir den Shipport in dem Sektor vorl�ufig entfernt. Es kann ja passieren,
		// dass die Station zerst�rt wird. Haben wir jetzt aber immernoch eine Station, dann bleibt der Shipport dort auch
		// bestehen
		if (ship.IsBase()) {
			sector.SetShipPort(TRUE, sRace);
			const SHIP_TYPE::Typ ship_type = ship.GetShipType();
			if (ship_type == SHIP_TYPE::OUTPOST)
				sector.SetOutpost(TRUE, sRace);
			else
				sector.SetStarbase(TRUE, sRace);
		}
		else {
			// Dem Sektor bekanntgeben, dass in ihm ein Schiff ist
			sector.SetOwnerOfShip(TRUE, sRace);
		}
	}
}

/// Diese Funktion �berpr�ft, ob neue Rassen kennengelernt wurden.
void CBotf2Doc::CalcContactNewRaces()
{
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();
	for (int y = 0; y < m_ShipArray.GetSize(); y++)
	{
		CPoint p = m_ShipArray[y].GetKO();
		CString sRace = m_ShipArray[y].GetOwnerOfShip();

		CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sRace));
		// kann der Major Rassen kennenlernen?
		if (pMajor && pMajor->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
			continue;

		CMinor* pMinor = NULL;
		// handelt es sich um eine Minorrace
		if (!pMajor)
			pMinor = dynamic_cast<CMinor*>(m_pRaceCtrl->GetRace(sRace));

		// kann der Minor Rassen kennenlernen?
		if (pMinor && pMinor->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
			continue;

		// Wenn dieser Sektor einer anderen Majorrace geh�rt, wir die noch nicht kannten, dann bekanntgeben
		if (pMajor != NULL && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != "" && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != sRace)
		{
			// normale Zugeh�rigkeit
			if (pMajor->IsRaceContacted(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector()) == false)
			{
				CRace* pSectorOwner = m_pRaceCtrl->GetRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector());
				if (pSectorOwner)
				{
					// kann der Sektorbesitzer andere Rassen kennenlernen?
					if (pSectorOwner->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
						continue;

					pMajor->SetIsRaceContacted(pSectorOwner->GetRaceID(), true);
					pSectorOwner->SetIsRaceContacted(pMajor->GetRaceID(), true);

					// Nachricht generieren, dass wir eine andere Rasse kennengelernt haben
					CString s;
					CString sect;
					sect.Format("%c%i",(char)(p.y+97),p.x+1);

					// der Sektor geh�rt geh�rt einem Major
					if (pSectorOwner->GetType() == MAJOR)
					{
						// Dem Sektorbesitzer eine Nachricht �ber Erstkontakt �berbringen
						s = CResourceManager::GetString("GET_CONTACT_TO_MAJOR",FALSE, pMajor->GetRaceName(),sect);
						CMessage message;
						message.GenerateMessage(s,MESSAGE_TYPE::DIPLOMACY,"",0,FALSE);
						dynamic_cast<CMajor*>(pSectorOwner)->GetEmpire()->AddMessage(message);

						// Nachricht generieren, dass wir eine Majorrace kennengelernt haben
						s = CResourceManager::GetString("GET_CONTACT_TO_MAJOR",FALSE, pSectorOwner->GetRaceName(),sect);
					}
					// der Sektor geh�rt einer Minorrace
					else
					{
						// Nachricht generieren, dass wir eine Minorrace kennengelernt haben
						s = CResourceManager::GetString("GET_CONTACT_TO_MINOR", FALSE, pSectorOwner->GetRaceName());
					}

					// dem Major, dem das Schiff geh�rt die Nachricht �berreichen
					CMessage message;
					message.GenerateMessage(s,MESSAGE_TYPE::DIPLOMACY,"",0,FALSE);
					pMajor->GetEmpire()->AddMessage(message);

					// Audiovorstellung der kennengelernten Majorrace
					if (pMajor->IsHumanPlayer() || (pSectorOwner->GetType() == MAJOR && dynamic_cast<CMajor*>(pSectorOwner)->IsHumanPlayer()))
					{
						network::RACE clientShip = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
						m_iSelectedView[clientShip] = EMPIRE_VIEW;

						// Systembesitzer ist ein Major

						if (pSectorOwner->GetType() == MAJOR)
						{
							CMajor* pSectorOwnerMajor = dynamic_cast<CMajor*>(pSectorOwner);
							network::RACE clientSystem = m_pRaceCtrl->GetMappedClientID(pSectorOwner->GetRaceID());
							m_iSelectedView[clientSystem] = EMPIRE_VIEW;

							// Systembesitzer
							if (pSectorOwnerMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_FIRSTCONTACT, clientShip, 2, 1.0f};
								m_SoundMessages[clientSystem].Add(entry);
								pSectorOwnerMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pSectorOwner->GetRaceID(), pMajor->GetRaceID()));
							}
							// Schiffsbesitzer
							if (pMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_FIRSTCONTACT, clientSystem, 2, 1.0f};
								m_SoundMessages[clientShip].Add(entry);
								pMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pMajor->GetRaceID(), pSectorOwner->GetRaceID()));
							}
						}
						// Systembesitzer ist ein Minor
						else if (pSectorOwner->GetType() == MINOR)
						{
							// Schiffsbesitzer
							if (pMajor->IsHumanPlayer())
							{
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_ALIENCONTACT, clientShip, 1, 1.0f};
								m_SoundMessages[clientShip].Add(entry);
								pMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pMajor->GetRaceID(), pSectorOwner->GetRaceID()));
							}
						}
					}
				}
			}

			// in dem Sektor lebt eine Minorrace, der der Sektor aber nicht mehr geh�rt, z.B. durch Mitgliedschaft
			if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetMinorRace())
			{
				CMinor* pMinor = m_pRaceCtrl->GetMinorRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetName());
				assert(pMinor);
				// kann der Sektorbesitzer andere Rassen kennenlernen?
				if (pMinor->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
					continue;

				// die Rasse ist noch nicht bekannt und nicht unterworfen
				if (pMajor->IsRaceContacted(pMinor->GetRaceID()) == false && pMinor->GetSubjugated() == false)
				{
					pMajor->SetIsRaceContacted(pMinor->GetRaceID(), true);
					pMinor->SetIsRaceContacted(pMajor->GetRaceID(), true);

					// Nachricht generieren, dass wir eine andere Rasse kennengelernt haben
					CString s;
					CString sect;
					sect.Format("%c%i",(char)(p.y+97),p.x+1);

					// Nachricht generieren, dass wir eine Minorrace kennengelernt haben
					s = CResourceManager::GetString("GET_CONTACT_TO_MINOR", FALSE, pMinor->GetRaceName());
					// dem Major, dem das Schiff geh�rt die Nachricht �berreichen
					CMessage message;
					message.GenerateMessage(s,MESSAGE_TYPE::DIPLOMACY,"",0,FALSE);
					pMajor->GetEmpire()->AddMessage(message);
					// Audiovorstellung der kennengelernten Majorrace
					if (pMajor->IsHumanPlayer())
					{
						network::RACE clientShip = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
						m_iSelectedView[clientShip] = EMPIRE_VIEW;

						// Systembesitzer ist ein Minor
						SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_ALIENCONTACT, clientShip, 1, 1.0f};
						m_SoundMessages[clientShip].Add(entry);
						pMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pMajor->GetRaceID(), pMinor->GetRaceID()));
					}
				}
			}
		}

		if (pMajor != NULL)
		{
			// wenn zwei Schiffe einer Rasse in diesem Sektor stationiert sind, so k�nnen sich die Besitzer auch kennenlernen
			for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
				if (pMajor->GetRaceID() != it->first)
					if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfShip(it->first) == TRUE)
						if (pMajor->IsRaceContacted(it->first) == false)
						{
							CMajor* pOtherMajor = it->second;
							// kann der andere Schiffsbesitzer Rassen kennenlernen?
							if (pOtherMajor->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
								continue;

							pMajor->SetIsRaceContacted(pOtherMajor->GetRaceID(), true);
							pOtherMajor->SetIsRaceContacted(pMajor->GetRaceID(), true);
							// Nachricht generieren, dass wir eine Majorrace kennengelernt haben
							CString s;
							CString sect; sect.Format("%c%i",(char)(p.y+97),p.x+1);
							// der Major, der das erste Schiff geh�rt
							s = CResourceManager::GetString("GET_CONTACT_TO_MAJOR", FALSE, pMajor->GetRaceName() ,sect);
							CMessage message;
							message.GenerateMessage(s, MESSAGE_TYPE::DIPLOMACY, "", 0, FALSE);
							pOtherMajor->GetEmpire()->AddMessage(message);
							// der Major, der das zweite Schiff geh�rt
							s = CResourceManager::GetString("GET_CONTACT_TO_MAJOR", FALSE, pOtherMajor->GetRaceName(), sect);
							message.GenerateMessage(s, MESSAGE_TYPE::DIPLOMACY, "", 0, FALSE);
							pMajor->GetEmpire()->AddMessage(message);
							if (pMajor->IsHumanPlayer() || pOtherMajor->IsHumanPlayer())
							{
								network::RACE clientShip = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
								network::RACE clientOther = m_pRaceCtrl->GetMappedClientID(pOtherMajor->GetRaceID());
								// Audiovorstellung der kennengelernten Majorrace
								SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_FIRSTCONTACT, clientShip, 2, 1.0f};
								m_SoundMessages[clientOther].Add(entry);
								SNDMGR_MESSAGEENTRY entry2 = {SNDMGR_MSG_FIRSTCONTACT, clientOther, 2, 1.0f};
								m_SoundMessages[clientShip].Add(entry2);
								m_iSelectedView[clientShip] = EMPIRE_VIEW;
								m_iSelectedView[clientOther] = EMPIRE_VIEW;
								// Eventscreen f�r das Kennenlernen
								if (pMajor->IsHumanPlayer())
									pMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pMajor->GetRaceID(), pOtherMajor->GetRaceID()));
								if (pOtherMajor->IsHumanPlayer())
									pOtherMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pOtherMajor->GetRaceID(), pMajor->GetRaceID()));
							}
						}
		}
		else if (pMinor != NULL)
		{
			// auf einen Sektor einer Majorrace geflogen
			if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != "" && m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() != sRace)
			{
				// normale Zugeh�rigkeit
				if (pMinor->IsRaceContacted(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector()) == false)
				{
					CMajor* pSectorOwner = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector()));
					if (pSectorOwner)
					{
						// kann der Sektorbesitzer andere Rassen kennenlernen?
						if (pSectorOwner->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
							continue;

						pMinor->SetIsRaceContacted(pSectorOwner->GetRaceID(), true);
						pSectorOwner->SetIsRaceContacted(pMinor->GetRaceID(), true);

						// Nachricht generieren, dass wir eine andere Rasse kennengelernt haben
						CString s;
						CString sect;
						sect.Format("%c%i",(char)(p.y+97),p.x+1);

						// Dem Sektorbesitzer eine Nachricht �ber Erstkontakt �berbringen
						s = CResourceManager::GetString("GET_CONTACT_TO_MINOR",FALSE, pMinor->GetRaceName(),sect);
						CMessage message;
						message.GenerateMessage(s,MESSAGE_TYPE::DIPLOMACY,"",0,FALSE);
						pSectorOwner->GetEmpire()->AddMessage(message);

						if (pSectorOwner->IsHumanPlayer())
						{
							network::RACE clientSystem = m_pRaceCtrl->GetMappedClientID(pSectorOwner->GetRaceID());
							m_iSelectedView[clientSystem] = EMPIRE_VIEW;

							SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_ALIENCONTACT, clientSystem, 1, 1.0f};
							m_SoundMessages[clientSystem].Add(entry);
							pSectorOwner->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pSectorOwner->GetRaceID(), pMinor->GetRaceID()));
						}
					}
				}
			}

			// treffen mit einem Schiff eines anderen Majors
			// wenn zwei Schiffe einer Rasse in diesem Sektor stationiert sind, so k�nnen sich die Besitzer auch kennenlernen
			for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
				if (m_Sectors.at(p.x+(p.y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfShip(it->first) == TRUE)
					if (pMinor->IsRaceContacted(it->first) == false)
					{
						CMajor* pOtherMajor = it->second;
						// kann der andere Schiffsbesitzer Rassen kennenlernen?
						if (pOtherMajor->HasSpecialAbility(SPECIAL_NO_DIPLOMACY))
							continue;

						pMinor->SetIsRaceContacted(pOtherMajor->GetRaceID(), true);
						pOtherMajor->SetIsRaceContacted(pMinor->GetRaceID(), true);
						// Nachricht generieren, dass wir eine Majorrace kennengelernt haben
						CString s;
						CString sect; sect.Format("%c%i",(char)(p.y+97),p.x+1);
						// der Major, der das erste Schiff geh�rt
						s = CResourceManager::GetString("GET_CONTACT_TO_MINOR", FALSE, pMinor->GetRaceName() ,sect);
						CMessage message;
						message.GenerateMessage(s, MESSAGE_TYPE::DIPLOMACY, "", 0, FALSE);
						pOtherMajor->GetEmpire()->AddMessage(message);

						if (pOtherMajor->IsHumanPlayer())
						{
							network::RACE clientOther = m_pRaceCtrl->GetMappedClientID(pOtherMajor->GetRaceID());
							m_iSelectedView[clientOther] = EMPIRE_VIEW;
							// Audiovorstellung der kennengelernten Minorrace
							SNDMGR_MESSAGEENTRY entry = {SNDMGR_MSG_ALIENCONTACT, clientOther, 2, 1.0f};
							m_SoundMessages[clientOther].Add(entry);
							// Eventscreen einf�gen
							pOtherMajor->GetEmpire()->GetEventMessages()->Add(new CEventFirstContact(pOtherMajor->GetRaceID(), pMinor->GetRaceID()));
						}
					}
		}
	}
}

/// Diese Funktion f�hrt allgemeine Berechnung durch, die immer zum Ende der NextRound-Calculation stattfinden m�ssen.
void CBotf2Doc::CalcEndDataForNextRound()
{
	// Nachdem Au�enposten und Sternbasen auch den Sektoren wieder bekanntgegeben wurden, k�nnen wir die Besitzerpunkte
	// f�r die Sektoren berechnen.
	map<CString, CMajor*>* pmMajors = m_pRaceCtrl->GetMajors();

	// ausgel�schte Hauptrassen behandeln
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		pMajor->GetEmpire()->GenerateSystemList(m_Systems, m_Sectors);
		pMajor->GetEmpire()->SetNumberOfSystems(pMajor->GetEmpire()->GetSystemList()->GetSize());

		// Wenn das Imperium keine Systeme mehr besitzt, so wird es f�r alle anderen Rassen auf unbekannt gestellt.
		// Scheidet somit aus dem Spiel aus
		if (pMajor->GetEmpire()->GetNumberOfSystems() == 0)
		{
			// Allen anderen bekannten Imperien die Nachricht zukommen lassen, dass die Rasse vernichtet wurde
			for (map<CString, CMajor*>::const_iterator itt = pmMajors->begin(); itt != pmMajors->end(); ++itt)
			{
				if (pMajor->GetRaceID() == itt->first)
					continue;

				if (itt->second->IsRaceContacted(pMajor->GetRaceID()))
				{
					// Nachricht �ber Rassenausl�schung (hier die gleiche wie bei Minorausl�schung
					CString news = CResourceManager::GetString("ELIMINATE_MINOR", FALSE, pMajor->GetRaceName());
					CMessage message;
					message.GenerateMessage(news, MESSAGE_TYPE::SOMETHING, "", 0, 0);
					itt->second->GetEmpire()->AddMessage(message);
					if (itt->second->IsHumanPlayer())
					{
						// Event �ber die Rassenausl�schung einf�gen
						CEventRaceKilled* eventScreen = new CEventRaceKilled(itt->first, pMajor->GetRaceID(), pMajor->GetRaceName(), pMajor->GetGraphicFileName());
						itt->second->GetEmpire()->GetEventMessages()->Add(eventScreen);

						network::RACE client = m_pRaceCtrl->GetMappedClientID(itt->first);
						m_iSelectedView[client] = EMPIRE_VIEW;
					}
				}
			}

			// Alle Nachrichten und Events l�schen
			for (int i = 0; i < pMajor->GetEmpire()->GetEventMessages()->GetSize(); i++)
				delete pMajor->GetEmpire()->GetEventMessages()->GetAt(i);

			pMajor->GetEmpire()->GetEventMessages()->RemoveAll();
			pMajor->GetEmpire()->GetMessages()->RemoveAll();
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
			m_SoundMessages[client].RemoveAll();

			// alle anderen Rassen durchgehen und die vernichtete Rasse aus deren Maps entfernen
			map<CString, CRace*>* mRaces = m_pRaceCtrl->GetRaces();
			for (map<CString, CRace*>::const_iterator itt = mRaces->begin(); itt != mRaces->end(); ++itt)
			{
				if (pMajor->GetRaceID() == itt->first)
					continue;

				CRace* pLivingRace = itt->second;

				pLivingRace->SetIsRaceContacted(pMajor->GetRaceID(), false);
				pLivingRace->SetAgreement(pMajor->GetRaceID(), DIPLOMATIC_AGREEMENT::NONE);

				pMajor->SetIsRaceContacted(pLivingRace->GetRaceID(), false);
				pMajor->SetAgreement(pLivingRace->GetRaceID(), DIPLOMATIC_AGREEMENT::NONE);

				// alle Diplomatischen Angebote und Antworten l�schen
				pMajor->GetIncomingDiplomacyNews()->clear();
				pMajor->GetOutgoingDiplomacyNews()->clear();

				for (UINT i = 0; i < pLivingRace->GetIncomingDiplomacyNews()->size(); i++)
				{
					CDiplomacyInfo* pInfo = &pLivingRace->GetIncomingDiplomacyNews()->at(i);
					if (pInfo->m_sCorruptedRace == pMajor->GetRaceID() || pInfo->m_sFromRace == pMajor->GetRaceID() || pInfo->m_sToRace == pMajor->GetRaceID() || pInfo->m_sWarpactEnemy == pMajor->GetRaceID() || pInfo->m_sWarPartner == pMajor->GetRaceID())
						pLivingRace->GetIncomingDiplomacyNews()->erase(pLivingRace->GetIncomingDiplomacyNews()->begin() + i--);
				}
				for (UINT i = 0; i < pLivingRace->GetOutgoingDiplomacyNews()->size(); i++)
				{
					CDiplomacyInfo* pInfo = &pLivingRace->GetOutgoingDiplomacyNews()->at(i);
					if (pInfo->m_sCorruptedRace == pMajor->GetRaceID() || pInfo->m_sFromRace == pMajor->GetRaceID() || pInfo->m_sToRace == pMajor->GetRaceID() || pInfo->m_sWarpactEnemy == pMajor->GetRaceID() || pInfo->m_sWarPartner == pMajor->GetRaceID())
						pLivingRace->GetOutgoingDiplomacyNews()->erase(pLivingRace->GetOutgoingDiplomacyNews()->begin() + i--);
				}

				if (pLivingRace->GetType() == MAJOR)
				{
					CMajor* pLivingMajor = dynamic_cast<CMajor*>(pLivingRace);
					pLivingMajor->SetDefencePact(pMajor->GetRaceID(), false);
					pMajor->SetDefencePact(pLivingRace->GetRaceID(), false);

					// Geheimdienstzuweiseungen anpassen
					// Spionage auf 0 setzen
					pLivingMajor->GetEmpire()->GetIntelligence()->GetAssignment()->SetGlobalPercentage(0, 0, pLivingMajor, pMajor->GetRaceID(), pmMajors);
					// Sabotage auf 0 setzen
					pLivingMajor->GetEmpire()->GetIntelligence()->GetAssignment()->SetGlobalPercentage(0, 0, pLivingMajor, pMajor->GetRaceID(), pmMajors);
					if (pLivingMajor->GetEmpire()->GetIntelligence()->GetResponsibleRace() == pMajor->GetRaceID())
						pLivingMajor->GetEmpire()->GetIntelligence()->SetResponsibleRace(pLivingRace->GetRaceID());
					pLivingMajor->GetEmpire()->GetIntelligence()->GetAssignment()->RemoveRaceFromAssignments(pMajor->GetRaceID());
				}
			}

			// Alle Schiffe entfernen
			for (int j = 0; j < m_ShipArray.GetSize(); j++)
			{
				if (m_ShipArray.GetAt(j).GetOwnerOfShip() == pMajor->GetRaceID())
				{
					// Alle noch "lebenden" Schiffe aus der Schiffshistory ebenfalls als zerst�rt ansehen
					pMajor->GetShipHistory()->ModifyShip(&m_ShipArray[j],
								m_Sectors.at(m_ShipArray[j].GetKO().x+(m_ShipArray[j].GetKO().y)*STARMAP_SECTORS_HCOUNT).GetName(TRUE), m_iRound,
								CResourceManager::GetString("UNKNOWN"), CResourceManager::GetString("DESTROYED"));
					m_ShipArray.RemoveAt(j--);
				}
			}

			// Sektoren und Systeme neutral schalten
			for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
				for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
				{
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSector() == pMajor->GetRaceID())
					{
						m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSector("");
						m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfSystem("");
						m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwned(false);
						m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetTakenSector(false);
					}
					if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetColonyOwner() == pMajor->GetRaceID())
						m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetColonyOwner("");
					// in allen Sektoren alle Schiffe aus den Sektoren nehmen
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetIsStationBuilding(false, pMajor->GetRaceID());
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOutpost(false, pMajor->GetRaceID());
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetOwnerOfShip(false, pMajor->GetRaceID());
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetShipPort(false, pMajor->GetRaceID());
					m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).SetStarbase(false, pMajor->GetRaceID());
				}

			// Wenn es ein menschlicher Spieler ist, so bekommt er den Eventscreen f�r die Niederlage angezeigt
			if (pMajor->IsHumanPlayer())
			{
				// einen neuen (und auch einzigen Event) einf�gen
				CEventGameOver* eventScreen = new CEventGameOver(pMajor->GetRaceID());
				pMajor->GetEmpire()->GetEventMessages()->Add(eventScreen);
			}
		}
	}

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;
		// Moralver�nderungen aufgrund m�glicher Ereignisse berechnen. Erst nach der Schiffsbewegung und allem anderen
		pMajor->GetMoralObserver()->CalculateEvents(m_Systems, pMajor->GetRaceID(), pMajor->GetRaceMoralNumber());
		///// HIER DIE BONI DURCH SPEZIALFORSCHUNG //////
		// Hier die Boni durch die Uniqueforschung "Lager und Transport" -> kein Abzug beim Stellaren Lager
		if (pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetFieldStatus(2) == RESEARCH_STATUS::RESEARCHED)
			pMajor->GetEmpire()->GetGlobalStorage()->SetLosing(pMajor->GetEmpire()->GetResearch()->GetResearchInfo()->GetResearchComplex(RESEARCH_COMPLEX::STORAGE_AND_TRANSPORT)->GetBonus(2));
		// Ressourcentransfers im globalen Lager vornehmen
		pMajor->GetEmpire()->GetGlobalStorage()->Calculate(m_Systems);
		pMajor->GetEmpire()->GetGlobalStorage()->SetMaxTakenRessources(1000 * pMajor->GetEmpire()->GetNumberOfSystems());
		// Befindet sich irgendeine Ressource im globalen Lager, bekommt der Spieler eine Imperiumsmeldung
		if (pMajor->GetEmpire()->GetGlobalStorage()->IsFilled())
		{
			CString s = CResourceManager::GetString("RESOURCES_IN_GLOBAL_STORAGE");
			CMessage message;
			message.GenerateMessage(s, MESSAGE_TYPE::ECONOMY, "", NULL, FALSE, 4);
			pMajor->GetEmpire()->AddMessage(message);
			if (pMajor->IsHumanPlayer())
			{
				network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
				m_iSelectedView[client] = EMPIRE_VIEW;
			}
		}

		// Schiffskosten berechnen
		int popSupport = pMajor->GetEmpire()->GetPopSupportCosts();
		int shipCosts  = pMajor->GetEmpire()->GetShipCosts();

		int costs = popSupport - shipCosts;
		if (costs < 0)
			pMajor->GetEmpire()->SetCredits(costs);
	}

	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CString sID = it->first;
		for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		{
			for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			{
				// Befindet sich ein Au�enposten oder ein System in einem der umliegenden Sektoren, so bekommt der
				// Sektor einen Besitzerpunkt. Bei einer Sternbasis sind es sogar zwei Besitzerpunkte.
				BYTE ownerPoints = 0;
				if (m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() == sID)
					ownerPoints += 1;
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOutpost(sID) == TRUE)
					ownerPoints += 1;
				if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetStarbase(sID) == TRUE)
					ownerPoints += 2;
				if (ownerPoints > 0)
				{
					for (int j = -1; j <= 1; j++)
						for (int i = -1; i <= 1; i++)
							if ((y+j < STARMAP_SECTORS_VCOUNT && y+j > -1) && (x+i < STARMAP_SECTORS_HCOUNT && x+i > -1))
									m_Sectors.at(x+i+(y+j)*STARMAP_SECTORS_HCOUNT).AddOwnerPoints(ownerPoints, sID);

					// in vertikaler und horizontaler Ausrichtung gibt es sogar 2 Felder vom Sector entfernt noch
					// Besitzerpunkte
					if (x-2 >= 0)
						m_Sectors.at(x-2+(y)*STARMAP_SECTORS_HCOUNT).AddOwnerPoints(ownerPoints, sID);
					if (x+2 < STARMAP_SECTORS_HCOUNT)
						m_Sectors.at(x+2+(y)*STARMAP_SECTORS_HCOUNT).AddOwnerPoints(ownerPoints, sID);
					if (y-2 >= 0)
						m_Sectors.at(x+(y-2)*STARMAP_SECTORS_HCOUNT).AddOwnerPoints(ownerPoints, sID);
					if (y+2 < STARMAP_SECTORS_VCOUNT)
						m_Sectors.at(x+(y+2)*STARMAP_SECTORS_HCOUNT).AddOwnerPoints(ownerPoints, sID);
				}
			}
		}
	}

	// Jetzt die Besitzer berechnen und die Variablen, welche n�chste Runde auch angezeigt werden sollen.
	for (int y = 0 ; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		{
			m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateOwner(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem());
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetSunSystem() == TRUE && m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem() != "")
			{
				CMajor* pMajor = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetOwnerOfSystem()));
				if (!pMajor || pMajor->GetType() != MAJOR)
					continue;

				// baubare Geb�ude, Schiffe und Truppen berechnen
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableBuildings(&m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT), &BuildingInfo, pMajor, &m_GlobalBuildings);
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableShips(this, CPoint(x,y));
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateBuildableTroops(&m_TroopInfo, pMajor->GetEmpire()->GetResearch());
				m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).CalculateVariables(&this->BuildingInfo, pMajor->GetEmpire()->GetResearch()->GetResearchInfo(), m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetPlanets(), pMajor, CTrade::GetMonopolOwner());

				// alle produzierten FP und SP der Imperien berechnen und zuweisen
				int currentPoints;
				currentPoints = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetResearchProd();
				pMajor->GetEmpire()->AddFP(currentPoints);
				currentPoints = m_Systems.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetProduction()->GetSecurityProd();
				pMajor->GetEmpire()->AddSP(currentPoints);
			}

			// Gibt es eine Anomalie im Sektor, so vielleicht die Scanpower niedriger setzen
			if (m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAnomaly())
				m_Sectors.at(x+(y)*STARMAP_SECTORS_HCOUNT).GetAnomaly()->ReduceScanPower(CPoint(x,y));
		}

	// Nachdem die Besitzerpunkte der Sektoren berechnet wurden kann versucht werden neue Rassen kennenzuelernen
	CalcContactNewRaces();

	// Nun das Schiffinformationsfeld durchgehen und in die WeaponObserver-Klasse aller Imperien
	// die baubaren Waffen eintragen. Wir brauchen dies um selbst Schiffe designen zu k�nnen
	// Dies gilt nur f�r Majorsraces.
	for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
	{
		CMajor* pMajor = it->second;

		BYTE researchLevels[6] =
		{
			pMajor->GetEmpire()->GetResearch()->GetBioTech(),
			pMajor->GetEmpire()->GetResearch()->GetEnergyTech(),
			pMajor->GetEmpire()->GetResearch()->GetCompTech(),
			pMajor->GetEmpire()->GetResearch()->GetPropulsionTech(),
			pMajor->GetEmpire()->GetResearch()->GetConstructionTech(),
			pMajor->GetEmpire()->GetResearch()->GetWeaponTech()
		};

		for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
		{
			if (m_ShipInfoArray.GetAt(i).GetRace() == pMajor->GetRaceShipNumber())
			{
				// nur aktuell baubare Schiffe d�rfen �berpr�ft werden, wenn wir die Beamwaffen checken
				if (m_ShipInfoArray.GetAt(i).IsThisShipBuildableNow(researchLevels))
				{
					// Wenn die jeweilige Rasse dieses technologisch bauen k�nnte, dann Waffen des Schiffes checken
					pMajor->GetWeaponObserver()->CheckBeamWeapons(&m_ShipInfoArray.GetAt(i));
					pMajor->GetWeaponObserver()->CheckTorpedoWeapons(&m_ShipInfoArray.GetAt(i));
				}
			}
		}
	}

	m_VictoryObserver.Observe();
	if (m_VictoryObserver.IsVictory())
	{
		// Victoryeventscreen f�r die Siegerrasse
		for (map<CString, CMajor*>::const_iterator it = pmMajors->begin(); it != pmMajors->end(); ++it)
		{
			CMajor* pMajor = it->second;
			// Alle Nachrichten und Events l�schen
			for (int i = 0; i < pMajor->GetEmpire()->GetEventMessages()->GetSize(); i++)
				delete pMajor->GetEmpire()->GetEventMessages()->GetAt(i);

			pMajor->GetEmpire()->GetEventMessages()->RemoveAll();
			pMajor->GetEmpire()->GetMessages()->RemoveAll();
			network::RACE client = m_pRaceCtrl->GetMappedClientID(pMajor->GetRaceID());
			m_SoundMessages[client].RemoveAll();

			// Wenn es ein menschlicher Spieler ist, so bekommt er den Eventscreen f�r den Sieg angezeigt
			if (pMajor->IsHumanPlayer())
			{
				// einen neuen (und auch einzigen Event) einf�gen
				CString sImageName;
				if (pMajor->GetRaceID() == m_VictoryObserver.GetVictoryRace())
					sImageName = "Victory" + pMajor->GetRaceID();
				else
					sImageName = "GameOver";
				CEventVictory* eventScreen = new CEventVictory(pMajor->GetRaceID(), m_VictoryObserver.GetVictoryRace(), (int)m_VictoryObserver.GetVictoryType(), sImageName);
				pMajor->GetEmpire()->GetEventMessages()->Add(eventScreen);
			}
		}
	}

	// Bestimmte Views zur�cksetzen
	CSmallInfoView::SetShipInfo(false);
	for (int i = network::RACE_1; i < network::RACE_ALL; i++)
		if (m_iSelectedView[i] == FLEET_VIEW)
			m_iSelectedView[i] = GALAXY_VIEW;

	m_NumberOfTheShipInArray = -1;
	m_iNumberOfFleetShip = -1;
	m_iNumberOfTheShipInFleet = -1;
}

/// Funktion berechnet, ob zuf�llig Alienschiffe ins Spiel kommen.
void CBotf2Doc::CalcRandomAlienEntities()
{
	// Aliens zuf�llig ins Spiel bringen
	for (int i = 0; i < m_ShipInfoArray.GetSize(); i++)
	{
		CShipInfo* pShipInfo = &m_ShipInfoArray.GetAt(i);
		if (pShipInfo->GetShipType() != SHIP_TYPE::ALIEN)
			continue;

		// zugeh�rige Minorrace finden
		if (CMinor* pAlien = dynamic_cast<CMinor*>(m_pRaceCtrl->GetRace(pShipInfo->GetOnlyInSystem())))
		{
			if (!pAlien->IsAlienRace())
			{
				ASSERT(pAlien->IsAlienRace());
				continue;
			}

			// Pr�fen ob das Alienschiff zum Galaxieweiten technologischen Fortschritt passt
			// Alienschiff das h�here Voraussetzungen als der technologische Fortschritt hat
			// kommt nicht ins Spiel. �ltere Alienschiffe, die viel geringer als der Fortschritt
			// sind kommen mit niedrigerer Wahrscheinlichkeit ins Spiel.
			BYTE byAvgTechLevel = m_Statistics.GetAverageTechLevel();
			BYTE researchLevels[6] = {byAvgTechLevel, byAvgTechLevel, byAvgTechLevel, byAvgTechLevel, byAvgTechLevel, byAvgTechLevel};
			if (!pShipInfo->IsThisShipBuildableNow(researchLevels))
				continue;

			// jedes Level unterhalb der durchschnittlichen Techstufe verringert sich die Wahrscheinlichkeit
			// des Auftauchens des Alienschiffes
			BYTE byAvgShipTech = (pShipInfo->GetBioTech() + pShipInfo->GetEnergyTech() + pShipInfo->GetComputerTech() + pShipInfo->GetConstructionTech() + pShipInfo->GetPropulsionTech() + pShipInfo->GetWeaponTech()) / 6;
			int nMod = max(byAvgTechLevel - byAvgShipTech, 0) * 5;

			// nur ca. aller 20 + Techmodifikator Runden kommt das Alienschiff ins Spiel
			if (rand()%(20 + nMod) != 0)
				continue;

			// zuf�lligen Sektor am Rand der Map ermitteln
			while (true)
			{
				// Schiff irgendwo an einem Rand der Map auftauchen lassen
				CPoint p;
				switch(rand()%4)
				{
				case 0: p = CPoint(0, rand()%STARMAP_SECTORS_VCOUNT); break;
				case 1: p = CPoint(STARMAP_SECTORS_HCOUNT - 1, rand()%STARMAP_SECTORS_VCOUNT); break;
				case 2: p = CPoint(rand()%STARMAP_SECTORS_HCOUNT, 0); break;
				case 3: p = CPoint(rand()%STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT - 1); break;
				default: p = CPoint(rand()%STARMAP_SECTORS_HCOUNT, rand()%STARMAP_SECTORS_VCOUNT);
				}

				// nicht auf einer Anomalie!
				if (!GetSector(p).GetAnomaly())
				{
					BuildShip(pShipInfo->GetID(), p, pAlien->GetRaceID());

					CShip* pShip = &m_ShipArray[m_ShipArray.GetUpperBound()];
					// unterschiedliche Aliens unterschieden und Schiffseigenschaften festlegen
					if (pAlien->GetRaceID() == "Ionisierendes Gaswesen")
					{
						pShip->SetAlienType(ALIEN_TYPE::IONISIERENDES_GASWESEN);
						pShip->SetCurrentOrder(SHIP_ORDER::AVOID);
					}
					else if (pAlien->GetRaceID() == "Gaballianer")
					{
						pShip->SetAlienType(ALIEN_TYPE::GABALLIANER_SEUCHENSCHIFF);
						pShip->SetCurrentOrder(SHIP_ORDER::ATTACK);
					}
					else if (pAlien->GetRaceID() == "Blizzard-Plasmawesen")
					{
						pShip->SetAlienType(ALIEN_TYPE::BLIZZARD_PLASMAWESEN);
						pShip->SetCurrentOrder(SHIP_ORDER::ATTACK);
					}

					break;
				}
			}
		}
	}
}

/// Funktion berechnet Auswirkungen von Alienschiffe auf Systeme, �ber denen sie sich befinden.
void CBotf2Doc::CalcAlienShipEffects()
{
	for (int i = 0; i < m_ShipArray.GetSize(); i++)
	{
		CShip* pShip = &m_ShipArray.GetAt(i);
		if (pShip->GetShipType() != SHIP_TYPE::ALIEN)
			continue;

		// Aliens mit R�ckzugsbefehl machen nix
		if (pShip->GetCombatTactic() == COMBAT_TACTIC::CT_RETREAT)
			continue;

		CMinor* pAlien = dynamic_cast<CMinor*>(m_pRaceCtrl->GetRace(pShip->GetOwnerOfShip()));
		if (!pAlien || !pAlien->IsAlienRace())
		{
			ASSERT(FALSE);
			continue;
		}

		// verschiedene Alienrassen unterscheiden
		if ((pShip->GetAlienType() & ALIEN_TYPE::IONISIERENDES_GASWESEN) > 0)
		{
			CString sSystemOwner = GetSystem(pShip->GetKO()).GetOwnerOfSystem();
			CMajor* pOwner = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sSystemOwner));
			if (!pOwner)
				continue;

			// Energie im System auf 0 setzen
			GetSystem(pShip->GetKO()).SetDisabledProduction(WORKER::ENERGY_WORKER);

			// Wenn Energie vorhanden war, dann die Nachricht bringen �ber Energieausfall
			if (GetSystem(pShip->GetKO()).GetProduction()->GetMaxEnergyProd() > 0)
			{
				// Nachricht und Event einf�gen
				CString s = CResourceManager::GetString("EVENT_IONISIERENDES_GASWESEN", FALSE, GetSector(pShip->GetKO()).GetName());
				CMessage message;
				message.GenerateMessage(s, MESSAGE_TYPE::SOMETHING, GetSector(pShip->GetKO()).GetName(), pShip->GetKO(), 0);
				pOwner->GetEmpire()->AddMessage(message);
				if (pOwner->IsHumanPlayer())
				{
					CEventAlienEntity* eventScreen = new CEventAlienEntity(pOwner->GetRaceID(), pAlien->GetRaceID(), pAlien->GetRaceName(), s);
					pOwner->GetEmpire()->GetEventMessages()->Add(eventScreen);

					network::RACE client = m_pRaceCtrl->GetMappedClientID(pOwner->GetRaceID());
					m_iSelectedView[client] = EMPIRE_VIEW;
				}
			}
		}
		if ((pShip->GetAlienType() & ALIEN_TYPE::GABALLIANER_SEUCHENSCHIFF) > 0)
		{
			CString sSystemOwner = GetSystem(pShip->GetKO()).GetOwnerOfSystem();
			if (CMajor* pOwner = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sSystemOwner)))
			{
				// Nahrung im System auf 0 setzen
				GetSystem(pShip->GetKO()).SetDisabledProduction(WORKER::FOOD_WORKER);
				GetSystem(pShip->GetKO()).SetFoodStore(GetSystem(pShip->GetKO()).GetFoodStore() / 2);

				// Wenn narung produziert oder vorhanden ist, dann die Nachricht bringen �ber Nahrung verseucht
				if (GetSystem(pShip->GetKO()).GetProduction()->GetMaxFoodProd() > 0 || GetSystem(pShip->GetKO()).GetFoodStore() > 0)
				{
					// Nachricht und Event einf�gen
					CString s = CResourceManager::GetString("EVENT_GABALLIANER_SEUCHENSCHIFF", FALSE, GetSector(pShip->GetKO()).GetName());
					CMessage message;
					message.GenerateMessage(s, MESSAGE_TYPE::SOMETHING, GetSector(pShip->GetKO()).GetName(), pShip->GetKO(), 0);
					pOwner->GetEmpire()->AddMessage(message);
					if (pOwner->IsHumanPlayer())
					{
						CEventAlienEntity* eventScreen = new CEventAlienEntity(pOwner->GetRaceID(), pAlien->GetRaceID(), pAlien->GetRaceName(), s);
						pOwner->GetEmpire()->GetEventMessages()->Add(eventScreen);

						network::RACE client = m_pRaceCtrl->GetMappedClientID(pOwner->GetRaceID());
						m_iSelectedView[client] = EMPIRE_VIEW;
					}
				}
			}

			// befinden sich Schiffe in diesem Sektor, so werden diese ebenfalls zu Seuchenschiffen (33%)
			if (GetSector(pShip->GetKO()).GetIsShipInSector() && rand()%3 == 0)
			{
				// alle Schiffe im Sektor zu Seuchenschiffen machen
				for (int y = 0; y < m_ShipArray.GetSize(); y++)
				{
					CShip* pOtherShip = &m_ShipArray.GetAt(y);
					// Schiff im gleichen Sektor?
					if (pOtherShip->GetKO() != pShip->GetKO())
						continue;

					// keine anderen Alienschiffe
					if (pOtherShip->GetShipType() == SHIP_TYPE::ALIEN || (pOtherShip->GetAlienType() & ALIEN_TYPE::GABALLIANER_SEUCHENSCHIFF) > 0)
						continue;

					// keine Au�enposten und Sternenbasen
					if (pOtherShip->GetShipType() == SHIP_TYPE::OUTPOST || pOtherShip->GetShipType() == SHIP_TYPE::STARBASE)
						continue;

					vector<CShip*> vShips;
					vShips.push_back(pOtherShip);

					if (pOtherShip->GetFleet())
					{
						for (int x = 0; x < pOtherShip->GetFleet()->GetFleetSize(); x++)
							vShips.push_back(pOtherShip->GetFleet()->GetShipFromFleet(x));
					}

					for (unsigned int n = 0; n < vShips.size(); n++)
					{
						// Schiffe mit R�ckzugsbefehl werden nie vom Virus befallen
						if (vShips[n]->GetCombatTactic() == COMBAT_TACTIC::CT_RETREAT)
							continue;

						vShips[n]->SetOwnerOfShip(pAlien->GetRaceID());
						vShips[n]->SetTargetKO(CPoint(-1, -1), 0);
						vShips[n]->SetAlienType(ALIEN_TYPE::GABALLIANER_SEUCHENSCHIFF);
						vShips[n]->SetCurrentOrder(SHIP_ORDER::ATTACK);
						vShips[n]->SetTerraformingPlanet(-1);
						vShips[n]->SetIsShipFlagShip(FALSE);

						// f�r jedes Schiff eine Meldung �ber den Verlust machen

						// In der Schiffshistoryliste das Schiff als ehemaliges Schiff markieren
						if (CMajor* pShipOwner = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(vShips[n]->GetOwnerOfShip())))
						{
							AddToLostShipHistory(vShips[n], CResourceManager::GetString("COMBAT"), CResourceManager::GetString("MISSED"));
							CString s;
							s.Format("%s", CResourceManager::GetString("DESTROYED_SHIPS_IN_COMBAT",0,vShips[n]->GetShipName()));
							CMessage message;
							message.GenerateMessage(s, MESSAGE_TYPE::MILITARY, "", 0, 0);
							pShipOwner->GetEmpire()->AddMessage(message);
						}
					}
				}
			}
		}
		if ((pShip->GetAlienType() & ALIEN_TYPE::BLIZZARD_PLASMAWESEN) > 0)
		{
			CString sSystemOwner = GetSystem(pShip->GetKO()).GetOwnerOfSystem();
			CMajor* pOwner = dynamic_cast<CMajor*>(m_pRaceCtrl->GetRace(sSystemOwner));
			if (!pOwner)
				continue;

			// Energie im System auf 0 setzen
			GetSystem(pShip->GetKO()).SetDisabledProduction(WORKER::ENERGY_WORKER);

			// Wenn Energie vorhanden war, dann die Nachricht bringen �ber Energieausfall
			if (GetSystem(pShip->GetKO()).GetProduction()->GetMaxEnergyProd() > 0)
			{
				// Nachricht und Event einf�gen
				CString s = CResourceManager::GetString("EVENT_BLIZZARD_PLASMAWESEN", FALSE, GetSector(pShip->GetKO()).GetName());
				CMessage message;
				message.GenerateMessage(s, MESSAGE_TYPE::SOMETHING, GetSector(pShip->GetKO()).GetName(), pShip->GetKO(), 0);
				pOwner->GetEmpire()->AddMessage(message);
				if (pOwner->IsHumanPlayer())
				{
					CEventAlienEntity* eventScreen = new CEventAlienEntity(pOwner->GetRaceID(), pAlien->GetRaceID(), pAlien->GetRaceName(), s);
					pOwner->GetEmpire()->GetEventMessages()->Add(eventScreen);

					network::RACE client = m_pRaceCtrl->GetMappedClientID(pOwner->GetRaceID());
					m_iSelectedView[client] = EMPIRE_VIEW;
				}
			}
		}
	}
}

void CBotf2Doc::OnUpdateFileNew(CCmdUI *pCmdUI)
{
	// TODO: F�gen Sie hier Ihren Befehlsaktualisierungs-UI-Behandlungscode ein.
	pCmdUI->Enable(FALSE);
}

void CBotf2Doc::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	// TODO: F�gen Sie hier Ihren Befehlsaktualisierungs-UI-Behandlungscode ein.
	pCmdUI->Enable(FALSE);
}

BOOL CBotf2Doc::OnOpenDocument(LPCTSTR lpszPathName)
{
//	return __super::OnOpenDocument(lpszPathName);

	MYTRACE("general")(MT::LEVEL_INFO, "loading savegame \"%s\"\n", lpszPathName);

	CFile file;
	BYTE *lpBuf = NULL;

	{
	CFileException ex;
	if (!file.Open(lpszPathName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary, &ex))
	{
		TCHAR cause[255];
		ex.GetErrorMessage(cause, 255);
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: could not open file (%s)\n", cause);
		return FALSE;
	}

	// Header lesen
	UINT nSize = 4 + 2 * sizeof(UINT);
	lpBuf = new BYTE[nSize];
	UINT nDone = file.Read(lpBuf, nSize);
	if (nDone < nSize)
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: unexpected end of file\n");
		goto error;
	}

	// Magic Number pr�fen
	BYTE *p = lpBuf;
	if (memcmp(p, "BotE", 4) != 0)
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: invalid magic number\n");
		goto error;
	}
	p += 4;

	// Versionsnummer pr�fen
	UINT nVersion = 0;
	memcpy(&nVersion, p, sizeof(UINT));
	p += sizeof(UINT);
	if (nVersion != DOCUMENT_VERSION)
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: wrong version\n");
		goto error;
	}

	// L�nge der Daten lesen
	memcpy(&nSize, p, sizeof(UINT));
	p += sizeof(UINT);

	// Daten aus Datei in Puffer lesen
	delete[] lpBuf;
	lpBuf = new BYTE[nSize];

	nDone = file.Read(lpBuf, nSize);
	if (nDone < nSize)
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: unexpected end of file\n");
		goto error;
	}

	// Dekomprimieren
	CMemFile memFile;
	if (!BotE_LzmaDecompress(lpBuf, nSize, memFile))
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: error during decompression\n");
		goto error;
	}
	memFile.Seek(0, CFile::begin);

	// Deserialisieren
	CArchive ar(&memFile, CArchive::load);
//	Reset();
	SetModifiedFlag();
	Serialize(ar);
	ar.Close();
	SetModifiedFlag(FALSE);

	// aufr�umen
	delete[] lpBuf;
	file.Close();
	return TRUE;
	}

error:
	if (lpBuf) delete[] lpBuf;
	file.Close();
	return FALSE;
}

BOOL CBotf2Doc::OnSaveDocument(LPCTSTR lpszPathName)
{
//	return __super::OnSaveDocument(lpszPathName);

	MYTRACE("general")(MT::LEVEL_INFO, "storing savegame \"%s\"\n", lpszPathName);

	// Savegame schreiben
	CFileException ex;
	CFile file;
	if (!file.Open(lpszPathName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeBinary,
		&ex))
	{
		TCHAR cause[255];
		ex.GetErrorMessage(cause, 255);
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: could not open file (%s)\n", cause);
		return FALSE;
	}

	// Magic Number
	file.Write("BotE", 4);

	// Versionsnummer
	UINT nVersion = DOCUMENT_VERSION;
	file.Write(&nVersion, sizeof(UINT));

	// Platzhalter f�r L�nge der Daten
	UINT nSize = 0;
	file.Write(&nSize, sizeof(UINT));

	// Daten serialisieren
	CMemFile memFile;
	CArchive ar(&memFile, CArchive::store);
	Serialize(ar);
	ar.Close();

	// komprimieren, in Datei schreiben
	nSize = memFile.GetLength();
	BYTE *lpBuf = memFile.Detach();
        //MYTRACE("general")(MT::LEVEL_INFO, "rainer-Test", lpszPathName);
	if (!BotE_LzmaCompress(lpBuf, nSize, file))
	{
		MYTRACE("general")(MT::LEVEL_ERROR, "savegame: error during compression\n");
		free(lpBuf);
		file.Close();
		try
		{
			CFile::Remove(lpszPathName);
		}
		catch (CFileException *pEx)
		{
			TCHAR cause[255];
			pEx->GetErrorMessage(cause, 255);
			MYTRACE("general")(MT::LEVEL_ERROR, "savegame: could not delete corrupted savegame (%s)\n", cause);
			pEx->Delete();
		}
		return FALSE;
	}
	free(lpBuf);

	// L�nge des komprimierten Puffers an Stelle des Platzhalters eintragen
	nSize = file.GetLength() - 4 - 2 * sizeof(UINT);
	file.Seek(4 + sizeof(UINT), CFile::begin);
	file.Write(&nSize, sizeof(UINT));

	file.Close();
	SetModifiedFlag(FALSE);
	return TRUE;
}

void CBotf2Doc::AllocateSectorsAndSystems()
{
	//m_Sectors.clear();
	//m_Systems.clear();
	const unsigned size = STARMAP_SECTORS_HCOUNT*STARMAP_SECTORS_VCOUNT;
	m_Sectors.resize(size);
	m_Systems.resize(size);
 }

CMainFrame* CBotf2Doc::GetMainFrame(void) const {
	return dynamic_cast<CMainFrame*>(AfxGetApp()->GetMainWnd());
}
