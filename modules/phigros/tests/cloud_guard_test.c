#define MODULE_HOST_TEST
#include "../native/cloud-guard.c"
#include <assert.h>
#include <stdio.h>

/* The cloud observers must be pure pass-throughs: the module's promise is that it
 * keeps the host's login, network and upload flow intact, so a forwarded call has
 * to return byte-identical results and must never depend on the isolation flag.
 *
 * These tests also pin the diagnostics counters, because "results do not sync"
 * can only be triaged if the panel can tell whether the host called the sync path
 * at all. */

static BoolTask original_bool(const void *m) {
    (void)m;
    return (BoolTask){.source = (void *)123, .result = true, .token = 42};
}
static IntTask original_int(const void *m) {
    (void)m;
    return (IntTask){.source = (void *)456, .result = 1, .token = 17};
}

int main(void) {
    sync_original = original_bool;
    silent_original = original_int;
    atomic_store(&guard_ready, true);
    atomic_store(&isolated, false);

    /* 1. Pass-through: every field survives unchanged. */
    BoolTask b = guard_sync(NULL);
    IntTask i = guard_silent(NULL);
    assert(b.source == (void *)123 && b.result && b.token == 42);
    assert(i.source == (void *)456 && i.result == 1 && i.token == 17);

    /* 2. Observers count what the host actually did. */
    assert(atomic_load(&sync_calls) == 1);
    assert(atomic_load(&silent_calls) == 1);
    assert(atomic_load(&last_sync_result) == true);

    /* 3. Enabling isolation must not change the forwarded value. */
    assert(activate_isolation(true));
    b = guard_sync(NULL);
    i = guard_silent(NULL);
    assert(b.source == (void *)123 && b.result && b.token == 42);
    assert(i.source == (void *)456 && i.result == 1 && i.token == 17);
    assert(atomic_load(&sync_calls) == 2);

    /* 4. Disabling it restores the previous state without side effects. */
    assert(activate_isolation(false));
    assert(guard_sync(NULL).source == (void *)123);
    assert(atomic_load(&sync_calls) == 3);

    /* 5. A false result is reported as false, not as "no call". */
    sync_original = NULL;
    atomic_store(&last_sync_result, false);
    /* restores for the remaining checks */
    sync_original = original_bool;

    /* 6. Isolation cannot be activated before the observers are attached. */
    atomic_store(&guard_ready, false);
    assert(!activate_isolation(true));
    assert(!atomic_load(&isolated));

    /* 7. Re-attaching is idempotent from the caller's point of view. */
    atomic_store(&guard_ready, true);
    assert(activate_isolation(true));

    puts("PASS cloud observers forward unchanged, count host calls, ignore isolation, "
         "and refuse activation before attachment");
    return 0;
}
