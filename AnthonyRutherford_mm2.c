// Anthony Rutherford
// CWID: 12192817

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>
#include <math.h>

#define FRAME_SIZE 256          
//#define FRAMES 256              
#define PAGE_SIZE 256                      
#define PAGE_MASKER 0xFFFF      
#define OFFSET_MASKER 0xFF      
#define ADDRESS_SIZE 10
#define TLB_SIZE 16 

struct page_to_frame_table
{
   int page_number;
   int frame_number;
   int time_since_use;
};

FILE *address_file;
FILE *backing_store;
int firstOpen = 0;
signed char value;
int TLB_Full = 0;
int TLBHits = 0;
int page_faults = 0;
char address [ADDRESS_SIZE]; 
signed char buffer[PAGE_SIZE];
//int Physical_Memory [FRAMES][FRAME_SIZE];
struct page_to_frame_table TLB[TLB_SIZE];
//struct page_to_frame_table PAGE_TABLE[FRAMES]; 


void get_page(int logical_address, struct page_to_frame_table* PT, int** PM, int max_size);
int read_from_store(int pageNumber, struct page_to_frame_table* PT, int** PM, int frames);
void insert_into_TLB(int pageNumber, int frameNumber);

void get_page(int logical_address, struct page_to_frame_table* PT, int** PM, int max_size){
    int pageNumber = ((logical_address & PAGE_MASKER)>>8); 
    int offset = (logical_address & OFFSET_MASKER); 
    int frameNumber = -1; 
    int i;  
    for(i = 0; i < TLB_SIZE; i++){
        if(TLB[i].page_number == pageNumber){   
            frameNumber = TLB[i].frame_number;
            TLB[i].time_since_use = 0; 
            for (int k = 0; k < firstOpen; k++)  {
                    if (PT[k].page_number != pageNumber){
                    PT[k].time_since_use++;
                    }
                }          
            TLBHits++;                          
        }
    }
    if(frameNumber == -1){
        int i;                                                   
        for(i = 0; i < firstOpen; i++){          
            if(PT[i].page_number == pageNumber){             
                frameNumber = PT[i].frame_number; 
                PT[i].time_since_use = 0;    
                for (int k = 0; k < firstOpen; k++)  {
                    if (k != i){
                    PT[k].time_since_use++;
                    }
                }                                     
            }
        }
        if(frameNumber == -1){                                      
            frameNumber = read_from_store(pageNumber, PT, PM, max_size);
            for (int k = 0; k < firstOpen; k++)  {
                    if (PT[k].frame_number != frameNumber){
                        PT[k].time_since_use++;
                    }
                }                                               
            page_faults++;                                        
        }
    }
    insert_into_TLB(pageNumber, frameNumber);                      
    value = PM[frameNumber][offset];                 
    FILE *res = fopen("results.txt","a");
    fprintf(res,"Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frameNumber << 8) | offset, value);
    fclose(res);
}

int read_from_store(int pageNumber, struct page_to_frame_table* PT, int** PM, int frames){
    if (firstOpen < frames){
        int i;
        for(i = 0; i < PAGE_SIZE; i++){
            PM[firstOpen][i] = buffer[i];
        }
        PT[firstOpen].page_number = pageNumber;
        PT[firstOpen].frame_number = firstOpen;
        PT[firstOpen].time_since_use = 0;
    
        firstOpen++;
        return PT[firstOpen-1].frame_number;
    }
    else{
        int max_time = -1;
        int max_location = -1;
        for(int j = 0; j < frames; j++){
            if (PT[j].time_since_use > max_time){
                max_time = PT[j].time_since_use;
                max_location = j;
            }
        }
        int i;
        for(i = 0; i < PAGE_SIZE; i++){
            PM[max_location][i] = buffer[i];
         }
        PT[max_location].page_number = pageNumber;
        PT[max_location].frame_number = max_location;
        PT[max_location].time_since_use = 0;
    
        return max_location;

    }
    
}

void insert_into_TLB(int pageNumber, int frameNumber){
   int i;  
    for(i = 0; i < TLB_Full; i++){
        if(TLB[i].page_number == pageNumber){               
            for(i = i; i < TLB_Full - 1; i++)     
                TLB[i] = TLB[i + 1];                        
            break;
        }
    }
    if(i == TLB_Full){
        int j;
        for (j=0; j<i; j++)
            TLB[j] = TLB[j+1];

    }
    TLB[i].page_number = pageNumber;       
    TLB[i].frame_number = frameNumber;
    if(TLB_Full < TLB_SIZE-1){     
        TLB_Full++;
    }    
}

int main(int argc, char *argv[]) {
    address_file = fopen(argv[1], "r");
    int frames = atoi(argv[2]);
    backing_store = fopen("BACKING_STORE.bin", "rb");
    int addresses_found = 0;
    int logical_address;
    struct page_to_frame_table PAGE_TABLE[frames];
    int **Physical_Memory = (int**)malloc(frames * sizeof(int*));
    for (int i = 0; i < frames; i++){
        Physical_Memory[i] = (int*)malloc(FRAME_SIZE * sizeof(int));
    }

    while (fgets(address, ADDRESS_SIZE, address_file) != NULL) {
        logical_address = atoi(address); 

        get_page(logical_address, &PAGE_TABLE, Physical_Memory, frames);
        addresses_found++; 
    }
    printf("Number of translated addresses = %d\n", addresses_found);
    double pfRate = page_faults / (double)addresses_found;
    double TLBRate = TLBHits / (double)addresses_found;
    
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n",pfRate);
    printf("TLB Hits = %d\n", TLBHits);
    printf("TLB Hit Rate = %.3f\n", TLBRate);
    
    fclose(address_file);
    fclose(backing_store);

    return 0;
}