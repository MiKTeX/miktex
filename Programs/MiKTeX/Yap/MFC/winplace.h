 ////////////////////////////////////////////////////////////////
// CWindowPlacement 1996 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.

////////////////
// CWindowPlacement reads and writes WINDOWPLACEMENT 
// from/to application profile and CArchive.
//
struct CWindowPlacement : public WINDOWPLACEMENT {
   CWindowPlacement();
   ~CWindowPlacement();
   
   // Read/write to app profile
   void GetProfileWP(LPCTSTR lpKeyName);
   void WriteProfileWP(LPCTSTR lpKeyName);

   // Save/restore window pos (from app profile)
   void Save(LPCTSTR lpKeyName, CWnd* pWnd);
   BOOL Restore(LPCTSTR lpKeyName, CWnd* pWnd);

   // Save/restore from archive
   friend CArchive& operator<<(CArchive& ar, const CWindowPlacement& wp);
   friend CArchive& operator>>(CArchive& ar, CWindowPlacement& wp);
};
