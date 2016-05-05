/*
    2013-09-11	Andrey Sukhanov

    *version 3 Analysis package for *.dq4 files, adopted from previous year.
    added PARst_ExTrig - delay between PARst and the external trigger

    2013-10-31	AS
*  - Binning of fhchns corrected xrange changed to 0,256.
*  - Printing FPGA version.
*  - hex position for 'EOE not found'

    2013-11-01	AS
*  - Take care of EOE at the end of file

2013-11-04	AS
*  - Better bookkeeping of errors.
  FindEOE prints corrected excess.

2013-11-08	AS
*  - The EOD stamp is tested first in the event processing.
  - fpos is set only in the GetHeader and it points to the beginning of event.

2014-01-20	AS
*  - detection of extra words was broken when ajusted for handling empty events
#define FINDEOE_ROLLBACK 16
Now the detecttion of empty events is broken

2014-03-17	AS
*  - ASICS_IN_CHAIN changed to 12, added Pad(), corrected number of channels in chain

2014-12-12as
*  - gMaxEntries

2015-12-24 AS
* Event format of FEM-v200+

v11 2016-04-28 work with less than 4 chain

2016-05-04
V12

*/

#include "globals.h"

//&RA/131108/ Instead of recompiling for DBG, use gDebug member.
// To use it in root instantiate 'side' Tdq just to access its static member gDebug
// i.e. Tdq* gg = new Tdq(""); and then manipulate gg.gDebug
// Uncomment the next line for debugging, if DBG>1 the printout will be very intense
//#define DBG 1

//#ifdef DBG
//#define MAXERRPRINT 10000
#define MAXERRPRINT 100
//#else
//#define MAXERRPRINT 4
//#endif

// for ntohl
//#ifndef WIN32
//#include <arpa/inet.h> 
//#else
//#include <winsock2.h> 
//#endif

#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>
#include <TH2S.h>
#include <TMath.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h> // for stat()
#include <arpa/inet.h> //for ntohs()

using namespace std;
using namespace TMath;

#include "Tdq.h"
//#include "svx2strip_nomap.h"	// for debugging
#include "svx2strip_map_v2.h" //v12/ do not use this mapping!
//#include "svx2strip_map_v1.h"

Int_t Tdq::gDebug=0;
Int_t dbg_cmnoise=0;

Int_t   Tdq::gCMNLimit = 40;	// this rejects 10% of events
Int_t   Tdq::gCMNControl = 0;	// &1: call DoCMNoise, &2: subtract CMNoise
Float_t Tdq::gCMNQuantile = 0.25;
Int_t	Tdq::gExtraWords = 0;	// For events with simulated data this should be set to 1
Int_t	Tdq::gStripMapping = 0;	// 1 to map ASIC channel number to sensor strip number
Int_t	Tdq::gSubtractPeds = 0; //
Int_t	Tdq::gMaxEntries = 10000000; // Number of events to process
Int_t   Tdq::gHitThreshold = 12;

struct Plane_Def Tdq::gplane[NPLANES] = {{0, 0, 127, 1},{0, 130, 255, 2},{1, 0, 127, 1},{1, 130, 255, 1}};

void Tdq::SetPed(Int_t chain, Int_t ich, Float_t pedestal, Int_t stats)
{
  if(ich<=MAX_STRIP_IN_PLANE)
  {
    ped[chain][ich]=pedestal;
  }
}

void Tdq::SetCMNoiseControl(Int_t value)
{
  printf("CMNControl is changed from %x to %x\n",
         gCMNControl,value);
  gCMNControl = value;
}

UChar_t Tdq::Bin2Gray(UChar_t gray)
{
  UChar_t rc=gray;
  rc ^= (gray>>1)&0x7f;
  return rc;
}
Int_t Tdq::UpdateCRC(UShort_t *crc, UShort_t* ptrs, Int_t len)
{
    Int_t ii;
    //UShort_t *ptr = (UShort_t*)ptrb;
    for(ii=0;ii<len;ii++) *crc ^= *ptrs++;
    return 0;
}
#define DATA_ARE_ADC !((f1sthdr[6])&0x80) //&RA150914

