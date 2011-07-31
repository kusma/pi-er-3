/*
	Lib for Eventyrlig og Ekstatisk Pixelbasert Realtime Animasjon
	aka LEEPRA.
*/

int leepra_open( char* title, BOOL fullscreen );
void leepra_update( void* data );
void leepra_update256( void* data );
void leepra_close();
