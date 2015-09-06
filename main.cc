#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "TCanvas.h"
#include "TH2F.h"
#include "TMarker.h"
#include "InputROOT.h"

struct MyHits
{
   int num_hits;
   int ilayer[10000];
   int icell[10000];
   int iturn[10000];
   double xhits[10000];
   double yhits[10000];
   double zhits[10000];
   double z1;
   double z2;

   // after conformal mapping
   double uhits[10000];
   double vhits[10000];
   MyHits()
   {
      num_hits = 0;
   };
   void clear()
   {
      num_hits = 0;
   };
   void add_hit(int _ilayer, int _icell, int _iturn)
   {
      ilayer[num_hits] = _ilayer;
      icell[num_hits] = _icell;
      iturn[num_hits] = _iturn;
      num_hits++;
   };
   void calc_xyz(struct config* config, double _z1, double _z2)
   {
      z1 = _z1;
      z2 = _z2;
      double dz = (z2 - z1)/num_hits;
      for (int ihit=0; ihit<num_hits; ihit++) {
         zhits[ihit] = z1 + ihit* dz;
         config_get_wire_pos(config, ilayer[ihit], LAYER_TYPE_SENSE, icell[ihit], WIRE_TYPE_SENSE, zhits[ihit], "center", &xhits[ihit], &yhits[ihit]);
         double r2 = xhits[ihit]*xhits[ihit] + yhits[ihit]*yhits[ihit];
         uhits[ihit] = 2.0*xhits[ihit]/r2;
         vhits[ihit] = 2.0*yhits[ihit]/r2;
      }
   };
   void print()
   {
      for (int ihit=0; ihit<num_hits; ihit++) {
         printf("ihit %d x %f y %f z %f u %f v %f\n", ihit, xhits[ihit], yhits[ihit], zhits[ihit], uhits[ihit], vhits[ihit]);
      }
   };
};

struct Hough
{
   double found_a;
   double found_b;
   int num_hits;
   int num_inside;
   void transform(MyHits& hits)
   {
      double astep = 0.1;
      double bstep = 0.1;
      double amin = 0;
      double bmin = 0;
      double amax = 10;
      double bmax = 10;
      int anum = (amax-amin)/astep;
      int bnum = (bmax-bmin)/bstep;
      printf("anum %d\n", anum);
      TH2F h2("h2","",anum, amin, amax, bnum, bmin, bmax);

      for (int i=0; i<hits.num_hits; i++) {
         for (int ia=0; ia<anum; ia++) {

            double a = ia*astep + amin;
            double b = -hits.uhits[i]*a + hits.vhits[i];

            //printf("i %d a %lf b %lf\n", i, a, b);
            h2.Fill(a, b, 1);
         }
      }
      int ia_min;
      int ib_min;
      int tmp;
      h2.GetMaximumBin(ia_min, ib_min, tmp);
      found_a = h2.GetXaxis()->GetBinCenter(ia_min);
      found_b = h2.GetYaxis()->GetBinCenter(ib_min);

      num_hits = hits.num_hits;
      num_inside=0;
      for (int ihit=0; ihit<num_hits; ihit++) {
         double v = found_a * hits.uhits[ihit] + found_b;
         double diff = v - hits.vhits[ihit];
         printf("ihit %d vcalc %f vhits %f diff %f\n", ihit, v, hits.vhits[ihit], diff);
         if (TMath::Abs(diff) < 0.05) {
            num_inside++;
         }
      }
   };
   void print(int iev)
   {
      printf("Hough:: iev %d found_a %f found_b %f (num_hits %d num_inside %d)\n", iev, found_a, found_b, num_hits, num_inside);
   };
};

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

