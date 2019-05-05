# SoalShift_modul4_E05
> 05111740000052 Anggar Wahyu Nur Wibowo

>  05111740000099 Bobbi Aditya
> 
## Nomor 1

Pada soal nomor 1, kita diminta untuk mendekripsi file yang awalnya sudah di enkripsi menggunakan *caesar chiper*

### Enkripsi

```c
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
```
Kodingan di atas berguna untuk mengenkripsi nama file sesuai dengan aturan *caesar chiper*

### Dekripsi
```c
void dec(char * input)
{
	if(!strcmp(input,".") || !strcmp(input,"..")) return;

	for(int i=0;i<strlen(input);i++)
	{
		for(int j=0;j<94;j++){
			if(input[i]==key[j]){
			input[i] = key[(j+94-17)%94];
				break;
			}
		}
	}
}
```
Kodingan di atas berguna untuk mendekripsi nama file sesuai dengan aturan *caesar chiper*

### Proses LS
```c
	strcpy(path, ppath);

	enc(path);
	if(strcmp(path,"/") == 0) sprintf(fpath,"%s",path);
	else sprintf(fpath, "%s%s",dirpath, path);
	res = lstat(fpath, stbuf);
```
Agar proses ls bisa dilakukan kita perlu mengubah path yang ada dalam proses enkripsi ( pada _xmp_getattr_ dan _xmp_readdir_ )

## Nomor 2 

```c
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
```

Fungsi di atas digunakan untuk mendapatkan nama file tanpa ekstensinya, misal jika berkas tersebut memiliki ekstensi aku.mp4.001 maka akan menjadi aku.mp4

```c
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
```

Fungsi di atas digunakan untuk membandingkan dua string yang telah terenkrip, agar dibandingkan secara alfabetis.

```c
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
```

Fungsi diatas merupakan fungsi yang dipanggil ketika fuse kita *mount*. Fungsi tersebut akan melakukan satu kali `fork()` untuk proses penggabungan ekstensi file yang terpisah. Proses di atas sangat bergantung pada berkas yang terurut sehingga berkas penggabungannya juga sesuai. Kemudian, kita mencoba menulisnya ke berkas tujuan di direktori "/Videos/". Proses menulis dan membaca berkas asli dilakukan secara bersamaan. Namun, proses tersebut dilakukan satu-persatu antar berkas yang terpisah, dimulai dari yang kecil terlebih dahulu. Karena kami menggunakan `scandir()` dengan *comparator* `decalphasort()`, dapat dijamin bahwa urutan berkas akan terurut secara alfabetis.

```c
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
```

Kode di atas akan melihat keseluruhan berkas di folder "/Videos/" dan akan menghapus semua yangg didalamnya.

## Nomor 3
Pada soal nomor 3, kita diminta untuk mengecek owner name dan group name dari sebuah file pada saat sebelum dimount.
Apabila file tersebut memiliki:
- Owner Name = `chipset` atau `ic_controller`
- Group Name = `rusak`
- Tidak dapat dibaca

Maka, kita harus menyimpan nama file, group ID, owner ID, dan waktu terakhir diakses dalam file `filemiris.txt`. Kemudian menghapus file yang dianggap berbahaya tersebut.

Pengecekan owner dan group name akan dilakukan setiap kita membuka direktori pada folder yang dimount

### Mendapatkan user & groupname
```c
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
```
Untuk mengecek owner name dan group name kita menggunakan anggota dari `sys/stat.h`. Dan untuk mengecek apakah file tersebut bisa dibaca atau tidak kita mengecek mode dari file tersebut.



### Cek Owner dan Group Name & hapus
```c
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
    } else 
```
Awalnya kita akan mengecek owner name dan group name dari file tersebut serta mengecek apakah file tersebut bisa dibaca atau tidak. Kemudian jika file tersebut memenuhi kriteria, maka file tersebut akan di delete( menggunakan fungsi _remove_ ) dan kita memasukkan log(nama file, group ID, owner ID, dan waktu terakhir diakses) ke dalam file `filemiris.txt`

