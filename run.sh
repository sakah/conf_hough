#!/bin/sh

topdir="/home/had/hideyuki/private/genfit2/KEKCC/"
exec_path="$topdir/CometPhase1-GENFIT2"
wire_config="$topdir/DriftChamber_new7/B1T_square_1.5m_ver_2014_03_28.txt"

inroot="$topdir/root/digit_all_turn/1600/sub/hoge1-0.root"
sample_type="SampleD"
t2r_type="MCR"
rdrift_err_cm="0.02"
posSmear_cm="0.1"
momSmear_percent="0.001"
./main $inroot $wire_config $sample_type $t2r_type $rdrift_err_cm $posSmear_cm $momSmear_percent
