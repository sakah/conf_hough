#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "TCanvas.h"
#include "TH2F.h"
#include "TF1.h"
#include "TMarker.h"
#include "InputROOT.h"
#include "TGraphErrors.h"
#include "TEllipse.h"

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
   double eu[10000];
   double ev[10000];
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
         eu[ihit] = 0.1;
         ev[ihit] = 0.1;
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
   TH2F* h2;
   double found_a;
   double found_b;
   int num_hits;
   int num_inside;
   Hough()
   {
      h2 = NULL;
   };
   void fit(MyHits& hits, int iev, double z1, double z2)
   {
      TGraphErrors* gr = new TGraphErrors(hits.num_hits, hits.uhits, hits.vhits, hits.eu, hits.ev);
      gr->Fit("pol1");
      TF1* f = gr->GetFunction("pol1");
      double chi2 = f->GetChisquare();
      printf("iev %d z1 %f z2 %f chi2 %f\n", iev, z1, z2, chi2);
   };
   void transform(MyHits& hits)
   {
      double astep = 0.1;
      double amin = -10;
      double amax = 10;
      double bstep = 0.001;
      double bmin = -0.05;
      double bmax = 0.05;
      int anum = (amax-amin)/astep;
      int bnum = (bmax-bmin)/bstep;
      //printf("anum %d %f %f bnum %d %f %f\n", anum, amin, amax, bnum, bmin, bmax);
      if (h2==NULL) {
         h2 = new TH2F("h2","",anum, amin, amax, bnum, bmin, bmax);
      }
      h2->Reset();

      for (int i=0; i<hits.num_hits; i++) {
         for (int ia=0; ia<anum; ia++) {

            double a = ia*astep + amin;
            double b = -hits.uhits[i]*a + hits.vhits[i];

            //printf("i %d a %lf b %lf\n", i, a, b);
            h2->Fill(a, b, 1);
         }
      }
      int ia_min;
      int ib_min;
      int tmp;
      h2->GetMaximumBin(ia_min, ib_min, tmp);
      found_a = h2->GetXaxis()->GetBinCenter(ia_min);
      found_b = h2->GetYaxis()->GetBinCenter(ib_min);
   };
   void calc_diff(MyHits& hits, double* diff)
   {
      num_hits = hits.num_hits;
      num_inside=0;
      for (int ihit=0; ihit<num_hits; ihit++) {
         double v = found_a * hits.uhits[ihit] + found_b;
         diff[ihit] = v - hits.vhits[ihit];
         //printf("ihit %d vcalc %f vhits %f diff %f\n", ihit, v, hits.vhits[ihit], diff);
         if (TMath::Abs(diff[ihit]) < 0.05) {
            num_inside++;
         }
      }
   };
   void print(int iev)
   {
      printf("Hough:: iev %d found_a %f found_b %f (num_hits %d num_inside %d)\n", iev, found_a, found_b, num_hits, num_inside);
   };
   TF1* get_line()
   {
      TF1* f1 = new TF1("f1", "[0]+[1]*x", -1e-1, 1e-1);
      f1->SetParameters(found_b, found_a);
      return f1;
   };
};

