#ifndef __ERRNO_H__
#define __ERRNO_H__

// errno.h: the 'errno' variable & error numbers definitions (as macros)

// errno is only valid in userspace
#ifndef KERNEL
extern int errno;
#endif

// ================ Error numbers (POSIX) ================

#define EPERM				001 // Error: Operation not permitted
#define ENOENT				002 // Error: No such file or directory
#define ESRCH				003 // Error: No such process
#define EINTR				004 // Error: Interrupted system call
#define EIO					005 // Error: Input/output error
#define ENXIO				006 // Error: No such device or address
#define E2BIG				007 // Error: Argument list too long
#define ENOEXEC				008 // Error: Exec format error
#define EBADF				009 // Error: Bad file descriptor
#define ECHILD				010 // Error: No child processes
#define EAGAIN				011 // Error: Resource temporarily unavailable
#define ENOMEM				012 // Error: Cannot allocate memory
#define EACCES				013 // Error: Permission denied
#define EFAULT				014 // Error: Bad address
#define ENOTBLK				015 // Error: Block device required
#define EBUSY				016 // Error: Device or resource busy
#define EEXIST				017 // Error: File exists
#define EXDEV				018 // Error: Invalid cross-device link
#define ENODEV				019 // Error: No such device
#define ENOTDIR				020 // Error: Not a directory
#define EISDIR				021 // Error: Is a directory
#define EINVAL				022 // Error: Invalid argument
#define ENFILE				023 // Error: Too many open files in system
#define EMFILE				024 // Error: Too many open files
#define ENOTTY				025 // Error: Inappropriate ioctl for device
#define ETXTBSY				026 // Error: Text file busy
#define EFBIG				027 // Error: File too large
#define ENOSPC				028 // Error: No space left on device
#define ESPIPE				029 // Error: Illegal seek
#define EROFS				030 // Error: Read-only file system
#define EMLINK				031 // Error: Too many links
#define EPIPE				032 // Error: Broken pipe
#define EDOM				033 // Error: Numerical argument out of domain
#define ERANGE				034 // Error: Numerical result out of range
#define EDEADLK				035 // Error: Resource deadlock avoided
#define ENAMETOOLONG		036 // Error: File name too long
#define ENOLCK				037 // Error: No locks available
#define ENOSYS				038 // Error: Function not implemented
#define ENOTEMPTY			039 // Error: Directory not empty
#define ELOOP				040 // Error: Too many levels of symbolic links
#define ENOMSG				042 // Error: No message of desired type
#define EIDRM				043 // Error: Identifier removed
#define ECHRNG				044 // Error: Channel number out of range
#define EL2NSYNC			045 // Error: Level 2 not synchronized
#define EL3HLT				046 // Error: Level 3 halted
#define EL3RST				047 // Error: Level 3 reset
#define ELNRNG				048 // Error: Link number out of range
#define EUNATCH				049 // Error: Protocol driver not attached
#define ENOCSI				050 // Error: No CSI structure available
#define EL2HLT				051 // Error: Level 2 halted
#define EBADE				052 // Error: Invalid exchange
#define EBADR				053 // Error: Invalid request descriptor
#define EXFULL				054 // Error: Exchange full
#define ENOANO				055 // Error: No anode
#define EBADRQC				056 // Error: Invalid request code
#define EBADSLT				057 // Error: Invalid slot
#define EBFONT				059 // Error: Bad font file format
#define ENOSTR				060 // Error: Device not a stream
#define ENODATA				061 // Error: No data available
#define ETIME				062 // Error: Timer expired
#define ENOSR				063 // Error: Out of streams resources
#define ENONET				064 // Error: Machine is not on the network
#define ENOPKG				065 // Error: Package not installed
#define EREMOTE				066 // Error: Object is remote
#define ENOLINK				067 // Error: Link has been severed
#define EADV				068 // Error: Advertise error
#define ESRMNT				069 // Error: Srmount error
#define ECOMM				070 // Error: Communication error on send
#define EPROTO				071 // Error: Protocol error
#define EMULTIHOP			072 // Error: Multihop attempted
#define EDOTDOT				073 // Error: RFS specific error
#define EBADMSG				074 // Error: Bad message
#define EOVERFLOW			075 // Error: Value too large for defined data type
#define ENOTUNIQ			076 // Error: Name not unique on network
#define EBADFD				077 // Error: File descriptor in bad state
#define EREMCHG				078 // Error: Remote address changed
#define ELIBACC				079 // Error: Can not access a needed shared library
#define ELIBBAD				080 // Error: Accessing a corrupted shared library
#define ELIBSCN				081 // Error: lib section in a.out corrupted
#define ELIBMAX				082 // Error: Attempting to link in too many shared libraries
#define ELIBEXEC			083 // Error: Cannot exec a shared library directly
#define EILSEQ				084 // Error: Invalid or incomplete multibyte or wide character
#define ERESTART			085 // Error: Interrupted system call should be restarted
#define ESTRPIPE			086 // Error: Streams pipe error
#define EUSERS				087 // Error: Too many users
#define ENOTSOCK			088 // Error: Socket operation on non-socket
#define EDESTADDRREQ		089 // Error: Destination address required
#define EMSGSIZE			090 // Error: Message too long
#define EPROTOTYPE			091 // Error: Protocol wrong type for socket
#define ENOPROTOOPT			092 // Error: Protocol not available
#define EPROTONOSUPPORT		093 // Error: Protocol not supported
#define ESOCKTNOSUPPORT		094 // Error: Socket type not supported
#define EOPNOTSUPP			095 // Error: Operation not supported
#define EPFNOSUPPORT		096 // Error: Protocol family not supported
#define EAFNOSUPPORT		097 // Error: Address family not supported by protocol
#define EADDRINUSE			098 // Error: Address already in use
#define EADDRNOTAVAIL		099 // Error: Cannot assign requested address
#define ENETDOWN			100 // Error: Network is down
#define ENETUNREACH			101 // Error: Network is unreachable
#define ENETRESET			102 // Error: Network dropped connection on reset
#define ECONNABORTED		103 // Error: Software caused connection abort
#define ECONNRESET			104 // Error: Connection reset by peer
#define ENOBUFS				105 // Error: No buffer space available
#define EISCONN				106 // Error: Transport endpoint is already connected
#define ENOTCONN			107 // Error: Transport endpoint is not connected
#define ESHUTDOWN			108 // Error: Cannot send after transport endpoint shutdown
#define ETOOMANYREFS		109 // Error: Too many references: cannot splice
#define ETIMEDOUT			110 // Error: Connection timed out
#define ECONNREFUSED		111 // Error: Connection refused
#define EHOSTDOWN			112 // Error: Host is down
#define EHOSTUNREACH		113 // Error: No route to host
#define EALREADY			114 // Error: Operation already in progress
#define EINPROGRESS			115 // Error: Operation now in progress
#define ESTALE				116 // Error: Stale file handle
#define EUCLEAN				117 // Error: Structure needs cleaning
#define ENOTNAM				118 // Error: Not a Xenix named type file
#define ENAVAIL				119 // Error: No Xenix semaphores available
#define EISNAM				120 // Error: Is a named type file
#define EREMOTEIO			121 // Error: Remote I/O error
#define EDQUOT				122 // Error: Disk quota exceeded
#define ENOMEDIUM			123 // Error: No medium found
#define EMEDIUMTYPE			124 // Error: Wrong medium type
#define ECANCELED			125 // Error: Operation canceled
#define ENOKEY				126 // Error: Required key not available
#define EKEYEXPIRED			127 // Error: Key has expired
#define EKEYREVOKED			128 // Error: Key has been revoked
#define EKEYREJECTED		129 // Error: Key was rejected by service
#define EOWNERDEAD			130 // Error: Owner died
#define ENOTRECOVERABLE		131 // Error: State not recoverable
#define ERFKILL				132 // Error: Operation not possible due to RF-kill
#define EHWPOISON			133 // Error: Memory page has hardware error
#define ENOTSUP				134 // Error: Not supported parameter or option

#endif
