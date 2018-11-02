#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int data[ 100 ];

//make header file for relay to avoid redeclaring variables and funcs

static int *dataPtr;

int create_buffer();
void display();

extern void* notify_consumers();

struct channelTable {
    int *dataPtr;
    int channelName;
};

struct channelTable* search();

int register_to_produce_data(int channelName, int dataSize) {
    
    //Calls create buffer which creates a shared memory block for publisher to publish to
    //Producer and corresponding data ptr are stored in hashArray

    dataPtr = create_buffer(channelName, dataSize); 

    display();

    return dataPtr;
}


int publish_data(int channelName, int dataSize, int data[]) {

    //Uses hashArray to find where to publish data to then memcpy's to there
    //Calls notify_consumers method of relay
	
    dataPtr = search(channelName)->dataPtr;

    memcpy(dataPtr,data, dataSize);

    notify_consumers(channelName);

    return 0;
}
