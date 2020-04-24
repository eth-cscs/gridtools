#!/bin/bash

# no_dependency "<of_module>" "<on_module>"
# checks that <of_module> does not depend on <on_module>
# i.e. <of_module> does not include any file from <on_module>
function no_dependency() {
    last_result=`grep -r "#include .*$2/.*hpp" include/gridtools/$1 | wc -l`
    if [ "$last_result" -gt 0 ]; then
        echo "ERROR Modularization violated: found dependency of $1 on $2"
        echo "`grep -r "#include .*$2/.*hpp" include/gridtools/$1`"
    fi
    modularization_result=$(( modularization_result || last_result ))
}

function are_independent() {
    no_dependency "$1" "$2"
    no_dependency "$2" "$1"
}
modularization_result=0

# list of non-dependencies
no_dependency "meta" "common"
no_dependency "common" "stencil"
no_dependency "common" "boundaries"
no_dependency "common" "gcl"
no_dependency "common" "storage"
no_dependency "common" "c_bindings"

are_independent "stencil" "boundaries"
are_independent "stencil" "gcl"
are_independent "stencil" "layout_transformation"

are_independent "gcl" "layout_transformation"
are_independent "gcl" "storage"

are_independent "boundaries" "c_bindings"
are_independent "boundaries" "layout_transformation"

no_dependency "storage" "stencil"
no_dependency "storage" "boundaries"

# we cannot use an exit code here because the git hook will terminate immediately
