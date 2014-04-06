#include <stdlib.h>
#include <stdio.h>


#ifndef min
#define min(a, b) ((a)<=(b) ? (a) : (b))
#endif

#define swap(a, b) { char *t=x[a]; \
                     x[a]=x[b]; x[b]=t; }
#define i2c(i) x[i][depth]
#define swapIndex(a,b) {int t=index[a];   index[a]=index[b]; index[b]=t;}

#define swap2(a, b) { t = *(a); *(a) = *(b); *(b) = t; }
#define swap2Index(a, b) {int t = *(a); *(a) = *(b); *(b) = t; }
#define ptr2char(i) (*(*(i) + depth))
#define med3(a, b, c) med3func(a, b, c, depth)

void vecswap(int i, int j, int n, char *x[])
{   while (n-- > 0) {
        swap(i, j);
        i++;
        j++;
    }
}

void vecswapIndex(int i, int j, int n, int *index)
{   while (n-- > 0) 
	{
        swapIndex(i,j);
        i++;
        j++;
    }
}

void ssort1(char *x[], int n, int depth,int* index)
{   
	int    a, b, c, d, r, v;
    if (n <= 1)
        return;
        
    /// Choosing the pivot
    //a = rand() % n;
    a=0;
    /// /////////////////
    
    swap(0, a);
    swapIndex(0,a);
    v = i2c(0);
    a = b = 1;
    c = d = n-1;
    
    
    for (;;) 
    {
        while (b <= c && (r = i2c(b)-v) <= 0) {
            if (r == 0) { swap(a, b); swapIndex(a,b);a++; }
            b++;
        }
        while (b <= c && (r = i2c(c)-v) >= 0) {
            if (r == 0) { swap(c, d); swapIndex(c,d);d--; }
            c--;
        }

        if (b > c) break;
        swap(b, c);
        swapIndex(b,c);
        b++;
        c--;
    }
    r = min(a, b-a);
    vecswap(0, b-r, r, x);
    vecswapIndex(0, b-r, r, index);
    r = min(d-c, n-d-1); 
    vecswap(b, n-r, r, x);
    vecswapIndex(b, n-r, r, index);
    r = b-a; 
     ssort1(x, r, depth,index);
    if (i2c(r) != 0)
        ssort1(x + r, a + n-d-1, depth+1,index+r);
    r = d-c; 
    ssort1(x + n-r, r, depth,index+ n-r);
}

void ssort1main(char *x[], int n,int* index)
{ ssort1(x, n, 0,index); }



/// needed function for fast ssort

void vecswap2(char **a, char **b, int n)
{   while (n-- > 0) {
        char *t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

void vecswap2Index(int *a, int *b, int n)
{   while (n-- > 0) {
        int t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

char **med3func(char **a, char **b, char **c, int depth)
{   int va, vb, vc;
    if ((va=ptr2char(a)) == (vb=ptr2char(b)))
        return a;
    if ((vc=ptr2char(c)) == va || vc == vb)
        return c;       
    return va < vb ?
          (vb < vc ? b : (va < vc ? c : a ) )
        : (vb > vc ? b : (va < vc ? a : c ) );
}


void inssort(char **a, int n, int d)
{   char **pi, **pj, *s, *t;
    for (pi = a + 1; --n > 0; pi++)
    {
		//printf("pi=%p\n",pi);
		//printf("a+1=%p\n",a+1);
        for (pj = pi; pj > a; pj--) 
        {
			
            // Inline strcmp: break if *(pj-1) <= *pj
            for (s=*(pj-1)+d, t=*pj+d; *s==*t && *s!=0; s++, t++)
                ;
            if (*s <= *t)
                break;
                
            //printf("pj=%p\n",pj);
            //printf("pj-1=%p\n",pj-1);
            swap2(pj, pj-1);
            
		}
	}
}

void ssort2(char **a, int n, int depth,int* index)
{   int d, r, partval;
    int Diff=0;
    char **pa, **pb, **pc, **pd, **pl, **pm, **pn, *t;
    //~ if (n < 10) {
        //~ inssort(a, n, depth);
        //~ return;
    //~ }
    pl = a;
    pm = a + (n/2);
    pn = a + (n-1);
    if (n > 30) { // On big arrays, pseudomedian of 9
        d = (n/8);
        pl = med3(pl, pl+d, pl+2*d);
        pm = med3(pm-d, pm, pm+d);
        pn = med3(pn-2*d, pn-d, pn);
    }
    pm = med3(pl, pm, pn);
    Diff=pm-a;
    int *IndexA,*IndexB,*IndexC,*IndexD,*IndexN;
    swap2(a, pm);
    swap2Index(index,index+Diff);
    partval = ptr2char(a);
    pa = pb = a + 1;
    pc = pd = a + n-1;
    IndexA=IndexB=index+1;
    IndexC=IndexD=index+n-1;
    for (;;) {
        while (pb <= pc && (r = ptr2char(pb)-partval) <= 0) {
            if (r == 0) { swap2(pa, pb);swap2Index(IndexA, IndexB);pa++;IndexA++; }
            pb++;
            IndexB++;
        }
        while (pb <= pc && (r = ptr2char(pc)-partval) >= 0) {
            if (r == 0) { swap2(pc, pd); swap2Index(IndexC, IndexD);pd--;IndexD-- ;}
            pc--;
            IndexC--;
        }
        if (pb > pc) break;
        swap2(pb, pc);
        swap2Index(IndexB,IndexC);
        pb++;
        IndexB++;
        pc--;
        IndexC--;
    }
    pn = a + n;
    IndexN=index+n;
    r = min(pa-a, pb-pa);    vecswap2(a,  pb-r, r); vecswap2Index(index,IndexB-r,r);
    r = min(pd-pc, pn-pd-1); vecswap2(pb, pn-r, r); vecswap2Index(IndexB,IndexN-r,r);
    if ((r = pb-pa) > 1)
        ssort2(a, r, depth,index);
    if (ptr2char(a + r) != 0)
        ssort2(a + r, pa-a + pn-pd-1, depth+1,index+r);
    if ((r = pd-pc) > 1)
        ssort2(a + n-r, r, depth,index+n-r);
}

void ssort2main(char **a, int n,int* index) 
{
	for (int i=0;i<n;i++)
	//printf("a=%p ,*a=%c\n",a[i],*a[i]) ;
	printf("Enter ssort2main\n");
	ssort2(a, n, 0,index); 
}

