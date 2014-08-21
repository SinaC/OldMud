
$from = 156;
$to = 327;

$x = $from;
while ( $x <= $to ) {
  $exec = "perl convertCircle.pl $x";
  system $exec;
  $x++;
}
