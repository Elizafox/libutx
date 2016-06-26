#include "utmpx.h"
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <sys/file.h>

static struct utmpx ut;
static int utmpfd = -1;
static bool utmpfd_open = false;


void setutxent(void)
{
	if(geteuid() != 0)
	{
		errno = EPERM;
		return;
	}

	if(!utmpfd_open && (utmpfd = open(UT_FILE, O_RDWR)) < 0)
	{
		syslog(LOG_ALERT, "Could not open utmp file "
				UT_FILE " for read and write: %m");
		return;
	}

	if(lseek(utmpfd, SEEK_SET, 0) == -1)
	{
		syslog(LOG_ALERT, "Could not seek in utmp file "
				UT_FILE ": %m");
		close(utmpfd);
		return;
	}

	utmpfd_open = true;
	errno = 0; // Reset just in case
}

struct utmpx *getutxent(void)
{
	int ret;
	struct utmpx *utl = NULL;

	if(!utmpfd_open)
	{
		setutxent();

		if(errno != 0)
			return NULL;
	}

	// Obtain reader lock
	if(flock(utmpfd, LOCK_SH) < 0)
	{
		syslog(LOG_ALERT, "Could not obtain shared lock on utmp file "
				UT_FILE ": %m");
		return NULL;
	}

	if((ret = read(utmpfd, &ut, sizeof(struct utmpx))) < 0)
	{
		syslog(LOG_ALERT, "Could not read from utmp file "
				UT_FILE ": %m");
		goto end;
	}
	else if(ret == 0)
	{
		// Simple EOF
		errno = ESRCH;
		goto end;
	}

	errno = 0;  // Reset just in case
	utl = &ut;

end:
	flock(utmpfd, LOCK_UN); // Just blindly assume it worked
	return utl;
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

	// No need to set errno, getutxent() does for us
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
			// TODO - find glibc behaviour
			errno = EINVAL;
			return NULL;
			break;
		}
	}

	// No need to set errno, getutxent() does for us
	return NULL;
}

struct utmpx *pututxline(const struct utmpx *uts)
{
	struct utmpx *utl = NULL;

	// Seek to proper position and reset errno unconditionally
	getutxid(uts);

	// Check errno to ensure it's what we expect
	if(errno != ESRCH && errno != 0)
		return NULL;

	/* Obtain lock on the file to avoid lossage.
	 * Better safe than sorry...
	 */
	if(flock(utmpfd, LOCK_EX) < 0)
	{
		syslog(LOG_ALERT, "Could not get exclusive lock on utmp file "
				UT_FILE ": %m");
		return NULL;
	}

	if(write(utmpfd, uts, sizeof(struct utmpx)) < 1)
	{
		syslog(LOG_ALERT, "Could not read from utmp file "
				UT_FILE ": %m");
		goto end;
	}

	utl = (struct utmpx *)uts; // Following glibc behaviour
	errno = 0; // Just in case

end:
	flock(utmpfd, LOCK_UN); // Blindly assume it succeeded
	return utl;
}

void endutxent(void)
{
	if(utmpfd_open)
		close(utmpfd);

	utmpfd_open = false;
	errno = 0; // Just in case
}


#if defined(__GNUC__) || defined(__clang__) || defined(weak_alias)

// Create weak_alias like musl does
#ifndef weak_alias

#define weak_alias(old, new) \
	extern __typeof(old) new __attribute__((weak, alias(#old)))

#endif // weak_alias

weak_alias(endutxent, endutent);
weak_alias(setutxent, setutent);
weak_alias(getutxent, getutent);
weak_alias(getutxid, getutid);
weak_alias(getutxline, getutline);
weak_alias(pututxline, pututline);

#else // defined(__GNUC__) || defined(__clang__)

// Shitty fallback

void setutent(void) { setutxent(); }
struct utmp *getutent(void) { return getutxent(); }
struct utmp *getutid(const struct utmp *uts) { return getutxid(uts); }
struct utmp *pututline(const struct utmp *uts) { return pututxline(uts); }
void endutent(void) { endutxent(); }

#endif // defined(__GNUC__) || defined(__clang__) || defined(weak_alias)
