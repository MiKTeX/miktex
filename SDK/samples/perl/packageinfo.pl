use Win32::OLE;
use Win32::OLE::Variant;
my $packageManager = Win32::OLE->new('MiKTeX.PackageManager')
  or die "MiKTeX is not properly installed.\n";
my $packageInfo = $packageManager->GetPackageInfo("$ARGV[0]");
print "$packageInfo->{'deploymentName'}\n";
print "$packageInfo->{'title'}\n";
if ($packageInfo->{'isInstalled'}) {
  my $timeInstalled = Variant(VT_DATE|VT_BYREF,
			      $packageInfo->{'timeInstalled'});
  print "installed on $timeInstalled\n";
} else {
  print "not installed\n";
}
