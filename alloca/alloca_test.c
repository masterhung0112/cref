#include <cmocka.h>
#include <alloca.h>

void alloca_create(void **state) {
    (void) state; /* unused */
	
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(alloca_create),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
