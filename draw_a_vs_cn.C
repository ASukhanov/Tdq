{
TCanvas cc; cc.Divide(2,1);
cc.cd(1); gPad->SetLogz(); gdq->GetChain(0)->Draw("colz");
cc.cd(2); gPad->SetLogz(); gdq->GetChain(1)->Draw("colz");
}
