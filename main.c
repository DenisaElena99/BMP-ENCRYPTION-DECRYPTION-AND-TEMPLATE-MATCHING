#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

typedef struct ///structura pentru reprezentarea primilor 3 octeti din reprezentarea fiecarui pixel
{
    unsigned char R,G,B;///canale de culoare: red, green, blue
}pixel;

pixel XorPixeli(pixel A,pixel B)
{
    ///functie ce realizeaza operatia XOR intre doi pixeli si se aplica pe fiecare canal de culoare
    pixel C;
    C.R=A.R^B.R;
    C.B=A.B^B.B;
    C.G=A.G^B.G;
    return C;
}

pixel XorInt(pixel A,unsigned int x)///functie ce realizeaza operatia XOR intre
{                                   ///un pixel si un nr intreg
    pixel C;
    C.R = A.R ^ ((x >> 16) & 255);///caut octeti din intreg ce se xoreaza
    C.G = A.G ^ ((x >> 8) & 255);///cu fiecare canal de culoare
    C.B = A.B ^ (x & 255);
    return C;
}
///cerinta 1
unsigned int Xorshift32(unsigned int x)
{
    x=x ^ x << 13;
    x=x ^ x >> 17;
    x=x ^ x <<5;
    return x;
}

void cerinta2(char* nume_fisier_sursa,pixel **P, int *latime, int *inaltime,unsigned char **header)
{
    FILE *fin;
    int i=0,j,line, dimensiune;
    fin=fopen(nume_fisier_sursa,"rb");

    if(fin==NULL)
    {
        printf("Nu am gasit imagine de stocat in memorie");
        return;
    }
    *header=(unsigned char*)malloc(54*sizeof(unsigned char));
    fread(*header,1,54,fin);///citire header
    fseek(fin,18,SEEK_SET);///ma deplasez cu 18 pozitii inapoi in fisierul binar
    fread(latime,4,1,fin);///citire latime
    fread(inaltime,4,1,fin);///citire inaltime
    dimensiune=(*latime)*(*inaltime);
    *P=(pixel*)malloc(dimensiune *sizeof( pixel));
    fseek(fin, 54, SEEK_SET);///trecere peste header
     line=*inaltime-1;
    ///citire vectori pixeli
    for(i=0;  i<*inaltime; i++)
    {
        for(j=0;j<*latime;j++)
            fread((*P)+line*(*latime)+j, sizeof(pixel),1,fin);
        line--;
    }

    free(header);free(P);///eliberare memorie
    fclose(fin);

}

void cerinta3(char *imagine_out,pixel *P,int latime,int inaltime,unsigned char *head)
{
    ///salvare in memoria externa a unei imagini BMP stocata in forma liniarizata in
    ///memoria interna
    int i,j;

    FILE *fout=fopen(imagine_out,"wb");
    if(fout==NULL)
    {
        printf("Eroare la crearea fisierului de scriere");
        return;
    }
    fwrite(head,54,1,fout);
    ///punerea pixelilor din memorie in fisier
    int line=inaltime-1;
    for(i=0;  i<inaltime; i++)
    {
        for(j=0;j<latime;j++)
            fwrite(P+line*latime+j, sizeof(pixel),1,fout);
        line--;
    }

    fclose(fout);
}


