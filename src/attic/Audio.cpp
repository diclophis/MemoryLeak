//

#include <iostream>
#include <cstdlib>

#include "Audio.h"

// Two-channel sawtooth wave generator.
int Audio::saw( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, void *userData ) {
  unsigned int i, j;
  double *buffer = (double *) outputBuffer;
  double *lastValues = (double *) userData;

  // Write interleaved audio data.
  for ( i=0; i<nBufferFrames; i++ ) {
    for ( j=0; j<2; j++ ) {
      *buffer++ = lastValues[j];
      lastValues[j] += 0.005 * (j+1+(j*0.1));
      if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
    }
  }

  return 0;
}

Audio::Audio() {
  
}
