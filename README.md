# SoalShift_modul4_E05
## Nomor 3
Pada soal nomor 3, kita diminta untuk mengecek owner name dan group name dari sebuah file pada saat sebelum dimount.
Apabila file tersebut memiliki:
- Owner Name = `chipset` atau `ic_controller`
- Group Name = `rusak`
- Tidak dapat dibaca

Maka, kita harus menyimpan nama file, group ID, owner ID, dan waktu terakhir diakses dalam file `filemiris.txt`. Kemudian menghapus file yang dianggap berbahaya tersebut.

Pengecekan owner dan group name akan dilakukan setiap kita membuka direktori pada folder yang dimount

### Cek Owner dan Group Name
```c
kodingannya belum
kodingannya belum
kodingannya belum
kodingannya belum
kodingannya belum
kodingannya belum
kodingannya belum
```
Awalnya kita akan mengecek owner name dan group name dari file tersebut serta mengecek apakah file tersebut bisa dibaca atau tidak. Kemudian jika file tersebut memenuhi kriteria, maka file tersebut akan di delete dan kita memasukkan log(nama file, group ID, owner ID, dan waktu terakhir diakses) ke dalam file `filemiris.txt`

Untuk mengecek owner name dan group name kita menggunakan anggota dari `sys/stat.h`. Dan untuk mengecek apakah file tersebut bisa dibaca atau tidak kita mengecek mode dari file tersebut.

## Nomor 1

Pada soal nomor 1, kita diminta untuk mendekripsi file yang awalnya sudah di enkripsi menggunakan *caesar chiper*

### Enkripsi

```c
void enc(char* yangdienc)
{   
	if(!strcmp(yangdienc,".") || !strcmp(yangdienc,"..")) return;
	for(int i=0;i<strlen(yangdienc);i++)
	{
		for(int j=0;j<94;j++){
			// printf("%c",key[j]);
			if(yangdienc[i]==key[j]){
				yangdienc[i] = key[(j+17)%94];
				break;
			}
		}
	}
}
```
Kodingan di atas berguna untuk mengenkripsi nama file sesuai dengan aturan *caesar chiper*

### Dekripsi
```c
void dec(char* yangdidec)
{
	if(!strcmp(yangdidec,".") || !strcmp(yangdidec,"..")) return;
	for(int i=0;i<strlen(yangdidec);i++)
	{
		for(int j=0;j<94;j++){
			// printf("%c",key[j]);
			if(yangdidec[i]==key[j]){
				yangdidec[i] = key[(j+77)%94];
				break;
			}
		}
	}
}
```
Kodingan di atas berguna untuk mendekripsi nama file sesuai dengan aturan *caesar chiper*