/**
 * secure_getenv() replacement for systems that don't have it.
 * For popt-1.16.
 */

#include "system.h"

#ifndef _WIN32
static int enable_secure_decided = 0;
static int enable_secure = 1;
#endif

/**
 * secure_getenv() replacement for systems that don't have it.
 * @param name Variable name.
 * @return Value, or NULL if not found or not available due to security restrictions.
 */
char *popt_secure_getenv(const char *name)
{
#ifdef _WIN32
	// TODO: There's no uid/gid on Windows...
	return getenv(name);
#else /* !_WIN32 */
	// glibc's secure_getenv() checks if
	// euid == uid and egid == gid.
	// If either of them don't match,
	// always return NULL.
	if (!enable_secure_decided) {
		if (geteuid() == getuid() &&
		    getegid() == getgid())
		{
			// UID/GID matches.
			// Security restrictions can be dropped.
			enable_secure = 0;
		} else {
			// UID or GID does not match.
			// Enable security restrictions.
			enable_secure = 1;
		}
		enable_secure_decided = 1;
	}

	if (enable_secure) {
		// Security restrictions are enabled.
		return NULL;
	}

	// Get the environment variable.
	return getenv(name);
#endif /* _WIN32 */
}
