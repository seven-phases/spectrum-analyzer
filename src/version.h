
#define VER       1,0,7
#define NAME      "Spectrum Analyzer"
#define COMPANY   "Seven Phases"
#define COPYRIGHT "© Max Mikhailov, 2009-2015"

// ............................................................................

#define PP_STR(A)          PP_STR__(A)
#define PP_STR_1(A)        PP_STR(A)
#define PP_STR__(A)        #A

#define PP_CAT(A, B)       PP_CAT__(A, B)
#define PP_CAT__(A, B)     A##B

#define VERSION__(A, B, C) A.PP_CAT(B, C)

#define VERSION            PP_CAT(VERSION__, (VER))
#define VERSION_STR        PP_STR_1(VERSION)
#define VERSION_RC         VER, 0

// ............................................................................

#ifdef PREPROCESS_ONLY

VERSION
VERSION_RC
VERSION_STR

#endif // ~ PREPROCESS_ONLY

// ............................................................................
