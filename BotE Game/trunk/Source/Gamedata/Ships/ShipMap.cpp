#include "stdafx.h"



#include "ShipMap.h"
#include "Ships.h"
#include "System/System.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CShipMap::CShipMap(std::vector<CSystem>* const systems, bool map_tile) :
	m_Ships(),
	m_NextKey(0),
	m_Systems(systems),
	m_bMapTile(map_tile)
{
	m_CurrentShip = begin();
	m_FleetShip = begin();

	AssertBotE(!m_Systems || !m_bMapTile);
}

CShipMap::~CShipMap(void)
{
	Reset();
}

CShipMap::CShipMap(const CShipMap& o) :
	m_Systems(o.m_Systems),
	m_bMapTile(o.m_bMapTile)
{
	Copy(o);
	AssertBotE(!m_Systems || !m_bMapTile);
}

CShipMap& CShipMap::operator=(const CShipMap& o)
{
	if(this == &o)
		return *this;

	Reset();
	Copy(o);

	AssertBotE(m_Systems == o.m_Systems && m_bMapTile == o.m_bMapTile);
	AssertBotE(!m_Systems || !m_bMapTile);
	return *this;
}

void CShipMap::Copy(const CShipMap& o)
{
	m_NextKey = o.m_NextKey;

	m_Ships = o.m_Ships;

	m_CurrentShip = begin();
	m_FleetShip = begin();
	if(!o.empty()) {
		m_CurrentShip = find(o.CurrentShip()->second->Key());
		m_FleetShip = find(o.FleetShip()->second->Key());
	}
}

//////////////////////////////////////////////////////////////////////
// getting iterators
//////////////////////////////////////////////////////////////////////

CShipMap::const_iterator CShipMap::begin() const {
	return m_Ships.begin();
}
CShipMap::const_iterator CShipMap::end() const {
	return m_Ships.end();
}

CShipMap::iterator CShipMap::begin() {
	return m_Ships.begin();
}
CShipMap::iterator CShipMap::end() {
	return m_Ships.end();
}

CShipMap::const_iterator CShipMap::find(unsigned key) const {
	return m_Ships.find(key);
}
CShipMap::iterator CShipMap::find(unsigned key) {
	return m_Ships.find(key);
}

static bool advance_by(int& advance, int size) {
	AssertBotE(0 <= advance && advance <= size);
	if(advance >= size/2) {
		advance = advance - size;
		return true;
	}
	else
		return false;
}
CShipMap::const_iterator CShipMap::iterator_at(int index) const {
	CShipMap::const_iterator i = begin();
	if(advance_by(index, GetSize()))
		i = end();
	std::advance(i, index);
	return i;
}
CShipMap::iterator CShipMap::iterator_at(int index) {
	CShipMap::iterator i = begin();
	if(advance_by(index, GetSize()))
		i = end();
	std::advance(i, index);
	return i;
}

//////////////////////////////////////////////////////////////////////
// adding elements
//////////////////////////////////////////////////////////////////////

CShipMap::iterator CShipMap::Add(const boost::shared_ptr<CShips>& ship) {
	//if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
	//	CString s;
	//	s.Format("CShipMap: adding ship %s", ship.GetName());
	//	MYTRACE("ships")(MT::LEVEL_INFO, s);
	//}

	const unsigned key = NextKey();
#ifdef CONSISTENCY_CHECKS
	//no duplicates
	for(const_iterator it = begin(); it != end(); ++it)
		AssertBotE(it->second != ship);
#endif
	CShipMap::iterator result = m_Ships.insert(end(), std::make_pair(key, ship));
	if(m_Systems)
	{
		const CPoint& co = result->second->GetCo();
		m_Systems->at(CoordsToIndex(co.x, co.y)).AddShip(result->second);
	}
	if(GetSize() == 1)
	{
		m_CurrentShip = result;
		m_FleetShip = result;
	}
	CShipMap::iterator temp = result;
	m_bMapTile ? result->second->SetMapTileKey(key) : result->second->SetKey(key);
	++temp;
	AssertBotE(temp == end());
	return result;
}

void CShipMap::Append(const CShipMap& other) {
	//if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
	//	CString s;
	//	s.Format("\nCShipMap: appending shiparray:\n%s\n", other.ToString());
	//	MYTRACE("ships")(MT::LEVEL_INFO, s);
	//}
	for(CShipMap::const_iterator i = other.begin(); i != other.end(); ++i)
		Add(i->second);
}

//////////////////////////////////////////////////////////////////////
// removing elements
//////////////////////////////////////////////////////////////////////

