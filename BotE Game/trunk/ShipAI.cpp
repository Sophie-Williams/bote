#include "stdafx.h"
#include "ShipAI.h"
#include "Botf2Doc.h"
#include "SectorAI.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
CShipAI::CShipAI(CBotf2Doc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
	for (int i = 0; i < DOMINION; i++)
	{
		m_AttackSector[i] = CPoint(-1,-1);
		m_BombardSector[i] = CPoint(-1,-1);
	}

	// Durchschnittsmoral berechnen
	int moralAll[DOMINION] = {0};
	int systems[DOMINION] = {0};
	for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
		for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
			if (m_pDoc->m_System[x][y].GetOwnerOfSystem() != NOBODY && m_pDoc->m_System[x][y].GetOwnerOfSystem() != UNKNOWN)
			{
				moralAll[m_pDoc->m_System[x][y].GetOwnerOfSystem()-1] += m_pDoc->m_System[x][y].GetMoral();
				systems[m_pDoc->m_System[x][y].GetOwnerOfSystem()-1]++;
			}
	for (int i = 0; i < DOMINION; i++)
	{
		if (systems[i] > NULL)
			m_iAverageMoral[i] = moralAll[i] / systems[i];
		else
			m_iAverageMoral[i] = 0;
	}
}

CShipAI::~CShipAI(void)
{
}

