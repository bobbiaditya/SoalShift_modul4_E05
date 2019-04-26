///////xmp lib https://github.com/rfjakob/fuse/blob/master/example/fusexmp_fh.c

#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>

static char *dirpath = "/home/anggar/shift4";

char key[97] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";

regex_t regex;
int reti;

void enc(char * input)
{
	if(!strcmp(input,".") || !strcmp(input,"..")) return;

	for(int i=0;i<strlen(input);i++)
	{
		for(int j=0;j<94;j++){
			if(input[i]==key[j]){
				input[i] = key[(j+17)%94];
				break;
			}
		}
	}
}

void dec(char * input)
{
	if(!strcmp(input,".") || !strcmp(input,"..")) return;

	for(int i=0;i<strlen(input);i++)
	{
		for(int j=0;j<94;j++){
			if(input[i]==key[j]){
				input[i] = key[(j+77)%94];
				break;
			}
		}
	}
}

static void* pre_init(struct fuse_conn_info *conn){
	// Create Videos folder in main
	char folder[40] = "/Videos";
	enc(folder);
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, folder);
	mkdir(fpath, 0775);

	(void) conn;
	return NULL;
}

static const int xmp_getattr(const char *ppath, struct stat *stbuf)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);

	enc(path);
	if(strcmp(path,"/") == 0) sprintf(fpath,"%s",path);
	else sprintf(fpath, "%s%s",dirpath, path);
	res = lstat(fpath, stbuf);

	// menangani waktu touch
	if (res == -1){
		dec(path);
		if(!strncmp(path, "/YOUTUBER", 9)){
			dec(fpath);
			strcat(fpath, ".iz1");
			enc(fpath);

			if(lstat(fpath, stbuf) != -1){
				return 0;
			}
		}

		return -errno;
	}

	return 0;
}

static int xmp_readdir(const char *ppath, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		char yeoreum[1000];
		char meureum[1000];
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		strcpy(yeoreum,de->d_name);
		// untuk filepath aslinya
		sprintf(meureum, "%s/%s", fpath, yeoreum);
		dec(yeoreum);

		// dapatkan user berkas aslinya
		struct stat tmstat;
		stat(meureum, &tmstat);
		struct passwd *ireum = getpwuid(tmstat.st_uid);
		struct group *gruppe = getgrgid(tmstat.st_gid);

		int ireumDiffA = strcmp(ireum->pw_name, "chipset");
		int ireumDiffB = strcmp(ireum->pw_name, "ic_controller");
		int gruppeDiff = strcmp(gruppe->gr_name, "rusak");

		FILE* fp;
		char miris[1000], timestr[1000], writebuf[1000], miramar[1000];

		// printf(":: %d %d %s", ireum->pw_uid, ireumDiffA, ireum->pw_name);

		if((!ireumDiffA || !ireumDiffB) && 
			!gruppeDiff && 
			!(tmstat.st_mode & 0444)) { // Gak bisa dibaca
			strcpy(miris, dirpath);
			strcpy(miramar, "/filemiris.txt"); // diroot
			enc(miramar);
			strcat(miris, miramar);

			fp = fopen(miris, "a+"); // TODO: ganti a aja

			// Untuk dapat tanggal
			// need improvement for atime
			strftime(timestr, 40, "%y%m%d (%H:%M:%S)", localtime(&tmstat.st_atime));
			fprintf(fp, "%s\t%d:%d\t%s\t%s\n", timestr, ireum->pw_uid, gruppe->gr_gid, path, yeoreum);

			remove(meureum);

			fclose(fp);
		} else {
			if (filler(buf, yeoreum, &st, 0))
				break;
		}
	}

	closedir(dp);
	return 0;
}

