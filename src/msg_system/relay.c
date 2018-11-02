#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <pthread.h>

#define SIZE 20
#define NUM_CONSUMERS 5

int pthread_create();

void* create_shared_memory(size_t size) {
    
    //Returns data ptr to shared memory which has been allocated to this producer
	
    int protection = PROT_READ | PROT_WRITE;

    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    return mmap(NULL, size, protection, visibility, 0, 0);
}

int consumers [NUM_CONSUMERS];
int rc;

struct channelTable {
    int *dataPtr;
    int channelName;
    int* consumers [NUM_CONSUMERS];
};

struct channelTable* hashArray[SIZE];
struct channelTable* dummyItem;
struct channelTable* item;

int hashCode(int channelName) {
    return channelName % SIZE;
}

struct channelTable *search(int channelName) {

    //Searches hashArray and returns channelTable object corresponding to channelName (if that object exists)

    //get the hash
    int hashIndex = hashCode(channelName);

    //move in array until an empty spot is found
    while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->dataPtr != NULL) {
        
	if(hashArray[hashIndex]->channelName == channelName)
            return hashArray[hashIndex];

	++hashIndex;		//go to next cell
	hashIndex %= SIZE; 	//wrap around the table
    }

    return NULL;
}

void insert_producer(int channelName, int *dataPtr) {

    //Makes entry into hashArray containing channnelName and data ptr

    struct channelTable *item = (struct channelTable*) malloc(sizeof(struct channelTable));
    item->dataPtr = dataPtr;
    item->channelName = channelName;
    int *consumers [ NUM_CONSUMERS ] = {};
    strcpy(item->consumers, consumers);

    int hashIndex = hashCode(channelName);		//get the hash

    //move in array until an empty or deleted cell is found
    while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->channelName != -1) {
        ++hashIndex;		//go to the next cell
	hashIndex %= SIZE;	//wrap around the table

    }
    hashArray[hashIndex] = item;
}

void insert_consumer(int channelName, void* consumer) {
    
    //Adds consumer data callback to hashArray corresponding to desired channel

    int hashIndex = hashCode(channelName);
    while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->channelName != channelName) {
        ++hashIndex;
        hashIndex %= SIZE;
    }

    printf("consumer = %p\n", consumer);
    int i;
    for(i=0;i < NUM_CONSUMERS; i++) {
        
        if(hashArray[hashIndex]->consumers[i] == NULL) {
	    hashArray[hashIndex]->consumers[i] == consumer;
	    return;
	}
    }
}

struct channelTable* delete(struct channelTable* item) {

    //Delete desired item from hashArray (eg: producer is done producing)

    int channelName = item->channelName;

    int hashIndex = hashCode(channelName);		//get the hash

    //move in array until non-empty cell is found
    while(hashArray[hashIndex] != NULL) {
        
        if(hashArray[hashIndex]->channelName == channelName) {
	    struct channelTable* temp = hashArray[hashIndex];

	    hashArray[hashIndex] = dummyItem;		//assign a dummy item at deleted position
	    return temp;
	}

    ++hashIndex;		//go to next cell
    hashIndex %= SIZE;		//wrap around the table

    }
    return NULL;
}

void display() {

    //Iterates through hashArray and creates visual representation (used for debugging)

    int i = 0;
//    int n = 0;
    for(i = 0; i<SIZE; i++) {

        if(hashArray[i] != NULL) {
	    printf(" (%d,%p,%p)", hashArray[i]->channelName, hashArray[i]->dataPtr,hashArray[i]->consumers);
//	    for(n=0; n < NUM_CONSUMERS; n++) {
//              if (hashArray[i]->consumers[n] != NULL) {
//		    printf(" (%p) ", hashArray[i]->consumers[n]);
//		}
//	    }
	}
        else
	    printf(" ~~ ");
    }
    printf("\n");
}

void display_consumers(int channelName) {
 
    //Displays consumers registered to channel
     
    strcpy(consumers, search(channelName)->consumers);

    for(int i=0; i<NUM_CONSUMERS; i++) {
        if(consumers[i]!=NULL) {
	    printf("%d,", consumers[i]);
	}
	else {
	    printf("~~~,");
	}
    }
    printf("\n");
    }

	   

void *notify_consumers(int channelName,int *dataPtr) {
   
    //Creates thread for each consumer callback subscribed to a channel
    //***Error here -- seems to not actually be looking into consumer data structure***

    strcpy(consumers, search(channelName)->consumers);
    pthread_t threads [sizeof(consumers)];
    
    printf("Inside of notify_consumers %i\n", sizeof(consumers));

    int i;
    display_consumers(channelName);
    for(i=0; i < sizeof(consumers); i++) {
	if(consumers[i] != NULL) {
            printf("consumer notified = %p\n", consumers[i]);
	    rc = pthread_create(&threads[i], NULL, &(consumers[i]), (int *) dataPtr);  
    	    if (rc)  {
	        printf("Error: unable to create thread");
	    }
        }
    }
}

int create_buffer(int channelName, int dataSize) {
   
    //Returns data ptr to shared memory allocated to channel

    int* dataPtr = create_shared_memory(dataSize*8);

//    printf("data pointer (from relay) = %p\n", dataPtr);

    insert_producer(channelName, dataPtr);

//    pthread_t threads[1];

//    pthread_create(&threads[0], NULL, &notify_consumers, (int *) channelName);

    return dataPtr;

}