//////////////////////////////////////////////////////////////////////
// sonstige Funktionen
//////////////////////////////////////////////////////////////////////
/// Diese Funktion erteilt allen Schiffen aller computergesteuerten Rassen Befehle.
void CShipAI::CalculateShipOrders(CSectorAI* SectorAI)
{
	ASSERT(SectorAI);
	m_pSectorAI = SectorAI;

	// einen m�glichen Angriffssektor berechnen lassen
	CalcAttackSector();
	// danach einen m�glichen Bombardierungssektor finden
	CalcBombardSector();

	for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
	{
		BYTE race = m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip();
		if (race != NOBODY && race != UNKNOWN)
		{
			if (m_pDoc->m_Empire[race].GetPlayerOfEmpire() != COMPUTER)
				continue;
			// Flotte versuchen zu erstellen
			DoMakeFleet(i);

			// Vielleicht haben unsere Schiffe ein Ziel, welches sie angreifen m�ssen/k�nnen			
			if (DoAttackMove(i))
			{				
				DoCamouflage(i);
				continue;
			}
			if (m_pDoc->m_ShipArray.GetAt(i).GetPath()->GetSize() == NULL)
			{
				// Scouts und Kriegsschiffe fliegen zuerst einmal zu den Minorracesystemen
				if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() > COLONYSHIP)
				{
					BOOLEAN set = FALSE;
					int count = m_pSectorAI->GetMinorraceSectors(race)->GetSize()*2;
					while (m_pSectorAI->GetMinorraceSectors(race)->GetSize() && count--)
					{
						int j = rand()%m_pSectorAI->GetMinorraceSectors(race)->GetSize();
						CPoint ko = m_pSectorAI->GetMinorraceSectors(race)->GetAt(j);
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(race, ko) == NULL || 
							(m_pSectorAI->GetCompleteDanger(race, ko) < m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (m_pDoc->starmap[race]->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								TRACE("RACE %d: Ship to Minor: %s (%s) - Ziel: %d,%d\n",race, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif
								m_pSectorAI->GetMinorraceSectors(race)->RemoveAt(j--);
								set = TRUE;
								break;
							}
					}
					if (set)
					{
						DoCamouflage(i);
						continue;
					}
				}
				// Kolonieschiffe zum Terraformen schicken. Andere Schiffe fliegen manchmal auch dort hin, wenn
				// sie gerade keinen anderen Flugauftrag haben.
				if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() >= COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() != TERRAFORM)
				{
					for (int j = 0; j < m_pSectorAI->GetSectorsToTerraform(race)->GetSize(); j++)
					{
						CPoint ko = m_pSectorAI->GetSectorsToTerraform(race)->GetAt(j).p;
						// Wenn das Kolonieschiff schon auf einem Sektor f�r unser Terraforming steht, so fliegt es nicht
						// weiter
						if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetKO() == ko)
							break;
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(race, ko) == NULL || (m_pSectorAI->GetCompleteDanger(race, ko) < m_pSectorAI->GetDanger(race, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (m_pDoc->starmap[race]->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								TRACE("RACE %d: Ship %s (%s) hat terraforming Ziel: %d,%d\n",race, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif
								break;
							}
					}
				}
				// Truppentransporter zu einem m�glichen Sektor fliegen lassen um dort einen Au�enposten bauen zu k�nnen
				if (m_pSectorAI->GetStationBuildSector(race).points > MINBASEPOINTS && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() != BUILD_OUTPOST)
				{
					// nur Truppentransporter oder andere Schiffe ohne Ziel fliegen zu diesem Punkt, niemals aber
					// Kolonieschiffe
					if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == TRANSPORTER
						|| (m_pDoc->m_ShipArray.GetAt(i).GetShipType() != COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetTargetKO() == m_pDoc->m_ShipArray.GetAt(i).GetKO()))
					{
						CPoint ko(m_pSectorAI->GetStationBuildSector(race).position.x, m_pSectorAI->GetStationBuildSector(race).position.y);
						// Wenn Gefahr der anderen Rassen kleiner als die der meinen ist
						if (m_pSectorAI->GetCompleteDanger(race, ko) == NULL || (m_pSectorAI->GetCompleteDanger(race, ko) < m_pSectorAI->GetDanger(race, m_pDoc->m_ShipArray.GetAt(i).GetKO())))
							if (m_pDoc->starmap[race]->GetRange(ko) <= m_pDoc->m_ShipArray.GetAt(i).GetRange())
							{
								// Zielkoordinate f�r das Schiff setzen
								m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko,0);
#ifdef TRACE_SHIPAI
								TRACE("RACE %d: Ship %s (%s) hat Au�enpostenbauziel: %d,%d\n",race, m_pDoc->m_ShipArray.GetAt(i).GetShipName(), m_pDoc->m_ShipArray.GetAt(i).GetShipTypeAsString(), ko.x,ko.y);
#endif								
							}
					}
				}				

				DoCamouflage(i);
				CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetSunSystem())
					if (!DoTerraform(i))
						DoColonize(i);
				DoStationBuild(i);
			}
			else
				DoCamouflage(i);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// private Funktionen
//////////////////////////////////////////////////////////////////////
/// Funktion erteilt einen Terraformbefehl, sofern dies auch m�glich ist. �bergebn wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Terraformbefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoTerraform(int index)
{
	/*
	   Es wird jeder Planet sofort genommen, welcher weniger als 7 Terraformpunkte
	   ben�tigt. Planeten welche mehr als 6 Terraformpunkte ben�tigen werden nicht
	   unbedingt sofort gew�hlt. Dort m�ssen wir die doppelte Anzahl an Runden erreicht
	   haben. Au�er es gibt keinen Planeten, welcher in diesem Moment kolonisiert werden
	   k�nnte.
    */
	int i = index;
	if (m_pDoc->m_ShipArray.GetAt(i).GetColonizePoints() == NULL)
		return FALSE;
	CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
	BYTE minTerraPoints = MAXBYTE;
	short planet = -1;
	BYTE terraPoints = m_pDoc->m_ShipArray.GetAt(i).GetColonizePoints();
	for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
		if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == FALSE
			&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetHabitable() == TRUE)
		{
			if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() < minTerraPoints)
				minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
			// minTerraPoints - TerraformingPoints des Schiffes > 6 + Planetentyp
			if ((minTerraPoints - terraPoints) > (4 + m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetType()) && 2*minTerraPoints > m_pDoc->m_iRound)
			{
				minTerraPoints = MAXBYTE;
				planet = -1;
			}
			if (minTerraPoints == MAXBYTE && (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() - terraPoints) <= (4 + m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetType()))
				minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
			if (minTerraPoints == m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints())
				planet = j;
		}
	if (planet != -1)
	{
		// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
		m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
		m_pDoc->m_ShipArray.GetAt(i).SetTerraformingPlanet(planet);
		m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(TERRAFORM);
		return TRUE;
	}
	// Wenn kein Planet zum Terraformen gefunden wurde, wir aber auch keinen Planeten kolonisieren
	// k�nnen, dann w�hle den mit der k�rzesten Terraformdauer.
	else
	{
		minTerraPoints = MAXBYTE;
		planet = -1;
		for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
		{
			if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == TRUE
				&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetColonized() == FALSE)
			{
				planet = -1;
				break;
			}
			else if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == FALSE
				&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetHabitable() == TRUE)
			{
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints() < minTerraPoints)
				{
					minTerraPoints = m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetNeededTerraformPoints();
					planet = j;
				}
			}
		}
		if (planet != -1)
		{
			// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
			m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
			m_pDoc->m_ShipArray.GetAt(i).SetTerraformingPlanet(planet);
			m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(TERRAFORM);
			return TRUE;
		}
	}
	return FALSE;
}

