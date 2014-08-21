$1 == "Race" { hist[substr($2,1,length($2)-1)]++ }

END {
  for (race in hist) 
    printf( "%10s %d\n", race, hist[race] )
}