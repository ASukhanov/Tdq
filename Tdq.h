// Version v2 2016-04-28. gCMNControl public

#ifndef dq_H
#define dq_H

#ifdef _Win32
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN             
// Windows Header Files:
#include <windows.h>
#endif

#include <TFile.h>
#include <TTree.h>
#include <TH1S.h>
#include <TH2S.h>

#include "globals.h"

#define DQ_EOE 0x0DFE

#define ERRDQ_IO -1
#define ERRDQ_STAMP 2

// &RA 20141030//disable this error checking by now
#define ERRDQ_CRC 4
#define ERRDQ_EVSIZE 8
#define ERRDQ_NOEOE 0x10	// bit4
// &RA 20130903//disable this error checking by now//
#define ERRDQ_CELLN 0x20	// bit5
#define ERRDQ_CMNOISE 0x40	// bit6
#define ERRDQ_0CHIP 0x80	// but7
#define ERRDQ_FCELLN_0 0x100	//bit8
#define ERRDQ_FCELLN_1 0x200	//bit9
#define ERRDQ_FCELLN_2 0x400	//bit10
#define ERRDQ_FCELLN_3 0x800	//bit11

//Added by Arbin
#define MAX_STRIP_IN_CHAIN ASICS_IN_CHAIN*CH_IN_ASIC
#define MAX_CH_IN_CHAIN ASICS_IN_CHAIN*(CH_IN_ASIC+1)

#define NPLANES 4
// Plane description
struct Plane_Def 
{  
  Int_t chain;            // FEM chain, the plane belongs to
  Int_t first_ch;         // index of the first strip in the chain
  Int_t last_ch;          // index of the last strip in the chain
  Int_t mapping_version;  // mapping version, not used for now
};

typedef Short_t CHV_t;
//typedef UChar_t CHV_t;

class Tdq : public TFile 
{
private:
        TTree	*ftree;
        Char_t	fname[256];
        const Char_t	*fhtitle;
        FILE    *fD;
        TFile   *ffile;
        Int_t	fsize;
        Int_t	fevcount;
        UInt_t	fevnum;
        Int_t   fNSkippedEvents;
        Int_t	fnchn; // length of the longest chain
        UShort_t	fchn[MAX_CH_IN_CHAIN];
        Int_t flchain[NCHAINS];
        CHV_t	fchv[NCHAINS][MAX_CH_IN_CHAIN];
        CHV_t	fcmnoise[NCHAINS][6];

        Float_t ped[NCHAINS][MAX_STRIP_IN_CHAIN];
        
        #define DQ_MINHDR 20	//&RA/141123/ was 14
        UChar_t f1sthdr[DQ_MINHDR];
        UChar_t fhdr[DQ_MINHDR];
        UInt_t frec_length;

        //elements of the tree
        ULong_t fpos; // file position
        UShort_t fcrc;
        UChar_t fevhl,fevtl;
        UChar_t fevnasics,fevchains;
        Int_t	fentry;
        UChar_t fcelln[NCHAINS][6]; // bunch number
        ULong_t fbclk, fbclkx;
        UShort_t fPARst_ExTrig; // time (in bclk*8) from preamp reset to external trigger, the bits [2:0] also could be used as clockphase of the ExTrig
        UChar_t fclkphase; // clock phase of the trigger 0:7
        UShort_t fHPARTime;
        UShort_t fHL1Stack;
        UShort_t fHDigTime;
        UShort_t fHFStatus;
        UShort_t fHPrevL1;

        ULong_t fevsize;    // event length
        ULong_t ferr;   // error. each bit has its meaning
        ULong_t ferrcount; // number off error events
        Int_t	fnerr;	// number of errors in event
        ULong_t fclkprev;
        ULong_t fclkdiff;
        //ULong_t ftimediff,fprevtime; // time difference with prev event
        //ULong_t ftime80MHz;	// 80MHz timer counter, reset by external signal
        //ULong_t ftime80MHzDiff, fprevtime8;
        TH2S *fhchns[NCHAINS];
        static Float_t gCMNQuantile;
        static Int_t gCMNLimit;

        UChar_t Bin2Gray(UChar_t gray);
        Int_t GetHeader();
        Int_t FindEOE(); // position file to next event
        Int_t Process();
        Int_t Next();
        Int_t UpdateCRC(UShort_t*, UShort_t* ptr, Int_t len);
        void	DoCMNoise(Int_t chain);

  public:
        static Int_t gDebug;
        static Int_t gExtraWords;
        static Int_t gStripMapping;
        static Int_t gSubtractPeds;
        static Int_t gMaxEntries;
        static Int_t gCMNControl;
        static int gHitThreshold;
        static Int_t gClustering;
        static struct Plane_Def gplane[NPLANES];
        Int_t fplane_hitp[NPLANES];
        CHV_t fplane_hitv[NPLANES];
        Int_t fplane_nhits[NPLANES];
        
        void FillErr();
        TH1I *fherr;
        TArrayI fchnmap;

        Tdq(const Char_t *name="", Int_t cmnproc_mode=0, const Char_t *htitle="");
        ~Tdq();
        //Bool_t IsOpen();
        //void SetChnMap(Int_t ich, Int_t pedestal);
        Bool_t IsOpen();

        //Added by Arbin
        //v12/void Swap(Int_t svx4ch, Int_t stripch);
        void SetPed(Int_t chain, Int_t ich, Float_t pedestal, Int_t status);
        
        Char_t* GetFileName() {return fname;}
        Int_t GetLChain(Int_t chain) {return (flchain[chain]);}
        TH2S* GetChain(Int_t chain) {return (fhchns[chain]);}
        //v12/TH2S* GetHits(Int_t chain) {return (histoadc[chain]);}
        TTree* MakeTree(Int_t mode);
        void SetCMNoiseQuantile(Float_t value) {gCMNQuantile = value;}
        //gCmnLimit is a level of the adc[quantile], above which an error 
        //will be assigned to the event
        void SetCMNoiseLimit(Int_t value) {gCMNLimit = value;}
        void SetCMNoiseControl(Int_t value);
        Int_t GetCMNoiseControl() {return gCMNControl;}
        Int_t Pad(Int_t channel); //converts chain channel nmber to pad number

        ClassDef(Tdq,0) // processing class for PHENIX NCC data 
};
#endif