/// Funktion erteilt einen Kolonisierungsbefehl, sofern dies auch m�glich ist. �bergebn wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Kolonisierungsbefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoColonize(int index)
{
	int i = index;
	// Kolonieschiffe eine Kolonisierung vorschlagen
	if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP)
	{
		CPoint shipKO = m_pDoc->m_ShipArray.GetAt(i).GetKO();
		// Geh�rt der Sektor aktuell auch keiner Minorrace
		if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetOwnerOfSector() != UNKNOWN)
			// Kolonisierungsbefehl geben
			for (int j = 0; j < m_pDoc->m_Sector[shipKO.x][shipKO.y].GetNumberOfPlanets(); j++)
				if (m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetTerraformed() == TRUE
					&& m_pDoc->m_Sector[shipKO.x][shipKO.y].GetPlanet(j)->GetColonized() == FALSE)
				{
					// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden
					m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(shipKO,0);
					m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(COLONIZE);
					return TRUE;
				}
	}
	return FALSE;
}

/// Funktion schickt Kriegsschiffe zu einem m�glichen Offensivziel. Unter Umst�nden wird auch der Befehl zum
/// Angriff automatisch erteilt. Auch kann es passieren, das diese Funktion die Kriegsschiffe zu einem
/// gemeinsamen Sammelpunkt schickt. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl
/// gegeben werden k�nnte.
BOOLEAN CShipAI::DoAttackMove(int index)
{
	if (m_pDoc->m_ShipArray.GetAt(index).GetShipType() <= COLONYSHIP
		|| m_pDoc->m_ShipArray.GetAt(index).GetShipType() >= OUTPOST)
		return FALSE;

	if (DoBombardSystem(index))
		return TRUE;
		
	BYTE race = m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip();
	BYTE min  = MAXBYTE;
	CPoint targetKO(-1,-1);
	int z = 0;
	// �berpr�ft wird zuerst, ob hier ein m�gliches Offensivziel in der N�he des Schiffes ist. Wenn ja, dann
	// wird �berpr�ft, ob unsere Schiffsst�rke gr��er als die beim Ziel ist oder wir 75% bis 90% unserer Flotte
	// auf unserem Sektorfeld schon versammelt haben. Wenn ja wird angegriffen.
	for (int i = 0; i < m_pSectorAI->GetOffensiveTargets(race)->GetSize(); i++)
	{
		CPoint p = m_pSectorAI->GetOffensiveTargets(race)->GetAt(i);
		// In der N�he ist das Ziel mit der geringsten Entfernung zu unserem Sektor.
		if (max(p.x,p.y) < min)
		{
			min = (BYTE)max(p.x,p.y);
			targetKO = p;
			z = i;
		}
	}
	// Es gibt ein Ziel in der N�he unseres Schiffes
	if (targetKO != CPoint(-1,-1))
	{
#ifdef TRACE_SHIPAI
		TRACE("RACE %d: Schiff '%s' hat vlt. nahes Ziel %d/%d\n",race, m_pDoc->m_ShipArray.GetAt(index).GetShipName(), targetKO.x, targetKO.y);
#endif
		// pr�fen ob wir st�rker sind oder ob wie 75 bis 90% unserer Flotte hier stationiert haben
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(race, targetKO) * 1.25
			|| m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > (m_pSectorAI->GetCompleteDanger(race) * (rand()%16+75) / 100))
		{
			// !!!
			// (besser w�re noch, wir bilden hier vorher eine Flotte, so l��t sich koordinierter Angreifen!!!)

			// Wenn dies zutrifft werden alle Schiffe aus diesem Sektor zum Zielsektor geschickt.
			for (int j = 0; j < m_pDoc->m_ShipArray.GetSize(); j++)
				if (m_pDoc->m_ShipArray.GetAt(j).GetOwnerOfShip() == race)
					if (m_pDoc->m_ShipArray.GetAt(j).GetKO() == m_pDoc->m_ShipArray.GetAt(index).GetKO())
						if (m_pDoc->m_ShipArray.GetAt(j).GetShipType() <= COLONYSHIP
							|| m_pDoc->m_ShipArray.GetAt(j).GetShipType() >= OUTPOST)
							if (m_pDoc->starmap[race]->GetRange(targetKO) <= m_pDoc->m_ShipArray.GetAt(j).GetRange())
							{
								m_pDoc->m_ShipArray.GetAt(j).SetTargetKO(targetKO, 0);
								#ifdef TRACE_SHIPAI
								TRACE("RACE %d: ATTACK auf nahes Ziel: %d/%d\n",targetKO.x,targetKO.y);
								#endif
								m_pSectorAI->GetOffensiveTargets(race)->RemoveAt(z);
								return TRUE;
							}
		}
	}
	// wenn es kein Ziel in der N�he gab, daf�r aber ein Bombardierungsziel ausgegeben wurde, so wird dieses mit
	// der h�chsten Priorit�t angeflogen
	if (m_BombardSector[race-1] != CPoint(-1,-1))
	{
		// pr�fen ob wir im Bombardierungssektor st�rker w�ren
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(race, m_BombardSector[race-1]))
		{
			// hier �berpr�fen, ob der Sektor erreicht werden kann
			if (m_pDoc->starmap[race]->GetRange(m_BombardSector[race-1]) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_BombardSector[race-1], 0);
				#ifdef TRACE_SHIPAI
				TRACE("RACE %d: BOMBARDIERUNGSZIEL auf Sektor: %d/%d\n",race,m_BombardSector[race-1].x,m_BombardSector[race-1].y);
				#endif
				return TRUE;
			}
		}
	}

	// wenn es kein Ziel in der N�he gibt, oder wenn das Ziel in der N�he nicht f�r den Angriff geeignet war, dann
	// wenn m�glich die Schiffe zu dem Punkt schicken, welcher als generelles Angriffsziel auserkoren wurde
	if (m_AttackSector[race-1] != CPoint(-1,-1))
	{
		// pr�fen ob wir st�rker sind oder ob wie 75 bis 90% unserer Flotte hier stationiert haben
		if (m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > m_pSectorAI->GetCompleteDanger(race, m_AttackSector[race-1])
			|| m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()) > (m_pSectorAI->GetCompleteDanger(race) * (rand()%16+75) / 100))
		{
			// hier noch �berpr�fen, ob der Sektor erreicht werden kann
			if (m_pDoc->starmap[race]->GetRange(m_AttackSector[race-1]) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_AttackSector[race-1], 0);
#ifdef TRACE_SHIPAI
				TRACE("RACE %d: GLOBALE ATTACKE auf Sektor: %d/%d\n",race,m_AttackSector[race-1].x,m_AttackSector[race-1].y);
				TRACE("Ship: %s\n",m_pDoc->m_ShipArray.GetAt(index).GetShipName());
				TRACE("OnlyDangerFromShips: %d in Sector: %d/%d\n",m_pSectorAI->GetDangerOnlyFromCombatShips(race, m_pDoc->m_ShipArray.GetAt(index).GetKO()),m_pDoc->m_ShipArray.GetAt(index).GetKO().x,m_pDoc->m_ShipArray.GetAt(index).GetKO().y);
				TRACE("CompleteDangerInTargetSector: %d\n",m_pSectorAI->GetCompleteDanger(race, m_AttackSector[race-1]));
				TRACE("Our Complete Danger overall: %d\n",(m_pSectorAI->GetCompleteDanger(race) * 100 / (rand()%16+75)));
#endif
				return TRUE;
			}
		}
		// falls wir nicht st�rker sind, so fliegen wir zu dem Feld, wo unserer st�rkste Schiffsansammlung ist
		else if (m_pSectorAI->GetHighestShipDanger(race) != CPoint(-1,-1))
		{
			CPoint p = m_pSectorAI->GetHighestShipDanger(race);
			// hier noch �berpr�fen, ob der Sektor erreicht werden kann
			if (m_pDoc->starmap[race]->GetRange(p) <= m_pDoc->m_ShipArray.GetAt(index).GetRange())
			{
				m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(p, 0);
#ifdef TRACE_SHIPAI
				TRACE("RACE %d: ANSAMMLUNG in Sektor: %d/%d\n",race,p.x,p.y);
#endif
				return TRUE;
			}
		}
	}
	return FALSE;
}

