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
open INFILE, "<$fileIn" or die "Cannot open $fileIn";

my $fileOut = $ARGV[1] || "";
open OUTFILE, ">$fileOut" or die "Cannot open $fileOut";

my $line = "";
my $makeline = "";
my $directoryline = "";
my $prelinkline = "";
my $compileline = "";
my $errorfound = 0;

while (defined($line = <INFILE>)) {
# print OUTFILE "Input: ";
# print OUTFILE $line;
   if ($line =~ / make /) { # execution of make command
      $makeline = $line;
      $directoryline = "";
      $prelinkline = "";
      $compileline = "";
      $errorfound = 0;
   }
   elsif ($line =~ /Entering/) { # entering another directory because of a make command
      $directoryline = $line;
      $prelinkline = "";
      $compileline = "";
      $errorfound = 0;
   }
   elsif ($line =~ /prelinker: /) { # prelinker line
      $prelinkline = $line;
      $compileline = "";
      $errorfound = 0;
   }
   elsif ($line =~ /CC / or $line =~ /cc /) { # compile line
      $compileline = $line;
      $errorfound = 0;
   }
   elsif ($line =~ /INFO/) { # an end of an error or warning
      $errorfound = 0;
   }
   elsif ($line =~ /warning/ and $line =~ /target `clean'/) { # ignore this wrning as we can't change it
      $errorfound = 0;
   }
   elsif ($errorfound eq 1 or $line =~ /ERROR/ or $line =~ /WARNING/ or $line =~ /warning/) {
      # begin of an error or warning
      if ($errorfound eq 0)
      {
	 if ($makeline ne "") {
            # print OUTFILE "Make: ";
	    print OUTFILE $makeline;
	    $makeline = "";
	 }
	 if ($directoryline ne "") {
            # print OUTFILE "Directory: ";
	    print OUTFILE $directoryline;
	    $directoryline = "";
	 }
	 if ($prelinkline ne "") {
            # print OUTFILE "Prelink: ";
	    print OUTFILE $prelinkline;
	    $prelinkline = "";
	 }
	 if ($compileline ne "") {
            # print OUTFILE "Compile: ";
	    print OUTFILE $compileline;
	    $compileline = "";
	 }
      }
      # print OUTFILE "Error: ";
      print OUTFILE $line;
      $errorfound = 1;
      if ($line =~ /warning/) {
         # only one line
	 $errorfound = 0;
      }
   }
}

close INFILE;
close OUTFILE;

exit;

