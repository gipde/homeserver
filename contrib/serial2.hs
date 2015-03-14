import System.IO
import System.Hardware.Serialport

main = do 
	let port = "/dev/ttyUSB0"   -- Linux
	h <- hOpenSerial port defaultSerialSettings
	hGetLine h >>= print
	hClose h
