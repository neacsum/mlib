/*!
  \file log.cpp Implementation of syslog functions.

*/
#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/defs.h>
#include <windows.h>
#include <mlib/log.h>
#include <psapi.h>
#include <mlib/trace.h>
#include <stdio.h>
#include <mlib/utf8.h>
#include <mlib/profile.h>


#ifdef _MSC_VER
# pragma comment (lib, "psapi.lib")
# pragma comment (lib, "ws2_32.lib")
#endif

/*!
  \defgroup syslog Syslog Emulator
  \brief Emulation of Unix syslog functionality.

  For the most part the functions are compatible with standard BSD syslog
  functions. The only significant addition is the set_log_ini_param function
  that allows control of syslog options from an INI file.
*/

#define LOG_DGRAM_SIZE 1024


#define SERVER_INI_KEY        "LogServername"
#define FILE_INI_KEY          "LogFilename"
#define FLAGS_INI_KEY         "LogOptions"
#define PRI_INI_KEY           "LogPriorityMask"

#define DEFAULT_FLAGS         0               //only UDP data log
#define DEFAULT_PRIMASK       0xff            //everything
#define DEFAULT_SERVERNAME    "localhost"
#define DEFAULT_FACILITY      LOG_USER

static char local_hostname[_MAX_PATH];

//values loaded from INI file
static int defopt = DEFAULT_FLAGS;
static int defmask = DEFAULT_PRIMASK;
static char *server_hostname = 0;
static char *logfname = 0;


//logger data
struct LOG {
  DWORD pid;
  int mask;       //priority mask
  int facility;   //default facility
  int option;
  FILE *file;
  SOCKADDR_IN sa_logger;
  SOCKET sock;
  char *ident;
  char str_pid[10];
};

static LOG *proclog = 0;    //default process log

static char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

//prototypes
static void logger_addr (LOG *plog);
static void init ();
static void dnit ();
static const char *processname (char *name);


static void init ()
{
  WSADATA wsd;
  DWORD namesz = 0;
  wchar_t whost[_MAX_PATH];
  if (proclog)
    return;     //been here before

  atexit (dnit);
  whost[0] = 0;
  namesz = _countof(whost);

  if( WSAStartup( MAKEWORD( 2, 2 ), &wsd ) )
  {
    //failed to initialize WINSOCK - will have to live with netbios and files
    GetComputerName (whost, &namesz);
  }
  else
  {
    if (!GetComputerNameEx (ComputerNameDnsFullyQualified, whost, &namesz))
      GetComputerName (whost, &namesz);
      
    if (!server_hostname)
      server_hostname = strdup (DEFAULT_SERVERNAME);
  }
  strcpy(local_hostname, utf8::narrow(whost).c_str());
  proclog = (LOG*)malloc (sizeof(LOG));
  //init LOG structure
  memset (proclog, 0, sizeof (LOG));
  proclog->pid = GetCurrentProcessId();
  proclog->mask = defmask;
  proclog->option = defopt;
  proclog->sock = INVALID_SOCKET;
}

static void dnit ()
{
  closelog ();

  WSACleanup ();
  if (server_hostname)
  {
    free (server_hostname);
    server_hostname = NULL;
  }
  free (proclog);
  proclog = 0;
}


static const char *processname (char *name)
{
  static char pname[256];
  static bool got_pname = false;
  
  if (!got_pname)
  {
    char *p;
    wchar_t wpn[256];
    if (!GetModuleBaseName (GetCurrentProcess(), 0, wpn, sizeof(wpn)))
      strcpy (pname, "unknown");
    else
    {
      strncpy (pname, utf8::narrow(wpn).c_str(), sizeof(pname)-1);
      pname[sizeof(pname)-1] = 0;
    }
    if (p = strchr (pname, '.'))
      *p = 0;
    got_pname = true;
  }
  if (name)
    strcpy (name, pname);
  return pname;
}


/**
  \ingroup syslog
  \param inifile    name of INI file
  \param section    section of the INI file

  If set_log_ini_param is called before a call to openlog,  the default values
  for server name, log options, and logmask are replaced with settings from the
  INI file. The following entries in the INI file are used:
\verbatim
  LogServername=<host name or IP address[:<port number>]
  LogFilename=<file name>
  LogOptions=<value>
  LogPriorityMask=<value>
\endverbatim
  The default values for these settings are:
  - LogServerName=localhost:514
  - LogFilename=program name.log
  - LogOptions=0
  - LogPriorityMask=0xff

  If set_log_ini_param is called after openlog the new settings will not be
  used until a call to closelog.

  \note 
  The options specified in openlog() are OR'ed with the options specified in
  the INI file, hence you can turn on file logging or debug string messages
  (OutputDebugString) just by changing the settings in the INI file.
*/
void set_log_ini_param (const char* inifile, const char *section)
{
  TRACE ("Setting syslog ini params from %s [%s]", inifile, section);
  MLIBSPACE::Profile ini(inifile);
  char tmp[256];
  if (!ini.GetString (tmp, sizeof(tmp), SERVER_INI_KEY, section))
    _snprintf (tmp, sizeof(tmp), "%s:%d", DEFAULT_SERVERNAME, LOG_PORT);
  if (server_hostname)
    free (server_hostname);
  server_hostname = strdup (tmp);

  if (!ini.GetString (tmp, sizeof(tmp), FILE_INI_KEY, section))
    _snprintf (tmp, sizeof(tmp), "%s.log", processname(0));
  if (logfname)
    free (logfname);
  logfname = strdup (tmp);

  defopt = ini.GetInt (FLAGS_INI_KEY, section);

  int itmp = ini.GetInt (PRI_INI_KEY, section);
  if (itmp = atoi(tmp))
    defmask = itmp;
}