void CShipMap::Reset() {
	for(CShipMap::iterator i = begin(); i != end(); ++i)
	{
		AssertBotE(i->second);
		i->second.reset();
	}
	m_Ships.clear();
	m_NextKey = 0;
	m_CurrentShip = begin();
	m_FleetShip = begin();

	if(m_Systems)
		for(std::vector<CSystem>::iterator it = m_Systems->begin(); it != m_Systems->end(); ++it)
			it->ClearShips();
}
void CShipMap::EraseAt(CShipMap::iterator& index) {
	//if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
	//	CString s;
	//	s.Format("CShipMap: removing ship %s", index->second->GetName());
	//	MYTRACE("ships")(MT::LEVEL_INFO, s);
	//}
	AssertBotE(!empty() && index != end());
	//need to copy the iterator, because it can come from a reference to m_CurrentShip (or m_FleetShip),
	//meaning updating m_CurrentShip/m_FleetShip would change it as well
	const CShipMap::iterator to_erase = index;
	UpdateSpecialShip(m_CurrentShip, to_erase);
	UpdateSpecialShip(m_FleetShip, to_erase);
	//now only prevent the passed iterator from becoming invalid via incrementing,
	//if the above two calls didn't already update it
	if(to_erase == index)
		++index;
	AssertBotE(to_erase->second);
	if(m_Systems)
	{
		const CPoint& co = to_erase->second->GetCo();
		m_Systems->at(CoordsToIndex(co.x, co.y)).EraseShip(to_erase->second);
	}
	m_Ships.erase(to_erase);
	if(empty())
		Reset();
}

//////////////////////////////////////////////////////////////////////
// getting elements
//////////////////////////////////////////////////////////////////////

boost::shared_ptr<const CShips> CShipMap::at(unsigned key) const
{
	CShipMap::const_iterator i = find(key);
	AssertBotE(i != end());
	return i->second;
}

boost::shared_ptr<CShips> CShipMap::at(unsigned key)
{
	CShipMap::iterator i = find(key);
	AssertBotE(i != end());
	return i->second;
}

//////////////////////////////////////////////////////////////////////
// getting info
//////////////////////////////////////////////////////////////////////

int CShipMap::GetUpperBound() const {
	return GetSize() - 1;
}
int CShipMap::GetSize() const {
	return m_Ships.size();
}
bool CShipMap::empty() const {
	return m_Ships.empty();
}

//////////////////////////////////////////////////////////////////////
// private functions
//////////////////////////////////////////////////////////////////////

unsigned CShipMap::NextKey() {
	return m_NextKey++;
}

int CShipMap::index_of(const CShipMap::const_iterator& position) const {
	return std::distance(begin(), position);
}

const CShips& CShipMap::GetAt(int index) const {
	AssertBotE(index < GetSize());
	return *iterator_at(index)->second;
}

const boost::shared_ptr<const CShips> CShipMap::front() const
{
	const CShipMap::const_iterator it = iterator_at(0);
	AssertBotE(it != end());
	return iterator_at(0)->second;
}

//////////////////////////////////////////////////////////////////////
// Serialisierungsfunktionen
//////////////////////////////////////////////////////////////////////

void CShipMap::Serialize(CArchive& ar) {
	if(ar.IsStoring()) {
		ar << GetSize();
		for(CShipMap::iterator i = begin(); i != end(); ++i)
			i->second->Serialize(ar);
	}
	else if(ar.IsLoading()) {
		int size;
		ar >> size;
		//We need to call clear(), since resize() calls destructors only for elements from after the new
		//lenght (if new size is smaller), and apparently the >> operator allocates stuff with new
		//with the CString members of the ship class in their Serialize functions above.
		//Thus, call the destructors of all the current elements explicitely and manually.
		//(This comment was appropriate for std::vector)
		Reset();
		for(int i = 0; i < size; ++i)
		{
			const boost::shared_ptr<CShips> ship(new CShips());
			ship->Serialize(ar);
			Add(ship);
		}
	}
	else
		AssertBotE(false);
}

void CShipMap::SerializeEndOfRoundData(CArchive& ar, const CString& sMajorID) {
	if(ar.IsStoring()) {
		std::vector<boost::shared_ptr<CShips>> ships;
		for(CShipMap::const_iterator i = begin(); i != end(); ++i)
			if (i->second->OwnerID() == sMajorID)
				ships.push_back(i->second);
		ar << ships.size();
		for(std::vector<boost::shared_ptr<CShips>>::iterator i = ships.begin(); i != ships.end(); ++i)
			(*i)->Serialize(ar);
	}
	else if(ar.IsLoading()) {
		int count = 0;
		ar >> count;
		for(CShipMap::iterator i = begin(); i != end();) {
			if (i->second->OwnerID() == sMajorID) {
				EraseAt(i);
				continue;
			}
			++i;
		}
		const int newsize = GetSize() + count;
		for (int i = GetSize(); i < newsize; ++i) {
			const boost::shared_ptr<CShips> ship(new CShips());
			ship->Serialize(ar);
			const CShipMap::iterator j = Add(ship);
			AssertBotE(j->second->OwnerID() == sMajorID);
		}
	}
	else
		AssertBotE(false);
}

