{
TH2S *h2 = new TH2S("h2",hTitle,256,0,256,256,0,256);
gPad->SetLogz();
gTree->Draw("chv0:chn>>h2",cut,"colz");
}
