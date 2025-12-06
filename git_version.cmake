function(configure_git_version_file IN OUT)
    execute_process(
        COMMAND git log --pretty=format:'%h' -n 1 ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REV
        ERROR_QUIET
    )

    # Check whether we got any revision (which isn't
    # always the case, e.g. when someone downloaded a zip
    # file from Github instead of a checkout)
    if ("${GIT_REV}" STREQUAL "")
        set(GIT_REV "N/A")
        set(GIT_DIFF "")
    else()
        execute_process(
            COMMAND bash -c "git diff --quiet --exit-code -- ${CMAKE_CURRENT_SOURCE_DIR} || echo +"
            OUTPUT_VARIABLE GIT_DIFF
        )

        string(STRIP "${GIT_REV}" GIT_REV)
        string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
        string(STRIP "${GIT_DIFF}" GIT_DIFF)
    endif()

    configure_file(${IN} ${OUT})
endfunction()
