#include <iostream>
#include <time.h>
#include <Windows.h>
#include <stdlib.h>
#include <random>
#include <chrono> 

using namespace std;

#define ARRAY_SIZE (1 << 28)                                    // test array size is 2^28
typedef unsigned char BYTE;										// define BYTE as one-byte type
typedef chrono::duration<long long, milli> milliseconds; // 毫秒 


BYTE array[ARRAY_SIZE];											// test array
int L1_cache_size = 1 << 15;
int L2_cache_size = 1 << 18;
int L1_cache_block = 64;
int L2_cache_block = 64;
int L1_way_count = 8;
int L2_way_count = 4;
int write_policy = 0;											// 0 for write back ; 1 for write through

int test_times = 11451419 * 10;


// have an access to arrays with L2 Data Cache'size to clear the L1 cache
void Clear_L1_Cache() {
	memset(array, 0, L2_cache_size);
}

// have an access to arrays with ARRAY_SIZE to clear the L2 cache
void Clear_L2_Cache() {
	memset(&array[L2_cache_size + 1], 0, ARRAY_SIZE - L2_cache_size);
}

// int test
void random_array()
{
	srand(time(NULL));
	for(int i = 0; i < ARRAY_SIZE; i++)
		array[i] = rand(); 
}

int L1_DCache_Size() {
	cout << "L1_Data_Cache_Test" << endl;
	//add your own code
	Clear_L1_Cache();
	Clear_L2_Cache();

	srand(time(NULL));
	int tmp = 0;
	// 随机数生成
	// std::random_device rd;  //Will be used to obtain a seed for the random number engine
    // std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	for(int gap = L1_cache_size>>3; gap<=(L1_cache_size<<1); gap=gap<<1)
	{
		cout<<"Test array size "<<gap/1024 << "KB"<<"\t\t";
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		for(int i = 0; i < test_times; i++)
		{
			tmp+=array[(rand()*rand())%gap];
		}
		// clock_t end = clock();
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
//		double elapsed_time = (double)(end-start)/CLOCKS_PER_SEC * 1000;
		cout<<"Average access time: "<<dt/(test_times)<<"ms"<<endl;

		Clear_L1_Cache();
		Clear_L2_Cache();
	}
	// test L1 cache
	// for(int i = 0; i < 2; i++)
	// {	
	// 	int tmp0 = 0, tmp1=0, tmp2=0, tmp3=0;	
	// 	random_array();
	// 	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	// 	for(int i = 0; i < test_times;i++)
	// 	{
	// 		tmp0+=array[0];
	// 		tmp1+=array[1];
	// 		tmp2+=array[2];
	// 		tmp3+=array[3];
	// 	}
	// 	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	// 	milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	// 	double dt = time_span.count();
	// 	cout<<"Test L1 cache "<<dt/(test_times*4)<<endl;
	// }
	// for(int test_size = L1_cache_size>>5; test_size<ARRAY_SIZE; test_size<<=1)
	// {
	// 	random_array();
	// 	int stride = test_size / 4, st2 = 2 * stride, st3 = st2+stride;
	// 	int acc0 = 0, acc1 = 0, acc2 = 0, acc3 = 0;
	// 	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	// 	for(int i = 0, idx=0; i < test_times; i++, (idx++)%stride)
	// 	{
	// 		acc0+=array[idx];
	// 		acc1+=array[idx+stride];
	// 		acc2+=array[idx+st2];
	// 		acc3+=array[idx+st3];
	// 	}
	// 	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	// 	milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	// 	double dt = time_span.count();
	// 	cout<<"Test size "<<test_size<<"\t";
	// 	cout<<"Average access time: "<<dt/(test_times*4)<<"ms"<<endl;
	// 	Clear_L1_Cache();
 	// 	Clear_L2_Cache();
	// }
	return 0;
}

