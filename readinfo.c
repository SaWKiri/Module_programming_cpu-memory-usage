/*
*
* Helper program to read cpu info and memory for
* sys_status_module.
*
* this app will read from /proc and calculate ram use in the system and cpu usage
* this info will be send to sys_status_module.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/klog.h>



#define READ_BUFFER 128
#define NEW_LINE 10
#define ASCII_OF_ZERO 48
#define ASCII_OF_NINE 57
#define ASCII_OF_SPACE 32
#define ASCII_OF_NEW_LINE 10
#define SUCCESS 0
#define ERR_READ_FAIL -1
#define ERR_WRITE_FAIL -2
#define ERR_OPEN_FAIL -3

double read_mem(void)
{
  int fd,i=0, j=0, _total = 0, _free = 0;
  double memory_used_precentage = 0;
  char buffer[READ_BUFFER];
  char memory_data[READ_BUFFER], total_memory[READ_BUFFER]={0},mem_free[READ_BUFFER]={0};

  fd = open("/proc/meminfo",O_RDONLY);

  if(fd==0)
  {
    return ERR_OPEN_FAIL;
  }

  flock(fd,LOCK_EX);
  if(read(fd,&buffer,READ_BUFFER) <= 0)
  {
    flock(fd,LOCK_UN);
    return ERR_READ_FAIL;
  }
  flock(fd,LOCK_UN);

  //read first line - total memory
  while(buffer[++i] != ASCII_OF_NEW_LINE)
  {
    if(buffer[i] >= ASCII_OF_ZERO && buffer[i] <= ASCII_OF_NINE)
    {
      total_memory[j++]=buffer[i];
    }
  }
  total_memory[j] = '\0';
  j=0;

  //skipping the 2nd row
  i = i + 28;

  //readind available memory
  while(buffer[++i] != ASCII_OF_NEW_LINE)
  {
    if(buffer[i] >= ASCII_OF_ZERO && buffer[i] <= ASCII_OF_NINE)
    {
      mem_free[j++]=buffer[i];
    }
  }
  mem_free[j] = '\0';

  //convert to string to int
  _total = atoi(total_memory);
  _free = atoi(mem_free);


  printf("\n\nmemory usage: %lf\n",100.0*((double)(_total - _free)/((double) _total)) );
  memory_used_precentage = ((double)(_total - _free) / (double)_total) * 100.0;
  printf("memory calculated: %lf\n",memory_used_precentage);

return memory_used_precentage;
}

int write_memory(int memory)
{
  int fd;
  char precentege_as_array[READ_BUFFER] = { 0 };

  fd = open("/sys/sys_status_module/memory",O_WRONLY);

  if(fd == 0)
  {
    printf("%s\n","failed open sysfs memory" );
    return ERR_OPEN_FAIL;
  }

  //converting integer to string
  sprintf(precentege_as_array,"%d",memory);

  flock(fd,LOCK_EX);
  if(write(fd,precentege_as_array,sizeof(char)*strlen(precentege_as_array)) < 0)
  {
    printf("write failed: failed to store memory");
    flock(fd,LOCK_UN);
    close(fd);
    return ERR_WRITE_FAIL;
  }
  flock(fd,LOCK_UN);

  //closing file
  close(fd);

 //returning number of byte write
  return sizeof(char)*strlen(precentege_as_array);


}

//previos of cpu snapshot - to more accuratly calculate cpu usage
static long int PrevIdle = 0 , PrevTotal =0 , PrevNonIdle = 0;

int read_cpu(void)
{
  char buffer[READ_BUFFER],number[READ_BUFFER];
  long int cpu_data[10] = {0};
  int fd;//file descriptor for reading cpu stat from /proc/stat
  int i=5,j=0,k=0;

  long int  Idle , Total , NonIdle;
  long int totald =0 , idled =0;

  int cpu_usage_in_precentage;

  if((fd = open("/proc/stat",O_RDONLY)) == 0)
  {
    close(fd);
    return ERR_OPEN_FAIL;
  }

  flock(fd,LOCK_EX);
  if(read(fd,&buffer,READ_BUFFER) < 0)
  {
    flock(fd,LOCK_UN);
    close(fd);
    return ERR_READ_FAIL;
  }
  flock(fd,LOCK_UN);

  /*
  * copy first line.
  * skiping the 5 first character (i=4), it will be allways "cpu  "
  * reading each number as a string and converting them to integer
  * all number going into cpu[] array in the same read order
  */
  while(buffer[i] != NEW_LINE)
  {

    if((buffer[i] >= ASCII_OF_ZERO && buffer[i] <= ASCII_OF_NINE) || buffer[i] == ASCII_OF_SPACE)
    {
      number[j++] = buffer[i];
    }

    i++;
    /*
    * if space or new line has been read reading number has ended
    * convering the string to number
    * resetting number string to 0
    */
    if(buffer[i] == ASCII_OF_SPACE || buffer[i] == ASCII_OF_NEW_LINE)
    {
      number[j] = '\0';//putting end string in the end of the number for atoi()
      cpu_data[k++] = atol(number);//convert string number to long int
      j=0;
      //0-ing "number" array for next string to read
      memset(number, 0, READ_BUFFER);
    }

  }//end of while loop

  Idle = cpu_data[3] + cpu_data[4];
  NonIdle = cpu_data[0] + cpu_data[1] + cpu_data[2] + cpu_data[5] + cpu_data[6] + cpu_data[7];

  Total = Idle + NonIdle;

  totald = Total - PrevTotal;
  idled = Idle - PrevIdle;
  printf("debug: totald: %ld, idled: %ld\ntotald - idled = %ld\nprecent:%lf\n",totald,idled,totald - idled, 100.0*((double)(totald-idled))/((double)(totald)));
  cpu_usage_in_precentage = 100.0*((double)(totald-idled))/((double)(totald));

  //saving last snapshot
  PrevIdle = Idle;
  PrevTotal = Total;

  printf("cpu precent as int:%d\n",cpu_usage_in_precentage);
  return cpu_usage_in_precentage;
}

