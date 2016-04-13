//process_latest_file()
//version 1 2015-12-28
{

// Execute init.C script
gROOT->ProcessLine(".x init.C");

// get the name of the last file
gSystem->Exec("ls -t /tmp/*.dq4 | head -1 > lastFile.txt");
FILE *fp = fopen("lastFile.txt","r");
char lastFile[81];
fgets(&lastFile,80,fp);
fclose(fp);
lastFile[strlen(lastFile)-1]=0; //strip 
//cout<<"("<<lastFile<<")"<<endl;
process_file(lastFile);
}
