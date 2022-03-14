#include <iostream>
#include <time.h>
#include <Windows.h>

using namespace std;

#define ARRAY_SIZE (1 << 28) // test array size is 2^28
typedef unsigned char BYTE;	 // define BYTE as one-byte type
#define SIZE_TEST_TIMES 123456789
#define WAY_TEST_TIMES 99999

BYTE array[ARRAY_SIZE]; // test array
int L1_cache_size = 1 << 15;
int L2_cache_size = 1 << 18;
int L1_cache_block = 64;
int L2_cache_block = 64;
int L1_way_count = 8;
int L2_way_count = 4;
int write_policy = 0; // 0 for write back ; 1 for write through

// have an access to arrays with L2 Data Cache'size to clear the L1 cache
void Clear_L1_Cache()
{
	memset(array, 0, L2_cache_size);
}

// have an access to arrays with ARRAY_SIZE to clear the L2 cache
void Clear_L2_Cache()
{
	memset(&array[L2_cache_size + 1], 0, ARRAY_SIZE - L2_cache_size);
}

int Test_Cache_Size(int index, int *avg_time)
{
	int size;
	int data_size;
	char data;
	clock_t begin, end;
	size = index;
	for (int i = 0; i < 5; i++)
	{
		data_size = (1 << (size + 10));
		Clear_L1_Cache();
		Clear_L2_Cache();
		begin = clock();
		for (int j = 0; j < SIZE_TEST_TIMES; ++j)
		{
			data += array[(rand() * rand()) % data_size];
		}
		end = clock();
		avg_time[i] = end - begin;
		size++;
	}

	int temp, process_time;
	size = index;
	int cache_size_result = (1 << size);
	process_time = avg_time[1] - avg_time[0];
	for (int i = 0; i < 5; i++)
	{
		cout << "Test_Array_Size = " << (1 << size) << "KB, ";
		cout << "Average access time = " << avg_time[i] << "ms" << endl;
		if (i < 4)
		{
			temp = avg_time[i + 1] - avg_time[i];
			if (temp > process_time)
			{
				cache_size_result = (1 << size);
				process_time = temp;
			}
		}
		size++;
	}
	return cache_size_result;
}

int Test_Cache_Block(int index, int *avg_time)
{

	int size;
	int data_size;
	char data;
	clock_t begin, end;
	unsigned int temp;
	temp = index;
	for (int i = 0; i < 8; ++i)
	{
		begin = clock();
		for (int j = 0; j < temp; ++j)
		{
			for (int k = 0; k < ARRAY_SIZE; k += temp)
			{
				data += array[k];
			}
		}
		end = clock();
		avg_time[i] = end - begin;
		temp = temp << 1;
	}

	temp = index;
	int process_time = avg_time[1] - avg_time[0], temp_time;
	int cache_block_result;
	for (int i = 0; i < 8; ++i)
	{
		cout << "Block_Size = " << temp << " B, ";
		cout << "Average access time = " << avg_time[i] << "ms " << endl;
		if (i < 7)
		{
			temp_time = avg_time[i + 1] - avg_time[i];
			if (temp_time > process_time)
			{
				cache_block_result = (1 << size);
				process_time = temp_time;
			}
		}
		size++;
		temp = temp << 1;
	}
	return cache_block_result;
}

int Test_Cache_Way_Count(int array_size, int index, int *avg_time)
{
	int temp;
	int array_jump;
	char data;
	clock_t begin, end, temp_time;
	int process_time, way_count_result;
	temp = index;
	for (int i = 0; i < 5; ++i)
	{
		array_jump = array_size / temp;
		begin = clock();
		for (int j = 0; j < WAY_TEST_TIMES; ++j)
		{
			for (int k = 0; k < temp; k += 2)
			{
				memset(&array[k * array_jump], 0, array_jump);
			}
		}
		end = clock();
		avg_time[i] = end - begin;
		temp = temp << 1;
	}

	temp = index;
	process_time = avg_time[1] - avg_time[0];
	way_count_result = temp / 2;
	for (int i = 0; i < 5; ++i)
	{
		cout << "Way_Count = " << temp / 2 << ", ";
		cout << "Average access time = " << avg_time[i] << "ms " << endl;
		if (i < 4)
		{
			temp_time = avg_time[i + 1] - avg_time[i];
			if (temp_time > process_time)
			{
				way_count_result = temp / 2;
				process_time = temp_time;
			}
		}
		temp = temp << 1;
	}
	return way_count_result;
}

int L1_DCache_Size()
{
	cout << "*****************************************************" << endl;
	cout << "L1_Data_Cache_Test" << endl;
	int avg_time[5];
	int index = 3;
	int result;
	result = Test_Cache_Size(index, avg_time);
	cout << "L1_Data_Cache_Size is " << result << "KB" << endl;
	cout << "*****************************************************" << endl;
	return result << 10;
}

int L2_Cache_Size()
{
	cout << "*****************************************************" << endl;
	cout << "L2_Data_Cache_Test" << endl;
	int avg_time[5];
	int index = 6;
	int result;
	result = Test_Cache_Size(index, avg_time);
	cout << "L2_Cache_Size is " << result << "KB" << endl;
	cout << "*****************************************************" << endl;
	return result << 10;
}

int L1_DCache_Block()
{
	cout << "*****************************************************" << endl;
	cout << "L1_DCache_Block_Test" << endl;
	int index = 1;
	int avg_time[8];
	int result;
	result = Test_Cache_Block(index, avg_time);
	cout << "L1_Data_Block_Size is " << result << "B" << endl;
	cout << "*****************************************************" << endl;
	return result;
}

int L2_Cache_Block()
{
	cout << "*****************************************************" << endl;
	cout << "L2_Cache_Block_Test" << endl;
	int index = 1;
	int avg_time[8];
	int result;
	result = Test_Cache_Block(index, avg_time);
	cout << "L2_Block_Size is " << result << "B" << endl;
	cout << "*****************************************************" << endl;
	return result;
}

int L1_DCache_Way_Count()
{
	cout << "*****************************************************" << endl;
	cout << "L1_DCache_Way_Count" << endl;
	int array_size = L1_cache_size << 1;
	int index = 2;
	int avg_time[5];
	int result;
	result = Test_Cache_Way_Count(array_size, index, avg_time);
	cout << "L1_DCache_Way_Count is " << result << endl;
	cout << "*****************************************************" << endl;
}

int L2_Cache_Way_Count()
{
	cout << "*****************************************************" << endl;
	cout << "L2_Cache_Way_Count" << endl;
	int array_size = L2_cache_size << 1;
	int index = 2;
	int avg_time[5];
	int result;
	result = Test_Cache_Way_Count(array_size, index, avg_time);
	cout << "L2_Way_Count is " << result << endl;
	cout << "*****************************************************" << endl;
}

int main()
{
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
//	L1_cache_size = L1_DCache_Size();
//	L2_cache_size = L2_Cache_Size();
//	L1_cache_block = L1_DCache_Block();
//	L2_cache_block = L2_Cache_Block();
	L1_way_count = L1_DCache_Way_Count();
	L2_way_count = L2_Cache_Way_Count();
	system("pause");
	return 0;
}
