// Starmap.cpp: implementation of the CStarmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Starmap.h"
#include "BotEDoc.h"
#include "Galaxy\Sector.h"
#include "Galaxy\Anomaly.h"
#include <math.h>



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// statische Variablen initialisieren
std::vector<double> CStarmap::m_BadMapModifiers(0, 0);


/**
 * @return <code>-1</code> wenn <code>x &lt; 0</code>, <code>0</code> wenn <code>x == 0</code>, <code>1</code> wenn <code>x &gt; 0</code>
 */
inline int sgn(int x)
{
	if (x == 0)
		return 0;
	else
		return x / abs(x);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStarmap::CStarmap(BOOL bAICalculation, char nAIRange) : m_bAICalculation(bAICalculation), m_nAIRange(nAIRange)
{

	AssertBotE(nAIRange >= SM_RANGE_FAR && nAIRange <= SM_RANGE_NEAR);

	// KI Berechnung mal auf immer TRUE gestellt. So wird bei einer Spielerrasse auch berechnet, aber nicht angewandt
	m_bAICalculation = TRUE;

	InitSomeMembers();

	// Standard-Rangemap
	CRangeMaps::CalcRangeMap(m_RangeMap, 0, CRangeMaps::CLASSIC);

	pathMap.resize(STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT);
}

void CStarmap::AllocateStatics()
{
	m_BadMapModifiers.resize(STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT, 0);
}

CStarmap::~CStarmap()
{
	if (m_RangeMap.range) delete[] m_RangeMap.range;
	m_AIRangePoints.clear();
	m_AINeighbourCount.clear();
	m_AIConnectionPoints.clear();
	m_AITargetPoints.clear();
	m_AIBadPoints.clear();
	pathMap.clear();
}

unsigned char CStarmap::GetRangeMapValue(char x, char y)
{
	x += m_RangeMap.x0;
	y += m_RangeMap.y0;
	AssertBotE(0 <= x && x < m_RangeMap.w && 0 <= y && y < m_RangeMap.h);
	return m_RangeMap.range[y * m_RangeMap.w + x];
}

Sector CStarmap::GetClickedSector(const CPoint &pt)
{
	Sector result;
	result.x = result.y = -1;

	// wenn innerhalb der Karte geklickt, dann Koordinaten umrechnen
	if (PT_IN_RECT(pt, 0, 0, STARMAP_TOTALWIDTH, STARMAP_TOTALHEIGHT))
	{
		result.x = pt.x / STARMAP_SECTOR_WIDTH;
		result.y = pt.y / STARMAP_SECTOR_HEIGHT;
		AssertBotE(result.on_map());
	}

	return result;
}

void CStarmap::Select(const Sector &sector)
{
	if (sector.x > -1 && sector.y > -1)
	{
		AssertBotE(sector.on_map());
		m_Selection = sector;
	}
}

void CStarmap::Deselect()
{
	m_Selection.x = m_Selection.y = -1;
}

CPoint CStarmap::GetSectorCoords(const Sector& sector)
{
	AssertBotE(sector.on_map());
	return CPoint(sector.x * STARMAP_SECTOR_WIDTH, sector.y * STARMAP_SECTOR_HEIGHT);
}

BOOL CStarmap::IsBase(const Sector &sector)
{
	for (std::list<Sector>::const_iterator it = m_lBases.begin(); it != m_lBases.end(); ++it)
		if (it->x == sector.x && it->y == sector.y)
			return TRUE;
	return FALSE;
}

void CStarmap::SetFullRangeMap(int nRange/* = SM_RANGE_NEAR*/, const std::vector<Sector>& vExceptions/* = std::vector<Sector>()*/)
{
	for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (vExceptions.empty() || std::find(vExceptions.begin(), vExceptions.end(), Sector(x, y)) == vExceptions.end())
				m_Range.at(CoordsToIndex(x, y)) = nRange;
}

void CStarmap::AddBase(const Sector &sector, BYTE propTech, CRangeMaps::EXPANSION_SPEED speed)
{
	AssertBotE(sector.on_map());

	// merken, dass Sektor einen Au�enposten besitzt; falls der Au�enposten schon vorhanden ist, die folgende
	// Berechnung trotzdem durchf�hren, da eine andere <code>rangeMap</code> vorgegeben sein k�nnte.
	if (!IsBase(sector)) m_lBases.push_back(sector);

	// --- Map mit Entfernungen aktualisieren ---
	CRangeMaps::CalcRangeMap(m_RangeMap, propTech, speed);

	// lokale Rangemap durchlaufen
	for (char x = -m_RangeMap.x0; x < m_RangeMap.w - m_RangeMap.x0; x++)
		for (char y = -m_RangeMap.y0; y < m_RangeMap.h - m_RangeMap.y0; y++)
		{
			Sector pt(sector.x + x, sector.y + y);
			if (pt.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT))
			{
				// Wert �berschreiben, wenn der neue Einfluss gr��er ist
				m_Range.at(CoordsToIndex(pt.x, pt.y)) = max(m_Range.at(CoordsToIndex(pt.x, pt.y)), GetRangeMapValue(x, y));
			}
		}

	// pathStart zur�cksetzen, damit s�mtliche Wege beim n�chsten Aufruf von
	// CalcPath() neu berechnet werden
	pathStart = Sector();

#ifdef DEBUG_AI_BASE_DEMO
	// ACHTUNG: verwenden i. A. falsche RangeMap!
	RecalcRangePoints();
	RecalcConnectionPoints();
	RecalcTargetPoints();
#endif
}

