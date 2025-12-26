#ifndef __ERRNO_H__
#define __ERRNO_H__

// errno.h: the 'errno' variable & error numbers definitions (as macros)

// errno is only valid in userspace
#ifndef KERNEL
extern int errno;
#endif

// ================ Error numbers (POSIX) ================

#define EPERM				  1 // Error: Operation not permitted
#define ENOENT				  2 // Error: No such file or directory
#define ESRCH				  3 // Error: No such process
#define EINTR				  4 // Error: Interrupted system call
#define EIO					  5 // Error: Input/output error
#define ENXIO				  6 // Error: No such device or address
#define E2BIG				  7 // Error: Argument list too long
#define ENOEXEC				  8 // Error: Exec format error
#define EBADF				  9 // Error: Bad file descriptor
#define ECHILD				 10 // Error: No child processes
#define EAGAIN				 11 // Error: Resource temporarily unavailable
#define ENOMEM				 12 // Error: Cannot allocate memory
#define EACCES				 13 // Error: Permission denied
#define EFAULT				 14 // Error: Bad address
#define ENOTBLK				 15 // Error: Block device required
#define EBUSY				 16 // Error: Device or resource busy
#define EEXIST				 17 // Error: File exists
#define EXDEV				 18 // Error: Invalid cross-device link
#define ENODEV				 19 // Error: No such device
#define ENOTDIR				 20 // Error: Not a directory
#define EISDIR				 21 // Error: Is a directory
#define EINVAL				 22 // Error: Invalid argument
#define ENFILE				 23 // Error: Too many open files in system
#define EMFILE				 24 // Error: Too many open files
#define ENOTTY				 25 // Error: Inappropriate ioctl for device
#define ETXTBSY				 26 // Error: Text file busy
#define EFBIG				 27 // Error: File too large
#define ENOSPC				 28 // Error: No space left on device
#define ESPIPE				 29 // Error: Illegal seek
#define EROFS				 30 // Error: Read-only file system
#define EMLINK				 31 // Error: Too many links
#define EPIPE				 32 // Error: Broken pipe
#define EDOM				 33 // Error: Numerical argument out of domain
#define ERANGE				 34 // Error: Numerical result out of range
#define EDEADLK				 35 // Error: Resource deadlock avoided
#define ENAMETOOLONG		 36 // Error: File name too long
#define ENOLCK				 37 // Error: No locks available
#define ENOSYS				 38 // Error: Function not implemented
#define ENOTEMPTY			 39 // Error: Directory not empty
#define ELOOP				 40 // Error: Too many levels of symbolic links
#define ENOMSG				 42 // Error: No message of desired type
#define EIDRM				 43 // Error: Identifier removed
#define ECHRNG				 44 // Error: Channel number out of range
#define EL2NSYNC			 45 // Error: Level 2 not synchronized
#define EL3HLT				 46 // Error: Level 3 halted
#define EL3RST				 47 // Error: Level 3 reset
#define ELNRNG				 48 // Error: Link number out of range
#define EUNATCH				 49 // Error: Protocol driver not attached
#define ENOCSI				 50 // Error: No CSI structure available
#define EL2HLT				 51 // Error: Level 2 halted
#define EBADE				 52 // Error: Invalid exchange
#define EBADR				 53 // Error: Invalid request descriptor
#define EXFULL				 54 // Error: Exchange full
#define ENOANO				 55 // Error: No anode
#define EBADRQC				 56 // Error: Invalid request code
#define EBADSLT				 57 // Error: Invalid slot
#define EBFONT				 59 // Error: Bad font file format
#define ENOSTR				 60 // Error: Device not a stream
#define ENODATA				 61 // Error: No data available
#define ETIME				 62 // Error: Timer expired
#define ENOSR				 63 // Error: Out of streams resources
#define ENONET				 64 // Error: Machine is not on the network
#define ENOPKG				 65 // Error: Package not installed
#define EREMOTE				 66 // Error: Object is remote
#define ENOLINK				 67 // Error: Link has been severed
#define EADV				 68 // Error: Advertise error
#define ESRMNT				 69 // Error: Srmount error
#define ECOMM				 70 // Error: Communication error on send
#define EPROTO				 71 // Error: Protocol error
#define EMULTIHOP			 72 // Error: Multihop attempted
#define EDOTDOT				 73 // Error: RFS specific error
#define EBADMSG				 74 // Error: Bad message
#define EOVERFLOW			 75 // Error: Value too large for defined data type
#define ENOTUNIQ			 76 // Error: Name not unique on network
#define EBADFD				 77 // Error: File descriptor in bad state
#define EREMCHG				 78 // Error: Remote address changed
#define ELIBACC				 79 // Error: Can not access a needed shared library
#define ELIBBAD				 80 // Error: Accessing a corrupted shared library
#define ELIBSCN				 81 // Error: lib section in a.out corrupted
#define ELIBMAX				 82 // Error: Attempting to link in too many shared libraries
#define ELIBEXEC			 83 // Error: Cannot exec a shared library directly
#define EILSEQ				 84 // Error: Invalid or incomplete multibyte or wide character
#define ERESTART			 85 // Error: Interrupted system call should be restarted
#define ESTRPIPE			 86 // Error: Streams pipe error
#define EUSERS				 87 // Error: Too many users
#define ENOTSOCK			 88 // Error: Socket operation on non-socket
#define EDESTADDRREQ		 89 // Error: Destination address required
#define EMSGSIZE			 90 // Error: Message too long
#define EPROTOTYPE			 91 // Error: Protocol wrong type for socket
#define ENOPROTOOPT			 92 // Error: Protocol not available
#define EPROTONOSUPPORT		 93 // Error: Protocol not supported
#define ESOCKTNOSUPPORT		 94 // Error: Socket type not supported
#define EOPNOTSUPP			 95 // Error: Operation not supported
#define EPFNOSUPPORT		 96 // Error: Protocol family not supported
#define EAFNOSUPPORT		 97 // Error: Address family not supported by protocol
#define EADDRINUSE			 98 // Error: Address already in use
#define EADDRNOTAVAIL		 99 // Error: Cannot assign requested address
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

