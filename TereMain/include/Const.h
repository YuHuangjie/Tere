#ifndef CONST_H
#define CONST_H

const int MAX_VERTEX = 1000000;			// maximum vertex size
const int MAX_FACE = MAX_VERTEX * 2;	// maximum face size

const int BYTES_PER_VERTEX = sizeof(float) * 3;	
const int BYTES_PER_FACE = sizeof(int) * 3;

// maximum number of interpolation cameras
#ifndef MAX_NUM_INTERP
#define MAX_NUM_INTERP 10
#endif
#define STRINGIFY(i) #i
#define STR_MAX_NUM_INTERP(i) STRINGIFY(i)

#endif /* CONST_H */