// Gleicht die komplette Rangemap mit der Galaxiemap ab. Gebiete, welche einer Rasse geh�ren mit der wir einen
// Nichtangriffspakt haben, werden von der Rangemap entfernt. �bergeben werden daf�r ein Zeiger auf alle
// Sektoren <code>sectors</code> und ein Wahrheitswert <code>races</code> f�r alle Rassen, ob wir einen
// Nichtangriffspakt mit dieser Rasse haben.
void CStarmap::SynchronizeWithMap(const std::vector<CSystem>& systems, const std::set<CString>* races)
{
	for(std::vector<CSystem>::const_iterator it = systems.begin(); it != systems.end(); ++it)
		if(!it->Free())
		{
			const CPoint& co = it->GetCo();
			if (m_Range.at(CoordsToIndex(co.x, co.y)) > 0 && races->find(it->OwnerID())!= races->end())
				m_Range.at(CoordsToIndex(co.x, co.y)) = 0;
		}
}

// F�hrt f�r gef�hrliche Anomalien mathematische Gewichte hinzu, so dass dieser Sektor bei der automatischen
// Wegsuche nicht �berflogen wird. Au�erdem wird solch ein Sektor auch nicht f�r einen Au�enpostenbau bestimmt.
void CStarmap::SynchronizeWithAnomalies(const std::vector<CSystem>& systems)
{
	for(std::vector<CSystem>::const_iterator it = systems.begin(); it != systems.end(); ++it)
	{
		const boost::shared_ptr<const CAnomaly>& anomaly = it->GetAnomaly();
		if (anomaly)
			m_BadMapModifiers.at(it - systems.begin()) = anomaly->GetWaySearchWeight();
	}
}

void CStarmap::InitSomeMembers()
{
	const int size = STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT;
	// m_Range komplett mit RANGE_SPACE f�llen
	m_Range.resize(size, SM_RANGE_SPACE);
	m_AINeighbourCount.resize(size, 0);
	m_AIRangePoints.resize(size, 0);
	m_AIConnectionPoints.resize(size, 0);
	m_AITargetPoints.resize(size, 0);
	m_AIBadPoints.resize(size, 0);
}

// Kantengewichte; das diagonale Kantengewicht muss h�her sein, da ein Weg sonst
// statt geradeaus auch im Zickzack verlaufen k�nnte
//#define WEIGHT_DIAG 1.41421356	// diagonal	(original von CBot)
#define WEIGHT_DIAG 1.4143			// diagonal (modded bei Sir Pustekuchen)
#define WEIGHT_DIR  1.				// horizontal, vertikal