void cerinta4(char *de_criptat,char *dest,char *secretkey)
{

    unsigned char *header;
    pixel *p,*pp,*c;unsigned int i;
    int latime,inaltime,dimensiune;
    ///incarc imaginea in memoria interna
    cerinta2(de_criptat,&p,&latime,&inaltime,&header);
    unsigned int* R; unsigned int *per;
    unsigned int starting;

    R=(unsigned int*)malloc(2*latime*inaltime*sizeof(unsigned int));
    per=(unsigned int*)malloc(latime*inaltime*sizeof(unsigned int));
    pp=(pixel*)malloc(latime*inaltime*sizeof(pixel));
    c=(pixel*)malloc(latime*inaltime*sizeof(pixel));
    FILE *fin=fopen(secretkey,"r");
    fscanf(fin,"%u %u",&R[0],&starting);
    ///generare numere pseudo-aleatoare
    for( i=1;i<2*latime*inaltime;i++)
        R[i]=Xorshift32(R[i-1]);
    ///initializare permutare
    for( i=0;i<latime*inaltime;i++)
    per[i]=latime*inaltime-i-1;

    unsigned int number,aux;
    /// algoritmul lui Durstenfeld
    for( i=latime*inaltime-1;i>=1;i--)
    {
        number=R[latime*inaltime-i]%(i+1);
        aux=per[i];
        per[i]=per[number];
        per[number]=aux;
    }

    for( i=0;i<latime*inaltime;i++)
    pp[per[i]]=p[i];

    c[0]=XorInt(XorInt(pp[0],R[latime*inaltime]),starting);

    for( i=1;i<latime*inaltime;i++)
        c[i]=XorPixeli(c[i-1],XorInt(pp[i],R[latime*inaltime+i]));

    ///salvam imaginea cifrata in memoria externa
    cerinta3(dest,c,latime,inaltime,header);
    ///eliberare memorie alocata
    free(header);free(pp);free(c);free(p);free(R);free(per);
    fclose(fin);


}

void cerinta5(char *criptata,char *decriptata,char *secretkey)
{
    unsigned char *header;
    pixel *p,*pp,*c;unsigned int i;
    unsigned int *R,*per,*inv;
    int latime,inaltime,dimensiune;
    ///incarc imaginea cifrata in memoria interna
    cerinta2(criptata,&p,&latime,&inaltime,&header);

    FILE *fin,*fout;
    fout=fopen(decriptata,"wb");
    fin=fopen(secretkey,"r");


    unsigned int number,aux,starting;
    R=(unsigned int*)malloc(2*latime*inaltime*sizeof(unsigned int));
    per=(unsigned int*)malloc(latime*inaltime*sizeof(unsigned int));
    inv=(unsigned int*)malloc(latime*inaltime*sizeof(unsigned int));
    pp=(pixel*)malloc(latime*inaltime*sizeof(pixel));
    c=(pixel*)malloc(latime*inaltime*sizeof(pixel));

    fscanf(fin,"%u %u",&R[0],&starting);
    ///determinare numere pseudo-aleatoare
    for( i=1;i<2*latime*inaltime;i++)
        R[i]=Xorshift32(R[i-1]);
    ///initializare permutare
    for( i=0;i<latime*inaltime;i++)
    per[i]=latime*inaltime-i-1;
    ///algoritmul lui Durstenfeld
    for( i=latime*inaltime-1;i>=1;i--)
    {
        number=R[latime*inaltime-i]%(i+1);
        aux=per[i];
        per[i]=per[number];
        per[number]=aux;
    }
    ///determin inversa permutarii
    for(i=0;i<latime*inaltime;i++)
    inv[per[i]]=i;

    c[0]=XorInt(XorInt(p[0],starting),R[latime*inaltime]);
    ///aplicam formula
    for(i=1;i<=latime*inaltime-1;i++)
        c[i]=XorInt(XorPixeli(p[i-1],p[i]),R[latime*inaltime+i]);

    for(i=0;i<latime*inaltime;i++)
        pp[inv[i]]=c[i];

    cerinta3(decriptata,pp,latime,inaltime,header);

    free(header);free(p);free(pp);free(c);free(R);free(per);free(inv);
    fclose(fin);fclose(fout);
}


double frecv(int value, int canal ,pixel *p,unsigned int dim)
{   ///parcurg toți pixelii, determinând câți pixeli
    ///au pe canalul de culoare codificat de color
    unsigned int i;
    double frecventa=0;
    if(canal==0)
            {for(i=0;i<dim;i++)
                if(p[i].R==value) frecventa++;
            }
    else if(canal==1)
            {for(i=0;i<dim;i++)
                if(p[i].G==value) frecventa++;
            }
    else if(canal==2)
            {for(i=0;i<dim;i++)
                if(p[i].B==value) frecventa++;
            }
    return frecventa;
}


