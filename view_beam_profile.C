// Shows amplitudes for selected phases phases
//
{
TCut cutPhase	= "fclkphase==0 || fclkphase==1 || fclkphase==7";
TCut cutV 	= "0<chn&&chn<128&&0<chv0&&chv0<255";

TString txt = gTree->GetName();
Int_t hmax = gTree->GetEntries()/100;
TCanvas *c2 = new TCanvas("c2",txt,0,10,1600,600);
Int_t ch_first=0,ch_last=128;
hh = new TH2F("a_c","ADC vs Channel selected phases",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cutV&&cutPhase,"colz");
}
