#!/usr/bin/env perl
use strict;
use warnings;
use 5.010001;

use Audio::Wav;

my $fn = shift;
die "supply a wav file: sample[N].wav" unless defined $fn && -e $fn;

my $N = 'N';
if ($fn =~ /sample(\d+)/) {
    $N = $1;
}


my $wav = Audio::Wav->new;
my $read = $wav->read($fn);

my $num_samples = $read->length_samples;

say "#define NUM_SAMPLE${N}_ELEMENTS $num_samples";
say "const int16_t sample${N}\[NUM_SAMPLE${N}_ELEMENTS\] = {";

my $total = 0;
my $col = 0;
my $n = 0;
while (defined (my $b = $read->read_raw_samples(1))) {
    # This 's' is for 16 bit
    my $v = unpack('s*', $b);
    print $v;
    if ($v >= 65535) {
        die "$v too large. Expected unsigned 16 bit values.";
    }
    $total++;
    if ($total < $num_samples) {
        print ",";
    }
    if (++$col >= 8) {
        print "\n";
        $col = 0;
    }
}
say "};";

print STDERR "Counted: $total ... vs $num_samples\n" if $total != $num_samples;
