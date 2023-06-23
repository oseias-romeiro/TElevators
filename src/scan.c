// SCAN Disk Scheduling algorithm (https://www.geeksforgeeks.org/scan-elevator-disk-scheduling-algorithms/)
#include <stdio.h>
#include <stdlib.h>

#define LIMIT 100
#define LEFT 0
#define RIGHT 1
#define SIZE 8
#define DISK_SIZE 200

int buf[SIZE];


void SCAN(int arr[], int head, int direction)
{
	int seek_count = 0;
	int distance, cur_track;
	int* left = malloc(sizeof(int) * SIZE);
    int* right = malloc(sizeof(int) * SIZE);
    int left_count = 0;
    int right_count = 0;

	// appending end values
	// which has to be visited
	// before reversing the direction
	if (direction == LEFT)
		left[left_count++] = -2;
	else if (direction == RIGHT)
		right[right_count++] = DISK_SIZE - 1;

	for (int i = 0; i < SIZE; i++) {
		if (arr[i] < head)
			left[left_count++] = arr[i];
		if (arr[i] > head)
			right[right_count++] = arr[i];
	}

	// sorting left and right vectors
	// Sorting left and right arrays
    for (int i = 0; i < left_count - 1; i++) {
        for (int j = 0; j < left_count - i - 1; j++) {
            if (left[j] > left[j + 1]) {
                int temp = left[j];
                left[j] = left[j + 1];
                left[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < right_count - 1; i++) {
        for (int j = 0; j < right_count - i - 1; j++) {
            if (right[j] > right[j + 1]) {
                int temp = right[j];
                right[j] = right[j + 1];
                right[j + 1] = temp;
            }
        }
    }

	// run the while loop two times.
	// one by one scanning right
	// and left of the head
	int run = 2;
    int c = 0;
	while (run--) {
		if (direction == LEFT) {
			for (int i = left_count - 1; i >= 0; i--) {
                cur_track = left[i];
                
                // Appending current track to seek sequence
                //printf("%d\n", cur_track);
                if(cur_track != -2){
                    buf[c] = cur_track;
                    c++;
                }

                // Calculate absolute distance
                distance = abs(cur_track - head);

                // Increase the total count
                seek_count += distance;

                // Accessed track is now the new head
                head = cur_track;
            }
			direction = RIGHT;
		}
		else if (direction == RIGHT) {
			for (int i = 0; i < right_count; i++) {
                cur_track = right[i];
                // Appending current track to seek sequence
                //printf("%d\n", cur_track);
                if(cur_track != -2){
                    buf[c] = cur_track;
                    c++;
                }
                // Calculate absolute distance
                distance = abs(cur_track - head);

                // Increase the total count
                seek_count += distance;

                // Accessed track is now new head
                head = cur_track;
            }
			direction = LEFT;
		}
	}
}

// Driver code
int main()
{
    for (int i = 0; i < SIZE; i++) {
        buf[i] = -1;
    }
	// request array
	int arr[SIZE] = { 176, 79, 34, 60, 92, 11, 41, 114 };
	int head = 50;

	SCAN(arr, head, LEFT);

    for (int i = 0; i < SIZE; i++) {
        printf("%d\n", buf[i]);
    }

	return 0;
}
