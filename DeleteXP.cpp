#include "DeleteXP.hpp"

int main( int argc, char *argv[] )
{
	t_ErrorCode MyReturn;

	InitialiseGlobal( );	

	MyReturn = Alloc_Memory( );
	if ( MyReturn != RET_NO_ERROR ) 
		return MyReturn;

	ParseForQuotes( argc, argv );

	ParseArgumentsForOptions( argc, argv );
	MyReturn = CheckOptions();
	if ( MyReturn != RET_NO_ERROR )  {
		RefreshIcon ( ) ;
		Release_Memory( );
		return MyReturn;
	}

	MyReturn = ParseArgumentsForFileSpecs( argc, argv);   // this calls delete routine with individual file names
	if ( MyReturn != RET_NO_ERROR ) {
		RefreshIcon ( ) ;
		Release_Memory( );
		return MyReturn;
	}

	RefreshIcon ( ) ;
	Release_Memory( );
	return RET_SUCCESS;	
}

void InitialiseGlobal ( )
{
	Param.Count				= 0;
	Param.FoundFileSpec	 	= FALSE;
	Param.Help				= FALSE;
	Param.Invalid			= FALSE;
	Param.Missing			= FALSE;
	Param.TooMany			= FALSE;
	Param.Wait				= FALSE;
	Param.AllAttribute		= FALSE;
	Param.Directory			= FALSE;
	Param.Hidden			= FALSE;
	Param.System			= FALSE;
	Param.ReadOnly			= FALSE;
    Param.Verbose			= FALSE;
	Param.Temporary			= FALSE;
}

void HelpDisplay ( void ) 
{
	cout	<< "Deletes one or more files into Window's Recycle Bin" << endl;
	cout  << endl 
         #ifdef INDIVIDUAL_PARAM
            << "DELETEXP [drive:][path]filenames [/P] [/H] [/S] [/R] [/A] [/V] [/D]" << endl;
         #else
            << "DELETEXP [drive:][path]filenames [/P] [/A] [/V] [/D]" << endl;
         #endif
	cout  << "  [drive:][path]filenames  Specifies the file(s) to delete.  Specify" << endl;
	cout  << "                           files by using wildcards." << endl;
	cout  << "  /P            Prompts for confirmation before deleting each file." << endl;
	cout  << "  /A            If specified deletes files irrespective of the attribute." << endl;
	cout  << "  /V            Verbose mode; displays filenames. Superfluous with /P" << endl;
	cout  << "  /D            If specified deletes directories/folders also " << endl;

   #ifdef INDIVIDUAL_PARAM
   cout  << "  /H            If specified deletes hidden files also." << endl;
   cout  << "  /S            If specified deletes system files also." << endl;
   cout  << "  /R            If specified deletes read-only files also." << endl;
   #endif

	cout  << endl
         <<  "DeleteXP is what DEL Command of Windows should have been." << endl;
	cout  <<  "This program is absolute FREEWARE; read deletexp.txt file for details." << endl;
	cout  <<  "Copyright 1996-2004, Venkatarangan, Chennai, India. Version 2.5." << endl;
	cout  <<  "Visit http://www.venkatarangan.com for more exciting downloads" << endl;
}

// !!!!!!!!!!!!!!!!!!!! Options Parsing functions Begin here !!!!!!!!!!!!!!!!

BOOL My_GetNextToken ( char* pString, char cToken, char* pBuffer ) 
{
	static char* pCache;
	BOOL bReturn = FALSE;

	unsigned int i = 0, iTotalLen = 0;

	//check whether this is the first time or the subsequent times
	if (pString != NULL)  pCache = pString;
	
	//check whether we have finished all characters or the cache is un-initialised
	if ( pCache ==  NULL ) return bReturn;

	iTotalLen = strlen( pCache );

	for ( i = 0; i < iTotalLen; i++ ){
		char CurrentChar;
		bReturn = TRUE;
		CurrentChar = *( pCache + i ) ;
		if ((i > 0) &&  ( CurrentChar == cToken ) ) {
			pCache += i;
			break;
		}
		*( pBuffer +  i )		= CurrentChar;
		*( pBuffer +  i + 1 )	= '\0';
	}

	if ( i == iTotalLen ) pCache = NULL;
	return bReturn;
}

