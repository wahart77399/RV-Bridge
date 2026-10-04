use strict;
use warnings;
use Cwd qw(abs_path);
use File::Basename qw(dirname);
use File::Temp qw(tempdir);

my $root = abs_path(dirname(__FILE__) . '/../..');
chdir $root or die "Cannot enter workspace: $!";
my $temporary = tempdir(CLEANUP => 1);
open my $input, '<', 'src/Packets/DGN.h' or die $!;
my $source = do {local $/; <$input>};
close $input;
$source =~ /(typedef enum\s*\{[\s\S]*?\}\s*RVC_DGN;)/ or die 'DGN enum not found';
open my $header, '>', "$temporary/DGN.h" or die $!;
print {$header} "#pragma once\n#include <cstdint>\n$1\n";
close $header;

my @command = (
    'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
    '-I', 'test/host/mocks', '-I', $temporary, '-I', 'src/Base',
    '-I', '.pio/libdeps/Release/ArduinoJson/src',
    'src/Base/LearnMode.cpp', 'src/Base/LearnRecord.cpp',
    'test/host/learning_failures.cpp', '-o', "$temporary/learning-failures"
);
my $result = system(@command);
if ($result == 0) {
    $result = system("$temporary/learning-failures");
}
if ($result == 0) {
    open my $factory, '<', 'src/Devices/DeviceFactory.cpp' or die $!;
    my $factorySource = do {local $/; <$factory>};
    close $factory;
    $factorySource =~ /(    uint32_t accessoryCount\([^\n]*\) \{[\s\S]*?\n    \})/ or die 'Accessory sizing helper not found';
    my $sizing = $1;
    $factorySource =~ /(bool DeviceFactory::assignStableAids\([^\n]*\) \{[\s\S]*?\n\})/ or die 'AID allocator not found';
    my $allocator = $1;
    open my $generated, '>', "$temporary/aid-production.cpp" or die $!;
    print {$generated} "#include \"DeviceFactory.h\"\n#include \"Preferences.h\"\nnamespace {\n$sizing\n}\n$allocator\n";
    close $generated;
    my @aidCommand = (
        'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
        '-I', 'test/host/mocks', '-I', $temporary,
        '-I', '.pio/libdeps/Release/ArduinoJson/src',
        "$temporary/aid-production.cpp", 'test/host/aid_failures.cpp',
        '-o', "$temporary/aid-failures"
    );
    $result = system(@aidCommand);
    if ($result == 0) {
        $result = system("$temporary/aid-failures");
    }
}
exit($result == 0 ? 0 : 1);