void Test(char *file)
{
    unsigned char *header;
    pixel *p;
    int latime,inaltime,dimensiune;
    cerinta2(file,&p,&latime,&inaltime,&header);
    unsigned int i,dim=latime*inaltime;
    double sum1=0,sum2=0,sum3=0,fbarat=latime*inaltime/256;

    for(i=0;i<256;i++)
    {
        sum1+=(frecv(i,0,p,dim)-fbarat) * (frecv(i,0,p,dim)-fbarat) /fbarat;
        sum2+=(frecv(i,1,p,dim)-fbarat) * (frecv(i,1,p,dim)-fbarat) /fbarat;
        sum3+=(frecv(i,2,p,dim)-fbarat) * (frecv(i,2,p,dim)-fbarat) /fbarat;

    }

    printf("%.2f %.2f %.2f",sum1,sum2,sum3);
    free(p);free(header);
}


///PARTEA A II-A
typedef struct {
    int inaltime, latime;
    int** img;
} Imagine;

typedef struct {
    int lin, col;
} fereastra;

typedef struct {
    fereastra f;
    double corelatie;
} detectie;

typedef struct {
    detectie* v;
    int nrDetectii;
} detectii;

void grayscale_image(char* nume_fisier_sursa, char* nume_fisier_destinatie)
{

   ///transformarea in grayscale
   FILE *fin, *fout;
   unsigned int dim_img, latime_img, inaltime_img;
   unsigned char pRGB[3], aux;

   printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);

   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 2, SEEK_SET);
   fread(&dim_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

   fseek(fin, 18, SEEK_SET);
   fread(&latime_img, sizeof(unsigned int), 1, fin);
   fread(&inaltime_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

   //copiaza octet cu octet imaginea initiala in cea noua
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);

	//calculam padding-ul pentru o linie
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    printf("padding = %d \n",padding);

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//citesc culorile pixelului
			fread(pRGB, 3, 1, fout);
			//fac conversia in pixel gri
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

double calcMedie(const Imagine img, double *medie, double *deviatie) {
    *medie = *deviatie = 0;

	int i,j;
	for(i = 0; i < img.inaltime; i++)
		for(j = 0; j < img.latime; j++)
            *medie += img.img[i][j];

	*medie /= img.inaltime * img.latime;

	for(i = 0; i < img.inaltime; i++)
		for(j = 0; j < img.latime; j++)
            *deviatie += (img.img[i][j] - *medie) * (img.img[i][j] - *medie);

	*deviatie /= (img.inaltime * img.latime - 1);
	*deviatie = sqrt(*deviatie);
}

void getImage(char* imagine, Imagine* img) {
    FILE *fin;
    int i, j;
    fin = fopen(imagine, "rb");
    unsigned int dim_img, latime_img, inaltime_img;
    unsigned char pRGB[3];

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);

	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

	img->inaltime = inaltime_img;
	img->latime = latime_img;
	img->img = (int**) malloc(img->inaltime * sizeof(int*));
    for(i = 0; i < img->inaltime; i++)
        img->img[i] = (int*) malloc(img->latime * sizeof(int));

	fseek(fin, 54, SEEK_SET);
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			fread(pRGB, 3, 1, fin);
            img->img[i][j] = pRGB[0];
		}
		fseek(fin,padding,SEEK_CUR);
	}
	fclose(fin);
}