void ParseArgumentsForOptions(int argc, char* argv[ ]) 
{
	int i;
	Param.Count = argc;

	//loop & parse all parameters for options;
	for ( i = 2 ; i <= Param.Count; i++) {
	  char CurrentParam[255];
	  BOOL bTemp;
	  
	  bTemp = My_GetNextToken ( argv[ i - 1 ], '/', CurrentParam );
	  while ( bTemp ) {
		#ifdef MYDEBUG
		  cout << "DEBUG Options & parameter(s): " << CurrentParam << endl ;
		#endif
		RTrim ( CurrentParam );
		ParseForOptions ( CurrentParam );
		bTemp = My_GetNextToken ( NULL, '/', CurrentParam);
	  }
	}
	return ;
}

void ParseForOptions ( char* ThisParam ) 
{
	// here sFiles.pSpec is used to carry the invalid parameter

	static OptionCount=0;

	if ( *( ThisParam ) == '/' )
		OptionCount++;
	else {
		Param.FoundFileSpec = TRUE;
		return;
	}
      
	//Check for parameters greater than two characters like /aa
	if ( strlen(ThisParam) > 2 ) {
		if ( !Param.Invalid && !Param.TooMany ) 
			strcpy ( sFiles.pSpec, ThisParam); 
		Param.Invalid = TRUE;
		return;
	}

	switch ( *( ThisParam + 1 )  ) {
	case '?':
		Param.Help = TRUE;
		break;
	case 'p' : case 'P':
		Param.Wait = TRUE;
		Param.Verbose = TRUE;	//switch verbose on ourself
		break;
	#ifdef INDIVIDUAL_PARAM
	case 'h' : case 'H':
		Param.Hidden = TRUE;
		break;
	case 's' : case 'S':
		Param.System = TRUE;
		break;
	case 'r' : case 'R':
		Param.ReadOnly = TRUE;
		break; 
	#endif
	case 'v' : case 'V':
		Param.Verbose = TRUE;
		break; 
	case 'd' : case 'D':
		Param.Directory = TRUE;
		break; 
	case 'a' : case 'A':
		Param.Hidden = TRUE;
		Param.System = TRUE;
		Param.ReadOnly = TRUE;
		Param.Temporary = TRUE;
		break;
	default:
		if ( !Param.Invalid ) {
			if ( !Param.TooMany )
				strcpy ( sFiles.pSpec, ThisParam); 
			Param.Invalid = TRUE;
		}
		break;
	}

	if ( !( Param.TooMany ) && ( OptionCount > MAXIMUM_PARAMETERS ) ) {
		strcpy ( sFiles.pSpec, ThisParam); 
		Param.TooMany = TRUE; 
	}
}

t_ErrorCode CheckOptions ( void ) 
{

	if ( Param.Help ) {
		HelpDisplay( );
		return RET_HELP_PARAM;
	}

	if ( Param.TooMany ) {
		cout << S_TOOMANY_PARAM << sFiles.pSpec << endl;
		return RET_EXTRA_PARAM;
	}

	if ( Param.Invalid ) {	
		cout << S_INVALID_PARAM << sFiles.pSpec << endl;
		return RET_INVALID_PARAM;
	}

	if ( Param.Missing || !Param.FoundFileSpec ){
		cout << S_NOREQUIRED_PARAM << endl;
		return RET_NO_PARAM;
	}

	return RET_NO_ERROR;
}

// !!!!!!!!!!!!!!!!!!!! Options Parsing functions End here !!!!!!!!!!!!!!!!

/* ************************************************************************
				New routines for getting file names
				2nd February 1997
/************************************************************************ */
//--------------------- General Functions begin here ----------------------
BOOL ParseForQuotes( int argc, char* argv[ ] ){
	int i;
	char *p;
	Param.Count = argc;

	//loop & parse all parameters for options;
	for ( i = 2 ; i <= Param.Count; i++) {
		do {
			p = strrchr( argv[ i - 1], '"');
			if ( p != NULL ) 
				*( p ) = ' ';
		} while (p != NULL);
	}
	return TRUE;
}

BOOL RTrim ( char* p )
{
	int i, len;
	len = strlen ( p );
	for ( i = len - 1; i >= 0; i-- ){
		if ( *( p + i ) == ' ' )
			*( p + i ) = '\0';
		else
			break;
	}
	return TRUE;
}

