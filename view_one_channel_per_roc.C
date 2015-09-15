TCanvas ccelln;
//ccelln,SetLogy();
ccelln.Divide(4,3);

void view_one_channel_per_roc(Int_t chain=0, Int_t roc_channel)
{
	TCanvas ccelln("c2","t",1600,900);
	ccelln.Divide(4,3);
	TString item = "chv";
	item += chain;
	item += "[";
	TString txt;
	int ii = 2;
	for (ii=0;ii<12;ii++)
	{
		ccelln.cd(ii+1);
	        ccelln.cd(ii+1)->SetLogy();
		txt = item;
		txt += ii*129+roc_channel;
		txt += "]";
		gTree->Draw(txt);
	}
}
