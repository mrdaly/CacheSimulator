#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//define a cache line to contain a valid bit, a tag and a pointer to another line
typedef struct cacheLine
{
    int valid;
    int tag;
    struct cacheLine* next;
} cacheLine;

//allocate the cache
cacheLine cache[10000];

//function to extract the index of the set this address belongs to
int getIndex(int address, int s, int l, int n)
{
    int numLines = s / l;
    int numSets = numLines / n;

    int numOffsetBits = ceil(log2(l));
    int numIndexBits = ceil(log2(numSets));

    int indexMask = ((1 << numIndexBits) - 1) << numOffsetBits;

    return (address & indexMask) >> numOffsetBits;
}

//function to extract the tag from the address
int getTag(int address, int s, int l, int n)
{
    int numLines = s / l;
    int numSets = numLines / n;

    int numOffsetBits = ceil(log2(l));
    int numIndexBits = ceil(log2(numSets));

    //the tag is all the bits that aren't the index and offset
    int tagMask = ((1 << (numOffsetBits + numIndexBits)) - 1);
    tagMask = ~tagMask;

    return (address & tagMask) >> (numOffsetBits + numIndexBits);
}

int main()
{
    int addresses[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72,
        76, 80, 0, 4, 8, 12, 16, 71, 3, 41, 81, 39, 38, 71, 15, 39, 11, 51, 57, 41};
    int numAddresses = 39;

    int s = 128; //cache size
    int l = 8; //cache line size
    int n = 1; //n-way associative cache

    printf("Cache size: ");
    scanf("%d", &s);
    printf("Cache line size: ");
    scanf("%d", &l);
    printf("Associative cache set size: ");
    scanf("%d", &n);

    int numLines = s / l;
    int numSets = numLines / n;

    //linked lists to implement FIFO for evicting cache lines
    cacheLine* freeList[numSets];
    cacheLine* usedList[numSets];

    //initialize cache
    for (int i = 0; i < numLines; i++)
    {
        cache[i].valid = 0;
        cache[i].tag = 0;
        cache[i].next = NULL;
    }

    // Initialize usedList[] and freeList[]
    for (int set = 0; set < numSets; set++) 
    {
        usedList[set] = NULL;
        freeList[set] = &cache[set * n];
    }
    for (int set = 0; set < numSets; set++) 
    {
        for (int index = set*n; index < (set+1)*n; index++) 
        {
            cache[index].next = & cache[index+1];
        }
        cache[(set+1)*n - 1].next = NULL;
    }


    for (int i = 0; i < numAddresses; i++)
    {
        //get the set and tag from the address
        int set = getIndex(addresses[i], s, l, n);
        int tag = getTag(addresses[i], s, l, n);

        int cacheHit = 0; //boolean
        //loop through the set of the given address
        for (int j = set*n; j < ((set*n) + n); j++)
        {
            //if the cache line is valid and the tag is the same as the given address's
            //tag, we have a hit
            if (cache[j].valid && cache[j].tag == tag)
            {
                cacheHit = 1;
                break;
            }
        }

        if (cacheHit)
        {
            printf("%d - HIT\n", addresses[i]);
        }
        else //cache miss
        {
            printf("%d - MISS\n", addresses[i]); 

            //if there are no free cache lines, evict a line from the set
            if (freeList[set] == NULL)
            {
                cacheLine* line = usedList[set];
                cacheLine* prev = NULL;
                //find the "first in" cache line and put it in the free list
                for (int j = 0; line != NULL; j++)
                {
                    if (line->next == NULL)
                    {
                        if (j == 0)
                            usedList[set] = NULL;
                        else
                            prev->next = NULL;
                        line->valid = 0;
                        line->next = freeList[set];
                        freeList[set] = line;
                        break;
                    }
                    else
                    {
                        prev = line;
                        line = line->next;
                    }
                }
            }

            // Now we know that freeList[set] points to a real cache line
            cacheLine* tmp = freeList[set];
            freeList[set] = tmp->next; // Now freeList[set] is correct.
            tmp->next = usedList[set];
            usedList[set] = tmp; // Now usedList[set] is correct.

            usedList[set]->valid = 1;
            usedList[set]->tag = tag;

        }

    }

}
