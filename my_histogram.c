#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

/* function type that is called for each filename */
typedef	int	Myfunc(const char *, const struct stat *, int);

static Myfunc	myfunc;
static int		myftw(char *, Myfunc *);
static int		dopath(Myfunc *);

static long	nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int
main(int argc, char *argv[])
{
	int		ret;
	fprintf(stdout, "%s\n\n", "Content-type: text/plain");

	if (argc != 2) {
		printf("usage:  ftw  <starting-pathname>");
		exit(0);
	}

	ret = myftw(argv[1], myfunc);		/* does it all */

	ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
	if (ntot == 0)
		ntot = 1;		/* avoid divide by 0; print 0 for all counts */

	FILE *pipe = popen("gnuplot -persist", "w");

	//printf("set boxwidth 0.5\nset style fill solid\nset key off\nset yrange [0:*]\nset xlabel \"Files\"\nset ylabel \"Frequency\"\nset title \"Frequency of File Types\"\n");
	//printf("set term gif\nset output 'testhist2.gif'\n");
	fprintf(pipe, "set term gif\nset output 'testhist3.gif'\nset title 'File Type Counts'\nset xlabel 'File Types'\nset ylabel 'Count'\nset boxwidth 0.5\nset style fill solid\nset key off\nset yrange [0:*]\n");

	fprintf(pipe, "plot '-' using (column(0)):2:xtic(1) ti col with boxes\n");
	fprintf(pipe, "NULL 0\n"); // I don't know why but the first column cuts off
	fprintf(pipe, "regular %ld\n", nreg);
	  //nreg*100.0/ntot); this is %f
	fprintf(pipe, "directory %ld\n", ndir);
	  //ndir*100.0/ntot);
	fprintf(pipe, "block %ld\n", nblk);
	  // nblk*100.0/ntot);
	fprintf(pipe, "character %ld\n", nchr);
	  // nchr*100.0/ntot);
	fprintf(pipe, "fifo %ld\n", nfifo);
	  // nfifo*100.0/ntot);
	fprintf(pipe, "link %ld\n", nslink);
	  // nslink*100.0/ntot);
	fprintf(pipe, "socket %ld\n", nsock);
	  // nsock*100.0/ntot);
	fprintf(pipe, "e");
	exit(ret);
}

/*
 * Descend through the hierarchy, starting at "pathname".
 * The caller's func() is called for every file.
 */
#define	FTW_F	1		/* file other than directory */
#define	FTW_D	2		/* directory */
#define	FTW_DNR	3		/* directory that can't be read */
#define	FTW_NS	4		/* file that we can't stat */

static char	*fullpath;		/* contains full pathname for every file */
static size_t pathlen;

static int					/* we return whatever func() returns */
myftw(char *pathname, Myfunc *func)
{
	fullpath = malloc(PATH_MAX+1);	/* malloc PATH_MAX+1 bytes */
										/* ({Prog pathalloc}) */
	if (pathlen <= strlen(pathname)) {
		pathlen = strlen(pathname) * 2;
		if ((fullpath = realloc(fullpath, pathlen)) == NULL)
			exit(0);
	}
	strcpy(fullpath, pathname);
	return(dopath(func));
}

/*
 * Descend through the hierarchy, starting at "fullpath".
 * If "fullpath" is anything other than a directory, we lstat() it,
 * call func(), and return.  For a directory, we call ourself
 * recursively for each name in the directory.
 */
static int					/* we return whatever func() returns */
dopath(Myfunc* func)
{
	struct stat		statbuf;
	struct dirent	*dirp;
	DIR				*dp;
	int				ret, n;

	if (lstat(fullpath, &statbuf) < 0)	/* stat error */
		return(func(fullpath, &statbuf, FTW_NS));
	if (S_ISDIR(statbuf.st_mode) == 0)	/* not a directory */
		return(func(fullpath, &statbuf, FTW_F));

	/*
	 * It's a directory.  First call func() for the directory,
	 * then process each filename in the directory.
	 */
	if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
		return(ret);

	n = strlen(fullpath);
	if (n + NAME_MAX + 2 > pathlen) {	/* expand path buffer */
		pathlen *= 2;
		if ((fullpath = realloc(fullpath, pathlen)) == NULL)
			exit(0);
	}
	fullpath[n++] = '/';
	fullpath[n] = 0;

	if ((dp = opendir(fullpath)) == NULL)	/* can't read directory */
		return(func(fullpath, &statbuf, FTW_DNR));

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0  ||
		    strcmp(dirp->d_name, "..") == 0)
				continue;		/* ignore dot and dot-dot */
		strcpy(&fullpath[n], dirp->d_name);	/* append name after "/" */
		if ((ret = dopath(func)) != 0)		/* recursive */
			break;	/* time to leave */
	}
	fullpath[n-1] = 0;	/* erase everything from slash onward */

	if (closedir(dp) < 0)
		exit(0);
	return(ret);
}

static int
myfunc(const char *pathname, const struct stat *statptr, int type)
{
	switch (type) {
	case FTW_F:
		switch (statptr->st_mode & S_IFMT) {
		case S_IFREG:	nreg++;		break;
		case S_IFBLK:	nblk++;		break;
		case S_IFCHR:	nchr++;		break;
		case S_IFIFO:	nfifo++;	break;
		case S_IFLNK:	nslink++;	break;
		case S_IFSOCK:	nsock++;	break;
		case S_IFDIR:	/* directories should have type = FTW_D */
			exit(0);
		}
		break;
	case FTW_D:
		ndir++;
		break;
	case FTW_DNR:
		break;
	case FTW_NS:
		break;
	default:
		break;
	}
	return(0);
}