int zFindChar ( char* pString, char cToken) 
{
	unsigned int n;
	for( n = 0; *pString != '\0'; n++, pString++ ){
		if ( *pString == cToken ) 
			return n;
	}
	return -1;
}

BOOL ExtensionPresent ( char* cAlternateName, char* cFileName  ) 
{
	char *p;

	if ( strlen( cAlternateName ) == 0 ) 
		//FileName is the altername as altername = ""
		p = cFileName + strlen( cFileName ) ;
	else
		p = cAlternateName + strlen( cAlternateName ) ;
	
	// You need not check for the presence of dot in the last character
	p -= 2;		

	return ( ( *(p-2) == '.' ) || ( *(p-1) == '.' ) || ( *(p) == '.' ) );
}

BOOL CanDel ( BOOL IsDir )
{
	char key[ 255 ];

	if ( Param.Wait ) //user has opted to confirm for every file, so no dir confirm
		return TRUE;

	do {
		key[ 0 ] = 'Z';
		if ( IsDir ) 
			cout << S_ALLFILES << endl;
		else
			cout << S_NETWORK_DRIVE << endl;
		
		cout <<  S_AREYOUSURE << flush;
		cin >> key;
		CharUpper( key );
	} while ( (key[ 0 ] != 'Y') && ( key[ 0 ] != 'N' ) );

   if ( key[ 0 ] == 'Y' ) 
		return TRUE;
   else  
		return FALSE;
}

//--------------------- General Functions End here ----------------------

// =================> Initialization & Memory functions Begin here <==========
t_ErrorCode Alloc_Memory ( void ) 
{
	sFiles.iCurrentDir = _MAX_DIR;
	sFiles.iSpec = _MAX_PATH;
	
	sFiles.iDriveName = _MAX_DRIVE + 1;
	sFiles.iPathName = _MAX_PATH;
	sFiles.iFullName = _MAX_PATH;
	DelList.iNames = MEM_BLOCK_SIZE;

   // Allocate the memory to store the parsed names
   try {

	   sFiles.pCurrentDir = new char [ sFiles.iCurrentDir ];
	   	if ( sFiles.pCurrentDir  == 0 )
			throw "Memory allocation failure!";
		memset( sFiles.pCurrentDir, '\0', sFiles.iCurrentDir );
		

	   sFiles.pSpec = new char [ sFiles.iSpec ];
	   	if ( sFiles.pSpec  == 0 )
			throw "Memory allocation failure!";
		memset( sFiles.pSpec, '\0', sFiles.iSpec );
	

		sFiles.pDriveName = new char [ sFiles.iDriveName ];
		if ( sFiles.pDriveName == 0 )
			throw "Memory allocation failure!";
		memset( sFiles.pDriveName, '\0', sFiles.iDriveName );


		sFiles.pPathName = new char [ sFiles.iPathName ];
		if ( sFiles.pPathName  == 0 )
			throw "Memory allocation failure!";
		memset( sFiles.pPathName, '\0', sFiles.iPathName );
	

		sFiles.pFullName = new char [ sFiles.iFullName ];
		if ( sFiles.pFullName  == 0 )
			throw "Memory allocation failure!";
		memset( sFiles.pFullName, '\0', sFiles.iFullName );


		DelList.pNames = new char [ DelList.iNames ];
		if( DelList.pNames == 0 )
			throw "Memory allocation failure!";
		memset(DelList.pNames, '\0', DelList.iNames );
		
   }
   catch( char * str ) {
     cout << "Exception raised: " << str << endl;
     return RET_MEMORY_ERROR;
   }

  return RET_NO_ERROR;
}

void Release_Memory ( ) 
{
	delete[] sFiles.pCurrentDir;
	sFiles.pCurrentDir = NULL;

	delete[] sFiles.pSpec;
	sFiles.pSpec = NULL;

	delete[] sFiles.pDriveName;
	sFiles.pDriveName = NULL;

	delete[] sFiles.pPathName;
	sFiles.pPathName = NULL;

   	delete[] sFiles.pFullName;
	sFiles.pFullName = NULL;

	delete[] DelList.pNames;
	DelList.pNames = NULL;

	sFiles.iCurrentDir = 0;
	sFiles.iSpec = 0;
	sFiles.iDriveName = 0;
	sFiles.iPathName = 0;
	sFiles.iFullName = 0;
	DelList.iNames = 0;

}
// =================> Initialization & Memory functions End here <==========

