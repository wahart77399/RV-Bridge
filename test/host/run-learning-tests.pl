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

open my $factory, '<', 'src/Devices/DeviceFactory.cpp' or die $!;
my $factorySource = do {local $/; <$factory>};
close $factory;
my @supported = ($factorySource =~ /creators\["(\w+)"\]/g);
open my $types, '>', "$temporary/SupportedDeviceTypes.h" or die $!;
print {$types} "#pragma once\ninline constexpr const char* supportedDeviceTypes[] = {", join(',', map {'"' . $_ . '"'} @supported), "};\n";
close $types;

$factorySource =~ /(    uint32_t accessoryCount\([^\n]*\) \{[\s\S]*?\n    \})/ or die 'Accessory sizing helper not found';
my $sizing = $1;
$factorySource =~ /(    bool replaceJsonFile\([\s\S]*?\n    \})/ or die 'Persistence helper not found';
my $persistence = $1;
my @functions;
for my $name ('assignStableAids', 'validateAndReserveConfiguration', 'saveChassisAid') {
    $factorySource =~ /(bool DeviceFactory::\Q$name\E\([^\n]*\) \{[\s\S]*?\n\})/ or die "$name not found";
    push @functions, $1;
}
open my $generated, '>', "$temporary/aid-production.cpp" or die $!;
print {$generated} "#include \"DeviceFactory.h\"\n#include \"Preferences.h\"\nnamespace {\n$sizing\n$persistence\n}\n", join("\n", @functions), "\n";
close $generated;

my @command = (
    'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
    '-I', 'test/host/mocks', '-I', $temporary, '-I', 'src/Base',
    '-I', '.pio/libdeps/Release/ArduinoJson/src',
    'src/Base/LearnMode.cpp', 'src/Base/LearnRecord.cpp', 'src/Base/EmailReports.cpp', "$temporary/aid-production.cpp",
    'test/host/learning_failures.cpp', '-o', "$temporary/learning-failures"
);
my $result = system(@command);
if ($result == 0) {
    $result = system("$temporary/learning-failures");
}
if ($result == 0) {
    my @emailCommand = (
        'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
        '-I', 'test/host/mocks', '-I', 'src/Base',
        '-I', '.pio/libdeps/Release/ArduinoJson/src',
        'src/Base/EmailReports.cpp', 'test/host/email_reports.cpp',
        '-o', "$temporary/email-reports"
    );
    $result = system(@emailCommand);
    if ($result == 0) {
        $result = system("$temporary/email-reports");
    }
}
if ($result == 0) {
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
if ($result == 0) {
    my @webCommand = (
        'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
        '-I', 'test/host/mocks', '-I', $temporary, '-I', 'src/Base',
        '-I', '.pio/libdeps/Release/ArduinoJson/src',
        'src/Base/SmartCoachWeb.cpp', 'src/Base/EmailReports.cpp', 'src/Base/LearnMode.cpp', 'src/Base/LearnRecord.cpp',
        "$temporary/aid-production.cpp", 'test/host/web_review_failures.cpp',
        '-o', "$temporary/web-review"
    );
    $result = system(@webCommand);
    if ($result == 0) {
        $result = system("$temporary/web-review");
    }
    if ($result == 0 && @ARGV && $ARGV[0] eq '--serve') {
        $result = system('ruby', 'test/host/portal_preview.rb', "$temporary/web-review");
    }
}
exit($result == 0 ? 0 : 1);