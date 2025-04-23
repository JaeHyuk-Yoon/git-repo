// CDlgImage.cpp: 구현 파일
//

#include "pch.h"
#include "gPrj.h"
#include "afxdialogex.h"
#include "CDlgImage.h"
#include <iostream>
#include "gPrjDlg.h"


// CDlgImage 대화 상자
using namespace std;

IMPLEMENT_DYNAMIC(CDlgImage, CDialogEx)

CDlgImage::CDlgImage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CDlgImage, pParent)
{
	m_pParent = pParent;
	m_nClickNum = 0;
}

CDlgImage::~CDlgImage()
{
}

void CDlgImage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgImage, CDialogEx)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CDlgImage 메시지 처리기


BOOL CDlgImage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//MoveWindow(0, 0, 640, 480);
	//MoveWindow(0, 0, 1280, 800);
	MoveWindow(0, 0, NWIDTH, NHEIGHT);
	initImage();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgImage::initImage()
{
	m_image.Create(NWIDTH, -NHEIGHT, NBPP);
	if (NBPP == 8) {
		static RGBQUAD rgb[256];
		for (int i = 0; i < 256; i++)
			rgb[i].rgbRed = rgb[i].rgbGreen = rgb[i].rgbBlue = i;
		m_image.SetColorTable(0, 256, rgb);
	}
	int nPitch = m_image.GetPitch();
	unsigned char* fm = (unsigned char*)m_image.GetBits();

	memset(fm, 0xff, NWIDTH * NHEIGHT);
}


void CDlgImage::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.
	if (m_image) {
		m_image.Draw(dc, 0, 0);
	}
}

BOOL CDlgImage::validImgPos(int x, int y)
{
	int nWidth = m_image.GetWidth();
	int nHeight = m_image.GetHeight();
	CRect rect(0, 0, nWidth, nHeight);

	return rect.PtInRect(CPoint(x, y));
}

void CDlgImage::drawCircle(unsigned char* fm, int x, int y, int nRadius, int nGray)
{
	int nCenterX = x;
	int nCenterY = y;
	int nPitch = m_image.GetPitch();

	for (int j = nCenterY-nRadius; j < nCenterY + nRadius * 2; j++) {
		for (int i = nCenterX -nRadius; i < nCenterX + nRadius * 2; i++) {
			if (isInCircle(i, j, nCenterX, nCenterY, nRadius)) {
				if (validImgPos(i, j))
					fm[j * nPitch + i] = nGray;
			}
		}
	}
}

bool CDlgImage::isInCircle(int i, int j, int nCenterX, int nCenterY, int nRadius)
{
	bool bRet = false;

	double dX = i - nCenterX;
	double dY = j - nCenterY;
	double dDist = dX * dX + dY * dY;

	if (dDist < nRadius * nRadius) {
		bRet = true;
	}

	return bRet;
}

void CDlgImage::updateDisplay()
{
	CClientDC dc(this);
	m_image.Draw(dc, 0, 0);
}

void CDlgImage::updateDisplay(CPoint* p_ClickPointList, int nClickNum, int nCircleRadius)
{
	CString strCenter;
	int x;
	int y;
	int nTextNum;

	CClientDC dc(this);
	m_image.Draw(dc, 0, 0);

	if (nClickNum > 3) {
		nTextNum = 3;
	}
	else {
		nTextNum = nClickNum;
	}

	for (int i = 0; i < nTextNum; i++) {
		x = p_ClickPointList[i].x;
		y = p_ClickPointList[i].y;
		strCenter.Format(_T("(%d, %d)"), x, y);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(0, 150, 50));
		dc.TextOutW(x + nCircleRadius, y + nCircleRadius, strCenter);
	}
}