void CShipMap::SerializeNextRoundData(CArchive& ar, const CPoint& ptCurrentCombatSector) {
	if(ar.IsStoring()) {
		int nCount = 0;
		for(CShipMap::const_iterator i = begin(); i != end(); ++i)
			if (i->second->GetCo() == ptCurrentCombatSector)
				nCount++;
		ar << nCount;
		// nur Schiffe aus diesem Sektor senden
		for(CShipMap::iterator i = begin(); i != end(); ++i)
			if (i->second->GetCo() == ptCurrentCombatSector)
				i->second->Serialize(ar);
	}
	else if(ar.IsLoading()) {
		// Es werden nur Schiffe aus dem aktuellen Kampfsektor empfangen
		int count = 0;
		ar >> count;
		// alle Schiffe aus dem Kampfsektor entfernen
		for(CShipMap::iterator i = begin(); i != end();) {
			if (i->second->GetCo() == ptCurrentCombatSector) {
				EraseAt(i);
				continue;
			}
			++i;
		}
		// empfangene Schiffe wieder hinzuf�gen
		const int newsize = GetSize() + count;
		for (int i = GetSize(); i < newsize; ++i) {
			const boost::shared_ptr<CShips> ship(new CShips());
			ship->Serialize(ar);
			const CShipMap::iterator j = Add(ship);
			AssertBotE(j->second->GetCo() == ptCurrentCombatSector);
		}
	}
	else
		AssertBotE(false);
}

//////////////////////////////////////////////////////////////////////
// special ships
//////////////////////////////////////////////////////////////////////
//current ship

void CShipMap::SetCurrentShip(unsigned key) {
	const CShipMap::iterator i = find(key);
	AssertBotE(i != end());
	m_CurrentShip = i;
}

void CShipMap::SetCurrentShip(const CShipMap::iterator& position) {
	AssertBotE(!empty() && position != end());
	m_CurrentShip = position;
}

const CShipMap::iterator& CShipMap::CurrentShip() const {
	AssertBotE(!empty() && m_CurrentShip != end());
	return m_CurrentShip;
}

//fleet ship

void CShipMap::SetFleetShip(unsigned key) {
	const CShipMap::iterator i = find(key);
	AssertBotE(i != end());
	m_FleetShip = i;
}

void CShipMap::SetFleetShip(const CShipMap::iterator& position) {
	AssertBotE(!empty() && position != end());
	m_FleetShip = position;
}

const CShipMap::iterator& CShipMap::FleetShip() const {
	AssertBotE(!empty() && m_FleetShip != end());
	return m_FleetShip;
}

//get leader
CShips* CShipMap::GetLeader(const CShips* pShip) const
{
	for (CShipMap::const_iterator i = begin(); i != end(); ++i)
	{
		// Schiff ist selbst F�hrer einer Flotte oder hat keine Flotte
		if (i->second.get() == pShip)
			return i->second.get();

		// Schiff ist in einer Flotte (nur pr�fen wenn der Flottenf�hrer auch von der gleichen Rasse ist)
		if (i->second->OwnerID() != pShip->OwnerID())
			continue;

		for (CShips::const_iterator j = i->second->begin(); j != i->second->end(); ++j)
		{
			if (j->second.get() == pShip)
				return i->second.get();
		}
	}

	AssertBotE(false);
	return NULL;
}

//updating

void CShipMap::UpdateSpecialShip(CShipMap::iterator& ship, const CShipMap::const_iterator& to_erase) {
	if(ship != to_erase)
		return;
	if(ship != begin())
		--ship;
	else if (ship != end())
		++ship;
}

//////////////////////////////////////////////////////////////////////
// debugging helper
//////////////////////////////////////////////////////////////////////

//CString CShipMap::ToString() const {
//	CString s("SHIPARRAY BEGINN\n");
//	unsigned index = 0;
//	for(CShipMap::const_iterator i = begin(); i != end(); ++i)
//		s.Format("%s%u: %s at (%u,%u)\n", s, index, i->GetName(), i->GetCo().x, i->GetCo().y);
//	s.Format("%s%s", s, "SHIPARRAY END");
//	return s;
//}
