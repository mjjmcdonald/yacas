set (CTEST_PROJECT_NAME "yacas")

set (YACAS_TESTS
  arithmetic.yts
  association.yts
  binaryfactors.yts
  bitops.yts
  c_tex_form.yts
  calculus.yts
  calendar.yts
  canprove.yts
  comments.yts
  complex.yts
  cyclotomic.yts
  deriv.yts
  dimensions.yts
  dot.yts
  GaussianIntegers.yts
  graphs.yts
  integrate.yts
  io.yts
  journal.yts
  linalg.yts
  lists.yts
  logic_simplify_test.yts
  macro.yts
  matrixpower.yts
  multivar.yts
  newly.yts
  nthroot.yts
  numbers.yts
  numerics.yts
  nummethods.yts
  ode.yts
  openmath.yts
  orthopoly.yts
  outer.yts
  padic.yts
  physics.yts
  plots.yts
  poly.yts
  predicates.yts
  products.yts
  programming.yts
  radsimp.yts
  rational.yts
  regress.yts
  scopestack.yts
  simplify.yts
  solve.yts
  sturm.yts
  sums.yts
  tensors.yts
  tr.yts
  trace.yts
  transforms.yts)


install (FILES ${YACAS_TESTS} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/yacas/tests COMPONENT app)
install (PROGRAMS ${TEST_YACAS_CMD} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/yacas/tests COMPONENT app)

if (ENABLE_CYACAS_CONSOLE)
    foreach (_test ${YACAS_TESTS})
        if (WIN32)
            add_test (NAME cyacas-${_test} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND "tests/test-yacas.bat" "$<TARGET_FILE:yacas> -pc --rootdir scripts" "tests" "${_test}")
        elseif (APPLE)
            add_test (NAME cyacas-${_test} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND "tests/test-yacas" "$<TARGET_FILE:yacas> -pc --rootdir scripts" "tests" "${_test}")
        else ()
            add_test (NAME cyacas-${_test} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND "tests/test-yacas" "'$<TARGET_FILE:yacas>' -pc --rootdir scripts" "tests" "${_test}")
        endif ()
    endforeach ()
endif ()

if (ENABLE_JYACAS)
    foreach (_test ${YACAS_TESTS})
        add_test (NAME jyacas-${_test} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND "${TEST_YACAS_CMD}" "${Java_JAVA_EXECUTABLE} -jar $<TARGET_PROPERTY:jyacas,JAR_FILE>" ${PROJECT_SOURCE_DIR}/tests ${_test})
    endforeach ()
endif ()
