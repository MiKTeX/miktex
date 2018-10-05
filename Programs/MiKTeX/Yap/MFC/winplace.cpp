 ////////////////////////////////////////////////////////////////
// CWindowPlacement 1996 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
#include "StdAfx.h"
#include "winplace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CWindowPlacement::CWindowPlacement()
{
   // Note: "length" is inherited from WINDOWPLACEMENT
   length = sizeof(WINDOWPLACEMENT);
}

CWindowPlacement::~CWindowPlacement()
{
}

//////////////////
// Restore window placement from profile key
BOOL CWindowPlacement::Restore(LPCTSTR lpKeyName, CWnd* pWnd)
{
   GetProfileWP(lpKeyName);

   // Only restore if window intersets the screen.
   //
   CRect rcTemp, rcScreen(0,0,GetSystemMetrics(SM_CXSCREEN),
      GetSystemMetrics(SM_CYSCREEN));
   if (!::IntersectRect(&rcTemp, &rcNormalPosition, &rcScreen))
      return FALSE;

   pWnd->SetWindowPlacement(this);  // set placement
   return TRUE;
}

//////////////////
// Get window placement from profile.
void CWindowPlacement::GetProfileWP(LPCTSTR lpKeyName)
{
   CWinApp *pApp = AfxGetApp();
   ASSERT_VALID(pApp);

   showCmd = pApp->GetProfileInt(lpKeyName, _T("wp.showCmd"), showCmd);
   flags   = pApp->GetProfileInt(lpKeyName, _T("wp.flags"), flags);

ptMinPosition.x = pApp->GetProfileInt(lpKeyName, _T("wp.ptMinPosition.x"), 
      ptMinPosition.x);
ptMinPosition.y = pApp->GetProfileInt(lpKeyName, _T("wp.ptMinPosition.y"),
      ptMinPosition.y);
ptMaxPosition.x = pApp->GetProfileInt(lpKeyName, _T("wp.ptMaxPosition.x"), 
      ptMaxPosition.x);
ptMaxPosition.y = pApp->GetProfileInt(lpKeyName, _T("wp.ptMaxPosition.y"),
      ptMaxPosition.y);

   RECT& rc = rcNormalPosition;  // because I hate typing
   rc.left  = pApp->GetProfileInt(lpKeyName, _T("wp.left"),   rc.left);
   rc.right = pApp->GetProfileInt(lpKeyName, _T("wp.right"),  rc.right);
   rc.top   = pApp->GetProfileInt(lpKeyName, _T("wp.top"),    rc.top);
   rc.bottom= pApp->GetProfileInt(lpKeyName, _T("wp.bottom"), rc.bottom);
}

////////////////
// Save window placement in app profile
void CWindowPlacement::Save(LPCTSTR lpKeyName, CWnd* pWnd)
{
   pWnd->GetWindowPlacement(this);
   WriteProfileWP(lpKeyName);
}

//////////////////
// Write window placement to app profile
void CWindowPlacement::WriteProfileWP(LPCTSTR lpKeyName)
{
   CWinApp *pApp = AfxGetApp();
   ASSERT_VALID(pApp);
   pApp->WriteProfileInt(lpKeyName, _T("wp.showCmd"),         showCmd);
   pApp->WriteProfileInt(lpKeyName, _T("wp.flags"),           flags);
   pApp->WriteProfileInt(lpKeyName, _T("wp.ptMinPosition.x"), ptMinPosition.x);
   pApp->WriteProfileInt(lpKeyName, _T("wp.ptMinPosition.y"), ptMinPosition.y);
   pApp->WriteProfileInt(lpKeyName, _T("wp.ptMaxPosition.x"), ptMaxPosition.x);
   pApp->WriteProfileInt(lpKeyName, _T("wp.ptMaxPosition.y"), ptMaxPosition.y);
   pApp->WriteProfileInt(lpKeyName, _T("wp.left"),  rcNormalPosition.left);
   pApp->WriteProfileInt(lpKeyName, _T("wp.right"), rcNormalPosition.right);
   pApp->WriteProfileInt(lpKeyName, _T("wp.top"),   rcNormalPosition.top);
   pApp->WriteProfileInt(lpKeyName, _T("wp.bottom"),rcNormalPosition.bottom);
}

// The ugly casts are required to help the VC++ 3.0 compiler decide which
// operator<< or operator>> to use. If you're using VC++ 4.0 or later, you 
// can delete this stuff.
//
#if (_MSC_VER < 1000)      // 1000 = VC++ 4.0
#define UINT_CAST (LONG)
#define UINT_CASTREF (LONG&)
#else
#define UINT_CAST
#define UINT_CASTREF
#endif

//////////////////
// Write window placement to archive
// WARNING: archiving functions are untested.
CArchive& operator<<(CArchive& ar, const CWindowPlacement& wp)
{
   ar << UINT_CAST wp.length;
   ar << UINT_CAST wp.flags;
   ar << UINT_CAST wp.showCmd;
   ar << wp.ptMinPosition;
   ar << wp.ptMaxPosition;
   ar << wp.rcNormalPosition;
   return ar;
}

//////////////////
// Read window placement from archive
// WARNING: archiving functions are untested.
CArchive& operator>>(CArchive& ar, CWindowPlacement& wp)
{
   ar >> UINT_CASTREF wp.length;
   ar >> UINT_CASTREF wp.flags;
   ar >> UINT_CASTREF wp.showCmd;
   ar >> wp.ptMinPosition;
   ar >> wp.ptMaxPosition;
   ar >> wp.rcNormalPosition;
   return ar;
}
