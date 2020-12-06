#!/usr/bin/env tclsh

# https://wiki.tcl-lang.org/page/Simple+Server%2FClient+Sockets

set Port 8023
set Debug 0

namespace eval server {}
namespace eval client {}

proc log {lvl msg} {
    global Debug
    if {$lvl eq "debug" && $Debug == 0} {return}
    puts stdout "\[$lvl\] $msg"
}

proc server::ConnectionHandler {chan addr port} {
    log debug [info level 0]
    variable Clients
    if {[catch {
        fconfigure $chan -blocking 0 -buffering line -translation binary -eofchar {}
        fileevent $chan readable [list [namespace current]::ClientReader $chan]
    } err]} {
        log error "Error configuring channel $chan from ${addr}:${port}, $err"
        catch {close $chan}
        return
    }
    set Clients($chan) [list $addr $port]
    log info "Client $Clients($chan) | Connected"
}

proc server::ClientReader chan {
    log debug [info level 0]
    variable Clients
    if {[catch {read -nonewline $chan} data]} {
        catch {close $chan}
        log error "Client $Clients($chan) | Read Error: $data"
        log debug $::errorInfo
        log info "Client $Clients($chan) | Dropping client"
        array unset Clients $chan
        return
    }
    if {[eof $chan]} {
        catch {close $chan}
        log debug EOF
        log info "Client $Clients($chan) | Disconnected"
        array unset Clients $chan
        return
    }

    if {$data eq ""} {return}
    log info "Client $Clients($chan) | Says '$data'"

    # Send data to all other connected clients
    foreach c [lsearch -all -inline -not [array names Clients] $chan] {
        log debug "Sending to $c"
        if {[catch {puts $c $data} err]} {
            log error "Client $Clients($c) | Error sending data: $err"
            log debug $::errorInfo
        }
    }
}

proc client::ReadHandler chan {
    log debug [info level 0]
    if {[catch {read -nonewline $chan} data]} {
        log error "Read Error: $data"
        log debug $::errorInfo
        log info "Disconnecting from service"
        catch {close $chan}
        exit -1
    }
    if {[eof $chan]} {
        log error "Connection to service lost"
        catch {close $chan}
        exit -1
    }

    puts -nonewline stdout $data
}

proc client::InputHandler chan {
    log debug [info level 0]

    if {[catch {read -nonewline stdin} data]} {
        log error "Stdin Read Error: $data"
        log debug $::errorInfo
        log info "Disconnecting from service"
        catch {close $chan}
        exit -1
    }
    if {[eof stdin]} {
        log error "Stdin EOF"
        log info "Disconnecting from service"
        catch {close $chan}
        exit -1
    }

    # Send the input to the service
    log debug "stdin got '$data'"
    if {[catch {puts $chan $data} err]} {
        log error "Error Sending to service: $err"
        log debug $::errorInfo
        catch {close $chan}
        exit -1
    }
}

if {[lsearch $argv -debug] != -1} {
    set Debug 1
    log info "Debugging messages ON"
}

if {[lsearch $argv -server] != -1} {
    puts "Starting partyline server on port $Port"
    socket -server server::ConnectionHandler $Port
    vwait forever
} else {
    log info "Connecting to partyline on port $Port"
    set chan [socket localhost $Port]
    fconfigure $chan -blocking 0 -buffering line -translation binary -eofchar {}
    fileevent $chan readable [list client::ReadHandler $chan]

    # Send stdin to party line service
    fconfigure stdin -blocking 0 -buffering line
    fileevent stdin readable [list client::InputHandler $chan]

    vwait forever
}