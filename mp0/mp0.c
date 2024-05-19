#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int dirs = 0;
int files = 0;

int search(const char* p, const char *key)
{
    int count = 0;

    while(*p)
    {
       if(*p == *key) count++;
       p++;
    }

    return count;
}

int check_dir(char* path, char* key)
{
  int fd;

  if((fd = open(path, 0)) < 0){
    fprintf(1, "%s [error opening dir]\n", path);
    return 1;
  }

  int res = search(path, key);
  printf("%s %d\n", path, res);

  close(fd);
  
  return 0;

}

void tree(char* path, char *key)
{

  char buf[512], *p;
  int fd;
  struct dirent de;  //directory entry
  struct stat st;
  int count = 0;

  //printf("check path: %s\n", path);

  if((fd = open(path, 0)) < 0){
    //printf(" fd: %d\n", fd);
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf+strlen(buf);
  *p++ = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
      //printf("de.inum: %d\n", de.inum);

      if(de.inum == 0)  // no file in this folder
        continue;
      
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      count++;
      
      if(stat(buf, &st) < 0){
        //printf("ls: cannot stat %s\n", buf);
        continue;
      }
    
    
    //printf("st.type: %d\n", st.type);
    switch(st.type){
        case T_FILE:  // file
            files++;
            int res = search(buf, key);
            printf("%s %d\n", buf, res);
            break;

        case T_DIR:  // directory
            if(count > 2){
              dirs++;
              int res = search(buf, key);
              printf("%s %d\n", buf, res);
              tree(buf, key);
            }
            break;
          }
    }

    close(fd);
    

}


void
mp0(char* root_path, char *key)
{
  int child_fd[2];
  pipe(child_fd);

  int pid = fork();

  if(pid < 0)
  {
    printf("Fork fail");
    return;
  }

  if(pid == 0)
  {
    close(child_fd[0]);

    if(check_dir(root_path, key)){
      exit(0);
    };

    tree(root_path, key);
    write(child_fd[1], &dirs, sizeof(int));
    write(child_fd[1], &files, sizeof(int));
    exit(0);

  }else{
    wait(0);

    close(child_fd[1]);
    int total_dir, total_file;
    read(child_fd[0], &total_dir, sizeof(int));
    read(child_fd[0], &total_file, sizeof(int));

    printf("\n");
    printf("%d directories, %d files\n", total_dir, total_file);
  }
 
  return;

}

int
main(int argc, char *argv[])
{

  if(argc != 3){
    printf("Invalid command");
    exit(0);
  }else{
    mp0(argv[1], argv[2]);
  }

  exit(0);
}