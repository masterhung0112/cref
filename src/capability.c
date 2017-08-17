#include "capability.h"

struct capabilities_t {
	uid_t uid;
	gid_t gid;

#ifdef CAPABILITIES_NATIVE
	struct __user_cap_data_struct caps[2];
#endif
};

/**
 * Returns TRUE if the current process/usr is member of the given group
 */
static bool has_group(gid_t group)
{
	gid_t *groups;
	long ngroups, i;
	bool found = FALSE;
	
	if (group == getegid()) {
		/* It's unspecified if this is part of the list below or not */
		return TRUE;
	}
	
	/* sysconf: unistd.h
	 */
	ngroups = sysconf(_SC_NGROUPS_MAX);
	if (ngroups == -1) {
		printf("getting groups for current process failed: %s\n", strerrno(errno));
		return FALSE;
	}
	groups = calloc(ngroups + 1, sizeof(gid_t));
	ngroups = getgroups(ngroups, groups);
	if (ngroups == -1) {
		printf("getting groups for current process failed: %s\n", strerrno(errno));
		free(groups);
		return FALSE;
	}
	for (i = 0; i < ngroups; ++i) {
		if (group == groups[i]) {
			found = TRUE;
			break;
		}
	}
	free(groups);
	
	return found;
}

bool capabilitiesCheck(capabilities_t *this, uint32_t cap)
{
	if (cap == CAP_CHOWN) {
		/* if new files/UNIX sockets are created they should be owned by the
		 * configured user and group. This requires a call to chown(2).
		 * But CAP_CHOWN is not always required */
		if (!this->uid || geteuid() == this->uid) {
			/* If the owner does not change CAP_CHOWN is not needed */
			if (!this->gid || has_group(this->gid)) {
				/* The same applies if the owner is the member of the group */
				if (ignore) {
					/* We don't have to keep this, if requested */
					*ignored = TRUE;
				}
				return TRUE;
			}
		}
	}
#ifdef CAPABILITIES_NATIVE
	struct __user_cap_header_struct header = {
#ifdef _LINUX_CAPABILITY_VERSION_3
		.version = _LINUX_CAPABILITY_VERSION_3,
#elif defined(_LINUX_CAPABILITY_VERSION_2)
		.version = _LINUX_CAPABILITY_VERSION_2,
#elif defined(_LINUX_CAPABILITY_VERSION_1)
		.version = _LINUX_CAPABILITY_VERSION_1,
#else
		.version = _LINUX_CAPABILITY_VERSION,
#endif
	};
	struct __user_cap_data_struct caps[2];
	int i = 0;
	
	if (cap >= 32) {
		i++;
		cap -= 32;
	}
	return capget(&header, caps) == 0 && caps[i].permitted & (1 << cap);
#endif /* CAPABILITIES_NATIVE */
}


/**
 * Keep the given capability if is held by the current process.
 * Return FALSE if this is not the case.
 */
static bool keep_capability(capabilities_t *this, uint32_t cap)
{
#ifdef CAPABILITIES_NATIVE
	int i = 0;
	if (cap >= 32) {
		cap -= 32;
		i++;
	}
	this->caps[i].effective |= 1 << cap;
	this->caps[i].permitted |= 1 << cap;
	this->caps[i].inheritable |= 1 << cap;
#endif /* CAPABILITIES_NATIVE */
	return TRUE;
}

bool capabilitiesResolveUid(capabilities_t *this, char *username)
{
	struct passwd *pwp;
	int err;
	
#ifdef HAVE_GETPWNAM_R
	struct passwd passwd;
	size_t buflen = 1024;
	char *buf = NULL;
	
	while (TRUE) {
		buf = realloc(buf, buflen);
		err = getpwnam_r(username, &passwd, buf, buflen, &pwp);
		if (err == ERANGE) {
			buflen *= 2;
			continue;
		}
		if (pwp) {
			this->uid = pwp->pw_uid;
		}
		break;
	}
	free(buf);
#else /* HAVE_GETPWNAM_R */
	pwp = getpwnam(username);
	if (pwp) {
		this->uid = pwp->pw_uid;
	}
	err = errno;
#endif /* HAVE_GETPWNAM_R */
	if (pwp) {
		return TRUE;
	}
	printf("resolving user '%s' failed: %s\n", username, err ? strerror(err) : "user not found");
	return FALSE;
}

bool capabilitiesResolveGid(capabilities_t *this, char *groupname)
{
	struct group *grp;
	int err;
	
#ifdef HAVE_GETGRNAM_R
	struct group group;
	size_t buflen = 1024;
	char *buf = NULL;
	
	while (TRUE) {
		buf = realloc(buf, buflen);
		err = getgrnam_r(groupname, &group, buf, buflen, &grp);
		if (err == ERANGE) {
			buflen *= 2;
			continue;
		}
		if (grp) {
			this->gid = grp->gr_gid;
		}
		break;
	}
	free(buf);
#else /* HAVE_GETGRNAM_R */
	grp = getgrnam(groupname);
	if (pwp) {
		this->gid = grp->gr_gid;
	}
	err = errno;
#endif /* HAVE_GETGRNAM_R */
	if (pwp) {
		return TRUE;
	}
	printf("resolving group '%s' failed: %s\n", groupname, err ? strerror(err) : "group not found");
	return FALSE;
}
