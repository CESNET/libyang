package require tcltest
package require Expect

# Save current terminal size, but only if we are in an environtment with a tty (i.g. may not be present in a CI)
set has_tty 0
if {![catch {exec stty size} stty_output]} {
    scan $stty_output "%d %d" orig_lines orig_columns
    set has_tty 1
}

if {$has_tty} {
    # setting some large terminal width
    stty columns 720
}

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

if {$has_tty} {
    # Restore original terminal size
    exec stty rows $orig_lines cols $orig_columns
}

exit $exitCode
