package require Expect

source [expr {[info exists ::env(TESTS_DIR)] ? "$env(TESTS_DIR)/common.tcl" : "../common.tcl"}]

# set the timeout to 1 second
set timeout 1
# prompt of yanglint
set prompt "> "
# turn off dialog between expect and yanglint
log_user 0
# setting some large terminal width
stty columns 720

variable ly_setup {
    spawn $::env(YANGLINT)
    ly_skip_warnings
    # Searchpath is set, so modules can be loaded via the 'load' command.
    ly_cmd "searchpath $::env(YANG_MODULES_DIR)"
}

variable ly_cleanup {
    ly_exit
}

# detection on eof and timeout will be on every expect command
expect_after {
    eof {
        global error_head
        error "$error_head unexpected termination"
    } timeout {
        global error_head
        error "$error_head timeout"
    }
}

# Run commands from command line
tcltest::loadTestedCommands

# namespace of internal functions
namespace eval ly::private {}

# Skip no dir and/or no history warnings and prompt.
proc ly_skip_warnings {} {
    global prompt
    expect -re "(YANGLINT.*)*$prompt" {}
}

# Send command 'cmd' to the process, then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex or an exact string to match. If is not specified, only prompt assumed afterwards.
# It must not contain a prompt. There can be an '$' character at the end of the pattern, in which case the regex
# matches the characters before the prompt.
# Parameter 'opt' can contain:
#   -ex     has a similar meaning to the expect command. The 'pattern' parameter is used as a simple string
#           for exact matching of the output. So 'pattern' is not a regular expression but some characters
#           must still be escaped, eg ][.
proc ly_cmd {cmd {pattern ""} {opt ""}} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    if { $pattern eq "" } {
        # command without output
        expect ^$prompt
        return
    }

    # definition of an expression that matches failure
    set failure_pattern "\r\n${prompt}$"

    if { $opt eq "" && [string index $pattern end] eq "$"} {
        # check output by regular expression
        # It was explicitly specified how the expression should end.
        set pattern [string replace $pattern end end]
        expect {
            -re "${pattern}\r\n${prompt}$" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } elseif { $opt eq "" } {
        # check output by regular expression
        expect {
            -re "${pattern}.*\r\n${prompt}$" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } elseif { $opt eq "-ex" } {
        # check output by exact matching
        expect {
            -ex "${pattern}\r\n${prompt}" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } else {
        global error_head
        error "$error_head unrecognized value of parameter 'opt'"
    }
}

# Send command 'cmd' to the process, expect error header and then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex. It must not contain a prompt.
proc ly_cmd_err {cmd pattern} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    expect {
        -re "YANGLINT\\\[E\\\]: .*${pattern}.*\r\n${prompt}$" {}
        -re "libyang\\\[\[0-9]+\\\]: .*${pattern}.*\r\n${prompt}$" {}
        -re "\r\n${prompt}$" {
            error "unexpected output:\n$expect_out(buffer)"
        }
    }
}

# Send command 'cmd' to the process, expect warning header and then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex. It must not contain a prompt.
proc ly_cmd_wrn {cmd pattern} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    expect {
        -re "YANGLINT\\\[W\\\]: .*${pattern}.*\r\n${prompt}$" {}
        -re "\r\n${prompt}$" {
            error "unexpected output:\n$expect_out(buffer)"
        }
    }
}

# Whatever is written is sent, output is ignored and then another prompt is expected.
# Parameter cmd is optional and any output is ignored.
proc ly_ignore {{cmd ""}} {
    global prompt

    send "${cmd}\r"
    expect -re "$prompt$"
}

# Send a completion request and check if the anchored regex output matches.
proc ly_completion {input output} {
    global prompt

    send -- "${input}\t"
    # expecting echoing input, output and 10 terminal control characters
    expect -re "^${input}\r> ${output}.*\r.*$"
}

# Send a completion request and check if the anchored regex hint options match.
proc ly_hint {input prev_input hints} {
    set output {}
    foreach i $hints {
        # each element might have some number of spaces and CRLF around it
        append output "${i} *(?:\\r\\n)?"
    }

    send -- "${input}\t"
    # expecting the hints, previous input from which the hints were generated
    # and some number of terminal control characters
    expect -re "^\r\n${output}\r> ${prev_input}.*\r.*$"
}

# Send 'exit' and wait for eof.
proc ly_exit {} {
    send "exit\r"
    expect eof
}