double calcCorr(Imagine imag, Imagine sabl, int lin, int col) {
    Imagine partial;
    int i, j;
    partial.inaltime = sabl.inaltime;
    partial.latime = sabl.latime;
    partial.img = (int**) malloc(partial.inaltime * sizeof(int*));
    for(i = 0; i < partial.inaltime; i++)
        partial.img[i] = (int*) malloc(partial.latime * sizeof(int));
    for(i = 0; i < partial.inaltime; i++)
        for(j = 0; j < partial.latime; j++)
            partial.img[i][j] = imag.img[lin + i][col + j];

    double medieS, deviatieS, medieP, deviatieP, corr = 0;
    calcMedie(partial, &medieP, &deviatieP);
    calcMedie(sabl, &medieS, &deviatieS);

    for(i = 0; i < partial.inaltime; i++)
        for(j = 0; j < partial.latime; j++) {
            double termen = (partial.img[i][j] - medieP) * (sabl.img[i][j] - medieS);
            termen /= deviatieS * deviatieP;
            corr += termen;
        }
    for(i = 0; i < partial.inaltime; i++)
        free(partial.img[i]);///eliberare memorie alocata
    free(partial.img);
    return corr / (partial.inaltime * partial.latime);
}

detectii templateMatching(char* imagine, char* sablon, double pS) {

    char* new_imagine = "grayscale_imagine.bmp";
    char* new_sablon = "grayscale_sablon.bmp";
    int i, j;
    ///transformare imagine si sablon in grayscale
    grayscale_image(imagine, new_imagine);
    grayscale_image(sablon, new_sablon);

    Imagine imag, sabl;
    getImage(new_imagine, &imag);
    getImage(new_sablon, &sabl);

    detectii solutii;
    for(i = 0; i < imag.inaltime - sabl.inaltime + 1; i++)
        for(j = 0; j < imag.latime - sabl.latime + 1; j++) {
                ///calculez corelatia pt fereastra curenta
            double corelatie = calcCorr(imag, sabl, i, j);
            if(corelatie >= pS) {
                ///daca fereastra depaseste pragul
                if(solutii.nrDetectii == 0)
                ///adaug detectia
                    solutii.v = (detectie*) malloc(sizeof(detectie));
                else
                    solutii.v = (detectie*) realloc(solutii.v, (solutii.nrDetectii + 1) * sizeof(detectie));
                fereastra new_fereastra = {i, j};
                detectie new_sol = {new_fereastra, corelatie};
                solutii.v[solutii.nrDetectii++] = new_sol;
            }
        }
    for(i = 0; i < imag.inaltime; i++)
        free(imag.img[i]);
    free(imag.img);
    for(i = 0; i < sabl.inaltime; i++)
        free(sabl.img[i]);
    free(sabl.img);
    return solutii;
}

int inContur(int lin, int col, fereastra fer) {

    if(lin == fer.lin && col >= fer.col && col < fer.col + 11)
        return 1;
    if(lin == fer.lin + 14 && col >= fer.col && col < fer.col + 11)
        return 1;
    if(col == fer.col && lin >= fer.lin && lin < fer.lin + 14)
        return 1;
    if(col == fer.col + 10 && lin >= fer.lin && lin < fer.lin + 14)
        return 1;
    return 0;
}

void paintWindow(char* imagine, fereastra fer, int* rgb) {
    FILE *fin = fopen(imagine, "rb+");
    int i, j;
    unsigned int dim_img, latime_img, inaltime_img;
    unsigned char pRGB[3];

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);

	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

	fseek(fin, 54, SEEK_SET);
	///desenez laturile dreptunghiului orizontale si verticale
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			fread(pRGB, 3, 1, fin);
			if(inContur(i, j, fer)) {
                pRGB[0] = rgb[0];
                pRGB[1] = rgb[1];
                pRGB[2] = rgb[2];
			}
        	fseek(fin, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fin);
        	fflush(fin);
		}
		fseek(fin,padding,SEEK_CUR);
	}
	fclose(fin);
}

int cmp(const void* a, const void* b) {
    return (*(detectie*)b).corelatie - (*(detectie*)a).corelatie;
}

void sortDetections(detectii* detectii) {
    qsort(detectii->v, detectii->nrDetectii, sizeof(detectie), cmp);
}

