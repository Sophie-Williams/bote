// Ships.cpp: Implementierung der Klasse CShips.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ships.h"

#include <cassert>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//IMPLEMENT_SERIAL (CShips, CObject, 1)

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CShips::CShips() :
	m_Leader(),
	m_Fleet(),
	m_Key(0),
	m_bLeaderIsCurrent(true)
{
}

CShips::CShips(const CShip& ship) :
	m_Leader(ship),
	m_Fleet(),
	m_Key(0),
	m_bLeaderIsCurrent(true)
{
}

//////////////////////////////////////////////////////////////////////
// Kopierkonstruktor
//////////////////////////////////////////////////////////////////////

CShips::CShips(const CShips& o) :
	m_Leader(o.m_Leader),
	m_Fleet(o.m_Fleet),
	m_Key(o.m_Key),
	m_bLeaderIsCurrent(o.m_bLeaderIsCurrent)
{
}

//////////////////////////////////////////////////////////////////////
// Zuweisungsoperator
//////////////////////////////////////////////////////////////////////

CShips& CShips::operator=(const CShips& o)
{
	if(this == &o)
		return *this;
	m_Leader = o.m_Leader;
	m_Fleet = o.m_Fleet;
	m_Key = o.m_Key;
	m_bLeaderIsCurrent = o.m_bLeaderIsCurrent;
	return *this;
}

CShips::~CShips()
{
	Reset();
}

///////////////////////////////////////////////////////////////////////
// Speichern / Laden
///////////////////////////////////////////////////////////////////////
void CShips::Serialize(CArchive &ar)
{
	/*CObject::Serialize(ar);*/

	m_Leader.Serialize(ar);
	m_Fleet.Serialize(ar);
}

//////////////////////////////////////////////////////////////////////
// iterators
//////////////////////////////////////////////////////////////////////
CShips::const_iterator CShips::begin() const {
	return m_Fleet.begin();
}
CShips::const_iterator CShips::end() const {
	return m_Fleet.end();
}
CShips::iterator CShips::begin() {
	return m_Fleet.begin();
}
CShips::iterator CShips::end() {
	return m_Fleet.end();
}

CShips::const_iterator CShips::find(unsigned key) const {
	return m_Fleet.find(key);
}
CShips::iterator CShips::find(unsigned key) {
	return m_Fleet.find(key);
}

CShips::const_iterator CShips::iterator_at(int index) const{
	return m_Fleet.iterator_at(index);
}
CShips::iterator CShips::iterator_at(int index) {
	return m_Fleet.iterator_at(index);
}

//////////////////////////////////////////////////////////////////////
// getting
//////////////////////////////////////////////////////////////////////

const CShips::const_iterator& CShips::CurrentShip() const {
	assert(!m_bLeaderIsCurrent && HasFleet());
	return m_Fleet.CurrentShip();
}

const CShips& CShips::at(unsigned key) const {
	return m_Fleet.at(key);
}
CShips& CShips::at(unsigned key) {
	return m_Fleet.at(key);
}

//////////////////////////////////////////////////////////////////////
// setting
//////////////////////////////////////////////////////////////////////

//removes the element pointed to by the passed iterator from this fleet
//@param index: will be updated and point to the new position of the element which followed the erased one
void CShips::RemoveShipFromFleet(CShips::iterator& ship)
{
	//if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
	//	CString s;
	//	s.Format("CShips: removing ship %s from fleet of %s", ship->m_Leader.GetShipName(),
	//		m_Leader.GetShipName());
	//	MYTRACE("ships")(MT::LEVEL_INFO, s);
	//}
	m_Fleet.EraseAt(ship);
	if(!HasFleet())
		Reset();
}

void CShips::Reset(void) {
	m_Fleet.Reset();
	m_bLeaderIsCurrent = true;
}

// Funktion �bernimmt die Befehle des hier als Zeiger �bergebenen Schiffsobjektes an alle Mitglieder der Flotte
void CShips::AdoptCurrentOrders(const CShip* ship)
{
	for(CShips::iterator i = begin(); i != end(); ++i) {
		i->second.m_Leader.AdoptOrdersFrom(*ship);
	}
}

void CShips::AddShipToFleet(CShips& fleet) {
	CString s;
	if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
		s.Format("CShips: adding ship with leader %s to fleet of %s", fleet.m_Leader.GetShipName(),
			m_Leader.GetShipName());
		MYTRACE("ships")(MT::LEVEL_INFO, s);
	}
	CShip leader = fleet.m_Leader;
	leader.AdoptOrdersFrom(m_Leader);
	m_Fleet.Add(fleet.m_Leader);
	if(fleet.HasFleet()) {
		if(MT::CMyTrace::IsLoggingEnabledFor("ships")) {
			s.Format("CShips: adding the fleet of leader %s to fleet of %s", fleet.m_Leader.GetShipName(),
				m_Leader.GetShipName());
			MYTRACE("ships")(MT::LEVEL_INFO, s);
		}
		m_Fleet.Append(fleet.m_Fleet);
		PropagateOrdersToFleet();
		fleet.Reset();
	}
}

