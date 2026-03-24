#ifndef SKIFREE_RESOURCE_H
#define SKIFREE_RESOURCE_H

#define IDS_TITLE 1
#define IDB_BITMAP1 1
#define IDC_MYICON 2
#define IDS_PAUSED 2
#define IDB_BITMAP2 2
#define IDS_TIME 3
#define IDB_BITMAP3 3
#define IDS_DIST 4
#define IDB_BITMAP4 4
#define IDS_SPEED 5
#define IDS_STYLE 6
#define IDS_TIME_BLANK 7
#define IDS_DIST_BLANK 8
#define IDS_SPEED_BLANK 9
#define IDS_STYLE_BLANK 10
#define IDS_TIME_FORMAT 11
#define IDS_DIST_FORMAT 12
#define IDS_SPEED_FORMAT 13
#define IDS_STYLE_FORMAT 14
#define IDS_HIGH_SCORES 15
#define IDS_THATS_YOU 16
#define IDS_TRY_AGAIN 17

/* String table (originally in .rc resource script) */
static char *STRINGTABLE[] = {
    "SkiFree",
    "Ski Paused ... Press F3 to continue",
    "Time:",
    "Dist:",
    "Speed:",
    "Style:",
    "00:00:00.00",
    " 0000m",
    " 0000m/s",
    "0000000",
    "%2u:%2.2u:%2.2u.%2.2u",
    "%5.2dm",
    "%5.2dm/s",
    "%7d",
    "High Scores",
    " <-- that's you",
    " <-- try again!"
};

#endif
