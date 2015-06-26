#include <stdio.h>
#include "jpeglib.h"
/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>



void resampleline( JSAMPLE *buffer , JSAMPLE *newbuf , JSAMPLE *lastline, double x1, double x2, double xd, double yd );


/******************** JPEG COMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to feed data into the JPEG compressor.
 * We present a minimal version that does not worry about refinements such
 * as error recovery (the JPEG code will just exit() if it gets an error).
 */


/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 * For this example, we'll assume that this data structure matches the way
 * our application has stored the image in memory, so we can just pass a
 * pointer to our image buffer.  In particular, let's say that the image is
 * RGB color and is described by:
 */

JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;		/* Number of columns in image */


/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */
  struct jpeg_compress_struct winfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */


GLOBAL(void) init_write_JPEG_file (char * filename, int quality, long w, long h )
{
  int row_stride;		/* physical row width in image buffer */

  winfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&winfo);
  if ( filename ) {
   if ((outfile = fopen(filename, "wb")) == NULL) {
     fprintf(stderr, "can't open %s\n", filename);
     exit(1);
   }
  } else { outfile = stdout; }
  
  jpeg_stdio_dest(&winfo, outfile);

  winfo.image_width = w; 	/* image width and height, in pixels */
  winfo.image_height = h;
  winfo.input_components = 3;		/* # of color components per pixel */
  winfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&winfo);
  jpeg_set_quality(&winfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&winfo, TRUE);
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */
}

GLOBAL(void) write_JPEG_line( JSAMPLE *line )
{
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */

  if (winfo.next_scanline < winfo.image_height) {
    row_pointer[0] = line;
    (void) jpeg_write_scanlines(&winfo, row_pointer, 1);
  } else {
	  jpeg_finish_compress(&winfo);
	  if ( outfile != stdout ) fclose(outfile);
	  jpeg_destroy_compress(&winfo);
  }
}



struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



long read_JPEG_info(char * filename, long *x, long *y )
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  *x = cinfo.output_width;
  *y = cinfo.output_height;

  //(void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  /* And we're done! */
  return 1;
}


/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */


GLOBAL(int) read_JPEG_file (char * filename, long x, long y, long w, long h, long z )
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  { double xd,yd, x1,y1, x2,y2,nexty;
  	long lines=0;
    char *newbuf = (char *)malloc( row_stride );
    char *lastline = (char *)malloc( row_stride  );
    
    xd = cinfo.output_width / (double)(w*(1<<z));
    yd = cinfo.output_height / (double)(h*(1<<z));
    x1 = x; y1 = y;
    x2 = x+(xd*w); y2 = y+(yd*h);
    init_write_JPEG_file ( 0, 65-(z*2), w, h );
	nexty = y1;

  	while ( cinfo.output_scanline < cinfo.output_height && cinfo.output_scanline < y2 ){
  	 if( jpeg_read_scanlines(&cinfo, buffer, 1) ){
  	  while( (long)cinfo.output_scanline-1 == (long)(nexty) ){
  	  	if ( yd<1 )
	  	  	resampleline( buffer[0], newbuf, lastline, x1, x2, xd, nexty );
	  	else
	  	  	resampleline( buffer[0], newbuf, lastline, x1, x2, xd, 0 );
  	  	write_JPEG_line( newbuf );
  	  	nexty = y1+(lines*yd);
  	  	lines++;
  	  }
  	  memcpy( lastline, buffer[0], row_stride );
  	 }
  	}
  }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  /* And we're done! */
  return 1;
}


#define	SCALEB	(14)
#define	SCALEV	(1000)

#define	GETPIXEL(ptr,r,g,b)	(r=*ptr++; g=*ptr++; b=*ptr++;)

