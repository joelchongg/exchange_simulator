function(exsim_set_warnings target)
    set(common
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wimplicit-fallthrough
        -Wcast-align
        -Wformat=2)

    set(gcc_only
        -Wuseless-cast
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op)

    target_compile_options(${target} INTERFACE
        ${common}
        $<$<CXX_COMPILER_ID:GNU>:${gcc_only}>
        $<$<BOOL:${EXSIM_WARNINGS_AS_ERRORS}>:-Werror>)
endfunction()
