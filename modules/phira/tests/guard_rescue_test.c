#include <assert.h>
#include <stdio.h>
#include "goal-plan.h"

/* The Phira native module never compiled before this change: controlled-frame.h
 * called make_profile_timed_ex and guard_needed_judgment, and module.c called
 * guard_rescue, none of which existed anywhere in either repository. These tests
 * pin the two functions that had to be written so the guard's behaviour is defined
 * rather than whatever happened to compile.
 */

static void test_guard_rescue(void) {
    /* AP protection upgrades anything below Perfect. */
    assert(guard_rescue(1, 100, 99, 0, 0, 0, 99, 99, 3, 0) == 0);
    assert(guard_rescue(2, 100, 99, 0, 0, 0, 99, 99, 3, 0) == 0);
    assert(guard_rescue(3, 100, 99, 0, 0, 0, 99, 99, 3, 0) == 0);

    /* A natural Perfect is never downgraded. */
    assert(guard_rescue(0, 100, 50, 0, 0, 0, 50, 50, 3, 0) == 0);
    assert(guard_rescue(0, 100, 50, 0, 0, 0, 50, 50, 1, 990000) == 0);

    /* FC protection rescues Bad and Miss but leaves a Good alone. */
    assert(guard_rescue(2, 100, 50, 10, 0, 0, 0, 50, 2, 0) == 0);
    assert(guard_rescue(3, 100, 50, 10, 0, 0, 0, 50, 2, 0) == 0);
    assert(guard_rescue(1, 100, 50, 10, 0, 0, 10, 50, 2, 0) == 1);

    /* Score floor: reachable means no intervention. */
    assert(guard_rescue(3, 100, 0, 0, 0, 0, 0, 0, 1, 900000) == 3);

    /* Score floor: unreachable without help means rescue to Perfect. */
    assert(guard_rescue(3, 100, 0, 0, 0, 50, 0, 0, 1, 900000) == 0);

    /* Guard off leaves the judgment untouched. */
    assert(guard_rescue(2, 100, 0, 0, 0, 0, 0, 0, 0, 0) == 2);

    /* Degenerate input is returned unchanged instead of corrupting the run. */
    assert(guard_rescue(3, 0, 0, 0, 0, 0, 0, 0, 3, 0) == 3);
    assert(guard_rescue(7, 100, 0, 0, 0, 0, 0, 0, 3, 0) == 7);
}

static void test_guard_needed(void) {
    /* AP protection always wants Perfect on the next note. */
    assert(guard_needed_judgment(100, 0, 0, 0, 0, 0, 0, 3, 0, 1) == 0);

    /* FC protection wants Perfect while the chain is clean. */
    assert(guard_needed_judgment(100, 10, 0, 0, 0, 10, 10, 2, 0, 1) == 0);

    /* A score floor that is still reachable needs no intervention. */
    assert(guard_needed_judgment(100, 0, 0, 0, 0, 0, 0, 1, 900000, 1) == -1);

    /* A floor that is already out of reach needs intervention. */
    assert(guard_needed_judgment(100, 0, 0, 0, 50, 0, 0, 1, 900000, 1) == 0);

    /* No notes left means nothing to rescue. */
    assert(guard_needed_judgment(10, 10, 0, 0, 0, 0, 0, 3, 0, 1) == -1);

    /* Guard off means never intervene. */
    assert(guard_needed_judgment(100, 0, 0, 0, 0, 0, 0, 0, 0, 1) == -1);

    /* A note type that cannot take a Good must be rescued to Perfect. */
    assert(guard_needed_judgment(100, 0, 0, 0, 50, 0, 0, 1, 900000, 0) == 0);
}

int main(void) {
    test_guard_rescue();
    test_guard_needed();
    puts("PASS phira guard: rescue upgrades only, AP/FC/score-floor semantics, "
         "no-notes and guard-off are no-ops");
    return 0;
}