// when zooming in close read pixels in percentage scales from 
// left and top of current pixel to get a 3dFX style ZOOM
void resampleline(
 JSAMPLE *buffer , 	// source buffer
 JSAMPLE *newbuf , 	// out put buffer
 JSAMPLE *lastline, 	// previous line before this one (source)
 double x1, 		// x start pos
 double x2, 		// x end pos
 double xd, 		// x scale zoom level
 double y )		// y pos of source
{
	JSAMPLE *s, *s2, *d;
	long r,g,b, r2,g2,b2, r3,g3,b3;
	long perc = (y - (long)y)*SCALEV, perc2;
	perc2 = SCALEV-perc;
		
	d = newbuf;
	while( x1 < x2 )
	{
		// read pixel
		s = buffer + (3*(long)x1);
		r = *(s++);
		g = *(s++);
		b = *(s++);

		if ( y >= 1 && x1 >= 1 )
		{
			long xperc = (x1 - (long)x1)*SCALEV, xperc2;
			xperc2 = SCALEV-xperc;

			s2 = lastline + (3*(long)x1);
			r2 = *(s2++);g2 = *(s2++);b2 = *(s2++);
			r = (r2*perc2 + r*perc)/SCALEV;
			g = (g2*perc2 + g*perc)/SCALEV;
			b = (b2*perc2 + b*perc)/SCALEV;

			s2 = buffer + (3*(long)(x1-1));
			r2 = *(s2++);g2 = *(s2++);b2 = *(s2++);
			s2 = lastline + (3*(long)(x1-1));
			r3 = *(s2++);g3 = *(s2++);b3 = *(s2++);

			r2 = (r3*perc2 + r2*perc)/SCALEV;
			g2 = (g3*perc2 + g2*perc)/SCALEV;
			b2 = (b3*perc2 + b2*perc)/SCALEV;

			r = (r2*xperc2 + r*xperc)/SCALEV;
			g = (g2*xperc2 + g*xperc)/SCALEV;
			b = (b2*xperc2 + b*xperc)/SCALEV;
		}
		// write pixel
		*(d++) = r;
		*(d++) = g;
		*(d++) = b;

		x1 += xd;
	}
	
}



/*

HTTP/1.1 200 OK
Date: Sat, 26 Dec 1998 13:12:50 GMT
Server: Apache/1.2.5
Last-Modified: Sun, 03 Aug 1997 15:31:57 GMT
ETag: "12de5-41d-33e4a46d"
Content-Length: 1053
Accept-Ranges: bytes
Connection: close
Content-Type: image/gif 
*/


int PrintFile( char *name )
{
	FILE *fp;
	
	if ( fp=fopen( name, "r" ) ){
		char line[300];
		while( !feof(fp) ){
			fgets( line, 300, fp );
			printf( line );
		}
		return 1;
	} else
		return 0;
}

void WriteHeader( int type )
{
	switch ( type ){
		case 0:
			printf( "Content-type: text/html\n\n" ); break;
		case 1:
			printf( "Accept-Ranges: bytes\nConnection: close\n" );
			printf( "Content-type: image/jpeg\n\n" ); break;
	}
}


void WriteHTMLTop( char *file )
{

	if ( !PrintFile( file ) ){
		printf( "<html>	<head>	<title>Zoom in for a closer view [GPL]</title>	</head>	<body bgcolor=#00000>\n" );
		printf( "<FONT COLOR=RED size=+3><I><B>Zoom in for a closer view [GPL]</B></I><BR><BR></FONT>\n" );
		printf( "<FONT COLOR=white size=-1><BR>\n" );
	}
	printf( "\n\n<!--\n\t\t-------------- ZOOM CODE BEGINS --------\n\n-->\n\n");	
}

void WriteHTMLBot( char *file )
{
	printf( "\n\n<!--\n\t\t-------------- ZOOM CODE ENDS --------\n\n-->\n\n");	
	if ( !PrintFile( file ) )
		printf( "</body>\n</html>\n\n" );
	
}


void ShowZoomLink( char *name, char *img, char *htmlargs, long w, long h, long x, long y, long z )
{
	if ( *htmlargs )
		printf( "<A HREF=\"zoom.cgi?f=%s&w=%d&h=%d&x=%d&y=%d&z=%d&t=0&%s\"> %s </A> \n", img, w,h, x,y,z, htmlargs, name );
	else
		printf( "<A HREF=\"zoom.cgi?f=%s&w=%d&h=%d&x=%d&y=%d&z=%d&t=0\"> %s </A> \n", img, w,h, x,y,z, name );
}


#define TerminateParam(x)	if (p=strchr( x, '&' ) ) *p=0;

