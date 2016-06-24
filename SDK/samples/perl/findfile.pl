use Win32::OLE;
use Win32::OLE::Variant;
my $session = Win32::OLE->new('MiKTeX.Session')
  or die "MiKTeX is not properly installed.\n";
my $path = Variant(VT_BSTR|VT_BYREF, '');
if ($session->FindFile($ARGV[0], $path)) {
  print "$path\n";
}