/// Funktion schickt Kriegsschiffe zu einem m�glichen System, welches Bombardiert werden k�nnte. �bergeben wird
/// daf�r der index <code>index</code> des Schiffes aus dem Schiffsarray des Documents. Der R�ckgabewert der
/// Funktion ist <code>TRUE</code>, wenn ein Bewegungsbefehl gegeben werden konnte oder eine Bombardierung
/// befohlen wurde oder das Schiff auf Verst�rkung zur Bombardierung im System wartet.
BOOLEAN CShipAI::DoBombardSystem(int index)
{
	if (m_pDoc->m_ShipArray.GetAt(index).GetKO() == m_BombardSector[m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip()-1])
	{
		// Hier muss als erstes ein m�glicher neuer Kurs gel�scht werden und ein alter Systemangriffsbefehl aufgehoben werden
		m_pDoc->m_ShipArray.GetAt(index).SetTargetKO(m_pDoc->m_ShipArray.GetAt(index).GetKO(), 0);
		// Wenn die defensive Schiffsst�rke im System des Angreifers ungef�hr doppelt so gro� als der Systemverteidigungswert
		// des Verteidigers ist, wird angegriffen.
		int shipValue = 0;
		for (int i = 0; i < m_pDoc->m_ShipArray.GetSize(); i++)
			if (m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip() == m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip())
				if (m_pDoc->m_ShipArray.GetAt(i).GetKO() == m_pDoc->m_ShipArray.GetAt(index).GetKO())
				{
					if (m_pDoc->m_ShipArray.GetAt(i).GetCloak() == FALSE)
						shipValue += (int)m_pDoc->m_ShipArray.GetAt(i).GetHull()->GetCurrentHull();
					if (m_pDoc->m_ShipArray.GetAt(i).GetFleet() != NULL)
						for (int j = 0; j < m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); j++)
							if (m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetPointerOfShipFromFleet(j)->GetCloak() == FALSE)
								shipValue += (int)m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetPointerOfShipFromFleet(j)->GetHull()->GetCurrentHull();
					
				}
		int shipDefend = m_pDoc->GetSystem(m_pDoc->m_ShipArray.GetAt(index).GetKO().x, m_pDoc->m_ShipArray.GetAt(index).GetKO().y).GetProduction()->GetShipDefend();
		shipDefend *= 1.25;
		//CString s;
		//s.Format("shipValue = %d\nshipDefend = %d\nSektor = %s", shipValue, shipDefend, m_pDoc->GetSector(m_pDoc->m_ShipArray.GetAt(index).GetKO().x, m_pDoc->m_ShipArray.GetAt(index).GetKO().y).GetName(true));
		//AfxMessageBox(s);
		// Wenn das Schiff bzw Schiffe aus der Flotte getarnt sind, dann m�ssen diese erst enttarnt werden
		if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() || (m_pDoc->m_ShipArray.GetAt(index).GetFleet() != 0 && m_pDoc->m_ShipArray.GetAt(index).GetFleet()->CheckOrder(&m_pDoc->m_ShipArray.GetAt(index), ATTACK_SYSTEM) == FALSE))
		{
			// AfxMessageBox(m_pDoc->GetSector(m_pDoc->m_ShipArray.GetAt(index).GetKO()).GetName());
			// Schiff enttarnen
			m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
			return TRUE;
		}		
		if (shipValue > shipDefend)
		{		
			#ifdef TRACE_SHIPAI
			TRACE("RACE %d: Ship %s (%s) bombardiert System: %d,%d\n",m_pDoc->m_ShipArray.GetAt(index).GetOwnerOfShip(), m_pDoc->m_ShipArray.GetAt(index).GetShipName(), m_pDoc->m_ShipArray.GetAt(index).GetShipTypeAsString(), m_pDoc->m_ShipArray.GetAt(index).GetKO().x,m_pDoc->m_ShipArray.GetAt(index).GetKO().y);
			#endif
			m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(ATTACK_SYSTEM);
			return TRUE;
		}		
	}
	return FALSE;
}