int main( int argc, char **argv )
{
	char	*sourcefile, *param, *p, type=0,
			*foothtml="footer.html",
			*headhtml="header.html";
	char	htmlargs[300] = "";
	char	cgi_param[4096];
	long	z=0,  // zoom level
			w=300, h=-1,		// output width/height
			x=0, y=0,			// source x/y pos
			cx=0, cy=0,			// new center pos
			iw, ih;				// originalsource width/height dimmensions
	long	scale, wperc=100, hperc=100,
			filestat;
	
	if ( argc>1 )
		param = argv[1];
	else
		param = getenv( "QUERY_STRING" );
	
	if( param ){
		strcpy( cgi_param, param );
		// get cgi parameters
		if ( p=strstr( param, "w=" ) ){ w=atoi(p+2); }
		if ( p=strstr( param, "h=" ) ){ h=atoi(p+2); }
		if ( p=strstr( param, "z=" ) ){ z=atoi(p+2); }
		if ( p=strstr( param, "x=" ) ){ x=atoi(p+2); }
		if ( p=strstr( param, "y=" ) ){ y=atoi(p+2); }
		if ( p=strstr( param, "xy=" ) ){
			cx=atoi(p+4);
			p = strchr( p+4, ',' );
			cy=atoi(p+1);
		}
		if ( p=strstr( param, "t=" ) ){ type=atoi(p+2); }

		if ( p=strstr( param, "f=" ) ){ sourcefile = p+2;
			TerminateParam(sourcefile)
			if (p=strchr( sourcefile, '\\' ) ) *p=0;
		}
		if ( p=strstr( param, "head=" ) ){ headhtml = p+5;
			TerminateParam(headhtml)
			sprintf( htmlargs, "head=%s", headhtml );
		}
		if ( p=strstr( param, "foot=" ) ){ foothtml = p+5;
			TerminateParam(foothtml)
			sprintf( htmlargs, "head=%s&foot=%s", headhtml, foothtml );
		}

		if( z<0 ) z=0;				// limit the zoom ability, not too ridiculously far
		if( z>31 ) z=31;
		
		WriteHeader( type );

		filestat = read_JPEG_info( sourcefile, &iw, &ih );
		if ( w==0 ) w=iw;
		if ( h==0 ) h=ih;
		

		if ( z>0 ){		// calc a new real X/Y relative to image src
			long vx, vy;

			scale = 1 << (z-1);
			vx = (cx - (w/4));
			vy = (cy - (h/4));
			if ( vx<0 ) vx = 0;
			if ( vy<0 ) vy = 0;
			if ( vx>w/2) vx = w/2;
			if ( vy>h/2) vy = h/2;
			x += ((vx*iw)/w)/scale;
			y += ((vy*ih)/h)/scale;
		} else {
			if ( h <=0 )
				h = (ih*w)/iw;
			else
			if ( w <=0 )
				w = (iw*h)/ih;
		}
		scale = 1<<z;

		if( x<0 ) x=0;
		if( y<0 ) y=0;
		if ( x>iw-(iw/scale) ) x = iw-(iw/scale);
		if ( y>ih-(iw/scale) ) y = ih-(ih/scale);
		
		if( type == 0 ){
			WriteHTMLTop( headhtml );
			printf( "<!--\nDEBUG: w=%d,h=%d, x=%d,y=%d, cx=%d,cy=%d, iw=%d,",w,h,x,y,cx,cy, iw );
			printf( "\nparam='%s', iw=%d, ih=%d\n" , cgi_param, iw, ih );
			printf( "scale=%ld, fs=%ld \n-->\n" , scale,filestat );

			printf( "<A HREF=\"zoom.cgi?f=%s&w=%d&h=%d&x=%d&y=%d&z=%d&t=0&%s&xy=\"><BR>\n", sourcefile, w,h, x,y, z+1, htmlargs );
			if ( filestat ){
				printf( "<IMG SRC=\"zoom.cgi?f=%s&w=%d&h=%d&x=%d&y=%d&z=%d&t=1\" ISMAP border=0><BR>\n", sourcefile, w,h, x,y,z );
			} else
				printf( "ERR: file %s not found<BR>\n", sourcefile );
			printf( "</A><BR>\n\n" );

			ShowZoomLink( "Top",	sourcefile, htmlargs, w,h, x,y,0 );
			ShowZoomLink( "ZoomIn", sourcefile, htmlargs, w,h, x,y,z+1 );
			ShowZoomLink( "ZoomOut",sourcefile, htmlargs, w,h, x,y,z-1 );
			ShowZoomLink( "Left",	sourcefile, htmlargs, w,h, x-(iw/2/scale),y,z );
			ShowZoomLink( "Right",	sourcefile, htmlargs, w,h, x+(iw/2/scale),y,z );
			ShowZoomLink( "Up",		sourcefile, htmlargs, w,h, x,y-(iw/2/scale),z );
			ShowZoomLink( "Down",	sourcefile, htmlargs, w,h, x,y+(iw/2/scale),z );

			WriteHTMLBot( foothtml );
		} else {
			read_JPEG_file ( sourcefile, x, y, w, h, z );
		}
	} else {
		WriteHeader( type );
		printf( "Error: no file or option specified<BR>\n\n" );
	}
}





