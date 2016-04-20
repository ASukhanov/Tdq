#ifndef globals_H
#define globals_H

// Uncomment the next line for debugging, if DBG>1 the printout will be very intense
//#define DBG 2

#define NCHAINS 4 // maximum, should be 4 for FEMr1
#define CH_IN_ASIC 128
#define CH_EXTRABYTES 1 // for FOCAL it was 1
//#define ASICS_IN_CHAIN 12 //for MPCEX
#define ASICS_IN_CHAIN 2 //for SiTracker
#define MAXCH NCHAINS*ASICS_IN_CHAIN*CH_IN_ASIC

//#undef SWAP	//to work with SVX channel numbers
//#define SWAP 1	//to work woth strip numbers

#undef PEDESTAL_PROCESSING

#endif //globals_H 