/// Funktion erteilt einen Tarnbefehl, wenn <code>camouflage</code> wahr ist. Ist <code>camouflage</code> falsch,
/// wird ein Enttarnbefehl gegeben. �bergebn wird daf�r der index <code>index</code> des Schiffes aus dem
/// Schiffsarray des Documents. Der R�ckgabewert der Funktion ist <code>TRUE</code>, wenn der Befehl gegeben wurde.
BOOLEAN CShipAI::DoCamouflage(int index, BOOLEAN camouflage)
{
	// Nur wenn das Schiff sich tarnen kann und nicht gerade dabei ist ein System zu bombardieren soll es sich tarnen
	if (m_pDoc->m_ShipArray.GetAt(index).GetStealthPower() > 3 && m_pDoc->m_ShipArray.GetAt(index).GetCurrentOrder() != ATTACK_SYSTEM)
	{
		// wollen tarnen
		if (camouflage)
		{
			if (m_pDoc->m_ShipArray.GetAt(index).GetStealthPower() > 3)
				if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() == FALSE)
				{
					m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
					//m_pDoc->m_ShipArray.GetAt(index).SetCloak();
					return TRUE;
				}
		}
		// wollen enttarnen
		else
			if (m_pDoc->m_ShipArray.GetAt(index).GetCloak() == TRUE)
			{
				m_pDoc->m_ShipArray.GetAt(index).SetCurrentOrder(CLOAK);
				//m_pDoc->m_ShipArray.GetAt(index).SetCloak();
				return TRUE;
			}
	}
	return FALSE;
}

