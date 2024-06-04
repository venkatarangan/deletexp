#include <iostream.h>	// cout, cin
#include <direct.h>		//_getdrive, etc
#include <conio.h>		// getch()

#include <windows.h>
#include <shlobj.h>

// #define MYDEBUG
// #define BULK_DELETE			// Deletes files in groups rather than in single

#ifndef INDIVIDUAL_PARAM
   #define MAXIMUM_PARAMETERS 4		// We exclude FilesNames [/p /v /a /?]
#else
   #define MAXIMUM_PARAMETERS 6		// [/p /v /h /s /r /?]
#endif

enum t_ErrorCode {
	RET_SUCCESS = 0,	// this has to be returned to OS when we get RET_NO_ERROR
	RET_HELP_PARAM,		// ? option has been specified
	RET_NO_PARAM,		// no options 
	RET_EXTRA_PARAM,	// too many parameters
	RET_INVALID_PARAM,	// parameters other than what we have defined are used
	RET_MEMORY_ERROR,	// memory allocation error
	RET_CTRL_C,			// immediate exit
	
    /* ------ Can Proceed for errors after this ---- */
	RET_NON_CRITICAL,	// just a line so that we know errors after this are non-critical
	RET_NETWORK_FILE,	// user has specified a network drive or network path
	RET_USER_ABORT,		// user has aborted in the Are your sure (y/n)? prompt 
	RET_DRIVE_ERROR,	// Invalid/Inaappropriate drive has been specified
	RET_DIRECTORY_ERROR,// Directory Set/Get failed
	RET_NO_PATH,		// Path not found
	RET_NO_FILE,		// File not found
	RET_FIND_ERROR,		// FindFirst family function failed
	RET_FULLPATH_ERROR, // The volume doesnot support fullpath names, RARE/IMPOSSIBLE
	RET_EXCEED_ERROR,	// Some where memory over-run
	RET_NO_ERROR		// no error
};

// --------------- Messages to be displayed to the user  ----------------
#define S_INVALID_DRIVE " Invalid drive specification" 
#define S_REMOTE_DRIVE	" Remote drive not supported"
#define S_CDROM_DRIVE	" CDROM drive not supported"
#define S_RAMDISK_DRIVE " Ramdisk drive not supported"
#define S_NETWORK_DRIVE " Network drives are not supported" 

#define S_TOOMANY_PARAM "Too many parameters -  "
#define S_INVALID_PARAM "Invalid switch - "
#define S_NOREQUIRED_PARAM "Required parameter missing - "

#define S_DELETEWAIT ",\tDelete (Y/N)?"
#define S_ALLFILES "All files in directory will be deleted!"
#define S_AREYOUSURE "Are you sure (Y/N)?"

#define S_MEMORY_ERROR "Could not allocate memory" 
#define S_FILE_NOT_FOUND "File not found"
#define S_PATH_NOT_FOUND "Path not found"

// --------------- Program Internal Constants ---------------------------------
#define STARDOTSTAR		"*.*"			// string to be used
#ifdef BULK_DELETE 
	#define MEM_BLOCK_SIZE  ( 32 * 1024 )	// memory allocated to store filenames
#else
	#define MEM_BLOCK_SIZE  ( 1024 )	// memory allocated to store filenames
#endif
#define I_CTRLC   3						// getch() const for CTRL_C key
// -----------------------------------------------------------------------------

// --------------- Global Variables --------------------------------------------
WIN32_FIND_DATA FileData;	//Structure used by FindFirstFile family

struct t_Param{				//This holds the user preferences as given in cmdline

      int Count;			// Total no. of parameters same as 'argc'
      BOOL FoundFileSpec;	// Whether user has specified any filespecs or not
      BOOL Help;			// User has requested help
      BOOL Invalid;			// Invalid parameters
      BOOL Missing;			// Required Parameter missing
      BOOL TooMany;			// Too-Many Options (not parameters, as they can be n)
      BOOL Wait;			// User has asked the /p wait for every file option
      BOOL Verbose;			// Display file names when deleting

      BOOL AllAttribute;	// Any Attributes. Just to store user choice. 
							// We switch on all other attribute choice ourself.
	  BOOL Temporary;		// Delete files with Temporary Attribute 
	  BOOL Directory;		// Deletes even directories
      BOOL Hidden;			// Delete files with Hidden Attribute 
      BOOL System;			// Delete files with System Attribute 
      BOOL ReadOnly;		// Delete files with ReadOnly Attribute 
} Param;

struct t_ParsedFileSpec {	//Store drive/dir/spec informations. 
	unsigned int iCurrentDir;	// Size of the memory allocated to pointer
	unsigned int iSpec;			// Size of the memory allocated to pointer
	unsigned int iDriveName;	// Size of the memory allocated to pointer
	unsigned int iPathName;		// Size of the memory allocated to pointer
	unsigned int iFullName;		// Size of the memory allocated to pointer

	HANDLE hFindHandle;		// Handle created by FindFirstFile

	BOOL AllowWithExtension; // denotes whether files with ext. can be allowed
	BOOL SwitchedDirectory;	 // whether we have switched directories due to user 
							// filespec	or we are in current directory

	char* pCurrentDir;	// Current directory. Should be initialized before 
						//	calling any drive/dir functions
	char* pSpec;		// Copy of the user file spec argument
	char* pDriveName;	// Drive names 
	char* pPathName;	// Switched path as specified by user file spec
	char* pFullName;	// Absolute file-name to be sent to Shell functions
	char* pFileNameOffset;	// Used by GetFullPathName() to denote the offset
							// from which fname starts in the pFullName
} sFiles;

struct t_DelBlock {
	char*				pNames;			// Memory holding the files names to be deleted
	unsigned int	iNames; // Size of the memory allocated to pointer
	char*	pOffset;		// How far have we written into pNames
} DelList;

// -----------------------------------------------------------------------------

// ----------------------- Function declarations -------------------------------
t_ErrorCode Alloc_Memory( void ) ;
void Release_Memory( ); 

int zFindChar ( char* pString, char cToken);
BOOL ExtensionPresent ( char* cAlternateName, char* cFileName  );
BOOL CanDel ( BOOL IsDir );
BOOL ParseForQuotes( int argc, char* argv[ ] );
BOOL RTrim ( char* p );

t_ErrorCode CheckAndCopySpec ( char* pSpec );
t_ErrorCode CheckAndSetDrive( char* pSpec );
t_ErrorCode CheckAndSetDir ( char* pSpec );
t_ErrorCode CheckFileHandle( HANDLE hTestHandle );
t_ErrorCode GetAllFiles ( HANDLE hFindHandle );
BOOL PrintFileName();
BOOL DeleteFileName( );
//------------------------------------------------------------------------
t_ErrorCode ParseArgumentsForFileSpecs( int argc, char* argv[ ]);
t_ErrorCode CheckAndDelete ( void ) ;


//------------------------------------------- Option Parsing functions
void InitialiseGlobal ( );
BOOL My_GetNextToken ( char* pString, char cToken, char* pBuffer );
void ParseArgumentsForOptions( int argc, char* argv[ ]);
void ParseForOptions ( char* ThisParam );
t_ErrorCode CheckOptions ( void );
void	HelpDisplay( );
//------------------------------------------------------------------------ 

//------------------------------------------- Actual-Deleting Functions
BOOL MyInternalDelete ( char* pFiles ); 
BOOL MyDelete ( ); 
BOOL MyFileDelete ( char* pFile ); 
BOOL MyAddFile ( char* pFile );
void MyInit ( void );
BOOL RefreshIcon ( void ) ;
//------------------------------------------------------------------------