#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

// Thanks to Mike Bobadilla

const int PAGE_SIZE = 256;
const int VIRTUAL_MEMORY_SIZE = 256;
const int MEMORY_SIZE = 128;
const int TLB_SIZE = 16;

int main(int argc, char *argv[])
{
  int physical_memory[MEMORY_SIZE];
  int virtual_memory[VIRTUAL_MEMORY_SIZE][2];
  int tlb[TLB_SIZE][2];

  //Got this help from Mike Bobadilla
  // Initial page filling here
  for (int i = 0; i < VIRTUAL_MEMORY_SIZE; i++)
  {
    if (i > MEMORY_SIZE - 1) 
    {
      virtual_memory[i][0] = -1;
    }
    else
    {
      virtual_memory[i][0] = i;
    }
    if (i > MEMORY_SIZE - 1)
    {
      virtual_memory[i][1] = -1;
    }
    else
    {
      virtual_memory[i][1] = MEMORY_SIZE - i;
    }
  }
  for (int i = 0; i < MEMORY_SIZE; i++)
  {
    physical_memory[i] = i;
  }

  for (int i = 0; i < TLB_SIZE; i++)
  {
    if (i > TLB_SIZE - 1)
    {
      tlb[i][0] = -1;
    }
    else
    {
      tlb[i][0] = i;
    }
    if (i > TLB_SIZE - 1)
    {
      tlb[i][1] = -1;
    }
    else
    {
      tlb[i][1] = TLB_SIZE - i;
    }
  }

  // Check the input argument here
  if (argc > 2)
  {
    printf("Too many arguments\n");
    return 1;
  }
  else if(argc < 2)
  {
    printf("Please give an txt file argument\n");
    return 1;
  }

  FILE *pFile;
  pFile = fopen(argv[1], "r"); //Read the input file

  //checks to see if the .txt file supplied is empty
  if (pFile == NULL)
  {
    printf("Error opening a file %s: %s\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  char *line = NULL; // store the reading lines
  size_t len = 0; //length
  ssize_t read;
  // initialize all the variables
  int page_number = 0;
  int physicalAddress = 0;
  int pageFaultCount = 0;
  double pageFaultRate = 0.0;
  double tlbHitRate = 0.0;

  printf("Translating 1000 Logical Addresses: \n\n");
  int tlb_hit_count = 0;

  // read until the end of the file
  while ((read = getline(&line, &len, pFile)) != -1)
  {
    int offset = atoi(line) & 255;
    int page = atoi(line) & 65280;
    int page_table_number = page >> 8; //https://www.geeksforgeeks.org/left-shift-right-shift-operators-c-cpp/#:~:text=%3C%3C%20(left%20shift)%20Takes,number%20of%20places%20to%20shift.
    int tlb_hit = 0;

    for (int i = 0; i < TLB_SIZE; i++)
    {
      if (tlb[i][0] == page_table_number)
      {
        tlb_hit = 1;
        printf("TLB HIT\n");
        tlb_hit_count += 1;
        break;
      }
    }

    if (virtual_memory[page_table_number][0] < 0 && !tlb_hit)
    {
      pageFaultCount++;
      srand(time(NULL));
      int r = rand();

      int largest = 0;
      int temp = 0;
      for (int i = 0; i < VIRTUAL_MEMORY_SIZE; i++)
      {
        if (virtual_memory[i][1] > largest)
        {
          largest = virtual_memory[i][1];
          temp = i;
        }
      }

      int tlb_replacement = r % 15;
      tlb[tlb_replacement][0] = page_table_number;
      tlb[tlb_replacement][1] = virtual_memory[temp][0];
      virtual_memory[page_table_number][0] = virtual_memory[temp][0];
      virtual_memory[page_table_number][1] = 0;
      virtual_memory[temp][0] = -1;
      virtual_memory[temp][1] = 0;
    }

    printf("Virtual Address = %d  \t", page);

    //calculates the physical address
    physicalAddress = (physical_memory[virtual_memory[page_table_number][0]] * PAGE_SIZE) + offset;
    printf("Physical Address: %d\n", physicalAddress);
    page_number++;

    for (int i = 0; i < VIRTUAL_MEMORY_SIZE; i++)
    {
      virtual_memory[i][1]++;
    }
  }

  free(line);
  fclose(pFile);

  //output
  pageFaultRate = (double)pageFaultCount / 1000 * 100;
  printf("Page Fault: %d \n", pageFaultCount);
  printf("Page Fault Rate: %.2f%% \n", pageFaultRate);
  tlbHitRate = (double)tlb_hit_count / 1000 * 100;
  printf("TLB Hit: %d \n", tlb_hit_count);
  printf("TLB Hit Rate: %.2f%% \n", tlbHitRate);

  exit(-1);
}
