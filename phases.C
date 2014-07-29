// Shows amplitudes for different clock phases
//
{
TString txt = gTree->GetName();
Int_t hmax = gTree->GetEntries()/100;
TCanvas *c2 = new TCanvas("c2",txt,0,10,1600,600);
Int_t ch_first=0,ch_last=128;
TCut cut = "0<chn&&chn<128&&0<chv0&&chv0<255";
c2->Divide(4,2);
c2->cd(1);
hh = new TH2F("a_c","ADC vs Channel phase 0",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==0","colz");
//hh->SetMaximum(hmax);
c2->cd(2);
hh = new TH2F("a_c","ADC vs Channel phase 1",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==1","colz");
//hh->SetMaximum(hmax);
c2->cd(3);
hh = new TH2F("a_c","ADC vs Channel phase 2",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==2","colz");
//hh->SetMaximum(hmax);
c2->cd(4);
hh = new TH2F("a_c","ADC vs Channel phase 3",ch_last-ch_first,ch_first,ch_last,256,0,256);

gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==3","colz");
//hh->SetMaximum(hmax);
c2->cd(5);
hh = new TH2F("a_c","ADC vs Channel phase 4",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==4","colz");
//hh->SetMaximum(hmax);
c2->cd(6);
hh = new TH2F("a_c","ADC vs Channel phase 5",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==5","colz");
//hh->SetMaximum(hmax);
c2->cd(7);
hh = new TH2F("a_c","ADC vs Channel phase 6",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==6","colz");
//hh->SetMaximum(hmax);
c2->cd(8);
hh = new TH2F("a_c","ADC vs Channel phase 7",ch_last-ch_first,ch_first,ch_last,256,0,256);
gTree->Draw("chv0:chn>>a_c",cut&&"fclkphase==7","colz");
//hh->SetMaximum(hmax);
}
