#  ZOOM 1.01

  Ever wanted a technology solution to allow you to view high res images
  from a server in thumbnail size or in smaller size and then have the ability
  to zoom in closer to see more detail ? Now you can and its free with source
  for all to use. All ability is in the server, so no need for client side
  modifications, all you need is this cgi that uses libjpeg and thats it. 
  And the BEST thing is its FREE, no fees to any corporation per image/pixel.

## INSTALL:

	type make and thats it, copy the zoom file to your web folder as
    zoom.cgi


## HOWTOUSE:

	simply call it as follows

		http://yoursite.com/folder/zoom.cgi?f=FILENAME.JPG&w=0

	Thats it, w=0 is used to default the image to source image size and proportional
    in X/Y. You could other wise do w=200&h=200 or even w=200, either way
    if w or h is <= 0, then its relatively proportional to its source image size.
	If w or h is -1, then the size is relative to the other value.

	Once viewed in your browser, click on the image to zoom in.

	You can insert custom html headers and footers, either call the files names
	header.html and footer.html in the same directory as the cgi or specify
	them in the cgi args as head=this.html&foot=end.html, too easy.


## HOWITWORKS:

	Simply calling the cgi will by default return html code which will then 
	self include it self, the parameter t=0 means to return html code, and
	the parameter t=1 means to output a jpeg binary file.

	Default jpeg output is at 65% quality, and lessons by 2% for each zoom.

	The code decodes the source jpeg and outputs a new jpeg at a new position
    and mag size, it decodes one line at a time so as not to use all of your
    machines memory. It uses the libjpeg library so in theory whatever the
	library can understand it will also understand, so .tiff source files
	should work.




## LICENCE:

	GPL, see COPYING text file.



## INSPIRATION:

	A dream.


