// goliath daemon - watch file, remove periodically if it exceeds requested size
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sstream>
#include <string>

using namespace std;

// standard measures, if not defined by argv parameters
#define WATCHSIZE 1024^2 // kbytes
#define WATCHTIME 60 // seconds
#define LOGFILE "goliath.log" // keep logfile

// definitions
typedef unsigned long ulong;
struct timeval tp_start, tp_end;

// global variables
int period=1, keeplogfile=1, quitonemptyfile=1, c;
ulong watchtime, twatchtime, watchsize, sizenow;
char *myname, *watchfile;
ulong ms_start, ms_end;

// functions
ulong measuremilliseconds(int flag=0);
void showusage();
void daemonize();
void logmessage(string message);
char* bringdatetimestamp(int flag=0);

int main(int argc, char *argv[])
{  
  // standard settings
  myname=argv[0];
  watchtime=WATCHTIME;
  watchsize=WATCHSIZE;
  
  // parse command line options
  while ((c = getopt(argc, argv, ":qlt:s:")) != -1)
   switch (c) {
    case 'l':
     keeplogfile=0;
    break;
    case 'q':
     quitonemptyfile=0;
    break;
	case 't':
     if (atoi(optarg))
      watchtime=atoi(optarg);
     else
      showusage();
    break;
    case 's':
     if (atoi(optarg))
      watchsize=atoi(optarg);
     else
      showusage();
    break;
    case '?':
     showusage();
    break;
    default:
     abort();
  break; }
  
  // file to keep track for size
  if (optind == argc)
   showusage();
  watchfile=argv[optind];
  
   ostringstream outlog;
   outlog << "{ " << bringdatetimestamp() << " } "<< " Goliath daemon starts " << endl << "keeping watch to " <<  watchfile << ", for size " << watchsize << " kbytes, every " << watchtime << " seconds" << endl;
   logmessage(outlog.str());
   watchtime*=1000;
   
   daemonize();
   while (1)
    sleep(1);

 return 0;
}

// chronographer for milliseconds
ulong measuremilliseconds(int flag)
{
  if (!flag) {
   gettimeofday(&tp_start, NULL);
   ms_start=tp_start.tv_sec * 1000 + tp_start.tv_usec / 1000;
  return ms_start; }
  else {
   gettimeofday(&tp_end, NULL);
  ms_end=tp_end.tv_sec * 1000 + tp_end.tv_usec / 1000; }

 return ms_end-ms_start;
}

// show daemon usage
void showusage()
{
  cout << myname << " [-l no logfile] [-q never exit] [-t seconds] [-s kbytes] [file]" << "\n";
 exit (1);
}

void daemonize()
{
   int i;
	if(getppid()==1) return; /* already a daemon */
	 i=fork();
	if (i<0) exit(1); /* fork error */
	if (i>0) exit(0); /* parent exits */
	/* child (daemon) continues */
	setsid(); /* obtain a new process group */
	for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
    i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
	umask(027); /* set newly created file permissions */
    while (1) {
    // start milliseconds measurement
    measuremilliseconds();
    twatchtime=0;
    while ((twatchtime=measuremilliseconds(1))<watchtime);
    fstream watchbird(watchfile, ios::in | ios::out | ios::binary);
    if (!watchbird) {
     ostringstream outlog;
     outlog << "{" << bringdatetimestamp(1) << "} " << watchfile << " is not there";
     if (quitonemptyfile) 
      outlog<< ", Goliath aborting";
     outlog << endl;
     logmessage(outlog.str());
     if (quitonemptyfile)
    exit (1); }
    watchbird.seekg(0, ios::end);
    sizenow=watchbird.tellg();
    watchbird.close();
    if ((sizenow/1024)>watchsize) {
     ostringstream outlog;
     outlog << "{" << bringdatetimestamp(1) << "} period " << period << " - removing, size reached " << sizenow << " bytes"<< endl;
     logmessage(outlog.str());
    remove(watchfile); }
    ++period; }

	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
}

void logmessage(string message)
{
  FILE *logfile=fopen(LOGFILE, "a");
  
   if(!logfile || !keeplogfile) 
    return;
  
   fprintf(logfile,"%s", message.c_str());
  fclose(logfile);
}

// bring current datetime stamp
char* bringdatetimestamp(int flag) // 1 time only
{
  int i=0;
  time_t now = time(0);
  static char stamp[50];

  strcpy(stamp, ctime(&now));
  stamp[strlen(stamp)-6]='\0';
  if (flag)
   i=11;
      
 return &stamp[i];  
}
