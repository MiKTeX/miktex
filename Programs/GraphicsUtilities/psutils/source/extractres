#!/usr/bin/env perl
# extractres: extract resources from PostScript file
# includeres: add resources to a PostScript file
#
# (c) Reuben Thomas 2012
# (c) Angus J. C. Duggan 1991-1997
# See file LICENSE for details.

use strict;
use warnings;

use File::Basename;
use Getopt::Long;

my $prog = basename($0);

# Resource extensions
my %extn = ("font" => ".pfa", "file" => ".ps", "procset" => ".ps",
            "pattern" => ".pat", "form" => ".frm", "encoding" => ".enc");
# Resource types
my %type = ("%%BeginFile:" => "file", "%%BeginProcSet:" => "procset",
            "%%BeginFont:" => "font");

sub filename {			# make filename for resource in @_
  my $name;
  foreach (@_) {		# sanitise name
    s/[!()\$\#*&\\\|\`\'\"\~\{\}\[\]\<\>\?]//g;
    $name .= $_;
  }
  $name =~ s@.*/@@;		# drop directories
  die "$prog: filename not found for resource ", join(" ", @_), "\n"
    if $name =~ /^$/;
  return $name;
}

sub extract {
  my %resources = ();          # list of resources included
  my %merge = ();              # list of resources extracted this time

  my $merge;
  unless (GetOptions('merge' => \$merge)) {
    print STDERR "Usage: $prog [-merge] [FILE]\n";
    exit 1;
  }

  my $prolog = "";
  my $body = "";
  my $resource = "";
  my $output = \$prolog;
  my $saveout;
  while (<>) {
    if (/^%%BeginResource:/ || /^%%BeginFont:/ || /^%%BeginProcSet:/) {
      my ($comment, @res) = split(/\s+/); # look at resource type
      my $type = defined($type{$comment}) ? $type{$comment} : shift(@res);
      my $name = filename(@res, $extn{$type}); # make file name
      $saveout = $output;
      if (!defined($resources{$name})) {
        $prolog .= "%%IncludeResource: $type " . join(" ", @res) . "\n";
        if (!-e $name) {
          open RES, ">$name" || die "$prog: can't write file `$name'\n";
          $resources{$name} = "";
          $merge{$name} = $merge;
          $output = \$resources{$name};
        } else {		# resource already exists
          close(RES);
          undef $output;
        }
      } elsif ($merge{$name}) {
        open RES, ">>$name" || die "$prog: can't append to file `$name'\n";
        $resources{$name} = "";
        $output = \$resources{$name};
      } else {			# resource already included
        undef $output;
      }
    } elsif (/^%%EndResource/ || /^%%EndFont/ || /^%%EndProcSet/) {
      if (defined $output) {
        $$output .= $_;
        print RES $$output;
      }
      $output = $saveout;
      next;
    } elsif ((/^%%EndProlog/ || /^%%BeginSetup/ || /^%%Page:/)) {
      $output = \$body;
    }
    $$output .= $_ if defined $output;
  }

  print $prolog . $body;
}

sub include {
  while (<>) {
    if (/^%%IncludeResource:/ || /^%%IncludeFont:/ || /^%%IncludeProcSet:/) {
      my ($comment, @res) = split(/\s+/);
      my $type = defined($type{$comment}) ? $type{$comment} : shift(@res);
      my $name = filename(@res);
      if (open(RES, $name) || open(RES, "$name$extn{$type}")) {
        print do { local $/; <RES> };
        close(RES);
      } else {
        print "%%IncludeResource: ", join(" ", $type, @res), "\n";
        print STDERR "$prog: resource `$name' not found\n";
      }
    } else {
      print $_;
    }
  }
}

# Perform function according to name
if ($prog eq "extractres") {
  extract();
} elsif ($prog eq "includeres") {
  include();
} else {
  die "$prog: I need to be called `extractres' or `includeres'\n";
}
