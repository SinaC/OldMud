#! /usr/bin/perl -w
# -*- mode: cperl -*-

#############################################
# Syntax: perl convertCircle.pl <area number>
#
#  Convert .zon, .mob, .obj, .shp, .wld  into  .are
#   and    .trg                          into  .script
#

##Global variables
#input dir
$circleDir = "c:/tmp/MUD/oasis/current_world/";
#output dir
$outputDir = "c:/cygwin/home/fch/Rom/area/toConvert/";
$header = "#Style Circle\n#AREADATA\n";
$resets = "#RESETS\n";

##Main
$areaNumber = $ARGV[0];

#read zon, get area name and store area header
$areaName = &readZon( $areaNumber );
#read mobiles
$mobiles = "#MOBILES\n" . &readInfo( $areaNumber, "mob" );
#read mobiles
$objects = "#OBJECTS\n" . &readInfo( $areaNumber, "obj" );
#read rooms
$rooms = "#ROOMS\n" . &readInfo( $areaNumber, "wld" );
#read shops
$shops = "#SHOPS\n" . &readInfo( $areaNumber, "shp" );
#read scripts
$scripts = "//script for AREA $areaName\n" . &readInfo( $areaNumber, "trg" );


#print "HEADER: $header\n";
#print "MOBILES: $mobiles\n";
#print "OBJECTS: $objects\n";
#print "ROOMS: $rooms\n";
#print "RESETS: $resets\n";
#print "SHOPS: $shops\n";

#output file .are
$areaName =~ s/ /_/g;
$output = "$outputDir$areaName\.are";
print "Writing $output\n";
open( OUTPUT_HANDLE, ">$output") || die "Cant open output file: $!\n";
print OUTPUT_HANDLE "$header\n\n";
print OUTPUT_HANDLE "$mobiles","#0\n\n";
print OUTPUT_HANDLE "$objects","#0\n\n";
print OUTPUT_HANDLE "$rooms","#0\n\n";
print OUTPUT_HANDLE "$resets","\n\n";
print OUTPUT_HANDLE "$shops","0\n\n";
print OUTPUT_HANDLE "\n#\$\n";
close( OUTPUT_HANDLE );

#output file .script
$output = "$outputDir$areaName\.script";
print "Writing $output\n";
open( OUTPUT_HANDLE, ">$output") || die "Cant open output file: $!\n";
foreach $line (split("\n",$scripts) ) {
  print OUTPUT_HANDLE "//$line\n";
}
close( OUTPUT_HANDLE );

print "Conversion completed.\n";


## Subroutines

## Read resets, return area name, create header and resets
sub readZon {
  local($areaNumber) = $_[0];
  local($filename) = "$circleDir"."zon/$areaNumber.zon";
  print "Reading $filename\n";
  # Read and store Resets
  open( ZON_HANDLE, "<$filename") || die "Cant open $filename file: $!\n";
  local($line);
  local($builder);
  local($area);
  local($minVnum);
  local($maxVnum);
  local($val1);
  local($val2);
  local($i) = 1;
  while ( $line = <ZON_HANDLE> ) {
    if ( ( $line eq "S" ) || ( index($line,'$') == 0 ) ) {#stops when S or $ is read
      last;
    }
    if ( $i == 1 ) { #1st line: skips
    }
    elsif ( $i == 2 ) { #2nd line: builder
      $builder = $line; chop($builder);
    }
    elsif ( $i == 3 ) { #3rd line: area name
      $area = $line; chop($area);
    }
    elsif ( $i == 4 ) { #4th line: <min vnum> <max vnum> <val1> <val2>
      ($minVnum, $maxVnum, $val1, $val2 ) = split( " ", $line );
    }
    else {
      $resets = $resets . $line; #concat resets
    }
    $i++;
 }
  close( ZON_HANDLE );

  #Create area header
  $header = $header . "Name $area\n";
  $header = $header . "Builders $builder\n";
  $header = $header . "VNUMs $minVnum $maxVnum\n";
  $header = $header . "Credits {{None}  CAW    $area\n";
  $header = $header . "Security 9\n";
  $header = $header . "Flags 0\n";
  $header = $header . "End\n";

  #return area name
  chop($area);
  $area;
}

## Read and store mob/obj/room/shops
sub readInfo {
  local($areaNumber) = $_[0];
  local($ext) = $_[1];
  local($result) = "\n";
  local($filename) = "$circleDir$ext/$areaNumber.$ext";
  if ( !(-e $filename ) ) {
    print "$filename cannot be found.\n";
  }
  else {
    print "Reading $filename\n";
    open( FILE_HANDLE, "$filename") || die "Cant open $filename file: $!\n";

    # Read and store
    local($line);
    while ( $line = <FILE_HANDLE> ) {
      if ( index($line,'$') == 0 ) { #stops when $ is read
	last;
      }
      $result = $result . $line; #concat info
    }
    
    close( FILE_HANDLE );
  }

  $result; #return result
}
