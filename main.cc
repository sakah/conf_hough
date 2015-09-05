#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "TCanvas.h"
#include "TH2F.h"
#include "TMarker.h"
#include "InputROOT.h"

TMarker* getMarker(int iturn, double x, double y)
{
   TMarker *m1 = new TMarker(x, y, 8);
   m1->SetMarkerSize(0.3);
   m1->SetMarkerColor(1);

   if (iturn==0) m1->SetMarkerColor(2);
   if (iturn==1) m1->SetMarkerColor(3);
   if (iturn==2) m1->SetMarkerColor(4);
   if (iturn==3) m1->SetMarkerColor(5);
   return m1;
}
int main(int argc, char** argv)
{
   if (argc != 8) {
      fprintf(stderr,"Usage %s <input.root> <wire_config.txt> <sample_type> <t2r_type> <rdrift_err_cm> <posSmear_cm> <momSmear_percent>\n", argv[0]);
      return -1;
   }
   char* input_root = argv[1];
   char* wire_config_txt = argv[2];
   char* sample_type = argv[3];
   char* t2r_type = argv[4];
   double rdrift_err_cm = atof(argv[5]);
   double posSmear_cm = atof(argv[6]);
   double momSmear_percent = atof(argv[7]);

   InputROOT inROOT(input_root, wire_config_txt, sample_type, t2r_type, rdrift_err_cm, posSmear_cm, momSmear_percent);
   //int total = inROOT.getEntries();
   int total = 100;

   double w_x1;
   double w_y1;
   double w_z1;
   double w_x2;
   double w_y2;
   double w_z2;
   double w_x;
   double w_y;
   double w_z;

   TVector3 mcPos;
   TVector3 mcMom;

   TCanvas* c1 = new TCanvas("c1","", 500*3, 500*2);
   c1->Divide(3,2);

   TH2F* h1 = new TH2F("h1", "", 100, -100, 100, 100, -100, 100);
   TH2F* h2 = new TH2F("h2", "", 100, -100, 100, 100, -100, 100);
   TH2F* h3 = new TH2F("h3", "", 100, -100, 100, 100, -100, 100);
   TH2F* h4 = new TH2F("h4", "", 100, 0, 200, 100, -150, 150);
   TH2F* h5 = new TH2F("h5", "", 100, 0, 200, 100, -150, 150); // MeV
   h1->SetStats(0);
   h2->SetStats(0);
   h3->SetStats(0);
   h4->SetStats(0);
   h5->SetStats(0);

   for (int iev=0; iev<total; iev++) {

      h1->SetTitle(Form("iev %d - MC XY", iev)); c1->cd(1); h1->Draw();
      h2->SetTitle(Form("iev %d - Wire XY @ endplate", iev)); c1->cd(2); h2->Draw();
      h3->SetTitle(Form("iev %d - Wire XY @ hitZ", iev)); c1->cd(3); h3->Draw();
      h4->SetTitle(Form("iev %d - MC Z vs ihit", iev)); c1->cd(4); h4->Draw();
      h5->SetTitle(Form("iev %d - MC PZ vs ihit", iev)); c1->cd(5); h5->Draw();

      inROOT.getEntry(iev);
      bool directHit = inROOT.InDirectHitAtTriggerCounter();
      if (!directHit) continue;

      int numHits = inROOT.getNumHits();
      if (numHits==0) continue;

      printf("iev %d numHits %d\n", iev, numHits );
      for (int ihit=0; ihit<numHits; ihit++) {
         int iturn = inROOT.getIturn(ihit);

         inROOT.getPosMom(ihit, mcPos, mcMom);
         inROOT.getWirePosAtEndPlates(ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         inROOT.getWirePosAtHitPoint(ihit, w_x, w_y, w_z);

         TMarker* m1 = getMarker(iturn, mcPos.X(), mcPos.Y());
         TMarker* m2 = getMarker(iturn, w_x1, w_y1);
         TMarker* m3 = getMarker(iturn, w_x, w_y);
         TMarker* m4 = getMarker(iturn, ihit, mcPos.Z());
         TMarker* m5 = getMarker(iturn, ihit, mcMom.Z()*1000); // GeV -> MeV

         c1->cd(1); m1->Draw();
         c1->cd(2); m2->Draw();
         c1->cd(3); m3->Draw();
         c1->cd(4); m4->Draw();
         c1->cd(5); m5->Draw();

         printf("iev %d MC:     ihit %d (%f, %f, %f)\n", iev, ihit, mcPos.X(), mcPos.Y(), mcPos.Z());
         printf("iev %d End:    ihit %d (%f, %f, %f) - (%f, %f, %f)\n", iev, ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         printf("iev %d MCWire: ihit %d (%f, %f, %f)\n", iev, ihit, w_x, w_y);
      }
      c1->Print(Form("pdf/%05d.pdf", iev));
   }
   return 0;
}
