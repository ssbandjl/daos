#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>


static int
sched_ut_setup(void **state)
{
	return 0;
}

static int
sched_ut_teardown(void **state)
{
	return 0;
}

static void
sched_test_6(void **state)
{

	print_message("Init Scheduler\n");
}

static const struct CMUnitTest sched_uts[] = {
	{ "SCHED_Test_6", sched_test_6, NULL, NULL}, // 依赖任务测试
};

int main(int argc, char **argv)
{

	return cmocka_run_group_tests_name("Event Queue unit tests", sched_uts,
					   sched_ut_setup, sched_ut_teardown);
}