Sector CStarmap::CalcPath(const Sector &pos, const Sector &target, unsigned char range,
	unsigned char speed, CArray<Sector> &path)
{
	AssertBotE(pos.on_map());
	AssertBotE(target.on_map());

	// bisherige Eintr�ge von path l�schen
	path.RemoveAll();

	// gegebene Parameter pr�fen
	if (pos == target								// Start == Ziel
		|| range < 1 || range > 3
		|| m_Range.at(CoordsToIndex(pos.x, pos.y)) < range			// Start au�erhalb des Gebiets der Reichweite
		|| m_Range.at(CoordsToIndex(target.x, target.y)) < range		// Ziel au�erhalb der Reichweite
		|| speed < 1)
	{
		return Sector();
	}

	// Array zur Berechnung der Koordinaten s�mtlicher Nachbarn eines Sektors (schr�g/gerade abwechselnd,
	// mit schr�g beginnend)
	Sector neighbours[8] = {Sector(-1, -1), Sector(0, -1), Sector(1, -1), Sector(1, 0), Sector(1, 1),
		Sector(0, 1), Sector(-1, 1), Sector(-1, 0)};

	// Berechnung neu beginnen?
	if (pos != pathStart || range != pathRange)
	{
		// pathMap zur�cksetzen
		for (int j = 0; j < STARMAP_SECTORS_VCOUNT; j++)
			for (int i = 0; i < STARMAP_SECTORS_HCOUNT; i++)
			{
				/*PathSector *tmp = &(pathMap.at(CoordsToIndex(i, j)));
				tmp->used = false;
				tmp->distance = 0.;
				tmp->hops = 0;
				tmp->parent.x = tmp->parent.y = -1;

				tmp->position.x = i; // f�r Zugriff aus leafList heraus merken
				tmp->position.y = j;*/
				const int index = CoordsToIndex(i, j);
				pathMap.at(index).used=false;
				pathMap.at(index).distance=0;
				pathMap.at(index).hops=0;
				pathMap.at(index).parent.x=-1;
				pathMap.at(index).parent.y=-1;
				pathMap.at(index).position.x=i;
				pathMap.at(index).position.y=j;
			}

		// leaves zur�cksetzen
		leaves.Clear();

		// Startknoten zur Liste der auszuw�hlenden Bl�tter hinzuf�gen
		leaves.Add(&(pathMap.at(CoordsToIndex(pos.x, pos.y))));

		// Parameter merken
		pathStart = pos;
		pathRange = range;
	}

	// ist der Weg zum angegebenen Ziel bereits bekannt?
	if (pathMap.at(CoordsToIndex(target.x, target.y)).parent.x == -1 || pathMap.at(CoordsToIndex(target.x, target.y)).parent.y == -1)
	{
		// k�rzeste Wege zu allen anderen Knoten bestimmen, bis uns der Zielknoten �ber den Weg l�uft
		bool found = false;
		while (!found)
		{
			// Zeiger auf ein neues Blatt mit einer k�rzesten Gesamtentfernung zum
			// Start-Sektor ermitteln
			PathSector *next = leaves.PopFirst();
			if (!next) return Sector(); // keine Knoten mehr, Zielknoten ist nicht erreichbar
			if (next->used) continue; // Knoten wurde schonmal gew�hlt

			// Knoten als ausgew�hlt markieren
			next->used = true;

			// bisher noch nicht ausgew�hlte Nachbarn innerhalb der Reichweite in leaves
			// eintragen;
			// die Nachbarn m�ssen auch eingetragen werden, wenn next bereits der Zielknoten ist,
			// da der n�chste Aufruf von CalcPath() die Zwischenergebnisse wiederverwendet
			for (int i = 0; i < 8; i++)
			{
				// Koordinaten des Nachbarn ermitteln (Sektoren nur betrachten, wenn sie
				// noch auf der Starmap liegen!)
				Sector npos = next->position + neighbours[i];
				if (!npos.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT))
					continue;

				// nur Nachbarn betrachten, die noch nicht ausgew�hlt wurden und innerhalb der
				// Reichweite liegen
				PathSector *neighb = &(pathMap.at(CoordsToIndex(npos.x, npos.y)));
				if (neighb->used || m_Range.at(CoordsToIndex(npos.x, npos.y)) < range)
					continue;

				// kann der Nachbar �ber next auf einem k�rzeren Weg als bisher erreicht werden,
				// dann die bisherige Info �berschreiben
				double distance = next->distance + ((i % 2) ? WEIGHT_DIR : WEIGHT_DIAG);
				// Anomalien beachten
				distance += m_BadMapModifiers.at(next->position.x + next->position.y * STARMAP_SECTORS_HCOUNT);

				if (neighb->distance == 0. || distance < neighb->distance)
				{
					// (distance ist f�r alle anderen Sektoren au�er dem Start-Sektor > 0.,
					// der Wert 0. weist darauf hin, dass distance noch nicht gesetzt wurde)

					neighb->distance = distance;
					neighb->hops = next->hops + 1;
					neighb->parent = next->position;

					// den Knoten in leaves neu einsortieren (derselbe Knoten ist evtl. unter
					// einer anderen Entfernung fr�her bereits einsortiert worden; da nun die
					// Entfernung aber k�rzer ist, wird das neu einsortierte Element zuerst
					// gew�hlt; liefert die Liste eines der vorher eingeordneten Elemente, ist
					// dessen used-Feld bereits true und es wird sofort mit dem n�chsten Eintrag
					// fortgesetzt);
					// @TODO besser w�re es, den fr�her einsortierten Knoten zu entfernen
					leaves.Add(neighb);
				}
			}

			if (next->position == target) found = true; // Zielknoten gefunden
		}
	}

	// Ziel gefunden; Weg vom Ziel bis zum Startknoten zur�ck verfolgen,
	// dabei von hinten beginnend in Array eintragen
	Sector next = target;
	int idx = pathMap.at(CoordsToIndex(target.x, target.y)).hops;
	AssertBotE(idx >= 1);

	path.SetSize(idx); // Gr��e des Arrays setzen (= L�nge des Weges)

	while (next.x > -1 && next.y > -1 && --idx >= 0) // Start-Sektor nicht mit eintragen
	{
		AssertBotE(next.on_map());
		path[idx] = next;
		next = pathMap.at(CoordsToIndex(next.x, next.y)).parent;
	}
	AssertBotE(idx == -1);

	// entsprechend speed den n�chsten Knoten des Weges zur�ckgeben; bzw. den Zielknoten,
	// wenn der Weg k�rzer ist
	return path[min(speed - 1, path.GetUpperBound())];
}