int L2_Cache_Size() {
	cout << "L2_Data_Cache_Test" << endl;
		//add your own code
// 	Clear_L1_Cache();
// 	Clear_L2_Cache();

// 	int test_times = 11451419 * 10;
// 	// srand(time(NULL));
// 	// 随机数生成
// 	std::random_device rd;  //Will be used to obtain a seed for the random number engine
//     std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
// 	for(int gap = L2_cache_size>>5; gap<=(L2_cache_size<<1); gap=gap<<1)
// 	{
// 		cout<<"Test array size "<<gap/1024 << "KB"<<"\t\t";
// 		double tmp = 0;
// 		int idx = 0; 
// 		std::uniform_int_distribution<> distrib(0, gap-1);
// 		// 生成随机访问序列
// 		vector<int> access_array;
// 		for(int i = 0; i < test_times; i++)
// 		{
// 			access_array.push_back(distrib(gen));
// 		}
// 		// clock_t start = clock();
// 		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
// 		for(int i = 0; i < test_times; i++)
// 		{
// //			int idx = (rand() % (ARRAY_SIZE/gap)) * gap + rand();
// //			long idx = (rand() % gap + rand() %  * gap) % ARRAY_SIZE;
// //			cout<<idx<<endl;
// 			tmp+=array[access_array[i]];
// 		}
// 		// clock_t end = clock();
// 		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
// 		// chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
// 		milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
// 		double dt = time_span.count();
// //		double elapsed_time = (double)(end-start)/CLOCKS_PER_SEC * 1000;
// 		cout<<"Average access time: "<<dt/(test_times)<<"ms"<<endl;

// 		Clear_L1_Cache();
// 		Clear_L2_Cache();

// 	}
	// for(int test_size = L2_cache_size>>5; test_size<L2_cache_size<<2; test_size<<=1)
	// {
	// 	int stride = test_size / 4, st2 = 2 * stride, st3 = st2+stride;
	// 	int acc0, acc1, acc2, acc3;
	// 	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	// 	for(int i = 0, idx=0; i < test_times; i++, (idx++)%stride)
	// 	{
	// 		acc0+=array[idx];
	// 		acc1+=array[idx+stride];
	// 		acc2+=array[idx+st2];
	// 		acc3+=array[idx+st3];
	// 	}
	// 	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	// 	milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	// 	double dt = time_span.count();
	// 	cout<<"Test size "<<test_size<<"\t";
	// 	cout<<"Average access time: "<<dt/(test_times*4)<<"ms"<<endl;
	// }
	// return 0;

		//add your own code
	Clear_L1_Cache();
	Clear_L2_Cache();

	srand(time(NULL));
	// 随机数生成
	// std::random_device rd;  //Will be used to obtain a seed for the random number engine
    // std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	int tmp = 0;
	for(int gap = L2_cache_size>>3; gap<=(L2_cache_size<<1); gap=gap<<1)
	{
		cout<<"Test array size "<<gap/1024 << "KB"<<"\t\t";
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		for(int i = 0; i < test_times; i++)
		{
			tmp+=array[(rand()*rand())%gap];
		}
		// clock_t end = clock();
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
//		double elapsed_time = (double)(end-start)/CLOCKS_PER_SEC * 1000;
		cout<<"Average access time: "<<dt/(test_times)<<"ms"<<endl;

		Clear_L1_Cache();
		Clear_L2_Cache();
	}
}

int L1_DCache_Block() {
	cout << "L1_DCache_Block_Test" << endl;
	Clear_L1_Cache();
	Clear_L2_Cache();
	//add your own code
	for(int gap = 16; gap <= L1_cache_block<<2; gap<<=1)
	{
		int tmp = 0;
		int times = L1_cache_size / gap;
		int loop_times = test_times / times;
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		// for(int i = 0, idx = 0; i < test_times; i++, idx=(idx+gap)%(L1_cache_size))
		// 	tmp+=array[idx];
		for(int i = 0; i < loop_times; i++)
			for(int j = 0; j < L1_cache_size; j+=gap)
				tmp+=array[j];
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		std::chrono::milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
		cout<<"Gap = "<<gap<<"\t"<<"Access time: "<<dt/(loop_times * times)<<"ms"<<endl;
	}	
}

int L2_Cache_Block() {
	cout << "L2_Cache_Block_Test" << endl;
	Clear_L1_Cache();
	Clear_L2_Cache();
	//add your own code
	for(int gap = 16; gap <= L2_cache_block<<2; gap<<=1)
	{
		int tmp = 0;
		int times = L2_cache_size / (gap+L1_cache_size);
		int loop_times = test_times / times;
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		// for(int i = 0, idx = 0; i < test_times; i++, idx=(idx+gap)%(L1_cache_size))
		// 	tmp+=array[idx];
		for(int i = 0; i < loop_times; i++)
			for(int j = 0; j < L2_cache_size; j+=(gap+L1_cache_size))
				tmp+=array[j];
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		std::chrono::milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
		cout<<"Gap = "<<gap<<"\t"<<"Access time: "<<dt/(loop_times * times)<<"ms"<<endl;
	}
}

int L1_DCache_Way_Count() {
	cout << "L1_DCache_Way_Count" << endl;
	//add your own code
	int tmp = 0;
	for(int way = 2; way <= 16; way<<=1)
	{
		int sz = L1_cache_size << 1;
		int way_sz = sz/way;
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		for(int n = 0; n < test_times;)
			for(int i = 0; i < sz; i+=(way_sz*2), n++)
			{
				memset(&array[i], 0, L1_cache_block);
				// tmp += array[i];
			}
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		std::chrono::milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
		cout<<"way = "<<way<<"\t"<<"Access time: "<<dt<<"ms"<<endl;
	}
}

int L2_Cache_Way_Count() {
	cout << "L2_Cache_Way_Count" << endl;
	//add your own code
	int tmp = 0;
	for(int way = 2; way <= 16; way<<=1)
	{
		int sz = L2_cache_size << 1;
		int way_sz = sz/way;
		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		for(int n = 0; n < test_times;)
			for(int i = 0; i < sz; i+=(way_sz*2), n++)
			{
				memset(&array[i], 0, L2_cache_block);
				// tmp+=array[i];
			}
		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
		std::chrono::milliseconds time_span = chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		double dt = time_span.count();
		cout<<"way = "<<way<<"\t"<<"Access time: "<<dt<<"ms"<<endl;
	}
}

int Cache_Write_Policy() {
	cout << "Cache_Write_Policy" << endl;
	//add your own code
}

void Check_Swap_Method() {
	cout << "L1_Check_Replace_Method" << endl;
	//add your own code
	
}

int main() {
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	L1_cache_size = L1_DCache_Size();
	L2_cache_size = L2_Cache_Size();
	L1_cache_block = L1_DCache_Block();
	L2_cache_block = L2_Cache_Block();
	L1_way_count = L1_DCache_Way_Count();
	L2_way_count = L2_Cache_Way_Count();
	write_policy = Cache_Write_Policy();
	Check_Swap_Method();
	system("pause");
	return 0;
}

