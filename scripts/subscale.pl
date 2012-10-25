#!/usr/bin/perl -w

use strict;
use diagnostics;

my $fileName = $ARGV[0] || "";
open INFILE, "<$fileName" or die "Cannot open $fileName";

my @info = ();

while (<INFILE>) {

   @info = split(/,/);

   my $directory   = $info[0] . "/";
   my $project_in  = $directory . $info[0] . ".project3d.reference";
   my $project_out = $directory . $info[0] . ".project3d";
   my $scaleX = $info[1];
   my $scaleY = $info[2];

   my @args = ("subscale", "-input", $project_in, "-output", $project_out,
   "-x", $scaleX, "-y", $scaleY);

   for (my $i = 0; $i < @args; $i++) {
      print $args[$i], " ";
   }
   print "\n";

   system(@args) == 0 or die "system @args failed: $?"

}


close INFILE;

exit;