////////////////////////////////////////////////////////////////////////////
// private Funktionen
////////////////////////////////////////////////////////////////////////////

void CStarmap::AddTarget(const Sector &target)
{
	AssertBotE(target.on_map());
	AssertBotE(m_bAICalculation);
	if (!m_bAICalculation || !target.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT)) return;

	// pr�fen, ob Ziel bereits in Liste vorhanden ist
	for (std::list<Sector>::const_iterator it = m_lAITargets.begin(); it != m_lAITargets.end(); ++it)
		if (*it == target)
			return;

	// sonst hinzuf�gen
	m_lAITargets.push_back(target);

#ifdef DEBUG_AI_BASE_DEMO
	// Bewertung f�r Ausbreitungsrichtungen aktualisieren
	// ACHTUNG: Berechnung hier nicht notwendig
	RecalcTargetPoints();
#endif
}

BOOL CStarmap::IsTarget(const Sector &sector)
{
	for (std::list<Sector>::const_iterator it = m_lAITargets.begin(); it != m_lAITargets.end(); ++it)
		if (it->x == sector.x && it->y == sector.y)
			return TRUE;
	return FALSE;
}

void CStarmap::AddKnownSystem(const Sector &sector)
{
	AssertBotE(sector.on_map());
	AssertBotE(m_bAICalculation);
	if (!m_bAICalculation || !sector.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT)) return;

	// pr�fen, ob Ziel bereits in Liste vorhanden ist
	for (std::list<Sector>::const_iterator it = m_lAIKnownSystems.begin(); it != m_lAIKnownSystems.end(); ++it)
		if (*it == sector)
			return;

	// sonst hinzuf�gen
	m_lAIKnownSystems.push_back(sector);