void CShips::SetCurrentShip(const CShips::iterator& position)
{
	if(position == end())
	{
		m_bLeaderIsCurrent = true;
		return;
	}
	m_bLeaderIsCurrent = false;
	m_Fleet.SetCurrentShip(position);
}

void CShips::PropagateOrdersToFleet()
{
	AdoptCurrentOrders(&m_Leader);
}

void CShips::ApplyTraining(int XP) {
	const bool veteran = HasVeteran();
	m_Leader.ApplyTraining(XP, veteran);
	// Wenn das Schiff eine Flotte anf�hrt, Schiffstraining auf alle Schiffe in der Flotte anwenden
	if(!HasFleet())
		return;
	for(CShips::iterator i = begin(); i != end(); ++i)
		i->second.m_Leader.ApplyTraining(XP, veteran);
}

bool CShips::UnassignFlagship() {
	// �berpr�fen ob ein Flagschiff in einer Flotte ist
	for(CShips::iterator i = begin(); i != end(); ++i)
	{
		if(i->second.UnassignFlagship())
			return true;;
	}
	if (m_Leader.GetIsShipFlagShip())
	{
		m_Leader.SetIsShipFlagShip(FALSE);
		return true;
	}
	return false;
}

void CShips::SetCloak(bool apply_to_fleet) {
	m_Leader.SetCloak();
	if(apply_to_fleet)
		for(CShips::iterator i = begin(); i != end(); ++i)
			i->second.SetCloak();
}

void CShips::UnsetCurrentOrder(bool apply_to_fleet) {
	m_Leader.UnsetCurrentOrder();
	if(apply_to_fleet)
		for(CShips::iterator i = begin(); i != end(); ++i)
			i->second.UnsetCurrentOrder();
}

//////////////////////////////////////////////////////////////////////
// calculated stements about this fleet (should be const functions, non-bool returning)
//////////////////////////////////////////////////////////////////////

// Funktion berechnet die Geschwindigkeit der Flotte. Der Parameter der hier �bergeben werden sollte
// ist der this-Zeiger des Schiffsobjektes, welches die Flotte besitzt
unsigned CShips::GetFleetSpeed(const CShip* ship) const
{
	BYTE speed = 127;
	if (ship != NULL)
		speed = ship->GetSpeed();
	for(CShips::const_iterator i = begin(); i != end(); ++i)
		if(i->second.GetSpeed() < speed)
			speed = i->second.GetSpeed();
	if (speed == 127)
		speed = 0;
	return speed;
}

// Funktion berechnet die Reichweite der Flotte. Der Parameter der hier �bergeben werden sollte
// ist der this-Zeiger des Schiffsobjektes, welches die Flotte besitzt
SHIP_RANGE::Typ CShips::GetFleetRange(const CShip* pShip) const
{
	SHIP_RANGE::Typ nRange = SHIP_RANGE::LONG;
	if (pShip)
		nRange = min(pShip->GetRange(), nRange);

	for(CShips::const_iterator i = begin(); i != end(); ++i)
		nRange = min(i->second.GetRange(), nRange);

	return nRange;
}

// Funktion berechnet den Schiffstyp der Flotte. Wenn hier nur der selbe Schiffstyp in der Flotte vorkommt,
// dann gibt die Funktion diesen Schiffstyp zur�ck. Wenn verschiedene Schiffstypen in der Flotte vorkommen,
// dann liefert und die Funktion ein -1. Der Parameter der hier �bergeben werden sollte ist der this-Zeiger
// des Schiffsobjektes, welches die Flotte besitzt
short CShips::GetFleetShipType() const
{
	short type = m_Leader.GetShipType();
	for(CShips::const_iterator i = begin(); i != end(); ++i)
		if (i->second.GetShipType() != type)
			return -1;
	return type;
}

// Funktion berechnet die minimale Stealthpower der Flotte. Der Parameter der hier �bergeben werden sollte
// ist der this-Zeiger bzw. die Adresse des Schiffsobjektes, welches die Flotte besitzt
BYTE CShips::GetFleetStealthPower(const CShip* ship) const
{
	BYTE stealthPower = MAXBYTE;
	if (ship != NULL)
	{
		stealthPower = ship->GetStealthPower() * 20;
		if (ship->GetStealthPower() > 3 && ship->GetCloak() == false)
			stealthPower = 3 * 20;
	}
	if (stealthPower == 0)
		return 0;

	for(CShips::const_iterator i = begin(); i != end(); ++i)
	{
		if (stealthPower == 0)
			return 0;

		BYTE fleetStealthPower = i->second.GetStealthPower() * 20;
		if (i->second.GetStealthPower() > 3  && !i->second.GetCloak())
			fleetStealthPower = 3 * 20;
		if (fleetStealthPower < stealthPower)
			stealthPower = fleetStealthPower;
	}

	return stealthPower;
}