void CDlgImage::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_nClickNum++;

	m_nBtnClickFlag = true;

	//CgPrjDlg* pDlg = (CgPrjDlg*)m_pParent;
	CgPrjDlg* pDlg = (CgPrjDlg*)GetParent();
	int nRadius = pDlg->m_nRadiusNum;
	int thickness = pDlg->m_nCircleThick;

	unsigned char* fm = (unsigned char*)m_image.GetBits();
	
	if (m_nClickNum < 3) {
		cout << "--------------------------" << endl;
		//cout << "Click Count : " << m_nClickNum << endl;

		m_pClickPoint[m_nClickNum-1] = point;

		drawCircle(fm, point.x, point.y, nRadius, nBlack);

		updateDisplay(m_pClickPoint, m_nClickNum, nRadius);
	}
	else if (m_nClickNum == 3) {
		m_pClickPoint[m_nClickNum - 1] = point;

		drawBigCircle(fm, m_pClickPoint, thickness);

		drawThreeCircle(fm, m_pClickPoint, m_nClickNum, nRadius, nBlack);

		updateDisplay(m_pClickPoint, m_nClickNum, nRadius);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CDlgImage::drawBigCircle(unsigned char* fm, CPoint* p_ClickPointList, int thickness)
{
	int nBlack = 0;
	int nWhite = 0xff;
	CPoint pBigCircleCenter;

	double* bigCircleInfo = calcBigCircleInfo(p_ClickPointList);
	if (bigCircleInfo[0] >= 0) {
		pBigCircleCenter.x = round(bigCircleInfo[0]);
		pBigCircleCenter.y = round(bigCircleInfo[1]);
		int nRadius = round(bigCircleInfo[2]);

		int dHalfThickness = thickness / 2;

		if (dHalfThickness == 0) dHalfThickness = 1;

		int nPitch = m_image.GetPitch();

		int nTopY = pBigCircleCenter.y - nRadius - dHalfThickness;
		int nBottomY = pBigCircleCenter.y + nRadius + dHalfThickness;
		int nLeftX = pBigCircleCenter.x - nRadius - dHalfThickness;
		int nRightX = pBigCircleCenter.x + nRadius + dHalfThickness;

		if (nTopY < 0) nTopY = 0;
		if (nBottomY > NHEIGHT) nBottomY = NHEIGHT;
		if (nLeftX < 0) nLeftX = 0;
		if (nRightX < NWIDTH) nRightX = NWIDTH;

		for (int j = nTopY; j < nBottomY; j++) {
			for (int i = nLeftX; i < nRightX; i++) {
				if (isInCircle(i, j, pBigCircleCenter.x, pBigCircleCenter.y, nRadius + dHalfThickness)) {
					if (validImgPos(i, j))
						fm[j * nPitch + i] = nBlack;
				}
			}
		}
		for (int j = nTopY; j < nBottomY; j++) {
			for (int i = nLeftX; i < nRightX; i++) {
				if (isInCircle(i, j, pBigCircleCenter.x, pBigCircleCenter.y, nRadius - dHalfThickness)) {
					if (validImgPos(i, j))
						fm[j * nPitch + i] = nWhite;
				}
			}
		}
	}
}

void CDlgImage::drawThreeCircle(unsigned char* fm, CPoint* p_ClickPointList, int nClickNum, int nRadius, int nBlack)
{
	int nPitch = m_image.GetPitch();
	int nCenterX;
	int nCenterY;

	for (int k = 0; k < nClickNum; k++) {
		nCenterX = p_ClickPointList[k].x;
		nCenterY = p_ClickPointList[k].y;

		for (int j = nCenterY - nRadius; j < nCenterY + nRadius * 2; j++) {
			for (int i = nCenterX - nRadius; i < nCenterX + nRadius * 2; i++) {
				if (isInCircle(i, j, nCenterX, nCenterY, nRadius)) {
					if (validImgPos(i, j))
						fm[j * nPitch + i] = nBlack;
				}
			}
		}
	}	
}

double* CDlgImage::calcBigCircleInfo(CPoint* p_ClickPointList)
{
	double static circleInfo[3];

	double dCenterX = -1;
	double dCenterY = 0;
	double dRadius = 0;
	
	CPoint interCenPt[2];

	CPoint interCenFisrtPt;
	CPoint interCenSecendPt;

	interCenFisrtPt.x = 0.5 * (p_ClickPointList[0].x + p_ClickPointList[1].x);
	interCenFisrtPt.y = 0.5 * (p_ClickPointList[0].y + p_ClickPointList[1].y);

	interCenSecendPt.x = 0.5 * (p_ClickPointList[0].x + p_ClickPointList[2].x);
	interCenSecendPt.y = 0.5 * (p_ClickPointList[0].y + p_ClickPointList[2].y);

	interCenPt[0] = interCenFisrtPt;
	interCenPt[1] = interCenSecendPt;

	double a1 = -1.0 * (p_ClickPointList[0].x - p_ClickPointList[1].x) / (p_ClickPointList[0].y - p_ClickPointList[1].y);
	double b1 = interCenPt[0].y - a1 * interCenPt[0].x;

	double a2 = -1.0 * (p_ClickPointList[0].x - p_ClickPointList[2].x) / (p_ClickPointList[0].y - p_ClickPointList[2].y);
	double b2 = interCenPt[1].y - a2 * interCenPt[1].x;


	if (a1 != a2) {
		dCenterX = (b2 - b1) / (a1 - a2);
		dCenterY = a1 * dCenterX + b1;
		dRadius = sqrt(pow(dCenterX - p_ClickPointList[0].x, 2) + pow(dCenterY - p_ClickPointList[0].y, 2));
	}
	//else {
		//AfxMessageBox(_T("Can not make Circle (is not three points)"));
	//}
	
	circleInfo[0] = dCenterX;
	circleInfo[1] = dCenterY;
	circleInfo[2] = dRadius;
	
	return circleInfo;
}

void CDlgImage::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_nBtnClickFlag = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CDlgImage::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CgPrjDlg* pDlg = (CgPrjDlg*)GetParent();
	int nRadius = pDlg->m_nRadiusNum;
	int thickness = pDlg->m_nCircleThick;
	unsigned char* fm = (unsigned char*)m_image.GetBits();

	CPoint tmpPt;

	int nCursorX = point.x;
	int nCursorY = point.y;

	bool bCheckInCircle = false;

	cout << "cursor : " << nCursorX << "," << nCursorY << endl;
	
	if (m_nClickNum > 3) {
		//cout << "m_nClickNum : " << m_nClickNum << endl;
		if (m_nBtnClickFlag) {
			//cout << "In m_nBtnClickFlag !!! : " << m_nBtnClickFlag << endl;
			if (!bKeepClick) {
				for (int i = 0; i < 3; i++) {
					if (isInCircle(nCursorX, nCursorY, m_pClickPoint[i].x, m_pClickPoint[i].y, nRadius)) {
						bKeepClick = true;
						bCheckInCircle = true;
						m_nUpdateIndex = i;
						tmpPt.x = nCursorX;
						tmpPt.y = nCursorY;
						m_pClickPoint[m_nUpdateIndex] = tmpPt;
						//cout << "m_pClickPoint[nUpdateIndex] : " << m_pClickPoint[m_nUpdateIndex].x << "," << m_pClickPoint[m_nUpdateIndex].y << endl;
					}
				}
			}
			else {
				tmpPt.x = nCursorX;
				tmpPt.y = nCursorY;
				m_pClickPoint[m_nUpdateIndex] = tmpPt;
				bCheckInCircle = true;
			}

			if (bCheckInCircle) {
				//cout << "m_pClickPoint[nUpdateIndex] : " << m_pClickPoint[m_nUpdateIndex].x << "," << m_pClickPoint[m_nUpdateIndex].y << endl;

				memset(fm, 0xff, NWIDTH * NHEIGHT);

				drawBigCircle(fm, m_pClickPoint, thickness);

				drawThreeCircle(fm, m_pClickPoint, m_nClickNum, nRadius, nBlack);

				updateDisplay(m_pClickPoint, m_nClickNum, nRadius);
			}
		}
		else {
			bKeepClick = false;
		}
	}
	CDialogEx::OnMouseMove(nFlags, point);
}
