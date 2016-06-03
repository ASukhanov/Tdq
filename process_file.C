// Process data file from MPCEX FEM, captured by dqc.py program
// Version v2  2016-06-03. Replaced IsDec with IsDigit

#include "globals.h"

#define PROCESS_MODE 1
// PROCESS_MODE 1
// Generates tree, useful for detailed analysis
// PROCESS_MODE 0 
// Generates only summary histograms

//COMMON_MODE_NOSE_PROCESSING
//nonzero: process common mode moise, 
//2: subtract common mode noise
//#define COMMON_MODE_NOSE_PROCESSING -1

/*
//#include "n3c.h"
#define MAXCHIPS 2
#define MAXCH 128*MAXCHIPS
TCut goodFormat = "(error&0x00060004)==0"; //selection with excluded severe format errors
// for error types see Tn3c.cxx
TCut goodClkPhase = "(clkphase==7||clkphase==0)";//||clkphase==1)
//TCut goodEvents = goodFormat && goodClkPhase;
TCut goodEvents = goodFormat;
TCut goodADC = "-128<fadc&&fadc<255";
TCut goodChannels = "fchn<2048";
TCut firstChip = "fchn<128";
TCut secondChip = "128<=fchn&&fchn<256";
//TH2F hadc_chn("adc_chn","ADC vs Channel ID",MAXCH,0,MAXCH,256,0,256);
*/
TCanvas cnv_pf("cnv_pf","",0,10,1200,800);
TCanvas cnv_err("cnv_err","");
TString ghtitle("");
TString ss;

Char_t *tdq_run_name()
{
	ss = gdq->GetFileName();
	Int_t i2 = ss.Last('/') + 1;
	ss = ss(i2,ss.Last('.')-i2);
	return ss.Data();
}
Int_t process_file(TString txt, Int_t cmnproc=-1)https://root.cern.ch/doc/master/classTCanvas.html
{

	Int_t maxch=0,ii,in,chain;
	char txtline[256];
	//if(cmnproc==-1) cmnproc=COMMON_MODE_NOSE_PROCESSING; //default processing

	//cout<<txt<<"("<<txt.Length()<<")"<<endl;
	
	// Uncomment for interactive processing:
	//cout<<"Macro for processing n3c files"<<endl;
	//cout<<"dq n("<<txt<<") > ";//gets(txtline);
	
	if(txt=="same") if(gdq) txt = gdq->GetFileName();
	if(gdq)
	{
		cout<<"Re-constructing Tdq"<<endl;
		delete gdq;
	}
	gdq = new Tdq((char*)txt, cmnproc, ghtitle.Data());
	if(!gdq->IsOpen()) return;
	cout<<endl;
	//cout<<"CmnProc="<<(gdq->gCMNControl)<<endl;
	
    cout<<"Zero pedestals"<<endl;
    for(int ichain=0;ichain<NCHAINS;ichain++)
    {
	for(int ich=0;ich<MAXCH;ich++) gdq->SetPed(ichain,ich,0,1);
    }
	if(gdq->gSubtractPeds)
	{
	    cout<<"Processing pedestals"<<endl;
	    TString tl;
	    TString tok;
            TString pref = "peds_hchain";
            TString suff = ".txt";
            TString fn="";
            ifstream fin;
	    string line;
	    Int_t iy=-1,ix=-2,ii=0,jj=0;
            for (jj=0;jj<2;jj++)
            {
              ii = 0;
              fn = pref;
              fn += jj;
              fn += suff;
              fin.open(fn);
              if(fin.is_open()) cout<<"opened "<<fn<<endl;
              else {cout<<"ERROR opening "<<fn<<endl; exit(1);}
                
	      while(getline(fin, line))
	      {
		    ix=0;
		    tl = TString(line);
		    tl.Tokenize(tok,ix,"\t");
		    if(ix<0)continue;
		    tl.Tokenize(tok,ix,"\t");
		    //cout<<ii<<":"<<tok<<endl;
		    if(tok.IsDigit())    iy = tok.Atoi();
		    else break;
		    //if (ii<10) cout<<ii<<":"<<iy<<endl;
		    gdq->SetPed(jj,ii,iy,1);
		    ii++;
	      }
              fin.close();
              if(ii==0)
              {
                cout<<"ERROR. No pedestals from file "<<fn<<endl;
                exit(1);
              }
	      cout<<"Set "<<ii<<" pedestals from "<<fn<<" to chain"<<jj<<endl;
            }
	}
	gdq->SetCMNoiseQuantile(0.25); //0. -disables common mode noise processing, default = .25
	gdq->SetCMNoiseLimit(50);
	//cout<<"TTree *t=gdq->MakeTree(); > ";//gets(txtline);
	//cout<<endl;
	gTree = gdq->MakeTree(PROCESS_MODE);
	
	for(in=0,ii=0;in<NCHAINS;in++) 
		if((maxch = gdq->GetLChain(in))!=0) 
			{ii++; cout<<"Found active chain "<<in<<"["<<maxch<<"]"<<endl; chain=in;}
	cout<<"File processed, "<<ii<<" active chains found"<<endl;
	if(ii)
	{
		//cout<<"You can draw the chain using: gdq->GetChain(chain)->Draw(\"colz\") and for hits only, use GetHits(chain)"<<endl;
                cout<<"To draw summary plots: .x draw_summary.C"<<endl;
                if(gRunInProgress) cout<<"Waiting for next file..."<<endl; //gRunInProgress is defined in Init.C
		cnv_pf.cd();
		cnv_pf.SetTitle(tdq_run_name());
		gdq->GetChain(chain)->Draw("colz");
		cnv_pf.SetLogz();
		cnv_pf.Update();
	}
	cnv_err.cd();
	gdq->fherr->SetFillColor(45);
	gdq->fherr->Draw();
	cnv_err.Update();
	cnv_pf.cd();
}
void tdq_write_image(Char_t *dir="/home/andrey/work/ncc/sensor_test/")
{
	TString fn = dir + ghtitle + "_" + tdq_run_name() + ".png";
	TImage *img = TImage::Create();
	img->FromPad(&cnv_pf);
	//img->Scale(img->GetWidth()/2,img->GetHeight()/2);
	img->SetImageCompression(100);
	img->WriteImage(fn);
	cout<<"Image "<<fn<<" saved."<<endl;
}
void tdq_rm_file()
{
	TString str = ".!rm ";
	str += gdq->GetFileName();
	cout << "Executing " << str << endl;
	gROOT->ProcessLine(str.Data());
}
void tdq_evsize(Int_t number_of_modules_in_chain=1)
{
	cout<<"Copy&paste the following dgbox command to change event size for "<<number_of_modules_in_chain<<" modules in chain"<<endl;
	cout<<".set daq_evsize "<<(26+516*number_of_modules_in_chain)<<endl;
}
void newest_file()
{
	FILE  *pipe = gSystem->OpenPipe("ls -t ~/data/*.dq4 | head -1","r");
	TString d;
	d.Gets(pipe);
	cout<<d;
	gSystem->ClosePipe(pipe);
	process_file(d);
}
