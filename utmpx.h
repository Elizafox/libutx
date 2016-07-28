#ifndef __UTMPX_H__
#define __UTMPX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* An ultra-lazy implementation of utmpx from POSIX.
 * The author of musl is kind of crazy and believes that utmp is a "security
 * risk". I believe people who own a system are entitled to know who is on it,
 * even if the information may not be accurate due to compromise.
 *
 * Besides that, working utmpx is needed for XSI compliance. This is really
 * made for Adélie Linux.
 *
 * --Elizafox
 */

#include <stdint.h>
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
#define UT_HOSTSIZE	256

// Hardcoded for now, set this to whatever you want
#define UT_FILE "/var/run/utmp"

// Required for POSIX
#define UTMPX_FILE UT_FILE

struct utmpx
{
	short ut_type;			// Type of entry.
	pid_t ut_pid;			// Process ID.
	char ut_line[UT_LINESIZE];	// Device name.
	char ut_id[4];			// Terminal name suffix or inittab(5) ID
	char ut_user[UT_NAMESIZE];	// User login name.

	// *** non-standard fields ***
	// Solaris contains all non-standard fields.
	// BSD has ut_host.
	// IRIX and z/OS have ut_host, ut_exit, and ut_session.
	char ut_host[UT_HOSTSIZE];	// Host name for remote logins.
	struct {
		short e_termination;
		short e_exit;
	} ut_exit;			// Exit status
	int32_t ut_session;		// Session ID for X11.

	// *** standard field! ***
	struct timeval ut_tv;		// Time entry was made.

	// *** non-standard fields ***
	union
	{
		uint32_t ut_addr;	// IPv4 address
		uint32_t ut_addr_v6[4];	// IPv6 address (v4 uses [0] only)
	};

	char __unused[20];		// Unused.
};

#define ut_name ut_user
#define ut_xtime ut_tv.tv_sec
#define ut_addr ut_addr_v6[0]

void setutxent(void);
struct utmpx *getutxent(void);
struct utmpx *getutxid(const struct utmpx *);
struct utmpx *getutxline(const struct utmpx *);
struct utmpx *pututxline(const struct utmpx *);
void endutxent(void);

// Compatibility functions
#define utmp utmpx

void setutent(void);
struct utmp *getutent(void);
struct utmp *getutid(const struct utmp *);
struct utmp *pututline(const struct utmp *);
void endutent(void);

#ifdef __cplusplus
}
#endif

#endif // __UTMPX_H__