struct Canvas
{
   TCanvas* c1;
   int nx;
   int ny;
   int num_h1d;
   int num_h2d;
   TH1F* h1d[1000];
   TH2F* h2d[1000];
   char h1name[100][1000];
   char h2name[100][1000];
   int h1idx[1000];
   int h2idx[1000];
   void init(int _nx, int _ny)
   {
      c1 = new TCanvas("c1","", 500*_nx, 500*_ny);
      c1->Divide(_nx,_ny);
      nx = _nx;
      ny = _ny;
      num_h1d = 0;
      num_h2d = 0;
   };
   void cd(int idx)
   {
      c1->cd(idx);
   };
   void add_h1d(int idx, char* hname, char* htitle, int nx, double xmin, double xmax)
   {
      h1d[num_h1d] = new TH1F(hname, "", nx, xmin, xmax);
      h1d[num_h1d]->SetStats(0);
      h1d[num_h1d]->SetStats(0);
      strcpy(h1name[num_h1d], htitle);
      h1idx[num_h1d] = idx;
      num_h1d++;
   };
   void add_h2d(int idx, char* hname, char* htitle, int nx, double xmin, double xmax, int ny, double ymin, double ymax)
   {
      h2d[num_h2d] = new TH2F(hname, "", nx, xmin, xmax, ny, ymin, ymax);
      h2d[num_h2d]->SetStats(0);
      strcpy(h2name[num_h2d], htitle);
      h2idx[num_h2d] = idx;
      num_h2d++;
   };
   void debug()
   {
      printf("num_h1d %d\n", num_h1d);
      for (int i=0; i<num_h1d; i++) {
         printf("i %d h1idx %d h1name %s\n", i, h1idx[i], h1name[i]);
      }
      printf("num_h2d %d\n", num_h2d);
      for (int i=0; i<num_h2d; i++) {
         printf("i %d h2idx %d h2name %s\n", i, h2idx[i], h2name[i]);
      }
   };
   void update_title_prefix(char* prefix)
   {
      char title[128];
      for (int i=0; i<num_h1d; i++) {
         sprintf(title, "%s %s", prefix, h1name[i]);
         h1d[i]->SetTitle(title);
      }
      for (int i=0; i<num_h2d; i++) {
         sprintf(title, "%s %s", prefix, h2name[i]);
         h2d[i]->SetTitle(title);
      }
   };
   void draw_hists()
   {
      for (int i=0; i<nx*ny; i++) {
         c1->cd(i+1);
         for (int j=0; j<num_h1d; j++) {
            if (h1idx[j]==i) h1d[j]->Draw();
         }
         for (int j=0; j<num_h2d; j++) {
            if (h2idx[j]==i) h2d[j]->Draw();
         }
      }
   };
   void print(char* pdf_name)
   {
      c1->Print(pdf_name);
   };
};
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
   //int total = 100;

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

   Canvas c1;
   c1.init(6, 3);

   c1.add_h2d(0, "h11", "ALL - MC XY", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(1, "h21", "ALL - Wire XY@endplate", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(2, "h31", "ALL - Conformal XY@endplate", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   c1.add_h2d(3, "h41", "ALL - Wire XY @hitz", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(4, "h51", "ALL - MC Z vs ihit XY", 100, 0, 200, 100, -150, 150);
   c1.add_h2d(5, "h61", "ALL - MC PZ vs ihit", 100, 0, 200, 100, -150, 150); // MeV

   c1.add_h2d(6, "h12", "EVEN - MC XY", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(7, "h22", "EVEN - Wire XY@endplate", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(8, "h32", "EVEN - Conformal XY@endplate", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   c1.add_h2d(9, "h42", "EVEN - Wire XY@hitz", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(10,"h52", "EVEN - MC Z vs ihit XY", 100, 0, 200, 100, -150, 150);
   c1.add_h2d(11,"h62", "EVEN - MC PZ vs ihit", 100, 0, 200, 100, -150, 150); // MeV

   c1.add_h2d(12,"h13", "EVEN - MC XY", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(13,"h23", "EVEN - Wire XY@endplate", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(14,"h33", "EVEN - Conformal XY@endplate", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);
   c1.add_h2d(15,"h43", "EVEN - Wire XY@hitz", 100, -100, 100, 100, -100, 100);
   c1.add_h2d(16,"h53", "EVEN - MC Z vs ihit XY", 100, 0, 200, 100, -150, 150);
   c1.add_h2d(17,"h63", "EVEN - MC PZ vs ihit", 100, 0, 200, 100, -150, 150); // MeV

   c1.debug();


   char title[12];
   for (int iev=8; iev<9; iev++) {

      sprintf(title, "iev %d ", iev);
      c1.update_title_prefix(title);
      c1.draw_hists();

      inROOT.getEntry(iev);
      bool directHit = inROOT.InDirectHitAtTriggerCounter();
      if (!directHit) continue;

      int numHits = inROOT.getNumHits();
      if (numHits==0) continue;

      printf("iev %d numHits %d\n", iev, numHits );

      struct MyHits hits;

      for (int ihit=0; ihit<numHits; ihit++) {
         int ilayer = inROOT.getIlayer(ihit);
         int icell = inROOT.getIcell(ihit);
         int iturn = inROOT.getIturn(ihit);
         if (iturn!=0) continue; 

         inROOT.getPosMom(ihit, mcPos, mcMom);
         inROOT.getWirePosAtEndPlates(ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         inROOT.getWirePosAtHitPoint(ihit, w_x, w_y, w_z);

         hits.add_hit(ilayer, icell, iturn);

         double r2 = w_x1*w_x1 + w_y1*w_y1;
         double w_u1 = 2.0*w_x1/r2;
         double w_v1 = 2.0*w_y1/r2;

         TMarker* m1 = getMarker(ilayer, iturn, mcPos.X(), mcPos.Y());
         TMarker* m2 = getMarker(ilayer, iturn, w_x1, w_y1);
         TMarker* m3 = getMarker(ilayer, iturn, w_u1, w_v1);
         TMarker* m4 = getMarker(ilayer, iturn, w_x, w_y);
         TMarker* m5 = getMarker(ilayer, iturn, ihit, mcPos.Z());
         TMarker* m6 = getMarker(ilayer, iturn, ihit, mcMom.Z()*1000); // GeV -> MeV

         c1.cd(1); m1->Draw();
         c1.cd(2); m2->Draw();
         c1.cd(3); m3->Draw();
         c1.cd(4); m4->Draw();
         c1.cd(5); m5->Draw();
         c1.cd(6); m6->Draw();
         int offset = (ilayer%2==0)?6:12;
         c1.cd(offset+1); m1->Draw();
         c1.cd(offset+2); m2->Draw();
         c1.cd(offset+3); m3->Draw();
         c1.cd(offset+4); m4->Draw();
         c1.cd(offset+5); m5->Draw();
         c1.cd(offset+6); m6->Draw();

         //printf("iev %d MC:     ihit %d (%f, %f, %f)\n", iev, ihit, mcPos.X(), mcPos.Y(), mcPos.Z());
         //printf("iev %d Wire End:    ihit %d (%f, %f, %f) - (%f, %f, %f)\n", iev, ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         //printf("iev %d Conf End:    ihit %d (%g, %g)\n", iev, ihit, w_u1, w_v1);
         //printf("iev %d MCWire: ihit %d (%f, %f, %f)\n", iev, ihit, w_x, w_y);
      }
      c1.print(Form("pdf/%05d.pdf", iev));

      Canvas c2;
      c2.init(2,1);

      c2.add_h2d(0,"h101", "Wire XY@z", 100, -100, 100, 100, -100, 100);
      c2.add_h2d(1,"h102", "Conf UV@z", 100, -1e-1, 1e-1, 100, -1e-1, 1e-1);

      struct Hough hough;
      int max_num = 0;
      double max_z1 = 0;
      double max_z2 = 0;
      //      for (int iz1=0; iz1<100; iz1++) {
      //         for (int iz2=0; iz2<100; iz2++) {
      //      for (int iz1=60; iz1<61; iz1++) {
      for (int iz1=0; iz1<1; iz1++) {
         for (int iz2=90; iz2<91; iz2++) {

            double z1 = iz1 - 50.0;
            double z2 = iz2 - 50.0;

            sprintf(title, "iev %d z1 %5.0f z2 %5.0f", iev, z1, z2);
            c2.update_title_prefix(title);
            c2.draw_hists();

            hits.calc_xyz(inROOT.getConfig(), z1, z2);
            hough.transform(hits);
            if (hough.num_inside > max_num) {
               max_num = hough.num_inside;
               max_z1 = z1;
               max_z2 = z2;
            }
            hough.print(iev);

            for (int ihit=0; ihit<hits.num_hits; ihit++) {
               int ilayer = hits.ilayer[ihit];
               int iturn = hits.iturn[ihit];
               TMarker* m1 = getMarker(ilayer, iturn, hits.xhits[ihit], hits.yhits[ihit]);
               TMarker* m2 = getMarker(ilayer, iturn, hits.uhits[ihit], hits.vhits[ihit]);

               c2.cd(1); m1->Draw();
               c2.cd(2); m2->Draw();
            }
            c2.print(Form("pdf/hough/%05d-%d-%d.pdf", iev,iz1,iz2));
         }
      }
      printf("iev %d num_hits %d max_num %d max_z1 %f max_z2 %f\n", iev, hough.num_hits, max_num, max_z1, max_z2);

   }
   return 0;
   }
