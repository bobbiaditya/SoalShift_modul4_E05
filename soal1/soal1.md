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