#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "TCanvas.h"
#include "TH2F.h"
#include "TMarker.h"
#include "InputROOT.h"

TMarker* getMarker(int ilayer, int iturn, double x, double y)
{
   TMarker *m1 = new TMarker(x, y, 8);
   m1->SetMarkerSize(0.3);
   m1->SetMarkerColor(1);

   if (ilayer%2==1) m1->SetMarkerColor(1);
   if (ilayer%2==0) m1->SetMarkerColor(2);

   if (iturn==0) m1->SetMarkerStyle(8); // ●
   if (iturn==1) m1->SetMarkerStyle(5); // x
   if (iturn==2) m1->SetMarkerStyle(22); // ▲
   if (iturn==3) m1->SetMarkerStyle(26); // △

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

   TCanvas* c1 = new TCanvas("c1","", 500*6, 500*3);
   c1->Divide(6,3);

   TH2F* h11 = new TH2F("h11", "", 100, -100, 100, 100, -100, 100);
   TH2F* h21 = new TH2F("h21", "", 100, -100, 100, 100, -100, 100);
   TH2F* h31 = new TH2F("h31", "", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   TH2F* h41 = new TH2F("h41", "", 100, -100, 100, 100, -100, 100);
   TH2F* h51 = new TH2F("h51", "", 100, 0, 200, 100, -150, 150);
   TH2F* h61 = new TH2F("h61", "", 100, 0, 200, 100, -150, 150); // MeV

   TH2F* h12 = new TH2F("h12", "", 100, -100, 100, 100, -100, 100);
   TH2F* h22 = new TH2F("h22", "", 100, -100, 100, 100, -100, 100);
   TH2F* h32 = new TH2F("h32", "", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   TH2F* h42 = new TH2F("h42", "", 100, -100, 100, 100, -100, 100);
   TH2F* h52 = new TH2F("h52", "", 100, 0, 200, 100, -150, 150);
   TH2F* h62 = new TH2F("h62", "", 100, 0, 200, 100, -150, 150); // MeV

   TH2F* h13 = new TH2F("h13", "", 100, -100, 100, 100, -100, 100);
   TH2F* h23 = new TH2F("h23", "", 100, -100, 100, 100, -100, 100);
   TH2F* h33 = new TH2F("h33", "", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   TH2F* h43 = new TH2F("h43", "", 100, -100, 100, 100, -100, 100);
   TH2F* h53 = new TH2F("h53", "", 100, 0, 200, 100, -150, 150);
   TH2F* h63 = new TH2F("h63", "", 100, 0, 200, 100, -150, 150); // MeV

   h11->SetStats(0);
   h21->SetStats(0);
   h31->SetStats(0);
   h41->SetStats(0);
   h51->SetStats(0);
   h61->SetStats(0);
   h12->SetStats(0);
   h22->SetStats(0);
   h32->SetStats(0);
   h42->SetStats(0);
   h52->SetStats(0);
   h62->SetStats(0);
   h13->SetStats(0);
   h23->SetStats(0);
   h33->SetStats(0);
   h43->SetStats(0);
   h53->SetStats(0);
   h63->SetStats(0);

   for (int iev=0; iev<total; iev++) {

      h11->SetTitle(Form("iev %d - ODD - MC XY", iev));
      h21->SetTitle(Form("iev %d - ODD - Wire XY @ endplate", iev));
      h31->SetTitle(Form("iev %d - ODD - Conformal XY @ endplate", iev));
      h41->SetTitle(Form("iev %d - ODD - Wire XY @ hitZ", iev));
      h51->SetTitle(Form("iev %d - ODD - MC Z vs ihit", iev));
      h61->SetTitle(Form("iev %d - ODD - MC PZ vs ihit", iev));
      h12->SetTitle(Form("iev %d - EVEN - MC XY", iev));
      h22->SetTitle(Form("iev %d - EVEN - Wire XY @ endplate", iev));
      h32->SetTitle(Form("iev %d - EVEN - Conformal XY @ endplate", iev));
      h42->SetTitle(Form("iev %d - EVEN - Wire XY @ hitZ", iev));
      h52->SetTitle(Form("iev %d - EVEN - MC Z vs ihit", iev));
      h62->SetTitle(Form("iev %d - EVEN - MC PZ vs ihit", iev));

      h13->SetTitle(Form("iev %d - ALL - MC XY", iev));
      h23->SetTitle(Form("iev %d - ALL - Wire XY @ endplate", iev));
      h33->SetTitle(Form("iev %d - ALL - Conformal XY @ endplate", iev));
      h43->SetTitle(Form("iev %d - ALL - Wire XY @ hitZ", iev));
      h53->SetTitle(Form("iev %d - ALL - MC Z vs ihit", iev));
      h63->SetTitle(Form("iev %d - ALL - MC PZ vs ihit", iev));

      c1->cd(1); h11->Draw();
      c1->cd(2); h21->Draw();
      c1->cd(3); h31->Draw();
      c1->cd(4); h41->Draw();
      c1->cd(5); h51->Draw();
      c1->cd(6); h61->Draw();
      c1->cd(7); h12->Draw();
      c1->cd(8); h22->Draw();
      c1->cd(9); h32->Draw();
      c1->cd(10);h42->Draw();
      c1->cd(11);h52->Draw();
      c1->cd(12);h62->Draw();
      c1->cd(13);h13->Draw();
      c1->cd(14);h23->Draw();
      c1->cd(15);h33->Draw();
      c1->cd(16);h43->Draw();
      c1->cd(17);h53->Draw();
      c1->cd(18);h63->Draw();

      inROOT.getEntry(iev);
      bool directHit = inROOT.InDirectHitAtTriggerCounter();
      if (!directHit) continue;

      int numHits = inROOT.getNumHits();
      if (numHits==0) continue;

      printf("iev %d numHits %d\n", iev, numHits );
      for (int ihit=0; ihit<numHits; ihit++) {
         int ilayer = inROOT.getIlayer(ihit);
         int iturn = inROOT.getIturn(ihit);

         inROOT.getPosMom(ihit, mcPos, mcMom);
         inROOT.getWirePosAtEndPlates(ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         inROOT.getWirePosAtHitPoint(ihit, w_x, w_y, w_z);

         double r2 = w_x1*w_x1 + w_y1*w_y1;
         double w_u1 = 2.0*w_x1/r2;
         double w_v1 = 2.0*w_y1/r2;
         TMarker* m1 = getMarker(ilayer, iturn, mcPos.X(), mcPos.Y());
         TMarker* m2 = getMarker(ilayer, iturn, w_x1, w_y1);
         TMarker* m3 = getMarker(ilayer, iturn, w_u1, w_v1);
         TMarker* m4 = getMarker(ilayer, iturn, w_x, w_y);
         TMarker* m5 = getMarker(ilayer, iturn, ihit, mcPos.Z());
         TMarker* m6 = getMarker(ilayer, iturn, ihit, mcMom.Z()*1000); // GeV -> MeV

         int offset = 0;
         if (ilayer%2==0) offset = 6;
         c1->cd(offset+1); m1->Draw();
         c1->cd(offset+2); m2->Draw();
         c1->cd(offset+3); m3->Draw();
         c1->cd(offset+4); m4->Draw();
         c1->cd(offset+5); m5->Draw();
         c1->cd(offset+6); m6->Draw();

         c1->cd(12+1); m1->Draw();
         c1->cd(12+2); m2->Draw();
         c1->cd(12+3); m3->Draw();
         c1->cd(12+4); m4->Draw();
         c1->cd(12+5); m5->Draw();
         c1->cd(12+6); m6->Draw();

         printf("iev %d MC:     ihit %d (%f, %f, %f)\n", iev, ihit, mcPos.X(), mcPos.Y(), mcPos.Z());
         printf("iev %d Wire End:    ihit %d (%f, %f, %f) - (%f, %f, %f)\n", iev, ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         printf("iev %d Conf End:    ihit %d (%g, %g)\n", iev, ihit, w_u1, w_v1);
         printf("iev %d MCWire: ihit %d (%f, %f, %f)\n", iev, ihit, w_x, w_y);
      }
      c1->Print(Form("pdf/%05d.pdf", iev));
   }
   return 0;
}
