#!/usr/bin/perl


#make-appimage.pl: constructs an AppImage using the AppImageTooKit
#
#Usage:
#	$ ./make-appimage.pl version BIN:path/to/app [BIN:path/to/nextfileinbin [CONF:path/to/conffile ...]]
#
#Dependencies (must be in $PATH):
#	wget
#	appimagetool-x86_64.AppImage
#	rm, chmod 
#	cp (if appimage is run from a shell) 
#	zenity (if appimage is run from a .desktop or file reference in a GUI
#

use File::Spec;
use File::Copy "cp";

$arch = "x86_64";
$version = shift @ARGV;
my ($dest,$pat) = split ":", $ARGV[0];
$program = $pat;

#get application name, from $program:
($volume,$directory,$file) = File::Spec->splitpath($program);

$rootdir = "$file-$arch.AppDir";

printf "$file...\n";

#make directory structure:
mkdir $rootdir;
mkdir "$rootdir/usr";
mkdir "$rootdir/usr/bin";
mkdir "$rootdir/usr/lib";
mkdir "$rootdir/usr/share";
mkdir "$rootdir/usr/share/metainfo";
mkdir "$rootdir/usr/share/$file";


#copy each arg prepended with BIN: to bin:
printf "BIN:\n";
foreach $arg (@ARGV) {
	my ($dest,$pat) = split ":", $arg;
	next if ($dest ne "BIN");
	printf "\t$pat\n";
	my ($vol,$dir,$fil) = File::Spec->splitpath($pat);
	if (-d $pat) {
		mkdir "$rootdir/usr/bin/$fil";
		for my $dirfile (glob "$pat/*") {
			cp $dirfile, "$rootdir/usr/bin/$fil/";
		}
	}
	else {
		cp  $pat, "$rootdir/usr/bin/";
	}
}

#copy each arg prepended with DATA: to share/$program/:
printf "DATA:\n";
foreach $arg (@ARGV) {
	my ($dest,$pat) = split ":", $arg;
	next if ($dest ne "DATA");
	printf "\t$pat\n";
	my ($vol,$dir,$fil) = File::Spec->splitpath($pat);
	if (-d $pat) {
		mkdir "$rootdir/usr/share/$file/$fil";
		for my $dirfile (glob "$pat/*") {
			cp $dirfile, "$rootdir/usr/share/$file/$fil/";
		}
	}
	else {
		cp  $pat, "$rootdir/usr/share/$file/";
	}
}


#set executable permissions for each application in $rootdir/usr/bin/
foreach $fname (<$rootdir/usr/bin/*>) {
	$result=`file $fname |grep -c \"ELF\"`;
	chomp $result;

	if ($result ne "0") {
		chmod 0775, $fname;
	}
}




#fill the usr/lib directory:

#local excludes, if needed:
#$exclude{'libharfbuzz.so.0'} = "foo";

#$result = `wget https://raw.githubusercontent.com/AppImage/AppImages/master/excludelist`;
$result = `wget https://raw.githubusercontent.com/AppImage/pkg2appimage/master/excludelist`;

open INFILE, "excludelist";
while ($line = <INFILE>) {
	chomp $line;
	@file = split /\#/, $line;
	if ($file[0] ne "") {
		$exclude{$file[0]} = "foo";
	}
}
close INFILE; 

$result = `rm excludelist`;

#local 'includes', remove from excludelist:
delete $exclude{'libglib-2.0.so.0'};  #accommodate older rev in some Debian systems, thanks @jade_nl

print "\n";

@lines = `ldd $program`;

foreach $line (@lines) {
	@items = split /\s/, $line;
	if (exists $exclude{$items[1]}) {
		print "Excluded: $items[1]\n";
	}
	else {
		if ($items[3] ne "") {
			cp $items[3], "$rootdir/usr/lib/";
		}
	} 
}

$APPRUN = <<'END_APPRUN';
#!/bin/sh

#Establish the unpacked location of the appimage:
HERE="$(dirname "$(readlink -f "${0}")")"

#Alter the paths to first point to the appimage bin and lib:
export PATH="${HERE}"/usr/bin/:"${HERE}"/usr/sbin/:"${HERE}"/usr/games/:"${HERE}"/bin/:"${HERE}"/sbin/:"${PATH}"
export LD_LIBRARY_PATH="${HERE}"/usr/lib/:"${HERE}"/usr/lib/i386-linux-gnu/:"${HERE}"/usr/lib/x86_64-linux-gnu/:"${HERE}"/usr/lib32/:"${HERE}"/usr/lib64/:"${HERE}"/lib/:"${HERE}"/lib/i386-linux-gnu/:"${HERE}"/lib/x86_64-linux-gnu/:"${HERE}"/lib32/:"${HERE}"/lib64/:"${LD_LIBRARY_PATH}"

#exec logic to allow symbolic linking of $file, and other program names to the appimage.  Default if no link is $file.
if [ ! -z $APPIMAGE ] ; then
	BINARY_NAME=$(basename "$ARGV0")
	if [ -e "$HERE/usr/bin/$BINARY_NAME" ] ; then
		exec "$HERE/usr/bin/$BINARY_NAME" "$@"
	else
		exec "$HERE/usr/bin/#file" "$@"
	fi
else
	exec "$HERE/usr/bin/$file" "$@"
fi
END_APPRUN

$APPRUN =~ s/#file/$file/g;

open OUTFILE, ">$file-$arch.AppDir/AppRun";
print OUTFILE $APPRUN;
close OUTFILE;

chmod 0775, "$file-$arch.AppDir/AppRun";

$DESKTOP = <<"END_DESKTOP";
[Desktop Entry]
Name=$file
Exec=$file
Icon=$file
Type=Application
Categories=Graphics;
END_DESKTOP

open OUTFILE, ">$file-$arch.AppDir/$file.desktop";
print OUTFILE $DESKTOP;
close OUTFILE;

#hack move files from bin to their appropriate places, replace with META: and ICON: later...
cp "$file-$arch.AppDir/usr/bin/$file.xpm", "$file-$arch.AppDir/.";
cp "$file-$arch.AppDir/usr/bin/$file.appdata.xml", "$file-$arch.AppDir/usr/share/metainfo/.";

$result = `appimagetool-x86_64.AppImage --no-appstream $rootdir $file-$version-$arch.AppImage`;
$result = `rm -rf $rootdir`;