//!!!!!!!!!!!!!!!!!!! Find Files management code begins here !!!!!!!!!!!!!!!
t_ErrorCode ParseArgumentsForFileSpecs( int argc, char* argv[ ] )
{
	int i;
   t_ErrorCode MyReturn;  
   char CurrentParam[255];
	BOOL bTemp;

	if ( GetCurrentDirectory(sFiles.iCurrentDir, sFiles.pCurrentDir) == 0)
		return RET_DIRECTORY_ERROR;

	//loop & parse all parameters for options;
	for ( i = 2 ; i <= Param.Count; i++) {
		if ( argv[ i - 1 ][0] != '/' ) {
			if ( !SetCurrentDirectory (sFiles.pCurrentDir) )
				return RET_DIRECTORY_ERROR;

   	   bTemp = My_GetNextToken ( argv[ i - 1 ], '/', CurrentParam );
			RTrim ( CurrentParam );	      
			if ( bTemp ) {
				MyReturn = CheckAndCopySpec( CurrentParam );
				if ( MyReturn < RET_NON_CRITICAL)  //Critical Error
				   return MyReturn;
				else {
					if (MyReturn != RET_NO_ERROR)  //Non-Critical Error
						continue; 
				}
	   		#ifdef MYDEBUG
		   	  cout << "DEBUG File Spec(s): " << sFiles.pSpec << endl ;
			   #endif

				MyReturn = CheckAndSetDrive( sFiles.pSpec );
				if ( MyReturn < RET_NON_CRITICAL)  //Critical Error
				   return MyReturn;
				else {
					if (MyReturn != RET_NO_ERROR)  //Non-Critical Error
						continue; 
				}

				MyReturn = CheckAndSetDir( sFiles.pSpec );
				if ( MyReturn < RET_NON_CRITICAL)  //Critical Error
				   return MyReturn;
				else {
					if (MyReturn != RET_NO_ERROR)  //Non-Critical Error
						continue; 
				}
				
				MyReturn = GetAllFiles ( sFiles.hFindHandle );
				if ( MyReturn < RET_NON_CRITICAL)  //Critical Error
				   return MyReturn;
				else {
					if (MyReturn != RET_NO_ERROR)  //Non-Critical Error
						continue; 
				}

         } //bTemp

			if ( i != 2 )	 // we should print newline for the first param
				cout << endl;
		} // argv != '/'
	} //for
	
	if ( !SetCurrentDirectory (sFiles.pCurrentDir) )	//for safety we set to old directory
			return RET_DIRECTORY_ERROR;

	return RET_NO_ERROR;
}

t_ErrorCode CheckAndCopySpec ( char* pSpec ) 
{
	if ( strlen( pSpec ) <= sFiles.iSpec )  
		strcpy( sFiles.pSpec, pSpec );
	else 
		return RET_MEMORY_ERROR;

	return RET_NO_ERROR;
}

t_ErrorCode CheckAndSetDrive ( char* pSpec )
{
	if ( ( *(pSpec) == '\\' ) &&  ( *(pSpec + 1) =='\\' ) ){
		cout << "Network drives not supported" << endl;
		return RET_NETWORK_FILE;
	}
	
	unsigned int Location;
	Location = zFindChar( pSpec, ':' );

	switch ( Location ) {
	case 1:
		*( sFiles.pDriveName + 0 ) = *( pSpec + 0 );
		*( sFiles.pDriveName + 1 ) = *( pSpec + 1 );
		*( sFiles.pDriveName + 2 ) = '\\';
		*( sFiles.pDriveName + 3 ) = '\0';
		break;
	case -1:
		// No drive specified, we create it. 
		*( sFiles.pDriveName + 0 ) = *( sFiles.pCurrentDir + 0 );
		*( sFiles.pDriveName + 1 ) = *( sFiles.pCurrentDir + 1 );
		*( sFiles.pDriveName + 2 ) = '\\';
		*( sFiles.pDriveName + 3 ) = '\0';
		break;
	default:
		cout << S_INVALID_DRIVE << endl;
		return RET_DRIVE_ERROR;
	}

	switch ( GetDriveType( sFiles.pDriveName ) ) {
	case DRIVE_UNKNOWN : case DRIVE_NO_ROOT_DIR :
		cout << sFiles.pDriveName << S_INVALID_DRIVE  << endl;
		return RET_DRIVE_ERROR;
	case DRIVE_REMOTE:	
		cout << sFiles.pDriveName << S_REMOTE_DRIVE << endl;
		return RET_DRIVE_ERROR;
	case DRIVE_CDROM:
		cout << sFiles.pDriveName << S_CDROM_DRIVE << endl;
		return RET_DRIVE_ERROR;
	case DRIVE_RAMDISK:
		cout << sFiles.pDriveName << S_RAMDISK_DRIVE << endl;
		return RET_DRIVE_ERROR;
	case DRIVE_REMOVABLE: case DRIVE_FIXED:
		break;			
	}

	return RET_NO_ERROR;
}

