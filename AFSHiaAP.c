///////xmp lib https://github.com/rfjakob/fuse/blob/master/example/fusexmp_fh.c

#define FUSE_USE_VERSION 28
#include <sys/wait.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define BUFFSIZE 4096

static char *dirpath = "/home/anggar/shift4";

char key[97] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";

regex_t regex;
int reti;

// name -> allname
// no 5 (ngubah nama biasa -> nama berwildcard)
// dipake buat hapus
int allname(char * ppath){
	char *ext = strrchr(ppath, '.'); 

	char fnext[1000];

	if(ext){
		memset(fnext, '\0', strlen(ppath)-strlen(ext)+1);
		strncpy(fnext, ppath, strlen(ppath)-strlen(ext));
	} else {
		strcpy(fnext, ppath);
	}

	char res[1000];

	memset(res, '\0', strlen(ppath)+40);
	sprintf(res, "/Backup/%s", fnext);

	strcpy(ppath, res);
}

// name -> bkname
// no 5, ngubah filename biasa
// menjadi filename backup
// (yang ada timestampnya)
int bkname(char * ppath){
	char *ext = strrchr(ppath, '.'); 

	char fnext[1000];

	if(ext){
		memset(fnext, '\0', strlen(ppath)-strlen(ext)+1);
		strncpy(fnext, ppath, strlen(ppath)-strlen(ext));
	} else {
		strcpy(fnext, ppath);
	}

	time_t now = time(NULL);
	char strnow[1000];
	strftime(strnow, 40, "%Y-%m-%d_%H:%M:%S", localtime(&now));

	char res[1000];

	memset(res, '\0', strlen(ppath)+40);
	sprintf(res, "%s_%s%s", fnext, strnow, ext);

	strcpy(ppath, res);
}

// name -> dlname
// no 5, ngubah filename biasa
// menjadi filename deleted
// (yang ada deleted_timestampnya)
int dlname(char * ppath){
	char *ext = strrchr(ppath, '.'); 

	char fnext[1000];

	if(ext){
		memset(fnext, '\0', strlen(ppath)-strlen(ext)+1);
		strncpy(fnext, ppath, strlen(ppath)-strlen(ext)); // 20 = len(timestamp)
	} else {
		strcpy(fnext, ppath);
	}

	time_t now = time(NULL);
	char strnow[1000];
	strftime(strnow, 40, "%Y-%m-%d_%H:%M:%S", localtime(&now));

	char res[1000];

	memset(res, '\0', strlen(ppath)+40);
	sprintf(res, "%s_deleted_%s.zip", fnext, strnow);

	strcpy(ppath, res);

}

// fullpath -> curfname
// fullpath /home/anggar/.../filename.txt
// -> filename.txt aja
int gname(char * ppath){
	char *parpath = strrchr(ppath, '/');
	char fnext[1000];

	if(parpath){
		strcpy(fnext, parpath+1);
	} else {
		strcpy(fnext, ppath);
	}

	strcpy(ppath, fnext);
}

// fullpath -> parpname
// fullpath /home/anggar/.../filename.txt
// -> parent path aja
// /home/anggar/.../
int pname(char * ppath){
	char *parpath = strrchr(ppath, '/');
	char fnext[1000];

	if(*ppath == *parpath){
		strcpy(ppath, "/");
		return 0;
	} else if(parpath){
		memset(fnext, '\0', strlen(ppath)-strlen(parpath)+1);
		strncpy(fnext, ppath, strlen(ppath)-strlen(parpath)+1); // include the '/' tail
	} else {
		strcpy(fnext, ppath);
	}

	strcpy(ppath, fnext);
}

// encrypt special
// jika ` tergantikan *
// untuk wildcard
// no 5
void encs(char * input)
{
	if(!strcmp(input,".") || !strcmp(input,"..")) return;

	for(int i=0;i<strlen(input);i++)
	{
		if(input[i]=='*') continue;
		for(int j=0;j<94;j++){
			if(input[i]==key[j]){
				if(key[(j+31)%94] == '`'){
					input[i] = '*';
				} else {
					input[i] = key[(j+31)%94];
				}

				break;
			}
		}
	}
}

