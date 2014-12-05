TCanvas ccelln;
//ccelln,SetLogy();
ccelln.Divide(4,3);

void view_cell_numbers(Int_t chain=0)
{
	TCanvas ccelln;
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
		txt += ii*129;
		txt += "]";
		gTree->Draw(txt);
	}
}
