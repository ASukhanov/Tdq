// Shows amplitudes for selected phases phases
//
{
//TCut cutPhase	= "fclkphase==0 || fclkphase==1 || fclkphase==7";
TCut cutPhase	= "";
TCut cutV 	= "0<chn&&chn<256&&0<chv0&&chv0<255";
//TCut cutV 	= "";

TString ss = gdq->GetFileName();
Int_t i2 = ss.Last('/') + 1;
ss = ss(i2,ss.Last('.')-i2);
ss = "Run "+ss+". ADC vs Channel for selected phases";
Int_t hmax = gTree->GetEntries()/100;
TCanvas *c2 = new TCanvas("c2",ss,0,10,1600,600);
Int_t ch_first=0,ch_last=256;
hh = new TH2F("a_c",ss,ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cutV&&cutPhase,"colz");
}
