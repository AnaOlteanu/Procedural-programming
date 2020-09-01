#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    unsigned char r,g,b;
}pixel;    ///structura cu ajutorul careia retinem un pixel al imaginii pe cele 3 canale red, green ,blue

typedef struct
{
    unsigned char x0,x1,x2,x3;
}octeti;    ///structura cu ajutorul careia retinem cei 4 octeti ai unui intreg pe 32 de biti


unsigned int xorShift32(unsigned int seed)   ///algoritmul ca in prezentarea powerpoint
{
    unsigned int r=seed;
        r=r^r<<13;
        r=r^r>>17;
        r=r^r<<5;
    return r;
}


pixel* liniarizare(char *nume_fisier_sursa, unsigned int latime, unsigned int inaltime)
{
    FILE *fin=fopen(nume_fisier_sursa,"rb");
    unsigned int i,j,k=0;
    unsigned char *pRGB;
    pixel *L;
    L=(pixel*)malloc(latime*inaltime*sizeof(pixel));
    pRGB=(unsigned char*)malloc(3*sizeof(unsigned char));

    unsigned int nr_octeti_linie=latime*3;
    unsigned int start=nr_octeti_linie*(inaltime-1)+54;

    fseek(fin,start,SEEK_SET);
    for(i=0;i<inaltime;i++)
    {
        for(j=0;j<latime;j++)
    {
        pixel p;
        fread(pRGB,3,1,fin);
        p.r=pRGB[2];
        p.g=pRGB[1];
        p.b=pRGB[0];
        L[k]=p;
        k++;
    }
        fseek(fin,-2*nr_octeti_linie,SEEK_CUR);

    }
    return L;
}

void afisareImagCriptata(char *nume_fisier_destinatie,pixel *c,unsigned int latime,unsigned int inaltime,unsigned char *header)
{
    FILE *fout=fopen(nume_fisier_destinatie,"wb");
    unsigned int d=latime*inaltime;
    int i,k=0,j;

    if(fout==NULL)
    {
        printf("eroare");
        return;
    }
    for(i=0;i<54;i++)
        fwrite(&header[i],1,1,fout);

    pixel aux;

    for(k=0;k<d-latime+1;k=k+latime)
    {
    j=k+latime-1;    ///plasarea la finalul unei grupe de pixeli (o grupa de pixeli este formata din "latime"-1 elemente)
    for(i=k;i<k+latime/2;i++)    ///interschimbarea celor 2 jumatati ale grupei, prin interschimbarea elementelor dintr-o pereche
    {                            ///se interschimba primul cu ultimul element din grupa, apoi al doilea cu penultimul si tot asa pana l jumatate
        aux=c[i];
        c[i]=c[j];
        c[j]=aux;
        j--;
    }
    }

    for(i=d-1;i>=0;i--)   ///vectorul rezultat va contine pixelii in ordine inversa decat in imaginea initiala
    {
        fwrite(&c[i].b,1,1,fout);
        fwrite(&c[i].g,1,1,fout);
        fwrite(&c[i].r,1,1,fout);
    }

    fclose(fout);
}

void chiPatrat(char *nume_fisier)
{
    FILE *f=fopen(nume_fisier,"rb");
    if(f==NULL)
    {
        printf("nu s-a gasit imaginea pt testul chi patrat");
        return;
    }

    unsigned int latime,inaltime;

    fseek(f,18,SEEK_SET);
    fread(&latime,sizeof(unsigned int),1,f);
    fseek(f,22,SEEK_SET);
    fread(&inaltime,sizeof(unsigned int),1,f);

    unsigned int i,j,k=0;
    unsigned char *pRGB;
    pixel *L;
    L=(pixel*)malloc(latime*inaltime*sizeof(pixel));
    pRGB=(unsigned char*)malloc(3*sizeof(unsigned int));


    ///liniarizarea imaginii
    fseek(f,54,SEEK_SET);
    for(i=0;i<inaltime;i++)
        for(j=0;j<latime;j++)
    {
        pixel p;
        fread(pRGB,3,1,f);
        p.r=pRGB[2];
        p.g=pRGB[1];
        p.b=pRGB[0];
        L[k]=p;
        k++;
    }

    unsigned int fr,fg,fb;
    float f_bar,chi_r=0,chi_g=0,chi_b=0;
    f_bar=inaltime*latime/256;

    for(i=0;i<=255;i++)
    {
        fr=0;   ///frecventa culorii rosii
        for(j=0;j<latime*inaltime;j++)
            if(L[j].r==i)
                fr++;
        chi_r+=(fr-f_bar)*(fr-f_bar)/f_bar;  ///valoarea testului chi patrat pe canalul rosu
    }
    for(i=0;i<=255;i++)
    {
        fg=0;    ///frecventa culorii verzi
        for(j=0;j<latime*inaltime;j++)
            if(L[j].g==i)
                fg++;
        chi_g+=(fg-f_bar)*(fg-f_bar)/f_bar;   ///valoarea testului chi patrat pe canalul verde
    }
    for(i=0;i<=255;i++)
    {
        fb=0;   ///frecventa culorii albastre
        for(j=0;j<latime*inaltime;j++)
            if(L[j].b==i)
                fb++;
        chi_b+=(fb-f_bar)*(fb-f_bar)/f_bar;   ///valoarea testului chi patrat pe canalul albastru
    }

    printf("%.2f %.2f %.2f",chi_r,chi_g,chi_b);
    fclose(f);
}

