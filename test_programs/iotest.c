#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FILE_SIZE 2048
#define WRITE_SIZE 32

char FileData[FILE_SIZE];

void CreateFile(char *filename)
{
   FILE *file;

   if (file = fopen(filename,"w")) {
      memset(FileData,'0',FILE_SIZE);
      memset(FileData,'-',WRITE_SIZE);
      fwrite(FileData,1,FILE_SIZE,file);
      fclose(file);
   }
}

void ReadWriteFile(char *filename)
{
   FILE *file;

   if (file = fopen(filename,"r+")) {
      fseek(file,0,SEEK_SET);
      fread(FileData,1,FILE_SIZE,file);
      fseek(file,0,SEEK_SET);
      memset(FileData,'1',WRITE_SIZE);
      fwrite(FileData,1,WRITE_SIZE,file);
      fclose(file);
   }
}

int main(int argc, char **argv)
{
   if (argc > 1) {
      CreateFile(argv[1]);
      ReadWriteFile(argv[1]);
   }

   return 0;
}
