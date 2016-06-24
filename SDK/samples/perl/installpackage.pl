use Win32::OLE;
my $packageManager = Win32::OLE->new('MiKTeX.PackageManager')
  or die "MiKTeX is not properly installed.\n";
$installer = $packageManager->CreateInstaller();
$installer->Add ("$ARGV[0]", 1);
$installer->InstallRemove;
