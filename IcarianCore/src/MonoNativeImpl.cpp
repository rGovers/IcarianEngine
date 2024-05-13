#include "Core/MonoNativeImpl.h"

#include <cstdint>
#include <cstring>
#include <ctime>

#include "Core/IcarianAssert.h"

#ifndef WIN32
#include <dirent.h>
#include <sys/stat.h>

// Reimplementation of System.Native as have been having issues with it on Linux.
// SystemNative_LChflagsCanSetHiddenFlag is the main culprit.

struct FileStatus
{
    int32_t Flags;     
    int32_t Mode;      
    uint32_t UID;      
    uint32_t GID;      
    int64_t Size;      
    int64_t ATime;     
    int64_t ATimeNsec; 
    int64_t MTime;     
    int64_t MTimeNsec; 
    int64_t CTime;     
    int64_t CTimeNsec; 
    int64_t BirthTime; 
    int64_t BirthTimeNsec; 
    int64_t Dev;       
    int64_t Ino;       
    uint32_t UserFlags; 
};

struct DirectoryEntry
{
    const char* Name;
    int32_t NameLength;
    int32_t InodeType;
};

enum e_Error
{
    Error_SUCCESS = 0,

    Error_E2BIG = 0x10001,           
    Error_EACCES = 0x10002,          
    Error_EADDRINUSE = 0x10003,      
    Error_EADDRNOTAVAIL = 0x10004,   
    Error_EAFNOSUPPORT = 0x10005,    
    Error_EAGAIN = 0x10006,          
    Error_EALREADY = 0x10007,        
    Error_EBADF = 0x10008,           
    Error_EBADMSG = 0x10009,         
    Error_EBUSY = 0x1000A,           
    Error_ECANCELED = 0x1000B,       
    Error_ECHILD = 0x1000C,          
    Error_ECONNABORTED = 0x1000D,    
    Error_ECONNREFUSED = 0x1000E,    
    Error_ECONNRESET = 0x1000F,      
    Error_EDEADLK = 0x10010,         
    Error_EDESTADDRREQ = 0x10011,    
    Error_EDOM = 0x10012,            
    Error_EDQUOT = 0x10013,          
    Error_EEXIST = 0x10014,          
    Error_EFAULT = 0x10015,          
    Error_EFBIG = 0x10016,           
    Error_EHOSTUNREACH = 0x10017,    
    Error_EIDRM = 0x10018,           
    Error_EILSEQ = 0x10019,          
    Error_EINPROGRESS = 0x1001A,     
    Error_EINTR = 0x1001B,           
    Error_EINVAL = 0x1001C,          
    Error_EIO = 0x1001D,             
    Error_EISCONN = 0x1001E,         
    Error_EISDIR = 0x1001F,          
    Error_ELOOP = 0x10020,           
    Error_EMFILE = 0x10021,          
    Error_EMLINK = 0x10022,          
    Error_EMSGSIZE = 0x10023,        
    Error_EMULTIHOP = 0x10024,       
    Error_ENAMETOOLONG = 0x10025,    
    Error_ENETDOWN = 0x10026,        
    Error_ENETRESET = 0x10027,       
    Error_ENETUNREACH = 0x10028,     
    Error_ENFILE = 0x10029,          
    Error_ENOBUFS = 0x1002A,         
    Error_ENODEV = 0x1002C,          
    Error_ENOENT = 0x1002D,          
    Error_ENOEXEC = 0x1002E,         
    Error_ENOLCK = 0x1002F,          
    Error_ENOLINK = 0x10030,         
    Error_ENOMEM = 0x10031,          
    Error_ENOMSG = 0x10032,          
    Error_ENOPROTOOPT = 0x10033,     
    Error_ENOSPC = 0x10034,          
    Error_ENOSYS = 0x10037,          
    Error_ENOTCONN = 0x10038,        
    Error_ENOTDIR = 0x10039,         
    Error_ENOTEMPTY = 0x1003A,       
    Error_ENOTRECOVERABLE = 0x1003B, 
    Error_ENOTSOCK = 0x1003C,        
    Error_ENOTSUP = 0x1003D,         
    Error_ENOTTY = 0x1003E,          
    Error_ENXIO = 0x1003F,           
    Error_EOVERFLOW = 0x10040,       
    Error_EOWNERDEAD = 0x10041,      
    Error_EPERM = 0x10042,           
    Error_EPIPE = 0x10043,           
    Error_EPROTO = 0x10044,          
    Error_EPROTONOSUPPORT = 0x10045, 
    Error_EPROTOTYPE = 0x10046,      
    Error_ERANGE = 0x10047,          
    Error_EROFS = 0x10048,           
    Error_ESPIPE = 0x10049,          
    Error_ESRCH = 0x1004A,           
    Error_ESTALE = 0x1004B,          
    Error_ETIMEDOUT = 0x1004D,       
    Error_ETXTBSY = 0x1004E,         
    Error_EXDEV = 0x1004F,           
    Error_ESOCKTNOSUPPORT = 0x1005E, 
    Error_EPFNOSUPPORT = 0x10060,    
    Error_ESHUTDOWN = 0x1006C,       
    Error_EHOSTDOWN = 0x10070,       
    Error_ENODATA = 0x10071,         

