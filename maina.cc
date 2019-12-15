// Maina - copy from stdin to file(s) - abort in determined idle time
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <iostream>

using namespace std;

// standard measures, if not defined by argv parameters
#define IDLETIME 10 // seconds
#define MAXLINE 9999

// definitions
typedef unsigned long ulong;
struct timeval tp_start, tp_end;

// global variables

ulong idletime, tidletime;
char *myname;
ulong ms_start, ms_end;

// functions
ulong measuremilliseconds(int flag=0);
void showusage();
void logmessage(char *filename, char *message);
int kbhit(void);

int main(int argc, char *argv[])
{
  int i, c, abortonidle=0, standardoutput=0;
  char line[MAXLINE];
  // standard settings
  myname=argv[0];
  idletime=IDLETIME;

  // parse command line options
  while ((c = getopt(argc, argv, ":t:")) != -1)
   switch (c) {
	case 't':
     if (atoi(optarg))
      idletime=atoi(optarg);
     else
      showusage();
    break;
    case '?':
     showusage();
    break;
    default:
     abort();
  break; }
  // output to stdout or to file
  if (optind==argc)
   standardoutput=1;
  idletime*=1000;
  
   // loop until idle limit
   while (!abortonidle) {
    measuremilliseconds();
    tidletime=0;
    while ((tidletime=measuremilliseconds(1))<idletime) {
     if (kbhit())
    break; }
    if (tidletime==idletime) {
     abortonidle=1;
    break; }
    // read input, direct to outfile
    cin >> line;
     if (standardoutput)
      cout << line << endl;
     else
      for (i=optind;i<argc;i++)
   logmessage(argv[i], line); }
  
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
  cout << myname << " [-t stream idle seconds] [files ...]" << "\n";
 exit (1);
}


void logmessage(char *filename, char *message)
{
  FILE *outlogfile=fopen(filename, "a");
  
   fprintf(outlogfile,"%s\n", message);
  fclose(outlogfile);
}

int kbhit(void)
{
  static int initialized = 0;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
