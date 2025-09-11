#include <sys/statfs.h>

int open_kernel(const char *pathname, int flags, ...);
long write_kernel( int fd, const void * buf, unsigned long count);
int fstat_kernel(int fd, struct stat * st);
int lstat_kernel( const char* pathname, struct stat* st);
int stat_kernel(const char *pathname, struct stat *st);
ssize_t read_kernel( int fd, void * buf, size_t count);
ssize_t pread_kernel(int fd, void *buf, size_t count, off_t offset);
int close_kernel(int fd);
int
mkdir_kernel(const char *pathname, mode_t mode);
int rmdir_kernel(const char* pathname);
int fcntl_kernel(int fd, int cmd, ...);
int ioctl_kernel(int fd, unsigned long int request, ...);
ssize_t pwrite_kernel(int fd, const void *buf, size_t count, off_t offset);
int dup2_kernel(int oldfd, int newfd);
int chdir_kernel( const char* pathname);
char* getcwd_kernel( char* path, size_t size);
off_t lseek_kernel(int fd, off_t offset, int whence);
int fsync_kernel(int fd);
int statvfs_kernel(const char *pathname, struct statvfs *buf);
int __statfs_kernel(const char *pathname, struct statfs *buf);
int unlink_kernel(const char *pathname);
int rename_kernel (const char* oldpath, const char* newpath);
int fstatat_kernel(int dirfd, const char *path, struct stat *st, int flags);
ssize_t writev_kernel(int fd, const struct iovec * vec, int vlen);
ssize_t readv_kernel(int fd, const struct iovec * iov, int iovcnt);
int pipe_kernel(int pipefd[2]);

