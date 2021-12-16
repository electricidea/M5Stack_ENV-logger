# R Script to analyze and visualize the measured data. 
# 
# see Hackster.io project page for the results:
# https://www.hackster.io/hague/sensor-comparison-temperature-humidity-and-air-pressure-554592

DataFilename <- file.choose()
Data <- read.table(DataFilename, dec = ".", numerals = c("warn.loss"), header=TRUE, fileEncoding="UTF-8-BOM")
# file format:
# ms	
# dht12_Temperature	
# dht12_Humidity	
# bme_Pressure	
# bme_Temperature	
# bme_altitute_1	
# bme_altitute_2	
# sht30_Temperature	
# sht30_Humidity	
# qmp_Pressure	
# qmp_altitute
Data$time <- ((Data$ms-Data$ms[1])/1000)/(60*60)

#Data$bme_Pressure <- (Data$bme_Pressure - mean(Data$bme_Pressure[1:5]))/100
#Data$qmp_Pressure <- (Data$qmp_Pressure - mean(Data$qmp_Pressure[1:5]))/100

Data$bme_Pressure <- (Data$bme_Pressure - mean(Data$bme_Pressure))
Data$qmp_Pressure <- (Data$qmp_Pressure - mean(Data$qmp_Pressure))

# turn on clipping:
par(xpd=FALSE)
plot(Data$time, Data$bme_Pressure, col="black", type="l", 
     ylim=c(min(c(min(Data$bme_Pressure), min(Data$qmp_Pressure))), 
            max(c(max(Data$bme_Pressure), max(Data$qmp_Pressure)))), 
     main="Pressure change over time",
     xlab = "time [h]", ylab = "pressure change [Pa]")
lines(Data$time, Data$qmp_Pressure, col="red", type="l")
grid(NULL,NULL, lwd = 2)
abline(h=0, col="red", lty=2)
# turn off clipping:
par(xpd=TRUE)
legend(0, max(c(max(Data$bme_Pressure), max(Data$qmp_Pressure)))+20, 
       legend=c("BMP280", "QMP6988"),
       col=c("black", "red"), lty=1, cex=0.8,
       box.lty=0)



DataFilename <- file.choose()
Data <- read.table(DataFilename, dec = ".", numerals = c("warn.loss"), header=TRUE, fileEncoding="UTF-8-BOM")
Data$time2 <- ((Data$ms-Data$ms[1])/1000)/(60*60)
Data$bme_Pressure <- Data$bme_Pressure/100
Data$qmp_Pressure <- Data$qmp_Pressure/100

# turn on clipping:
par(xpd=FALSE)
plot(Data$time, Data$bme_Pressure, col="black", type="l", 
     ylim=c(min(c(min(Data$bme_Pressure), min(Data$qmp_Pressure))), 
            max(c(max(Data$bme_Pressure), max(Data$qmp_Pressure)))), 
     main="Atmospheric pressure over time",
     xlab = "time [h]", ylab = "atmospheric pressure [hPa]")
lines(Data$time, Data$qmp_Pressure, col="red", type="l")
grid(NULL,NULL, lwd = 2)
# turn off clipping:
par(xpd=TRUE)
legend(0, max(c(max(Data$bme_Pressure), max(Data$qmp_Pressure)))+0.5, 
       legend=c("BMP280", "QMP6988"),
       col=c("black", "red"), lty=1, cex=0.8,
       box.lty=0)


# turn on clipping:
par(xpd=FALSE)
plot(Data$time, Data$dht12_Temperature, col="black", ylim=c(0, 25),
     type="l", main="Temperature over time outdoor",
     xlab = "Time [h]", ylab = "Temperature [°C]")
lines(Data$time, Data$sht30_Temperature, col="red", type="l")
abline(h=0, col="red", lty=2)
grid(NULL,NULL, lwd = 2)
# turn off clipping:
par(xpd=TRUE)
legend(0, 31, 
       legend=c("DHT12 ", "SHT30 "),
       col=c("black", "red"), lty=1, cex=0.8,
       box.lty=0)


# turn on clipping:
par(xpd=FALSE)
plot(Data$time, Data$dht12_Humidity, col="black", ylim=c(25, 85),
     type="l", 
     main="Relative Humidity over time outdoor",
     xlab = "Time [h]", ylab = "rel. Humidity [%]")
lines(Data$time, Data$sht30_Humidity, col="red", type="l")
grid(NULL,NULL, lwd = 2) 
# turn off clipping:
par(xpd=TRUE)
legend(0, 102.5, 
       legend=c("DHT12 ", "SHT30 "),
       col=c("black", "red"), lty=1, cex=0.8,
       box.lty=0)
