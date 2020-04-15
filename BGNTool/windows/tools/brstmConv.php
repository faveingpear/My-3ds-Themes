<?php


// Written by soneek [soneek00@gmail.com][http://github.com/soneek]

function hex2str($hex) {    // from http://www.linux-support.com/cms/php-convert-hex-strings-to-ascii-strings/
    $str = '';
    for($i=0;$i<strlen($hex);$i+=2) $str .= chr(hexdec(substr($hex,$i,2)));
    return $str;
}

function read( $handle, $offset, $nbytes, $type/*bin=0 hex=1 dec=2*/, $endianFlag)
{
    fseek( $handle, $offset );
    if ($endianFlag)
        $output = strrev(fread( $handle, $nbytes ));
    else
        $output = fread( $handle, $nbytes );
    if ( $type != "b" ) {
        $output = bin2hex($output);
    }
    if ( $type == "d" ) {
        $output = hexdec( $output );
    }
    else if ($type == "s") {
        $output = hex2str($output);
    }
    return $output;
}


$file = $argv[1];
if ( !file_exists( $file ) ) {
    echo "File doesn't exist (" . $file . ")\n";
    exit;
}

else {
    if (!isset($argv[2]) || ($argv[2] != "bcstm" && $argv[2] != "bfstm"))
        $type = "bcstm";    // Default option will be bcstm output
    else
        $type = $argv[2];
  
    if (isset($argv[3]) && $argv[3] == "fea")
        $game = "fea";
    else
        $game = "";
      
    if ($type == "bcstm")
        $endFlag = true;
    else
        $endFlag = false;
  
    $brstm = fopen($file, "rb");
    if (read($brstm, 0, 4, "s", 0) != "RSTM") {
        echo "Not a brstm file\n";
        fclose($brstm);
        exit;
    }
    else {
        echo "BRSTM opened\n";
        $rstmSize = read($brstm, 8, 4, "d", $endFlag);
        $rstmInfoOffset = read($brstm, 0x10, 4, "d", 0);
        $rstmType = read($brstm, 0x60, 1, "d", $endFlag);
        if ($rstmType != 2) {
            echo "This only supports BRSTMs with the DSP-ADPCM codec\n";
            fclose($brstm);
            exit;
        }
        else {
            $rstmSize = read($brstm, 0x8, 4, "d", false);
            $rstmInfoSize = read($brstm, 0x14, 4, "d", false);
            $rstmSeekOffset = read($brstm, 0x18, 4, "d", false);
            $rstmSeekSize = read($brstm, 0x1c, 4, "d", false);
            $rstmDataOffset = read($brstm, 0x20, 4, "d", false);
            $rstmDataSize = read($brstm, 0x24, 4, "d", false);
          
            $rstmLoopFlag = read($brstm, 0x61, 1, "d", false);
            $rstmChannelCount = read($brstm, 0x62, 1, "d", false);
            $rstmSampleRate = read($brstm, 0x64, 2, "d", false);
            $rstmLoopStart = read($brstm, 0x68, 4, "d", false);
            $rstmSampleCount = read($brstm, 0x6c, 4, "d", false);
            $coeffPtr1 = read($brstm, 0x5c, 4, "d", false);
            $coeffPtr2 = read($brstm, $coeffPtr1 + 0x50, 4, "d", false);
            $coeffOffset = 0x50 + $coeffPtr2;
          
            // Opening output file
            $outfile = fopen(str_replace("brstm", $type, $file), "wb");
          
            if ($type == "bcstm") {
                fwrite($outfile, "CSTM" . pack("vvvnVvvvvVVvvVVvvV*", 0xfeff, 0x40, 0, 2, $rstmSize, 3, 0, 0x4000, 0, $rstmInfoOffset, $rstmInfoSize, 0x4001, 0, $rstmSeekOffset, $rstmSeekSize, 0x4002, 0, $rstmDataOffset, $rstmDataSize, 0, 0) . "INFO" . pack("VvvVvvVvvV", $rstmInfoSize, 0x4100, 0, 0x18, 0x101, 0, 0x50, 0x101, 0, 0x54 + ceil($rstmChannelCount / 2) * 8) . pack("CCCCV*", $rstmType, $rstmLoopFlag, $rstmChannelCount, 0, $rstmSampleRate, $rstmLoopStart, $rstmSampleCount));
                for ($i = 0; $i < 6; $i++)
                    fwrite($outfile, read($brstm, 0x74 + $i * 4, 4, "b", $endFlag));
                fwrite($outfile, pack("VVvvVV", 4, 0x3800, 0x1f00, 0, 0x18, ceil($rstmChannelCount / 2)));
                for ($i = 0; $i < ceil($rstmChannelCount / 2); $i++)
                    fwrite($outfile, pack("vvV", 0x4101, 0, $rstmChannelCount * 12 + 8 + $i * 0x14));
                fwrite($outfile, pack("V", $rstmChannelCount));
                for ($i = 0; $i < $rstmChannelCount; $i++)
                    fwrite($outfile, pack("vvV", 0x4102, 0, 4 + $rstmChannelCount * 8 + ($rstmChannelCount / 2) * 0x14 + $i * 8));
                $rstmBlahMetaDataOffset = read($brstm, 0x9c, 4, "d", false);
                for ($i = 0; $i < $rstmChannelCount / 2; $i++) {
                    if ($game == "fea" && $i == 0)
                        fwrite($outfile, pack("CCvvvVVccv", 0x73, 64, 0, 0x100, 0, 0xc, 2, 2 * $i, 2 * $i + 1, 0));
                    else
                        fwrite($outfile, pack("CCvvvVVccv", 127, 64, 0, 0x100, 0, 0xc, 2, 2 * $i, 2 * $i + 1, 0));
                }
                for ($i = 0; $i < $rstmChannelCount; $i++)
                    fwrite($outfile, pack("vvV", 0x300, 0, $rstmChannelCount * 8 + $i * 0x26));  
              
                // Now to write coefficients and stuff

                for ($i = 0; $i < $rstmChannelCount; $i++) {
                    for ($j = 0; $j < 16; $j++)
                        fwrite($outfile, read($brstm, $coeffOffset + $i * 0x38 + $j * 2, 2, "b", $endFlag));
                    for ($j = 0; $j < 7; $j++)
                        fwrite($outfile, read($brstm, $coeffOffset + $i * 0x38 + $j * 2 + 0x22, 2, "b", $endFlag));
                }
              
                // Padding the file
                if ((ftell($outfile) % 16) != 0)
                    $padlength = 16 - (ftell($outfile) % 16);
                else
                    $padlength = 0;
                for ($i = 0; $i < $padlength; $i++)
                    fwrite($outfile, pack("C", 0));
                $infoSizeDifference = $rstmInfoSize + $rstmInfoOffset - ftell($outfile);
                echo $rstmInfoOffset;
                fseek($outfile, 0xc, SEEK_SET);
                fwrite($outfile, pack("V", $rstmSize - $infoSizeDifference));
                fseek($outfile, 0x1c, SEEK_SET);
                fwrite($outfile, pack("V", $rstmInfoSize - $infoSizeDifference));
                fseek($outfile, 0x24, SEEK_SET);
                fwrite($outfile, pack("V", $rstmSeekOffset - $infoSizeDifference));
                fseek($outfile, 0x30, SEEK_SET);
                fwrite($outfile, pack("V", $rstmDataOffset - $infoSizeDifference));
                fseek($outfile, 0, SEEK_END);
                fseek($brstm, $rstmSeekOffset + 0x10, SEEK_SET);
                fwrite($outfile, "SEEK" . pack("V*", $rstmSeekSize, 0, 0));
                for ($i = 0; $i < ($rstmSeekSize - 16) / 2; $i++)
                    fwrite($outfile, strrev(fread($brstm, 2)));
                fseek($brstm, $rstmDataOffset + 0x20, SEEK_SET);
                fwrite($outfile, "DATA" . pack("V*", $rstmDataSize, 0, 0, 0, 0, 0, 0));
              
            }
          
            // Converting BRSTM to BFSTM
          
            else if ($type == "bfstm") {
                fwrite($outfile, "FSTM" . pack("nnnnNnnnnNNnnNNnnN*", 0xfeff, 0x40, 3, 0, $rstmSize, 3, 0, 0x4000, 0, $rstmInfoOffset, $rstmInfoSize, 0x4001, 0, $rstmSeekOffset, $rstmSeekSize, 0x4002, 0, $rstmDataOffset, $rstmDataSize, 0, 0) . "INFO" . pack("NnnNnnNnnN", $rstmInfoSize, 0x4100, 0, 0x18, 0x101, 0, 0x50, 0x101, 0, 0x54 + ceil($rstmChannelCount / 2) * 8) . pack("CCCCN*", $rstmType, $rstmLoopFlag, $rstmChannelCount, 0, $rstmSampleRate, $rstmLoopStart, $rstmSampleCount));
                              
                for ($i = 0; $i < 6; $i++)
                    fwrite($outfile, read($brstm, 0x74 + $i * 4, 4, "b", $endFlag));
                fwrite($outfile, pack("NNnnNV", 4, 0x3800, 0x1f00, 0, 0x18, ceil($rstmChannelCount / 2)));
                for ($i = 0; $i < ceil($rstmChannelCount / 2); $i++)
                    fwrite($outfile, pack("nnN", 0x4101, 0, $rstmChannelCount * 12 + 8 + $i * 0x14));
                fwrite($outfile, pack("N", $rstmChannelCount));
                for ($i = 0; $i < $rstmChannelCount; $i++)
                    fwrite($outfile, pack("nnN", 0x4102, 0, 4 + $rstmChannelCount * 8 + ceil($rstmChannelCount / 2) * 0x14 + $i * 8));
                for ($i = 0; $i < $rstmChannelCount / 2; $i++) {
                    fwrite($outfile, pack("CCnNNNCCn", 127, 64, 0, 0x100, 0xc, 2, 2 * $i, 2 * $i + 1, 0));      
                }
                for ($i = 0; $i < $rstmChannelCount; $i++)
                    fwrite($outfile, pack("nnN", 0x300, 0, $rstmChannelCount * 8 + $i * 0x26));  
                for ($i = 0; $i < $rstmChannelCount; $i++) {
                    for ($j = 0; $j < 16; $j++)
                        fwrite($outfile, read($brstm, $coeffOffset + $i * 0x38 + $j * 2, 2, "b", $endFlag));
                    for ($j = 0; $j < 7; $j++)
                        fwrite($outfile, read($brstm, $coeffOffset + $i * 0x38 + $j * 2 + 0x22, 2, "b", $endFlag));
                }
              
                // Padding the file
                if ((ftell($outfile) % 16) != 0)
                    $padlength = 16 - (ftell($outfile) % 16);
                else
                    $padlength = 0;
                for ($i = 0; $i < $padlength; $i++)
                    fwrite($outfile, pack("C", 0));
                $infoSizeDifference = $rstmInfoSize + $rstmInfoOffset - ftell($outfile);
                echo $rstmInfoOffset;
                fseek($outfile, 0xc, SEEK_SET);
                fwrite($outfile, pack("N", $rstmSize - $infoSizeDifference));
                fseek($outfile, 0x1c, SEEK_SET);
                fwrite($outfile, pack("N", $rstmInfoSize - $infoSizeDifference));
                fseek($outfile, 0x24, SEEK_SET);
                fwrite($outfile, pack("N", $rstmSeekOffset - $infoSizeDifference));
                fseek($outfile, 0x30, SEEK_SET);
                fwrite($outfile, pack("N", $rstmDataOffset - $infoSizeDifference));
                fseek($outfile, 0, SEEK_END);
                fseek($brstm, $rstmSeekOffset + 0x10, SEEK_SET);
                fwrite($outfile, "SEEK" . pack("N*", $rstmSeekSize, 0, 0));
                for ($i = 0; $i < ($rstmSeekSize - 16) / 2; $i++)
                    fwrite($outfile, fread($brstm, 2));
                fseek($brstm, $rstmDataOffset + 0x20, SEEK_SET);
                fwrite($outfile, "DATA" . pack("N*", $rstmDataSize, 0, 0, 0, 0, 0, 0));
            }
          
            // The DATA portion is exactly the same, between B(R/C/F)STM,  so we can copy the rest as is
            while(!feof($brstm))
                fwrite($outfile, fread($brstm, 1024 * 8));
          
        }
    }
    fclose($outfile);
    fclose($brstm);
}

?>
