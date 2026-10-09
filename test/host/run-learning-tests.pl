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

if (@ARGV && ($ARGV[0] eq '--rvc' || $ARGV[0] eq '--rvc-awning-bypass')) {
    open my $powerHeader, '<', 'src/Devices/Sensors/Power/PowerSensor.h' or die $!;
    my $powerSource = do {local $/; <$powerHeader>};
    close $powerHeader;
    $powerSource =~ /(typedef enum[\s\S]*?)\nclass PowerSensorView;/ or die 'Power constants not found';
    open my $constants, '>', "$temporary/power-constants.h" or die $!;
    print {$constants} $1;
    close $constants;
    open my $inverterHeader, '<', 'src/Devices/Sensors/Power/Inverter.h' or die $!;
    my $inverterSource = do {local $/; <$inverterHeader>};
    close $inverterHeader;
    $inverterSource =~ /(enum class InverterStatus[\s\S]*?\n\};)/ or die 'Inverter states not found';
    open my $inverterStates, '>', "$temporary/inverter-states.h" or die $!;
    print {$inverterStates} "$1\n";
    close $inverterStates;
    $inverterSource =~ /(const uint8_t INVERTER_INVALID[\s\S]*?)constexpr uint8_t INVERTER_TEMPERATURES/ or die 'Inverter constants not found';
    my $inverterConstants = $1;
    open my $production, '>', "$temporary/rvc-production.inc" or die $!;
    print {$production} $inverterConstants;
    open my $tankHeader, '<', 'src/Devices/Sensors/Tanks.h' or die $!;
    my $tankSource = do {local $/; <$tankHeader>};
    close $tankHeader;
    open my $tankMethods, '>', "$temporary/tank-methods.inc" or die $!;
    for my $method ('size', 'level') {
        $tankSource =~ /(^        uint16_t \Q$method\E\([\s\S]*?^        \})/m or die "Tank $method not found";
        print {$tankMethods} "$1\n";
    }
    close $tankMethods;
    open my $chassisHeader, '<', 'src/Devices/Sensors/ChassisMobility.h' or die $!;
    my $chassisSource = do {local $/; <$chassisHeader>};
    close $chassisHeader;
    open my $chassisConstants, '>', "$temporary/chassis-constants.inc" or die $!;
    my @chassisConstants = ($chassisSource =~ /^(    static constexpr[^\n]+;)/mg);
    print {$chassisConstants} join("\n", @chassisConstants), "\n";
    close $chassisConstants;
    open my $atsDefinitions, '<', 'src/Devices/Sensors/Power/ATS_Definitions.h' or die $!;
    my $atsConstantsSource = do {local $/; <$atsDefinitions>};
    close $atsDefinitions;
    $atsConstantsSource =~ s/^#include[^\n]*\n//mg;
    open my $atsConstants, '>', "$temporary/ats-constants.h" or die $!;
    print {$atsConstants} $atsConstantsSource;
    close $atsConstants;
    open my $atsHeader, '<', 'src/Devices/Sensors/Power/AutomaticTransferSwitch.h' or die $!;
    my $atsSource = do {local $/; <$atsHeader>};
    close $atsHeader;
    open my $atsMethods, '>', "$temporary/ats-methods.inc" or die $!;
    for my $method ('byte0', 'lineOf', 'ioOf') {
        $atsSource =~ /(^        (?:static )?uint8_t \Q$method\E\([\s\S]*?^        \})/m or die "ATS $method not found";
        print {$atsMethods} "$1\n";
    }
    close $atsMethods;
    open my $batteryDefinitions, '<', 'src/Devices/Sensors/Power/BatteryDefinitions.h' or die $!;
    my $batteryConstantsSource = do {local $/; <$batteryDefinitions>};
    close $batteryDefinitions;
    $batteryConstantsSource =~ s/^#include[^\n]*\n//mg;
    open my $batteryConstants, '>', "$temporary/battery-constants.h" or die $!;
    print {$batteryConstants} $batteryConstantsSource;
    close $batteryConstants;
    open my $batteryHeader, '<', 'src/Devices/Sensors/Power/Battery.h' or die $!;
    my $batterySource = do {local $/; <$batteryHeader>};
    close $batteryHeader;
    open my $batteryMethods, '>', "$temporary/battery-methods.inc" or die $!;
    for my $method ('rd_le32', 'directCurrentVoltage', 'directCurrentAmperage', 'level', 'rmsRipple') {
        $batterySource =~ /(^        [^\n]*\b\Q$method\E\([^\n]*\)[^\n]*\{[\s\S]*?^        \})/m or die "Battery $method not found";
        print {$batteryMethods} "$1\n";
    }
    close $batteryMethods;
    open my $batteryView, '<', 'src/Devices/Sensors/Power/BatteryView.h' or die $!;
    my $batteryViewSource = do {local $/; <$batteryView>};
    close $batteryView;
    open my $batteryViewMethods, '>', "$temporary/battery-view-methods.inc" or die $!;
    for my $method ('setDCVoltage', 'setRMSRipple') {
        $batteryViewSource =~ /(^            void \Q$method\E[^\n]*)/m or die "Battery $method publisher not found";
        print {$batteryViewMethods} "$1\n";
    }
    close $batteryViewMethods;
    open my $chargerHeader, '<', 'src/Devices/Sensors/Power/Charger.h' or die $!;
    my $chargerSource = do {local $/; <$chargerHeader>};
    close $chargerHeader;
    $chargerSource =~ /(enum class ChargerOperatingState[\s\S]*?\n\};)/ or die 'Charger states not found';
    open my $chargerStates, '>', "$temporary/charger-states.h" or die $!;
    print {$chargerStates} "$1\n";
    close $chargerStates;
    open my $chargerConstants, '>', "$temporary/charger-constants.inc" or die $!;
    my @chargerConstants = ($chargerSource =~ /^(    static constexpr[^\n]+;)/mg);
    print {$chargerConstants} join("\n", @chargerConstants), "\n";
    close $chargerConstants;
    open my $configTypes, '<', 'src/Devices/ConfigTypes.h' or die $!;
    my $configSource = do {local $/; <$configTypes>};
    close $configTypes;
    $configSource =~ /(^    bool findCoverTiming\([\s\S]*?^    \})/m or die 'Coach timing lookup not found';
    open my $coachMethods, '>', "$temporary/coach-methods.inc" or die $!;
    print {$coachMethods} "$1\n";
    close $coachMethods;
    my %methods = (
        PowerSensor => [qw(getACPointValue validateVolts validateAmps setData clearReadings rmsVoltage rmsCurrent isOpenGroundFault isOpenNeutralFault isReversePolarityFault isGroundCurrentFault)],
        AutomaticTransferSwitch => [qw(setData)],
        Charger => [qw(Charger copyBuffer setData operatingStateName lineOf ioOf rawOperatingStateByte operatingState rawMeasuredChargeVoltage measuredChargeVoltage rawMeasuredChargeCurrent measuredChargeCurrent)],
        Inverter => [qw(statusName lineOf ioOf)],
        Tanks => [qw(levelPercent)],
        ChassisMobility => [qw(statusReceived hasValidStatus isBrakeEngaged isMoving isParked applyStatus)]
    );
    for my $class (sort keys %methods) {
        my $path = $class eq 'Tanks' || $class eq 'ChassisMobility'
            ? "src/Devices/Sensors/$class.cpp" : "src/Devices/Sensors/Power/$class.cpp";
        open my $model, '<', $path or die $!;
        my $modelSource = do {local $/; <$model>};
        close $model;
        for my $method (@{$methods{$class}}) {
            my @definitions = ($modelSource =~ /(^[^\n]*\Q$class\E::\Q$method\E\([\s\S]*?^\})/mg);
            @definitions or die "${class}::$method not found";
            print {$production} join("\n", @definitions), "\n";
        }
    }
    open my $cover, '<', 'src/Devices/Controllable/Switches/CoverView.cpp' or die $!;
    my $coverSource = do {local $/; <$cover>};
    close $cover;
    $coverSource =~ s{/\*.*?\*/}{}gs;
    $coverSource =~ /(^bool CoverView::canOperate\([\s\S]*?^\})/m or die 'Cover interlock policy not found';
    print {$production} "$1\n";
    for my $method ('publishAwningStatus', 'moveTo', 'stopMoving', 'isReadyToStop', 'update', 'requestPosition', 'requestFullExtend', 'requestFullRetract') {
        $coverSource =~ /(^[^\n]*CoverView::CoverController::\Q$method\E\([\s\S]*?^\})/m or die "Cover controller $method not found";
        print {$production} "$1\n";
    }
    $coverSource =~ /(^boolean CoverView::CoverExtendRetractController::update\([\s\S]*?^\})/m or die 'Cover Out/In update not found';
    print {$production} "$1\n";
    open my $routing, '<', 'src/Devices/DeviceFactory.cpp' or die $!;
    my $routingSource = do {local $/; <$routing>};
    close $routing;
    $routingSource =~ /(^bool DeviceFactory::instanceFromData\([\s\S]*?^\})/m or die 'Factory instance decoder not found';
    print {$production} "$1\n";
    open my $chargerView, '<', 'src/Devices/Sensors/Power/ChargerView.cpp' or die $!;
    my $chargerViewSource = do {local $/; <$chargerView>};
    close $chargerView;
    $chargerViewSource =~ /(^bool ChargerView::updateView\([\s\S]*?^\})/m or die 'Charger view update not found';
    print {$production} "$1\n";
    close $production;
    my @diagnosticFlags = $ARGV[0] eq '--rvc-awning-bypass' ? ('-DSMARTCOACH_AWNING_PARK_BYPASS=1') : ();
    my $result = system('clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra', @diagnosticFlags, '-I', $temporary, '-I', 'test/host/mocks',
                        'test/host/rvc_payloads.cpp', '-o', "$temporary/rvc-payloads");
    $result = system("$temporary/rvc-payloads") if $result == 0;
    if ($result == 0 && $ARGV[0] eq '--rvc') {
        $result = system($^X, $0, '--rvc-awning-bypass');
    }
    exit($result == 0 ? 0 : 1);
}

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
    my @diagnosticsCommand = (
        'clang++', '-arch', 'x86_64', '-std=c++17', '-Wall', '-Wextra',
        '-I', 'test/host/mocks', '-I', $temporary, '-I', 'src/Base',
        '-I', '.pio/libdeps/Release/ArduinoJson/src',
        'src/Base/BridgeDiagnostics.cpp', 'src/Base/LearnRecord.cpp', 'test/host/diagnostics_reports.cpp',
        '-o', "$temporary/diagnostics-reports"
    );
    $result = system(@diagnosticsCommand);
    if ($result == 0) {
        $result = system("$temporary/diagnostics-reports");
    }
    if ($result == 0) {
        $result = system("$temporary/diagnostics-reports", '--unmapped');
    }
    if ($result == 0) {
        $result = system("$temporary/diagnostics-reports", '--unmapped-overflow');
    }
    if ($result == 0) {
        $result = system("$temporary/diagnostics-reports", '--live-scale');
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
        'src/Base/SmartCoachWeb.cpp', 'src/Base/EmailReports.cpp', 'src/Base/BridgeDiagnostics.cpp', 'src/Base/LearnMode.cpp', 'src/Base/LearnRecord.cpp',
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
if ($result == 0) {
    $result = system($^X, $0, '--rvc');
}
exit($result == 0 ? 0 : 1);