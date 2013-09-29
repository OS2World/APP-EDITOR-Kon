#include "autosave.h"
#include "globals.h"
#include "bufadmin.h"

AutosaveThread *AutosaveThread::instance = NULL;

void AutosaveThread::createInstance() 
{ 
	instance = new AutosaveThread(); 
	instance->start( 16384 );
}

void AutosaveThread::run()
{
	while( 1 )
	{
		System::sleep( Settings::autosaveDelay * 1000 );

		// Loop through all buffers and save them if modified

		if( Settings::autoSave )
		{
			for( int i = 0; i < BufferAdmin::getBuffersCount(); i++ )
			{
				TextBuffer *buffer = BufferAdmin::getBuffer(i);

				if( buffer->textModified() && strrchr( buffer->getFilename(), '\\' ) != NULL )
				{
					char filename[MAXPATH];

					strcpy( filename, Settings::autosavePath );
					strcat( filename, "Autosaved_" );
					strcat( filename, strrchr( buffer->getFilename(), '\\' ) + 1 );
					
					BlockFileWriter *writer = new BlockFileWriter( filename );

					if( writer->getStatus() == 0 )
					{
						buffer->readRequest( NULL );
						buffer->beginEnum();

						writer->setFileSize( buffer->getSize() );

						int blockSize;

						TextElem *text = buffer->enumTextBlock( blockSize );

            if( buffer->getEncoding() != ANSI )
              writer->writeBlock( buffer->getLeadBytes(), buffer->getLeadBytesCount(), ANSI );

						while( text )
						{
							writer->writeBlock( (char *)text, blockSize, buffer->getEncoding() );
							text = buffer->enumTextBlock( blockSize );
						}

						buffer->readRelease();
					}

					delete writer;
				}
			}
		}
	}
}

void AutosaveThread::deleteASVFile( char *buffername )
{
	char filename[MAXPATH];

	if( strrchr( buffername, '\\' ) != NULL )	// Valid filenames always has a "\"
	{
		strcpy( filename, Settings::autosavePath );
		strcat( filename, "Autosaved_" );
		strcat( filename, strrchr( buffername, '\\' ) + 1 );

		System::deleteFile( filename );
	}
}
