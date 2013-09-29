#include "bowl.h"

PSZ PrinterGraphics::pszqueue = "PM_Q_STD";
PBYTE PrinterGraphics::pBuf = NULL;
PDRIVDATA PrinterGraphics::defaultJobProp = NULL;
PSZ PrinterGraphics::defaultPrinter = NULL;
PPRJINFO2 PrinterGraphics::prj2 = NULL;
int PrinterGraphics::queuesCount = 0;

PrinterGraphics::PrinterGraphics( char *documentName, char *printer ) : Graphics()
{
	DEVOPENSTRUC dops;

  strcpy( this->documentName, documentName );

	dops.pszLogAddress = NULL;  // in case PrinterSetup fails, this will tell
																// GetPrinterHPS() to use default printer info

	printerSetup( &dops, printer );

	hps = getPrinterHPS(&dops, &hdc, documentName, 0L, 0L );

  if( hps == GPI_ERROR ) DosBeep( 1000, 1000 );

  GpiCreateLogColorTable( hps, LCOL_PURECOLOR, LCOLF_RGB, 0, 0, NULL );

  GpiSetMix( hps, FM_OVERPAINT );
	GpiSetBackMix( hps, BM_OVERPAINT );

  POINTL ptl;

/*  GpiSetBackMix( hps, BM_LEAVEALONE );
	GpiSetMix( hps, BM_OVERPAINT );*/

/*	GpiBeginPath( hps, 1);
	ptl.x = 0;
	ptl.y = 0;
	GpiSetCurrentPosition( hps, &ptl );
	ptl.x = getWidth();
	ptl.y = getHeight();
	GpiBox( hps, DRO_OUTLINE, &ptl, 0L, 0L );
	GpiEndPath( hps );
	GpiSetClipPath( hps, 1L, SCP_AND );*/

//  ::Graphics( hps );
}

void PrinterGraphics::getQueues()
{
	ULONG  cbBuf;
	ULONG  cTotal;
	ULONG  cbNeeded;
	PSZ    pszComputerName;
	ULONG  ulLevel;
	SPLERR splerr;

  if( pBuf != NULL ) freeQueueMem();

	queuesCount = 0;

	ulLevel = 4L;//3L;
	pszComputerName = (PSZ)NULL ;
	splerr = SplEnumQueue(pszComputerName, ulLevel, pBuf, 0L, /* cbBuf */
												(PULONG)&queuesCount, (PULONG)&cTotal,
												(PULONG)&cbNeeded, NULL);
	if ( splerr == ERROR_MORE_DATA )
	{
		if (!DosAllocMem( (PPVOID)&pBuf, cbNeeded,
										 PAG_READ|PAG_WRITE|PAG_COMMIT) )
		{
			cbBuf = cbNeeded ;
			splerr = SplEnumQueue(pszComputerName, ulLevel, pBuf, cbBuf,
															(PULONG)&queuesCount, (PULONG)&cTotal,
															(PULONG)&cbNeeded, NULL);
			if (splerr != NO_ERROR)
				queuesCount = 0;
		}
	}
}


void PrinterGraphics::freeQueueMem()
{
	DosFreeMem(pBuf);
}

/*
******************************************************************************
**  FUNCTION: PrinterSetup
**  PURPOSE :   This function allows the user to change printer settings.
**
**  PARAMS  :   lpdos - the printer info
**  RETURN  :   BOOL   (TRUE = success)
**
**  DATE    :   11-Dec-1992
**  AUTHOR  :   Carl Samos
******************************************************************************
**  Modified: Date:     Description of changes
**  CNS     :   11-Dec-1992 Initial version
******************************************************************************
*/

BOOL PrinterGraphics::printerSetup(DEVOPENSTRUC FAR *lpdos, PSZ Port )
{
	PPRQINFO3 prq;
	unsigned long ulSize;          /*  The buffer size required  */

	 // get the printer driver, name
	 // GetPrinterInformation allocates space for pszDriver and pszLogicalPort

	PSZ						pszPrinter;
	PSZ 					pszDeviceName;

	if( (prq = getINFO3( Port )) != NULL )
	{
		pszPrinter = prq->pszDriverName;
		pszDeviceName = prq->pszDriverName;

		PSZ pszTmp = strchr( pszPrinter, ',' );

		if( pszTmp) *pszTmp = 0;

		pszDeviceName = strchr( pszDeviceName, '.' );

		if( pszDeviceName )
		{
			*pszDeviceName = 0;
			pszDeviceName++;
		}
		memset( (PVOID)lpdos, 0, sizeof( DEVOPENSTRUC ) );
		prq->pszDriverName[strcspn(prq->pszDriverName, ".")] = '\0';
		lpdos->pszLogAddress = prq->pszName;
		lpdos->pszDriverName = prq->pszDriverName;
		lpdos->pszDataType   = pszqueue;
		lpdos->pdriv         = prq->pDriverData;
	}
	else
		return FALSE;
	return TRUE;
}