unsigned int random(unsigned int i,unsigned int r)
{
    return r%i;
}

unsigned char extrage8Biti(unsigned int n,unsigned int k, unsigned int p)
{
    return (((1<<k)-1)&(n>>(p-1)));
}


void criptare(char *nume_fisier_sursa, char *nume_fisier_destinatie, char *nume_fisier_key)
{
    FILE *fin;
    fin=fopen(nume_fisier_sursa,"rb");
    printf("nume_fisier_sursa: %s\n",nume_fisier_sursa);
    if(fin==NULL)
    {
        printf("nu am gasit fisierul sursa care sa contina imaginea\n");
        return;
    }

    unsigned char *header;
    header=(unsigned char*)malloc(54*sizeof(unsigned char));
    unsigned int i;
    fseek(fin,0,SEEK_SET);
    for(i=0;i<54;i++)
        fread(&header[i],1,1,fin);

    unsigned int dim_imag,latime_imag,inaltime_imag;

    fseek(fin,2,SEEK_SET);
    fread(&dim_imag,sizeof(unsigned int),1,fin);
    printf("dimensiune imagine in octeti: %u\n",dim_imag);

    fseek(fin,18,SEEK_SET);
    fread(&latime_imag,sizeof(unsigned int),1,fin);
    fseek(fin,22,SEEK_SET);
    fread(&inaltime_imag,sizeof(unsigned int),1,fin);
    printf("dimensiune imagine in pixeli(l x h): %u x %u\n",latime_imag,inaltime_imag);

    int padding;
    if(latime_imag%4!=0)
        padding=4-(3*latime_imag)%4;
    else
        padding=0;
    printf("padding= %d\n",padding);


    unsigned int d=latime_imag*inaltime_imag;

    pixel *p;   ///imaginea initiala liniarizata prin vectorul de pixeli p
    p=(pixel*)malloc(d*sizeof(pixel));
    p=liniarizare(nume_fisier_sursa,latime_imag,inaltime_imag);


    FILE *g=fopen(nume_fisier_key,"r");
    if(g==NULL)
    {
        printf("nu s-a gasit fisierul care contine cheia secreta\n");
        return;
    }
    unsigned int R0,SV,*R,r;
    fscanf(g,"%u",&R0);
    fscanf(g,"%u",&SV);

    r=R0;
    R=(unsigned int*)malloc((2*d-1)*sizeof(unsigned int));

    ///crearea vectorului R cu numere aleatoare obtinute cu ajutorul functiei xorShift32, plecand de la cheia secreta R0
    for(i=1;i<=2*d-1;i++)
        {
            R[i]=xorShift32(r);
            r=R[i];
        }

    ///se creeaza o permutare aleatoare folosing algoritmul lui Durstenfeld(ca in prezentare)

    unsigned int *v;
    v=(unsigned int*)malloc(d*sizeof(unsigned int));
    for(i=0;i<d;i++)
        v[i]=i;
    unsigned int rd,aux,j=1;

    for(i=d-1;i>=1;i--)
    {
        r=R[j];
        rd=random(i,r);
        j++;
        aux=v[rd];
        v[rd]=v[i];
        v[i]=aux;
    }

    ///se creeaza o imagine intermediara reprezentata de vectorul de pixeli p2, obtinut cu ajutorul permutarii aleatoare
    pixel *p2;
    p2=(pixel*)malloc(d*sizeof(pixel));
    for(i=0;i<=d-1;i++)
        p2[v[i]]=p[i];


    unsigned char *x_SV;
    octeti *x_R;
    x_SV=(unsigned char*)malloc(4*sizeof(unsigned char));   ///elementele lui x_SV vor reprezenta cei 4 octeti ai lui SV
    x_R=(octeti*)malloc(d*sizeof(octeti));   ///fiecare element al vectorului va retine cate un numar rezultat in urma impartirii in 4 octeti a fiecarui element din R
    unsigned int index=3;

    for(i=1;i<=32;i+=8)
    {
        x_SV[index]=extrage8Biti(SV,8,i);
        index--;
    }


    index=0;
    for(i=d;i<=2*d-1;i++)
    {

        for(j=1;j<=32;j+=8)
        {
            if(j==1)
                x_R[index].x3=extrage8Biti(R[i],8,j);
            if(j==9)
                x_R[index].x2=extrage8Biti(R[i],8,j);
            if(j==17)
                x_R[index].x1=extrage8Biti(R[i],8,j);
            if(j==25)
                x_R[index].x0=extrage8Biti(R[i],8,j);
        }
        index++;
    }


    pixel *C;    ///imaginea criptata reprezentata de vectorul de pixeli C
    C=(pixel*)malloc(d*sizeof(pixel));

    ///xorari asemenea celor din prezentare
    C[0].r=(p2[0].r)^(x_SV[2])^(x_R[0].x2);
    C[0].g=(p2[0].g)^(x_SV[1])^(x_R[0].x1);
    C[0].b=(p2[0].b)^(x_SV[0])^(x_R[0].x0);

    for(i=1;i<d;i++)
    {
        C[i].r=(C[i-1].r)^(p2[i].r)^(x_R[i].x2);
        C[i].g=(C[i-1].g)^(p2[i].g)^(x_R[i].x1);
        C[i].b=(C[i-1].b)^(p2[i].b)^(x_R[i].x0);

    }

    afisareImagCriptata(nume_fisier_destinatie,C,latime_imag,inaltime_imag,header);

    fclose(fin);
    fclose(g);
}