    Error_EOPNOTSUPP = Error_ENOTSUP, 
    Error_EWOULDBLOCK = Error_EAGAIN, 

    Error_ENONSTANDARD = 0x1FFFF,
};

constexpr int32_t SystemNative_ConvertErrorPlatformToPal(int32_t a_platformError)
{
    switch (a_platformError)
    {
    case 0:
    {
        return Error_SUCCESS;
    }
    case E2BIG:
    {
        return Error_E2BIG;
    }
    case EACCES:
    {
        return Error_EACCES;
    }
    case EADDRINUSE:
    {
        return Error_EADDRINUSE;
    }
    case EADDRNOTAVAIL:
    {
        return Error_EADDRNOTAVAIL;
    }
    case EAFNOSUPPORT:
    {
        return Error_EAFNOSUPPORT;
    }
    case EAGAIN:
    {
        return Error_EAGAIN;
    }
    case EALREADY:
    {
        return Error_EALREADY;
    }
    case EBADF:
    {
        return Error_EBADF;
    }
    case EBADMSG:
    {
        return Error_EBADMSG;
    }
    case EBUSY:
    {
        return Error_EBUSY;
    }
    case ECANCELED:
    {
        return Error_ECANCELED;
    }
    case ECHILD:
    {
        return Error_ECHILD;
    }
    case ECONNABORTED:
    {
        return Error_ECONNABORTED;
    }
    case ECONNREFUSED:
    {
        return Error_ECONNREFUSED;
    }
    case ECONNRESET:
    {
        return Error_ECONNRESET;
    }
    case EDEADLK:
    {
        return Error_EDEADLK;
    }
    case EDESTADDRREQ:
    {
        return Error_EDESTADDRREQ;
    }
    case EDOM:
    {
        return Error_EDOM;
    }
    case EDQUOT:
    {
        return Error_EDQUOT;
    }
    case EEXIST:
    {
        return Error_EEXIST;
    }
    case EFAULT:
    {
        return Error_EFAULT;
    }
    case EFBIG:
    {
        return Error_EFBIG;
    }
    case EHOSTUNREACH:
    {
        return Error_EHOSTUNREACH;
    }
    case EIDRM:
    {
        return Error_EIDRM;
    }
    case EILSEQ:
    {
        return Error_EILSEQ;
    }
    case EINPROGRESS:
    {
        return Error_EINPROGRESS;
    }
    case EINTR:
    {
        return Error_EINTR;
    }
    case EINVAL:
    {
        return Error_EINVAL;
    }
    case EIO:
    {
        return Error_EIO;
    }
    case EISCONN:
    {
        return Error_EISCONN;
    }
    case EISDIR:
    {
        return Error_EISDIR;
    }
    case ELOOP:
    {
        return Error_ELOOP;
    }
    case EMFILE:
    {
        return Error_EMFILE;
    }
    case EMLINK:
    {
        return Error_EMLINK;
    }
    case EMSGSIZE:
    {
        return Error_EMSGSIZE;
    }
    case EMULTIHOP:
    {
        return Error_EMULTIHOP;
    }
    case ENAMETOOLONG:
    {
        return Error_ENAMETOOLONG;
    }
    case ENETDOWN:
    {
        return Error_ENETDOWN;
    }
    case ENETRESET:
    {
        return Error_ENETRESET;
    }
    case ENETUNREACH:
    {
        return Error_ENETUNREACH;
    }
    case ENFILE:
    {
        return Error_ENFILE;
    }
    case ENOBUFS:
    {
        return Error_ENOBUFS;
    }
    case ENODEV:
    {
        return Error_ENODEV;
    }
    case ENOENT:
    {
        return Error_ENOENT;
    }
    case ENOEXEC:
    {
        return Error_ENOEXEC;
    }
    case ENOLCK:
    {
        return Error_ENOLCK;
    }
    case ENOLINK:
    {
        return Error_ENOLINK;
    }
    case ENOMEM:
    {
        return Error_ENOMEM;
    }
    case ENOMSG:
    {
        return Error_ENOMSG;
    }
    case ENOPROTOOPT:
    {
        return Error_ENOPROTOOPT;
    }
    case ENOSPC:
    {
        return Error_ENOSPC;
    }
    case ENOSYS:
    {
        return Error_ENOSYS;
    }
    case ENOTCONN:
    {
        return Error_ENOTCONN;
    }
    case ENOTDIR:
    {
        return Error_ENOTDIR;
    }
#if ENOTEMPTY != EEXIST 
    case ENOTEMPTY:
    {
        return Error_ENOTEMPTY;
    }
#endif
#ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE:
    {
        return Error_ENOTRECOVERABLE;
    }
#endif
    case ENOTSOCK:
    {
        return Error_ENOTSOCK;
    }
    case ENOTSUP:
    {
        return Error_ENOTSUP;
    }
    case ENOTTY:
    {
        return Error_ENOTTY;
    }
    case ENXIO:
    {
        return Error_ENXIO;
    }
    case EOVERFLOW:
    {
        return Error_EOVERFLOW;
    }
#ifdef EOWNERDEAD
    case EOWNERDEAD:
    {
        return Error_EOWNERDEAD;
    }
#endif
    case EPERM:
    {
        return Error_EPERM;
    }
    case EPIPE:
    {
        return Error_EPIPE;
    }
    case EPROTO:
    {
        return Error_EPROTO;
    }
    case EPROTONOSUPPORT:
    {
        return Error_EPROTONOSUPPORT;
    }
    case EPROTOTYPE:
    {
        return Error_EPROTOTYPE;
    }
    case ERANGE:
    {
        return Error_ERANGE;
    }
    case EROFS:
    {
        return Error_EROFS;
    }
    case ESPIPE:
    {
        return Error_ESPIPE;
    }
    case ESRCH:
    {
        return Error_ESRCH;
    }
    case ESTALE:
    {
        return Error_ESTALE;
    }
    case ETIMEDOUT:
    {
        return Error_ETIMEDOUT;
    }
    case ETXTBSY:
    {
        return Error_ETXTBSY;
    }
    case EXDEV:
    {
        return Error_EXDEV;
    }
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
    {
        return Error_ESOCKTNOSUPPORT;
    }
#endif
    case EPFNOSUPPORT:
    {
        return Error_EPFNOSUPPORT;
    }
    case ESHUTDOWN:
    {
        return Error_ESHUTDOWN;
    }
    case EHOSTDOWN:
    {
        return Error_EHOSTDOWN;
    }
    case ENODATA:
    {
        return Error_ENODATA;
    }
#if EOPNOTSUPP != ENOTSUP
    case EOPNOTSUPP:
    {
        return Error_EOPNOTSUPP;
    }
#endif
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
    {
        return Error_EWOULDBLOCK;
    }
#endif
    }

    return Error_ENONSTANDARD;
}

