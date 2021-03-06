//Check the file daqcapture.dq0 for changed contents and ii it chaged then
//invoke the process_file.C script.
//
// Note, the process_file.C script should be pre-loaded, naturally in init.C:
// gROOT->ProcessLine(".L process_file.C");

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
TString dataDir = "D:/data/";

void check_for_file(TString &prev_line="", TString txt = "")
{
	char txtline[256];

	if(txt.Length()==0) txt = dataDir + "daqcapture.dq0";
	ifstream captured_file(txt);
	captured_file.getline(txtline,sizeof(txtline));
	if(strlen(txtline)==0)
	{	
		cout<<"Error reading "<<txt<<endl;
		return;
	}
	//cout<<txtline<<"\n";
	if( !prev_line.CompareTo(txtline) ) return;
	cout<<"File changed from "<<prev_line<<" to "<<txtline<<endl;
	prev_line = txtline;
	txt = dataDir;
	txt += txtline;
	cout<<"File to process: "<<txt<<endl;
	process_file(txt,-1);
}
