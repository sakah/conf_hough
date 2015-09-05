#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "TCanvas.h"
#include "TH2F.h"
#include "TMarker.h"
#include "InputROOT.h"

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

   double mc_px;
   double mc_py;
   double mc_pz;
   TCanvas* c1 = new TCanvas("c1");
   TH2F* h2 = new TH2F("h2", "", 100, -100, 100, 100, -100, 100);
   for (int iev=0; iev<total; iev++) {
      h2->SetTitle(Form("iev %d", iev));
      h2->Draw();
      inROOT.getEntry(iev);
      bool directHit = inROOT.InDirectHitAtTriggerCounter();
      if (!directHit) continue;
      int numHits = inROOT.getNumHits();
      for (int ihit=0; ihit<numHits; ihit++) {
         inROOT.getWirePosAtEndPlates(ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         TMarker *m1 = new TMarker(w_x1, w_y1, 20);
         m1->Draw();
         printf("iev %d ihit %d (%f, %f, %f) - (%f, %f, %f)\n", iev, ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
      }
      c1->Print(Form("pdf/%05d.pdf", iev));
   }
   return 0;
}