void decriptare(char *nume_fisier_sursa, char *nume_fisier_destinatie, char *nume_fisier_key)
{
    FILE *fin;
    fin=fopen(nume_fisier_sursa,"rb");
    printf("nume_fisier_sursa: %s\n",nume_fisier_sursa);
    if(fin==NULL)
    {
        printf("nu am gasit fisierul sursa care sa contina imaginea");
        return;
    }

    unsigned char *head;
    head=(unsigned char*)malloc(54*sizeof(unsigned char));
    unsigned int i;
    fseek(fin,0,SEEK_SET);
    for(i=0;i<54;i++)
        fread(&head[i],1,1,fin);

    unsigned int dim_imag,latime_imag,inaltime_imag;

    fseek(fin,2,SEEK_SET);
    fread(&dim_imag,sizeof(unsigned int),1,fin);
    printf("dimensiune imagine in octeti: %u\n",dim_imag);

    fseek(fin,18,SEEK_SET);
    fread(&latime_imag,sizeof(unsigned int),1,fin);
    fseek(fin,22,SEEK_SET);
    fread(&inaltime_imag,sizeof(unsigned int),1,fin);
    printf("dimensiune imagine in pixeli(l x h): %u x %u\n",latime_imag,inaltime_imag);

    int padding;
    if(latime_imag%4!=0)
        padding=4-(3*latime_imag)%4;
    else
        padding=0;
    printf("padding= %d\n",padding);


    unsigned int d=latime_imag*inaltime_imag;

    pixel *C;
    C=(pixel*)malloc(d*sizeof(pixel));
    C=liniarizare(nume_fisier_sursa,latime_imag,inaltime_imag);


    FILE *g=fopen(nume_fisier_key,"r");
    if(g==NULL)
    {
        printf("nu s-a gasit fisierul care contine cheia secreta\n");
        return;
    }
    unsigned int R0,SV,*R,r;
    fscanf(g,"%u",&R0);
    fscanf(g,"%u",&SV);

    r=R0;
    R=(unsigned int*)malloc((2*d-1)*sizeof(unsigned int));


    for(i=1;i<=2*d-1;i++)
        {
            R[i]=xorShift32(r);
            r=R[i];
        }

    unsigned int *v;
    v=(unsigned int*)malloc(d*sizeof(unsigned int));
    for(i=0;i<d;i++)
        v[i]=i;
    unsigned int rd,aux,j=1;

    for(i=d-1;i>=1;i--)
    {
        r=R[j];
        rd=random(i,r);
        j++;
        aux=v[rd];
        v[rd]=v[i];
        v[i]=aux;
    }

    unsigned int *inversa;    ///calcularea inversei permutarii v
    inversa=(unsigned int*)malloc(d*sizeof(unsigned int));
    for(i=0;i<d;i++)
    {
        inversa[v[i]]=i;
    }

    unsigned char *x_SV;
    octeti *x_R;
    x_SV=(unsigned char*)malloc(4*sizeof(unsigned char));
    x_R=(octeti*)malloc(d*sizeof(octeti));
    unsigned int index=3;

    for(i=1;i<=32;i+=8)
    {
        x_SV[index]=extrage8Biti(SV,8,i);
        index--;
    }

    index=0;
    for(i=d;i<=2*d-1;i++)
    {
        for(j=1;j<=32;j+=8)
        {
            if(j==1)
                x_R[index].x3=extrage8Biti(R[i],8,j);
            if(j==9)
                x_R[index].x2=extrage8Biti(R[i],8,j);
            if(j==17)
                x_R[index].x1=extrage8Biti(R[i],8,j);
            if(j==25)
                x_R[index].x0=extrage8Biti(R[i],8,j);
        }
        index++;
    }

    pixel *C2;
    C2=(pixel*)malloc(d*sizeof(pixel));

    C2[0].r=(C[0].r)^(x_SV[2])^(x_R[0].x2);
    C2[0].g=(C[0].g)^(x_SV[1])^(x_R[0].x1);
    C2[0].b=(C[0].b)^(x_SV[0])^(x_R[0].x0);

    for(i=1;i<d;i++)
    {
        C2[i].r=(C[i-1].r)^(C[i].r)^(x_R[i].x2);
        C2[i].g=(C[i-1].g)^(C[i].g)^(x_R[i].x1);
        C2[i].b=(C[i-1].b)^(C[i].b)^(x_R[i].x0);
    }

    pixel *D;   ///imaginea initiala liniarizata prin vectorul de pixeli D
    D=(pixel*)malloc(d*sizeof(pixel));
    for(i=0;i<d;i++)
        D[inversa[i]]=C2[i];

    afisareImagCriptata(nume_fisier_destinatie,D,latime_imag,inaltime_imag,head);
}