TMarker* getMarker(int marker_style, int ilayer, int iturn, double x, double y)
{
   //TMarker *m1 = new TMarker(x, y, 8);
   //m1->SetMarkerSize(0.3);

   //if (ilayer%2==1) m1->SetMarkerColor(kRed);
   //if (ilayer%2==0) m1->SetMarkerColor(kBlue);

   //if (iturn==0) m1->SetMarkerStyle(8); // ●
   //if (iturn==1) m1->SetMarkerStyle(5); // x
   //if (iturn==2) m1->SetMarkerStyle(22); // ▲
   //if (iturn==3) m1->SetMarkerStyle(26); // △

   //return m1;
   TMarker *m1 = new TMarker(x, y, marker_style);
   m1->SetMarkerSize(0.3);

   int color=kBlack;
   if (iturn==0) color=kRed;
   if (iturn==1) color=kBlue;
   if (iturn==2) color=kCyan;
   if (iturn==3) color=kMagenta;
   if (iturn==4) color=kGreen;
   if (iturn>=5) color=kBlack;
   m1->SetMarkerColor(color);

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
      c1 = new TCanvas("c1","", 1000*_nx, 1000*_ny);
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

void draw_radius(struct config* config)
{
   for (int ilayer=0; ilayer<config->sense_layer_size; ilayer++) {
      if (ilayer%2==0) continue;
      double r = config_get_layer_radius(config, ilayer, LAYER_TYPE_SENSE, 0);
      TEllipse* e = new TEllipse(0,0,r);
      e->SetFillStyle(0);
      e->SetLineWidth(1);
      e->SetLineStyle(1);
      e->Draw();
   }
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

   //for (int iev=0; iev<1000; iev++) {
   for (int iev=306; iev<307; iev++) {

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
//         if (iturn!=0) continue; 
//         printf("ilayer %d icell %d iturn %d\n", ilayer, icell, iturn);

         inROOT.getPosMom(ihit, mcPos, mcMom);
         inROOT.getWirePosAtEndPlates(ihit, w_x1, w_y1, w_z1, w_x2, w_y2, w_z2);
         inROOT.getWirePosAtHitPoint(ihit, w_x, w_y, w_z);

         hits.add_hit(ilayer, icell, iturn);

         double r2 = w_x1*w_x1 + w_y1*w_y1;
         double w_u1 = 2.0*w_x1/r2;
         double w_v1 = 2.0*w_y1/r2;

         TMarker* m1 = getMarker(8, ilayer, iturn, mcPos.X(), mcPos.Y());
         TMarker* m2 = getMarker(8, ilayer, iturn, w_x1, w_y1);
         TMarker* m3 = getMarker(8, ilayer, iturn, w_u1, w_v1);
         TMarker* m4 = getMarker(8, ilayer, iturn, w_x, w_y);
         TMarker* m5 = getMarker(8, ilayer, iturn, ihit, mcPos.Z());
         TMarker* m6 = getMarker(8, ilayer, iturn, ihit, mcMom.Z()*1000); // GeV -> MeV

      }

      // Endplate
      Canvas c2;
      c2.init(1,1);

      c2.add_h2d(0, "h100", "Wire XY@endplate/MCZ", 100, -100, 100, 100, -100, 100);


      c2.h2d[0]->Reset();

      c2.draw_hists();
      c2.cd(1); draw_radius(inROOT.getConfig());

      for (int ihit=0; ihit<hits.num_hits; ihit++) {
         int ilayer = hits.ilayer[ihit];
         int icell = hits.icell[ihit];
         int iturn = hits.iturn[ihit];
         double x1, y1, z1;
         double x2, y2, z2;
         //printf("ilayer %d icell %d iturn %d\n", ilayer, icell, iturn);
         inROOT.getWirePosAtEndPlates(ihit, x1, y1, z1, x2, y2, z2);
         TMarker* m1 = getMarker(8, ilayer, iturn, x1, y1);
         c2.cd(1); 
         m1->Draw();
      }
      c2.print(Form("pdf/endplate_%05d.pdf", iev));

      // MCZ
      Canvas c3;
      c3.init(1,1);

      c3.add_h2d(0, "h100", "Wire XY@endplate/MCZ", 100, -100, 100, 100, -100, 100);


      c3.h2d[0]->Reset();

      c3.draw_hists();
      c3.cd(1); draw_radius(inROOT.getConfig());

      for (int ihit=0; ihit<hits.num_hits; ihit++) {
         int ilayer = hits.ilayer[ihit];
         int icell = hits.icell[ihit];
         int iturn = hits.iturn[ihit];
         double xmc, ymc, zmc;
         printf("ilayer %d icell %d iturn %d\n", ilayer, icell, iturn);
         inROOT.getWirePosAtHitPoint(ihit, xmc, ymc, zmc);
         TMarker* m1 = getMarker(5, ilayer, iturn, xmc, ymc);
         c3.cd(1); 
         m1->Draw();
      }
      c3.print(Form("pdf/mcz_%05d.pdf", iev));
   }
   return 0;
}
