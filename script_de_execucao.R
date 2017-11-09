#!/usr/bin/env Rscript
args = commandArgs(trailingOnly=TRUE)

for (j in seq(from=0, to=29, by=1)){

	for (i in seq(from=10, to=50, by=10)){
	  rand <- sample(1:9382, 1)
	  print(paste ('Rodada=',  j))
	  print(paste ('Numero Nos=',  i))
	  print("CBR e UDP")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=true --transportMode=0 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	  print("CBR e TCP")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=true --transportMode=1 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	  print("CBR e MISTO")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=true --transportMode=2 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	  print("BURST e UDP")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=false --transportMode=0 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	  print("BURST e TCP")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=false --transportMode=1 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	  print("BURST e MISTO")
	  system(paste("NS_GLOBAL_VALUE=\"RngRun=", rand,"\" ./waf --run \"scratch/wifiinfra --nodes=",i," --runningTime=60 --traffic=false --transportMode=2 --printLog=false --prefix=\\\"",j,"\\\"   \"", collapse=", ", sep=""))
	}

}