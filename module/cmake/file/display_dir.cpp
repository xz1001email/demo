#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

int getFileId(char *filename)
{
    printf("filename = %s\n", filename);
    
    return strtol(filename, NULL, 10);
}

 
#if 0
struct stat {
    dev_t     st_dev;         /* ID of device containing file */
    ino_t     st_ino;         /* inode number */
    mode_t    st_mode;        /* protection */
    nlink_t   st_nlink;       /* number of hard links */
    uid_t     st_uid;         /* user ID of owner */
    gid_t     st_gid;         /* group ID of owner */
    dev_t     st_rdev;        /* device ID (if special file) */
    off_t     st_size;        /* total size, in bytes */
    blksize_t st_blksize;     /* blocksize for filesystem I/O */
    blkcnt_t  st_blocks;      /* number of 512B blocks allocated */

    /* Since Linux 2.6, the kernel supports nanosecond
     *                   precision for the following timestamp fields.
     *                                     For the details before Linux 2.6, see NOTES. */

    struct timespec st_atim;  /* time of last access */
    struct timespec st_mtim;  /* time of last modification */
    struct timespec st_ctim;  /* time of last status change */

#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};

#endif


uint64_t getAllFileSize(const char *dir, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    int maxid = 0;
    int tmpid = 0;
    struct stat fstat;
    uint64_t sum=0;

    char pathname[100];
    const char *p;

    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "Can`t open directory %s\n", dir);
        return -1;
    }

    chdir(dir);
    while ((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
#if 0
            if (strcmp(entry->d_name, ".") == 0 || 
                    strcmp(entry->d_name, "..") == 0 )  
                continue;
            printf("    %*s%s/\n", depth, "", entry->d_name);
            printdir(entry->d_name, depth+4);
#endif
        } else{
            printf("%s: st_size = %ld\n", entry->d_name, statbuf.st_size);
            sum += statbuf.st_size;
            stat((const char *)&pathname[0], &fstat);
        }
    }
    chdir("..");
    closedir(dp);

    printf("files size sum = %ld\n", sum);

    return sum;
}


int printdir(const char *dir, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    int maxid = 0;
    int tmpid = 0;
    struct stat fstat;
    uint64_t sum=0;

    char pathname[100];
    const char *p;

    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "Can`t open directory %s\n", dir);
        return -1;
    }

    chdir(dir);
    while ((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
#if 0
            if (strcmp(entry->d_name, ".") == 0 || 
                    strcmp(entry->d_name, "..") == 0 )  
                continue;
            printf("    %*s%s/\n", depth, "", entry->d_name);
            printdir(entry->d_name, depth+4);
#endif
        } else{
#if 0
            struct timespec st_atim;  /* time of last access */
            struct timespec st_mtim;  /* time of last modification */
            struct timespec st_ctim;
#endif
            printf("%s: st_size = %ld\n", entry->d_name, statbuf.st_size);
            sum += statbuf.st_size;
#if 1
            //printf("%*s%s\n", depth, "", entry->d_name);
            //printf("%s/%s\n", dir, entry->d_name);
            //snprintf(pathname, sizeof(pathname), "%s/%s\n", dir, entry->d_name);
            tmpid = getFileId(entry->d_name);
            maxid = maxid > tmpid ? maxid : tmpid;
            printf("max id = %d\n", maxid);
            stat((const char *)&pathname[0], &fstat);
#endif

        }
    }
    chdir("..");
    closedir(dp);

    printf("files size sum = %lfM\n", sum/(1024*1024*1.0));

    return maxid;
}


int main(int argc, char *argv[])
{
    uint64_t Bsize=0;
    char topdir[128];
    if (argc != 2){
        printf("input file name!\n");
        return -1;
    }

    strcpy(topdir, argv[1]);
    printf("Directory scan of %s\n", topdir);
    printdir(topdir, 0);

    Bsize = getAllFileSize(topdir, 0);
    printf("size = %ld, %.1lfM\n", Bsize, Bsize/(1024*1024*1.0));

    printf("done.\n");
    exit(0);
}


