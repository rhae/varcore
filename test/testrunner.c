
#include "CUnit/Automated.h"
#include "CUnit/Basic.h"
#include "CUnit/CUnit_intl.h"
#include "varcore_test.h"

static void dump_results();

int main(int argc, char* argv[])
{
  CU_BOOL Run = CU_FALSE;
  CU_BOOL ConsoleOutput = CU_FALSE;

  setvbuf(stdout, NULL, _IONBF, 0);

  if (argc > 1) {
    if (!strcmp("-i", argv[1])) {
      Run = CU_TRUE ;
      CU_set_error_action(CUEA_IGNORE);
    }
    else if (!strcmp("-f", argv[1])) {
      Run = CU_TRUE ;
      CU_set_error_action(CUEA_FAIL);
    }
    else if (!strcmp("-A", argv[1])) {
      Run = CU_TRUE ;
      CU_set_error_action(CUEA_ABORT);
    }
    else if( !strcmp("-C", argv[1])) {
      ConsoleOutput = CU_TRUE;
    }
    #if 0
    else if (!strcmp("-e", argv[1])) {
      print_example_results();
    }
    #endif
    else {
      printf("\nUsage:  AutomatedTest [option]\n\n"
               "        Options: -i  Run, ignoring framework errors [default].\n"
               "                 -f  Run, failing on framework error.\n"
               "                 -A  Run, aborting on framework error.\n"
               "                 -C  Run, console interface\n"
               "                 -h  Print this message.\n\n");
    }
  }
  else {
    Run = CU_TRUE;
    CU_set_error_action(CUEA_IGNORE);
  }

  if (CU_TRUE == Run) {
    CU_automated_enable_junit_xml(CU_TRUE);
    if (CU_initialize_registry()) {
      printf("\nInitialization of Test Registry failed.");
    }
    else {
      test_add_s16();
      test_add_s32();
      test_add_f32();
      test_add_string();
      test_add_enum();
      test_add_dump();

      if( ConsoleOutput ) {
        // CU_console_run_tests();
        CU_basic_run_tests();
      }
      else {
        CU_set_output_filename("TestAutomated");
        CU_list_tests_to_file();
        CU_automated_run_tests();
      }
      dump_results();
      CU_cleanup_registry();
    }
  }

  return 0;
}

void dump_results() {

  unsigned int i;
  CU_pFailureRecord pFailure = CU_get_failure_list();
  CU_pRunSummary pRunSummary = CU_get_run_summary();

  fprintf(stdout, "\n%s",
                    _("--------------- Test Run Summary -------------------------\n"));
  fprintf( stdout, "Package: %s\n", pRunSummary->PackageName );
  fprintf( stdout, "nSuitesRun      = %d\n", pRunSummary->nSuitesRun);
  fprintf( stdout, "nSuitesFailed   = %d\n", pRunSummary->nSuitesFailed);
  fprintf( stdout, "nSuitesInactive = %d\n", pRunSummary->nSuitesInactive);
  fprintf( stdout, "nTestsRun       = %d\n", pRunSummary->nTestsRun);
  fprintf( stdout, "nTestsFailed    = %d\n", pRunSummary->nTestsFailed);
  fprintf( stdout, "nTestsInactive  = %d\n", pRunSummary->nTestsInactive);
  fprintf( stdout, "nAsserts        = %d\n", pRunSummary->nAsserts);
  fprintf( stdout, "nAssertsFailed  = %d\n", pRunSummary->nAssertsFailed);
  fprintf( stdout, "nFailureRecords = %d\n", pRunSummary->nFailureRecords);
  fprintf( stdout, "ElapsedTime     = %.3f [s]\n", pRunSummary->ElapsedTime);
  fprintf( stdout, "nTestsSkipped   = %d\n", pRunSummary->nTestsSkipped);
  fprintf( stdout, "nSuitesSkipped  = %d\n", pRunSummary->nSuitesSkipped);


  if (NULL == pFailure) {
    fprintf(stdout, "\n%s\n", _("No failures."));
  }
  else {

    fprintf(stdout, "\n%s",
                    _("--------------- Test Run Failures -------------------------"));
    fprintf(stdout, "\n%s\n",
                    _("   src_file:line# : (suite:test) : failure_condition"));

    for (i = 1 ; (NULL != pFailure) ; pFailure = pFailure->pNext, i++) {
      fprintf(stdout, "\n%d. %s:%u : (%s : %s) : %s", i,
          (NULL != pFailure->strFileName)
              ? pFailure->strFileName : "",
          pFailure->uiLineNumber,
          ((NULL != pFailure->pSuite) && (NULL != pFailure->pSuite->pName))
              ? pFailure->pSuite->pName : "",
          ((NULL != pFailure->pTest) && (NULL != pFailure->pTest->pName))
              ? pFailure->pTest->pName : "",
          (NULL != pFailure->strCondition)
              ? pFailure->strCondition : "");
    }
    fprintf(stdout, "\n-----------------------------------------------------------");
    fprintf(stdout, "\n");
    fprintf(stdout, _("Total Number of Failures : %-u"), i - 1);
    fprintf(stdout, "\n");
  }
}