//////////////////////////////////////////////////////////////////////
// bool statements about this fleet or the ship leading it, should be const
//////////////////////////////////////////////////////////////////////

//// Diese Funktion liefert true wenn die Flotte den "order" ausf�hren kann.
//// Kann die Flotte den Befehl nicht befolgen liefert die Funktion false zur�ck
bool CShips::CanHaveOrder(SHIP_ORDER::Typ order) const {
	if(HasFleet())
	{
		if (order == SHIP_ORDER::ASSIGN_FLAGSHIP)
			return false;
		for(CShips::const_iterator i = m_Fleet.begin(); i != m_Fleet.end(); ++i)
			if(!i->second.CanHaveOrder(order))
				return false;
	}
	return m_Leader.CanHaveOrder(order);
}

bool CShips::AllOnTactic(COMBAT_TACTIC::Typ tactic) const {
	for(CShips::const_iterator i = begin(); i != end(); ++i)
	{
		if(i->second.GetCombatTactic() != tactic)
			return false;
	}
	return true;
}

bool CShips::HasFleet() const {
	return !m_Fleet.empty();
}

bool CShips::NeedsRepair() const {
	for(CShips::const_iterator i = begin(); i != end(); ++i) {
		if(i->second.NeedsRepair())
			return true;
	}
	return m_Leader.NeedsRepair();
}

bool CShips::FleetHasTroops() const {
	for(CShips::const_iterator j = begin(); j != end(); ++j)
		if(j->second.FleetHasTroops())
			return true;
	return m_Leader.HasTroops();
}

bool CShips::HasVeteran() const {
	for(CShips::const_iterator j = begin(); j != end(); ++j)
		if(j->second.HasVeteran())
			return true;
	return m_Leader.IsVeteran();
}

//////////////////////////////////////////////////////////////////////
// other functions
//////////////////////////////////////////////////////////////////////

CShips CShips::GiveFleetToFleetsFirstShip() {
	assert(HasFleet());
	// erstes Schiff aus der Flotte holen
	CShips::iterator i = begin();
	CShips new_fleet_ship = i->second;

	while(true)
	{
		++i;
		if(i == end())
			break;
		new_fleet_ship.AddShipToFleet(i->second);
	}
	Reset();
	return new_fleet_ship;
}

CString CShips::GetTooltip(bool bShowFleet)
{
	if(bShowFleet && HasFleet())
		return m_Leader.GetTooltip(&CShip::FleetInfoForGetTooltip(
			GetFleetShipType(), GetFleetRange(&m_Leader), GetFleetSpeed(&m_Leader))
		);
	return m_Leader.GetTooltip();
}

void CShips::DrawShip(Gdiplus::Graphics* g, CGraphicPool* pGraphicPool, const CPoint& pt, bool bIsMarked,
	bool bOwnerUnknown, bool bDrawFleet, const Gdiplus::Color& clrNormal,
	const Gdiplus::Color& clrMark, const Gdiplus::Font& font) const {

	const bool draw_troop_symbol = bDrawFleet ? FleetHasTroops() : m_Leader.HasTroops();
	m_Leader.DrawShip(g, pGraphicPool, pt, bIsMarked, bOwnerUnknown, bDrawFleet && HasFleet(),
		clrNormal,clrMark, font, draw_troop_symbol, GetFleetShipType(), GetFleetSize());
}

void CShips::Repair(BOOL bAtShipPort, bool bFasterShieldRecharge) {
	for(CShips::iterator i = begin(); i != end(); ++i)
		i->second.Repair(bAtShipPort, bFasterShieldRecharge);
	m_Leader.Repair(bAtShipPort, bFasterShieldRecharge);
	if(m_Leader.GetCurrentOrder() == SHIP_ORDER::REPAIR && (!NeedsRepair() || !bAtShipPort))
		m_Leader.UnsetCurrentOrder();
}

void CShips::RetreatFleet(const CPoint& RetreatSector) {
	for(CShips::iterator j = begin(); j != end(); ++j)
		j->second.Retreat(RetreatSector);
}

void CShips::CalcEffects(CSector& sector, CRace* pRace,
			bool bDeactivatedShipScanner, bool bBetterScanner) {

		m_Leader.CalcEffectsForSingleShip(sector, pRace, bDeactivatedShipScanner, bBetterScanner, false);
		// wenn das Schiff eine Flotte besitzt, dann die Schiffe in der Flotte auch beachten
		for(CShips::iterator j = begin(); j != end(); ++j)
			j->second.m_Leader.CalcEffectsForSingleShip(sector, pRace, bDeactivatedShipScanner, bBetterScanner, true);
}

CString CShips::SanityCheckUniqueness(std::set<CString>& already_encountered) const {
	for(CShips::const_iterator i = begin(); i != end(); ++i) {
		const CString& duplicate = i->second.SanityCheckUniqueness(already_encountered);
		if(!duplicate.IsEmpty())
			return duplicate;
	}
	return m_Leader.SanityCheckUniqueness(already_encountered);
}
