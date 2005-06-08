#!/usr/bin/perl
#  ********************************************************************
#  * COPYRIGHT:
#  * Copyright (c) 2002-2005, International Business Machines Corporation and
#  * others. All Rights Reserved.
#  ********************************************************************

# Script to generate the icudata.jar and testdata.jar files.  This file is
# part of icu4j.  It is checked into CVS.  It is generated from
# locale data in the icu4c project.  See usage() notes (below)
# for more information.

# This script requires perl.  For Win32, I recommend www.activestate.com.

# Ram Viswanadha
# copied heavily from genrbjar.pl
use File::Find;
use File::Basename;
use IO::File;
use Cwd;
use File::Copy;
use Getopt::Long;
use File::Path;
use File::Copy;
use Cwd;
use Cwd 'abs_path'; 

main();

#------------------------------------------------------------------
sub main(){

    GetOptions(
             "--icu-root=s" => \$icuRootDir,
             "--jar=s" => \$jarDir,
             "--icu4j-root=s" => \$icu4jDir,
             "--version=s" => \$version
             );
    $cwd = abs_path(getcwd);
    unless (defined $icuRootDir){
        $icuRootDir =abs_path($cwd."/../../..");
    }
    unless (defined $icu4jDir){
        $icu4jDir =abs_path($icuRootDir."/../icu4j");
    }
    unless (defined $jarDir){
        if(defined $ENV{'JAVA_HOME'}){
            $jarDir=$ENV{'JAVA_HOME'}."/bin";
        }else{
            print("JAVA_HOME enviroment variable undefined and --jar argument not specifed.\n");
            usage(); 
        }
    }
    
    $platform = getPlatform();
    $icuBinDir = $icuRootDir;
    
    $path=$ENV{'PATH'};
    
    if($platform eq "cygwin"){
        $icuBinDir .= "/source/bin";
        $icuLibDir = abs_path($icuBinDir."/../lib");
        $path .=":$icuBinDir:$icuLibDir";
        
        $libpath = $ENV{'LD_LIBRARY_PATH'}.":$icuLibDir";
        $ENV{'LD_LIBRARY_PATH'} = $libpath;
        
        print ("#####  LD_LIBRARY_PATH = $ENV{'LD_LIBRARY_PATH'}\n");
        
    }elsif($platform eq "MSWin32"){
        $icuBinDir =$icuRootDir."/bin";
        $path .=$icuBinDir;
        
    }
    $ENV{'PATH'} = $path;
    print ("#####  PATH = $ENV{'PATH'}\n");
    # TODO add more platforms and test on Linux and Unix
    
    $icuBuildDir =$icuRootDir."/source/data/out/build";
    $icuTestDataDir =$icuRootDir."/source/test/testdata/out/build/";    
    
    # now build ICU
    buildICU($platform, $icuRootDir, $icuTestDataDir);
    
    #figure out the version and endianess
    unless (defined $version){
        ($version, $endian) = getVersion();
        #print "#################### $version, $endian ######\n";
    }
    
    $icuswap = $icuBinDir."/icuswap -tb";
    $tempDir = $cwd."/temp";
    $version =~ s/\.//;
    $icu4jImpl = "com/ibm/icu/impl/data/";
    $icu4jDataDir = $icu4jImpl."/icudt".$version."b";
    $icu4jDevDataDir = "com/ibm/icu/dev/data/";
    $icu4jTestDataDir = "$icu4jDevDataDir/testdata";
    
    $icuDataDir =$icuBuildDir."/icudt".$version.checkPlatformEndianess();

    convertData($icuDataDir, $icuswap, $tempDir, $icu4jDataDir);
    #convertData($icuDataDir."/coll/", $icuswap, $tempDir, $icu4jDataDir."/coll");
    createJar("$jarDir/jar", "icudata.jar", $tempDir, $icu4jDataDir);
    
    convertTestData($icuTestDataDir, $icuswap, $tempDir, $icu4jTestDataDir);
    createJar("$jarDir/jar", "testdata.jar", $tempDir, $icu4jTestDataDir);
    copyData($icu4jDir, $icu4jImpl, $icu4jDevDataDir, $tempDir);
}

#-----------------------------------------------------------------------
sub buildICU{
    local($platform, $icuRootDir, $icuTestDataDir) = @_;
    $icuSrcDir = $icuRootDir."/source";
    $icuSrcDataDir = $icuSrcDir."/data";
    
    chdir($icuSrcDir);
    # clean the data directories
    unlink($icuBuildDir."../"); 
    unlink($icuTestDataDir."../"); 
    
    if($platform eq "cygwin"){
        # make all in ICU
        cmd("make all");
        chdir($icuSrcDataDir);
        cmd("make uni-core-data");
        chdir($icuTestDataDir."../../");
        print($icuTestDataDir."../../\n");
        cmd("make");
    }elsif($platform eq "MSWin32"){
        #devenv.com $projectFileName \/build $configurationName > \"$cLogFile\" 2>&1
        cmd("devenv.com allinone/allinone.sln /useenv /build Debug");
        # build required data. this is required coz building icu will not build all the data
        chdir($icuSrcDataDir);
        cmd("NMAKE /f makedata.mak ICUMAKE=\"$icuSrcDataDir\" CFG=debug uni-core-data");

    }else{
        print "ERROR: Could not build ICU unknown platform $platform. \n";
        exit(-1);
    }
    
    chdir($cwd);    
}
#-----------------------------------------------------------------------
sub getVersion{
    my @list;
    opendir(DIR,$icuBuildDir);
    
    @list =  readdir(DIR);
    closedir(DIR);
    
    if(scalar(@list)>3){
        print("ERROR: More than 1 directory in build. Can't decide the version");
        exit(-1);
    }
    foreach $item (@list){
        next if($item eq "." || $item eq "..");
        my ($ver, $end) =$item =~ m/icudt(.*)(l|b|e)$/;
        return $ver,$end;
    }
}

