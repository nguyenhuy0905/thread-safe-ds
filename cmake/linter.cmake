# a custom lint target that could be run if on CI/CD

if(NOT DEFINED LINT_CMD)
  set(LINT_CMD cppcheck --error-exitcode=1 -U TSDS_MODULE)
endif()

add_custom_target(lint
  COMMENT "Running linter"
  COMMAND ${LINT_CMD}
  ${PROJECT_SOURCE_DIR}/src
)