/**  
  \ingroup syslog
  \param ident      string prepended to every message, typically
                    set to the program name.

  \param option     flags which control the operation of subsequent calls 
                    to syslog().

  \param facility   establishes a default to be used if none is specified 
                    in subsequent calls to syslog()

  The use of openlog is optional; it will automatically be called by syslog()
  if necessary, in which case \p ident will default to NULL.

  If \p ident is NULL, or if openlog is not called, the default identification 
  string used in syslog messages will be the program name.

  You can cause the syslog to drop the reference to \p ident and go back to the 
  default string (the program name), by calling closelog().
*/
void openlog (const char* ident, int option, int facility)
{
  if (!proclog)
    init ();

  if (ident)
    proclog->ident = strdup (ident);
  proclog->facility = facility? facility : DEFAULT_FACILITY;
  proclog->option = option | defopt;
  proclog->sock = INVALID_SOCKET;
  if( proclog->option & LOGOPT_PID )
    _snprintf( proclog->str_pid, sizeof(proclog->str_pid), "[%lu]", GetCurrentProcessId() );
  else
    proclog->str_pid[0] = 0;

  if (proclog->option & LOGOPT_FILE)
  {
    char fname[MAX_PATH];
    if (!logfname)
    {
      _snprintf (fname, MAX_PATH, ".\\%s.log", proclog->ident);
      logfname = strdup(fname);
    }
    proclog->file = fopen (logfname, "ac");
  }

  if (!(proclog->option & LOGOPT_NOUDP))
  {
    logger_addr (proclog);
    proclog->sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if( INVALID_SOCKET == proclog->sock )
      goto done;

    if (proclog->sa_logger.sin_addr.S_un.S_addr == INADDR_BROADCAST)
    {
      BOOL opt = true;
      setsockopt (proclog->sock, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof (opt));
    }
  }

done:
  ;
}

static void logger_addr(LOG *plog)
{
  char host[MAX_PATH];
  char *p;
  struct hostent * phe;

  strcpy (host, server_hostname);
  plog->sa_logger.sin_family = AF_INET;
  
  p = strchr( host, ':' );
  if( p )
    *p++ = 0;

  phe = gethostbyname( host );
  if( !phe )
    goto use_default;

  memcpy( &plog->sa_logger.sin_addr.s_addr, phe->h_addr, phe->h_length );

  if( p )
    plog->sa_logger.sin_port = htons( (unsigned short) strtoul( p, NULL, 0 ) );
  else
    plog->sa_logger.sin_port = htons( LOG_PORT );
  return;

use_default:
  plog->sa_logger.sin_addr.S_un.S_addr = htonl( 0x7F000001 );
  plog->sa_logger.sin_port = htons( LOG_PORT );
  plog->sa_logger.sin_family = AF_INET;
}

/*!
  \ingroup syslog
  closelog also resets the identification string for syslog messages back to 
  the default, if openlog was called with a non-NULL argument to ident. 
  The default identification string is the program name.
*/
void closelog ()
{
  if (!proclog)
    return;

  if (proclog->file)
  {
    fclose (proclog->file);
    proclog->file = 0;
  }
  if (proclog->sock != INVALID_SOCKET)
  {
    closesocket (proclog->sock);
    proclog->sock = INVALID_SOCKET;
  }
  if (proclog->ident)
  {
    free (proclog->ident);
    proclog->ident = 0;
  }
}

/**
  \ingroup syslog
  \param facility_priority combined facility and priority value. See below.
  \param fmt                printf-like format string
  \param ...                any other arguments required by format

  If the corresponding priority bit in log mask is set, (see setlogmask())
  syslog submits the message with the facility and priority for processing.

  The message is prepended with an identification string set in openlog() or
  by the program name if openlog hasn't been called.

  The macro LOG_MAKEPRI generates a facility/priority from  a facility and a
  priority, as in the following example:

          LOG_MAKEPRI(LOG_USER, LOG_WARNING)
 
  Example:
\verbatim
       syslog (LOG_MAKEPRI(LOG_USER, LOG_ERROR),
          "Unable to make network connection to %s.", host );
\endverbatim
 */
