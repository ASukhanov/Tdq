Int_t get_pedestals(TH2 *hh)
{
        Int_t ii,base=50;
        Int_t vv;
        ofstream fout;
        fout.open("pedestals_calc.txt");
        if (!fout || fout.bad()) return -1;
		fout<<"#Pedestals calculated from file "<<hh->GetTitle()<<endl;
        TProfile *prfx = hh->ProfileX("prfx");
        if(prfx==NULL) return -2;
        Int_t nbins = prfx->GetNbinsX();
        for(ii=0;ii<nbins;ii++)
        {
                vv = prfx->GetBinContent(ii+1);
                fout<<ii<<"\t"<<vv<<endl;
        }
	cout<<"Pedestals written to pedestals_calc.txt"<<endl;
        fout.close();
}