/// Funktion erstellt eine Flotte. Schiffe werden der Flotte nur hinzugef�gt, wenn diese bestimmte Voraussetzungen erf�llen.
/// So muss der ungef�hre Schiffstyp �bereinstimmen (Combat <-> NonCombat) sowie die Reichweite des Schiffes.
void CShipAI::DoMakeFleet(int index)
{
	CShip* ship = &m_pDoc->m_ShipArray.GetAt(index);
	if (ship->GetShipType() == OUTPOST || ship->GetShipType() == STARBASE)
		return;

	for (int i = index + 1; i < m_pDoc->m_ShipArray.GetSize(); i++)
		// Schiffe m�ssen von der selben Rasse sein
		if (ship->GetOwnerOfShip() == m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip())
		{
			// Schiffe m�ssen sich im selben Sektor befinden
			if (ship->GetKO() == m_pDoc->m_ShipArray.GetAt(i).GetKO())
			{
				// beide Schiffe m�ssen die selbe Reichweite haben
				if (ship->GetRange() == m_pDoc->m_ShipArray.GetAt(i).GetRange())
				{
					// das hinzuzuf�gende Schiff darf kein Au�enposten oder Sternbasis sein
					if (m_pDoc->m_ShipArray.GetAt(i).GetShipType() != OUTPOST && m_pDoc->m_ShipArray.GetAt(i).GetShipType() != STARBASE)
					{
						// das hinzuzuf�gende Schiff darf kein individuelles Ziel haben
						if (m_pDoc->m_ShipArray.GetAt(i).GetPath()->GetSize() == NULL)
						{
							// wenn sich das F�hrungsschiff tarnen kann, dann muss das hinzuzuf�gende Schiff sich auch tarnen k�nnen
							if ((ship->GetStealthPower() > 3 && m_pDoc->m_ShipArray.GetAt(i).GetStealthPower() > 3) || (ship->GetStealthPower() <= 3 && m_pDoc->m_ShipArray.GetAt(i).GetStealthPower() <= 3))
							{
								// es muss sich bei beiden Schiffen um Kriegsschiffe handeln oder bei beiden Schiffen um Transporter oder bei beiden
								// Schiffen um Kolonieschiffe
								if ((!IS_NONCOMBATSHIP(ship->GetShipType()) && !IS_NONCOMBATSHIP(m_pDoc->m_ShipArray.GetAt(i).GetShipType()))
									||(ship->GetShipType() == TRANSPORTER && m_pDoc->m_ShipArray.GetAt(i).GetShipType() == TRANSPORTER && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() < BUILD_OUTPOST)
									||(ship->GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetShipType() == COLONYSHIP && m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() < COLONIZE))
								{
									ship->CreateFleet();
									// hat das hinzuzuf�gende Schiff eine eigene Flotte
									if (m_pDoc->m_ShipArray.GetAt(i).GetFleet() != NULL)
									{
										// Schiffe aus der Flotte der neuen Flotte hinzuf�gen
										for (int n = 0; n < m_pDoc->m_ShipArray.GetAt(i).GetFleet()->GetFleetSize(); )
											ship->GetFleet()->AddShipToFleet(m_pDoc->m_ShipArray.GetAt(i).GetFleet()->RemoveShipFromFleet(n));
										m_pDoc->m_ShipArray.GetAt(i).CheckFleet();								
									}
									ship->GetFleet()->AddShipToFleet(m_pDoc->m_ShipArray.GetAt(i));
									m_pDoc->m_ShipArray.RemoveAt(i--);							
								}
							}
						}
					}
				}
			}
		}
}

