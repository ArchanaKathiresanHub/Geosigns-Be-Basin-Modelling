#!/usr/bin/perl -w

use strict;
use diagnostics;

# print "# args = ";
# print $#ARGV + 1;
# print "\n";

if (@ARGV ne 2)
{
   print "Usage: analyzebuild.pl infile outfile";
   exit;
}

my $fileIn = $ARGV[0] || "";
open (INFILE, "<$fileIn") or die ("Cannot open $fileIn");

my $fileOut = $ARGV[1] || "";
open (OUTFILE, ">> $fileOut") or die ("Cannot open $fileOut");

my $headerline = "";
my $filesline = "";
my $dashline = "";
my $presenceline = "";
my $comparisonsline = "";
my $diffsline = "";

my $line = "";

while (defined($line = <INFILE>)) {
# print OUTFILE "Input: ";
# print OUTFILE $line;
   if ($line =~ /\$h5diff/) { # execution of make command
      $headerline = $line;
      $filesline = "";
      $dashline = "";
      $presenceline = "";
      $comparisonsline = "";
      $diffsline = "";
   }
   elsif ($line =~ /file1/) { # entering another directory because of a make command
      $filesline = $line;
      $dashline = "";
      $presenceline = "";
      $comparisonsline = "";
      $diffsline = "";
   }
   elsif ($line =~ /-----/) { # entering another directory because of a make command
      $dashline = $line;
      $presenceline = "";
      $comparisonsline = "";
      $diffsline = "";
   }
   elsif ($line =~ /    x   /and $line !~ /    x      x/) { # prelinker line
      $presenceline = $line;
      $comparisonsline = "";
      $diffsline = "";
   }
   elsif ($line =~ /Comparing/) { # compile line
      $comparisonsline = $line;
      $diffsline = "";
   }
   elsif ($line =~ /differences found/ and $line !~ /0 differences found/) { # an end of an error or warning
      $diffsline = $line;
   }

   if ($presenceline ne "")
   {
      print OUTFILE $headerline;
      print OUTFILE $filesline;
      print OUTFILE $dashline;
      print OUTFILE $presenceline;
      $headerline = "";
      $filesline = "";
      $dashline = "";
      $presenceline = "";
   }

   if ($diffsline ne "")
   {
      print OUTFILE $headerline;
      print OUTFILE "\n";
      print OUTFILE $comparisonsline;
      print OUTFILE $diffsline;
      $comparisonsline = "";
      $diffsline = "";
   }
}

close INFILE;
close OUTFILE;

exit;

