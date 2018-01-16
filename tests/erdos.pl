#!/usr/bin/env perl

use strict;
use warnings;

my $n = 1000;
my @ei = ();
my @ej = ();
my $p = 0.5;

for(my $i = 0; $i < $n; $i++) {
    for(my $j = $i + 1; $j < $n; $j++) {
        my $r = rand(1);
        push(@ei, $i) if $r < $p;
        push(@ej, $j) if $r < $p;
    }
}

my $m = @ei;
print "$n $m\n";
for(my $i = 0; $i < $m; $i++) {
    print "$ei[$i] $ej[$i]\n";
}
