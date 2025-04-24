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

	// 클릭 지점 원 중심 좌표 출력
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

	//부모 클래스에서 멤버 변수 값(반지름, 정원 두께) 가져오기
	CgPrjDlg* pDlg = (CgPrjDlg*)GetParent();
	int nRadius = pDlg->m_nRadiusNum;
	int thickness = pDlg->m_nCircleThick;

	unsigned char* fm = (unsigned char*)m_image.GetBits();
	

	if (m_nClickNum < 3) {

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
	int nPitch = m_image.GetPitch();
	int nBlack = 0;
	int nWhite = 0xff;
	CPoint pBigCircleCenter;

	// 세 점으로 원의 정보(중심 좌표, 반지름) 계산 후 반환 함수 호출
	double* bigCircleInfo = calcBigCircleInfo(p_ClickPointList);

	// 
	if (bigCircleInfo[3] == 0) {
		pBigCircleCenter.x = round(bigCircleInfo[0]);
		pBigCircleCenter.y = round(bigCircleInfo[1]);
		int nRadius = round(bigCircleInfo[2]);

		int dHalfThickness = thickness / 2;

		if (dHalfThickness == 0) dHalfThickness = 1;

		int nTopY = pBigCircleCenter.y - nRadius - dHalfThickness;
		int nBottomY = pBigCircleCenter.y + nRadius + dHalfThickness;
		int nLeftX = pBigCircleCenter.x - nRadius - dHalfThickness;
		int nRightX = pBigCircleCenter.x + nRadius + dHalfThickness;


		// 현재 다이얼로그 크기 (size = [(0,0),(NWIDTH,NHEIGHT)]) 외부의 좌표는 원 포함 여부 체크하지 않기 위해 초과한 rect 좌표에 대해 값 보정
		if (nTopY < 0) nTopY = 0;
		if (nBottomY > NHEIGHT) nBottomY = NHEIGHT;
		if (nLeftX < 0) nLeftX = 0;
		if (nRightX < NWIDTH) nRightX = NWIDTH;

		// 두께 설정된 원 그리기
		// 반지름 + 정원 두께/2 를 반지름으로 설정하여 외부 원 검은색 그리기
		for (int j = nTopY; j < nBottomY; j++) {
			for (int i = nLeftX; i < nRightX; i++) {
				if (isInCircle(i, j, pBigCircleCenter.x, pBigCircleCenter.y, nRadius + dHalfThickness)) {
					if (validImgPos(i, j))
						fm[j * nPitch + i] = nBlack;
				}
			}
		}
		// 반지름 - 정원 두께/2 를 반지름으로 설정하여 내부 흰색 원 그리기
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
	int nCircleNum;

	if (nClickNum < 3) nCircleNum = nClickNum;
	else nCircleNum = 3;

	// 클릭 지점 3개 위치 한번에 그리기
	for (int k = 0; k < nCircleNum; k++) {
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
	double static circleInfo[4];

	double dCenterX = 0;
	double dCenterY = 0;
	double dRadius = 0;
	// 세점이 일자로 이어진 경우 예외 처리 하기 위한 변수
	double dParallelCheck = 0;
	
	CPoint interCenPt[2];

	CPoint interCenFisrtPt;
	CPoint interCenSecendPt;

	// 첫 번째 클릭 지점과 두 번째 클릭 지점의 중간 좌표
	interCenFisrtPt.x = 0.5 * (p_ClickPointList[0].x + p_ClickPointList[1].x);
	interCenFisrtPt.y = 0.5 * (p_ClickPointList[0].y + p_ClickPointList[1].y);

	// 첫 번째 클릭 지점과 세 번째 클릭 지점의 중간 좌표
	interCenSecendPt.x = 0.5 * (p_ClickPointList[0].x + p_ClickPointList[2].x);
	interCenSecendPt.y = 0.5 * (p_ClickPointList[0].y + p_ClickPointList[2].y);

	interCenPt[0] = interCenFisrtPt;
	interCenPt[1] = interCenSecendPt;

	double b1=0;
	double b2=0;

	// 첫 번째 클릭 지점과 두 번째 클릭 지점을 잇는 선의 수직선 기울기
	double a1 = -1.0 * (p_ClickPointList[0].x - p_ClickPointList[1].x) / (p_ClickPointList[0].y - p_ClickPointList[1].y);

	// 첫 번째 클릭 지점과 두 번째 클릭 지점을 잇는 선의 수직선 기울기
	double a2 = -1.0 * (p_ClickPointList[0].x - p_ClickPointList[2].x) / (p_ClickPointList[0].y - p_ClickPointList[2].y);


	// 두 중간 좌표에서 수직이되는 a1과 a2가 모두 실수인경우
	if (!IsFiniteNumber(a1) && !IsFiniteNumber(a2)) {
		if (a1 != a2) {
			b1 = interCenPt[0].y - a1 * interCenPt[0].x;
			b2 = interCenPt[1].y - a2 * interCenPt[1].x;

			dCenterX = (b2 - b1) / (a1 - a2);
			dCenterY = a1 * dCenterX + b1;
		}
		else {
			dParallelCheck = 1;
			//AfxMessageBox(_T("Can not make Circle (is not three points)"));
		}
	}
	// 두 중간 좌표에서 수직이되는 a1과 a2 중 하나라도 inf 인 경우
	else {
		// a1이 inf 인 경우 -> 첫 번째 클릭 지점과 두 번째 클릭 지점을 잇는 선의 기울기가 0 -> 기울기가 0 인 선의 수직선 기울기는 inf
		if (IsFiniteNumber(a1) && !IsFiniteNumber(a2)) {
			b2 = interCenPt[1].y - a2 * interCenPt[1].x;

			dCenterX = interCenPt[0].x;
			dCenterY = a2 * dCenterX + b2;
		}
		// a2가 inf 인 경우 -> 첫 번째 클릭 지점과 세 번째 클릭 지점을 잇는 선의 기울기가 0 -> 기울기가 0 인 선의 수직선 기울기는 inf
		else if (!IsFiniteNumber(a1) && IsFiniteNumber(a2)) {
			b1 = interCenPt[1].y - a1 * interCenPt[1].x;

			dCenterX = interCenPt[1].x;
			dCenterY = a1 * dCenterX + b1;
		}
		// a1과 a2가 모두 inf 인 경우 -> 클릭 된 세 점이 일직선 -> 세 점을 잇는 정원 그리기 불가능
		else {
			// 현재 함수에서 반환시 정원 그리는 프로세스를 생략하기 위한 Flag 변수 -> 1인 경우 원 그리는 소스 코드 생략
			dParallelCheck = 1;
			//AfxMessageBox(_T("Can not make Circle (is not three points)"));
		}
	}

	dRadius = sqrt(pow(dCenterX - p_ClickPointList[0].x, 2) + pow(dCenterY - p_ClickPointList[0].y, 2));
	
	circleInfo[0] = dCenterX;
	circleInfo[1] = dCenterY;
	circleInfo[2] = dRadius;
	circleInfo[3] = dParallelCheck;
	
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

	// 현재 마우스 커서 위치 획득
	int nCursorX = point.x;
	int nCursorY = point.y;

	bool bCheckInCircle = false;
	
	// 세 점이 모두 클릭된 경우에만 동작
	if (m_nClickNum > 3) {
		// 현재 마우스 왼쪽 버튼이 클릭 되어있는 상태인지 확인 (클릭 후 첫 드래그 이벤트 호출시)
		if (m_nBtnClickFlag) {
			// 마우스 왼쪽 버튼 클릭 상태로 유지중인지 확인 (클릭 후 두번째 이상의 드래그 이벤트 호출시)
			if (!bKeepClick) {
				// 어떤 클릭 지점을 클릭하고 드래그하는지 확인 -> 클릭 좌표 리스트의 index 획득 및 클릭 유지 상태 Flag 변경 -> 클릭 좌표 리스트 업데이트
				for (int i = 0; i < 3; i++) {
					if (isInCircle(nCursorX, nCursorY, m_pClickPoint[i].x, m_pClickPoint[i].y, nRadius)) {
						bKeepClick = true;
						bCheckInCircle = true;
						m_nUpdateIndex = i;
						tmpPt.x = nCursorX;
						tmpPt.y = nCursorY;
						m_pClickPoint[m_nUpdateIndex] = tmpPt;
					}
				}
			}
			else {
				// 마우스 왼쪽 버튼 클릭 유지 Flag가 true(유지 중 : 이미 클릭 좌표 리스트의 index 존재) 인 경우 ->클릭 좌표 리스트 업데이트
				tmpPt.x = nCursorX;
				tmpPt.y = nCursorY;
				m_pClickPoint[m_nUpdateIndex] = tmpPt;
				bCheckInCircle = true;
			}

			// 클릭한 지점이 기존 클릭된 원 내부에 존재하는 경우 클릭 위치 원 이미지 갱신 및 정원 이미지 갱신
			if (bCheckInCircle) {
				// 화면 초기화
				memset(fm, 0xff, NWIDTH * NHEIGHT);

				// 정원 그리기
				drawBigCircle(fm, m_pClickPoint, thickness);
				
				// 클릭 위치 3개에 원 그리기
				drawThreeCircle(fm, m_pClickPoint, m_nClickNum, nRadius, nBlack);

				// 화면 업데이트 및 클릭 위치 3개의 각각 중심 좌표 UI에 출력
				updateDisplay(m_pClickPoint, m_nClickNum, nRadius);
			}
		}
		// 드래그 중 마우스 왼쪽 클릭 해제 한 경우
		else {
			bKeepClick = false;
		}
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CDlgImage::resetProcess()
{
	// 초기화 버튼 클릭 시 현재 클래스 멤버 변수 값 초기화
	m_nClickNum = 0;

	m_pClickPoint[0] = {};
	m_pClickPoint[1] = {};
	m_pClickPoint[2] = {};

	bKeepClick = false;
}

void CDlgImage::randomProcess()
{
	// 랜덤 이동
	if (m_nClickNum >= 3) {
		CgPrjDlg* pDlg = (CgPrjDlg*)GetParent();
		int nRadius = pDlg->m_nRadiusNum;
		int thickness = pDlg->m_nCircleThick;
		unsigned char* fm = (unsigned char*)m_image.GetBits();

		m_nClickNum = 3;

		// 랜덤 좌표 3개 생성 후 클릭 좌표 리스트 업데이트
		for (int i = 0; i < m_nClickNum; i++) {
			m_pClickPoint[i].x = rand() % NWIDTH;
			m_pClickPoint[i].y = rand() % NHEIGHT;
		}
		memset(fm, 0xff, NWIDTH * NHEIGHT);

		// 랜덤 좌표로 업데이트 된 클릭 좌표 리스트로 정원 그리기
		drawBigCircle(fm, m_pClickPoint, thickness);

		// 랜덤 좌표로 업데이트 된 클릭 좌표의 원 3개 그리기
		drawThreeCircle(fm, m_pClickPoint, m_nClickNum, nRadius, nBlack);

		updateDisplay(m_pClickPoint, m_nClickNum, nRadius);
	}
}

// inf 인지 확인 함수
bool CDlgImage::IsFiniteNumber(double x)
{
	return !(x <= DBL_MAX && x >= -DBL_MAX);
}