static FileStatus ToFileStatus(const struct stat* a_stat)
{
    FileStatus status;
    memset(&status, 0, sizeof(FileStatus));

    status.Dev = (int64_t)a_stat->st_dev;
    status.Ino = (int64_t)a_stat->st_ino;
    status.Mode = (int32_t)a_stat->st_mode;
    status.UID = a_stat->st_uid;
    status.GID = a_stat->st_gid;
    status.Size = a_stat->st_size;

    status.ATime = a_stat->st_atim.tv_sec;
    status.MTime = a_stat->st_mtim.tv_sec;
    status.CTime = a_stat->st_ctim.tv_sec;

    status.ATimeNsec = a_stat->st_atim.tv_nsec;
    status.MTimeNsec = a_stat->st_mtim.tv_nsec;
    status.CTimeNsec = a_stat->st_ctim.tv_nsec;

    return status;
}   

static int32_t SystemNative_Stat2(const char* a_path, FileStatus* a_output)
{
    struct stat result = { 0 };

    int32_t ret;

    do
    {
        ret = stat(a_path, &result);
    }
    while (ret < 0 && errno == EINTR);

    if (ret == 0)
    {
        *a_output = ToFileStatus(&result);
    }

    return ret;
}
static int32_t SystemNative_LStat2(const char* a_path, FileStatus* a_output)
{
    struct stat result = { 0 };

    int32_t ret;

    do
    {
        ret = lstat(a_path, &result);
    }
    while (ret < 0 && errno == EINTR);

    if (ret == 0)
    {
        *a_output = ToFileStatus(&result);
    }

    return ret;
}