/// Funktion erteilt einen Au�enpostenbaubefehl, sofern dies auch m�glich ist. �bergeben wird daf�r der index
/// <code>index</code> des Schiffes aus dem Schiffsarray des Dokuments. Der R�ckgabewert der Funktion ist
/// <code>TRUE</code>, wenn ein Au�enpostenbaubefehl gegeben werden k�nnte.
BOOLEAN CShipAI::DoStationBuild(int index)
{
	int i = index;
	if (m_pDoc->m_ShipArray.GetAt(i).GetStationBuildPoints() == NULL || m_pDoc->m_ShipArray.GetAt(i).GetCurrentOrder() == BUILD_OUTPOST)
		return FALSE;

	BYTE race = m_pDoc->m_ShipArray.GetAt(i).GetOwnerOfShip();
	if (m_pSectorAI->GetStationBuildSector(race).points <= MINBASEPOINTS)
		return FALSE;

	CPoint ko(m_pSectorAI->GetStationBuildSector(race).position.x, m_pSectorAI->GetStationBuildSector(race).position.y);
	if (m_pDoc->m_ShipArray.GetAt(i).GetKO() == ko)
	{
		if (m_pDoc->m_Sector[ko.x][ko.y].GetOwnerOfSector() == NOBODY || m_pDoc->m_Sector[ko.x][ko.y].GetOwnerOfSector() == race)
		{
			m_pDoc->m_ShipArray.GetAt(i).SetTargetKO(ko, 0);
			m_pDoc->m_ShipArray.GetAt(i).SetCurrentOrder(BUILD_OUTPOST);
			return TRUE;
		}
	}
	return FALSE;
}

/// Funkion berechnet einen m�glichen Angriffssektor, welcher sp�ter gesammelt angegriffen werden kann.
void CShipAI::CalcAttackSector(void)
{
	for (int i = HUMAN; i <= DOMINION; i++)
	{
		int nearestSector = MAXSHORT;
		// beinhaltet die Sektorkoordinate mit unserem Angriffsziel
		CPoint sector = CPoint(-1,-1);
		for (int j = 0; j < m_pSectorAI->GetOffensiveTargets(i)->GetSize(); j++)
		{
			int space = max(abs(m_pSectorAI->GetHighestShipDanger(i).x - m_pSectorAI->GetOffensiveTargets(i)->GetAt(j).x),
				abs(m_pSectorAI->GetHighestShipDanger(i).y - m_pSectorAI->GetOffensiveTargets(i)->GetAt(j).y));
			if (space < nearestSector)
			{
				// Wenn dieses Ziel eine kleinere Gefahr darstellt als unsere gr��te Flottenansammlung oder wir
				// in schon mindst 50% unserer Flotte hier versammelt haben, so wird es als Ziel aufgenommen
				if ((m_pSectorAI->GetDangerOnlyFromCombatShips(i, m_pSectorAI->GetHighestShipDanger(i)) > m_pSectorAI->GetCompleteDanger(i, m_pSectorAI->GetOffensiveTargets(i)->GetAt(j)))
					|| (m_pSectorAI->GetDangerOnlyFromCombatShips(i, m_pSectorAI->GetHighestShipDanger(i)) > (m_pSectorAI->GetCompleteDanger(i) * 50 / 100)))
				{
					nearestSector = space;
					sector = m_pSectorAI->GetOffensiveTargets(i)->GetAt(j);
				}
			}
		}
		m_AttackSector[i-1] = sector;
#ifdef TRACE_SHIPAI
		TRACE("RACE %d: Globaler AttackSector %d/%d\n",i, m_AttackSector[i-1].x, m_AttackSector[i-1].y);
#endif
	}
}

