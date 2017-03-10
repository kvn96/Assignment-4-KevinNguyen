#include "pixutils.h"
#include "bmp/bmp.h"

//private methods -> make static
static pixMap* pixMap_init();
static pixMap* pixMap_copy(pixMap *p);

//plugin methods <-new for Assignment 4;
static void rotate(pixMap *p, pixMap *oldPixMap,int i, int j,void *data);
static void convolution(pixMap *p, pixMap *oldPixMap,int i, int j,void *data);
static void flipVertical(pixMap *p, pixMap *oldPixMap,int i, int j,void *data);
static void flipHorizontal(pixMap *p, pixMap *oldPixMap,int i, int j,void *data);

static pixMap* pixMap_init(){
	pixMap *p=malloc(sizeof(pixMap));
	p->pixArray_overlay=0;
	return p;
}	

void pixMap_destroy (pixMap **p){
	if(!p || !*p) return;
	pixMap *this_p=*p;
	if(this_p->pixArray_overlay)
	 free(this_p->pixArray_overlay);
	if(this_p->image)free(this_p->image);	
	free(this_p);
	this_p=0;
}
	
pixMap *pixMap_read(char *filename){
	pixMap *p=pixMap_init();
 int error;
 if((error=lodepng_decode32_file(&(p->image), &(p->imageWidth), &(p->imageHeight),filename))){
  fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
  return 0;
	}
	p->pixArray_overlay=malloc(p->imageHeight*sizeof(rgba*));
	p->pixArray_overlay[0]=(rgba*) p->image;
	for(int i=1;i<p->imageHeight;i++){
  p->pixArray_overlay[i]=p->pixArray_overlay[i-1]+p->imageWidth;
	}			
	return p;
}
int pixMap_write(pixMap *p,char *filename){
	int error=0;
 if(lodepng_encode32_file(filename, p->image, p->imageWidth, p->imageHeight)){
  fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
  return 1;
	}
	return 0;
}	 

pixMap *pixMap_copy(pixMap *p){
	pixMap *new=pixMap_init();
	*new=*p;
	new->image=malloc(new->imageHeight*new->imageWidth*sizeof(rgba));
	memcpy(new->image,p->image,p->imageHeight*p->imageWidth*sizeof(rgba));	
	new->pixArray_overlay=malloc(new->imageHeight*sizeof(void*));
	new->pixArray_overlay[0]=(rgba*) new->image;
	for(int i=1;i<new->imageHeight;i++){
  new->pixArray_overlay[i]=new->pixArray_overlay[i-1]+new->imageWidth;
	}	
	return new;
}

	
void pixMap_apply_plugin(pixMap *p,plugin *plug){
	pixMap *copy=pixMap_copy(p);
	for(int i=0;i<p->imageHeight;i++){
	 for(int j=0;j<p->imageWidth;j++){
			plug->function(p,copy,i,j,plug->data);
		}
	}
	pixMap_destroy(&copy);	 
}

