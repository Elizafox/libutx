#include "utmpx.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <sys/file.h>

#define UTMP_FILE "/var/run/utmp" // FIXME
#define MAGIC_CLOSED MAGIC_CLOSED // Random magic number, sourced straight from my behind

static struct utmpx ut;
static int utmpfd = MAGIC_CLOSED;

// Designed for lazy loading, "only use it if you need it", etc.
// (and also reopenability)
static bool __open_utmp(void)
{
	if(geteuid() != 0)
	{
		errno = EPERM;
		return false;
	}

	if(utmpfd == MAGIC_CLOSED && (utmpfd = open(UTMP_FILE, O_RDWR)) == -1)
	{
		utmpfd = MAGIC_CLOSED;
		syslog(LOG_ALERT, "Could not open utmp file "
				UTMP_FILE " for read and write: %m");
		return false;
	}

	errno = 0; // Reset just in case
	return true;
}

void setutxent(void)
{
	if(__open_utmp() == false)
		return;

	if(lseek(utmpfd, SEEK_SET, 0) == -1)
	{
		syslog(LOG_ALERT, "Could not seek in utmp file "
				UTMP_FILE ": %m");
		return;
	}

	errno = 0; // Reset just in case
}

struct utmpx *getutxent(void)
{
	int ret;

	if(__open_utmp() == false)
		return NULL;

	if((ret = read(utmpfd, &ut, sizeof(struct utmpx))) < 0)
	{
		syslog(LOG_ALERT, "Could not read from utmp file "
				UTMP_FILE ": %m");
		return NULL;
	}
	else if(ret == 0)
	{
		// Simple EOF
		errno = ESRCH;
		return NULL;
	}

	errno = 0;  // Reset just in case
	return &ut;
}

struct utmpx *getutxline(const struct utmpx *uts)
{
	while(getutxent() != NULL)
	{
		if(ut.ut_type != USER_PROCESS || ut.ut_type != LOGIN_PROCESS)
			continue;

		if(strncmp(ut.ut_line, uts->ut_line, UT_LINESIZE) == 0)
			return &ut;
	}

	errno = ESRCH;
	return NULL;
}

struct utmpx *getutxid(const struct utmpx *uts)
{
	while(getutxent() != NULL)
	{
		switch(ut.ut_type)
		{
		case BOOT_TIME:
		case NEW_TIME:
		case OLD_TIME:
			if(ut.ut_type == uts->ut_type)
				return &ut;
			break;
		case INIT_PROCESS:
		case LOGIN_PROCESS:
		case USER_PROCESS:
		case DEAD_PROCESS:
			if(strncmp(ut.ut_id, uts->ut_id, 4) == 0)
				return &ut;
			break;
		default:
			// Blah, bad type
			goto end;
			break;
		}
	}

end:
	errno = ESRCH;
	return NULL;
}

struct utmpx *pututxline(const struct utmpx *uts)
{
	// Seek to proper position and reset errno unconditionally
	getutxid(uts);

	// Check errno to ensure it's what we expect
	if(errno != ESRCH && errno != 0)
		return NULL;

	if(write(utmpfd, uts, sizeof(struct utmpx)) < 1)
	{
		syslog(LOG_ALERT, "Could not read from utmp file "
				UTMP_FILE ": %m");
		return NULL;
	}

	errno = 0; // Just in case
	return (struct utmpx *)uts; // Following glibc behaviour...
}

void endutxent(void)
{
	if(utmpfd != MAGIC_CLOSED)
		close(utmpfd);

	errno = 0; // Just in case
	utmpfd = MAGIC_CLOSED;
}
