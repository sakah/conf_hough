all: 
	g++ -I ../CometPhase1-GENFIT2/ `root-config --cflags` `root-config --libs` ../CometPhase1-GENFIT2/InputROOT.o  \
		/home/had/hideyuki/private/genfit2/KEKCC/DriftChamber_new7/config.o  \
		/home/had/hideyuki/private/genfit2/KEKCC/DriftChamber_new7/wirehit.o  \
		/home/had/hideyuki/private/genfit2/KEKCC/DriftChamber_new7/util.o  \
      main.cc -o main

clean:
	rm -f *.o