int pixMap_write_bmp16(pixMap *p,char *filename){
 BMP16map *bmp16=BMP16map_init(p->imageHeight,p->imageWidth,0,5,6,5); //initialize the bmp type
 if(!bmp16) return 1;
 

	//bmp16->pixArray[i][j] is 2-d array for bmp files. It is an-logous to the one for our png file pixMaps except that it is 16 bits
	
 //However pixMap and BMP16_map are "upside down" relative to each other
 //need to flip one of the the row indices when copying
 //uint16_t temp16 = 0;
 //Do a for loop through image height then width,
 for(int i = 0; i < p->imageHeight; i++) {
   for(int j = 0; j < p->imageWidth; j++) {
    uint16_t r16 = p -> pixArray_overlay[i][j].r
    uint16_t g16 = p -> pixArray_overlay[i][j].g
    uit16_t b16 = p -> pixArray_overlay[i][j].b
    //RRRRRrrr this pushes the lower case out.
    r16 = (r16 & 0xF0) >> 3;
    g16 = (g16 & 0xF0) >> 2;
    b16 = (b16 & 0xF0) >> 3;
    //a16 = (a16 & 0xF0);
    //Pushes the big R.
    
    r16 = (r16 & 0xF0) << 11;
    g16 = (g16 & 0xF0) << 5;
    b16 = (b16 & 0xF0); 
    //a16 = (a16 & 0xF0);
    //Do i have to initialize this as 0 in the beginning?
    uint16_t temp16 = r16 | g16 | b16 | a16;
    //this flips it.
    bmp16 -> pixArray_overlay[p->imageHeight -i -1][j]= temp16;
`
    }
 }
 BMP16map_write(bmp16,filename)
 BMP16map_destroy(&bmp16);
 return 0;
}	 
void plugin_destroy(plugin **plug){
 //free the allocated memory and set *plug to zero (NULL)
 //How different is this from the other destroy? Need help on this.
 	if(!plug || !*plug) return;
	plugin *this_plug=*plug;
	if(this_plug->function)
	 free(this_plug->function);// 
	if(this_plug->data)free(this_plug->data);
	free(this_plug);
	this_plug=0;
}

plugin *plugin_parse(char *argv[] ,int *iptr){
	//malloc new plugin
	plugin *new=malloc(sizeof(plugin));
	new->function=0;
	new->data=0;
	
	int i=*iptr;
	if(!strcmp(argv[i]+2,"rotate")){
		//code goes here
      
     new->function=rotate;
     new->data=malloc(2 *sizeof(float));
     float *sc=(float*) new->data;
     int i=*iptr;
     float theta=atof(argv[i+1]);
     sc[0]=sin(degreesToRadians(-theta));
     sc[1]=cos(degreesToRadians(-theta));
   	*iptr=i+2;
   		return new;	
  }
  if(!strcmp(argv[i]+2,"convolution")){{
				//code goes here
	  new -> function = convolution;
      new -> data = malloc (9*sizeof(int));
      float *kernel=(float*) new->data;
      for(int j = 0; j < 9; j++) {
        float newKernel = atoi(argv[i+j])
        kernel[j] = newKernel;
      }
      *iptr=i+10;	
       return new;
	}
	if(!strcmp(argv[i]+2,"flipHorizontal")){
			//code goes here
         //Does this just call the function on the bottom.	
     new -> function = flipHorizontal;   
     *iptr=i+1;
     return new;
	}
	if(!strcmp(argv[i]+2,"flipVertical")){
		//code goes here
      new -> function = flipVertical;
      *iptr=i+1;
      return new;
	}		
	return(0);
}	

static void rotate(pixMap *p, pixMap *oldPixMap,int i, int j,void *data){
 float *sc=(float*) data;
 const float ox=p->imageWidth/2.0f;
 const float oy=p->imageHeight/2.0f;
 const float s=sc[0];
	const float c=sc[1];
	const int y=i;
	const int x=j;
	float rotx = c*(x-ox) - s * (oy-y) + ox;
 float roty = -(s*(x-ox) + c * (oy-y) - oy);
 int rotj=rotx+.5;
	int roti=roty+.5; 
	if(roti >=0 && roti < oldPixMap->imageHeight && rotj >=0 && rotj < oldPixMap->imageWidth){
   memcpy(p->pixArray_overlay[y]+x,oldPixMap->pixArray_overlay[roti]+rotj,sizeof(rgba));			 
		}
		else{
   memset(p->pixArray_overlay[y]+x,0,sizeof(rgba));		
		}		
}


static void convolution(pixMap *p, pixMap *oldPixMap,int i, int j,void *data){
	//implement algorithm givne in https://en.wikipedia.org/wiki/Kernel_(image_processing)
	//assume that the kernel is a 3x3 matrix of integers
	//don't forget to normalize by dividing by the sum of all the elements in the matrix
   
   //height -> width;
   int *kernel = (int *) data;
   int r = 0;
   int g = 0;
   int b = 0;
   //This is for the kernel array.
   int pixelValue = 0;
   //i = imageHeight, j = imageWidth?
	//The bottom for loops is to help remember why I used I and j
	//    for(int y = 0; y < oldPixMap -> imageHeight - 1; y++) {
	//       for(x = 0; x < oldPixMap -> imageWidth -1; x++) {
	for(int y = i - 1; y < i + 1; y++) {
	   int pixLocationI = y;
       for(int x = j - 1; x < j + 1; x++) {
    	  int pixLocationJ = x;
    	  /* Just like the normalize function you have to
    	   * check if the pixValues are beyond the range
    	   * except the ranges are the edge cases?
    	   */
    	 //bitmaps start like graphs, can't be negative.
    	 if(pixLocationI < 0) pixLocationI = 0;
    	 if(pixLocationJ < 0) pixLocationJ = 0;
    	 if(pixLocationI > (oldPixMap -> imageWidth -1)) {
    		 pixLocationI = oldPixMap -> imageWidth -1;
    	 }
    	 if(pixLocationJ > (oldPixMap -> imageHeight -1)) {
    		 pixLocationJ = oldPixMap -> imageWidth -1;
    	 }
         //Java version: red = red + p.getRed() * weight
    	 //accumulator = r, pixel value = pixArray.r? , element value = kernel[pixelValue]?
         r = r + (oldPixMap-> pixArray_overlay[pixLocationI][pixLocationJ].r) * kernel[pixelValue]);
         g = g + (oldPixMap-> pixArray_overlay[pixLocationI][pixLocationJ].r) * kernel[pixelValue]);
         b = b + (oldPixMap-> pixArray_overlay[pixLOcationI][pixLocationJ].r) * kernel[pixelValue]);
         pixelValue++;


      }
   }
   //Normalizing everything

   //theScale = the scale down factor
   int theScale = 0;

   /* Java version
    *red = normalize(red / theScale);
    *green = normalize(green / theScale);
    *blue = normalize(blue / theScale);
    *  for (final int[] row : theWeights) {
            for (final int col : row) {
                sum = sum + col;
    */
   for (int rowAndCol = 0; rowAndCol < 9; rowAndCol++) {
	   //is the kernel[rNc] = col; from the java version (iterating through the pixels right?)
	   theScale = theScale + kernel[rowAndCol];
   }
   r = r / theScale;
   g = g / theScale;
   b = b / theScale;
   //account for negative/too high color values.
   if (theScale == 0) {
       theScale = theScale + 1;
   }

   if (r < 0) r = 0;
   else if (r > 255) r = 255;
   if (b < 0) b = 0;
   else if (b > 255) b = 255;
   if (g < 0) g = 0;
   else if (g > 255) g = 255;
   //newPixels[y][x] = new Pixel(red, green, blue);
   //newPixels[y][x] = p->pixArrayOverlay[i][j].r, new Pixel(red) = r?
   p->pixArrayOverlay[i][j].r =(char)r;
   p->pixArrayOverlay[i][j].g =(char)g;
   p->pixArrayOverlay[i][j].b =(char)b;
}
 
//very simple functions - does not use the data pointer - good place to start
//http://stackoverflow.com/questions/33514842/how-do-i-flip-an-image-vertically-in-java
static void flipVertical(pixMap *p, pixMap *oldPixMap,int i, int j,void *data){
 //The java implementation is imageArr[height - row - 1] i'm assuming this is the equivalent since
 //pixArray_overlay = imageArr and i = row?
 p->pixArray_overlay[ i][j] = oldPixMap -> pixArray_overlay[oldPixMap -> imageHeight - i - 1)][j];		
}	 
 static void flipHorizontal(pixMap *p, pixMap *oldPixMap,int i, int j,void *data){
  //reverse the pixels horizontally - can be done in one line
 //Same as the previous but involves the width instead.
  p->p ixArray_overlay[i][j] = oldPixMap -> pixArray_overlay[i][oldPixMap -> imageWidth - j - 1)];		                                          
}      
