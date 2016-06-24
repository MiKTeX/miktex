#!/usr/bin/env perl
#
# psjoin - concatenate PostScript files
#
# version 0.2, 2002-07-18
# version 0.3, 2003-11-30
# version 0.31, 2013-10-20
#
# versions up to 0.3 by Tom Sato <VEF00200@nifty.ne.jp>, http://homepage3.nifty.com/tsato/
# versions from 0.31 by Reuben Thomas <rrt@sc3d.org>

use strict;
use warnings;

use Getopt::Long;

my $version = 0.31;

sub usage {
    my ($exit_code) = @_;
    print STDERR <<END;
Usage: psjoin [OPTION...] FILENAME...
Concatenate PostScript files

  --even, -a    force each file to take an even number of pages
  --save, -s    try to close unclosed save operators
  --strip, -p   do not strip prolog or trailer from input files
  --help, -h    print this help, then exit
  --version,-V  print version information and exit
END
    exit $exit_code;
}

my $force_even = 0;
my $force_save = 0;
my $dont_strip = 0;
my $save = "save %psjoin\n";
my $restore = "restore %psjoin\n";
my ($help_flag, $version_flag);

GetOptions(
  "even|a" => \$force_even,
  "save|s" => \$force_save,
  "strip|p" => \$dont_strip,
  "help|h" => \$help_flag,
  "version|V" => \$version_flag,
 ) or usage(1);

if ($help_flag) {
    usage(0);
} elsif ($version_flag) {
    print "psjoin (version $version)\n";
    print "(c) Tom Sato <VEF00200\@nifty.ne.jp> and Reuben Thomas <rrt\@sc3d.org>\n";
    exit 0;
}

if ($force_save) {
    $save = "/#psjoin-save# save def %psjoin\n";
    $restore = "#psjoin-save# restore %psjoin\n";
}
my (@prolog, $prolog_inx, @trailer, @comments, @pages);
if ($dont_strip) {
    $prolog_inx = 9999;
    $prolog[$prolog_inx] = "% [ psjoin: don't strip ]\n";
    $trailer[$prolog_inx] = "% [ psjoin: don't strip ]\n";
} else {
    for (my $i = 0; $i <= $#ARGV; $i++) {
	open(IN, $ARGV[$i]) || die "$0: can't open \"$ARGV[$i]\" ($!)";

	my $in_comment = 1;
	my $in_prolog = 1;
	my $in_trailer = 0;
	$comments[$i] = "";
	$prolog[$i] = "";
	$trailer[$i] = "";
	$pages[$i] = 0;
	while (<IN>) {
	    next if /^%%BeginDocument/ .. /^%%EndDocument/;

	    if ($in_comment) {
		next if /^%!PS-Adobe-/;
		next if /^%%Title/;
		next if /^%%Pages/;
		next if /^%%Creator/;
		$in_comment = 0 if /^%%EndComments/;
		$comments[$i] .= $_;
		next;
	    } elsif ($in_prolog) {
		if (/^%%Page:/) {
		    $in_prolog = 0;
		} else {
		    $prolog[$i] .= $_;
		    next;
		}
	    }

	    $in_trailer = 1 if /^%%Trailer/;
	    if ($in_trailer) {
		$trailer[$i] .= $_;
		next;
	    }

	    $pages[$i]++ if /^%%Page:/;
	}
	close(IN);

	if ($prolog[$i]) {
	    for (my $j = 0; $j < $i; $j++) {
		if ($prolog[$j] eq $prolog[$i]) {
		    $pages[$j] += $pages[$i];
		    last;
		}
	    }
	}
    }

    my $largest = 0;
    $prolog_inx = 0;
    for (my $i = 0; $i <= $#ARGV; $i++) {
	my $size = length($prolog[$i]) * $pages[$i];
	if ($largest < $size) {
	    $largest = $size;
	    $prolog_inx = $i;
	}
    }
}

print <<END;
%!PS-Adobe-3.0
%%Title: @ARGV
%%Creator: psjoin $version
%%Pages: (atend)
END
print $comments[$prolog_inx];

print "\n$prolog[$prolog_inx]";
for (my $i = 0; $i <= $#ARGV; $i++) {
    $prolog[$i] =~ s/^%%/% %%/;
    $prolog[$i] =~ s/\n%%/\n% %%/g;
    $trailer[$i] =~ s/^%%/% %%/;
    $trailer[$i] =~ s/\n%%/\n% %%/g;
}

my $total_pages = 0;
for (my $i = 0; $i <= $#ARGV; $i++) {
    print "\n% [ psjoin: file = $ARGV[$i] ]\n";
    if ($prolog[$i] ne $prolog[$prolog_inx]) {
        print "% [ psjoin: Prolog/Trailer will be inserted in each page ]\n";
    } else {
        print "% [ psjoin: common Prolog/Trailer will be used ]\n";
    }

    my $in_comment = 1 if !$dont_strip;
    my $in_prolog = 1 if !$dont_strip;
    my $in_trailer = 0;
    my $saved = 0;
    my $pages = 0;

    open(IN, $ARGV[$i]) || die "$0: can't open \"$ARGV[$i]\" ($!)";
    while (<IN>) {
        if (/^%%BeginDocument/ .. /^%%EndDocument/) {
            # s/^(%[%!])/% \1/;
            print $_;
        } else {
            if ($in_comment) {
                $in_comment = 0 if /^%%EndComments/;
            } elsif ($in_prolog) {
                if (/^%%Page:/) {
                    $in_prolog = 0;
                } else {
                    next;
                }
            }
            $in_trailer = 1 if !$dont_strip && /^%%Trailer/;
            next if $in_trailer;

            if (/^%%Page:/) {
                if ($saved) {
                    print $trailer[$i];
		    print $restore;
                    $saved = 0;
                }

                $pages++;
                $total_pages++;
                print "\n";
                print "%%Page: ($i-$pages) $total_pages\n";
                if ($prolog[$i] ne $prolog[$prolog_inx]) {
		    print $save;
                    print $prolog[$i];
                    $saved = 1;
                } elsif ($force_save) {
		    print $save;
		}
            } else {
                s/^(%[%!])/% $1/;
                print $_;
            }
        }
    }
    close(IN);

    if ($force_even && $pages % 2 != 0) {
	$pages++;
	$total_pages++;
        print <<END;

%%Page: ($i-E) $total_pages
% [ psjoin: empty page inserted to force even pages ]
showpage
END
    }

    print $trailer[$i] if $saved;
    print $restore if $saved || $force_save;
}

print "\n%%Trailer\n";
print $trailer[$prolog_inx];
print "\n%%Pages: $total_pages\n%%EOF";
