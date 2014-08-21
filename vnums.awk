$1 == "VNUMs" { 
  for (i= $2/100;i< ($3+1)/100;i++)
    map[i] = FILENAME
}

END {
  for (i=0;i<500;i++)
    printf("%4d : %s\n", i, map[i])
}

