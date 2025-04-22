#pragma once
#include "afxdialogex.h"
#include <cmath>


// CDlgImage 대화 상자

class CDlgImage : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgImage)

public:
	CDlgImage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgImage();

	CImage m_image;
	CWnd* m_pParent;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CDlgImage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	BOOL validImgPos(int x, int y);
	void drawCircle(unsigned char* fm, int x, int y, int nRadius, int nGray);
	bool isInCircle(int i, int j, int nCenterX, int nCenterY, int nRadius);
	void updateDisplay();
	void updateDisplay(CPoint* p_ClickPointList, int nClickNum, int nCircleRadius);

private:
	void initImage();
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	int m_nClickNum;
	CPoint m_pClickPoint[3];
	void CDlgImage::drawBigCircle(unsigned char* fm, CPoint* p_ClickPointList, int thickness);
	void drawThreeCircle(unsigned char* fm, CPoint* p_ClickPointList, int nClickNum, int nRadius, int nBlack);
	int nBlack = 0;
	double* calcBigCircleInfo(CPoint* p_ClickPointList);
};