HPS PrinterGraphics::getPrinterHPS(DEVOPENSTRUC FAR *lpdops, HDC FAR* lphDC, PSZ pszDocName,
									LONG lWidth, LONG lHeight )
{
	 LONG  lReturn;
	 SIZEL sizl;
	 HPS   hPS;

	 // open the printer DC

	 *lphDC = DevOpenDC (Application::hab, OD_QUEUED, "*", 4L, (PDEVOPENDATA)lpdops, (HDC) NULL);

   if (*lphDC == DEV_ERROR) {
			return(GPI_ERROR);
	 }
   
  // start the document

	DevEscape( *lphDC, DEVESC_STARTDOC, strlen(documentName), 
  					 documentName,NULL, NULL);

	 // get the PS for the printer

   sizl.cx = lWidth;
   sizl.cy = lHeight;

   if (lWidth == 0) {
      hPS =  GpiCreatePS (Application::hab, *lphDC, &sizl,
                          PU_PELS | GPIF_DEFAULT |
                          GPIT_NORMAL | GPIA_ASSOC);
      return(hPS);
   }
   else {
      hPS = GpiCreatePS (Application::hab, *lphDC, &sizl,
                          PU_ARBITRARY | PU_LOENGLISH |
                          GPIF_DEFAULT | GPIT_NORMAL |
                          GPIA_ASSOC);
      return(hPS);
  }

  return(GPI_ERROR);
}

PPRQINFO3 PrinterGraphics::getINFO3( PSZ pszName )
{
	// Loop through list and return the correct one. If noone match, return NULL

	PPRQINFO3 prq;
	prq = (PPRQINFO3)pBuf;

	for( int i = 0; i < queuesCount; i++ )
	{
		if( strcmp( pszName, prq->pszComment ) == 0 )
		{
			BOOL New = FALSE;

			if( defaultJobProp != NULL )
			{
				// Get the stored information
				if( prq->pDriverData->cb == defaultJobProp->cb &&
						strcmp( defaultPrinter, prq->pszComment ) == 0 )
					memcpy( prq->pDriverData, defaultJobProp, defaultJobProp->cb );
				else
					New = TRUE;
			}
			else
				New = TRUE;

			if( New )
				saveJobProp( prq );

			return prq;
		}

		USHORT jobCount = prq->cJobs;

		/* Increment the pointer past the PRQINFO3  structure.         */

		prq++;

		/* Set a pointer to point to the first PRJINFO2 structure.     */

		prj2 = (PPRJINFO2)prq;
		prj2 += jobCount;
		/* After doing all the job structures, prj2 points to the next */
		/* queue structure. Set the pointer for a PRQINFO3 structure.  */
		prq = (PPRQINFO3)prj2;
	}
	return NULL;
}

void PrinterGraphics::saveJobProp( PPRQINFO3 prq )
{
	if( defaultJobProp != NULL )
		free( defaultJobProp );

	if( defaultPrinter != NULL )
		free( defaultPrinter );

	defaultPrinter = (PSZ)malloc( strlen( prq->pszComment ) + 1 );
	defaultJobProp = (PDRIVDATA)malloc( prq->pDriverData->cb );
	memcpy( defaultJobProp, prq->pDriverData, prq->pDriverData->cb );
	strcpy( defaultPrinter, prq->pszComment );
	defaultJobProp->cb = prq->pDriverData->cb;
}

void PrinterGraphics::getPrinters( char ***list, int &count )
{
	PPRQINFO3 prq;

  getQueues();

	prq = (PPRQINFO3)pBuf;

  *list = (char **)malloc( sizeof( char * ) * queuesCount );

  count = queuesCount;

	for( int i = 0; i < queuesCount; i++ )
	{
  	(*list)[i] = (char *)malloc( strlen( prq->pszComment ) + 1 );
    strcpy( (*list)[i], prq->pszComment );

		USHORT jobCount = prq->cJobs;

		/* Increment the pointer past the PRQINFO3  structure.         */

		prq++;

		/* Set a pointer to point to the first PRJINFO2 structure.     */

		prj2 = (PPRJINFO2)prq;
		prj2 += jobCount;
		/* After doing all the job structures, prj2 points to the next */
		/* queue structure. Set the pointer for a PRQINFO3 structure.  */
		prq = (PPRQINFO3)prj2;
	}
}