Int_t Tdq::GetHeader()
{
  Int_t ii;
  //UChar_t hdr[DQ_MINHDR];
  UShort_t ww;
  fpos = ftell(fD);
  if (fread(&frec_length, sizeof(frec_length), 1, fD) !=1) return ERRDQ_IO;
  if(gDebug) printf("frec_length = %08x\n",frec_length);
  Int_t nitems = fread(fhdr, sizeof(fhdr), 1, fD);
  if(nitems!=1) return ERRDQ_IO;
  if(gDebug) {
    printf("fhdr @ 0x%lx:",fpos);
    for(ii=0;ii<(Int_t)sizeof(fhdr);ii++) printf("%02x ",fhdr[ii]);
    printf("\n");
  }//#endif
  if((fhdr[4] != 0xf0) || (fhdr[5] != 0xc2)) 
  {
    ferr |= ERRDQ_STAMP;
    if(ferrcount<MAXERRPRINT)printf("ERRDQ_STAMP %02x%02x != F0C2 @ %08lx\n",fhdr[4],fhdr[5],fpos);
    FillErr();
    return ferr;
  }
  // header
  fcrc = 0;
  UpdateCRC(&fcrc,(UShort_t*)fhdr,sizeof(fhdr)/2);
  if(fevhl == 0) 
  {
    // First event
    for(ii=0;ii<DQ_MINHDR;ii++) f1sthdr[ii]=fhdr[ii];
    //estimate event length
    fevhl = f1sthdr[11]&0xf;
    fevtl = ((f1sthdr[11])>>4)&0xf;
    //&RA150914/fevnasics = ((f1sthdr[3])>>4)&0xf;
    fevnasics = ASICS_IN_CHAIN;
    fevchains = f1sthdr[6]&0xf;
    ii = fevnasics*(CH_IN_ASIC+1);
    printf("header constructed hl=%i, tl=%i, vers=%02x, na=%i, chain_flag=0x%1x[%i], data are ",
           fevhl,fevtl,f1sthdr[10],fevnasics,fevchains,ii);
    if(DATA_ARE_ADC) printf("ADC\n"); else printf("Channel numbers\n");
    flchain[0] = (fevchains&1)?ii:0;
    flchain[1] = (fevchains&2)?ii:0;
    flchain[2] = (fevchains&4)?ii:0;
    flchain[3] = (fevchains&8)?ii:0;
    //if(ftree==NULL)
    TString ss = GetFileName();
    {
      TString hname;
      TString mname;
      TString htitle = fhtitle;
      htitle += ", run "; 
      ii = ss.Last('/')+1;
      ss = ss(ii, ss.Last('.')-ii);
      htitle += ss.Data();
      htitle += ", CMN="; htitle += gCMNControl;
      for(ii=0;ii<NCHAINS;ii++)
      {
        if(flchain[ii]==0) continue;
        hname = "hchain";
        hname +=ii;
        mname = "adchain";
        mname +=ii;
        printf("Creating %s[%i]\n",hname.Data(),flchain[ii]);
        fhchns[ii] = new TH2S(hname,htitle,flchain[ii],0,flchain[ii],256,0,256);
      }
    }
  }
  
  // The real header could be longer than the pre-defined one
  // skip the rest of the header
  for(ii=sizeof(fhdr)/2;ii<fevhl;ii++)
  {
    nitems = fread(&ww, sizeof(ww), 1, fD);
    if(gDebug) { printf("Skipping %04x\n",ww);}
    fcrc ^= ww;
  }
  return 0;
}
Int_t Tdq::FindEOE()
{
  UShort_t ww;
  Int_t ii,nn,rc=0;
  
  #define FINDEOE_RANGE 4000
  //TODO check for empty events here
  //fseek(fD,fpos+sizeof(fhdr),SEEK_SET); // roll back to past header, this is to correctly handle empty events
  
  //Handle possible padding
  #define FINDEOE_ROLLBACK 16
  fseek(fD,-(fevhl+FINDEOE_ROLLBACK)*2,SEEK_CUR);	// roll back 16 bytes to deal with possible unexpected padding
  for(ii=0;ii<FINDEOE_RANGE;ii++)
  {
    nn = fread(&ww, sizeof(ww), 1, fD);
    if(nn!=1) {
      printf("ERRDQ_IO nn = %d\n",nn);
      perror("fread");
      return ERRDQ_IO;
    }
    if(ww == DQ_EOE) break;
  }
  
  if(ii>=FINDEOE_RANGE) 
    rc |= ERRDQ_NOEOE;
  else
    nn = fread(&ww, sizeof(ww), 1, fD);//skip CRC to position for next event
    //fpos = ftell(fD);
    //printf("EOE not found. FindEOE=%i excess=%i @ %lx\n",rc,FINDEOE_ROLLBACK-ii,ftell(fD));
    printf("EOE not found. FindEOE=%i skipped %li bytes @ %lx\n",rc,ftell(fD)-fpos,ftell(fD));
  return rc;
}
void	Tdq::FillErr()
{
  Int_t ii,ib;
  {
    ferrcount++;
    if(ferrcount==MAXERRPRINT) printf("Too many errors, will not print anymore\n");
    for(ii=0,ib=1;ii<32;ii++)
    {
      if(ferr&ib) fherr->Fill(ii);
      ib = ib<<1;
    }
  }
}
Int_t Tdq::Pad(Int_t channel)
{
        return channel/(CH_IN_ASIC+1)*CH_IN_ASIC + channel%(CH_IN_ASIC+1);
}
UShort_t gPrevEvNum = 0;
Int_t Tdq::Process()
{
  Int_t ii,chain,nn;
  Int_t bytes_per_chip=NCHAINS*(CH_IN_ASIC+1);
  UShort_t body[MAXCH+64],nw;
  UChar_t *bbody = (UChar_t*)body;
  UChar_t byte;
  ULong_t cclk=0;
  UShort_t *phdr = (UShort_t *) &(fhdr[0]);
  
  ferr = 0;
  // get body of the event
  //&RA 130903/nw = fevtl + fevnasics*bytes_per_chip/2 +2;// the +2 could be an artifact in the hardware
  //nw = fevtl + fevnasics*bytes_per_chip/2 + 1 + gExtraWords;
  //&RA 150914/nw = fevtl + fevnasics*bytes_per_chip/2 + gExtraWords;
  nw = (frec_length - sizeof(fhdr))/sizeof(Short_t);
  if(gDebug) { printf("At %lx, Expected nw=%04x, hdr crc=0x%04x\n",ftell(fD),nw,fcrc);}
  nn = fread(body, sizeof(UShort_t), nw, fD);
  if(nn!=nw)
  {
    printf("READ ERROR in Process @ %lx\n",ftell(fD));
    return ERRDQ_IO;
  }
  // CRC
  //for(ii=0;ii<nw;ii++) fcrc ^= body[ii];
  UShort_t crch = fcrc,crcb=0,crct=0;
  UpdateCRC(&crcb,body,nw-fevtl);
  UpdateCRC(&crct,body+nw-fevtl,fevtl);
  fcrc ^= crcb ^ crct;
  if(gDebug) {
    printf("crc=%04x (h,b,t=%04x,%04x,%04x), top/end of svx[0]/bottom:\n",fcrc,crch,crcb,crct);
    for(ii=0;ii<16;ii++) printf("%04x ",body[ii]);
    printf("\n");
    for(ii=2*(CH_IN_ASIC+1-2);ii<2*(CH_IN_ASIC+1+6);ii++) printf("%04x ",body[ii]);
    printf("\n");    
    for(ii=nw-16;ii<nw;ii++) printf("%04x ",body[ii]);
    printf("\n");
  }
  #ifdef ERRDQ_CRC
  if(fcrc) {ferr |= ERRDQ_CRC;}
  if((ferr&ERRDQ_CRC)!=0)
  {
    if(ferrcount<MAXERRPRINT) printf("ERROR ERRDQ_CRC %04x @ %08lx, nerrs=%li\n",fcrc,fpos,ferrcount);
    #undef ERREXIT
    #ifdef ERREXIT
    FillErr();
    return ERRDQ_CRC;
    #endif
  }
  #endif // ERRDQ_CRC
  
  // event should be OK, fill the members
  fevnum = ntohs(phdr[0]);
  if(fentry<=0) gPrevEvNum = fevnum;
  fNSkippedEvents = fevnum - gPrevEvNum;
  if (fNSkippedEvents<0) fNSkippedEvents += 65536;
  gPrevEvNum = fevnum;
  fevsize = (fevhl + nw)*2;
  //fcelln = Bin2Gray(bbody[3]);
  //printf("ph4=%04x->%04x ph7=%04x->%04x\n",phdr[4],ntohs(phdr[4]),phdr[7],ntohs(phdr[7]));
  fbclk = ntohs(phdr[4]) + (((ntohs(phdr[7])>>4)&0xfff)<<16);
  fclkphase = fhdr[7]&0x7;
  cclk = fbclk;
  Long_t clkdiff;
  clkdiff=cclk-fclkprev;
  fclkprev = cclk;
  if(clkdiff<0) clkdiff += 16777216; // add 2^24 if overflow
  if(fentry>0) fclkdiff = clkdiff;
  fHPARTime = ntohs(phdr[6]);
  fHL1Stack = (ntohs(phdr[1])>>12)&0xf;
  fHDigTime = (ntohs(phdr[1]))&0xfff;
  fHFStatus = (ntohs(phdr[8])>>12)&0xf;
  fHPrevL1 = ntohs(phdr[9]);
  if(gDebug) {
    for(ii=0;ii<16;ii++) printf("%02x ",bbody[ii]);
    printf("\nbclk=%08lx, phase=%04x, pt=%04x, l1p=%04x, dt=%04x, hfs=%04x, pl1=%04x, diff=%li:, cc=%li\n",
           fbclk,fclkphase,fHPARTime,fHL1Stack,fHDigTime,fHFStatus,fHPrevL1,fclkdiff,cclk);
  }//#endif
  if(DATA_ARE_ADC)
  {
    // check for valid cell numbers
    // find first non-zero celln and assign it to fcelln
    for(ii=0;ii<NCHAINS;ii++) if(bbody[ii]) break;
    byte = Bin2Gray(bbody[ii]);
    for(ii=0;ii<NCHAINS;ii++) fcelln[ii][0] = byte;
    if(gDebug) { 
      printf("fcelln=%02x, celln0: %02x,%02x,%02x,%02x\n",
             byte,bbody[0],bbody[1],bbody[2],bbody[3]);
    }//#endif
    #ifdef ERRDQ_CELLN
    Int_t i1;
    for(i1=bytes_per_chip; i1<nw*2-bytes_per_chip; i1 += bytes_per_chip)
    {
      nn = i1/bytes_per_chip;// module number
      if(gDebug) {
        printf("celln%i:%02x,%02x,%02x,%02x\n",
               nn,bbody[i1+0],bbody[i1+1],bbody[i1+2],bbody[i1+3]);
      }//#endif
      for(ii=0;ii<NCHAINS;ii++)
      {
        chain = ii^1;
        if(bbody[i1+ii] == 0) continue;
        fcelln[chain][nn] = Bin2Gray(bbody[i1+ii]);
        if(fcelln[chain][nn]>47) 
        {
          ferr |= ERRDQ_CELLN; 
          if(ferrcount<MAXERRPRINT) 
            printf("ERR=%04lx. Cell[%i]# %i>47. @ %08lx, nerrs=%li\n",
                   ferr,chain,fcelln[chain][nn],fpos,ferrcount);
        }
        if(fcelln[chain][nn]!=fcelln[chain][0])
        {
          ferr |= ERRDQ_FCELLN_0<<ii; 
          if(ferrcount<MAXERRPRINT) 
            printf("ERR=%04lx in ev %04x, mod %04x. Cell[%i]# %02x!=%02x. @ %08lx\n",ferr,fevnum,nn,chain,fcelln[chain][nn],fcelln[chain][0],fpos);
        }
      }
    }
    #endif //ERRDQ_CELLN
  }
  
  Int_t channel =-999;
  //v12/for(ii=NCHAINS;ii<(nw-fevtl)*2;ii++)
  for(ii=0;ii<(nw-fevtl)*2;ii++)
  {
    //v12/ Why? Not needed for rev1 FEMs /channel = ii/NCHAINS-1; 
    channel = ii/NCHAINS;
    if(channel>=ASICS_IN_CHAIN*(CH_IN_ASIC+1))
    {
      if(fentry<2) {cout<<"Skipping channel "<<channel<<endl;	continue;}
    }
    if(NCHAINS == 1) chain = 0;
    else  chain = (ii%NCHAINS)^1; //Tested for v9E
    if(flchain[chain]==0 && fentry<2) 
    {//cout<<"chain "<<chain<<" empty "<<channel<<","<<ii<<endl;   
      continue;
    }
    int ichip = channel/(CH_IN_ASIC+1); 
    int ich = channel % (CH_IN_ASIC+1);
    if (gStripMapping) ich = PadNumber[ich];
    CHV_t chv = (CHV_t)bbody[ii];
    //if(gSubtractPeds) chv -= ped[chain][channel] + 50;
    if(gSubtractPeds) chv -= ped[chain][channel];
    if(gDebug && chain < 2) cout<<"c("<<chain<<","<<ichip*(CH_IN_ASIC+1) + ich<<")/a("<<ichip<<","<<ich<<")="<<chv<<"\t";
    fchv[chain][ichip*(CH_IN_ASIC+1) + ich] = chv;
  }
  if(gCMNControl && DATA_ARE_ADC) 
    for(ii=0;ii<NCHAINS;ii++) if(flchain[ii]) DoCMNoise(ii);
    
    //Do the hits
    int ip,ipc,ipcm;
    for (ip=0;ip<NPLANES;ip++)
    {
      chain = gplane[ip].chain;
      fplane_nhits[ip] = 0; fplane_hitv[ip]=-1; fplane_hitp[ip]=-1; ipcm = 0;
      for (ipc=gplane[ip].first_ch; ipc<=gplane[ip].last_ch; ipc++)
      {
        if(ipc>=flchain[chain]) break;
        if(fchv[chain][ipc] > gHitThreshold) fplane_nhits[ip] +=1;
        if(fchv[chain][ipc] > fplane_hitv[ip])
        {
          ipcm = ipc;
          fplane_hitp[ip] = ipc-gplane[ip].first_ch;
          fplane_hitv[ip] = fchv[chain][ipc];
        }
      }
      if(gDebug) cout<<"p"<<ip<<", nhits:"<<fplane_nhits[ip]<<", mainhit:"<<fplane_hitv[ip]<<" @ "<<fplane_hitp[ip]<<", nl:"<<fchv[chain][ipcm-1]<<", nr:"<<fchv[chain][ipcm+1]<<endl;
      //add neighbors
      if (fchv[chain][ipcm-1] > gHitThreshold) fplane_hitv[ip] += fchv[chain][ipcm-1];
      if (fchv[chain][ipcm+1] > gHitThreshold) fplane_hitv[ip] += fchv[chain][ipcm+1];
    }
    //fill histogram
    for(chain=0;chain<NCHAINS;chain++)
      if(fhchns[chain]) 
        for(channel=0;channel<flchain[chain];channel++) 
        {
          //note: start with channel=1 only if SWAP
          fhchns[chain]->Fill((Double_t) channel, (Double_t) fchv[chain][channel]);
        }
        if(gDebug) {printf("\n");}
        //fpos = ftell(fD);
        if(ftree) ftree->Fill();
        if(ferr)
        {
          fnerr++;
          FillErr();
        }
        // Check if trailer is correct
        if(body[nw-2] != DQ_EOE)
        {
          ferr |= ERRDQ_EVSIZE;
          printf("ERROR ERRDQ_EVSIZE %04x!=%04x @ %08lx\n",body[nw-2],DQ_EOE,ftell(fD));
          return ERRDQ_EVSIZE;
        }
        return 0;
}


