package require tcltest
package require Expect

# Save current terminal size
set stty_output [exec stty size]
scan $stty_output "%d %d" orig_lines orig_columns
# setting some large terminal width
stty columns 720

# Hook to determine if any of the tests failed.
# Sets a global variable exitCode to 1 if any test fails otherwise it is set to 0.
proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]
}

if {[info exists ::env(TESTS_DIR)]} {
    tcltest::configure -testdir "$env(TESTS_DIR)/interactive"
}

# run all interactive tests
tcltest::runAllTests

# Restore original terminal size
exec stty rows $orig_lines cols $orig_columns

exit $exitCode
