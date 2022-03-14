#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>
using   namespace   std;   

int main()
{
	clock_t start, finish;
	clock_t start1, finish1;

	int i,j,k;
	//initial two 1000*1000 matrix
	int (*a)[1000],(*b)[1000];
	a = new int[1000][1000];
	b = new int[1000][1000];
    
	
	for(i = 0; i < 1000; i++)
	{
		for(j = 0; j < 1000; j++)
		{
			a[i][j] = i % (j+1);
			b[i][j] = i / (j+1);
		}
	}
	//store A*B result
	int (*c)[1000],(*d)[1000];
	c = new int[1000][1000];
	d = new int[1000][1000];

	//initial 0
	memset(c,0, 1000*1000*sizeof(int));
	memset(d,0, 1000*1000*sizeof(int));

	start = clock();	
	for(i = 0; i < 1000; i++)
	{
		for(j = 0; j < 1000; j++)
		{
			for (k = 0; k < 1000; k++)
			{
				c[i][j] += a[i][k] * b[k][j];
			}

		}
	}
	finish = clock();

	start1 = clock();

	//======================================================
	//add your own code
	//======================================================
	// Plan A
	for(i = 0; i < 1000; i++)
	{
		for(j = 0; j < 1000; j++)
		{
			for (k = 0; k < 1000; k++)
			{
				d[i][k] += a[i][j] * b[j][k];
			}

		}
	}
	// Plan B
	// int (*bT)[1000] = new int[1000][1000];
	// memset(bT, 0, 1000*1000*sizeof(int));
	// for (i = 0; i < 1000; i ++) {
	// 	for (j = 0; j < 1000; j ++) {
	// 		bT[j][i] = b[i][j];
	// 	}
	// }
	// for(i = 0; i < 1000; i++)
	// {
	// 	for(j = 0; j < 1000; j++)
	// 	{
	// 		for (k = 0; k < 1000; k++)
	// 		{
	// 			d[i][j] += a[i][k] * bT[j][k];
	// 		}

	// 	}
	// }






	finish1 = clock();


	//compare the results
	for(i = 0; i < 1000; i++)
	{
		for(j = 0; j < 1000; j++)
		{
			if (c[i][j] != d[i][j])
			{
				cout<<"you have got an error in algorithm modification!"<<endl;
				exit(1);
			}

		}
	}



	cout<<"time spent for original method : "<<finish - start<<" ms"<<endl;
	cout<<"time spent for new method : "<<finish1 - start1<<" ms"<<endl;
	return 0;
}