static DIR* SystemNative_OpenDir(const char* a_path)
{
    return opendir(a_path);
}
static int32_t SystemNative_CloseDir(DIR* dir)
{
    return closedir(dir);
}

static int32_t SystemNative_ReadDirR(DIR* a_dir, uint8_t* a_buffer, int32_t a_bufferLength, DirectoryEntry* a_out)
{
    memset(a_out, 0, sizeof(DirectoryEntry));

    const struct dirent* entry = readdir(a_dir);

    if (entry == NULL)
    {
        return -1;
    }

    a_out->Name = entry->d_name;
    a_out->NameLength = (int32_t)strlen(entry->d_name);
    a_out->InodeType = entry->d_type;

    return 0;
}

static int32_t SystemNative_GetReadDirRBufferSize()
{
    return 0;
}

static int32_t SystemNative_LChflagsCanSetHiddenFlag()
{
    return 0;
}

static void SystemNative_GetNonCryptographicallySecureRandomBytes(uint8_t* a_buffer, int32_t a_bufferLength)
{
    ICARIAN_ASSERT(a_buffer != nullptr);

    const uint32_t iBufferLen = (uint32_t)a_bufferLength / sizeof(int32_t);
    const uint32_t iBufferRemainder = (uint32_t)a_bufferLength % sizeof(int32_t);
    const uint32_t offset = iBufferLen * sizeof(int32_t);

    int32_t* buffer = (int32_t*)a_buffer;

    for (uint32_t i = 0; i < iBufferLen; ++i)
    {
        buffer[i] = (int32_t)rand();
    }

    for (uint32_t i = 0; i < iBufferRemainder; ++i)
    {
        a_buffer[offset + i] = (uint8_t)((double)rand() / RAND_MAX * UINT8_MAX);
    }
}
#endif

namespace IcarianCore
{
    static MonoNativeImpl* Instance = nullptr;

    MonoNativeImpl::MonoNativeImpl()
    {

    }
    MonoNativeImpl::~MonoNativeImpl()
    {

    }

    void MonoNativeImpl::Init()
    {
        if (Instance == nullptr)
        {
            Instance = new MonoNativeImpl();

#ifndef WIN32
            Instance->m_functions.emplace("SystemNative_ConvertErrorPlatformToPal", (void*)SystemNative_ConvertErrorPlatformToPal);

            Instance->m_functions.emplace("SystemNative_Stat2", (void*)SystemNative_Stat2);
            Instance->m_functions.emplace("SystemNative_LStat2", (void*)SystemNative_LStat2);

            Instance->m_functions.emplace("SystemNative_OpenDir", (void*)SystemNative_OpenDir);
            Instance->m_functions.emplace("SystemNative_CloseDir", (void*)SystemNative_CloseDir);
            Instance->m_functions.emplace("SystemNative_ReadDirR", (void*)SystemNative_ReadDirR);

            Instance->m_functions.emplace("SystemNative_GetReadDirRBufferSize", (void*)SystemNative_GetReadDirRBufferSize);

            Instance->m_functions.emplace("SystemNative_LChflagsCanSetHiddenFlag", (void*)SystemNative_LChflagsCanSetHiddenFlag);

            Instance->m_functions.emplace("SystemNative_GetNonCryptographicallySecureRandomBytes", (void*)SystemNative_GetNonCryptographicallySecureRandomBytes);
#endif
        }
    }

    void MonoNativeImpl::Destroy()
    {
        if (Instance != nullptr)
        {
            delete Instance;
            Instance = nullptr;
        }
    }

    void* MonoNativeImpl::GetFunction(const std::string_view& a_name)
    {
        if (Instance != nullptr)
        {
            const auto iter = Instance->m_functions.find(std::string(a_name));
            if (iter != Instance->m_functions.end())
            {
                return iter->second;
            }
        }

        return nullptr;
    }
}