void syslog (int facility_priority, char* fmt, ...)
{
  SYSTEMTIME stm;
  va_list ap;
  int len;
  char datagram[LOG_DGRAM_SIZE + 3];
  if (!proclog)
    openlog (NULL, 0, 0);

  int prm = LOG_MASK (facility_priority & LOG_PRIMASK);
  if (!(prm & proclog->mask))
    return;

  if (!(facility_priority & LOG_FACMASK))
    facility_priority |= proclog->facility;

  va_start (ap, fmt);
  GetLocalTime (&stm);
  len = sprintf (datagram, "<%d>%s %2d %02d:%02d:%02d %s %s%s: ",
                 facility_priority,
                 month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond,
                 local_hostname,
                 proclog->ident ? proclog->ident : processname (0), proclog->str_pid);
  vsnprintf (datagram + len, LOG_DGRAM_SIZE - len, fmt, ap);

  if (proclog->sock != INVALID_SOCKET)
    sendto (proclog->sock, datagram, (int)strlen (datagram), 0, (SOCKADDR*)&proclog->sa_logger, sizeof (SOCKADDR_IN));

  strcat (datagram, "\n");
  if (proclog->option & LOGOPT_OUTDEBUG)
    OutputDebugString (utf8::widen (datagram).c_str ());

  if (proclog->file)
  {
    fwrite (datagram, sizeof (char), strlen (datagram), proclog->file);
    fflush (proclog->file);
  }

}

void syslog_debug (const char* fmt, ...)
{
  SYSTEMTIME stm;
  va_list ap;
  int len;
  char datagram[LOG_DGRAM_SIZE + 3];
  if (!proclog)
    openlog(NULL, 0, 0);

  int prm = LOG_MASK(LOG_DEBUG);
  if (!(prm & proclog->mask))
    return;

  int facility_priority = proclog->facility | LOG_DEBUG;

  va_start(ap, fmt);
  GetLocalTime(&stm);
  len = sprintf(datagram, "<%d>%s %2d %02d:%02d:%02d %s %s%s: ",
                facility_priority,
                month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond,
                local_hostname,
                proclog->ident ? proclog->ident : processname(0), proclog->str_pid);
  vsnprintf(datagram + len, LOG_DGRAM_SIZE - len, fmt, ap);

  if (proclog->sock != INVALID_SOCKET)
    sendto(proclog->sock, datagram, (int)strlen(datagram), 0, (SOCKADDR*)&proclog->sa_logger, sizeof(SOCKADDR_IN));

  strcat(datagram, "\n");
  if (proclog->option & LOGOPT_OUTDEBUG)
    OutputDebugString(utf8::widen(datagram).c_str());

  if (proclog->file)
  {
    fwrite(datagram, sizeof(char), strlen(datagram), proclog->file);
    fflush(proclog->file);
  }
}

/*!
  \ingroup syslog
  The setlogmask() function sets this logmask for the current process, 
  and returns the previous mask. If the mask argument is 0, 
  the current logmask is not modified. 

  A process has a log priority mask that determines which calls to syslog 
  may be logged. All other calls will be ignored. Logging is enabled for the 
  priorities that have the corresponding bit set in mask. 

  setlogmask sets a mask (the "logmask") that determines which future syslog 
  calls shall be ignored.  You can use setlogmask to specify that messages of 
  particular priorities shall be ignored in the future.

  If a program has not called setlogmask, the default mask loaded from the INI
  file (through set_log_ini_param() function) is used. If there is no mask
  specified in INI file the default is to process everything except LOG_DEBUG.

  A setlogmask call overrides any previous setlogmask call.

  Note that the logmask exists entirely independently of opening and closing
  of Syslog connections.

  mask is a bit string with one bit corresponding to each of the possible 
  message priorities. If the bit is on, syslog handles messages of that 
  priority normally. If it is off, syslog discards messages of that priority.
  
  Use the message priority values described in <log.h> and the LOG_MASK or 
  LOG_UPTO macros to  construct an appropriate mask value, as in this example:

    LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ERROR)
         
  or

    ~(LOG_MASK(LOG_INFO))
         
  The LOG_UPTO macro generates a mask with the bits on for 
  a certain priority and all priorities above it:

    LOG_UPTO(LOG_ERROR)
         
  The unfortunate naming of the macro is due to the fact that internally, 
  higher numbers are used for lower message priorities.   
*/
int setlogmask (int mask)
{
  int prev;

  prev = proclog?proclog->mask : defmask;
  if (mask)
  {
    defmask = mask;
    if (proclog)
      proclog->mask = mask;
  }
  return prev;
}

/**
  \ingroup syslog
  \param opt can be any combination of LOGOPT_... values.
  
  For instance
\verbatim    
    setlogoption (LOGOPT_FILE | LOGOPT_NOUDP)
\endverbatim
  turns on file logging and turns off sending of UDP data.

  The new options take effect immediately.
*/
int setlogopt (int opt)
{
  int prev;

  prev = proclog?proclog->option : defopt;
  defopt = opt;
  if (proclog)
    proclog->option = opt;
  return prev;
}