/*void PrinterGraphics::startDocument()
{
  // start the document

	DevEscape( hdc, DEVESC_STARTDOC, strlen(documentName), 
  					 documentName,NULL, NULL);

}
*/
int PrinterGraphics::getWidth()
{
	int r;

	DevQueryCaps( hdc, CAPS_WIDTH, 1, (PLONG)&r );

	return r;
}

int PrinterGraphics::getHeight()
{
	int r;

	DevQueryCaps( hdc, CAPS_HEIGHT, 1, (PLONG)&r );

	return r;
}

int PrinterGraphics::getXRes()
{
	int r;

	DevQueryCaps( hdc, CAPS_HORIZONTAL_RESOLUTION, 1, (PLONG)&r );

	return r;
}

int PrinterGraphics::getYRes()
{
	int r;

	DevQueryCaps( hdc, CAPS_VERTICAL_RESOLUTION, 1, (PLONG)&r );

	return r;
}

void PrinterGraphics::formfeed()
{
	DevEscape( hdc, DEVESC_NEWFRAME, 0, NULL, NULL,NULL );
}

void PrinterGraphics::terminateDocument()
{
	DevEscape( hdc, DEVESC_ABORTDOC, strlen(documentName),
						 documentName, NULL, NULL );
}

void PrinterGraphics::endDocument()
{
	DevEscape(hdc, DEVESC_ENDDOC, strlen(documentName),
						documentName, NULL, NULL );
}

void PrinterGraphics::setupPrinter( char *name )
{
  PPRQINFO3 prq;

  if( (prq = getINFO3( name )) != NULL )
  {
  	ULONG cbNeeded;

   	CHAR        achDriverName[128];
   	CHAR        achDeviceName[128];
   	INT         i;
   	LONG        l;
   	PSZ         pszTemp;

   	/*
  	 * The pszDriverName is of the form DRIVER.DEVICE (e.g.,
  	 * LASERJET.HP LaserJet IID) so we need to separate it
  	 * at the dot
  	 */
   	i = strcspn(prq->pszDriverName, ".");
   	if (i)
   	{
  		strncpy(achDriverName, prq->pszDriverName, i);
  		achDriverName[i] = '\0';
  		strcpy(achDeviceName, &prq->pszDriverName[i + 1]);
   	}
   	else
   	{
  		strcpy(achDriverName, prq->pszDriverName);
  		*achDeviceName = '\0';
   	}

   /*
  	* There may be more than one printer assigned to this print queue
  	* We will use the first in the comma separated list.  We would
  	* need an expanded dialog for the user to be more specific.
  	*/

   	pszTemp = strchr(prq->pszPrinters, ',');

   	if ( pszTemp )
   	{
  		/* Strip off comma and trailing printer names */
  		*pszTemp = '\0' ;
   	}

   	/*
  	 * Post the job properties dialog for the printer to allow the
  	 * user to modify the options
  	 */
   	DevPostDeviceModes( Application::hab,
   											prq->pDriverData,
   										  achDriverName,
   										  achDeviceName,
   										  prq->pszPrinters,
   										  DPDM_POSTJOBPROP );

  	saveJobProp(prq );
  }
}

void PrinterGraphics::saveSettings( FILE *f )
{
}

void PrinterGraphics::restoreSettings( FILE *f )
{
}

void PrinterGraphics::drawStringClip( int x, int y, Rect *rect, int options, int length,
                           STRING str )
{
  RECTL orect = { rect->x, rect->y,
                  rect->x+rect->width, rect->y + rect->height };
	POINTL ptl;

	GpiBeginPath( hps, 1);
	ptl.x = rect->x;
	ptl.y = rect->y;
	GpiSetCurrentPosition( hps, &ptl );
	ptl.x += rect->width;
	ptl.y += rect->height;
	GpiBox( hps, DRO_OUTLINE, &ptl, 0L, 0L );
	GpiEndPath( hps );
	GpiSetClipPath( hps, 1L, SCP_AND );

  ptl.x = x;
  ptl.y = y;
  GpiSetColor( hps, fc );
  GpiSetBackColor( hps, bc );
  int foptions = CHS_CLIP | ((options & fontOpaque) ? CHS_OPAQUE : 0);
  if( length > 512 ) length = 512;

  GpiCharStringPosAt( hps, &ptl, &orect, foptions, length, str, NULL );
}

PrinterGraphics::~PrinterGraphics()
{
	GpiAssociate(hps, (HDC) NULL);
	GpiDestroyPS(hps);
	DevCloseDC(hdc);
};