//TH2S *
Int_t subtract_pedestals(TH2 *hh)
{
	Int_t iy=-1,ix=-2,ii;
	ifstream fin("pedestals.txt");
	TString tl;
	TString tok;
	string line;
	
    //fin.open("pedestals.txt");
	for(ii=0;ii<10;ii++)
	{
		ix=0;
		getline(fin,line);
		tl = TString(line);
		//cout<<tl<<endl;
		tl.Tokenize(tok,ix,"\t");
		if(ix<0)continue;
		//cout<<ii<<",0,"<<ix<<":"<<tok<<endl;
		tl.Tokenize(tok,ix,"\t");
		cout<<tok<<endl;
		//fin>>ix>>iy;
		//cout<<ix<<","<<iy<<endl;
		//if(!fin.good())	break;
	}
    //if (!fin || fin.bad()) {cout<<"ERR opening input pedestals.txt\n";return -1;}
	//Int_t nbx = hh->GetNbinsX();
	//Int_t nby = hh->GetNbinsY();

	//TArrayS ped(nbx);
	//for (ix=0;ix<nbx;ix++) {fin>>iy>>ped[ix]; cout<<ped[ix]<<" ";} cout<<endl;

	//while(!(fin.getline(line,100))==0) cout<<line<<"/n";
	/*
	TArrayS arry(nby);
	for(ix=1;ix<nbx,ix++)
	{
		
		for(iy=1;iy<nby;iy++) arry[iy] = hh->GetBinContent(ix,iy);
		 
	}
	*/
}
