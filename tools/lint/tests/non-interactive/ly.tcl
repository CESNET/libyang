source [expr {[info exists ::env(TESTS_DIR)] ? "$env(TESTS_DIR)/common.tcl" : "../common.tcl"}]

# namespace of internal functions
namespace eval ly::private {
    namespace export *
}

# Run the process with arguments.
# Parameter cmd is a string with arguments.
# Parameter wrn is a flag. Set to 1 if stderr should be ignored.
# Returns a pair where the first is the return code and the second is the output.
proc ly::private::ly_exec {cmd {wrn ""}} {
    try {
        set results [exec -- $::env(YANGLINT) {*}$cmd]
        set status 0
    } trap CHILDSTATUS {results options} {
        # return code is not 0
        set status [lindex [dict get $options -errorcode] 2]
    } trap NONE results {
        if { $wrn == 1 } {
            set status 0
        } else {
            error "return code is 0 but something was written to stderr:\n$results\n"
        }
    } trap CHILDKILLED {results options} {
        set status [lindex [dict get $options -errorcode] 2]
        error "process was killed: $status"
    }
    list $status $results
}

# Internal function.
# Check the output with pattern.
# Parameter pattern is a regex or an exact string to match.
# Parameter msg is the output to check.
# Parameter 'opt' is optional. If contains '-ex', then the 'pattern' parameter is
# used as a simple string for exact matching of the output.
proc ly::private::output_check {pattern msg {opt ""}} {
    if { $opt eq "" } {
        expr {![regexp -- $pattern $msg]}
    } elseif { $opt eq "-ex" } {
        expr {![string equal "$pattern" $msg]}
    } else {
        global error_head
        error "$error_head unrecognized value of parameter 'opt'"
    }
}

# Execute yanglint with arguments and expect success.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex or an exact string to match.
# Parameter 'opt' is optional. If contains '-ex', then the 'pattern' parameter is
# used as a simple string for exact matching of the output.
proc ly_cmd {cmd {pattern ""} {opt ""}} {
    namespace import ly::private::*
    lassign [ly_exec $cmd] rc msg
    if { $rc != 0 } {
        error "unexpected return code $rc:\n$msg\n"
    }
    if { $pattern ne "" && [output_check $pattern $msg $opt] } {
        error "unexpected output:\n$msg\n"
    }
    return
}

# Execute yanglint with arguments and expect error.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex.
proc ly_cmd_err {cmd pattern} {
    namespace import ly::private::*
    lassign [ly_exec $cmd] rc msg
    if { $rc == 0 } {
        error "unexpected return code $rc"
    }
    if { [output_check $pattern $msg] } {
        error "unexpected output:\n$msg\n"
    }
    return
}

# Execute yanglint with arguments, expect warning in stderr but success.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex.
proc ly_cmd_wrn {cmd pattern} {
    namespace import ly::private::*
    lassign [ly_exec $cmd 1] rc msg
    if { $rc != 0 } {
        error "unexpected return code $rc:\n$msg\n"
    }
    if { [output_check $pattern $msg] } {
        error "unexpected output:\n$msg\n"
    }
    return
}

# Check if yanglint supports the specified option.
# Parameter opt is a option to be found.
# Return true if option is found otherwise false.
proc ly_opt_exists {opt} {
    namespace import ly::private::*
    lassign [ly_exec "--help"] rc msg
    if { $rc != 0 } {
        error "unexpected return code $rc:\n$msg\n"
    }
    if { [output_check $opt $msg] } {
        return false
    } else {
        return true
    }
}