#ifdef DEBUG_AI_BASE_DEMO
	// Bewertung f�r Ausbreitungsrichtungen aktualisieren
	// ACHTUNG: Berechnung hier nicht notwendig
	RecalcTargetPoints();
#endif
}

BOOL CStarmap::IsKnownSystem(const Sector &sector)
{
	for (std::list<Sector>::const_iterator it = m_lAIKnownSystems.begin(); it != m_lAIKnownSystems.end(); ++it)
		if (it->x == sector.x && it->y == sector.y)
			return TRUE;
	return FALSE;
}

void CStarmap::RecalcRangePoints()
{
	//memset(m_AIRangePoints, 0, STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT * sizeof(short));
	for(int i=0;i<STARMAP_SECTORS_HCOUNT;i++)
		for(int j=0;j<STARMAP_SECTORS_VCOUNT;j++)
			m_AIRangePoints.at(CoordsToIndex(i, j))=0;

	// komplette Starmap durchlaufen, Werte aus Effektivit�tsgr�nden nur f�r Sektoren innerhalb der gegebenen
	// Reichweite berechnen
	for (char x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (char y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (m_Range.at(CoordsToIndex(x, y)) >= m_nAIRange)
			{
				// f�r Sektoren innerhalb der gegebenen Reichweite Bewertung neu berechnen, die sich ergibt,
				// wenn hier ein Au�enposten gebaut w�rde

				m_AIRangePoints.at(CoordsToIndex(x, y)) = 0;

				// lokale Rangemap durchlaufen
				for (char mx = -m_RangeMap.x0; mx < m_RangeMap.w - m_RangeMap.x0; mx++)
					for (char my = -m_RangeMap.y0; my < m_RangeMap.h - m_RangeMap.y0; my++)
					{
						Sector mpt(x + mx, y + my);
						if (mpt.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT))
						{
							// Gebietszuwachs ermitteln
							m_AIRangePoints.at(CoordsToIndex(x, y)) += max(GetRangeMapValue(mx, my) - m_Range.at(CoordsToIndex(mpt.x, mpt.y)), 0);
						}
					}
			}
}

void CStarmap::RecalcConnectionPoints()
{
	//memset(m_AINeighbourCount, 0, STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT * sizeof(unsigned char));
	for(int i=0;i<STARMAP_SECTORS_HCOUNT;i++)
		for(int j=0;j<STARMAP_SECTORS_VCOUNT;j++)
			m_AINeighbourCount.at(CoordsToIndex(i, j))=0;
	//memset(m_AIConnectionPoints, 0, STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT * sizeof(short));

	for(int i=0;i<STARMAP_SECTORS_HCOUNT;i++)
		for(int j=0;j<STARMAP_SECTORS_VCOUNT;j++)
			m_AIConnectionPoints.at(CoordsToIndex(i, j))=0;

	// f�r Sektoren, die au�erhalb der Reichweite liegen, die Anzahl der Nachbarn innerhalb der Reichweite neu bestimmen
	for (char x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (char y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (m_Range.at(CoordsToIndex(x, y)) >= m_nAIRange)
			{
				// Nachbaranzahl f�r Nachbarsektoren von (x, y), die au�erhalb der Reichweite liegen, hochz�hlen
				for (int nx = -1; nx <= 1; nx++)
					for (int ny = -1; ny <= 1; ny++)
					{
						Sector npt(x + nx, y + ny);
						if (npt.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT) &&
							m_Range.at(CoordsToIndex(npt.x, npt.y)) < m_nAIRange)
						{
							// (npt.x, npt.y) ist jetzt immer != (x, y)
							m_AINeighbourCount.at(CoordsToIndex(npt.x, npt.y))++;
						}
					}
			}

	// Bewertungen f�r Zusammenhang neu berechnen
	for (char x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (char y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (m_Range.at(CoordsToIndex(x, y)) >= m_nAIRange)
			{
				// f�r Sektoren (x, y) innerhalb der Reichweite die lokale Rangemap durchlaufen
				for (char mx = -m_RangeMap.x0; mx < m_RangeMap.w - m_RangeMap.x0; mx++)
					for (char my = -m_RangeMap.y0; my < m_RangeMap.h - m_RangeMap.y0; my++)
					{
						Sector mpt(x + mx, y + my);
						if (mpt.is_in_rect(0, 0, STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT))
						{
							// f�r die (x, y) umgebenden Sektoren deren Anzahl der Nachbarn innerhalb der Reichweite
							// verwerten: ist diese Anzahl hoch, befindet sich in der N�he von (x, y) ein weiteres
							// Gebiet innerhalb der Reichweite, das mit dem Gebiet von (x, y) nicht "direkt" zusammenh�ngt
							m_AIConnectionPoints.at(CoordsToIndex(x, y)) += m_AINeighbourCount.at(CoordsToIndex(mpt.x, mpt.y)) * GetRangeMapValue(mx, my);
						}
					}
			}
}

#define NEAR_TARGET_DISTANCE	5			///< Entfernung, die f�r ein Ziel als "nah" interpretiert wird
#define NEAR_SYSTEM_DISTANCE	3			///< Entfernung, die f�r ein bekanntes System als "nah" interpretiert wird

void CStarmap::RecalcTargetPoints()
{
	//memset(m_AITargetPoints, 0, STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT * sizeof(short));
	for(int i=0;i<STARMAP_SECTORS_HCOUNT;i++)
		for(int j=0;j<STARMAP_SECTORS_VCOUNT;j++)
			m_AITargetPoints.at(CoordsToIndex(i, j))=0;

	// abbrechen, wenn keine Ziele angegeben und keine Systeme bekannt sind
	if (m_lAITargets.empty() && m_lAIKnownSystems.empty()) return;

	// alle Sektoren innerhalb der Reichweite durchlaufen
	for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (m_Range.at(CoordsToIndex(x, y)) >= m_nAIRange)
			{
				// alle Ziele untersuchen
				for (std::list<Sector>::const_iterator it = m_lAITargets.begin(); it != m_lAITargets.end(); ++it)
				{
					// Ziel �berspringen, wenn es sich innerhalb der gegebenen Reichweite befindet
					if (m_Range.at(CoordsToIndex(it->x, it->y)) >= m_nAIRange) continue;

					// z�hlen, wieviele Sektoren innerhalb der Reichweite besucht werden, wenn man
					// vom Sektor (x, y) aus horizontal zu (tx, y) und vertikal zu (x, ty) l�uft,
					// wobei (tx, ty) die Koordinaten des Zielsektors sind
					bool intersect = false;

					short incx = sgn(it->x - x), px = x + incx;
					while (incx)
					{
						if (m_Range.at(CoordsToIndex(px, y)) >= m_nAIRange)
						{
							intersect = true; break;
						}
						if (px == it->x) break;
						px += incx;
					}

					short incy = sgn(it->y - y), py = y + incy;
					while (incy && !intersect)
					{
						if (m_Range.at(CoordsToIndex(x, py)) >= m_nAIRange)
						{
							intersect = true; break;
						}
						if (py == it->y) break;
						py += incy;
					}

					// Bonus, wenn keine Sektoren innerhalb der Reichweite besucht wurden
					if (!intersect) m_AITargetPoints.at(CoordsToIndex(x, y)) += 20;

					// Entfernung zum Ziel ermitteln (da (x, y) innerhalb der Reichweite und das Ziel au�erhalb liegt,
					// ist diese Entfernung >= 1)
					short distance = max(abs(it->x - x), abs(it->y - y));
					if (distance <= NEAR_TARGET_DISTANCE)
						m_AITargetPoints.at(CoordsToIndex(x, y)) += (NEAR_TARGET_DISTANCE - distance + 1) * 5;
				}

				// die bekannten Systeme untersuchen
				for (std::list<Sector>::const_iterator it = m_lAIKnownSystems.begin(); it != m_lAIKnownSystems.end(); ++it)
				{
					// System �berspringen, wenn es sich bereits innerhalb der gegebenen Reichweite befindet
					if (m_Range.at(CoordsToIndex(it->x, it->y)) >= m_nAIRange) continue;

					// Bonus, wenn der Bau eines Au�enpostens in (x, y) das System in die gegebene Reichweite
					// eingliedern w�rde
					Sector system(it->x - x, it->y - y);
					if (system.is_in_rect(-m_RangeMap.x0, -m_RangeMap.y0, m_RangeMap.w - m_RangeMap.x0, m_RangeMap.h - m_RangeMap.y0)
						&& GetRangeMapValue(system.x, system.y) >= m_nAIRange)
					{
						m_AITargetPoints.at(CoordsToIndex(x, y)) += 30;
					}
					// sonst in Abh�ngigkeit von der Entfernung zum System
					else
					{
						short distance = max(abs(it->x - x), abs(it->y - y));
						if (distance <= NEAR_SYSTEM_DISTANCE)
							m_AITargetPoints.at(CoordsToIndex(x, y)) += (NEAR_SYSTEM_DISTANCE - distance + 1) * 5;
					}
				}
			}
}

short CStarmap::GetPoints(const Sector &sector) const
{
	short points = m_AIRangePoints.at(CoordsToIndex(sector.x, sector.y));
	if (points > 0)
	{
		// Verbinden von Gebieten und bevorzugte Ausbreitungsrichtung sind nur dort sinnvoll,
		// wo Gebietszuwachs erreicht wird
		points += m_AIConnectionPoints.at(CoordsToIndex(sector.x, sector.y)) + m_AITargetPoints.at(CoordsToIndex(sector.x, sector.y));
	}

	points -= m_AIBadPoints.at(CoordsToIndex(sector.x, sector.y));
	return points;

//	return m_AIRangePoints.at(CoordsToIndex(sector.x, sector.y));
//	return m_AIConnectionPoints.at(CoordsToIndex(sector.x, sector.y));
//	return m_AITargetPoints.at(CoordsToIndex(sector.x, sector.y));
}

void CStarmap::SetBadAIBaseSectors(const std::vector<CSystem>& systems, const CString& race)
{
	//memset(m_AIBadPoints, 0, STARMAP_SECTORS_HCOUNT * STARMAP_SECTORS_VCOUNT * sizeof(short));
	for(int i=0;i<STARMAP_SECTORS_HCOUNT;i++)
		for(int j=0;j<STARMAP_SECTORS_VCOUNT;j++)
			m_AIBadPoints.at(CoordsToIndex(i, j))=0;

	for(std::vector<CSystem>::const_iterator it = systems.begin(); it != systems.end(); ++it)
	{
		const CPoint& co = it->GetCo();
		const CString& owner = it->OwnerID();
		if (m_Range.at(CoordsToIndex(co.x, co.y)) >= m_nAIRange)
		{
			double dValue = 0.0;
			if (owner == race || it->Free())
			{
				int number = 0;
				// in einem Umkreis von einem Sektor um den Sektor scannen
				for (int j = -1; j <= 1; j++)
					for (int i = -1; i <= 1; i++)
						if (co.y + j > -1 && co.y + j < STARMAP_SECTORS_VCOUNT && co.x + i > -1 && co.x + i < STARMAP_SECTORS_HCOUNT)
							if (systems.at(co.x+i+(co.y+j)*STARMAP_SECTORS_HCOUNT).OwnerID() != race && !systems.at(co.x+i+(co.y+j)*STARMAP_SECTORS_HCOUNT).Free())
								number++;
				dValue += (50.0 * number);
			}
			else
				dValue += 1000.0;

			if (it->GetAnomaly())
				dValue += it->GetAnomaly()->GetWaySearchWeight() * 100.0;

			if ((double)m_AIBadPoints.at(CoordsToIndex(co.x, co.y)) + dValue > MAXSHORT)
				m_AIBadPoints.at(CoordsToIndex(co.x, co.y)) = MAXSHORT;
			else
				m_AIBadPoints.at(CoordsToIndex(co.x, co.y)) += (short)dValue;
		}
	}
}

BaseSector CStarmap::CalcAIBaseSector(double variance)
{
	AssertBotE(m_bAICalculation);
	AssertBotE(0. <= variance && variance <= 1.);

	if (!m_bAICalculation) return BaseSector();
	variance = min(max(variance, 0.), 1.);

	// Bewertung der Sektoren neu berechnen
	RecalcRangePoints();
	RecalcConnectionPoints();
	RecalcTargetPoints();

	// Sektoren innerhalb der Reichweite mit Gebietszuwachs + zus�tzlicher Bewertung in Liste aufnehmen
	std::list<BaseSector> lSectors;
	for (int x = 0; x < STARMAP_SECTORS_HCOUNT; x++)
		for (int y = 0; y < STARMAP_SECTORS_VCOUNT; y++)
			if (m_Range.at(CoordsToIndex(x, y)) >= m_nAIRange)
			{
				BaseSector sector;
				sector.position = Sector(x, y);
				sector.points = GetPoints(sector.position);
				lSectors.push_back(sector);
			}

	// abbrechen, wenn kein Sektor innerhalb der Reichweite existiert
	if (lSectors.empty()) return BaseSector();

	// nach Gesamtbewertung sortieren
	lSectors.sort();

	// die Bewertungsstufen bestimmen, die innerhalb der besten (100 * variance)% liegen
	int count = 0;
	short max_points = -1, old_points = -1;

	for (std::list<BaseSector>::const_iterator it = lSectors.begin(); it != lSectors.end(); ++it)
	{
		if (max_points == -1) max_points = it->points;
		if ((double)(max_points - it->points) / max_points > variance)
			break;
		if (it->points != old_points)
		{
			old_points = it->points;
			count += it->points;
		}
	}

	// eine der Stufen zuf�llig bestimmen, h�here Stufen mit h�herer Wahrscheinlichkeit w�hlen
	int n = (int)(((double)rand() / RAND_MAX) * count);

	old_points = -1;
	for (std::list<BaseSector>::const_iterator it = lSectors.begin(); it != lSectors.end(); ++it)
	{
		if (it->points != old_points)
		{
			old_points = it->points;
			n -= it->points;
			if (n < 0)
			{
				n = it->points;
				break;
			}
		}
	}

	// Eintr�ge der gew�hlten Stufe z�hlen
	count = 0;
	for (std::list<BaseSector>::const_iterator it = lSectors.begin(); it != lSectors.end(); ++it)
	{
		if (it->points == n)
			count++;
		else if (it->points < n)
			break;
	}

	// einen der Eintr�ge gleichwahrscheinlich w�hlen
	int m = (int)(((double)rand() / RAND_MAX) * count);
	for (std::list<BaseSector>::const_iterator it = lSectors.begin(); it != lSectors.end(); ++it)
	{
		if (it->points == n)
		{
			if (!m--) return *it;
		}
		else if (it->points < n)
			break;
	}

	// bei sonstigen Fehlern
	return BaseSector();
}

static bool CheckCoords(int x, int y, const CString& owned_by, const std::vector<CSystem>& systems)
{
	if(!IsOnMap(x, y))
		return false;
	const CSystem& system = systems.at(CoordsToIndex(x, y));
	return system.OwnerID() == owned_by && system.GetShipPort(owned_by);
}

CPoint CStarmap::NearestPort(const std::vector<CSystem>& systems, const CPoint& start, const CString& owned_by)
{
	AssertBotE(IsOnMap(start.x, start.y));
	if(CheckCoords(start.x, start.y, owned_by, systems))
		return start;

	for(int radius = 1; radius < max(STARMAP_SECTORS_HCOUNT, STARMAP_SECTORS_VCOUNT); ++radius)
	{
		const int from_x = start.x - radius;
		const int to_x = start.x + radius;
		const int from_y = start.y - radius;
		const int to_y = start.y + radius;
		int x = from_x;
		int y = from_y;
		for(; x < to_x; ++x)
		{
			if(CheckCoords(x, y, owned_by, systems))
				return CPoint(x, y);
		}
		for(; y < to_y; ++y)
		{
			if(CheckCoords(x, y, owned_by, systems))
				return CPoint(x, y);
		}
		for(; x > from_x; --x)
		{
			if(CheckCoords(x, y, owned_by, systems))
				return CPoint(x, y);
		}
		for(; y > from_y; --y)
		{
			if(CheckCoords(x, y, owned_by, systems))
				return CPoint(x, y);;
		}
	}

	return CPoint(0, 0);
}

