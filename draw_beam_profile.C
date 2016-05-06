{
  TH2S *h2 = new TH2S("h2","Beam Profile",127,0,127,127,0,127);

  // cut distorted amplitudes when trigger comes n=latency clocks after preamp reset.
  // for latency=5 the hPARTime in [7:9] should be cut out
  TCut cutPT = "hPARTime<7 || hPARTime>9";
  
  // Noisy channels cut
  TCut cutDead = "hitp[0]!=63 && hitp[0]!=64 && hitp[1]!=0 && hitp[1]!=97";

  // MIP cut
  TCut cutHitThr = "hitv[0]>15 && hitv[1]>15";

  //final cut
  TCut cut = cutPT && cutDead && cutHitThr;

  // Draw the profile:
  h2->SetMaximum(20); // adjust it as you wish
  gPad->SetLogz(0);
  gTree->Draw("hitp[0]:hitp[1]>>h2",cut,"colz");
}