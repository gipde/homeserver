import qualified Data.ByteString.Char8 as B
import System.Hardware.Serialport

main = do 
	let port = "/dev/ttyUSB0" 
	s <- openSerial port defaultSerialSettings { commSpeed = CS38400 }
	recv s 10 >>=print
	closeSerial s
