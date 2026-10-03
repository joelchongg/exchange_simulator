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

    # -Wno-interference-size: GCC warns that std::hardware_*_interference_size
    # can vary with -mtune. Everything here is built together with the same
    # flags, so the value cannot differ across translation units.
    set(gcc_only
        -Wuseless-cast
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wno-interference-size)

    target_compile_options(${target} INTERFACE
        ${common}
        $<$<CXX_COMPILER_ID:GNU>:${gcc_only}>
        $<$<BOOL:${EXSIM_WARNINGS_AS_ERRORS}>:-Werror>)
endfunction()
