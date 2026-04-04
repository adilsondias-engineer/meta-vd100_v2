#include <pthread.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <errno.h>

void* thread_func(void* arg) {
    syslog(LOG_DEBUG, 
        "Simple Hello World from Thread!");
    return NULL;
}

int main() {
    // uname -a via system call, not hardcoded string
  
   struct utsname buffer;

   errno = 0;
   if (uname(&buffer) < 0) {
      perror("uname");
      exit(EXIT_FAILURE);
   }
    
   printf("system name = %s\n", buffer.sysname);
   printf("node name   = %s\n", buffer.nodename);
   printf("release     = %s\n", buffer.release);
   printf("version     = %s\n", buffer.version);
   printf("machine     = %s\n", buffer.machine);
   
    openlog("pthread", LOG_PID, LOG_USER);
	
	
	syslog(LOG_DEBUG,
        "%s %s %s %s %s GNU/Linux", buffer.sysname,buffer.nodename, buffer.release,  buffer.version, buffer.machine );
    syslog(LOG_DEBUG,
        "Hello World from Main!");
    
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
    pthread_join(tid, NULL);
    
    closelog();
    return 0;
}