void enc(char * input)
{
	if(!strcmp(input,".") || !strcmp(input,"..")) return;

	for(int i=0;i<strlen(input);i++)
	{
		for(int j=0;j<94;j++){
			if(input[i]==key[j]){
				input[i] = key[(j+31)%94];
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
			input[i] = key[(j+94-31)%94];
				break;
			}
		}
	}
}

// untuk mendapat nama tanpa ekstensi
int namenoext(char * ppath){
	char *ext = strrchr(ppath, '.'); 

	char fnext[1000];

	if(ext){
		memset(fnext, '\0', strlen(ppath)-strlen(ext)+1);
		strncpy(fnext, ppath, strlen(ppath)-strlen(ext));
	} else {
		strcpy(fnext, ppath);
	}

	char res[1000];

	memset(res, '\0', strlen(ppath)+40);
	sprintf(res, "%s", fnext);

	strcpy(ppath, res);
}

// untuk compare string
int decalphasort(const struct dirent **a, const struct dirent **b){
	char cstr1[1000];
	char cstr2[1000];

	strcpy(cstr1, (*a)->d_name);
	strcpy(cstr2, (*b)->d_name);

	dec(cstr1);
	dec(cstr2);

	int ret = strcoll(cstr1, cstr2);

	return ret;
}

// waktu mau ngemount
static void* pre_init(struct fuse_conn_info *conn){
	// Create Videos folder in main
	char folder[40] = "/Videos";
	enc(folder);
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, folder);
	mkdir(fpath, 0775);

	DIR *dp;
	pid_t child;
	int i, n, k;

	char combinename[1000];
	char filename[1000];
	char pathasli[1000];
	char dname_dec[1000];
	char cideos[] = "/Videos/";
	enc(cideos);
	sprintf(combinename, "%s%s", dirpath, cideos);

	// enc(combinename);

	dp = opendir(dirpath);

	if (dp == NULL) return -errno;

	// regex untuk menyocokan berkas yang .001 .002 dst
	reti = regcomp(&regex, ".*\\.[0-9][0-9][0-9]", 0);

	child = fork();

	// child process  buat join vid
	if(child == 0) { // child
		int fp1, fp2;
		char buf[BUFFSIZE];
		struct dirent **dlist;

		// dengan scandir dan komparator kustom
		// decalphasort, dipastikan file
		// akan berurutan
		n = scandir(dirpath, &dlist, 0, decalphasort);
		
		for(i=0; i<n; i++) {
			// dec(combinename);
			//jika file regex sesuai dengan yang diinginkan

			strcpy(dname_dec, dlist[i]->d_name);
			dec(dname_dec);

			// pencocokan berkas dengan regex
			reti = regexec(&regex, dname_dec, 0, NULL, 0);

			if(!reti){ 
				strcpy(filename, dname_dec);
				sprintf(pathasli, "%s/%s", dirpath, dlist[i]->d_name);
				namenoext(filename);
				enc(filename);
				strcat(combinename, filename);
				dec(filename);

				// file yang akan dibaca
				fp1 = open(pathasli, O_RDONLY);

				printf("\nFILENAME: %s, %s\n", combinename, pathasli);

				// could be segfault either because the file
				// directed is folder, or the parent directory
				// doesn't exist
				// fpath file yang akan digabungkan
				// pake mode append
				fp2 = open(combinename, O_WRONLY | O_APPEND | O_CREAT, 0775);

				// pindah kursor ke belakang dulu
				lseek(fp2, 0, SEEK_END);

				// baru append, perlahan demi perlahan
				// sampai isi dari fp1 habis
				while((k = read(fp1, buf, BUFFSIZE)) > 0){
					write(fp2, buf, k);
				}

				// tutup semuanya
				close(fp1);
				close(fp2);
			}

			printf("\nCBNAME: %s, %s\n", combinename, dname_dec);

			sprintf(combinename, "%s%s", dirpath, cideos);
			// enc(combinename);
		}

		free(dlist);
		// execl("/bin/sh", "/bin/sh", "-c", "echo", "haha", NULL);
		exit(0);
	}

	(void) conn;
	return NULL;
}