double suprapunere(fereastra f1, fereastra f2) {
    double hInter = (f1.lin < f2.lin ? f1.lin + 14 - f2.lin : f2.lin + 14 - f1.lin);
    double lenInter = (f1.col < f2.col ? f1.col + 10 - f2.col : f2.col + 10 - f1.col);
    if(hInter < 0 || lenInter < 0)
        return 0;
    return (hInter * lenInter) / (330 - hInter * lenInter);
}

void maxElimination(detectii* detectii) {
    int i,j;
    sortDetections(detectii);
    int *good = (int*) malloc(detectii->nrDetectii * sizeof(int));
    memset(good, 1, detectii->nrDetectii * sizeof(int));
    for(i = 0; i < detectii->nrDetectii; i++)
        for(j = i + 1; j < detectii->nrDetectii; j++)
            if(suprapunere(detectii->v[i].f, detectii->v[j].f) > 0.2)
                good[j] = 0;

    int nr_good = 0;
    for(i = 0; i < detectii->nrDetectii; i++)
        if(good[i])
            detectii->v[nr_good++] = detectii->v[i];
    detectii->v = (detectie*) realloc(detectii->v, nr_good * sizeof(detectie));
    detectii->nrDetectii = nr_good;

    free(good);
}

void copyImages(char *initial_img, char *final_img) {
    FILE *fin = fopen(initial_img, "rb"), *fout = fopen(final_img, "wb");
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);
	fclose(fout);
}

void mergeDet(detectii *d1, detectii *d2) {
    d1->v = (detectie*) realloc(d1->v, (d1->nrDetectii + d2->nrDetectii) * sizeof(detectie));
    int i;
    for(i = 0; i < d2->nrDetectii; i++)
        d1->v[d1->nrDetectii + i] = d2->v[i];
    d1->nrDetectii += d2->nrDetectii;
    free(d2->v);
}

void fullProcess() {
    int i;
    char* testImage = "test.bmp";
    char* finalImage = "final_image.bmp";
    copyImages(testImage, finalImage);
    char* sabloane[] = {"cifra0.bmp", "cifra1.bmp", "cifra2.bmp", "cifra3.bmp", "cifra4.bmp", "cifra5.bmp",
                        "cifra6.bmp", "cifra7.bmp", "cifra8.bmp", "cifra9.bmp"};

    detectii det = templateMatching(testImage, sabloane[0], 0.5);
    for(i = 1; i < 10; i++) {
        detectii d = templateMatching(testImage, sabloane[i], 0.5);
        mergeDet(&det, &d);
    }

    maxElimination(&det);
    int rgb[3] = {0, 0, 0};
    int *used = (int*) malloc(256 * 256 * 256 * sizeof(int));
    memset(used, 0, 256 * 256 * 256 * sizeof(int));
    srand(time(0));
    for(i = 0; i < det.nrDetectii; i++) {
        while(1) {
            if(!used[rgb[0] * 256 * 256 + rgb[1] * 256 + rgb[2]])
                break;
            rgb[0] = rand() % 256;
            rgb[1] = rand() % 256;
            rgb[2] = rand() % 256;
        }
        used[rgb[0] * 256 * 256 + rgb[1] * 256 + rgb[2]] = 1;
        paintWindow(finalImage, det.v[i].f, rgb);
    }
    free(used);
}

int main()
{

  ///partea I
  /*unsigned char *header;
  pixel *p;
  int latime,inaltime;
    cerinta2("peppers.bmp",&p,&latime,&inaltime,&header);
  cerinta3("newfile.bmp",p,latime,inaltime,header);
  */
  cerinta4("peppers.bmp","newfile.bmp","secret_key.txt");
  cerinta5("newfile.bmp","decriptata.bmp","secret_key.txt");
  Test("peppers.bmp");

  ///partea a II-a
  fullProcess();
    return 0;
}