// Mapping:
// <=> integer error number (e.g `002`)
// <=> posix macro (e.g. `ENOENT`)
// <=> String describing the error (e.g. `No such file or directory`)
// It should correspond one-to-one with the macros defined above
static const char* ERRORS[] = {
	"Success",
	"Operation not permitted",								// EPERM
	"No such file or directory",							// ENOENT
	"No such process",										// ESRCH
	"Interrupted system call",								// EINTR
	"Input/output error",									// EIO
	"No such device or address",							// ENXIO
	"Argument list too long",								// E2BIG
	"Exec format error",									// ENOEXEC
	"Bad file descriptor",									// EBADF
	"No child processes",									// ECHILD
	"Resource temporarily unavailable",						// EAGAIN
	"Cannot allocate memory",								// ENOMEM
	"Permission denied",									// EACCES
	"Bad address",											// EFAULT
	"Block device required",								// ENOTBLK
	"Device or resource busy",								// EBUSY
	"File exists",											// EEXIST
	"Invalid cross-device link",							// EXDEV
	"No such device",										// ENODEV
	"Not a directory",										// ENOTDIR
	"Is a directory",										// EISDIR
	"Invalid argument",										// EINVAL
	"Too many open files in system",						// ENFILE
	"Too many open files",									// EMFILE
	"Inappropriate ioctl for device",						// ENOTTY
	"Text file busy",										// ETXTBSY
	"File too large",										// EFBIG
	"No space left on device",								// ENOSPC
	"Illegal seek",											// ESPIPE
	"Read-only file system",								// EROFS
	"Too many links",										// EMLINK
	"Broken pipe",											// EPIPE
	"Numerical argument out of domain",						// EDOM
	"Numerical result out of range",						// ERANGE
	"Resource deadlock avoided",							// EDEADLK
	"File name too long",									// ENAMETOOLONG
	"No locks available",									// ENOLCK
	"Function not implemented",								// ENOSYS
	"Directory not empty",									// ENOTEMPTY
	"Too many levels of symbolic links",					// ELOOP
	"No message of desired type",							// ENOMSG
	"Identifier removed",									// EIDRM
	"Channel number out of range",							// ECHRNG
	"Level 2 not synchronized",								// EL2NSYNC
	"Level 3 halted",										// EL3HLT
	"Level 3 reset",										// EL3RST
	"Link number out of range",								// ELNRNG
	"Protocol driver not attached",							// EUNATCH
	"No CSI structure available",							// ENOCSI
	"Level 2 halted",										// EL2HLT
	"Invalid exchange",										// EBADE
	"Invalid request descriptor",							// EBADR
	"Exchange full",										// EXFULL
	"No anode",												// ENOANO
	"Invalid request code",									// EBADRQC
	"Invalid slot",											// EBADSLT
	"Bad font file format",									// EBFONT
	"Device not a stream",									// ENOSTR
	"No data available",									// ENODATA
	"Timer expired",										// ETIME
	"Out of streams resources",								// ENOSR
	"Machine is not on the network",						// ENONET
	"Package not installed",								// ENOPKG
	"Object is remote",										// EREMOTE
	"Link has been severed",								// ENOLINK
	"Advertise error",										// EADV
	"Srmount error",										// ESRMNT
	"Communication error on send",							// ECOMM
	"Protocol error",										// EPROTO
	"Multihop attempted",									// EMULTIHOP
	"RFS specific error",									// EDOTDOT
	"Bad message",											// EBADMSG
	"Value too large for defined data type",				// EOVERFLOW
	"Name not unique on network",							// ENOTUNIQ
	"File descriptor in bad state",							// EBADFD
	"Remote address changed",								// EREMCHG
	"Can not access a needed shared library",				// ELIBACC
	"Accessing a corrupted shared library",					// ELIBBAD
	"lib section in a.out corrupted",						// ELIBSCN
	"Attempting to link in too many shared libraries",		// ELIBMAX
	"Cannot exec a shared library directly",				// ELIBEXEC
	"Invalid or incomplete multibyte or wide character",	// EILSEQ
	"Interrupted system call should be restarted",			// ERESTART
	"Streams pipe error",									// ESTRPIPE
	"Too many users",										// EUSERS
	"Socket operation on non-socket",						// ENOTSOCK
	"Destination address required",							// EDESTADDRREQ
	"Message too long",										// EMSGSIZE
	"Protocol wrong type for socket",						// EPROTOTYPE
	"Protocol not available",								// ENOPROTOOPT
	"Protocol not supported",								// EPROTONOSUPPORT
	"Socket type not supported",							// ESOCKTNOSUPPORT
	"Operation not supported",								// EOPNOTSUPP
	"Protocol family not supported",						// EPFNOSUPPORT
	"Address family not supported by protocol",				// EAFNOSUPPORT
	"Address already in use",								// EADDRINUSE
	"Cannot assign requested address",						// EADDRNOTAVAIL
	"Network is down",										// ENETDOWN
	"Network is unreachable",								// ENETUNREACH
	"Network dropped connection on reset",					// ENETRESET
	"Software caused connection abort",						// ECONNABORTED
	"Connection reset by peer",								// ECONNRESET
	"No buffer space available",							// ENOBUFS
	"Transport endpoint is already connected",				// EISCONN
	"Transport endpoint is not connected",					// ENOTCONN
	"Cannot send after transport endpoint shutdown",		// ESHUTDOWN
	"Too many references: cannot splice",					// ETOOMANYREFS
	"Connection timed out",									// ETIMEDOUT
	"Connection refused",									// ECONNREFUSED
	"Host is down",											// EHOSTDOWN
	"No route to host",										// EHOSTUNREACH
	"Operation already in progress",						// EALREADY
	"Operation now in progress",							// EINPROGRESS
	"Stale file handle",									// ESTALE
	"Structure needs cleaning",								// EUCLEAN
	"Not a Xenix named type file",							// ENOTNAM
	"No Xenix semaphores available",						// ENAVAIL
	"Is a named type file",									// EISNAM
	"Remote I/O error",										// EREMOTEIO
	"Disk quota exceeded",									// EDQUOT
	"No medium found",										// ENOMEDIUM
	"Wrong medium type",									// EMEDIUMTYPE
	"Operation canceled",									// ECANCELED
	"Required key not available",							// ENOKEY
	"Key has expired",										// EKEYEXPIRED
	"Key has been revoked",									// EKEYREVOKED
	"Key was rejected by service",							// EKEYREJECTED
	"Owner died",											// EOWNERDEAD
	"State not recoverable",								// ENOTRECOVERABLE
	"Operation not possible due to RF-kill",				// ERFKILL
	"Memory page has hardware error",						// EHWPOISON
	"Not supported parameter or option",					// ENOTSUP
};

#endif