## Nomor 4
Pada soal nomor 4 kita diminta untuk:
- Pada saat setiap membuat folder, permissionnya diubah menjadi 750
- ketika membuat file permissionnya akan otomatis menjadi 640 dan ekstensi filenya akan bertambah ".iz1"
-  ​File berekstensi ".izi" tidak bisa diubah permissionnya dan memunculkan error bertuliskan "​File ekstensi iz1 tidak boleh diubah permissionnya."

### Ubah permission pembuatan folder
```c
if(!strncmp(path,"/YOUTUBER", 9)){  // jika di folder YOUTUBER
		printf("UYEEEE .. ");
		cmode = 0750; // ubah permissionnya jadi 750
	}
```
(pada xmp_mkdir)


Jika folder tersebut berada pada folder YOUTUBER, maka permissionnya akan diubah menggunakan chmode menjadi 750


### Pembuatan file baru
```c
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
```

Proses di atas digunakan pada saat pembuatan file baru(proses _touch_). Jika ada pembuatan file baru pada folder YOUTUBER maka akan ditambahkan `.iz1` di bagian belakang file. Kita juga perlu melakukan enkrip dekrip pada file tersebut

### Ubah Permision File
```c
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
```
Kodingan diatas berguna untuk melakukan pengecekan apakah ada nama file yang memiliki extension `.iz1.`

Jika file tersebut tidak memiliki extension `.iz1` maka akan ditampilkan kode error `"File ekstensi iz1 tidak boleh diubah permissionnya"` sesuai pada perintah soal

## Nomor 5
Pada soal nomor 5 kita diminta pada saat kita mengedit suatu file dan melakukan save, makan akan terbuat folder baru `Backup` kemudian hasil dari save tersebut akan disimpan pada backup dengan nama `namafile_[timestamp].ekstensi.

Dan ketika file asli dihapus maka akan dibuat folder bernama `RecycleBin`,  kemudian file yang dihapus beserta semua backup dari file yang dihapus tersebut (jika ada) di zip dengan nama ​`namafile_deleted_[timestamp].zip` ​dan  ditaruh kedalam folder RecycleBin (file asli dan backup terhapus). Dengan format`[timestamp]` ​adalah `​yyyy-MM-dd_HH:mm:ss`


### Pengubahan Nama File
```c
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
```
Fungsi diatas digunakan untuk mengubah nama biasa(nama berwildcard).

Fungsi ini nantinya digunakan pada saat penghapusan nama file

```c
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
```
Fungsi diatas digunakan untuk mengubah nama file menjadi filename backup agar sesuai dengan ketentuan soal yang ada.



```c
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
```
Fungsi diatas digunakan untuk mengubah nama file menjadi filename deleted agar sesuai dengan ketentuan soal yang ada

### Encrypt spesial
```
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
				if(key[(j+17)%94] == '`'){
					input[i] = '*';
				} else {
					input[i] = key[(j+17)%94];
				}

				break;
			}
		}
	}
}
```
Jika nama file \` tergantikan maka kita perlu mengubah namanya. Ini dibutuhkan saat kita memanggil wildcard(`*`) nantinya.

### Pembuatan Folder
```c
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

	int doBackup = 1;

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
```
Kodingan diatas berguna untuk pembuatan folder baru(backup). Kita perlu mengecek apakah yang kita backup bukan file temporary seperti(`.swx, .swp, .swo`)

Kemudian kita akan membuat file sesuai dengan ketentuan backup yang ada, dan mencopy isi dari file tersebut ke dalam backup file.

### Proses Delete
```c
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

	int doBackup = 1;

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
```

Untuk melakukan proses delete, awalnya kita perlu membuat folder recycle bin.

Kita perlu mengecek apakah yang kita backup bukan file temporary seperti(`.swx, .swp, .swo`)

Kemudian ktia harus mengecek namafile, path file tersebut dan mengenkripsinya juga.

Dalam proses delete kita akan membuat child process. Kita harus melakukan zip terlebih dahulu sebelum melakukan deletion.






