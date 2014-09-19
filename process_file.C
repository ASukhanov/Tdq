#include "globals.h"

#define PROCESS_MODE 1
// PROCESS_MODE 1
// Generates tree, useful for detailed analysis
// PROCESS_MODE 0 
// Generates only summary histograms

	//COMMON_MODE_NOSE_PROCESSING
	//nonzero: process common mode moise, 
	//&2: subtract common mode noise
//&RA/120314/#define COMMON_MODE_NOSE_PROCESSING 1
#define COMMON_MODE_NOSE_PROCESSING 0

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
TCanvas cnv_pf("cnv_pf","",0,10,1500,800);
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
Int_t process_file(TString txt, Int_t cmnproc=0)
{

	Int_t maxch=0,ii,in,chain;
	char txtline[256];
	if(cmnproc==-1) cmnproc=COMMON_MODE_NOSE_PROCESSING; //default processing

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
	
#ifdef PEDESTAL_PROCESSING
//Set pedestal value for each channel in each plane
		cout<<"Processing pedestals"<<endl;
       		ifstream pedin("pedestals.txt");
   		string line2;

   		 while (getline(pedin, line2)) {
       		        istringstream tokenizer(line2);
        		string token2;

        		getline(tokenizer, token2, ',');
        		istringstream int_iss(token2);
        		Int_t chain;
        		int_iss >> chain;
        		//cout<<"chain: 	"<<chain<<endl;

        		getline(tokenizer, token2, ',');
        		istringstream int_isss(token2);
        		Int_t channel;
        		int_isss >> channel;
        		//cout<<"channel: "<<channel<<endl;

			getline(tokenizer, token2, ',');
        		istringstream float_issss(token2);
        		Float_t pedestal;
        		float_issss >> pedestal;
        		//cout<<"pedestal: "<<pedestal<<endl;
				
			getline(tokenizer, token2, ',');
        		istringstream float_issss(token2);
        		Float_t pedrms;
        		float_issss >> pedrms;
        		//cout<<"rms: "<<pedrms<<endl;

			getline(tokenizer, token2, ',');
        		istringstream int_isssss(token2);
        		Int_t status;
        		int_isssss >> status;
        		//cout<<"status: "<<status<<endl;

			gdq->SetPed(chain,channel,pedestal,status);
				       	  }
#else
    cout<<"Zero pedestals"<<endl;
    for(int ichain=0;ichain<NCHAINS;ichain++)
    {
	for(int ich=0;ich<MAXCH;ich++) gdq->SetPed(ichain,ich,0,1);
    }
#endif//	PEDPROC
	if(gdq->gSubtractPeds)
	{
	    cout<<"Processing pedestals"<<endl;
	    ifstream fin("pedestals.txt");
	    TString tl;
	    TString tok;
	    string line;
	    Int_t iy=-1,ix=-2,ii=0;
	    while(getline(fin, line))
	    {
		    ix=0;
		    tl = TString(line);
		    tl.Tokenize(tok,ix,"\t");
		    if(ix<0)continue;
		    tl.Tokenize(tok,ix,"\t");
		    //cout<<ii<<":"<<tok<<endl;
		    if(tok.IsDec())    iy = tok.Atoi();
		    else break;
		    //if (ii<10) cout<<ii<<":"<<iy<<endl;
		    gdq->SetPed(0,ii,iy,1);	// 2014-07-31. only first chain is processed.
		    ii++;
	    }
	    cout<<"Set "<<ii<<" pedestals from pedestals.txt to chain0"<<endl;
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
		cout<<"You can draw the chain using: gdq->GetChain(chain)->Draw(\"colz\") and for hits only, use GetHits(chain)"<<endl;
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
