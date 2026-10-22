/* Wii and GameCube (libogc): stdout and stderr go to the debug UART, which
 * Dolphin shows in its log as OSREPORT. Built only by build_wii.sh and
 * build_gamecube.sh. */
#include <gccore.h>

__attribute__((constructor)) static void ogc_stdio_report(void)
{
    SYS_STDIO_Report(true);
}
