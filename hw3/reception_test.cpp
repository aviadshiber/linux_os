#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "ReceptionHour.h"

using namespace std;

int basicTest()
{
	// create reception hour
	printf("Run basic test \n");
	ReceptionHour rh(2);
	rh.startStudent(1);
	StudentStatus status = rh.finishStudent(1);
	bool test_good = false;
	if (status == ENTERED)
	{
		test_good = true;
		printf("The test completed successfully \n");
	}
	else
	{
		printf("fuccccccccccccccccck \n");
	}
	rh.closeTheDoor();
    sleep(3);
	if (test_good)
	{
		printf("basicTest completed successfully\n");
	}
	else
	{
		printf("basicTest completed not good!\n");
	}
	return 1;
}

#define NUM 9
#define MAX_NUM 90

int test2()
{
	StudentStatus status[MAX_NUM+1];
	// create reception hour
	ReceptionHour rh(NUM);

	// start
	for (int i =0; i< MAX_NUM; i++)
	{
		rh.startStudent(i);
	}
	
	
	//printf("******** adding 1 more student *********\n");
	//rh.startStudent(MAX_NUM - 1);
	
	// finish
	for (int i =0; i< MAX_NUM; i++)
	{
		status[i] = rh.finishStudent(i);
		
		printf("******** student[%d].status == %d *********\n", i, status[i]);
	}
	
	// close the reception hour
	printf("******** closing the reception hour door *********\n");
	rh.closeTheDoor();
	
	rh.startStudent(MAX_NUM);
	status[MAX_NUM] = rh.finishStudent(MAX_NUM);
	printf("******** student[%d].status == %d *********\n", MAX_NUM, status[MAX_NUM]);
	// counters
	int count_entered = 0;
	int count_closed = 0;
	int count_no_seat = 0;
	for (int i =0; i<= MAX_NUM; i++)
	{
		if(status[i] == ENTERED)
		{
			count_entered++;
		}
		
		if(status[i] == LEFT_BECAUSE_NO_SEAT)
		{
			count_no_seat++;
		}
		
		if(status[i] == LEFT_BECAUSE_DOOR_CLOSED)
		{
			count_closed++;
		}	
	}
	
	if ((count_entered + count_no_seat == MAX_NUM) && count_closed == 1)
	{
		printf("The test completed successfully\n");
	
		sleep(3);
		printf("test2 completed successfully\n");
	}
	else
	{
		printf("the test failed fuckkkk\n");
		printf("closed %d \n", count_closed);
		printf("others %d \n", count_entered + count_no_seat);
		
		sleep(3);
	}
	return 1;
}

int test3()
{
	printf("Start test3\n");
	ReceptionHour rh(NUM);
	rh.closeTheDoor();
	sleep(3);
	printf("test3 completed successfully\n");
	return 1;
}

int test4()
{
	printf("Start test4\n");
	ReceptionHour rh(NUM);
	rh.startStudent(1);
	rh.startStudent(2);
	rh.finishStudent(1);
	rh.finishStudent(2);
	printf("test4: close the door\n");
	rh.closeTheDoor();
	sleep(3);
	printf("test4 completed successfully\n");
	return 1;
}

int test5()
{
	StudentStatus status[MAX_NUM];
	// create reception hour
	ReceptionHour rh(NUM);

	// start
	for (int i =0; i< MAX_NUM; i++)
	{
		rh.startStudent(i);
	}
	
	
	//printf("******** adding 1 more student *********\n");
	//rh.startStudent(MAX_NUM - 1);
	
	// finish
	for (int i =0; i< MAX_NUM; i++)
	{
		status[i] = rh.finishStudent(i);
		
		printf("******** student[%d].status == %d *********\n", i, status[i]);
	}
	
	// counters
	int count_entered = 0;
	int count_closed = 0;
	int count_no_seat = 0;
	bool test_good = false;
	for (int i =0; i< MAX_NUM; i++)
	{
		if(status[i] == ENTERED)
		{
			count_entered++;
		}
		
		if(status[i] == LEFT_BECAUSE_NO_SEAT)
		{
			count_no_seat++;
		}
		
		if(status[i] == LEFT_BECAUSE_DOOR_CLOSED)
		{
			count_closed++;
		}	
	}
	
	if ((count_entered + count_no_seat == MAX_NUM) && count_closed == 0)
	{
		printf("half test completed successfully\n");
		test_good = true;
	}
	else
	{
		printf("the 1st half test failed fuckkkk\n");
		printf("closed == %d\n", count_closed);
		printf("sum %d \n", count_entered + count_no_seat);
	}
	
	for (int i =0; i< NUM; i++)
	{
		rh.startStudent(MAX_NUM + i);
	}
	
	for (int i =0; i< NUM; i++)
	{
		status[i] = rh.finishStudent(MAX_NUM + i);
		
		printf("******** student[%d].status == %d *********\n", i, status[i]);
	}
	
	count_entered = 0;
	count_closed = 0;
	count_no_seat = 0;
	for (int i =0; i< NUM; i++)
	{
		if(status[i] == ENTERED)
		{
			count_entered++;
		}
		
		if(status[i] == LEFT_BECAUSE_NO_SEAT)
		{
			count_no_seat++;
		}
		
		if(status[i] == LEFT_BECAUSE_DOOR_CLOSED)
		{
			count_closed++;
		}	
	}
	
	if (count_entered == NUM && count_no_seat == 0 && count_closed == 0)
	{
		printf("The test completed successfully\n");
	}
	else
	{
		test_good = false;
		printf("the 2nd half test failed fuckkkk\n");
		printf("closed == %d\n", count_closed);
		printf("sum %d \n", count_entered + count_no_seat);
	}
	
	rh.closeTheDoor();
	
	sleep(3);
	if (test_good)
	{
		printf("test5 completed successfully\n");
	}
	else
	{
		printf("test5 completed not good\n");
	}
	return 1;
}

	
int main(int argc, char* argv[])
{
	// create reception hour
	basicTest();
	test2();
	test3();
	test4();
	test5();
    return 1;
}