t_ErrorCode CheckAndSetDir ( char* pSpec )
{
	t_ErrorCode MyReturn;
	sFiles.AllowWithExtension =  TRUE;	

	int Length = strlen( pSpec );
	if ( pSpec[ Length - 1 ] == ':' ) {
		cout << S_FILE_NOT_FOUND << endl;
		return RET_NO_FILE;
	}

	if ( pSpec[ Length - 1 ]  == '.' ) {
		char PrevChar = pSpec[ Length - 2 ];
		switch ( PrevChar ){
		case ':' : case '\\':
			//Check for names like c:., c:\., c:\temp\., if found remove .
			// we should remove from c:\*. c:\test.,\..
			pSpec[ Length - 1 ] = '\0';
			break;
		case '*':
			// Handle extensions
			// We should allow extensions only when the name is like *. or Temp*.,etc
			// names like test. or test are handled effectively by FindFirstFile
			sFiles.AllowWithExtension =  FALSE;
		}
	}

	if ( SetCurrentDirectory( pSpec ) ) {
		//we where able to switch into it so it is a directory
		if ( strcmp( pSpec, "?") != 0 ) {
			if ( !CanDel( TRUE ) )
				return RET_USER_ABORT;
		}

		sFiles.AllowWithExtension = TRUE;

		sFiles.hFindHandle = FindFirstFile( STARDOTSTAR, &FileData );
		MyReturn = CheckFileHandle( sFiles.hFindHandle );
		if ( MyReturn != RET_NO_ERROR ) 
			return MyReturn;
	}
	else {
		//user has typed absolute filespec.
		//since we may switch directory after this we first get the find-first handle
		sFiles.hFindHandle = FindFirstFile( pSpec, &FileData );	
		MyReturn = CheckFileHandle( sFiles.hFindHandle );
		if ( MyReturn != RET_NO_ERROR ) 
			return MyReturn;

		//Check whether the first file returned is a dot, if so get confirmation
		//we should not ask confirmation if user has typed *. so we check
		//the sFiles.AllowWithExtension 
		if ( sFiles.AllowWithExtension && (strcmp( FileData.cFileName, "." ) == 0) ){
			if ( strcmp( pSpec, "?") != 0 ) {
				if ( !CanDel( TRUE ) )	
					return RET_USER_ABORT;
			}
		}

		//even though findfirst handles it correctly we later need
		//absolute path to print so we find the path and switch to it.
		// we look for the last \ or the last : and put a \0 after that
		char *p;
		strcpy ( sFiles.pPathName, pSpec );
		p = strrchr( sFiles.pPathName, '\\' );

		if ( p != NULL ) {
			//we have found a slash, after which is the filename
			// if ( !IsFullDirectory( p + 1 ) )
			//	return RET_USER_ABORT;
			*( p + 1 ) = '\0';
			if ( !SetCurrentDirectory( sFiles.pPathName ) )
				return RET_DIRECTORY_ERROR;
		}
		else{
			p = strrchr( sFiles.pPathName, ':' );
			if ( p != NULL ) { 
				//we have found a colon, after which is the filename
				*( p + 1 ) = '\0';
				if ( !SetCurrentDirectory( sFiles.pPathName ) )
					return RET_DIRECTORY_ERROR;
			}
		}
	}

	return RET_NO_ERROR;
}

t_ErrorCode CheckFileHandle( HANDLE hTestHandle )
{
	if ( hTestHandle == INVALID_HANDLE_VALUE) {	
		DWORD LastError;
		LastError = GetLastError();
		if ( LastError == 3 ) {
			cout << S_PATH_NOT_FOUND << endl;
			return RET_NO_PATH;
		}
		else {
			cout << S_FILE_NOT_FOUND << endl;
			return RET_NO_FILE;
		}
	}
	return RET_NO_ERROR;
}