Int_t Tdq::Next()
{
  Int_t rc;
  rc = GetHeader();
  if(rc<0) return rc;
  if(rc&ERRDQ_STAMP)
  {
    if((rc = FindEOE())!=0) return -rc;
    rc = GetHeader();
  }
  if (rc)	return rc;
  // event seems to be OK
  return Process();
}
void Tdq::DoCMNoise(Int_t chain)
{
  Int_t ic,nasics,ii;
  Int_t nErrsToPrint = 10;
  Long64_t size = CH_IN_ASIC;
  Long64_t order = Long64_t((Float_t)(CH_IN_ASIC)*gCMNQuantile);
  Long64_t work[CH_IN_ASIC];
  nasics = flchain[chain]/CH_IN_ASIC;
  //Int_t chnn;
  for(ic=0;ic<nasics;ic++)
  {
    fcmnoise[chain][ic] =  (TMath::KOrdStat(size, fchv[chain]+ic*CH_IN_ASIC, order, work));
    if(TMath::Abs(fcmnoise[chain][ic])>gCMNLimit)
    {
      if(fcmnoise[chain][ic]==-255) {ferr |= ERRDQ_0CHIP;}
      else
      {
        ferr |= ERRDQ_CMNOISE;
        if(dbg_cmnoise<nErrsToPrint)
        {
          dbg_cmnoise++;
          printf("***Error cmn noise %i>%i in chip %i.%i, ev%i, err#%i\n",
                 fcmnoise[chain][ic],gCMNLimit,chain,ic,fevnum, nErrsToPrint-dbg_cmnoise);
        }
      }
    }
    
    if(gCMNControl&2)
    {
      //Int_t chnn = 0;
      for(ii=0;ii<CH_IN_ASIC;ii++)
      {
        fchv[chain][ic*(CH_IN_ASIC+1) + ii] -= (CHV_t)(fcmnoise[chain][ic]);
      }
    }
  }
}
Bool_t Tdq::IsOpen()
{
        return fD != 0;
}
Tdq::Tdq(const Char_t *name, Int_t cmnproc_control, const Char_t *htitle)
{
    Int_t rc,ii;
    struct stat statv;
    const Char_t *tname;
    Char_t	oname[256];
    fhtitle = htitle;

    fentry = 0;
    fD = NULL;
    ffile = NULL;
    ftree = NULL;
    ferrcount = 0;
    fclkprev = 0;
    fevhl = 0;
    //fchnmap.Set(MAXCH);
    //fchnmap.Reset();
    if(strlen(name)==0)
      return;
    if((tname = gSystem->ExpandPathName(name))!=0)
    {
        strcpy(fname,tname);

                // Crashes Win32 as deleting unallocated memory?
                // That error doesn't seem right, but commenting this out allows things to continue
                // properly. I think ExpandPathName is using the string class, which is *managed*
                // under Win32 - JGL 10/19/2013
#ifndef WIN32
                delete [](char*)tname;
#endif

    }
    if(gDebug) {    printf("Opening %s\n",fname);}
    fD = fopen(fname,"rb");
    if(fD==NULL)
    {
        perror("Could not open file ");
        perror(fname);
        return;
    }
    rc = stat(fname,&statv);
    if(rc!=0) {perror("Cannot fstat"); fsize = -1;}
    fsize = statv.st_size;
    printf("File opened %s[%d]\n",fname,fsize);
    strcpy(oname,fname);
    char *substr = strrchr(oname,'.');
    strcpy(substr+1,"root");
    if(gDebug) {    printf("Output file %s\n",oname);}
    if(ffile) {printf("deleting file\n");delete ffile;}
    ffile = new TFile(oname,"recreate");
    if(ffile == NULL) {printf("ERROR. Could not open %s\n",oname); return;}	
    if(gDebug) {    printf("File opened\n");}
    fnchn = ASICS_IN_CHAIN*CH_IN_ASIC;
    for(ii=0;ii<fnchn;ii++) fchn[ii] = ii;
    fherr = new TH1I("herr","Format Errors",32,0,32);
    printf("Tdq is constructed\n");
    if( cmnproc_control >=0 ) gCMNControl = cmnproc_control;
    printf("Common mode noise subtraction is ");
    if(gCMNControl&2) printf("ON\n");
    else	printf("OFF\n");
    printf("Channels are: ");
    if(gStripMapping)	printf("Sensor strip numbers.\n");
    else		printf("ASIC channel numbers.\n");
    printf("To change mapppig change the global variable gdq->gStripMapping\n"); 
    printf("Number of extra words (gdq->gExtraWords)=%i\n",gExtraWords);
    return;
}
Tdq::~Tdq()
{
    if(gDebug) {
    printf("Deleting fD, tree and file\n");
    }//#endif
    if(fD) {fclose(fD); fD = NULL;}
    if(ftree) {delete ftree; ftree = NULL;}
    if(ffile) {delete ffile; ffile = NULL;}
}
TTree* Tdq::MakeTree(Int_t mode)
{
    Int_t ii;
    if(mode)
    {
        printf("Making Tree (%i)\n",mode);
        if(ftree) {printf("deleting tree\n");delete ftree;}
        ftree = new TTree("dqtree","dq analysis tree");
        if(ftree==NULL){printf("failed to create tree\n");return ftree;}
            //printf("Tree created\n");
        ftree->Branch("entry",&fentry,"fentry/i");
        ftree->Branch("fevnasics",&fevnasics,"fevnasics/b");
        ftree->Branch("celln0",fcelln[0],"fcelln0[fevnasics]/b");
        ftree->Branch("celln1",fcelln[1],"fcelln1[fevnasics]/b");
        ftree->Branch("celln2",fcelln[2],"fcelln2[fevnasics]/b");
        ftree->Branch("celln3",fcelln[3],"fcelln3[fevnasics]/b");
        ftree->Branch("evsize",(Int_t*)&fevsize,"evsize/s");
        ftree->Branch("error",(Int_t*)&ferr,"ferr/i");
        ftree->Branch("evnum",(Int_t*)&fevnum,"fevnum/i");
        ftree->Branch("fNSkippedEvents",&fNSkippedEvents,"fNSkippedEvents/i");
        ftree->Branch("fnchn",&fnchn,"fnchn/I");
        ftree->Branch("chn",fchn,"chn[fnchn]/s");
        ftree->Branch("f1chain0",&(flchain[0]),"flchain0/I");
        ftree->Branch("f1chain1",&(flchain[1]),"flchain1/I");
        ftree->Branch("f1chain2",&(flchain[2]),"flchain2/I");
        ftree->Branch("f1chain3",&(flchain[3]),"flchain3/I");
        ftree->Branch("chv0",fchv[0],"fchv0[flchain0]/S");
        ftree->Branch("chv1",fchv[1],"fchv1[flchain1]/S");
        ftree->Branch("chv2",fchv[2],"fchv2[flchain2]/S");
        ftree->Branch("chv3",fchv[3],"fchv3[flchain3]/S");
        ftree->Branch("cmnn0",fcmnoise[0],"fcmnn0[fevnasics]/s");
        ftree->Branch("cmnn1",fcmnoise[1],"fcmnn1[fevnasics]/s");
        ftree->Branch("cmnn2",fcmnoise[2],"fcmnn2[fevnasics]/s");
        ftree->Branch("cmnn3",fcmnoise[3],"fcmnn3[fevnasics]/s");
        ftree->Branch("clkphase",(Int_t*)&fclkphase,"fclkphase/b");
        ftree->Branch("clkdiff",&fclkdiff,"clkdiff/i");
        ftree->Branch("nerr",(Int_t*)&fnerr,"fnerr/s");
        ftree->Branch("bclk",&fbclk,"bclk/I");
        ftree->Branch("hPARTime",&fHPARTime,"hPARTime/s");
        ftree->Branch("hL1Stack",&fHL1Stack,"hL1Stack/s");
        ftree->Branch("hDigTime",&fHDigTime,"hDigTime/s");
        ftree->Branch("hFStatus",&fHFStatus,"hFStatus/s");
        ftree->Branch("hPrevL1",&fHPrevL1,"hPrevL1/s");
        // change 4 to real NPLANES below
        ftree->Branch("hitv",&fplane_hitv,"fplane_hitv[4]/S");
        ftree->Branch("hitp",&fplane_hitp,"fplane_hitp[4]/I");
        ftree->Branch("nhits",&fplane_nhits,"fplane_nhits[4]/I");
    }
    fevcount = 0;
    ferrcount = 0;
    //printf("Event loop\n");
    if(gDebug) {
      gMaxEntries = 20;
    }
    for(fentry=0;fentry<gMaxEntries;fentry++)
    {
      if(Next()<0) break;
      if((fentry%100)==0) printf("nev=%i,nerr=%li,err=%08lx\n",fentry,ferrcount,ferr);
    }
    printf("Break after %i events\n",fentry);
    if(fentry)
    {
      cout<<"writing tree"<<endl;
      if(ftree) ftree->Write();
      cout<<"writing hist"<<endl;
      if(fherr) fherr->Write();
      cout<<"loop"<<endl;
      for(ii=0;ii<NCHAINS;ii++)
      {
        cout<<"ii="<<ii<<endl;
        if(flchain[ii]<=0) break;
        cout<<"writing chain["<<ii<<"]["<<flchain[ii]<<endl;
        if(fhchns[ii]) fhchns[ii]->Write();
      }
    }
    return ftree;
}
