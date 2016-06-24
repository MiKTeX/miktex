use Win32::OLE;
use Win32::OLE::Variant;
my $session = Win32::OLE->new('MiKTeX.Session')
  or die "MiKTeX is not properly installed.\n";
my $path = Variant(VT_BSTR|VT_BYREF, '');
if ($session->FindPkFile($ARGV[0], $ARGV[1], $ARGV[2], $path)) {
  print "$path\n";
}
