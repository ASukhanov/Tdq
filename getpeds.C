Int_t getpeds()
{
	Int_t ii,base=50;
	Int_t vv;
	ofstream fout;
	fout.open("chnmapx.txt");
	if (!fout || fout.bad()) return -1;
	TProfile *prfx = gh->ProfileX("prfx");
	if(prfx==NULL) return -2;
	Int_t nbins = prfx->GetNbinsX();
	for(ii=0;ii<nbins;ii++)
	{
		vv = prfx->GetBinContent(ii+1);
		fout<<ii<<"\t"<<vv<<endl;
	}
	fout.close();
}