int write_cpu(int cpu)
{
  int fd,j=0;
  char precentege_as_array[READ_BUFFER];

  fd = open("/sys/sys_status_module/cpu",O_WRONLY);

  if(fd == 0)
  {
    return ERR_OPEN_FAIL;
  }

  //converting integer to string
  sprintf(precentege_as_array,"%d",cpu);
  flock(fd,LOCK_EX);
  if(write(fd,precentege_as_array,sizeof(char)*strlen(precentege_as_array)) < 0)
  {

    flock(fd,LOCK_UN);
    close(fd);
    return ERR_WRITE_FAIL;
  }
  flock(fd,LOCK_UN);
  //closing file
  close(fd);

 //returning number of byte write
  return sizeof(char)*strlen(precentege_as_array);


}


int main(int argc , char* argv[])
{
  /* variable to hold the system status read from /proc*/
  double cpu_usage_in_precentage = 0;
  double memory_usage_in_precentage = 0;
  int to_run = 0;//if to_run = 0: run and write info to sysfa, to_run == 1: run but dont read/write info, otherwise program end.
  int fd;
  char buffer =  0 ;



  do
  {
    fd=open("/sys/sys_status_module/to_run",O_RDWR);

    flock(fd,LOCK_EX);
    read(fd,&buffer,1);
    flock(fd,LOCK_UN);
	  close(fd);
    to_run = atoi(&buffer);

    memset(&buffer,0,1);

    if(to_run != 1){

      memory_usage_in_precentage = read_mem();


      write_memory(memory_usage_in_precentage);

      cpu_usage_in_precentage = read_cpu();


     write_cpu(cpu_usage_in_precentage);

   }
    sleep(1);


  }while(to_run == 0 || to_run == 1);

return SUCCESS;
}