/// Funktion berechnet einen m�glich besten Sektor f�r eine Bombardierung. Wurde solch ein Sektor ermittelt hat dieser
/// die allerh�chste Priorit�t.
void CShipAI::CalcBombardSector(void)
{
	for (int i = HUMAN; i <= DOMINION; i++)
	{
		// die Hauptrassen bombardieren nicht alle gleichoft und gleich lang. Die F�deration z.B. kann wegen den
		// Moralabzu�gen nicht so lang bombardieren.
		BOOLEAN bombard = TRUE;
		switch (m_pDoc->GetMajorRace(i)->GetKind())
		{
			case NOTHING_SPECIAL:	break;
			case FINANCIAL:			break;
			case WARLIKE:			break;
			case FARMER:			if (m_iAverageMoral[i-1] < 90) bombard = FALSE; break;
			case INDUSTRIAL:		break;
			case SECRET:			break;
			case RESEARCHER:		if (m_iAverageMoral[i-1] < 110) bombard = FALSE; break;
			case PRODUCER:			break;
			case PACIFIST:			bombard = FALSE; break;
			case SNEAKY:			break;
		}
		if (!bombard)
			continue;
		// ist der globale Angriffssektor auch bei den Bombardierungzielen vorhanden, so wird dieser benutzt und kein
		// am n�chsten befindlicher Bombardierungssektor
		for (int j = 0; j < m_pSectorAI->GetBombardTargets(i)->GetSize(); j++)
			if (m_pSectorAI->GetBombardTargets(i)->GetAt(j) == m_AttackSector[i-1])
			{
				m_BombardSector[i-1] = m_pSectorAI->GetBombardTargets(i)->GetAt(j);
				break;				
			}
		// Wurde kein Bombardierungziel mit dem gleichen globalen Attackziel gefunden, so wird versucht ein n�chstes
		// zu unserer gr��ten Flottenansammlung zu finden
		if (m_BombardSector[i-1] == CPoint(-1,-1))
		{
			int nearestSector = MAXSHORT;
			// beinhaltet die Sektorkoordinate mit unserem Bombardierungsziel
			CPoint sector = CPoint(-1,-1);
			for (int j = 0; j < m_pSectorAI->GetBombardTargets(i)->GetSize(); j++)
			{
				// Es wird das Ziel ausgew�hlt, welches am n�chsten von unserer gr��ten Flottensammlung entfernt ist
				int space = max(abs(m_pSectorAI->GetHighestShipDanger(i).x - m_pSectorAI->GetBombardTargets(i)->GetAt(j).x),
					abs(m_pSectorAI->GetHighestShipDanger(i).y - m_pSectorAI->GetBombardTargets(i)->GetAt(j).y));
				if (space < nearestSector)
				{
					// Wenn dieses Ziel eine kleinere Gefahr darstellt als unsere gr��te Flottenansammlung, so wird
					// es als Ziel aufgenommen
					if (m_pSectorAI->GetDangerOnlyFromCombatShips(i, m_pSectorAI->GetHighestShipDanger(i)) > m_pSectorAI->GetCompleteDanger(i, m_pSectorAI->GetBombardTargets(i)->GetAt(j)))
					{
						nearestSector = space;
						sector = m_pSectorAI->GetBombardTargets(i)->GetAt(j);
					}
				}
			}
			m_BombardSector[i-1] = sector;
		}
#ifdef TRACE_SHIPAI
		TRACE("RACE %d: Globaler BombardierungsSector %d/%d\n",i, m_BombardSector[i-1].x, m_BombardSector[i-1].y);
#endif
	}
}