int main()
{
    char *nume_img_sursa,*nume_img_c,*nume_img_d,*nume_cheie_secreta;

    nume_img_sursa=(char*)malloc(100*sizeof(char));
    nume_img_c=(char*)malloc(100*sizeof(char));
    nume_img_d=(char*)malloc(100*sizeof(char));
    nume_cheie_secreta=(char*)malloc(100*sizeof(char));

    printf("Numele fisierului care contine imaginea de test: ");
    fgets(nume_img_sursa, 100, stdin);
    nume_img_sursa[strlen(nume_img_sursa) - 1] = '\0';

    printf("Numele fisierului care contine imaginea criptata: ");
    fgets(nume_img_c, 100, stdin);
    nume_img_c[strlen(nume_img_c) - 1] = '\0';

    printf("Numele fisierului care contine imaginea decriptata: ");
    fgets(nume_img_d, 100, stdin);
    nume_img_d[strlen(nume_img_d) - 1] = '\0';

    printf("Numele fisierului care contine cheia secreta: ");
    fgets(nume_cheie_secreta, 100, stdin);
    nume_cheie_secreta[strlen(nume_cheie_secreta) - 1] = '\0';



	criptare(nume_img_sursa,nume_img_c,nume_cheie_secreta);
	decriptare(nume_img_c,nume_img_d,nume_cheie_secreta);
	chiPatrat(nume_img_c);

	free(nume_img_sursa);
	free(nume_img_c);
	free(nume_img_d);
	free(nume_cheie_secreta);

	return 0;
}
