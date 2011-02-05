// CombatSimulatorView.h : Schnittstelle der Klasse CCombatSimulatorView
//


#pragma once


class CCombatSimulatorView : public CView
{
protected: // Nur aus Serialisierung erstellen
	CCombatSimulatorView();
	DECLARE_DYNCREATE(CCombatSimulatorView)

// Attribute
	CRect r;
	UINT counter;
	BOOLEAN xy_ebene;
	std::map<CString, COLORREF> color;
	std::map<CString, BYTE> winner;

public:
	CCombatSimulatorDoc* GetDocument() const;

// Vorg�nge
public:

// �berschreibungen
public:
	virtual void OnDraw(CDC* pDC);  // �berschrieben, um diese Ansicht darzustellen
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementierung
public:
	virtual ~CCombatSimulatorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generierte Funktionen f�r die Meldungstabellen
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
protected:
//	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // Debugversion in CombatSimulatorView.cpp
inline CCombatSimulatorDoc* CCombatSimulatorView::GetDocument() const
   { return reinterpret_cast<CCombatSimulatorDoc*>(m_pDocument); }
#endif

