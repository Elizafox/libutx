#ifndef __UTMPX_H__
#define __UTMPX_H__

/* An ultra-lazy implementation of utmpx from POSIX.
 * The author of musl is kind of crazy and believes that utmp is a "security
 * risk". I believe people who own a system are entitled to know who is on it,
 * even if the information may not be accurate due to compromise.
 *
 * Besides that, working utmpx is needed for XSI compliance. This is really
 * made for Adélie Linux.
 *
 * I'm lazy and don't really feel like making a "full-featured" implementation
 * that is really suitable for auditing purposes and whatnot (this also means
 * no remote IP addresses yet). Deal with it for now.
 *
 * --Elizafox
 */

#include <unistd.h>
#include <sys/time.h>

// IEEE Std 1003.1, 2013 Edition, <utmpx.h>

#define EMPTY		0
#define BOOT_TIME	1
#define OLD_TIME	2
#define NEW_TIME	3
#define USER_PROCESS	4
#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define DEAD_PROCESS	7

// Not in POSIX but needed anyway... conforming to what glibc does.
#define UT_LINESIZE	32
#define UT_NAMESIZE	32

// Hardcoded for now, set this to whatever you want
#define UT_FILE "/var/run/utmp"

struct utmpx
{
	char ut_user[UT_NAMESIZE];	// User login name.
	char ut_id[4];			// Terminal name suffix or inittab(5) ID
	char ut_line[UT_LINESIZE];	// Device name.
	pid_t ut_pid;			// Process ID.
	short ut_type;			// Type of entry.
	struct timeval ut_tv;		// Time entry was made.
};

void setutxent(void);
struct utmpx *getutxent(void);
struct utmpx *getutxid(const struct utmpx *);
struct utmpx *getutxline(const struct utmpx *);
struct utmpx *pututxline(const struct utmpx *);
void endutxent(void);

#endif // __UTMPX_H__