// no 1 dan nomor 4
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
		// ngecek parent folder harus
		// YOUTUBER
		if(!strncmp(path, "/YOUTUBER", 9)){
			dec(fpath);
			// nambah filename + .iz1
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

// ls
// buat nomor 1 dan 3
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
		// dapetin user & groupname (3)
		struct passwd *ireum = getpwuid(tmstat.st_uid);
		struct group *gruppe = getgrgid(tmstat.st_gid);

		// (3) cek perbedaan user & group
		int ireumDiffA = strcmp(ireum->pw_name, "chipset");
		int ireumDiffB = strcmp(ireum->pw_name, "ic_controller");
		int gruppeDiff = strcmp(gruppe->gr_name, "rusak");

		FILE* fp;
		char miris[1000], timestr[1000], writebuf[1000], miramar[1000];

		// printf(":: %d %d %s", ireum->pw_uid, ireumDiffA, ireum->pw_name);

		// (3) jika metadata berkas memenuhi prasyarat
		// masuk ke if
		if((!ireumDiffA || !ireumDiffB) && 
			!gruppeDiff && 
			!(tmstat.st_mode & 0444)) { // Gak bisa dibaca (0444)
			strcpy(miris, dirpath);
			strcpy(miramar, "/filemiris.txt"); // diroot
			enc(miramar);
			strcat(miris, miramar);

			fp = fopen(miris, "a+"); // TODO: ganti a aja

			// Untuk dapat tanggal
			strftime(timestr, 40, "%y%m%d (%H:%M:%S)", localtime(&tmstat.st_atime));
			fprintf(fp, "%s\t%d:%d\t%s\t%s\n", timestr, ireum->pw_uid, gruppe->gr_gid, path, yeoreum);

			// ngehapus file bahaya tersebut
			remove(meureum);

			fclose(fp);
		} else {
			// ngelist biasa
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

	// (4) cmode buat modifikasi mode
	mode_t cmode = mode; // mode default
	if(!strncmp(path,"/YOUTUBER", 9)){  // jika di folder YOUTUBER
		printf("UYEEEE .. ");
		cmode = 0750; // ubah permissionnya jadi 750
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

// waktu mau hapus, perlu fname sama ekstension
// dipisah
// nomor (5)
static int xmp_unlink(const char *ppath)
{
	int res;
	char fpath[1000];
	char path[1000];
	char filname[1000];
	char delname[1000];
	char ballname[1000];
	char ext[100];

	// buat nama folder di RecycleBin
	char folder[1000];
	sprintf(folder, "/RecycleBin");
	enc(folder);
	char ffolder[1000];
	sprintf(ffolder, "%s%s", dirpath, folder);
	mkdir(ffolder, 0775);

	// buat ngefork
	pid_t child1, child2;
	int status;

	int doBackup = 0;

	// jika bukan file temporary kaya .swp, .swx
	// .swo, dan .ekstensi~ (tilde)
	if(strstr(ppath, ".sw") != NULL) doBackup = 0;
	if(strstr(ppath, "~") != NULL) doBackup = 0;

	strcpy(path, ppath);
	strcpy(filname, ppath);
	strcpy(ext, ppath);

	// dapetin filenamenya dari full path
	gname(filname);

	strcpy(delname, "/RecycleBin/");
	strcat(delname, filname);
	// dapetin format file untuk file
	// yang akan dihapus
	dlname(delname);

	strcpy(ballname, filname);
	// dapatin file buat wildcard *
	allname(ballname);

	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	enc(ballname);
	enc(delname);
	// postfix _e untuk encrypted filename + dirpath
	char ballname_e[1000];
	char delname_e[1000];
	sprintf(ballname_e, "%s%s", dirpath, ballname);
	sprintf(delname_e, "%s%s", dirpath, delname);

	// buat child process untuk execute proses
	// buat ngezip sama ngehapus
	child1 = fork();

	// nyimpan command buat ngezip sama hapus
	// backup (5)
	char cmd[1000];
	sprintf(cmd, "zip '%s' '%s' '%s'* && rm -f '%s'*", delname_e, fpath, ballname_e, ballname_e);

	if(doBackup && (child1 == 0)) { // child
		// buat execute command
		execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
	}

	// nunggu child selesai dulu
	while((wait(&status)) > 0);

	// baru hapus
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
// (5) buat ngubah permission
static int xmp_chmod(const char *ppath, mode_t mode)
{
	int res;
	char fpath[1000];
	char path[1000];
	strcpy(path, ppath);

	// nentuin di folder YOUTUBER apa nggak
	int pathDiff = strncmp(path, "/YOUTUBER", 9);

	// compare filename dengan regex .iz1
	// apakah memiliki ekstensi iz1
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

	// jika file biasa (bukan folder)
	// sama math patternnya (file .iz1)
	if(!reti && S_ISREG(stbuf.st_mode)) { // MATCH
		// kasih pesan error
		perror("File ekstensi iz1 tidak boleh diubah permissionnya");

		// langsung balik
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

// touch kan sebenernya ngubah waktu modifikasinya
// kalau gak salah, jadi doi bakal ngecall fungsi ini
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
		// (4) jika di folder YOUTUBER
		if(!strncmp(path, "/YOUTUBER", 9)){
			dec(fpath); // dekrip bentar ...
			strcat(fpath, ".iz1"); // tambah .iz1
			enc(fpath); // .. baru enkrip lagi

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

// no (4), waktu ngesave
static int xmp_write(const char *ppath, char *buf, size_t size,
	off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char path[1000];
	char bkpath[1000];
	char bkpath_e[1000];
	char parname[1000];
	char filname[1000];
	strcpy(path, ppath);
	strcpy(parname, ppath);
	strcpy(filname, ppath);
	pid_t child1;

	// buat ngecreate folder
	char folder[1000];
	sprintf(folder, "/Backup");
	enc(folder);
	char ffolder[1000];
	sprintf(ffolder, "%s%s", dirpath, folder);
	mkdir(ffolder, 0775); // ini nih waktu buat foldernya

	gname(filname); // versi non-timestamped
	pname(parname); // dapetin parent path-nya

	// karena filename tidak mengandung /
	strcpy(bkpath, "/Backup/");
	strcat(bkpath, filname);

	int doBackup = 0;

	// pastikan bukan file temporary
	// kaya .swx, .swp, .swo
	if(strstr(ppath, ".sw") != NULL) doBackup = 0;

	enc(path);
	if(strcmp(path,"/") == 0)
	{
		strcpy(path,dirpath);;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	printf("BACKUP?: %d, %s, %s\n", doBackup, fpath, ppath);

	if(doBackup){
		// buat ngegenerate format backupnya
		bkname(bkpath);
		enc(bkpath);
		sprintf(bkpath_e, "%s%s", dirpath, bkpath);
		printf("%s\n", bkpath_e);

		child1 = fork();

		// langsung copy aja filenya
		// gak usah pake fwrite
		char *argv[4] = {"cp", fpath, bkpath_e, NULL};
		printf("WILL BE COPIED: %s %s\n", fpath, bkpath_e);
		if(child1 == 0){
			execv("/bin/cp", argv);
		}
	}

	// gak penting buat debug aja, yang atas juga	
	printf("BACKUP?: %d, %s, %s\n", doBackup, fpath, ppath);

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

	if(!strncmp(path, "/YOUTUBER", 9)){
		strcat(path, ".iz1");
		cmode = 0640;
	}

	strcpy(bpath, path);

	enc(path);
	sprintf(fpath, "%s%s", dirpath, path);

	res = creat(fpath, cmode);

	if (res == -1) return -errno;

	close(res);

	return 0;
}

// Ketika akan unmount memanggil fungsi ini
void destroy(void* private_data){
	DIR *dp;
	struct dirent *de;

	char cideos[1000] = "/Videos/";
	char vidpath[1000];
	char filefullpath[1000];
	enc(cideos);
	sprintf(vidpath, "%s%s", dirpath, cideos);

	dp = opendir(vidpath);

	// Membaca seluruh isi dari Videos
	while((de = readdir(dp)) != NULL) {
		sprintf(filefullpath, "%s%s", vidpath, de->d_name);
		// lalu menghapusnya
		remove(filefullpath);
	}

	printf("\n\nTERLEPAS AKU\n\n");

	closedir(dp);
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
	.create 	= xmp_create,
	.destroy	= destroy
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

// 1 -> readdir, getattr
// 2 -> pre_init, destroy
//   -> sama fungsi utilitas buat komparasi
//      string yang telah dienkrip (decalphasort)
// 3 -> readdir (yang if banyak tadi)
// 4 -> mkdir, utimens (waktu buat file), chmod
// 5 -> fungsi utilitas *name (buat ngubah path)
//   -> write, unlink (hapus)