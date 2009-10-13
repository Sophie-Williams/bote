// MainBaseView.cpp : implementation file
//

#include "stdafx.h"
#include "botf2.h"
#include "MainBaseView.h"
#include "Races\RaceController.h"

// CMainBaseView
CMajor* CMainBaseView::m_pPlayersRace = NULL;

IMPLEMENT_DYNCREATE(CMainBaseView, CView)

CMainBaseView::CMainBaseView()
{
}

CMainBaseView::~CMainBaseView()
{	
}

BEGIN_MESSAGE_MAP(CMainBaseView, CView)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CMainBaseView drawing

void CMainBaseView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here	
}


// CMainBaseView diagnostics

#ifdef _DEBUG
void CMainBaseView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CMainBaseView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CMainBaseView message handlers

void CMainBaseView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CBotf2Doc* pDoc = (CBotf2Doc*)GetDocument();
	ASSERT(pDoc);

	CString sID = pDoc->GetPlayersRaceID();
	m_pPlayersRace = dynamic_cast<CMajor*>(pDoc->GetRaceCtrl()->GetRace(sID));
	ASSERT(m_pPlayersRace);

	// TODO: Add your specialized code here and/or call the base class
	m_TotalSize = CSize(1075, 750);	
}

void CMainBaseView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class
/*	CBotf2Doc* pDoc = (CBotf2Doc*)GetDocument();

	CRect client;
	GetClientRect(&client);
		
	// MapMode setzen
	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetWindowExt(m_TotalSize);		
	pDC->SetViewportExt(client.right, client.bottom);
		
	pDC->SetViewportOrg(0, 0);
	pDC->SetBkColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);*/	
		
	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CMainBaseView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	//return CView::OnEraseBkgnd(pDC);
}

void CMainBaseView::DrawGDIButtons(Graphics* g, CArray<CMyButton*>* buttonArray, int counter, Gdiplus::Font &font, Gdiplus::SolidBrush &fontBrush)
{
	CBotf2Doc* pDoc = (CBotf2Doc*)GetDocument();
	ASSERT(pDoc);

	// Wenn wir im jeweiligen Men� sind, pr�fen, ob der dazugeh�rige Button auch inaktiv ist.
	for (int j = 0; j < buttonArray->GetSize(); j++)
		if (counter == j && buttonArray->GetAt(j)->GetState() != 2)
		{
			for (int i = 0; i < buttonArray->GetSize(); i++)
				if (buttonArray->GetAt(i)->GetState() == 2)
					buttonArray->GetAt(i)->SetState(0);
			buttonArray->GetAt(j)->SetState(2);
			break;
		}
	// Buttons zeichnen
	for (int i = 0; i < buttonArray->GetSize(); i++)
		buttonArray->GetAt(i)->DrawButton(*g, pDoc->GetGraphicPool(), font, fontBrush);
}

void CMainBaseView::ButtonReactOnMouseOver(const CPoint &point, CArray<CMyButton*>* buttonArray)
{
	CBotf2Doc* pDoc = (CBotf2Doc*)GetDocument();
	ASSERT(pDoc);

	for (int i = 0; i < buttonArray->GetSize(); i++)
	{
		if (buttonArray->GetAt(i)->ClickedOnButton(point))
		{			
			if (buttonArray->GetAt(i)->Activate())
			{
				CSoundManager::GetInstance()->PlaySound(SNDMGR_SOUND_MAINMENU, SNDMGR_PRIO_NORMAL, 0.5f);
				CRect r = buttonArray->GetAt(i)->GetRect();
				CalcDeviceRect(r);
				InvalidateRect(r, FALSE);
				break;
			}
		}
		else if (buttonArray->GetAt(i)->Deactivate())
		{
			CRect r = buttonArray->GetAt(i)->GetRect();
			CalcDeviceRect(r);
			InvalidateRect(r, FALSE);
		}
	}
}

BOOLEAN CMainBaseView::ButtonReactOnLeftClick(const CPoint &point, CArray<CMyButton*>* buttonArray, int &counter, BOOLEAN invalidate, BOOLEAN onlyActivate)
{
	short button = -1;
	for (int i = 0; i < buttonArray->GetSize(); i++)
		if (buttonArray->GetAt(i)->ClickedOnButton(point))
		{
			// Wenn der Button schon deaktiviert ist, dann kann er nicht angeklickt werden
			if (buttonArray->GetAt(i)->GetState() == 2)
				return FALSE;			
			button = i;
			if (!onlyActivate)
			{
				// Button auf inaktiv schalten, anderen inaktiven Button auf normal schalten
				for (int j = 0; j < buttonArray->GetSize(); j++)
					if (buttonArray->GetAt(j)->GetState() == 2)
					{
						buttonArray->GetAt(j)->SetState(0);
						break;
					}
				buttonArray->GetAt(i)->SetState(2);
			}
			else
			{
				// Button auf aktiv schalten, anderen aktiven Button auf normal schalten
				for (int j = 0; j < buttonArray->GetSize(); j++)
					if (buttonArray->GetAt(j)->GetState() == 1)
					{
						buttonArray->GetAt(j)->SetState(0);
						break;
					}
					buttonArray->GetAt(i)->SetState(1);
			}
			counter = button;
			if (invalidate)
				Invalidate(FALSE);
			return TRUE;
		}
	return FALSE;
}

void CMainBaseView::CalcLogicalPoint(CPoint &point)
{
	CRect client;
	GetClientRect(&client);
	
	point.x *= (float)m_TotalSize.cx / (float)client.Width();
	point.y *= (float)m_TotalSize.cy / (float)client.Height();	
}

void CMainBaseView::CalcDeviceRect(CRect &rect)
{
	CRect client;
	GetClientRect(&client);

	CPoint p1 = rect.TopLeft();
	p1.x *= (float)client.Width() / (float)m_TotalSize.cx;
	p1.y *= (float)client.Height() / (float)m_TotalSize.cy;
	
	CPoint p2 = rect.BottomRight();
	p2.x *= (float)client.Width() / (float)m_TotalSize.cx;
	p2.y *= (float)client.Height() / (float)m_TotalSize.cy;
	rect.SetRect(p1, p2);
}