t_ErrorCode GetAllFiles ( HANDLE hFindHandle )
{
	BOOL FoundNextFile= FALSE;
	BOOL Eureka = FALSE; //Indicates whether we found atleast one validfile
	
	// check whether we have switched directories
	if ( GetCurrentDirectory( sFiles.iPathName, sFiles.pPathName) == 0 )
		return RET_DIRECTORY_ERROR;

	if (strcmp( sFiles.pCurrentDir, sFiles.pPathName ) == 0)
		sFiles.SwitchedDirectory = FALSE;
	else
		sFiles.SwitchedDirectory = TRUE;
#ifdef BULK_DELETE
	MyInit ( );		//Initialization of Shell Routines
#endif
	do {
		BOOL Temp;
		if ( sFiles.AllowWithExtension ) {
			Temp = DeleteFileName( );
			if ( !Eureka && Temp) //we have found the first valid file
				Eureka = TRUE;
		}
		else { //Stop files with extensions
			if ( !ExtensionPresent( FileData.cAlternateFileName, FileData.cFileName ) ) {
				Temp = DeleteFileName( );
				if ( !Eureka && Temp) //we have found the first valid file
					Eureka = TRUE;
			}
		}
		FoundNextFile = FindNextFile( hFindHandle, &FileData );
	} while ( FoundNextFile );

	if ( !FindClose ( sFiles.hFindHandle ) )
		return RET_FIND_ERROR;

	if ( !Eureka ) { 
		/* we never found a file, or whatever file we found where all 
			directories or hidden/system */
		cout  << S_FILE_NOT_FOUND << endl;
      return RET_NO_FILE;
	}

	#ifdef BULK_DELETE
		if ( !Param.Wait )  
			MyDelete( ); // delete all the files whose names are in memory
	#endif

	return RET_NO_ERROR;
}

BOOL PrintFileName() 
{
	if ( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		cout << "<DIR>\t";
	}
	else {
		cout << "     \t";
	}

	if ( GetFullPathName( FileData.cFileName, sFiles.iFullName, sFiles.pFullName, &sFiles.pFileNameOffset) == 0 )
		return RET_FULLPATH_ERROR;

	if ( sFiles.SwitchedDirectory )
		// display full name since we have changed directories
		cout << sFiles.pFullName << endl;
	else
		// display just the filename since we are in the current directory
		cout << FileData.cFileName << endl;

	return TRUE;
}
//!!!!!!!!!!!!!!!!!!! Find Files management code End here !!!!!!!!!!!!!!!

// -----------------------------> Shell functions begin here <--------------

BOOL MyInternalDelete ( char* pFiles ) 
{ 
	//Code which deletes files if pFiles is a double-zero terminated string

   SHFILEOPSTRUCT shMyfile;
   HWND hwndDesktop;

	// Check whether pFiles has any name or not
	if ( strlen( pFiles ) == 0 ) return FALSE;

   hwndDesktop = GetDesktopWindow( );

   memset( &shMyfile, 0, sizeof( shMyfile ) );

   shMyfile.hwnd		= hwndDesktop;
   shMyfile.wFunc		=	FO_DELETE;
   shMyfile.fFlags	=	FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
   shMyfile.pFrom    =	pFiles;
   cout << shMyfile.pFrom;
   
   #ifdef MYDEBUG
      cout << "DEBUG SHFILE: " << pFiles << endl;
   #endif

   SHFileOperation( &shMyfile ) ; //IMPORTANT	

	return TRUE;
}

BOOL MyFileDelete ( char* pFile ) 
{  
	// Function deletes a single file & pFile is absolute filename

   strcpy( DelList.pNames, pFile );

   unsigned int i = strlen( DelList.pNames );

   * ( DelList.pNames + i + 1 ) = '\0'; //Maintain double zero-termination

   return MyInternalDelete( DelList.pNames );
}

BOOL MyAddFile ( char* pFile )
{
	//pFile is absolute filename
   unsigned int i = strlen( pFile );
 
   //Check whether we have enough memory to copy this name also, if not
   // we delete these files now. 11 is a random buffer number for safety
   if ( ( (DelList.pOffset - DelList.pNames) + i + 11) >= MEM_BLOCK_SIZE ) {
      MyInternalDelete( DelList.pNames );
      MyInit( );
   }

   strcpy( DelList.pOffset, pFile );				// Copy this name into memory      
   * ( DelList.pOffset + i + 1 ) = '\0';			//Maintain double zero-termination
   DelList.pOffset = (DelList.pOffset + i + 1); //Increment the pointer to point to next slot
   
   return TRUE;
}

