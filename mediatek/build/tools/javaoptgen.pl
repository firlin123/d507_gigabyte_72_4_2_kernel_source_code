#!usr/bin/perl
my $configFile = $ARGV[0];
my $optrConfigFile = $ARGV[1];
die "the file $configFile is NOT exsit\n" if ( ! -e $configFile);
die "the file $optrConfigFile is NOT exsit\n" if ( (! -e $optrConfigFile) && ($optrConfigFile ne "") );
open FILE, "<$configFile";
my %config_for;
while (<FILE>) {
	if (/^(\w+)\s*=\s*(\w+)/) {
		$config_for{$1} = $2;
	}
}
close FILE;
open FILE, "<$optrConfigFile";
while (<FILE>) {
        if (/^(\w+)\s*=\s*(\w+)/) {
                $config_for{$1} = $2;
        }
}
close FILE;


my $filedir = "mediatek/frameworks/common/src/com/mediatek/common/featureoption";
my $write_filename = "$filedir/FeatureOption.java";
my $input_file = "mediatek/build/tools/javaoption.pm";
open(INPUT,$input_file) or die "can not open $input_file:$!\n";
my %javaoption;
while(<INPUT>)
{
	chomp;
	next if(/^\#/);
	next if(/^\s*$/);
	if(/\s*(\w+)\s*/)
	{
                if ($javaoption{$1} == 1)
                {   
                        die "$1 already define in $input_file";
                } else {
                        push (@need_options,$1);
                        $javaoption{$1} = 1;
                }   
        }   
}


system("chmod u+w $write_filename") if (-e $write_filename);
system("mkdir -p $filedir") if ( ! -d "$filedir");
die "can NOT open $write_filename:$!" if ( ! open OUT_FILE, ">$write_filename");
print OUT_FILE "\/* generated by mediatek *\/\n\n";
print OUT_FILE "package com.mediatek.common.featureoption;\n";
print OUT_FILE "\npublic final class FeatureOption\n{\n";

#pre-parse dfo array start
my @dfoAll = ();
my @dfoSupport = split(/\s+/, $ENV{"DFO_NVRAM_SET"});
foreach my $dfoSet (@dfoSupport) {
    my $dfoSetName = $dfoSet."_VALUE";
    my @dfoValues = split(/\s+/, $ENV{"$dfoSetName"});
    foreach my $dfoValue (@dfoValues) {
        push(@dfoAll, $dfoValue);
    }
}

my @dfoArray = ();
foreach my $tempDfo (@dfoAll) {
    my $isFind = 0;
    #only eng load will enable dfo
    if ($ENV{"TARGET_BUILD_VARIANT"} eq "eng") {
        foreach my $isDfoSupport (@dfoSupport) {
            if ($ENV{$isDfoSupport} eq "yes") {
                my $dfoSupportName = $isDfoSupport."_VALUE";
                my @dfoValues = split(/\s+/, $ENV{"$dfoSupportName"});
                foreach my $dfoValue (@dfoValues) {
                    if ($tempDfo eq $dfoValue) {
                        $isFind = 1;
                        break;
                    }
                }

                if ($isFind == 1) {
                    break;
                }
            }
        }
    }

    if ($isFind == 1) {
        push(@dfoArray, $tempDfo);
    }
}
my %dfoHashArray;
@dfoHashArray{@dfoArray} = ();
#print "Enable: @dfoArray\n";
#pre-parse dfo array end

foreach my $option (@need_options) {
    # if option in  DFO_LIST THEN GEN FUNCTION CALL ELSE GEN AS NOMARL!
    if ($config_for{$option} eq "yes") {
        if (exists $dfoHashArray{$option}){
            &gen_java_file ($write_filename, $option, "DynFeatureOption.getBoolean(\"$option\")", "boolean");
        }else {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
    }
    elsif ($config_for{$option} eq "no") {
        if (exists $dfoHashArray{$option}) {
            &gen_java_file ($write_filename, $option, "DynFeatureOption.getBoolean(\"$option\")", "boolean");
        } else {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
    }
    elsif ($config_for{$option} =~/^[+-]?\d+$/)
    {
        if (exists $dfoHashArray{$option}) {
            &gen_java_file($write_filename,$option,"DynFeatureOption.getInt(\"$option\")", "int")
        } else {
            &gen_java_file($write_filename, $option, $config_for{$option}, "int");
        }
    }
    else
    {
	#add BUG_ID:DELB-176 liangyulong 20130618 (start)
 	if ($option eq "RGK_AIRPLANE_SWITCH_WIFIBLUETOOTH") {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
	#add BUG_ID:DELB-176 liangyulong 20130618 (end)
	#add BUG_ID:DELB-213 liangyulong 20130618 (start)
	elsif ($option eq "RGK_OTA_DELETE_SETTINGS_ABOUT_SYSTEMUPDATES") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	#add BUG_ID:DELB-213 liangyulong 20130618 (end)
        #add BUG_ID:JELY-2 daizhiyi 20130416 (start)
        elsif ($option eq "RGK_COOLPAD_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add BUG_ID:JELY-2 daizhiyi 20130416 (end)
        #add BUG_ID:JEEY-247 liuxianbang 20130523 (start)
        elsif($option eq "RGK_FLASH_SUPPORT") {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
        #add BUG_ID:JEEY-247 liuxianbang 20130523 (end)
		#add BUG_ID:JELY-578 zhushengui 20130527 (start)
		elsif($option eq "RGK_LANGUAGE_SET_BY_SIM_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false" , "boolean");
        }
		#add BUG_ID:JELY-578 zhushengui 20130527 (end)

        #add  guocl enable RGK_OTA FEATURE (start)
        elsif ($option eq "RGK_OTA_SUPPORT") {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
        #add  guocl enable RGK_OTA FEATURE (end)
        #add  guocl enable RGK_OTA FEATURE (start)
        elsif ($option eq "RGK_FOR_TCL_CUSTOM") {
            &gen_java_file ($write_filename, $option, "false","boolean");
        }
		elsif ($option eq "RGK_FOR_CUCC_CUSTOMER") {
            &gen_java_file ($write_filename, $option, "false","boolean");
		}
        #add  guocl enable RGK_OTA FEATURE (end)
        #add BUG_ID:DELB-108 qiujingyu 20130609 (start)
        elsif ($option eq "RGK_D208_LAVA_S1_AUDIO_CHANGE") {
            &gen_java_file ($write_filename, $option, "false","boolean");
        }
        #add BUG_ID:DELB-108 qiujingyu 20130609 (end)
        #add for XOLO daizhiyi 20130618 (start)
        elsif ($option eq "RGK_XOLO_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add for XOLO daizhiyi 20130618 (end)
        #add for MAXIMUS huruilong 20130625 (start)
        elsif ($option eq "RGK_MAXIMUS_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        elsif ($option eq "GMT_FOTA_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        elsif ($option eq "RGK_D208_GIGABYTE_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        elsif ($option eq "RGK_GIGABYTE_IOUI_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        elsif ($option eq "RGK_D620_GIGABYTE_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add for MAXIMUS huruilong 20130625 (end)
        #add for DAUL SIM IMEI zhanghongyu 20130625 (start)
        elsif ($option eq "RGK_DUALSIMIMEI_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add for DAUL SIM IMEI zhanghongyu 20130625 (end)
	#add BUG_ID:DELB-310 zhaolong 20130625 (start)
        elsif ($option eq "RGK_IP_DIAL") {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
        #add BUG_ID:DELB-310 zhaolong 20130625 (end)
        #add DLELTCLW-118 songqingming 20130701 (start)
        elsif ($option eq "RGK_CONTACTS_SIM_RECOGNIZED") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add DLELTCLW-118 songqingming 20130701 (end)
        #add DLELTCLW-418 songqingming 20130709 (start)
        elsif ($option eq "RGK_CUSTOM_SIM_COLOR_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add DLELTCLW-418 songqingming 20130709 (end)
	 #add MDEYW-28 liangyulong 20130702 (start)
        elsif ($option eq "RGK_IS_WIFISOUND") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	        #add MDEYW-28 liangyulong 20130702 (end)
		#Add, DELB-427, daihongyi, 20130703, start
		elsif ($option eq "RGK_POSITIVO_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
		#Add, DELB-427, daihongyi, 20130703, end
		#add BUG_ID:DELB-620 qiujingyu 20130709 (start)
		elsif ($option eq "RGK_CMCC_AUDIOPROFILE_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
		#add BUG_ID:DELB-620 qiujingyu 20130709 (end)
	#add  DLEL-251  qianyadong  20130725(start)
	elsif ($option eq "RGK_KARBONN_SUPPORT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	#add  DLEL-251  qianyadong  20130725(end)
        #add for default mobile data enabled daizhiyi 20130730 (start)
        elsif ($option eq "RGK_DEFAULT_MOBILE_DATA_ENABLED") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
        #add for default mobile data enabled daizhiyi 20130730 (end)
        #add BUG_ID:DELB-1134 by yb 20130730(start)
	elsif ($option eq "RGK_NGM_EMERGENCY") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        } 
        #add BUG_ID:DELB-1134 by yb 20130730(end)
	#add  DELB-1230  qianyadong  20130801(start)
	elsif ($option eq "RGK_MATCH_EIGHT") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	#add  DELB-1230  qianyadong  20130801(end)
	# add shengui.zhu DELB-1259 20130805 (start)
	elsif ($option eq "RGK_HOOK_DOUBLE_CLICK_EVENT_ENABLED") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	# add shengui.zhu DELB-1259 20130805 (end)
	# add zhaoguangyou DELB-1256 20130813 (start)
	elsif ($option eq "RGK_BOOT_SHUT_TONE") {
            &gen_java_file ($write_filename, $option, "false", "boolean");
        }
	# add zhaoguangyou DELB-1256 20130813 (end)
	# add for sales,liangyulong 20130814 (start)
	elsif ($option eq "RGK_SALES_SUPPORT") {
            &gen_java_file ($write_filename, $option, "true", "boolean");
        }
	# add for sales,liangyulong 20130814 (end)
        print "\"$option\" not match\n";
    }
}
print OUT_FILE "}\n";
close OUT_FILE;
sub gen_java_file {
	my ($filename, $option, $value, $type) = @_;
	print OUT_FILE "    /**\n     * check if $option is turned on or not\n     */\n";
	if ( $option eq "GEMINI") {
		print OUT_FILE "    public static final $type MTK_${option}_SUPPORT = $value;\n";
		print "public static final $type MTK_${option}_SUPPORT = $value\n";
	}
	else {
		print OUT_FILE "    public static final $type ${option} = $value;\n";
		print "public static final $type ${option} = $value\n";
	}
}
