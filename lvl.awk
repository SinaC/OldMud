BEGIN {
  for (i = 1; i<= 110; i++)
    hist[i] = 0
}

$1 == "Levl" {
  hist[$2]++
  if (hist[$2] <= 5)
    name[$2] = name[$2] " " FILENAME
  else
    name[$2] = ""
}

END {
  for (i = 1; i<= 110; i++) 
    printf( "%4d %4d %s\n", i, hist[i], name[i] )
}