void MyInit ( void ) 
{ // Initializes the lastly written pointer to start
   DelList.pOffset = DelList.pNames;
}

BOOL MyDelete ( )
{
   return MyInternalDelete( DelList.pNames );
}

BOOL RefreshIcon ( void ) 
{
	DWORD  *pDwItem1, *pDwItem2;
	HWND hwndDesktop;

	pDwItem1 = new DWORD;
	pDwItem2 = new DWORD;
	
	memset( pDwItem1 , 0, sizeof( pDwItem1 ) );
	memset( pDwItem2 , 0, sizeof( pDwItem2 ) );

	hwndDesktop = GetDesktopWindow();
//	SHChangeNotify( SHCNE_DELETE, SHCNF_FLUSH, pDwItem1, pDwItem2 ); 

	InvalidateRect( hwndDesktop, NULL, FALSE );
	delete pDwItem1;
	delete pDwItem2;
	
	return TRUE;
}

// -----------------------------> Shell functions End here <--------------

BOOL DeleteFileName() 
{
	if ( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		if ( ( strcmp(FileData.cFileName,".")==0 ) || ( strcmp(FileData.cFileName,"..")==0) ) {
			return FALSE;
		}
		else if (Param.Directory == FALSE) {
			return FALSE;
		}
	}

	if ( GetFullPathName( FileData.cFileName, sFiles.iFullName, sFiles.pFullName, &sFiles.pFileNameOffset) == 0 )
		return FALSE;

    if ( FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) {
		if ( Param.Hidden ) {
			if ( Param.Verbose ) 
				cout << 'H' << flush;
		}
   	else
			return FALSE;
	}

    if ( FileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ) {
		if ( Param.System ) {
			if ( Param.Verbose ) 
				cout << 'S' << flush;
		}
		else  
			return FALSE;
    }

    if  ( FileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ) {
		if ( Param.Temporary ) {
			if ( Param.Verbose ) 
				cout << 'T' << flush;
		}
		else
			return FALSE;
    }

    if ( FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) {
		if ( Param.ReadOnly ) { 
			if ( Param.Verbose ) 
				cout << 'R' << flush;
		}
      else  
			/* Even though we are not deleting this file, we have found
			a legitimate file */
			return TRUE;
	}

	if ( Param.Verbose ) { // Display filename, so that user can decide
		if ( Param.System || Param.Hidden || Param.ReadOnly || Param.Temporary){
			cout << "\t";
		}
		
	  	if ( sFiles.SwitchedDirectory ){
			// display full name since we have changed directories
			cout << sFiles.pFullName ;
		}
		else{
			// display just the filename since we are in the current directory
			cout << FileData.cFileName ;
		}
	}
         
	if ( !Param.Wait )  {	
		#ifdef BULK_DELETE 
			MyAddFile( sFiles.pFullName );
		#else  //Immediate Delete
			MyFileDelete( sFiles.pFullName ); 
		#endif
	}
	else {
		BOOL InvalidKey;
		do {
			InvalidKey = FALSE;
			char key;
			cout << S_DELETEWAIT << flush;
			key = _getch( );
			if ( (int) key == I_CTRLC ) {
				RefreshIcon ( );
				Release_Memory();
				exit (RET_CTRL_C);		// WARNING PROGRAM EXISTS HERE TOO
			}
			cout << key << flush;

			switch ( key ) {
			case 'N': case 'n':				// skip the file
				// cout << "\t Skipped.";
				break;
			case 'Y': case 'y':				// Delete the file physically 
				MyFileDelete( sFiles.pFullName ); 
				//cout << "\t Deleted.";
				break;
			default:								// wrong key, so repeat
				cout << endl;
				if ( Param.System || Param.Hidden || Param.ReadOnly || Param.Temporary ) 
					cout << "\t";
				cout << sFiles.pFullName << flush;
				InvalidKey = TRUE;
			}
		} while ( InvalidKey ); 
	}
	if ( Param.Verbose ) { 
		cout << endl;
	}

	return TRUE;
}