#-----------------------------------------------------------------------
sub getPlatform{
    $platform = $^O;
    return $platform;
}
#-----------------------------------------------------------------------
sub createJar{
    local($jar, $jarFile, $tempDir, $dirToJar) = @_;
    chdir($tempDir);
    $command = "$jar cvf $jarFile -C $tempDir $dirToJar";
    cmd($command);
}
#-----------------------------------------------------------------------
sub checkPlatformEndianess {
    my $is_big_endian = unpack("h*", pack("s", 1)) =~ /01/;
    if ($is_big_endian) {
        return "b";
    }else{
        return "l";
    }
}
#-----------------------------------------------------------------------
sub copyData{
    local($icu4jDir, $icu4jImpl, $icu4jDevDataDir, $tempDir) =@_;
    print("Copying: $tempDir/icudata.jar to $icu4jDir/src/$icu4jImpl");
    copy("$tempDir/icudata.jar", "$icu4jDir/src/$icu4jImpl"); 
    print("Copying: $tempDir/testData.jar $icu4jDir/src/$icu4jDevDataDir");
    copy("$tempDir/testData.jar","$icu4jDir/src/$icu4jDevDataDir");
}
#-----------------------------------------------------------------------
sub convertData{
    local($icuDataDir, $icuswap, $tempDir, $icu4jDataDir)  =@_;
    my $dir = $tempDir."/".$icu4jDataDir;
    # create the temp directory
    mkpath("$tempDir/$icu4jDataDir");
    # cd to the temp directory
    chdir($tempDir);

    my @list;
    opendir(DIR,$icuDataDir);
    print $icuDataDir;
    @list =  readdir(DIR);
    closedir(DIR);
    print "{Command: $op*.*}\n";
    my $op = $icuswap;
    $i=0;
    # now convert
    foreach $item (@list){
        next if($item eq "." || $item eq "..");
        next if($item =~ /^t_.*$\.res/ ||$item =~ /^translit_.*$\.res/   || $item =~ /$\.cnv/ ||
               $item=~/$\.crs/ || $item=~ /$\.txt/ || $item=~ /^zoneinfo/  ||
               $item=~/icudata\.res/ || $item=~/$\.exp/ || $item=~/$\.lib/ || $item=~/$\.obj/ ||
               $item=~/cnvalias\.icu/ || $item=~/$\.lst/);
        if(-d "$icuDataDir/$item"){
            convertData("$icuDataDir/$item/", $icuswap, $tempDir, "$icu4jDataDir./$item/");
            next;
        }
        $command = $icuswap." $icuDataDir/$item $tempDir/$icu4jDataDir/$item";
        cmd($command);

    }
    chdir("..");
    print "\nDONE\n";
}
#-----------------------------------------------------------------------
sub convertTestData{
    local($icuDataDir, $icuswap, $tempDir, $icu4jDataDir)  =@_;
    my $dir = $tempDir."/".$icu4jDataDir;
    # create the temp directory
    mkpath("$tempDir/$icu4jDataDir");
    # cd to the temp directory
    chdir($tempDir);
    print "{Command: $op*.*}\n";
    my $op = $icuswap;
    my @list;
    opendir(DIR,$icuDataDir);
    print $icuDataDir;
    @list =  readdir(DIR);
    closedir(DIR);

    $i=0;
    # now convert
    foreach $item (@list){
        next if($item eq "." || $item eq "..");
        next if($item =~ /$\.cnv/ || item=~/$\.crs/ || $item=~ /$\.txt/ ||
                $item=~/$\.exp/ || $item=~/$\.lib/ || $item=~/$\.obj/ ||
                $item=~/$\.mak/ || $item=~/test\.icu/ || $item=~/$\.lst/);
        
        if($item =~ /^testdata_/){
            $file = $item;
            $file =~ s/testdata_//g;
            $command = "$icuswap $icuDataDir/$item $tempDir/$icu4jDataDir/$file";
            cmd($command);
        }

    }
    chdir("..");
    print "\nDONE\n";
}
#------------------------------------------------------------------------------------------------
sub cmd {
    my $cmd = shift;
    my $prompt = shift;
    $prompt = "Command: $cmd.." unless ($prompt);
    print $prompt."\n";
    system($cmd);
    my $exit_value  = $? >> 8;
    #my $signal_num  = $? & 127;
    #my $dumped_core = $? & 128;
    if ($exit_value == 0) {
        print "ok\n";
    } else {
        ++$errCount;
        print "ERROR ($exit_value)\n";
        exit(1);
    }
}
#-----------------------------------------------------------------------
sub usage {
    print << "END";
Usage:
gendtjar.pl
Options:
        --icu-root=<directory where icu4c lives>
        --jar=<directory where jar.exe lives>
        --icu4j-root=<directory>
        --version=<ICU4C version>
e.g:
gendtjar.pl --icu-root=\\work\\icu --jar=\\jdk1.4.1\\bin --icu4j-root=\\work\\icu4j --version=3.0
END
  exit(0);
}