static int xmp_flush(const char *path, struct fuse_file_info *fi)
{
	int res;

	(void) path;
	/* This is called from every close on an open file, so call the
	   close on the underlying filesystem.	But since flush may be
	   called multiple times for an open file, this must not really
	   close the file.  This is important if used on a network
	   filesystem like NFS which flush the data/metadata on close() */
	res = close(dup(fi->fh));
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_mknod(const char *ppath, mode_t mode, dev_t rdev)
{
	int res;

	char path[1000];
	char fpath[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

// Asumsi untuk folder YOUTUBER ada di root path
static int xmp_mkdir(const char *ppath, mode_t mode)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);

	// buat folder youtuber
	mode_t cmode = mode;
	if(!strncmp(path,"/YOUTUBER", 9)){
		printf("UYEEEE .. ");
		cmode = 0750;
	}

	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	res = mkdir(fpath, cmode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *ppath)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *ppath)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(char *from, char *to)
{
	int res;
	char new_from[1000];
	char new_to[1000];
	enc(from);
	enc(to);
	sprintf(new_from,"%s%s",dirpath,from);
	sprintf(new_to,"%s%s",dirpath,to);
	res = rename(new_from, new_to);
	if (res == -1)
		return -errno;

	return 0;
}

// IZ*ONE
static int xmp_chmod(const char *ppath, mode_t mode)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);

	int pathDiff = strncmp(path, "/YOUTUBER", 9);

	reti = regcomp(&regex, ".*\\.iz1$", 0);

	reti = regexec(&regex, path, 0, NULL, 0);

	enc(path);

	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	struct stat stbuf;
	stat(fpath, &stbuf);

	if(!reti && S_ISREG(stbuf.st_mode)) { // MATCH
		perror("File ekstensi iz1 tidak boleh diubah permissionnya");

		return -errno;
	}

	res = chmod(fpath, mode);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *ppath, uid_t uid, gid_t gid)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *ppath, off_t size)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *ppath, const struct timespec ts[2])
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1){
		// menangani waktu touch
		dec(path);
		if(!strncmp(path, "/YOUTUBER", 9)){
			dec(fpath);
			strcat(fpath, ".iz1");
			enc(fpath);

			if(utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW)!= -1){
				return 0;
			}
		}

		return -errno;
	}

	return 0;
}

static int xmp_open(const char *ppath, struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *ppath, char *buf, size_t size, off_t offset,
	struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *ppath, char *buf, size_t size,
	off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);
	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_create(const char *ppath, mode_t mode, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char path[1000];
	char bpath[1000];
	char strtime[1000];
	char zipname[1000];
	char bkpath[1000];
	mode_t cmode = mode;
	strcpy(path, ppath);
	// enc(path);

	// folder youtuber
	if(!strncmp(path, "/YOUTUBER", 9)){
		strcat(path, ".iz1");
		cmode = 0640;
	}

	// Ketika akan membuat file swap
	// reti = regcomp(&regex, ".*\\.swp$", 0);
	// reti = regexec(&regex, path, 0, NULL, 0);

	strcpy(bpath, path);
	char spp* = strstr(bpath, ".swp");

//	if(spp) {
//		char bkF = "/Backup";
//		char rbF = "/RecycleBin";
//
//		strcpy(spp, "");
//
//		time_t n = time(NULL);
//		strftime(strtime, 40, "_%Y-%m-%d_%H:%M:%S", localtime(&now));

//		sprintf(bkpath, "%s/Backup/%s%s", dirpath, bpath, strtime);

		// Jika dihapus
		// sprintf(zipname, "%s/RecycleBin/%s_deleted_%s.zip", dirpath, bpath, strtime);
///
//		pid_t erzi1, erzi2;
//		erzi1 = fork();
//
//		if(erzi1 == 0) {
//			enc(path);
//			sprintf(fpath, "%s%s", dirpath, path);
//		}
//
//	}


	enc(path);
	sprintf(fpath, "%s%s", dirpath, path);

	res = creat(fpath, cmode);

	// fd = open(path, fi->flags, mode);
	// if (fd == -1)
	// 	return -errno;

	// fi->fh = fd;

	// Waktu dia akan create file

	// retina = regcomp(&xeger, ".*([^/]*\\.swp)$");
	// size_t maxGroups = 3;
	// regmatch_t groupArrays[maxGroups];

	// dec(fpath);

	// retina = regexec(&xeger, fpath, maxGroups, groupArrays, 0);

	// if(!reti){ // MATCH

	// }

	if (res == -1) return -errno;

	close(res);

	return 0;
}


static struct fuse_operations xmp_oper = {
	.init		= pre_init,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.chmod		= xmp_chmod,
	.chown 		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.flush 		= xmp_flush,
	.create 	= xmp_create
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
